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
POSITION_EACH(from) { \
for (Position to = from.safety ## dir(); (full ? to : to.safety ## dir()).isValid(); to = to.safety ## dir()) { \
  _ ## type[from].set(to); \
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
  POSITION_EACH(basePos) {
    {
      uint64_t magicLow = 0ULL;
      uint64_t magicHigh = 0ULL;
      for (Position pos = basePos.safetyLeftUp(); pos.safetyLeftUp().isValid(); pos = pos.safetyLeftUp()) {
        if (Bitboard::isLow(pos)) {
          magicLow |= 1ULL << (64 - 7 + (pos.getRank() - 2) - pos);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (pos.getRank() - 2) - (pos - Bitboard::LowBits));
        }
      }
      for (Position pos = basePos.safetyRightDown(); pos.safetyRightDown().isValid(); pos = pos.safetyRightDown()) {
        if (Bitboard::isLow(pos)) {
          magicLow |= 1ULL << (64 - 7 + (pos.getRank() - 2) - pos);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (pos.getRank() - 2) - (pos - Bitboard::LowBits));
        }
      }
      _leftUp[basePos].init(magicHigh, magicLow);
    }
    {
      uint64_t magicLow = 0ULL;
      uint64_t magicHigh = 0ULL;
      for (Position pos = basePos.safetyRightUp(); pos.safetyRightUp().isValid(); pos = pos.safetyRightUp()) {
        if (Bitboard::isLow(pos)) {
          magicLow |= 1ULL << (64 - 7 + (pos.getRank() - 2) - pos);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (pos.getRank() - 2) - (pos - Bitboard::LowBits));
        }
      }
      for (Position pos = basePos.safetyLeftDown(); pos.safetyLeftDown().isValid(); pos = pos.safetyLeftDown()) {
        if (Bitboard::isLow(pos)){
          magicLow |= 1ULL << (64 - 7 + (pos.getRank() - 2) - pos);
        } else {
          magicHigh |= 1ULL << (64 - 7 + (pos.getRank() - 2) - (pos - Bitboard::LowBits));
        }
      }
      _rightUp[basePos].init(magicHigh, magicLow);
    }
  }
  for (int rank = 1; rank <= 9; rank++) {
    uint64_t magicLow = 0ULL;
    uint64_t magicHigh = 0ULL;
    for (int file = 2; file <= 8; file++) {
      Position pos(file, rank);
      if (Bitboard::isLow(pos)) {
        magicLow |= 1ULL << (64 - 7 + (8 - file) - pos);
      } else {
        magicHigh |= 1ULL << (64 - 7 + (8 - file) - (pos - Bitboard::LowBits));
      }
    }
    for (int file = 1; file <= 9; file++) {
      Position pos(file, rank);
      _rank[pos].init(magicHigh, magicLow);
    }
  }
}

/**
 * MovePatternTable
 */
