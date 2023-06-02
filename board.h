#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>

#define BOARD_SIZE 8
#define WHITE 0
#define BLACK 1

typedef struct {
  uint64_t pawns[2];    // lsb is a1
  uint64_t knights[2];  // lsb is a1
  uint64_t bishops[2];  // lsb is a1
  uint64_t rooks[2];    // lsb is a1
  uint64_t queens[2];   // lsb is a1
  uint64_t kings[2];    // lsb is a1
  uint64_t occupied[2]; // lsb is a1
  int sideToMove;       // 0 for white, 1 for black
  int enPassantSquare;  // store the file number of the en passant square, -1 if none
  int castleRights[2];  // Store castling rights.
  uint64_t zobristKey;  // This will be useful later for the transposition table
} Board;

void initializeBoard(Board *board);
void setBoardtoFEN(Board *board, const char *fen);

#endif
