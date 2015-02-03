/* Move.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MOVE__
#define __SUNFISH_MOVE__

#include "../def.h"
#include "../base/Piece.h"
#include "../base/Position.h"
#include <cassert>
#include <climits>
#include <algorithm>

namespace sunfish {

	class Board;

	class Move {
	private:

		static const unsigned FROM       = 0x0000007f;
		static const unsigned TO         = 0x00003f80;
		static const unsigned PROMOTE    = 0x00004000;
		static const unsigned PIECE      = 0x00078000;
		static const unsigned CAP        = 0x00f80000;
		static const unsigned UNUSED     = 0xff000000;
		static const unsigned EMPTY      = 0xffffffff;

		static const unsigned TO_SHIFT = 7;
		static const unsigned PIECE_SHIFT = 15;
		static const unsigned CAP_SHIFT = 19;

		unsigned _move;

	public:

		static Move empty() {
			Move empty;
			empty.setEmpty();
			return empty;
		}

		Move() {
		}
		Move(const Move& src) : _move(src._move) {
		}
		Move(const Piece& piece, const Position& from, const Position& to, bool promote, bool safe = true) {
			if (safe) {
				set(piece, from, to, promote);
			} else {
				setUnsafe(piece, from, to, promote);
			}
		}
		Move(const Piece& piece, const Position& to, bool safe = true) {
			if (safe) {
				set(piece, to);
			} else {
				setUnsafe(piece, to);
			}
		}

		// serialization
		static unsigned serialize(const Move& obj) {
			return obj._move;
		}
		static Move deserialize(unsigned value) {
			Move move;
			move._move = value;
			return move;
		}

		// move from board
	private:
		template<bool safe>
		void _set(const Piece& piece, const Position& from, const Position& to, bool promote) {
			assert(!piece.isEmpty());
			assert(piece.isUnpromoted() || !promote);
			_move = ((unsigned)from + 1)
			      | ((unsigned)to << TO_SHIFT)
						| ((unsigned)(safe ? piece.kindOnly() : piece) << PIECE_SHIFT);
			if (promote) {
				_move |= PROMOTE;
			}
		}
	public:
		void set(const Piece& piece, const Position& from, const Position& to, bool promote) {
			_set<true>(piece, from, to, promote);
		}
		void setUnsafe(const Piece& piece, const Position& from, const Position& to, bool promote) {
			_set<false>(piece, from, to, promote);
		}

		// move from hand
	private:
		template<bool safe>
		void _set(const Piece& piece, const Position& to) {
			assert(!piece.isEmpty());
			assert(!piece.isWhite());
			assert(piece.isUnpromoted());
			const Piece& hand = (safe ? piece.hand() : piece);
			_move = ((unsigned)to << TO_SHIFT)
						| ((unsigned)hand << PIECE_SHIFT);
		}
	public:
		void set(const Piece& piece, const Position& to) {
			_set<true>(piece, to);
		}
		void setUnsafe(const Piece& piece, const Position& to) {
			_set<false>(piece, to);
		}

		void setEmpty() {
			_move = EMPTY;
		}
		bool isEmpty() const {
			return _move == EMPTY;
		}

		// setters
		void setFrom(const Position& from) {
			_move = (_move & ~FROM) | (from + 1);
		}
		void setTo(const Position& to) {
			_move = (_move & ~TO) | (to << TO_SHIFT);
		}
		void setPromote(bool enable = true) {
			if (enable) {
				_move |= PROMOTE;
			} else {
				_move &= ~PROMOTE;
			}
		}
		void setPiece(const Piece& piece) {
			_move = (_move & (~PIECE)) | ((unsigned)piece.kindOnly() << PIECE_SHIFT);
		}
		void setPieceUnsafe(const Piece& piece) {
			_move = (_move & (~PIECE)) | ((unsigned)piece << PIECE_SHIFT);
		}
		void setCaptured(const Piece& captured) {
			_move = (_move & (~CAP)) | ((unsigned)(captured.kindOnly() + 1U) << CAP_SHIFT);
		}
		void setCapturedUnsafe(const Piece& captured) {
			_move = (_move & (~CAP)) | ((unsigned)(captured + 1U) << CAP_SHIFT);
		}
		void unsetCaptured() {
			_move = _move & (~CAP);
		}

		// getters
		Position from() const {
			return (_move & FROM) - 1;
		}
		Position to() const {
			return (_move & TO) >> TO_SHIFT;
		}
		bool promote() const {
			return _move & PROMOTE;
		}
		Piece piece() const {
			return (_move & PIECE) >> PIECE_SHIFT;
		}
		Piece captured() const {
			unsigned cap = _move & CAP;
			return cap ? ((cap >> CAP_SHIFT) - 1L) : Piece::Empty;
		}
		bool isCapturing() const {
			return _move & CAP;
		}
		bool isHand() const {
			return !(_move & FROM);
		}

		// comparator
		bool equals(const Move& obj) const {
			return (_move & ~(UNUSED | CAP)) == (obj._move & ~(UNUSED | CAP));
		}
		bool operator==(const Move& obj) const {
			return _move == obj._move;
		}
		bool operator!=(const Move& obj) const {
			return _move != obj._move;
		}
		bool operator<(const Move& obj) const {
			return _move < obj._move;
		}
		bool operator<=(const Move& obj) const {
			return _move <= obj._move;
		}
		bool operator>(const Move& obj) const {
			return _move > obj._move;
		}
		bool operator>=(const Move& obj) const {
			return _move >= obj._move;
		}

		// cast operator
		operator unsigned() const {
			return _move;
		}

		std::string toString() const;

		std::string toStringCsa(bool black) const;

	};

}

#endif //__SUNFISH_MOVE__
