/* Data.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_DATA__
#define SUNFISH_DATA__

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
