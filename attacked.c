#include <stdio.h>
#include "attacked.h"
#include "board.h"
#include "game.h"

// These arrays define the rank and file offsets for each piece
int kingRankOffsets[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
int kingFileOffsets[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

int knightRankOffsets[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
int knightFileOffsets[8] = {1, 2, 2, 1, -1, -2, -2, -1};

int rookRankOffsets[4] = {-1, 0, 0, 1};
int rookFileOffsets[4] = {0, -1, 1, 0};

int bishopRankOffsets[4] = {-1, -1, 1, 1};
int bishopFileOffsets[4] = {-1, 1, -1, 1};

// ... (define the pawn rank and file offsets as needed)

int is_square_attacked(Board *board, int square) {
	int side = (board->sideToMove + 1) % 2; // the attacking side is the opposite of the side to move
	int rank = square / 8;
	int file = square % 8;

	// Check for pawn attacks
	if (side == WHITE && rank > 0) {
		if (file > 0 && board->pawns[WHITE] & (1ULL << (square - 9))) {
			return 1;
		}
		if (file < 7 && board->pawns[WHITE] & (1ULL << (square - 7))) {
			return 1;
		}
	} else if (side == BLACK && rank < 7) { // side == BLACK
		if (file > 0 && board->pawns[BLACK] & (1ULL << (square + 7))) {
			return 1;
		}
		if (file < 7 && board->pawns[BLACK] & (1ULL << (square + 9))) {
			return 1;
		}
	}

	// Check for knight attacks
	for (int i = 0; i < 8; i++) {
		int targetRank = rank + knightRankOffsets[i];
		int targetFile = file + knightFileOffsets[i];

		// Check for out of bounds
		if (targetRank < 0 || targetRank >= 8 || targetFile < 0 || targetFile >= 8) {
			continue;
		}

		int targetSquare = targetRank * 8 + targetFile;
		if (board->knights[side] & (1ULL << targetSquare)) {
			return 1;
		}
	}

	// Check for bishop and queen diagonal attacks
	for (int i = 0; i < 4; i++) {
		int targetRank = rank + bishopRankOffsets[i];
		int targetFile = file + bishopFileOffsets[i];

		while (targetRank >= 0 && targetRank < 8 && targetFile >= 0 && targetFile < 8) {
			int targetSquare = targetRank * 8 + targetFile;

			// If the square is occupied by any piece, stop
			if (board->occupied[WHITE] & (1ULL << targetSquare) || board->occupied[BLACK] & (1ULL << targetSquare)) {
				if (board->bishops[side] & (1ULL << targetSquare) || board->queens[side] & (1ULL << targetSquare)) {
					return 1;
				}
				break;
			}

			targetRank += bishopRankOffsets[i];
			targetFile += bishopFileOffsets[i];
		}
	}

	// Check for rook and queen straight attacks
	for (int i = 0; i < 4; i++) {
		int targetRank = rank + rookRankOffsets[i];
		int targetFile = file + rookFileOffsets[i];

		while (targetRank >= 0 && targetRank < 8 && targetFile >= 0 && targetFile < 8) {
			int targetSquare = targetRank * 8 + targetFile;

			// If the square is occupied by any piece, stop
			if (board->occupied[WHITE] & (1ULL << targetSquare) || board->occupied[BLACK] & (1ULL << targetSquare)) {
				if (board->rooks[side] & (1ULL << targetSquare) || board->queens[side] & (1ULL << targetSquare)) {
					return 1;
				}
				break;
			}

			targetRank += rookRankOffsets[i];
			targetFile += rookFileOffsets[i];
		}
	}

	// Check for king attacks
	for (int i = 0; i < 8; i++) {
		int targetRank = rank + kingRankOffsets[i];
		int targetFile = file + kingFileOffsets[i];

		// Check for out of bounds
		if (targetRank < 0 || targetRank >= 8 || targetFile < 0 || targetFile >= 8) {
			continue;
		}

		int targetSquare = targetRank * 8 + targetFile;
		if (board->kings[side] & (1ULL << targetSquare)) {
			return 1;
		}
	}

	return 0;
}
