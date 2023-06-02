#include <stdlib.h>
#include "zobrist.h"

// 6 for different piece types, 2 for colors, 64 for different squares
uint64_t zobrist_piece_keys[6][2][64];
// 2 for colors, 2 for castle side (king/queen)
uint64_t zobrist_castling_keys[2][2];

uint64_t zobrist_ep_keys[8]; // 8 possible files for en passant
uint64_t zobrist_side_key;	 // Different key for black's turn

void init_zobrist() {
	// Randomly initialize the keys using a PRNG
	for (int piece = 0; piece < 6; ++piece) {
		for (int color = 0; color < 2; ++color) {
			for (int square = 0; square < 64; ++square) {
				zobrist_piece_keys[piece][color][square] = (uint64_t)rand();
			}
		}
	}
	for (int color = 0; color < 2; ++color) {
		for (int castle_side = 0; castle_side < 2; ++castle_side) {
			zobrist_castling_keys[color][castle_side] = (uint64_t)rand();
		}
	}
	for (int i = 0; i < 8; ++i) {
		zobrist_ep_keys[i] = (uint64_t)rand();
	}
	zobrist_side_key = (uint64_t)rand();
}

uint64_t compute_zobrist_key(const Board *board) {
	uint64_t key = 0;
	uint64_t pieces[6][2] = {
		{board->pawns[WHITE], board->pawns[BLACK]}, {board->knights[WHITE], board->knights[BLACK]}, {board->bishops[WHITE], board->bishops[BLACK]},
		{board->rooks[WHITE], board->rooks[BLACK]}, {board->queens[WHITE], board->queens[BLACK]},	{board->kings[WHITE], board->kings[BLACK]},
	};

	for (int piece = 0; piece < 6; ++piece) {
		for (int color = 0; color < 2; ++color) {
			uint64_t bitboard = pieces[piece][color];
			while (bitboard) {
				int square = __builtin_ctzll(bitboard); // find the index of the least significant bit
				key ^= zobrist_piece_keys[piece][color][square];
				bitboard &= bitboard - 1; // unset the least significant bit
			}
		}
	}

	for (int color = 0; color < 2; ++color) {
		for (int castle_side = 0; castle_side < 2; ++castle_side) {
			if (board->castleRights[color] & (1 << castle_side)) {
				key ^= zobrist_castling_keys[color][castle_side];
			}
		}
	}

	if (board->enPassantSquare >= 0) {
		key ^= zobrist_ep_keys[board->enPassantSquare % 8];
	}

	if (board->sideToMove == BLACK) {
		key ^= zobrist_side_key;
	}

	return key;
}
