#ifndef SEARCH_H
#define SEARCH_H

#include "game.h"
#include "move.h"

Move searchBestMove(Game *game, const char *goCommand, int depth);

#endif
