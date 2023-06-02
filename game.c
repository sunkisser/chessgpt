#include <stdio.h>
#include "game.h"
#include "board.h"

// Define the piece characters
const char emptySquare = '.';
const char whitePawns = 'P';
const char whiteKnights = 'N';
const char whiteBishops = 'B';
const char whiteRooks = 'R';
const char whiteQueens = 'Q';
const char whiteKing = 'K';
const char blackPawns = 'p';
const char blackKnights = 'n';
const char blackBishops = 'b';
const char blackRooks = 'r';
const char blackQueens = 'q';
const char blackKing = 'k';

void game_newGame(Game *game) {
	// Initialize a new board for the new game
	initializeBoard(&game->board);
	// Initialize other game state variables as needed
	game->positionHistoryLength = 0;
}

void game_setFEN(Game *game, const char *fen) {
	// Convert the FEN string to your game state here.
	setBoardtoFEN(&game->board, fen);
	printf("info string Board set to new FEN.\n");

	game->positionHistoryLength = 0;
	game->positionHistory[game->positionHistoryLength] = game->board.zobristKey;
	game->positionHistoryLength++;
}

void game_make_move(Game *game, Move move) {
	make_move(&game->board, move);

	if (game->positionHistoryLength < MAX_POSITION_HISTORY) {
		game->positionHistory[game->positionHistoryLength] = game->board.zobristKey;
		game->positionHistoryLength++;
	} else {
		// Handle the case where positionHistory is full - you might want to
		// simply stop the game at this point
	}
}

void print_bitboard(uint64_t bb) {
	for (int rank = 7; rank >= 0; rank--) {
		for (int file = 0; file < 8; file++) {
			int square = rank * 8 + file;
			printf("%llu ", (bb >> square) & 1ULL);
		}
		printf("\n");
	}
}

void printBoard(const Board *board) {

	printf("\n");
	for (int rank = BOARD_SIZE - 1; rank >= 0; --rank) {
		for (int file = 0; file < BOARD_SIZE; ++file) {
			int square = rank * BOARD_SIZE + file;
			int pieceType = (board->pawns[WHITE] >> square) & 1		? whitePawns
							: (board->knights[WHITE] >> square) & 1 ? whiteKnights
							: (board->bishops[WHITE] >> square) & 1 ? whiteBishops
							: (board->rooks[WHITE] >> square) & 1	? whiteRooks
							: (board->queens[WHITE] >> square) & 1	? whiteQueens
							: (board->kings[WHITE] >> square) & 1	? whiteKing
							: (board->pawns[BLACK] >> square) & 1	? blackPawns
							: (board->knights[BLACK] >> square) & 1 ? blackKnights
							: (board->bishops[BLACK] >> square) & 1 ? blackBishops
							: (board->rooks[BLACK] >> square) & 1	? blackRooks
							: (board->queens[BLACK] >> square) & 1	? blackQueens
							: (board->kings[BLACK] >> square) & 1	? blackKing
																	: emptySquare;

			// Print the piece character
			printf("%c ", pieceType);
		}
		printf("\n");
	}
	printf("\n");

	// Print side to move
	printf("Side to move: %s\n", board->sideToMove == WHITE ? "White" : "Black");

	// Print en passant square
	if (board->enPassantSquare != -1) {
		int file = board->enPassantSquare % BOARD_SIZE;
		int rank = board->enPassantSquare / BOARD_SIZE;
		printf("En passant square: %c%d\n", 'a' + file, rank + 1);
	} else {
		printf("En passant square: -\n");
	}

	// Print castling rights
	const char *castleRightsSymbols[2] = {"KQ", "kq"};
	for (int color = 0; color <= 1; ++color) {
		printf("%s can castle: ", color == WHITE ? "White" : "Black");
		if (board->castleRights[color] == 0) {
			printf("-");
		} else {
			for (int i = 0; i < 2; ++i) {
				if ((board->castleRights[color] >> i) & 1) {
					printf("%c", castleRightsSymbols[color][i]);
				}
			}
		}
		printf("\n");
	}

	// Print Zobrist key
	printf("Zobrist key: %lu  \n", board->zobristKey);
}

void printGame(const Game *game) {
    const Board *board = &game->board;
    printBoard(board);

    // Print out the Zobrist keys
    printf("Zobrist keys:\n");
    for (int i = 0; i < game->positionHistoryLength; i++) {
        printf("%lu\n", game->positionHistory[i]);
    }
}
