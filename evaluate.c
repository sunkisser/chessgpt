#include "evaluate.h"
#include "board.h"

#define PAWN_VALUE 100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 333
#define ROOK_VALUE 510
#define QUEEN_VALUE 950
#define KING_VALUE 10000
int evaluate_material(Board *board) {

	int score = 0;

	score += __builtin_popcountll(board->pawns[WHITE]) * PAWN_VALUE;
	score -= __builtin_popcountll(board->pawns[BLACK]) * PAWN_VALUE;
	score += __builtin_popcountll(board->knights[WHITE]) * KNIGHT_VALUE;
	score -= __builtin_popcountll(board->knights[BLACK]) * KNIGHT_VALUE;
	score += __builtin_popcountll(board->bishops[WHITE]) * BISHOP_VALUE;
	score -= __builtin_popcountll(board->bishops[BLACK]) * BISHOP_VALUE;
	score += __builtin_popcountll(board->rooks[WHITE]) * ROOK_VALUE;
	score -= __builtin_popcountll(board->rooks[BLACK]) * ROOK_VALUE;
	score += __builtin_popcountll(board->queens[WHITE]) * QUEEN_VALUE;
	score -= __builtin_popcountll(board->queens[BLACK]) * QUEEN_VALUE;
	score += __builtin_popcountll(board->kings[WHITE]) * KING_VALUE;
	score -= __builtin_popcountll(board->kings[BLACK]) * KING_VALUE;

	return score;
}

#define DEVELOPED_KNIGHT_BONUS 10
#define CENTRAL_KNIGHT_BONUS 10
#define CORNER_KNIGHT_PENALTY 20
#define RIM_KNIGHT_PENALTY 10						 // Penalty for knights on the rim
#define CENTRAL_SQUARES 0x0000001818000000ULL		 // This bitboard represents the squares e4-d4-e5-d5
#define CORNER_SQUARES 0x8100000000000081ULL		 // This bitboard represents the corner squares a1, h1, a8, h8
#define RIM_SQUARES 0x7E8181818181817EULL			 // Bitboard with the rim squares set
#define WHITE_DEVELOPED_SQUARES 0x0000000000240000LL // bitboard with c3, f3 set
#define BLACK_DEVELOPED_SQUARES 0x0000240000000000LL // bitboard with c6, f6 set
int evaluate_knight_position(Board *board) {

	int score = 0;

	// Apply bonus for centralized knights and penalty for corner knights
	uint64_t whiteKnights = board->knights[WHITE];
	uint64_t blackKnights = board->knights[BLACK];

	score += CENTRAL_KNIGHT_BONUS * __builtin_popcountll(whiteKnights & CENTRAL_SQUARES);
	score -= CENTRAL_KNIGHT_BONUS * __builtin_popcountll(blackKnights & CENTRAL_SQUARES);
	// Apply bonus for developed knights
	score += DEVELOPED_KNIGHT_BONUS * __builtin_popcountll(whiteKnights & WHITE_DEVELOPED_SQUARES);
	score -= DEVELOPED_KNIGHT_BONUS * __builtin_popcountll(blackKnights & BLACK_DEVELOPED_SQUARES);

	score -= CORNER_KNIGHT_PENALTY * __builtin_popcountll(whiteKnights & CORNER_SQUARES);
	score += CORNER_KNIGHT_PENALTY * __builtin_popcountll(blackKnights & CORNER_SQUARES);
	// Apply penalty for rim knights
	score -= RIM_KNIGHT_PENALTY * __builtin_popcountll(whiteKnights & RIM_SQUARES);
	score += RIM_KNIGHT_PENALTY * __builtin_popcountll(blackKnights & RIM_SQUARES);

	return score;
}


