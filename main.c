#include <string.h>
#include "game.h"
#include "perft.h"
#include "uci.h"
#include "zobrist.h"

// Credit & Thanks to BlueFever Software
// https://youtube.com/playlist?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg

// Credit & Thanks to ChatGPT https://chat.openai.com 
// Credit & Thanks to https://github.com/ChrisWhittington/Chess-EPDs
// Credit & Thanks to Marcel for perft-marcel.epd 
// Credit & Thanks to Chess Programming Wiki https://www.chessprogramming.org/Main_Page 

// Code written 99%+ by ChatGPT.  

// Prompts inspired & informed by 
// the knowledge gained from the BlueFever Software Youtube Tutorial Series.

int main(int argc, char *argv[]) {
	Game game;

	// At the start of your engine
	init_zobrist();

	game_newGame(&game);

	if (argc > 1) {
		// The second argument is argv[1], argv[0] is the program name itself
		if (strcmp(argv[1], "perft") == 0) {
			performPerft(&game, 6);
			return 0;
		}
	}

	uciLoop(&game);

	return 0;
}
