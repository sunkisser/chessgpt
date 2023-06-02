#include <omp.h>
#include <stdio.h>
#include <time.h>
#include "search.h"
#include "attacked.h"
#include "board.h" // Assuming that this file contains the generateMoves and make_move functions
#include "evaluate.h"
#include "move.h"
#include "uci.h"

// Global variable for node count
unsigned long long nodeCount = 0;

// Recursive depth-first search function with alpha-beta pruning
int dfs(Game *game, int depth, int alpha, int beta) {
#pragma omp atomic
        nodeCount++;

    // If we've reached the maximum depth, return the static evaluation score
    if (depth == 0) {
		return evaluate(&game->board);
    }

	// Check if the position is a repeated position
	int repeatedcount = 0;
    for (int i = 0; i < game->positionHistoryLength - 1; i++) {
        if (game->board.zobristKey == game->positionHistory[i])
			repeatedcount++;
        if (repeatedcount > 1)
			return 100;
	}

    // Generate all legal moves
    Move moveList[MAX_MOVES];
    int moveCount = generateMoves(&game->board, moveList);

    // If there are no legal moves, this is a checkmate or stalemate.
    if (moveCount == 0) {
        int kingSquare = __builtin_ffsll(game->board.kings[game->board.sideToMove]) - 1;
        if (is_square_attacked(&game->board, kingSquare)) {
            // The king of the current side to move is in check, so this is a  checkmate
            return -100000-depth;
        } else {
            // The king is not in check, so this is a stalemate
            return 0; // or some other score that represents a draw
        }
    }

    // For each legal move, make that move, then recursively search the resulting position. Keep track of the best score found.
    int bestScore = -1000000;
    for (int i = 0; i < moveCount; i++) {
        // Make a copy of the game and make the move on the copy
        Game tempGame = *game;
        game_make_move(&tempGame, moveList[i]);

        // Recursively search the resulting position
        int score = -dfs(&tempGame, depth - 1, -beta, -alpha);

        // If this score is the best so far, update bestScore
        bestScore = (score > bestScore) ? score : bestScore;

        // Alpha-beta pruning condition
        alpha = (score > alpha) ? score : alpha;
        if (alpha >= beta) {
            break; // beta cut-off
        }
    }

    // After searching all moves, return the best score found
    return bestScore;
}

Move searchBestMove(Game *game, const char *goCommand, int depth) {
	// Initialize variables at the start of each search
	nodeCount = 0;
	clock_t startTime = omp_get_wtime();

	if (!goCommand) {
	};

	// Generate all legal moves
	Move moveList[MAX_MOVES];
	int moveCount = generateMoves(&game->board, moveList);
	printf("info string Generated %d moves\n", moveCount);

	// If there are no legal moves, this is a checkmate or stalemate.
	// Handle this case in some appropriate way. Here, I'll just return a dummy move.
	if (moveCount == 0) {
		return 0;
	}

	// For each legal move, make that move, then use dfs to search the resulting position.
	// Keep track of the best move and its score.
	Move bestMove = moveList[0];
	int bestScore = -1000000; // Start with a large negative number
	int alpha = -1000000;
	int beta = 1000000;

#pragma omp parallel for
	for (int i = 0; i < moveCount; i++) {
		// Make a copy of the game and make the move on the copy
		Game tempGame = *game;
		game_make_move(&tempGame, moveList[i]);

		// Use dfs to search the resulting position
		int score = -dfs(&tempGame, depth - 1, -beta, -alpha); // Note the minus sign here

		// Penalty for bishops and knights still on starting squares
		int adjustment = position_penalty(&tempGame.board, tempGame.board.sideToMove);
		score -= adjustment;

		// Evaluation information
		// Convert the best move to UCI format
		char bestMoveUci[6];
		moveToUCI(bestMove, bestMoveUci);

		char thisMoveUci[6];
		moveToUCI(moveList[i], thisMoveUci);

		// Performance report
		double currentTime = omp_get_wtime();

		double elapsedTime = currentTime - startTime;
		unsigned long long nps = elapsedTime > 0 ? nodeCount / elapsedTime : 0;

#pragma omp critical
		{
			if (score > bestScore) {
				bestMove = moveList[i];
				bestScore = score;
				alpha = score;
			}
		}
		printf("info depth %d score cp %d thismove %s thismovescore %d penalty %d nodes %llu nps %llu pv %s\n", depth, bestScore, thisMoveUci, score, adjustment, nodeCount, nps, bestMoveUci);
	}

	// After searching all moves, return the best move found
	return bestMove;
}
