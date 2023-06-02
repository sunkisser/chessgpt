#include <stdio.h>
#include <stdlib.h>
#include "move.h"
#include "attacked.h"
#include "board.h"
#include "game.h"
#include "zobrist.h"
/*

Decoding a move:
int from = FROM_SQUARE(move);
int to = TO_SQUARE(move);
int promotedPiece = PROMOTED_PIECE(move);
int capturedPiece = CAPTURED_PIECE(move);
int enPassant = IS_EN_PASSANT(move);
int castling = IS_CASTLING(move);
int doublePawnMove = IS_DOUBLE_PAWN_MOVE(move);
int score = MOVE_SCORE(move);

Encoding a move:
Move m = CREATE_MOVE(fromSquare, toSquare, promotedPiece, capturedPiece,
enPassantFlag, castlingFlag, doublePawnMoveFlag, moveScore);


*/

#define A1 0
#define H1 7
#define A8 56
#define H8 63
#define E1 4
#define F1 5
#define E8 60
#define F8 61
#define D1 3
#define D8 59
#define KINGSIDE_CASTLING 0	 // Assuming the first bit in the castling rights bit field represents kingside castling
#define QUEENSIDE_CASTLING 1 // Assuming the second bit in the castling rights bit field represents  queenside castling

int getPieceType(Board *board, int square) {
	uint64_t mask = 1ULL << square;
	if (board->pawns[board->sideToMove] & mask)
		return PAWN;
	if (board->knights[board->sideToMove] & mask)
		return KNIGHT;
	if (board->bishops[board->sideToMove] & mask)
		return BISHOP;
	if (board->rooks[board->sideToMove] & mask)
		return ROOK;
	if (board->queens[board->sideToMove] & mask)
		return QUEEN;
	if (board->kings[board->sideToMove] & mask)
		return KING;
	return EMPTY;
}

void removePiece(Board *board, int pieceType, int side, uint64_t mask) {
	switch (pieceType) {
	case PAWN:
		board->pawns[side] &= ~mask;
		break;
	case KNIGHT:
		board->knights[side] &= ~mask;
		break;
	case BISHOP:
		board->bishops[side] &= ~mask;
		break;
	case ROOK:
		board->rooks[side] &= ~mask;
		break;
	case QUEEN:
		board->queens[side] &= ~mask;
		break;
	case KING:
		board->kings[side] &= ~mask;
		break;
	}
	board->occupied[side] &= ~mask;
}

void placePiece(Board *board, int pieceType, int side, uint64_t mask) {
	switch (pieceType) {
	case PAWN:
		board->pawns[side] |= mask;
		break;
	case KNIGHT:
		board->knights[side] |= mask;
		break;
	case BISHOP:
		board->bishops[side] |= mask;
		break;
	case ROOK:
		board->rooks[side] |= mask;
		break;
	case QUEEN:
		board->queens[side] |= mask;
		break;
	case KING:
		board->kings[side] |= mask;
		break;
	}
	board->occupied[side] |= mask;
}


extern uint64_t zobrist_piece_keys[6][2][64];
extern uint64_t zobrist_side_key;
extern uint64_t zobrist_castling_keys[2][2];
extern uint64_t zobrist_ep_keys[8]; // 8 possible files for en passant

