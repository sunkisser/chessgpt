#ifndef EVALUATE_H
#define EVALUATE_H

#include "board.h"

int evaluate(Board *board);
int position_penalty(const Board *board, int side);

#endif // EVALUATE_H
