#ifndef MOVE_H
#define MOVE_H

#include "board.h"

typedef enum { EMPTY = 0, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING } Piece;

typedef uint64_t Move;
#define MAX_MOVES 218

// Bits 0-5 store the 'from' square (0 to 63)
#define FROM_SQUARE(move) ((move)&0x3F)

// Bits 6-11 store the 'to' square (0 to 63)
#define TO_SQUARE(move) (((move) >> 6) & 0x3F)

// Bits 12-14 store the promoted piece (QUEEN = 5, ROOK = 4, BISHOP = 3, KNIGHT = 2)
#define PROMOTED_PIECE(move) (((move) >> 12) & 0x7)

// Bits 15-17 store the captured piece (same encoding as promoted piece)
#define CAPTURED_PIECE(move) (((move) >> 15) & 0x7)

// Bit 18 is for en passant
#define IS_EN_PASSANT(move) (((move) >> 18) & 0x1)

// Bit 19 is for castling
#define IS_CASTLING(move) (((move) >> 19) & 0x1)

// Bit 20 is for pawn double move
#define IS_DOUBLE_PAWN_MOVE(move) (((move) >> 20) & 0x1)

// Bits 21-31 store the move score (range -1024 to 1023)
#define MOVE_SCORE(move) ((int8_t)(((move) >> 21) & 0xFF))

#define CREATE_MOVE(from, to, promotedPiece, capturedPiece, enPassant, castling, doublePawnMove, score)                                                                                                \
	((from) | ((to) << 6) | ((promotedPiece) << 12) | ((capturedPiece) << 15) | ((enPassant) << 18) | ((castling) << 19) | ((doublePawnMove) << 20) | ((score & 0xFF) << 21))

int generateMoves(Board *board, Move *moves);
void make_move(Board *board, Move move);
void printMove(Move move);

#endif
