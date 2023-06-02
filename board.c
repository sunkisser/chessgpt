#include "board.h"
#include "zobrist.h"

#define INITIAL_POSITION_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void initializeBoard(Board *board) {
	setBoardtoFEN(board, INITIAL_POSITION_FEN);
}

void setBoardtoFEN(Board *board, const char *fen) {
	// Reset all the pieces and occupied squares
	for (int i = 0; i < 2; ++i) {
		board->pawns[i] = 0;
		board->knights[i] = 0;
		board->bishops[i] = 0;
		board->rooks[i] = 0;
		board->queens[i] = 0;
		board->kings[i] = 0;
		board->occupied[i] = 0;
	}

	board->sideToMove = WHITE;
	board->enPassantSquare = -1;
	board->castleRights[WHITE] = 0;
	board->castleRights[BLACK] = 0;

	int rank = 7;
	int file = 0;

	while (*fen != ' ') {
		switch (*fen) {
		case 'p':
			board->pawns[BLACK] |= 1ULL << (rank * 8 + file);
			break;
		case 'n':
			board->knights[BLACK] |= 1ULL << (rank * 8 + file);
			break;
		case 'b':
			board->bishops[BLACK] |= 1ULL << (rank * 8 + file);
			break;
		case 'r':
			board->rooks[BLACK] |= 1ULL << (rank * 8 + file);
			break;
		case 'q':
			board->queens[BLACK] |= 1ULL << (rank * 8 + file);
			break;
		case 'k':
			board->kings[BLACK] |= 1ULL << (rank * 8 + file);
			break;
		case 'P':
			board->pawns[WHITE] |= 1ULL << (rank * 8 + file);
			break;
		case 'N':
			board->knights[WHITE] |= 1ULL << (rank * 8 + file);
			break;
		case 'B':
			board->bishops[WHITE] |= 1ULL << (rank * 8 + file);
			break;
		case 'R':
			board->rooks[WHITE] |= 1ULL << (rank * 8 + file);
			break;
		case 'Q':
			board->queens[WHITE] |= 1ULL << (rank * 8 + file);
			break;
		case 'K':
			board->kings[WHITE] |= 1ULL << (rank * 8 + file);
			break;
		case '/':
			rank -= 1;
			file = -1;
			break; // go to the next rank, reset the file
		default:
			file += *fen - '0' - 1; // handle numbers (empty squares)
		}

		file += 1;
		fen += 1;
	}

	fen += 1; // skip the space

	board->sideToMove = (*fen == 'w') ? WHITE : BLACK;

	fen += 2; // skip the side to move and the space

	// Parse castling availability
	while (*fen != ' ') {
		switch (*fen) {
		case 'K':
			board->castleRights[WHITE] |= 1 << 0;
			break;
		case 'Q':
			board->castleRights[WHITE] |= 1 << 1;
			break;
		case 'k':
			board->castleRights[BLACK] |= 1 << 0;
			break;
		case 'q':
			board->castleRights[BLACK] |= 1 << 1;
			break;
		case '-':
			break;
		default: /* Handle error */
			break;
		}
		fen += 1;
	}

	fen += 1; // skip the space

	// Parse en passant target square
	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = (fen[1] - '1');
		board->enPassantSquare = rank * 8 + file;
		fen += 3; // skip the en passant square
	} else {
		fen += 2; // skip the dash
	}

	// Skip any remaining parts of the FEN string
	// (halfmove clock, fullmove number)

	// Update occupied squares
	board->occupied[WHITE] = board->pawns[WHITE] | board->knights[WHITE] | board->bishops[WHITE] | board->rooks[WHITE] | board->queens[WHITE] | board->kings[WHITE];
	board->occupied[BLACK] = board->pawns[BLACK] | board->knights[BLACK] | board->bishops[BLACK] | board->rooks[BLACK] | board->queens[BLACK] | board->kings[BLACK];

	// In setBoardtoFEN
	board->zobristKey = compute_zobrist_key(board);
}
