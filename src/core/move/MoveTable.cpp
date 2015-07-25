/* MoveTable.cpp
 *
 * Kubo Ryosuke
 */

#include "./MoveTable.h"
#include "../util/StringUtil.h"
#include "logger/Logger.h"
#include <cassert>

namespace sunfish {

const DirectionMaskTable<true> dirMask;
const DirectionMaskTable<false> dirMask7x7;
const MagicNumberTable magic;
const MovePatternTable movePattern;
const OneStepMoveTable<MoveTableType::Horse> horseOneStepMove;
const OneStepMoveTable<MoveTableType::Dragon> dragonOneStepMove;

/**
 * DirectionMaskTable
 */
template <bool full>
DirectionMaskTable<full>::DirectionMaskTable() {
  // mask
#define GEN_MASK(type, dir) \
SQUARE_EACH(from) { \
for (Square to = from.safety ## dir(); (full ? to : to.safety ## dir()).isValid(); to = to.safety ## dir()) { \
  type ## _[from].set(to); \
} \
}
  GEN_MASK(file, Up);
  GEN_MASK(file, Down);
  GEN_MASK(rank, Left);
  GEN_MASK(rank, Right);
  GEN_MASK(leftUpX, LeftUp);
  GEN_MASK(leftUpX, RightDown);
  GEN_MASK(rightUpX, RightUp);
  GEN_MASK(rightUpX, LeftDown);
  GEN_MASK(up, Up);
  GEN_MASK(down, Down);
  GEN_MASK(left, Left);
  GEN_MASK(right, Right);
  GEN_MASK(leftUp, LeftUp);
  GEN_MASK(rightDown, RightDown);
  GEN_MASK(rightUp, RightUp);
  GEN_MASK(leftDown, LeftDown);
#undef GEN_MASK
}

/**
 * MagicNumberTable
 */
MagicNumberTable::MagicNumberTable() {
  SQUARE_EACH(basePos) {
    {
      uint64_t magicLow = 0ULL;
      uint64_t magicHigh = 0ULL;
      for (Square sq = basePos.safetyLeftUp(); sq.safetyLeftUp().isValid(); sq = sq.safetyLeftUp()) {
        if (Bitboard::isLow(sq)) {
          magicLow |= 1ULL << (64 - 7 + (sq.getRank() - 2) - sq);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (sq.getRank() - 2) - (sq - Bitboard::LowBits));
        }
      }
      for (Square sq = basePos.safetyRightDown(); sq.safetyRightDown().isValid(); sq = sq.safetyRightDown()) {
        if (Bitboard::isLow(sq)) {
          magicLow |= 1ULL << (64 - 7 + (sq.getRank() - 2) - sq);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (sq.getRank() - 2) - (sq - Bitboard::LowBits));
        }
      }
      leftUp_[basePos].init(magicHigh, magicLow);
    }
    {
      uint64_t magicLow = 0ULL;
      uint64_t magicHigh = 0ULL;
      for (Square sq = basePos.safetyRightUp(); sq.safetyRightUp().isValid(); sq = sq.safetyRightUp()) {
        if (Bitboard::isLow(sq)) {
          magicLow |= 1ULL << (64 - 7 + (sq.getRank() - 2) - sq);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (sq.getRank() - 2) - (sq - Bitboard::LowBits));
        }
      }
      for (Square sq = basePos.safetyLeftDown(); sq.safetyLeftDown().isValid(); sq = sq.safetyLeftDown()) {
        if (Bitboard::isLow(sq)){
          magicLow |= 1ULL << (64 - 7 + (sq.getRank() - 2) - sq);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (sq.getRank() - 2) - (sq - Bitboard::LowBits));
        }
      }
      rightUp_[basePos].init(magicHigh, magicLow);
    }
  }
  for (int rank = 1; rank <= 9; rank++) {
    uint64_t magicLow = 0ULL;
    uint64_t magicHigh = 0ULL;
    for (int file = 2; file <= 8; file++) {
      Square sq(file, rank);
      if (Bitboard::isLow(sq)) {
        magicLow |= 1ULL << (64 - 7 + (8 - file) - sq);
      } else {
        magicHigh |= 1ULL << (64 - 7 + (8 - file) - (sq - Bitboard::LowBits));
      }
    }
    for (int file = 1; file <= 9; file++) {
      Square sq(file, rank);
      rank_[sq].init(magicHigh, magicLow);
    }
  }
}

