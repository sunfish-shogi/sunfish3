/* Data.cpp
 *
 * Kubo Ryosuke
 */

#include "Data.h"

namespace sunfish {

const uint8_t MovableTable[] = {
  0x80, // black pawn
  0x80, // black lance
  0x00, // black knight
  0x8f, // black silver
  0xfa, // black gold
  0x0f, // black bishop
  0xf0, // black rook
  0xff, // black king
  0xfa, // black tokin
  0xfa, // black pro-lance
  0xfa, // black pro-knight
  0xfa, // black pro-silver
  0x00, // n/a
  0xff, // black horse
  0xff, // black dragon
  0x00, // n/a
  0x40, // white pawn
  0x40, // white lance
  0x00, // white knight
  0x4f, // white silver
  0xf5, // white gold
  0x0f, // white bishop
  0xf0, // white rook
  0xff, // white king
  0xf5, // white tokin
  0xf5, // white pro-lance
  0xf5, // white pro-knight
  0xf5, // white pro-silver
  0x00, // n/a
  0xff, // white horse
  0xff, // white dragon
  0x00, // n/a
};

const uint8_t LongMovableTable[] = {
  0x00, // black pawn
  0x80, // black lance
  0x00, // black knight
  0x00, // black silver
  0x00, // black gold
  0x0f, // black bishop
  0xf0, // black rook
  0x00, // black king
  0x00, // black tokin
  0x00, // black pro-lance
  0x00, // black pro-knight
  0x00, // black pro-silver
  0x00, // n/a
  0x0f, // black horse
  0xf0, // black dragon
  0x00, // n/a
  0x00, // white pawn
  0x40, // white lance
  0x00, // white knight
  0x00, // white silver
  0x00, // white gold
  0x0f, // white bishop
  0xf0, // white rook
  0x00, // white king
  0x00, // white tokin
  0x00, // white pro-lance
  0x00, // white pro-knight
  0x00, // white pro-silver
  0x00, // n/a
  0x0f, // white horse
  0xf0, // white dragon
  0x00, // n/a
};

template <int PieceType>
AtacckableTable<PieceType>::AtacckableTable() {

  SQUARE_EACH(king) {
    Bitboard& bb = table[king.index()];
    bb.init();

    if (PieceType == Piece::BPawn) {
      bb.set(king.safetyDown(2));
      if (king.isPromotable<true>()) {
        bb.set(king.safetyDown().safetyLeft());
        bb.set(king.safetyDown().safetyRight());
      }
      if (king.safetyDown() != Square::Invalid && king.safetyDown().isPromotable<true>()) {
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
      if (king.safetyUp() != Square::Invalid && king.safetyUp().isPromotable<false>()) {
        bb.set(king.safetyUp(2).safetyLeft());
        bb.set(king.safetyUp(2).safetyRight());
      }
    }

    if (PieceType == Piece::BLance) {
      for (Square from = king.safetyDown(); from != Square::Invalid; from = from.safetyDown()) {
        bb.set(from);
        if (king.isPromotable<true>()) {
          bb.set(from.safetyLeft());
          bb.set(from.safetyRight());
        }
      }
    }

    if (PieceType == Piece::WLance) {
      for (Square from = king.safetyUp(); from != Square::Invalid; from = from.safetyUp()) {
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
      if (king.safetyUp() != Square::Invalid && king.safetyUp().isPromotable<true>()) {
        bb.set(king.safetyDown().safetyLeft());
        bb.set(king.safetyDown().safetyRight());
      }
      if (king.isPromotable<true>()) {
        bb.set(king.safetyDown(2).safetyLeft(2));
        bb.set(king.safetyDown(2));
        bb.set(king.safetyDown(2).safetyRight(2));
      }
      if (king.safetyDown() != Square::Invalid && king.safetyDown().isPromotable<true>()) {
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
      if (king.safetyDown() != Square::Invalid && king.safetyDown().isPromotable<false>()) {
        bb.set(king.safetyUp().safetyLeft());
        bb.set(king.safetyUp().safetyRight());
      }
      if (king.isPromotable<false>()) {
        bb.set(king.safetyUp(2).safetyLeft(2));
        bb.set(king.safetyUp(2));
        bb.set(king.safetyUp(2).safetyRight(2));
      }
      if (king.safetyUp() != Square::Invalid && king.safetyUp().isPromotable<false>()) {
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
      if (king.safetyUp() != Square::Invalid && king.safetyUp().isPromotable<true>()) {
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
      if (king.safetyDown() != Square::Invalid && king.safetyDown().isPromotable<false>()) {
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

#define SQ_BISHOP_EACH(king, dir1, dir2, dir3) \
for (Square p1 = (king).safety ## dir1(); p1 != Square::Invalid; p1 = (p1).safety ## dir1()) { \
for (Square p = p1.safety ## dir2(); p != Square::Invalid; p = (p).safety ## dir2()) { \
bb.set(p); \
} \
for (Square p = p1.safety ## dir3(); p != Square::Invalid; p = (p).safety ## dir3()) { \
bb.set(p); \
} \
}
#define SET_BISHOP(king) do { \
SQ_BISHOP_EACH((king), LeftUp, LeftDown, RightUp); \
SQ_BISHOP_EACH((king), RightDown, LeftDown, RightUp); \
SQ_BISHOP_EACH((king), LeftDown, LeftUp, RightDown); \
SQ_BISHOP_EACH((king), RightUp, LeftUp, RightDown); \
} while(false)

    if (PieceType == Piece::BBishop) {
      SET_BISHOP(king);
      if (king.safetyUp() != Square::Invalid && king.safetyUp().isPromotable<true>()) {
        SET_BISHOP(king.safetyUp());
      }
      if (king.isPromotable<true>()) {
        SET_BISHOP(king.safetyLeft());
        SET_BISHOP(king.safetyRight());
      }
      if (king.safetyDown() != Square::Invalid && king.safetyDown().isPromotable<true>()) {
        SET_BISHOP(king.safetyDown());
      }
    }

    if (PieceType == Piece::WBishop) {
      SET_BISHOP(king);
      if (king.safetyDown() != Square::Invalid && king.safetyDown().isPromotable<false>()) {
        SET_BISHOP(king.safetyDown());
      }
      if (king.isPromotable<false>()) {
        SET_BISHOP(king.safetyLeft());
        SET_BISHOP(king.safetyRight());
      }
      if (king.safetyUp() != Square::Invalid && king.safetyUp().isPromotable<false>()) {
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

#undef SQ_BISHOP_EACH
#undef SET_BISHOP

  }

}

const AtacckableTable<Piece::BPawn> AttackableTables::BPawn;
const AtacckableTable<Piece::BLance> AttackableTables::BLance;
const AtacckableTable<Piece::BKnight> AttackableTables::BKnight;
const AtacckableTable<Piece::BSilver> AttackableTables::BSilver;
const AtacckableTable<Piece::BGold> AttackableTables::BGold;
const AtacckableTable<Piece::BBishop> AttackableTables::BBishop;
const AtacckableTable<Piece::WPawn> AttackableTables::WPawn;
const AtacckableTable<Piece::WLance> AttackableTables::WLance;
const AtacckableTable<Piece::WKnight> AttackableTables::WKnight;
const AtacckableTable<Piece::WSilver> AttackableTables::WSilver;
const AtacckableTable<Piece::WGold> AttackableTables::WGold;
const AtacckableTable<Piece::WBishop> AttackableTables::WBishop;
const AtacckableTable<Piece::Horse> AttackableTables::Horse;

} // namespace sunfish
