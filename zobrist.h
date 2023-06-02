#ifndef ZOBRIST_H
#define ZOBRIST_H

#include <stdint.h>
#include "board.h"

// Initialize the Zobrist keys
void init_zobrist();

// Calculate the Zobrist key for a given board
uint64_t compute_zobrist_key(const Board *board);

#endif