/**
 * MovePatternTable
 */
MovePatternTable::MovePatternTable() {
  SQUARE_EACH(basePos) {
    for (unsigned b = 0; b < 0x80; b++) {
      // up
      for (Square sq = basePos.safetyUp(); sq.isValid() && sq.getRank() >= 1; sq = sq.safetyUp()) {
        up_[basePos][b].set(sq);
        file_[basePos][b].set(sq);
        if (b & (1 << (sq.getRank() - 2))) { break; }
      }
      // down
      for (Square sq = basePos.safetyDown(); sq.isValid() && sq.getRank() <= 9; sq = sq.safetyDown()) {
        down_[basePos][b].set(sq);
        file_[basePos][b].set(sq);
        if (b & (1 << (sq.getRank() - 2))) { break; }
      }
      // left
      for (Square sq = basePos.safetyLeft(); sq.isValid() && sq.getFile() <= 9; sq = sq.safetyLeft()) {
        rank_[basePos][b].set(sq);
        left_[basePos][b].set(sq);
        if (b & (1 << (8 - sq.getFile()))) { break; }
      }
      // right
      for (Square sq = basePos.safetyRight(); sq.isValid() && sq.getFile() >= 1; sq = sq.safetyRight()) {
        rank_[basePos][b].set(sq);
        right_[basePos][b].set(sq);
        if (b & (1 << (8 - sq.getFile()))) { break; }
      }
      // left-up
      for (Square sq = basePos.safetyLeftUp(); sq.isValid() && sq.getFile() <= 9 && sq.getRank() >= 1; sq = sq.safetyLeftUp()) {
        leftUpX_[basePos][b].set(sq);
        leftUp_[basePos][b].set(sq);
        if (b & (1 << (sq.getRank() - 2))) { break; }
      }
      for (Square sq = basePos.safetyRightDown(); sq.isValid() && sq.getFile() >= 1 && sq.getRank() <= 9; sq = sq.safetyRightDown()) {
        leftUpX_[basePos][b].set(sq);
        rightDown_[basePos][b].set(sq);
        if (b & (1 << (sq.getRank() - 2))) { break; }
      }
      // right-up
      for (Square sq = basePos.safetyRightUp(); sq.isValid() && sq.getFile() >= 1 && sq.getRank() >= 1; sq = sq.safetyRightUp()) {
        rightUpX_[basePos][b].set(sq);
        rightUp_[basePos][b].set(sq);
        if (b & (1 << (sq.getRank() - 2))) { break; }
      }
      for (Square sq = basePos.safetyLeftDown(); sq.isValid() && sq.getFile() <= 9 && sq.getRank() <= 9; sq = sq.safetyLeftDown()) {
        rightUpX_[basePos][b].set(sq);
        leftDown_[basePos][b].set(sq);
        if (b & (1 << (sq.getRank() - 2))) { break; }
      }
    }
  }
}

/**
 * OneStepMoveTable
 * 跳び駒以外の移動
 */
