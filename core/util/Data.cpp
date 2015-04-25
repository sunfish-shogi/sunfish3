/* Data.cpp
 *
 * Kubo Ryosuke
 */

#include "Data.h"

namespace sunfish {

	MovableFlags MovableTable[] = {
		//   up,  down,  left, right,   l-u,   l-d,   r-u,   r-d
		{  true, false, false, false, false, false, false, false }, // black pawn
		{  true, false, false, false, false, false, false, false }, // black lance
		{ false, false, false, false, false, false, false, false }, // black knight
		{  true, false, false, false,  true,  true,  true,  true }, // black silver
		{  true,  true,  true,  true,  true, false,  true, false }, // black gold
		{ false, false, false, false,  true,  true,  true,  true }, // black bishop
		{  true,  true,  true,  true, false, false, false, false }, // black rook
		{  true,  true,  true,  true,  true,  true,  true,  true }, // black king
		{  true,  true,  true,  true,  true, false,  true, false }, // black tokin
		{  true,  true,  true,  true,  true, false,  true, false }, // black pro-lance
		{  true,  true,  true,  true,  true, false,  true, false }, // black pro-knight
		{  true,  true,  true,  true,  true, false,  true, false }, // black pro-silver
		{ false, false, false, false, false, false, false, false }, // n/a
		{  true,  true,  true,  true,  true,  true,  true,  true }, // black horse
		{  true,  true,  true,  true,  true,  true,  true,  true }, // black dragon
		{ false, false, false, false, false, false, false, false }, // n/a
		{ false,  true, false, false, false, false, false, false }, // white pawn
		{ false,  true, false, false, false, false, false, false }, // white lance
		{ false, false, false, false, false, false, false, false }, // white knight
		{ false,  true, false, false,  true,  true,  true,  true }, // white silver
		{  true,  true,  true,  true, false,  true, false,  true }, // white gold
		{ false, false, false, false,  true,  true,  true,  true }, // white bishop
		{  true,  true,  true,  true, false, false, false, false }, // white rook
		{  true,  true,  true,  true,  true,  true,  true,  true }, // white king
		{  true,  true,  true,  true, false,  true, false,  true }, // white tokin
		{  true,  true,  true,  true, false,  true, false,  true }, // white pro-lance
		{  true,  true,  true,  true, false,  true, false,  true }, // white pro-knight
		{  true,  true,  true,  true, false,  true, false,  true }, // white pro-silver
		{ false, false, false, false, false, false, false, false }, // n/a
		{  true,  true,  true,  true,  true,  true,  true,  true }, // white horse
		{  true,  true,  true,  true,  true,  true,  true,  true }, // white dragon
		{ false, false, false, false, false, false, false, false }, // n/a
	};

	MovableFlags LongMovableTable[] = {
		//   up,  down,  left, right,   l-u,   l-d,   r-u,   r-d
		{ false, false, false, false, false, false, false, false }, // black pawn
		{  true, false, false, false, false, false, false, false }, // black lance
		{ false, false, false, false, false, false, false, false }, // black knight
		{ false, false, false, false, false, false, false, false }, // black silver
		{ false, false, false, false, false, false, false, false }, // black gold
		{ false, false, false, false,  true,  true,  true,  true }, // black bishop
		{  true,  true,  true,  true, false, false, false, false }, // black rook
		{ false, false, false, false, false, false, false, false }, // black king
		{ false, false, false, false, false, false, false, false }, // black tokin
		{ false, false, false, false, false, false, false, false }, // black pro-lance
		{ false, false, false, false, false, false, false, false }, // black pro-knight
		{ false, false, false, false, false, false, false, false }, // black pro-silver
		{ false, false, false, false, false, false, false, false }, // n/a
		{ false, false, false, false,  true,  true,  true,  true }, // black horse
		{  true,  true,  true,  true, false, false, false, false }, // black dragon
		{ false, false, false, false, false, false, false, false }, // n/a
		{ false, false, false, false, false, false, false, false }, // white pawn
		{ false,  true, false, false, false, false, false, false }, // white lance
		{ false, false, false, false, false, false, false, false }, // white knight
		{ false, false, false, false, false, false, false, false }, // white silver
		{ false, false, false, false, false, false, false, false }, // white gold
		{ false, false, false, false,  true,  true,  true,  true }, // white bishop
		{  true,  true,  true,  true, false, false, false, false }, // white rook
		{ false, false, false, false, false, false, false, false }, // white king
		{ false, false, false, false, false, false, false, false }, // white tokin
		{ false, false, false, false, false, false, false, false }, // white pro-lance
		{ false, false, false, false, false, false, false, false }, // white pro-knight
		{ false, false, false, false, false, false, false, false }, // white pro-silver
		{ false, false, false, false, false, false, false, false }, // n/a
		{ false, false, false, false,  true,  true,  true,  true }, // white horse
		{  true,  true,  true,  true, false, false, false, false }, // white dragon
		{ false, false, false, false, false, false, false, false }, // n/a
	};

