#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uci.h"
#include "board.h"
#include "evaluate.h"
#include "game.h"
#include "move.h"
#include "perft.h"
#include "search.h"

Move uciToMove(Board *board, const char *str) {
	unsigned int from = (str[0] - 'a') + 8 * (str[1] - '1');
	unsigned int to = (str[2] - 'a') + 8 * (str[3] - '1');
	unsigned int promotion = str[4] ? str[4] : 0;

	// Convert the promotion piece from UCI format to your internal format
	switch (promotion) {
	case 'q':
		promotion = QUEEN;
		break;
	case 'r':
		promotion = ROOK;
		break;
	case 'b':
		promotion = BISHOP;
		break;
	case 'n':
		promotion = KNIGHT;
		break;
	default:
		promotion = EMPTY;
		break;
	}

	Move moveList[MAX_MOVES];
	int moveCount = generateMoves(board, moveList);

	// Find the move in the move list that matches the from, to, and promotion pieces
	for (int i = 0; i < moveCount; i++) {
		if (FROM_SQUARE(moveList[i]) == from && TO_SQUARE(moveList[i]) == to && PROMOTED_PIECE(moveList[i]) == promotion) {
			return moveList[i];
		}
	}

	// The move was not found in the move list, return an invalid move
	return 0;
}

void moveToUCI(Move move, char *str) {
	// Convert the from square to UCI format
	str[0] = 'a' + (FROM_SQUARE(move) % 8);
	str[1] = '1' + (FROM_SQUARE(move) / 8);

	// Convert the to square to UCI format
	str[2] = 'a' + (TO_SQUARE(move) % 8);
	str[3] = '1' + (TO_SQUARE(move) / 8);

	// Convert the promotion piece to UCI format
	switch (PROMOTED_PIECE(move)) {
	case QUEEN:
		str[4] = 'q';
		break;
	case ROOK:
		str[4] = 'r';
		break;
	case BISHOP:
		str[4] = 'b';
		break;
	case KNIGHT:
		str[4] = 'n';
		break;
	default:
		str[4] = '\0';
		break;
	}

	// Null terminate the string
	str[5] = '\0';
}

