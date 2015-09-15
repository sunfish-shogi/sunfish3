/* Data.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_DATA__
#define SUNFISH_DATA__

#include "../board/Bitboard.h"
#include "../base/Piece.h"

namespace sunfish {

namespace MovableFlag_ {
  enum Type : uint8_t {
    UP         = 0x80,
    DOWN       = 0x40,
    LEFT       = 0x20,
    RIGHT      = 0x10,
    LEFT_UP    = 0x08,
    LEFT_DOWN  = 0x04,
    RIGHT_UP   = 0x02,
    RIGHT_DOWN = 0x01,
  };
}
using MovableFlag = MovableFlag_::Type;
extern const uint8_t MovableTable[32];
extern const uint8_t LongMovableTable[32];

template <int PieceType>
class AtacckableTable {
public:
  Bitboard table[81];
  AtacckableTable();
};

class AttackableTables {
private:
  AttackableTables();

  static const AtacckableTable<Piece::BPawn> BPawn;
  static const AtacckableTable<Piece::BLance> BLance;
  static const AtacckableTable<Piece::BKnight> BKnight;
  static const AtacckableTable<Piece::BSilver> BSilver;
  static const AtacckableTable<Piece::BGold> BGold;
  static const AtacckableTable<Piece::BBishop> BBishop;
  static const AtacckableTable<Piece::WPawn> WPawn;
  static const AtacckableTable<Piece::WLance> WLance;
  static const AtacckableTable<Piece::WKnight> WKnight;
  static const AtacckableTable<Piece::WSilver> WSilver;
  static const AtacckableTable<Piece::WGold> WGold;
  static const AtacckableTable<Piece::WBishop> WBishop;
  static const AtacckableTable<Piece::Horse> Horse;

public:
  static const Bitboard& bpawn(const Square& king) {
    return BPawn.table[king.index()];
  }
  static const Bitboard& blance(const Square& king) {
    return BLance.table[king.index()];
  }
  static const Bitboard& bknight(const Square& king) {
    return BKnight.table[king.index()];
  }
  static const Bitboard& bsilver(const Square& king) {
    return BSilver.table[king.index()];
  }
  static const Bitboard& bgold(const Square& king) {
    return BGold.table[king.index()];
  }
  static const Bitboard& bbishop(const Square& king) {
    return BBishop.table[king.index()];
  }
  static const Bitboard& wpawn(const Square& king) {
    return WPawn.table[king.index()];
  }
  static const Bitboard& wlance(const Square& king) {
    return WLance.table[king.index()];
  }
  static const Bitboard& wknight(const Square& king) {
    return WKnight.table[king.index()];
  }
  static const Bitboard& wsilver(const Square& king) {
    return WSilver.table[king.index()];
  }
  static const Bitboard& wgold(const Square& king) {
    return WGold.table[king.index()];
  }
  static const Bitboard& wbishop(const Square& king) {
    return WBishop.table[king.index()];
  }
  static const Bitboard& horse(const Square& king) {
    return Horse.table[king.index()];
  }

};

} // namespace sunfish

#endif // SUNFISH_DATA__