template <MoveTableType type>
OneStepMoveTable<type>::OneStepMoveTable() {
  SQUARE_EACH(sq) {
    Bitboard bb;
    bb.init();
    switch (type) {
    case MoveTableType::BPawn:
      bb |= Bitboard::mask(sq.safetyUp());
      break;
    case MoveTableType::BKnight:
      bb |= Bitboard::mask(sq.safetyUp(2).safetyLeft());
      bb |= Bitboard::mask(sq.safetyUp(2).safetyRight());
      break;
    case MoveTableType::BSilver:
      bb |= Bitboard::mask(sq.safetyUp().safetyLeft());
      bb |= Bitboard::mask(sq.safetyUp());
      bb |= Bitboard::mask(sq.safetyUp().safetyRight());
      bb |= Bitboard::mask(sq.safetyDown().safetyLeft());
      bb |= Bitboard::mask(sq.safetyDown().safetyRight());
      break;
    case MoveTableType::BGold:
      bb |= Bitboard::mask(sq.safetyUp().safetyLeft());
      bb |= Bitboard::mask(sq.safetyUp());
      bb |= Bitboard::mask(sq.safetyUp().safetyRight());
      bb |= Bitboard::mask(sq.safetyLeft());
      bb |= Bitboard::mask(sq.safetyRight());
      bb |= Bitboard::mask(sq.safetyDown());
      break;
    case MoveTableType::WPawn:
      bb |= Bitboard::mask(sq.safetyDown());
      break;
    case MoveTableType::WKnight:
      bb |= Bitboard::mask(sq.safetyDown(2).safetyLeft());
      bb |= Bitboard::mask(sq.safetyDown(2).safetyRight());
      break;
    case MoveTableType::WSilver:
      bb |= Bitboard::mask(sq.safetyDown().safetyLeft());
      bb |= Bitboard::mask(sq.safetyDown());
      bb |= Bitboard::mask(sq.safetyDown().safetyRight());
      bb |= Bitboard::mask(sq.safetyUp().safetyLeft());
      bb |= Bitboard::mask(sq.safetyUp().safetyRight());
      break;
    case MoveTableType::WGold:
      bb |= Bitboard::mask(sq.safetyDown().safetyLeft());
      bb |= Bitboard::mask(sq.safetyDown());
      bb |= Bitboard::mask(sq.safetyDown().safetyRight());
      bb |= Bitboard::mask(sq.safetyLeft());
      bb |= Bitboard::mask(sq.safetyRight());
      bb |= Bitboard::mask(sq.safetyUp());
      break;
    case MoveTableType::Bishop:
    case MoveTableType::Dragon:
      bb |= Bitboard::mask(sq.safetyUp().safetyLeft());
      bb |= Bitboard::mask(sq.safetyUp().safetyRight());
      bb |= Bitboard::mask(sq.safetyDown().safetyLeft());
      bb |= Bitboard::mask(sq.safetyDown().safetyRight());
      break;
    case MoveTableType::Rook:
    case MoveTableType::Horse:
      bb |= Bitboard::mask(sq.safetyUp());
      bb |= Bitboard::mask(sq.safetyLeft());
      bb |= Bitboard::mask(sq.safetyRight());
      bb |= Bitboard::mask(sq.safetyDown());
      break;
    case MoveTableType::King:
      bb |= Bitboard::mask(sq.safetyUp().safetyLeft());
      bb |= Bitboard::mask(sq.safetyUp());
      bb |= Bitboard::mask(sq.safetyUp().safetyRight());
      bb |= Bitboard::mask(sq.safetyLeft());
      bb |= Bitboard::mask(sq.safetyRight());
      bb |= Bitboard::mask(sq.safetyDown().safetyLeft());
      bb |= Bitboard::mask(sq.safetyDown());
      bb |= Bitboard::mask(sq.safetyDown().safetyRight());
      break;
    default:
      assert(false);
    }
    table_[sq] = bb;
  }
}

const OneStepMoveTable<MoveTableType::BPawn> MoveTables::BPawn;
const OneStepMoveTable<MoveTableType::BKnight> MoveTables::BKnight;
const OneStepMoveTable<MoveTableType::BSilver> MoveTables::BSilver;
const OneStepMoveTable<MoveTableType::BGold> MoveTables::BGold;
const OneStepMoveTable<MoveTableType::WPawn> MoveTables::WPawn;
const OneStepMoveTable<MoveTableType::WKnight> MoveTables::WKnight;
const OneStepMoveTable<MoveTableType::WSilver> MoveTables::WSilver;
const OneStepMoveTable<MoveTableType::WGold> MoveTables::WGold;
const OneStepMoveTable<MoveTableType::Bishop> MoveTables::Bishop1;
const OneStepMoveTable<MoveTableType::Rook> MoveTables::Rook1;
const OneStepMoveTable<MoveTableType::King> MoveTables::King;

} // namespace sunfish