void uciLoop(Game *game) {
	int depth = 6;
	FILE *logFile = fopen("logfile.txt", "w"); // Open a new log file

	// Check if the file was opened successfully
	if (logFile == NULL) {
		printf("error opening log file\n");
		return;
	}

	char buffer[4096];
	Move moveList[MAX_MOVES];

	while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
		// Remove the newline character
		buffer[strcspn(buffer, "\n")] = 0;

		// Log the input from stdin
		fprintf(logFile, "Input: %s\n", buffer);
		fflush(logFile);

		if (strcmp(buffer, "uci") == 0) {
			// Send the "uciok" response
			printf("id name MyChessEngine\n");
			printf("id author MyName\n");
			printf("option name Depth type spin default 6 min 1 max 100\n");
			printf("uciok\n");
			fflush(stdout);

			// Log the output
			fprintf(logFile, "Output: id name MyChessEngine\n");
			fprintf(logFile, "Output: id author MyName\n");
			fprintf(logFile, "Output: uciok\n");
			fflush(logFile);
		} else if (strcmp(buffer, "isready") == 0) {
			// Send the "readyok" response
			printf("readyok\n");
			fflush(stdout);

			fprintf(logFile, "Output: readyok\n");
			fflush(logFile);
		} else if (strcmp(buffer, "ucinewgame") == 0) {
			// Initialize a new game
			game_newGame(game);
			printf("info string New game initialized\n");
			fflush(stdout);

			fprintf(logFile, "Output: info string New game initialized\n");
			fflush(logFile);
		} else if (strncmp(buffer, "setoption name Depth value ", 26) == 0) {
			int newDepth = atoi(buffer + 26);
			// Validate and set the depth. Here, I assume `depth` is a variable in scope.
			if (newDepth >= 1 && newDepth <= 100) {
				depth = newDepth;
				printf("info string depth set to %d\n",depth);
				fprintf(logFile, "Output: Set search depth to %d\n", depth);
			} else {
				printf("info string ignored invalid depth setting: %d\n", newDepth);
				fprintf(logFile, "Output: Ignored invalid depth setting: %d\n", newDepth);
			}
			fflush(stdout);
			fflush(logFile);
		} else if (strncmp(buffer, "position", 8) == 0) {
			// We got a "position" command
			if (strncmp(buffer, "position startpos moves", 22) == 0) {
				game_newGame(game); // Set up the starting position
				// Handle the list of moves
				char *moves = strdup(buffer + 23); // Create a copy of the moves string
				char *move = strtok(moves, " ");
				int moveCount = 0; // Counter for the number of moves applied
				while (move != NULL) {
					Move internalMove = uciToMove(&game->board, move);
					game_make_move(game, internalMove);
					move = strtok(NULL, " ");
					moveCount++; // Increment the counter for each move applied
				}
				free(moves); // Don't forget to free the copy when you're done
				printf("info string Applied %d moves\n",
					   moveCount); // Display the number of moves applied
				fflush(stdout);

				fprintf(logFile, "Output: info string Applied  moves\n");
				fflush(logFile);

			} else if (strncmp(buffer, "position startpos", 17) == 0) {
				// We got a "position startpos" command
				game_newGame(game);
				printf("info string Received position startpos.\n");
				fflush(stdout);

				fprintf(logFile, "Output: info string Received position startpos\n");
				fflush(logFile);
			} else if (strncmp(buffer, "position fen", 12) == 0) {
				// We got a "position fen" command
				char *movesStart = strstr(buffer, "moves");

				// If a move list is present, split the command into a FEN string and move list
				if (movesStart != NULL) {
					// Replace the space before "moves" with a null terminator to end the FEN string
					*(movesStart - 1) = '\0';
					// Start the move list after the "moves " part of the command
					movesStart += 6;
				}

				char *fen = strdup(buffer + 13); // copy the rest of the string after "position fen "
				printf("info string Received FEN: %s\n", fen);
				fflush(stdout);

				fprintf(logFile, "Output: info string Received FEN: \n");
				fflush(logFile);
				game_setFEN(game, fen); // Here's the call to game_setFEN
				free(fen);

				// If a move list is present, apply each move in the list
				if (movesStart != NULL) {
					char *moves = strdup(movesStart); // Create a copy of the moves string
					char *move = strtok(moves, " ");
					int moveCount = 0; // Counter for the number of moves applied
					while (move != NULL) {
						Move internalMove = uciToMove(&game->board, move);
						game_make_move(game, internalMove);
						move = strtok(NULL, " ");
						moveCount++; // Increment the counter for each move  applied
					}
					free(moves); // Don't forget to free the copy when you're done
					printf("info string Applied %d moves\n",
						   moveCount); // Display the number of moves applied
					fflush(stdout);

					fprintf(logFile, "Output: info string Applied \n");
					fflush(logFile);
				}
			}
		} else if (strncmp(buffer, "go", 2) == 0) {
			// We got a "go" command
			printf("info string Received go command\n");
			fflush(stdout);

			fprintf(logFile, "info string Received go command\n");
			fflush(logFile);

			// Search for the best move
			Move bestMove = searchBestMove(game, buffer, depth);

			// Check if a valid move was found
			if (bestMove != 0) {
				// Make the chosen move
				game_make_move(game, bestMove);

				// Send the chosen move to the user interface
				char uciMove[6];
				moveToUCI(bestMove, uciMove);
				printf("bestmove %s\n", uciMove);
				fflush(stdout);

				fprintf(logFile, "bestmove %s\n", uciMove);
			} else {
				printf("info string No legal moves\n");
				fflush(stdout);

				fprintf(logFile, "info string No legal moves\n");
			}
		} else if (strcmp(buffer, "quit") == 0) {
			// The "quit" command terminates the loop
			break;
		}
		//
		// NONSTANDARD COMMANDS
		//
		else if (strcmp(buffer, "print") == 0) {
			// Print the current board position
			printGame(game);
		} else if (strcmp(buffer, "movelist") == 0) {
			// Call generateMoves and print the generated moves
			int moveCount = generateMoves(&(game->board), moveList); // Assume game contains a Board struct named board

			printf("info string Generated %d moves:\n", moveCount);
			for (int i = 0; i < moveCount; i++) {
				printf("Move %d: from %lu to %lu : ", i + 1, (unsigned long)FROM_SQUARE(moveList[i]), (unsigned long)TO_SQUARE(moveList[i]));
				printMove(moveList[i]);

				// Make a copy of the board and make the move on the copy
				Board tempBoard = game->board;
				make_move(&tempBoard, moveList[i]);

				// Use evaluate to get the static evaluation score
				int score = evaluate(&tempBoard);

				printf(" | Eval Score: %d", score);
				printf("\n");
			}
		} else if (strncmp(buffer, "perftEPD", 8) == 0) {
			// We got a "perftEPD" command
			char *filePath = strdup(buffer + 9); // Copy the rest of the string after "perftEPD "
			printf("info string Received EPD file path: %s\n", filePath);
			perftFromEPD(filePath);
			free(filePath);
		} else if (strncmp(buffer, "perft", 5) == 0) {
			// We got a "perft" command
			int depth = atoi(buffer + 6); // Convert the rest of the string after "perft " to an integer
			if (depth <= 0) {
				printf("info string Invalid perft depth: %d\n", depth);
			} else {
				printf("info string Performing perft to depth %d\n", depth);
				uint64_t perftResult = performPerft(game, depth);
				printf("info string Perft result: %lu\n", perftResult);
			}
		} else if (strncmp(buffer, "help", 4) == 0) {
			printf(" uci\n");
			printf(" isready\n");
			printf(" ucinewgame\n");
			printf(" setoption name Depth value <n>\n");
			printf(" position startpos\n");
			printf(" position startpos [moves ...]\n");
			printf(" position <fen>\n");
			printf(" position <fen> [moves ...]\n");
			printf(" go\n");
			printf(" quit\n");
			printf(" print\n");
			printf(" movelist\n");
			printf(" perftEPD <filename>\n");
			printf(" perft <n>\n");
		} else {
			printf("info string Received unknown command: %s\n", buffer);
		}
	}
	// Close the log file when you're done
	fclose(logFile);
}