#define PAWN_ADVANCED_BONUS 20
#define WHITE_PAWN_ADVANCED_ROWS 0x00FFFF0000000000ULL // Sixth and seventh rows for white
#define BLACK_PAWN_ADVANCED_ROWS 0x0000000000FFFF00ULL // Second and third rows for black
int evaluate_pawn_position(Board *board) {

	int score = 0;


	// Apply bonus for pawns on advanced rows
	score += PAWN_ADVANCED_BONUS * __builtin_popcountll(board->pawns[WHITE] & WHITE_PAWN_ADVANCED_ROWS);
	score -= PAWN_ADVANCED_BONUS * __builtin_popcountll(board->pawns[BLACK] & BLACK_PAWN_ADVANCED_ROWS);

	return score;
}
#define BISHOP_OPENING_SQUARES_PENALTY 20
#define WHITE_BISHOP_OPENING_SQUARES 0x0000000000000042ULL // c1, f1 squares
#define BLACK_BISHOP_OPENING_SQUARES 0x4200000000000000ULL // c8, f8 squares
int evaluate_bishop_position(Board *board) {
    int score = 0;

    uint64_t whiteBishops = board->bishops[WHITE];
    uint64_t blackBishops = board->bishops[BLACK];

    // Apply penalty for bishops on their opening squares
    score -= BISHOP_OPENING_SQUARES_PENALTY * __builtin_popcountll(whiteBishops & WHITE_BISHOP_OPENING_SQUARES);
    score += BISHOP_OPENING_SQUARES_PENALTY * __builtin_popcountll(blackBishops & BLACK_BISHOP_OPENING_SQUARES);

    return score;
}

int evaluate(Board *board) {
	int score = 0;

	// Evaluate material balance
	score += evaluate_material(board);

	// Evaluate positional bonuses for knights
	score += evaluate_knight_position(board);

    // Evaluate positional penalties for bishops
    // score += evaluate_bishop_position(board);

	score += evaluate_pawn_position(board);

	// If it's black's turn, negate the score
	if (board->sideToMove == BLACK) {
		score = -score;
	}

	return score;
}

#define BISHOP_KNIGHT_START_PENALTY 10
#define WHITE_BISHOP_START_SQUARES 0x0000000000000024ULL // c1, f1 squares
#define BLACK_BISHOP_START_SQUARES 0x2400000000000000ULL // c8, f8 squares
#define WHITE_KNIGHT_START_SQUARES 0x0000000000000042ULL // b1, g1 squares
#define BLACK_KNIGHT_START_SQUARES 0x4200000000000000ULL // b8, g8 squares
#define CENTRAL_PAWN_PENALTY 200
#define WHITE_CENTRAL_PAWNS_START 0x0000000000001800ULL // d2, e2 squares
#define BLACK_CENTRAL_PAWNS_START 0x0018000000000000ULL // d7, e7 squares

int position_penalty(const Board *board, int side) {
    int adjustment = 0;
    if (side == BLACK) {
        adjustment = BISHOP_KNIGHT_START_PENALTY * (
            __builtin_popcountll(board->bishops[WHITE] & WHITE_BISHOP_START_SQUARES) +
            __builtin_popcountll(board->knights[WHITE] & WHITE_KNIGHT_START_SQUARES)
        );
        if ((board->pawns[WHITE] & WHITE_CENTRAL_PAWNS_START) == WHITE_CENTRAL_PAWNS_START) {
			adjustment += CENTRAL_PAWN_PENALTY;
		}
    } else {
        adjustment = BISHOP_KNIGHT_START_PENALTY * (
            __builtin_popcountll(board->bishops[BLACK] & BLACK_BISHOP_START_SQUARES) +
            __builtin_popcountll(board->knights[BLACK] & BLACK_KNIGHT_START_SQUARES)
        );

		if ((board->pawns[BLACK] & BLACK_CENTRAL_PAWNS_START) == BLACK_CENTRAL_PAWNS_START) {
			adjustment += CENTRAL_PAWN_PENALTY;
		}
    }

    return adjustment;
}