void make_move(Board *board, Move move) {
	// Get all move properties
	int from = FROM_SQUARE(move);
	int to = TO_SQUARE(move);
	int promotedPiece = PROMOTED_PIECE(move);
	int capturedPiece = CAPTURED_PIECE(move);
	int enPassant = IS_EN_PASSANT(move);
	int castling = IS_CASTLING(move);
	int doublePawnMove = IS_DOUBLE_PAWN_MOVE(move);
	// int score = MOVE_SCORE(move);

	// Assuming only pawns can promote for simplicity
	int pieceType = (promotedPiece != EMPTY) ? PAWN : getPieceType(board, from);
	int side = board->sideToMove;
	uint64_t fromMask = 1ULL << from;
	uint64_t toMask = 1ULL << to;

	// Remove piece from the 'from' square
	removePiece(board, pieceType, side, fromMask);
	board->zobristKey ^= zobrist_piece_keys[pieceType][side][from]; // update Zobrist key

	// If a piece was captured, remove it from the 'to' square
	if (capturedPiece != EMPTY && !enPassant) {
		removePiece(board, capturedPiece, side ^ 1, toMask);
		board->zobristKey ^= zobrist_piece_keys[capturedPiece][side ^ 1][to]; // update Zobrist key

		// If a rook was captured, update the castling rights
		if (capturedPiece == ROOK) {
			if (to == (side == WHITE ? H8 : H1)) {											 // original position of opponent's Kingside Rook
				if (board->castleRights[side ^ 1] & (1 << KINGSIDE_CASTLING)) {				 // Check if opponent's kingside  castling is currently allowed
					board->castleRights[side ^ 1] &= ~(1 << KINGSIDE_CASTLING);				 // Turn off opponent's kingside castling bit
					board->zobristKey ^= zobrist_castling_keys[side ^ 1][KINGSIDE_CASTLING]; // update Zobrist key
				}
			} else if (to == (side == WHITE ? A8 : A1)) {									  // original position of opponent's Queenside Rook
				if (board->castleRights[side ^ 1] & (1 << QUEENSIDE_CASTLING)) {			  // Check if opponent's queenside castling is currently allowed
					board->castleRights[side ^ 1] &= ~(1 << QUEENSIDE_CASTLING);			  // Turn off opponent's queenside castling bit
					board->zobristKey ^= zobrist_castling_keys[side ^ 1][QUEENSIDE_CASTLING]; // update Zobrist key
				}
			}
		}
	}

	// Place piece (or promoted piece if applicable) to the 'to' square
	placePiece(board, promotedPiece != EMPTY ? promotedPiece : pieceType, side, toMask);
	board->zobristKey ^= zobrist_piece_keys[promotedPiece != EMPTY ? promotedPiece : pieceType][side][to]; // update Zobrist key

	// If a rook is moved from its original position, update the castling rights
	if (pieceType == ROOK) {
		if (from == (side == WHITE ? H1 : H8)) {									 // original position of Kingside Rook
			if (board->castleRights[side] & (1 << KINGSIDE_CASTLING)) {				 // Check if kingside castling is currently allowed
				board->castleRights[side] &= ~(1 << KINGSIDE_CASTLING);				 // Turn off kingside castling bit
				board->zobristKey ^= zobrist_castling_keys[side][KINGSIDE_CASTLING]; // update Zobrist key
			}
		} else if (from == (side == WHITE ? A1 : A8)) {								  // original position of Queenside Rook
			if (board->castleRights[side] & (1 << QUEENSIDE_CASTLING)) {			  // Check if queenside castling is currently allowed
				board->castleRights[side] &= ~(1 << QUEENSIDE_CASTLING);			  // Turn off queenside castling bit
				board->zobristKey ^= zobrist_castling_keys[side][QUEENSIDE_CASTLING]; // update Zobrist key
			}
		}
	}

	// If the king is moved, update the castling rights
	if (pieceType == KING) {
		if (from == (side == WHITE ? E1 : E8)) {									 // original position of the King
			if (board->castleRights[side] & (1 << KINGSIDE_CASTLING)) {				 // Check if kingside castling is currently allowed
				board->castleRights[side] &= ~(1 << KINGSIDE_CASTLING);				 // Turn off kingside castling bit
				board->zobristKey ^= zobrist_castling_keys[side][KINGSIDE_CASTLING]; // update Zobrist key
			}
			if (board->castleRights[side] & (1 << QUEENSIDE_CASTLING)) {			  // Check if queenside castling is  currently allowed
				board->castleRights[side] &= ~(1 << QUEENSIDE_CASTLING);			  // Turn off queenside castling bit
				board->zobristKey ^= zobrist_castling_keys[side][QUEENSIDE_CASTLING]; // update Zobrist key
			}
		}
	}

	// If a pawn moved two squares forward, set the en passant square
	if (doublePawnMove) {
		int oldEnPassantFile = board->enPassantSquare >= 0 ? board->enPassantSquare % 8 : -1;
		if (oldEnPassantFile >= 0) {
			board->zobristKey ^= zobrist_ep_keys[oldEnPassantFile]; // update Zobrist key if there was an en passant target square before
		}
		board->enPassantSquare = (side == WHITE ? from + 8 : from - 8);
		board->zobristKey ^= zobrist_ep_keys[board->enPassantSquare % 8]; // update Zobrist key with new en passant target square
	} else {
		int oldEnPassantFile = board->enPassantSquare >= 0 ? board->enPassantSquare % 8 : -1;
		if (oldEnPassantFile >= 0) {
			board->zobristKey ^= zobrist_ep_keys[oldEnPassantFile]; // update Zobrist key if there was an en passant target square before
		}
		board->enPassantSquare = -1;
	}

	// Handle en passant
	if (enPassant) {
		int enPassantCaptureSquare = (side == WHITE ? to - 8 : to + 8);
		uint64_t enPassantMask = 1ULL << enPassantCaptureSquare;
		removePiece(board, PAWN, side ^ 1, enPassantMask);
		// Update Zobrist key for en passant capture
		board->zobristKey ^= zobrist_piece_keys[PAWN][side ^ 1][enPassantCaptureSquare];
	}

	// Handle castling
	if (castling) {
		// Determine whether it's kingside or queenside castling based on the to and from squares
		if (to - from > 1) { // Kingside castling
			// Remove the rook from its original square
			uint64_t kingsideRookMask = 1ULL << (side == WHITE ? H1 : H8);
			removePiece(board, ROOK, side, kingsideRookMask);
			// Place the rook next to the king
			uint64_t newRookMask = 1ULL << (side == WHITE ? F1 : F8);
			placePiece(board, ROOK, side, newRookMask);
		} else { // Queenside castling
			// Remove the rook from its original square
			uint64_t queensideRookMask = 1ULL << (side == WHITE ? A1 : A8);
			removePiece(board, ROOK, side, queensideRookMask);
			// Place the rook next to the king
			uint64_t newRookMask = 1ULL << (side == WHITE ? D1 : D8);
			placePiece(board, ROOK, side, newRookMask);
		}
		// After castling, update castling rights - turn off both castling bits
		if (board->castleRights[side] & (1 << KINGSIDE_CASTLING)) {				 // Check if kingside castling is currently allowed
			board->castleRights[side] &= ~(1 << KINGSIDE_CASTLING);				 // Turn off kingside castling bit  update Zobrist key
			board->zobristKey ^= zobrist_castling_keys[side][KINGSIDE_CASTLING]; //
		}
		if (board->castleRights[side] & (1 << QUEENSIDE_CASTLING)) {			  // Check if queenside castling is currently allowed
			board->castleRights[side] &= ~(1 << QUEENSIDE_CASTLING);			  // Turn off queenside castling bit update Zobrist key
			board->zobristKey ^= zobrist_castling_keys[side][QUEENSIDE_CASTLING]; //
		}
	}

	// Switch the side to move
	board->sideToMove ^= 1;
	board->zobristKey ^= zobrist_side_key; // update Zobrist key
}

