#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "perft.h"
#include "move.h"

#define MAX_DEPTH 4

uint64_t parseDepthResult(const char *line, int depth) {
	char depthStr[5];
	sprintf(depthStr, "D%d ", depth);

	char *start = strstr(line, depthStr);
	if (start == NULL) {
		return 0;
	}

	start += strlen(depthStr);
	return strtoull(start, NULL, 10);
}

void perftFromEPD(const char *filePath) {
	FILE *file = fopen(filePath, "r");
	if (file == NULL) {
		printf("Failed to open EPD file: %s\n", filePath);
		return;
	}

	char line[256];
	Game game;
	int linecount = 0;
	int matchcount = 0;
	while (fgets(line, sizeof(line), file)) {
		linecount++;

		// Add this check for blank lines
		if (line[0] == '\n' || line[0] == '\r') {
			continue;
		}

		char *fenEnd = strchr(line, ';');
		if (fenEnd == NULL) {
			printf("Invalid EPD line: %s\n", line);
			continue;
		}

		*fenEnd = '\0';
		setBoardtoFEN(&game.board, line);
		// printBoard(&game.board);

		char *perftResults = fenEnd + 1; // Start from the next character after the ';'

		for (int depth = 1; depth <= MAX_DEPTH; depth++) {
			uint64_t expectedNodes = parseDepthResult(perftResults, depth);

			uint64_t result = performPerft(&game, depth);
			if (result != expectedNodes) {
				printf("Mismatch in perft result for FEN: %s\n", line);
				printf("Depth: %d, expected: %lu, got: %lu\n", depth, expectedNodes, result);
			} else {
				matchcount++;
				if (matchcount % 100 == 0) 
					printf("Matches so far %d \n",matchcount);
				// printf("Matched %d", linecount);
				// printf("Depth: %d, expected: %lu, got: %lu\n", depth,
				// expectedNodes, result);
			}
		}
	}
	fclose(file);
	printf("Matches: %d\n", matchcount);
	matchcount = 0;
}

void perft_depth(Board *board, int depth, PerftResult *result) {
	if (depth == 0) {
		result->nodes++;
		return;
	}

	Move moves[MAX_MOVES];
	int moveCount = generateMoves(board, moves);

	for (int i = 0; i < moveCount; i++) {
		Board tempBoard = *board;
		make_move(&tempBoard, moves[i]);
		PerftResult child_result = {0, 0, 0, 0, 0};
		perft_depth(&tempBoard, depth - 1, &child_result);

		if (CAPTURED_PIECE(moves[i]) != EMPTY)
			child_result.captures++;
		if (IS_EN_PASSANT(moves[i]))
			child_result.enPassants++;
		if (IS_CASTLING(moves[i]))
			child_result.castlings++;
		if (PROMOTED_PIECE(moves[i]) != EMPTY)
			child_result.promotions++;

		result->nodes += child_result.nodes;
		result->captures += child_result.captures;
		result->enPassants += child_result.enPassants;
		result->promotions += child_result.promotions;
		result->castlings += child_result.castlings;
	}
}

uint64_t performPerft(Game *game, int depth) {
	time_t start_time = time(NULL);
	PerftResult results[MAX_MOVES] = {0};
	Move moves[MAX_MOVES];
	int moveCount = generateMoves(&game->board, moves);

#pragma omp parallel for
	for (int i = 0; i < moveCount; i++) {
		Board tempBoard = game->board;
		make_move(&tempBoard, moves[i]);
		perft_depth(&tempBoard, depth - 1, &results[i]);
	}

	PerftResult total = {0, 0, 0, 0, 0};
	for (int i = 0; i < moveCount; i++) {
		total.nodes += results[i].nodes;
	}

	time_t end_time = time(NULL);
	int elapsed_time = difftime(end_time, start_time);
	//printf("info string elapsed time %d seconds.\n", elapsed_time);
	return total.nodes;
}
