#ifndef PERFT_H
#define PERFT_H

#include "game.h"

typedef struct {
  uint64_t nodes;
  uint64_t captures;
  uint64_t enPassants;
  uint64_t promotions;
  uint64_t castlings;
} PerftResult;

uint64_t performPerft(Game *game, int depth);
void perftFromEPD(const char *filePath);

#endif