int generateMoves(Board *board, Move *moves) {
	const int knightRankOffsets[8] = {-2, -1, 1, 2, 2, 1, -1, -2};
	const int knightFileOffsets[8] = {1, 2, 2, 1, -1, -2, -2, -1};
	int moveCount = 0;
	int side = board->sideToMove;
	int opponent = side ^ 1;

	// Loop over all squares
	for (int square = 0; square < 64; square++) {

		// Pawns
		if ((board->pawns[side] >> square) & 1) {
			int rank = square / 8;
			int file = square % 8;
			int forwardRank = (side == WHITE) ? rank + 1 : rank - 1;
			if (forwardRank >= 0 && forwardRank < 8) {
				// Normal move forward
				int forwardSquare = forwardRank * 8 + file;
				uint64_t forwardSquareMask = 1ULL << forwardSquare;

				// Check if the forward square is not occupied
				if (!(board->occupied[WHITE] & forwardSquareMask) && !(board->occupied[BLACK] & forwardSquareMask)) {
					if (forwardRank == 0 || forwardRank == 7) { // It's a promotion
						// Create a move for each possible promotion piece
						int promotionPieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
						for (int i = 0; i < 4; i++) {
							Move m = CREATE_MOVE(square, forwardSquare, promotionPieces[i], EMPTY, 0, 0, 0, 0);
							moves[moveCount++] = m;
						}
					} else { // Not a promotion
						// Create move
						Move m = CREATE_MOVE(square, forwardSquare, EMPTY, EMPTY, 0, 0, 0, 0);
						moves[moveCount++] = m;
					}
				}

				// Double move forward
				if ((side == WHITE && rank == 1) || (side == BLACK && rank == 6)) {
					int doubleForwardRank = (side == WHITE) ? rank + 2 : rank - 2;
					int doubleForwardSquare = doubleForwardRank * 8 + file;
					uint64_t doubleForwardSquareMask = 1ULL << doubleForwardSquare;

					// Check if the two forward squares are not occupied
					if (!(board->occupied[WHITE] & forwardSquareMask) && !(board->occupied[BLACK] & forwardSquareMask) && !(board->occupied[WHITE] & doubleForwardSquareMask) &&
						!(board->occupied[BLACK] & doubleForwardSquareMask)) {
						// Create move
						Move m = CREATE_MOVE(square, doubleForwardSquare, EMPTY, EMPTY, 0, 0, 1, 0);
						moves[moveCount++] = m;
					}
				}
			}

			// Generate pawn capture moves
			int captureOffsets[2] = {(side == WHITE) ? -1 : 1, (side == WHITE) ? 1 : -1};
			for (int i = 0; i < 2; i++) {
				int targetRank = rank + (side == WHITE ? 1 : -1);
				int targetFile = file + captureOffsets[i];

				if (targetRank >= 0 && targetRank < 8 && targetFile >= 0 && targetFile < 8) {
					int toSquare = targetRank * 8 + targetFile;
					uint64_t toSquareMask = 1ULL << toSquare;

					// Check if the target square is occupied by a piece of the opposite color
					if (board->occupied[opponent] & toSquareMask) {
						int capturedPiece = EMPTY;
						if (board->pawns[opponent] & toSquareMask)
							capturedPiece = PAWN;
						else if (board->knights[opponent] & toSquareMask)
							capturedPiece = KNIGHT;
						else if (board->bishops[opponent] & toSquareMask)
							capturedPiece = BISHOP;
						else if (board->rooks[opponent] & toSquareMask)
							capturedPiece = ROOK;
						else if (board->queens[opponent] & toSquareMask)
							capturedPiece = QUEEN;
						else if (board->kings[opponent] & toSquareMask)
							capturedPiece = KING;
						else
							capturedPiece = EMPTY;
						// Create move
						if (targetRank == 0 || targetRank == 7) { // It's a promotion during capture
							// Create a move for each possible promotion piece
							int promotionPieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
							for (int i = 0; i < 4; i++) {
								Move m = CREATE_MOVE(square, toSquare, promotionPieces[i], capturedPiece, 0, 0, 0, 0);
								moves[moveCount++] = m;
							}
						} else { // Not a promotion
							// Create move
							Move m = CREATE_MOVE(square, toSquare, EMPTY, capturedPiece, 0, 0, 0, 0);
							moves[moveCount++] = m;
						}
					}
				}
			}

			// Generate en passant moves
			if (board->enPassantSquare != -1) {
				int rank = square / 8;
				int file = square % 8;

				int targetRank = board->enPassantSquare / 8;
				int targetFile = board->enPassantSquare % 8;

				// The en passant square is always adjacent to the capturing pawn, so the rank should be the same as the pawn's target rank, and the file should be one off.
				if (targetRank == rank + (side == WHITE ? 1 : -1) && (targetFile == file + 1 || targetFile == file - 1)) {
					uint64_t toSquareMask = 1ULL << board->enPassantSquare;

					// Check if the en passant square is not occupied
					if (!(board->occupied[WHITE] & toSquareMask) && !(board->occupied[BLACK] & toSquareMask)) {
						// Create move
						Move m = CREATE_MOVE(square, board->enPassantSquare, EMPTY, PAWN, 1, 0, 0, 0);

						// Add generated move to the movelist
						moves[moveCount++] = m;
					}
				}
			}
		}

		// Knights
		if ((board->knights[side] >> square) & 1) {
			int rank = square / 8;
			int file = square % 8;

			for (int i = 0; i < 8; i++) {
				int targetRank = rank + knightRankOffsets[i];
				int targetFile = file + knightFileOffsets[i];

				if (targetRank >= 0 && targetRank < 8 && targetFile >= 0 && targetFile < 8) {
					int toSquare = targetRank * 8 + targetFile;
					uint64_t toSquareMask = 1ULL << toSquare;

					// Check if the target square is occupied by a piece of the same color
					if (board->occupied[side] & toSquareMask) {
						continue; // Skip this move
					}

					int capturedPiece = EMPTY;
					// Check if the target square is occupied by a piece of the opposite color
					if (board->occupied[opponent] & toSquareMask) {
						if (board->pawns[opponent] & toSquareMask)
							capturedPiece = PAWN;
						else if (board->knights[opponent] & toSquareMask)
							capturedPiece = KNIGHT;
						else if (board->bishops[opponent] & toSquareMask)
							capturedPiece = BISHOP;
						else if (board->rooks[opponent] & toSquareMask)
							capturedPiece = ROOK;
						else if (board->queens[opponent] & toSquareMask)
							capturedPiece = QUEEN;
						else if (board->kings[opponent] & toSquareMask)
							capturedPiece = KING;
						else
							capturedPiece = EMPTY;
					}

					// Create move
					Move m = CREATE_MOVE(square, toSquare, EMPTY, capturedPiece, 0, 0, 0, 0);

					// Add generated moves to the movelist
					moves[moveCount++] = m;
				}
			}
		}

		// Bishops and diagonally moving Queens
		if (((board->bishops[side] | board->queens[side]) >> square) & 1) {
			const int directionOffsets[4] = {-9, -7, 7, 9}; // Represents the directions in which a bishop can move

			for (int i = 0; i < 4; i++) {
				for (int targetSquare = square + directionOffsets[i]; targetSquare >= 0 && targetSquare < 64; targetSquare += directionOffsets[i]) {
					// If the bishop moved off the board to the left or right, stop checking this direction
					if (abs(targetSquare % 8 - (targetSquare - directionOffsets[i]) % 8) > 2) {
						break;
					}

					uint64_t targetSquareMask = 1ULL << targetSquare;
					int capturedPiece = EMPTY;

					// If the target square is occupied by a piece of the same color, stop checking this direction
					if (board->occupied[side] & targetSquareMask) {
						break;
					}

					// Check if the target square is occupied by a piece of the opposite color
					if (board->occupied[opponent] & targetSquareMask) {
						if (board->pawns[opponent] & targetSquareMask)
							capturedPiece = PAWN;
						else if (board->knights[opponent] & targetSquareMask)
							capturedPiece = KNIGHT;
						else if (board->bishops[opponent] & targetSquareMask)
							capturedPiece = BISHOP;
						else if (board->rooks[opponent] & targetSquareMask)
							capturedPiece = ROOK;
						else if (board->queens[opponent] & targetSquareMask)
							capturedPiece = QUEEN;
						else if (board->kings[opponent] & targetSquareMask)
							capturedPiece = KING;
						else
							capturedPiece = EMPTY;
					}

					// Create move
					Move m = CREATE_MOVE(square, targetSquare, EMPTY, capturedPiece, 0, 0, 0, 0);
					// Add generated move to the movelist
					moves[moveCount++] = m;

					// If the target square is occupied (either by same color or opponent), stop checking this direction
					if (board->occupied[WHITE] & targetSquareMask || board->occupied[BLACK] & targetSquareMask) {
						break;
					}
				}
			}
		}
		// rooks and orthogonally moving Queens
		if (((board->rooks[side] | board->queens[side]) >> square) & 1) {
			const int directionOffsets[4] = {-8, 8, -1, 1}; // Represents the directions in which a rook can move

			for (int i = 0; i < 4; i++) {
				for (int targetSquare = square + directionOffsets[i]; targetSquare >= 0 && targetSquare < 64; targetSquare += directionOffsets[i]) {
					// If the rook moved off the board to the left or right, stop checking this direction
					if (abs(targetSquare % 8 - (targetSquare - directionOffsets[i]) % 8) > 1) {
						break;
					}

					uint64_t targetSquareMask = 1ULL << targetSquare;
					int capturedPiece = EMPTY;

					// If the target square is occupied by a piece of the same color, stop checking this direction
					if (board->occupied[side] & targetSquareMask) {
						break;
					}

					// Check if the target square is occupied by a piece of the opposite color
					if (board->occupied[opponent] & targetSquareMask) {
						if (board->pawns[opponent] & targetSquareMask)
							capturedPiece = PAWN;
						else if (board->knights[opponent] & targetSquareMask)
							capturedPiece = KNIGHT;
						else if (board->bishops[opponent] & targetSquareMask)
							capturedPiece = BISHOP;
						else if (board->rooks[opponent] & targetSquareMask)
							capturedPiece = ROOK;
						else if (board->queens[opponent] & targetSquareMask)
							capturedPiece = QUEEN;
						else if (board->kings[opponent] & targetSquareMask)
							capturedPiece = KING;
						else
							capturedPiece = EMPTY;
					}

					// Create move
					Move m = CREATE_MOVE(square, targetSquare, EMPTY, capturedPiece, 0, 0, 0, 0);
					// Add generated move to the movelist
					moves[moveCount++] = m;

					// If the target square is occupied (either by same color or opponent), stop checking this direction
					if (board->occupied[WHITE] & targetSquareMask || board->occupied[BLACK] & targetSquareMask) {
						break;
					}
				}
			}
		}

		// Kings
		if ((board->kings[side] >> square) & 1) {
			const int kingOffsets[8] = {-9, -8, -7, -1, 1, 7, 8, 9};

			for (int i = 0; i < 8; i++) {
				int targetSquare = square + kingOffsets[i];

				// Check that the king isn't going off the edges of the board
				if (targetSquare < 0 || targetSquare >= 64 || abs(targetSquare % 8 - (targetSquare - kingOffsets[i]) % 8) > 2) {
					continue;
				}

				uint64_t targetSquareMask = 1ULL << targetSquare;
				int capturedPiece = EMPTY;

				// If the target square is occupied by a piece of the same color, skip this square
				if (board->occupied[side] & targetSquareMask) {
					continue;
				}

				// Check if the target square is occupied by a piece of the opposite color
				if (board->occupied[opponent] & targetSquareMask) {
					if (board->pawns[opponent] & targetSquareMask)
						capturedPiece = PAWN;
					else if (board->knights[opponent] & targetSquareMask)
						capturedPiece = KNIGHT;
					else if (board->bishops[opponent] & targetSquareMask)
						capturedPiece = BISHOP;
					else if (board->rooks[opponent] & targetSquareMask)
						capturedPiece = ROOK;
					else if (board->queens[opponent] & targetSquareMask)
						capturedPiece = QUEEN;
					else if (board->kings[opponent] & targetSquareMask)
						capturedPiece = KING;
					else
						capturedPiece = EMPTY;
				}

				// Create move
				Move m = CREATE_MOVE(square, targetSquare, EMPTY, capturedPiece, 0, 0, 0, 0);
				// Add generated move to the movelist
				moves[moveCount++] = m;
			}

			// Consider castling
			// White side
			if (side == WHITE) {
				// Kingside
				if (board->castleRights[WHITE] & 1 << 0) {
					// Check that squares between the king and the rook are not occupied and not attacked
					if (!((board->occupied[WHITE] | board->occupied[BLACK]) & ((1ULL << (0 * 8 + 5)) | (1ULL << (0 * 8 + 6)))) && !is_square_attacked(board, 0 * 8 + 5) &&
						!is_square_attacked(board, 0 * 8 + 6) && !is_square_attacked(board, square)) {
						// If it's safe, generate a castling move
						Move m = CREATE_MOVE(square, 0 * 8 + 6, EMPTY, EMPTY, 0, 1, 0, 0);
						moves[moveCount++] = m;
					}
				}
				// Queenside
				if (board->castleRights[WHITE] & 1 << 1) {
					// Check that squares between the king and the rook are not occupied and not attacked
					if (!((board->occupied[WHITE] | board->occupied[BLACK]) & ((1ULL << (0 * 8 + 3)) | (1ULL << (0 * 8 + 2)) | (1ULL << (0 * 8 + 1)))) && !is_square_attacked(board, 0 * 8 + 3) &&
						!is_square_attacked(board, 0 * 8 + 2) && !is_square_attacked(board, square)) {
						// If it's safe, generate a castling move
						Move m = CREATE_MOVE(square, 0 * 8 + 2, EMPTY, EMPTY, 0, 1, 0, 0);
						moves[moveCount++] = m;
					}
				}
			}
			// Black side
			else if (side == BLACK) {
				// Kingside
				if (board->castleRights[BLACK] & 1 << 0) {
					// Check that squares between the king and the rook are not occupied and not attacked
					if (!((board->occupied[WHITE] | board->occupied[BLACK]) & ((1ULL << (7 * 8 + 5)) | (1ULL << (7 * 8 + 6)))) && !is_square_attacked(board, 7 * 8 + 5) &&
						!is_square_attacked(board, 7 * 8 + 6) && !is_square_attacked(board, square)) {
						// If it's safe, generate a castling move
						Move m = CREATE_MOVE(square, 7 * 8 + 6, EMPTY, EMPTY, 0, 1, 0, 0);
						moves[moveCount++] = m;
					}
				}
				// Queenside
				if (board->castleRights[BLACK] & 1 << 1) {
					// Check that squares between the king and the rook are not occupied and not attacked
					if (!((board->occupied[WHITE] | board->occupied[BLACK]) & ((1ULL << (7 * 8 + 3)) | (1ULL << (7 * 8 + 2)) | (1ULL << (7 * 8 + 1)))) && !is_square_attacked(board, 7 * 8 + 3) &&
						!is_square_attacked(board, 7 * 8 + 2) && !is_square_attacked(board, square)) {
						// If it's safe, generate a castling move
						Move m = CREATE_MOVE(square, 7 * 8 + 2, EMPTY, EMPTY, 0, 1, 0, 0);
						moves[moveCount++] = m;
					}
				}
			}
		}
	}

	// Finally, filter out any moves that would leave the king in check
	Board tempBoard;
	Move tempMoves[moveCount];
	int newMoveCount = 0;
	// printf("Pseudolegal moves: %d\n",moveCount);

	for (int i = 0; i < moveCount; i++) {
		tempBoard = *board;
		make_move(&tempBoard, moves[i]);
		tempBoard.sideToMove ^= 1;
		int tempKingSquare = __builtin_ffsll(tempBoard.kings[board->sideToMove]) - 1;

		if (!is_square_attacked(&tempBoard, tempKingSquare)) {
			tempMoves[newMoveCount] = moves[i];
			newMoveCount++;
		}
	}

	for (int i = 0; i < newMoveCount; i++) {
		moves[i] = tempMoves[i];
	}

	// Append a null move at the end
	moves[moveCount] = 0;

	// Return the count of moves
	return newMoveCount;
}




void printMove(Move move) {
	int from = FROM_SQUARE(move);
	int to = TO_SQUARE(move);
	char fromFile = 'a' + (from % 8);
	char fromRank = '1' + (from / 8);
	char toFile = 'a' + (to % 8);
	char toRank = '1' + (to / 8);

	if (PROMOTED_PIECE(move)) {
		char promPiece;
		switch (PROMOTED_PIECE(move)) {
		case QUEEN:
			promPiece = 'q';
			break;
		case ROOK:
			promPiece = 'r';
			break;
		case BISHOP:
			promPiece = 'b';
			break;
		case KNIGHT:
			promPiece = 'n';
			break;
		default:
			promPiece = '?';
			break;
		}
		printf("%c%c%c%c=%c", fromFile, fromRank, toFile, toRank, promPiece);
	} else {
		printf("%c%c%c%c", fromFile, fromRank, toFile, toRank);
	}

	if (IS_EN_PASSANT(move)) {
		printf(" e.p.");
	}

	if (IS_CASTLING(move)) {
		if (abs(to - from) > 1) {
			printf(" O-O"); // Kingside
		} else {
			printf(" O-O-O"); // Queenside
		}
	}

	printf(" ");
}
