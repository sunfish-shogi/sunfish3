/* Zobrist.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_ZOBRIST__
#define __SUNFISH_ZOBRIST__

#include "../base/Position.h"
#include "../base/Piece.h"
#include <cassert>

namespace sunfish {

	class Zobrist {
	private:

		Zobrist();

		static const uint64_t Board[Position::N][Piece::Num];
		static const uint64_t HandBPawn[18];
		static const uint64_t HandBLance[4];
		static const uint64_t HandBKnight[4];
		static const uint64_t HandBSilver[4];
		static const uint64_t HandBGold[4];
		static const uint64_t HandBBishop[2];
		static const uint64_t HandBRook[2];
		static const uint64_t HandWPawn[18];
		static const uint64_t HandWLance[4];
		static const uint64_t HandWKnight[4];
		static const uint64_t HandWSilver[4];
		static const uint64_t HandWGold[4];
		static const uint64_t HandWBishop[2];
		static const uint64_t HandWRook[2];
		static const uint64_t Black;

	public:

		static uint64_t board(const Position& pos, const Piece& piece) {
			return Board[pos][piece];
		}

#define __FUNC_HAND__(piece)										\
		static uint64_t handB ## piece(int num) { \
			return HandB ## piece[num]; \
		} \
		static uint64_t handW ## piece(int num) { \
			return HandW ## piece[num]; \
		}

		__FUNC_HAND__(Pawn)
		__FUNC_HAND__(Lance)
		__FUNC_HAND__(Knight)
		__FUNC_HAND__(Silver)
		__FUNC_HAND__(Gold)
		__FUNC_HAND__(Bishop)
		__FUNC_HAND__(Rook)

#undef __FUNC_HAND__

		static uint64_t handBlack(const Piece piece, int num) {
			switch (piece) {
			case Piece::Pawn: return HandBPawn[num];
			case Piece::Lance: return HandBLance[num];
			case Piece::Knight: return HandBKnight[num];
			case Piece::Silver: return HandBSilver[num];
			case Piece::Gold: return HandBGold[num];
			case Piece::Bishop: return HandBBishop[num];
			case Piece::Rook: return HandBRook[num];
			default: assert(false);
			}
			return 0; // unreachable
		}

		static uint64_t handWhite(const Piece piece, int num) {
			switch (piece) {
			case Piece::Pawn: return HandWPawn[num];
			case Piece::Lance: return HandWLance[num];
			case Piece::Knight: return HandWKnight[num];
			case Piece::Silver: return HandWSilver[num];
			case Piece::Gold: return HandWGold[num];
			case Piece::Bishop: return HandWBishop[num];
			case Piece::Rook: return HandWRook[num];
			default: assert(false);
			}
			return 0; // unreachable
		}

		static uint64_t black() {
			return Black;
		}

	};

}

#endif // __SUNFISH_ZOBRIST__