MovePatternTable::MovePatternTable() {
  POSITION_EACH(basePos) {
    for (unsigned b = 0; b < 0x80; b++) {
      // up
      for (Position pos = basePos.safetyUp(); pos.isValid() && pos.getRank() >= 1; pos = pos.safetyUp()) {
        _up[basePos][b].set(pos);
        _file[basePos][b].set(pos);
        if (b & (1 << (pos.getRank() - 2))) { break; }
      }
      // down
      for (Position pos = basePos.safetyDown(); pos.isValid() && pos.getRank() <= 9; pos = pos.safetyDown()) {
        _down[basePos][b].set(pos);
        _file[basePos][b].set(pos);
        if (b & (1 << (pos.getRank() - 2))) { break; }
      }
      // left
      for (Position pos = basePos.safetyLeft(); pos.isValid() && pos.getFile() <= 9; pos = pos.safetyLeft()) {
        _rank[basePos][b].set(pos);
        _left[basePos][b].set(pos);
        if (b & (1 << (8 - pos.getFile()))) { break; }
      }
      // right
      for (Position pos = basePos.safetyRight(); pos.isValid() && pos.getFile() >= 1; pos = pos.safetyRight()) {
        _rank[basePos][b].set(pos);
        _right[basePos][b].set(pos);
        if (b & (1 << (8 - pos.getFile()))) { break; }
      }
      // left-up
      for (Position pos = basePos.safetyLeftUp(); pos.isValid() && pos.getFile() <= 9 && pos.getRank() >= 1; pos = pos.safetyLeftUp()) {
        _leftUpX[basePos][b].set(pos);
        _leftUp[basePos][b].set(pos);
        if (b & (1 << (pos.getRank() - 2))) { break; }
      }
      for (Position pos = basePos.safetyRightDown(); pos.isValid() && pos.getFile() >= 1 && pos.getRank() <= 9; pos = pos.safetyRightDown()) {
        _leftUpX[basePos][b].set(pos);
        _rightDown[basePos][b].set(pos);
        if (b & (1 << (pos.getRank() - 2))) { break; }
      }
      // right-up
      for (Position pos = basePos.safetyRightUp(); pos.isValid() && pos.getFile() >= 1 && pos.getRank() >= 1; pos = pos.safetyRightUp()) {
        _rightUpX[basePos][b].set(pos);
        _rightUp[basePos][b].set(pos);
        if (b & (1 << (pos.getRank() - 2))) { break; }
      }
      for (Position pos = basePos.safetyLeftDown(); pos.isValid() && pos.getFile() <= 9 && pos.getRank() <= 9; pos = pos.safetyLeftDown()) {
        _rightUpX[basePos][b].set(pos);
        _leftDown[basePos][b].set(pos);
        if (b & (1 << (pos.getRank() - 2))) { break; }
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
  POSITION_EACH(pos) {
    Bitboard bb;
    bb.init();
    switch (type) {
    case MoveTableType::BPawn:
      bb |= Bitboard::mask(pos.safetyUp());
      break;
    case MoveTableType::BKnight:
      bb |= Bitboard::mask(pos.safetyUp(2).safetyLeft());
      bb |= Bitboard::mask(pos.safetyUp(2).safetyRight());
      break;
    case MoveTableType::BSilver:
      bb |= Bitboard::mask(pos.safetyUp().safetyLeft());
      bb |= Bitboard::mask(pos.safetyUp());
      bb |= Bitboard::mask(pos.safetyUp().safetyRight());
      bb |= Bitboard::mask(pos.safetyDown().safetyLeft());
      bb |= Bitboard::mask(pos.safetyDown().safetyRight());
      break;
    case MoveTableType::BGold:
      bb |= Bitboard::mask(pos.safetyUp().safetyLeft());
      bb |= Bitboard::mask(pos.safetyUp());
      bb |= Bitboard::mask(pos.safetyUp().safetyRight());
      bb |= Bitboard::mask(pos.safetyLeft());
      bb |= Bitboard::mask(pos.safetyRight());
      bb |= Bitboard::mask(pos.safetyDown());
      break;
    case MoveTableType::WPawn:
      bb |= Bitboard::mask(pos.safetyDown());
      break;
    case MoveTableType::WKnight:
      bb |= Bitboard::mask(pos.safetyDown(2).safetyLeft());
      bb |= Bitboard::mask(pos.safetyDown(2).safetyRight());
      break;
    case MoveTableType::WSilver:
      bb |= Bitboard::mask(pos.safetyDown().safetyLeft());
      bb |= Bitboard::mask(pos.safetyDown());
      bb |= Bitboard::mask(pos.safetyDown().safetyRight());
      bb |= Bitboard::mask(pos.safetyUp().safetyLeft());
      bb |= Bitboard::mask(pos.safetyUp().safetyRight());
      break;
    case MoveTableType::WGold:
      bb |= Bitboard::mask(pos.safetyDown().safetyLeft());
      bb |= Bitboard::mask(pos.safetyDown());
      bb |= Bitboard::mask(pos.safetyDown().safetyRight());
      bb |= Bitboard::mask(pos.safetyLeft());
      bb |= Bitboard::mask(pos.safetyRight());
      bb |= Bitboard::mask(pos.safetyUp());
      break;
    case MoveTableType::Bishop:
    case MoveTableType::Dragon:
      bb |= Bitboard::mask(pos.safetyUp().safetyLeft());
      bb |= Bitboard::mask(pos.safetyUp().safetyRight());
      bb |= Bitboard::mask(pos.safetyDown().safetyLeft());
      bb |= Bitboard::mask(pos.safetyDown().safetyRight());
      break;
    case MoveTableType::Rook:
    case MoveTableType::Horse:
      bb |= Bitboard::mask(pos.safetyUp());
      bb |= Bitboard::mask(pos.safetyLeft());
      bb |= Bitboard::mask(pos.safetyRight());
      bb |= Bitboard::mask(pos.safetyDown());
      break;
    case MoveTableType::King:
      bb |= Bitboard::mask(pos.safetyUp().safetyLeft());
      bb |= Bitboard::mask(pos.safetyUp());
      bb |= Bitboard::mask(pos.safetyUp().safetyRight());
      bb |= Bitboard::mask(pos.safetyLeft());
      bb |= Bitboard::mask(pos.safetyRight());
      bb |= Bitboard::mask(pos.safetyDown().safetyLeft());
      bb |= Bitboard::mask(pos.safetyDown());
      bb |= Bitboard::mask(pos.safetyDown().safetyRight());
      break;
    default:
      assert(false);
    }
    _table[pos] = bb;
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
