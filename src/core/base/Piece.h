/* Piece.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_PIECE__
#define SUNFISH_PIECE__

#include <cstdint>

namespace sunfish {

class Piece {
public:
  static const uint8_t Promotion = 0x08;
  static const uint8_t Empty = 0x20;
  static const uint8_t White = 0x10;

  static const uint8_t HandMask = 0x27;
  static const uint8_t KindMask = 0x2f;

  static const uint8_t Pawn = 0;
  static const uint8_t Lance = 1;
  static const uint8_t Knight = 2;
  static const uint8_t Silver = 3;
  static const uint8_t Gold = 4;
  static const uint8_t Bishop = 5;
  static const uint8_t Rook = 6;
  static const uint8_t King = 7;
  static const uint8_t Tokin = Promotion | Pawn;
  static const uint8_t ProLance = Promotion | Lance;
  static const uint8_t ProKnight = Promotion | Knight;
  static const uint8_t ProSilver = Promotion | Silver;
  static const uint8_t Horse = Promotion | Bishop;
  static const uint8_t Dragon = Promotion | Rook;

  static const uint8_t BPawn = Pawn;
  static const uint8_t BLance = Lance;
  static const uint8_t BKnight = Knight;
  static const uint8_t BSilver = Silver;
  static const uint8_t BGold = Gold;
  static const uint8_t BBishop = Bishop;
  static const uint8_t BRook = Rook;
  static const uint8_t BKing = King;
  static const uint8_t BTokin = Tokin;
  static const uint8_t BProLance = ProLance;
  static const uint8_t BProKnight = ProKnight;
  static const uint8_t BProSilver = ProSilver;
  static const uint8_t BHorse = Horse;
  static const uint8_t BDragon = Dragon;

  static const uint8_t WPawn = White | Pawn;
  static const uint8_t WLance = White | Lance;
  static const uint8_t WKnight = White | Knight;
  static const uint8_t WSilver = White | Silver;
  static const uint8_t WGold = White | Gold;
  static const uint8_t WBishop = White | Bishop;
  static const uint8_t WRook = White | Rook;
  static const uint8_t WKing = White | King;
  static const uint8_t WTokin = White | Tokin;
  static const uint8_t WProLance = White | ProLance;
  static const uint8_t WProKnight = White | ProKnight;
  static const uint8_t WProSilver = White | ProSilver;
  static const uint8_t WHorse = White | Horse;
  static const uint8_t WDragon = White | Dragon;

  static const uint8_t Num = WDragon + 1;
  static const uint8_t Begin = BPawn;
  static const uint8_t End = WDragon + 1;

  static const uint8_t HandNum = Rook + 1;
  static const uint8_t HandBegin = Pawn;
  static const uint8_t HandEnd = Rook + 1;

  static const uint8_t KindNum = Dragon + 1;
  static const uint8_t KindBegin = Pawn;
  static const uint8_t KindEnd = Dragon + 1;

  static const char* names[0x21];
  static const char* namesCsa[0x21];
  static const char* namesCsaKindOnly[0x21];

private:

  uint8_t index_;

public:

  Piece() : index_(Empty) {
  }

  Piece(uint8_t index) : index_(index) {
  }

  operator uint8_t() const {
    return index_;
  }

  bool exists() const {
    return index_ != Empty;
  }
  bool isEmpty() const {
    return index_ == Empty;
  }

  Piece hand() const {
    return Piece(index_ & HandMask);
  }
  Piece promote() const {
    return Piece(index_ | Promotion);
  }
  Piece unpromote() const {
    return Piece(index_ & ~Promotion);
  }
  Piece kindOnly() const {
    return Piece(index_ & KindMask);
  }
  Piece black() const {
    return Piece(index_ & ~White);
  }
  Piece white() const {
    return Piece(index_ | White);
  }

  bool isUnpromoted() const {
    return !isPromoted();
  }
  bool isPromoted() const {
    return index_ & Promotion;
  }
  bool isBlack() const {
    return !(index_ & (Empty | White));
  }
  bool isWhite() const {
    return index_ & White;
  }

  Piece next() const {
    uint8_t nextIndex = index_ + 1U;
    if ((nextIndex == (Promotion | BGold)) ||
        (nextIndex == (Promotion | WGold)) ||
        (nextIndex == (Promotion | BKing))) {
      nextIndex++;
    }
    return Piece(nextIndex);
  }
  Piece nextUnsafe() const {
    return Piece(index_ + 1U);
  }

  const char* toString() const {
    return names[index_];
  }
  const char* toStringCsa(bool kind_only = false) const {
    return kind_only ? namesCsaKindOnly[kindOnly().index_] : namesCsa[index_];
  }
  static Piece parse(const char* str);
  static Piece parseCsa(const char* str);

};

} // namespace sunfish

#define PIECE_EACH(piece)        for (sunfish::Piece (piece) = sunfish::Piece::Begin; (piece) != sunfish::Piece::End; (piece) = (piece).next())
#define PIECE_EACH_UNSAFE(piece) for (sunfish::Piece (piece) = sunfish::Piece::Begin; (piece) != sunfish::Piece::End; (piece) = (piece).nextUnsafe())
#define PIECE_KIND_EACH(piece)   for (sunfish::Piece (piece) = sunfish::Piece::KindBegin; (piece) != sunfish::Piece::KindEnd; (piece) = (piece).nextUnsafe())
#define HAND_EACH(piece)         for (sunfish::Piece (piece) = sunfish::Piece::HandBegin; (piece) != sunfish::Piece::HandEnd; (piece) = (piece).nextUnsafe())

#endif //SUNFISH_PIECE__