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
	public:

		static CONSTEXPR uint16_t S16_HAND   = 0x8000;
		static CONSTEXPR uint16_t S16_EMPTY  = 0xffff;
		static CONSTEXPR uint32_t S16_HAND_SHIFT = 7;

		static CONSTEXPR uint32_t FROM       = 0x0000007f;
		static CONSTEXPR uint32_t TO         = 0x00003f80;
		static CONSTEXPR uint32_t PROMOTE    = 0x00004000;
		static CONSTEXPR uint32_t PIECE      = 0x00078000;
		static CONSTEXPR uint32_t CAP        = 0x00f80000;
		static CONSTEXPR uint32_t UNUSED     = 0xff000000;
		static CONSTEXPR uint32_t EMPTY      = 0xffffffff;

		static CONSTEXPR uint32_t TO_SHIFT = 7;
		static CONSTEXPR uint32_t PIECE_SHIFT = 15;
		static CONSTEXPR uint32_t CAP_SHIFT = 19;

	private:

		uint32_t _move;

	public:

		static Move empty() {
			Move empty;
			empty.setEmpty();
			return empty;
		}

		Move() {
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
		static uint32_t serialize(const Move& obj) {
			return obj._move & (FROM | TO | PROMOTE | PIECE);
		}
		static Move deserialize(uint32_t value) {
			Move move;
			move._move = value;
			return move;
		}
		static uint16_t serialize16(const Move& obj);
		static Move deserialize16(uint16_t value, const Board& board);

		static void swap(Move& move1, Move& move2) {
			Move temp = move1;
			move1 = move2;
			move2 = temp;
		}

	private:
		template<bool safe>
		void _set(const Piece& piece, const Position& from, const Position& to, bool promote) {
			assert(!piece.isEmpty());
			assert(piece.isUnpromoted() || !promote);
			_move = ((uint32_t)from + 1)
			      | ((uint32_t)to << TO_SHIFT)
						| ((uint32_t)(safe ? piece.kindOnly() : piece) << PIECE_SHIFT);
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
			_move = ((uint32_t)to << TO_SHIFT)
						| ((uint32_t)hand << PIECE_SHIFT);
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
			_move = (_move & (~PIECE)) | ((uint32_t)piece.kindOnly() << PIECE_SHIFT);
		}
		void setPieceUnsafe(const Piece& piece) {
			_move = (_move & (~PIECE)) | ((uint32_t)piece << PIECE_SHIFT);
		}
		void setCaptured(const Piece& captured) {
			_move = (_move & (~CAP)) | ((uint32_t)(captured.kindOnly() + 1U) << CAP_SHIFT);
		}
		void setCapturedUnsafe(const Piece& captured) {
			_move = (_move & (~CAP)) | ((uint32_t)(captured + 1U) << CAP_SHIFT);
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
			uint32_t cap = _move & CAP;
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
			return equals(obj);
		}
		bool operator!=(const Move& obj) const {
			return !equals(obj);
		}

		// cast operator
		operator uint32_t() const {
			return _move;
		}

		std::string toString() const;

		std::string toStringCsa(bool black) const;

	};

}

#endif //__SUNFISH_MOVE__
