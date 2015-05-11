/* Data.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_DATA__
#define __SUNFISH_DATA__

#include "../board/Bitboard.h"
#include "../base/Piece.h"

namespace sunfish {

struct MovableFlags {
  bool up;
  bool down;
  bool left;
  bool right;
  bool leftUp;
  bool leftDown;
  bool rightUp;
  bool rightDown;
};
extern MovableFlags MovableTable[32];
extern MovableFlags LongMovableTable[32];

template <int PieceType>
class AtacckableTable {
public:
  Bitboard table[81];
  AtacckableTable();
};

class AttackableTables {
private:
  AttackableTables();

  static AtacckableTable<Piece::BPawn> BPawn;
  static AtacckableTable<Piece::BLance> BLance;
  static AtacckableTable<Piece::BKnight> BKnight;
  static AtacckableTable<Piece::BSilver> BSilver;
  static AtacckableTable<Piece::BGold> BGold;
  static AtacckableTable<Piece::BBishop> BBishop;
  static AtacckableTable<Piece::WPawn> WPawn;
  static AtacckableTable<Piece::WLance> WLance;
  static AtacckableTable<Piece::WKnight> WKnight;
  static AtacckableTable<Piece::WSilver> WSilver;
  static AtacckableTable<Piece::WGold> WGold;
  static AtacckableTable<Piece::WBishop> WBishop;
  static AtacckableTable<Piece::Horse> Horse;

public:
  static const Bitboard& bpawn(const Position& king) {
    return BPawn.table[(uint8_t)king];
  }
  static const Bitboard& blance(const Position& king) {
    return BLance.table[(uint8_t)king];
  }
  static const Bitboard& bknight(const Position& king) {
    return BKnight.table[(uint8_t)king];
  }
  static const Bitboard& bsilver(const Position& king) {
    return BSilver.table[(uint8_t)king];
  }
  static const Bitboard& bgold(const Position& king) {
    return BGold.table[(uint8_t)king];
  }
  static const Bitboard& bbishop(const Position& king) {
    return BBishop.table[(uint8_t)king];
  }
  static const Bitboard& wpawn(const Position& king) {
    return WPawn.table[(uint8_t)king];
  }
  static const Bitboard& wlance(const Position& king) {
    return WLance.table[(uint8_t)king];
  }
  static const Bitboard& wknight(const Position& king) {
    return WKnight.table[(uint8_t)king];
  }
  static const Bitboard& wsilver(const Position& king) {
    return WSilver.table[(uint8_t)king];
  }
  static const Bitboard& wgold(const Position& king) {
    return WGold.table[(uint8_t)king];
  }
  static const Bitboard& wbishop(const Position& king) {
    return WBishop.table[(uint8_t)king];
  }
  static const Bitboard& horse(const Position& king) {
    return Horse.table[(uint8_t)king];
  }

};

} // namespace sunfish

#endif // __SUNFISH_DATA__