	template <int PieceType>
	AtacckableTable<PieceType>::AtacckableTable() {

		POSITION_EACH(king) {
			Bitboard& bb = table[(uint8_t)king];
			bb.init();

			if (PieceType == Piece::BPawn) {
				bb.set(king.safetyDown(2));
				if (king.isPromotable<true>()) {
					bb.set(king.safetyDown().safetyLeft());
					bb.set(king.safetyDown().safetyRight());
				}
				if (king.safetyDown() != Position::Invalid && king.safetyDown().isPromotable<true>()) {
					bb.set(king.safetyDown(2).safetyLeft());
					bb.set(king.safetyDown(2).safetyRight());
				}
			}

			if (PieceType == Piece::WPawn) {
				bb.set(king.safetyUp(2));
				if (king.isPromotable<false>()) {
					bb.set(king.safetyUp().safetyLeft());
					bb.set(king.safetyUp().safetyRight());
				}
				if (king.safetyUp() != Position::Invalid && king.safetyUp().isPromotable<false>()) {
					bb.set(king.safetyUp(2).safetyLeft());
					bb.set(king.safetyUp(2).safetyRight());
				}
			}

			if (PieceType == Piece::BLance) {
				for (Position from = king.safetyDown(); from != Position::Invalid; from = from.safetyDown()) {
					bb.set(from);
					if (king.isPromotable<true>()) {
						bb.set(from.safetyLeft());
						bb.set(from.safetyRight());
					}
				}
			}

			if (PieceType == Piece::WLance) {
				for (Position from = king.safetyUp(); from != Position::Invalid; from = from.safetyUp()) {
					bb.set(from);
					if (king.isPromotable<false>()) {
						bb.set(from.safetyLeft());
						bb.set(from.safetyRight());
					}
				}
			}

			if (PieceType == Piece::BKnight) {
				bb.set(king.safetyDown(4).safetyLeft(2));
				bb.set(king.safetyDown(4));
				bb.set(king.safetyDown(4).safetyRight(2));
				if (king.safetyUp() != Position::Invalid && king.safetyUp().isPromotable<true>()) {
					bb.set(king.safetyDown().safetyLeft());
					bb.set(king.safetyDown().safetyRight());
				}
				if (king.isPromotable<true>()) {
					bb.set(king.safetyDown(2).safetyLeft(2));
					bb.set(king.safetyDown(2));
					bb.set(king.safetyDown(2).safetyRight(2));
				}
				if (king.safetyDown() != Position::Invalid && king.safetyDown().isPromotable<true>()) {
					bb.set(king.safetyDown(3).safetyLeft(2));
					bb.set(king.safetyDown(3).safetyLeft());
					bb.set(king.safetyDown(3));
					bb.set(king.safetyDown(3).safetyRight());
					bb.set(king.safetyDown(3).safetyRight(2));
				}
			}

			if (PieceType == Piece::WKnight) {
				bb.set(king.safetyUp(4).safetyLeft(2));
				bb.set(king.safetyUp(4));
				bb.set(king.safetyUp(4).safetyRight(2));
				if (king.safetyDown() != Position::Invalid && king.safetyDown().isPromotable<false>()) {
					bb.set(king.safetyUp().safetyLeft());
					bb.set(king.safetyUp().safetyRight());
				}
				if (king.isPromotable<false>()) {
					bb.set(king.safetyUp(2).safetyLeft(2));
					bb.set(king.safetyUp(2));
					bb.set(king.safetyUp(2).safetyRight(2));
				}
				if (king.safetyUp() != Position::Invalid && king.safetyUp().isPromotable<false>()) {
					bb.set(king.safetyUp(3).safetyLeft(2));
					bb.set(king.safetyUp(3).safetyLeft());
					bb.set(king.safetyUp(3));
					bb.set(king.safetyUp(3).safetyRight());
					bb.set(king.safetyUp(3).safetyRight(2));
				}
			}

			if (PieceType == Piece::BSilver) {
				bb.set(king.safetyDown(2).safetyLeft(2));
				bb.set(king.safetyDown(2).safetyLeft());
				bb.set(king.safetyDown(2));
				bb.set(king.safetyDown(2).safetyRight());
				bb.set(king.safetyDown(2).safetyRight(2));
				bb.set(king.safetyLeft(2));
				bb.set(king.safetyLeft());
				bb.set(king.safetyRight());
				bb.set(king.safetyRight(2));
				bb.set(king.safetyUp(2).safetyLeft(2));
				bb.set(king.safetyUp(2));
				bb.set(king.safetyUp(2).safetyRight(2));
				if (king.safetyUp() != Position::Invalid && king.safetyUp().isPromotable<true>()) {
					bb.set(king.safetyUp().safetyLeft(2));
					bb.set(king.safetyUp());
					bb.set(king.safetyUp().safetyRight(2));
					bb.set(king.safetyUp(2).safetyLeft());
					bb.set(king.safetyUp(2).safetyRight());
				}
				if (king.isPromotable<true>()) {
					bb.set(king.safetyDown().safetyLeft(2));
					bb.set(king.safetyDown().safetyRight(2));
				}
			}

			if (PieceType == Piece::WSilver) {
				bb.set(king.safetyUp(2).safetyLeft(2));
				bb.set(king.safetyUp(2).safetyLeft());
				bb.set(king.safetyUp(2));
				bb.set(king.safetyUp(2).safetyRight());
				bb.set(king.safetyUp(2).safetyRight(2));
				bb.set(king.safetyLeft(2));
				bb.set(king.safetyLeft());
				bb.set(king.safetyRight());
				bb.set(king.safetyRight(2));
				bb.set(king.safetyDown(2).safetyLeft(2));
				bb.set(king.safetyDown(2));
				bb.set(king.safetyDown(2).safetyRight(2));
				if (king.safetyDown() != Position::Invalid && king.safetyDown().isPromotable<false>()) {
					bb.set(king.safetyDown().safetyLeft(2));
					bb.set(king.safetyDown());
					bb.set(king.safetyDown().safetyRight(2));
					bb.set(king.safetyDown(2).safetyLeft());
					bb.set(king.safetyDown(2).safetyRight());
				}
				if (king.isPromotable<false>()) {
					bb.set(king.safetyUp().safetyLeft(2));
					bb.set(king.safetyUp().safetyRight(2));
				}
			}

			if (PieceType == Piece::BGold) {
				bb.set(king.safetyDown(2).safetyLeft(2));
				bb.set(king.safetyDown(2).safetyLeft());
				bb.set(king.safetyDown(2));
				bb.set(king.safetyDown(2).safetyRight());
				bb.set(king.safetyDown(2).safetyRight(2));
				bb.set(king.safetyDown().safetyLeft(2));
				bb.set(king.safetyDown().safetyRight(2));
				bb.set(king.safetyLeft(2));
				bb.set(king.safetyRight(2));
				bb.set(king.safetyUp().safetyLeft());
				bb.set(king.safetyUp().safetyRight());
				bb.set(king.safetyUp(2));
			}

			if (PieceType == Piece::WGold) {
				bb.set(king.safetyUp(2).safetyLeft(2));
				bb.set(king.safetyUp(2).safetyLeft());
				bb.set(king.safetyUp(2));
				bb.set(king.safetyUp(2).safetyRight());
				bb.set(king.safetyUp(2).safetyRight(2));
				bb.set(king.safetyUp().safetyLeft(2));
				bb.set(king.safetyUp().safetyRight(2));
				bb.set(king.safetyLeft(2));
				bb.set(king.safetyRight(2));
				bb.set(king.safetyDown().safetyLeft());
				bb.set(king.safetyDown().safetyRight());
				bb.set(king.safetyDown(2));
			}

#define POS_BISHOP_EACH(king, dir1, dir2, dir3) \
for (Position p1 = (king).safety ## dir1(); p1 != Position::Invalid; p1 = (p1).safety ## dir1()) { \
for (Position p = p1.safety ## dir2(); p != Position::Invalid; p = (p).safety ## dir2()) { \
	bb.set(p); \
} \
for (Position p = p1.safety ## dir3(); p != Position::Invalid; p = (p).safety ## dir3()) { \
	bb.set(p); \
} \
}
#define SET_BISHOP(king) do { \
POS_BISHOP_EACH((king), LeftUp, LeftDown, RightUp); \
POS_BISHOP_EACH((king), RightDown, LeftDown, RightUp); \
POS_BISHOP_EACH((king), LeftDown, LeftUp, RightDown); \
POS_BISHOP_EACH((king), RightUp, LeftUp, RightDown); \
} while(false)

			if (PieceType == Piece::BBishop) {
				SET_BISHOP(king);
				if (king.safetyUp() != Position::Invalid && king.safetyUp().isPromotable<true>()) {
					SET_BISHOP(king.safetyUp());
				}
				if (king.isPromotable<true>()) {
					SET_BISHOP(king.safetyLeft());
					SET_BISHOP(king.safetyRight());
				}
				if (king.safetyDown() != Position::Invalid && king.safetyUp().isPromotable<true>()) {
					SET_BISHOP(king.safetyDown());
				}
			}

			if (PieceType == Piece::WBishop) {
				SET_BISHOP(king);
				if (king.safetyDown() != Position::Invalid && king.safetyUp().isPromotable<false>()) {
					SET_BISHOP(king.safetyDown());
				}
				if (king.isPromotable<false>()) {
					SET_BISHOP(king.safetyLeft());
					SET_BISHOP(king.safetyRight());
				}
				if (king.safetyUp() != Position::Invalid && king.safetyUp().isPromotable<false>()) {
					SET_BISHOP(king.safetyUp());
				}
			}

			if (PieceType == Piece::Horse) {
				SET_BISHOP(king);
				SET_BISHOP(king.safetyUp());
				SET_BISHOP(king.safetyLeft());
				SET_BISHOP(king.safetyRight());
				SET_BISHOP(king.safetyDown());
			}

#undef POS_BISHOP_EACH
#undef SET_BISHOP

		}

	}

	AtacckableTable<Piece::BPawn> AttackableTables::BPawn;
	AtacckableTable<Piece::BLance> AttackableTables::BLance;
	AtacckableTable<Piece::BKnight> AttackableTables::BKnight;
	AtacckableTable<Piece::BSilver> AttackableTables::BSilver;
	AtacckableTable<Piece::BGold> AttackableTables::BGold;
	AtacckableTable<Piece::BBishop> AttackableTables::BBishop;
	AtacckableTable<Piece::WPawn> AttackableTables::WPawn;
	AtacckableTable<Piece::WLance> AttackableTables::WLance;
	AtacckableTable<Piece::WKnight> AttackableTables::WKnight;
	AtacckableTable<Piece::WSilver> AttackableTables::WSilver;
	AtacckableTable<Piece::WGold> AttackableTables::WGold;
	AtacckableTable<Piece::WBishop> AttackableTables::WBishop;
	AtacckableTable<Piece::Horse> AttackableTables::Horse;
}
