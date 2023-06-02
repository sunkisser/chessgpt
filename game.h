#ifndef GAME_H
#define GAME_H

#include "board.h"
#include "move.h"

#define MAX_POSITION_HISTORY 1000 // This can be adjusted as needed

typedef struct {
  Board board;
  uint64_t positionHistory[MAX_POSITION_HISTORY]; // Add this line
  int positionHistoryLength;                      // And this line
  // Add other game state variables as needed
} Game;

void game_newGame(Game *game);
void game_setFEN(Game *game, const char *fen);
void game_make_move(Game *game, Move move);
void printGame(const Game *game);
void printBoard(const Board *board);
void print_bitboard(uint64_t bb);
// Add other function prototypes as needed

#endif
