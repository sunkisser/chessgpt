#ifndef UCI_H
#define UCI_H

#include "game.h"
#include "move.h"

void uciLoop(Game *game);
void moveToUCI(Move move, char *str);

#endif
