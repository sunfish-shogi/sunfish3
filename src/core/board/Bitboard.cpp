/* Bitboard.cpp
 *
 * Kubo Ryosuke
 */

#include "Bitboard.h"

namespace sunfish {

#if defined(UNKNOWN_API) || defined(__MINGW32__)
const int Bitboard::bfirst_[256] = {
   0, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1,
   3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1,
   4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1,
   3, 1, 2, 1, 7, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
   5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 6, 1, 2, 1,
   3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1,
   4, 1, 2, 1, 3, 1, 2, 1, 8, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1,
   3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
   6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1,
   3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1, 7, 1, 2, 1, 3, 1, 2, 1,
   4, 1, 2, 1, 3, 1, 2, 1, 5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1,
   3, 1, 2, 1, 6, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
   5, 1, 2, 1, 3, 1, 2, 1, 4, 1, 2, 1, 3, 1, 2, 1,
};
const int Bitboard::bfirst_[256] = {
   0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5,
   5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
   6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
   7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
   8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};
#endif

const Bitboard& Bitboard::mask(int pos) {
#define BB(sq) Bitboard(sq)
  static Bitboard masks[] = {
    Bitboard(0, 0), // Position::Invalid(=-1)
    BB( 0), BB( 1), BB( 2), BB( 3), BB( 4), BB( 5), BB( 6), BB( 7), BB( 8),
    BB( 9), BB(10), BB(11), BB(12), BB(13), BB(14), BB(15), BB(16), BB(17),
    BB(18), BB(19), BB(20), BB(21), BB(22), BB(23), BB(24), BB(25), BB(26),
    BB(27), BB(28), BB(29), BB(30), BB(31), BB(32), BB(33), BB(34), BB(35),
    BB(36), BB(37), BB(38), BB(39), BB(40), BB(41), BB(42), BB(43), BB(44),
    BB(45), BB(46), BB(47), BB(48), BB(49), BB(50), BB(51), BB(52), BB(53),
    BB(54), BB(55), BB(56), BB(57), BB(58), BB(59), BB(60), BB(61), BB(62),
    BB(63), BB(64), BB(65), BB(66), BB(67), BB(68), BB(69), BB(70), BB(71),
    BB(72), BB(73), BB(74), BB(75), BB(76), BB(77), BB(78), BB(79), BB(80),
  };
#undef BB
  return masks[pos+1];
}

const Bitboard Bitboard::Zero(0x00LL, 0x00LL);

const Bitboard Bitboard::file_[9] = {
  Bitboard(Zero).set(P11).set(P12).set(P13).set(P14).set(P15).set(P16).set(P17).set(P18).set(P19),
  Bitboard(Zero).set(P21).set(P22).set(P23).set(P24).set(P25).set(P26).set(P27).set(P28).set(P29),
  Bitboard(Zero).set(P31).set(P32).set(P33).set(P34).set(P35).set(P36).set(P37).set(P38).set(P39),
  Bitboard(Zero).set(P41).set(P42).set(P43).set(P44).set(P45).set(P46).set(P47).set(P48).set(P49),
  Bitboard(Zero).set(P51).set(P52).set(P53).set(P54).set(P55).set(P56).set(P57).set(P58).set(P59),
  Bitboard(Zero).set(P61).set(P62).set(P63).set(P64).set(P65).set(P66).set(P67).set(P68).set(P69),
  Bitboard(Zero).set(P71).set(P72).set(P73).set(P74).set(P75).set(P76).set(P77).set(P78).set(P79),
  Bitboard(Zero).set(P81).set(P82).set(P83).set(P84).set(P85).set(P86).set(P87).set(P88).set(P89),
  Bitboard(Zero).set(P91).set(P92).set(P93).set(P94).set(P95).set(P96).set(P97).set(P98).set(P99),
};
const Bitboard Bitboard::notFile_[9] = {
  ~file_[0],
  ~file_[1],
  ~file_[2],
  ~file_[3],
  ~file_[4],
  ~file_[5],
  ~file_[6],
  ~file_[7],
  ~file_[8],
};
const Bitboard Bitboard::BPawnMovable = Bitboard(HIGH_RANGE__, LOW_RANGE__)
  .unset(P11).unset(P21).unset(P31).unset(P41).unset(P51).unset(P61).unset(P71).unset(P81).unset(P91);
const Bitboard& Bitboard::BLanceMovable = BPawnMovable;
const Bitboard Bitboard::BKnightMovable = Bitboard(HIGH_RANGE__, LOW_RANGE__)
  .unset(P11).unset(P21).unset(P31).unset(P41).unset(P51).unset(P61).unset(P71).unset(P81).unset(P91)
  .unset(P12).unset(P22).unset(P32).unset(P42).unset(P52).unset(P62).unset(P72).unset(P82).unset(P92);
const Bitboard Bitboard::WPawnMovable = Bitboard(HIGH_RANGE__, LOW_RANGE__)
  .unset(P19).unset(P29).unset(P39).unset(P49).unset(P59).unset(P69).unset(P79).unset(P89).unset(P99);
const Bitboard& Bitboard::WLanceMovable = WPawnMovable;
const Bitboard Bitboard::WKnightMovable = Bitboard(HIGH_RANGE__, LOW_RANGE__)
  .unset(P19).unset(P29).unset(P39).unset(P49).unset(P59).unset(P69).unset(P79).unset(P89).unset(P99)
  .unset(P18).unset(P28).unset(P38).unset(P48).unset(P58).unset(P68).unset(P78).unset(P88).unset(P98);
const Bitboard Bitboard::BPromotable = Bitboard(0, 0)
  .set(P11).set(P21).set(P31).set(P41).set(P51).set(P61).set(P71).set(P81).set(P91)
  .set(P12).set(P22).set(P32).set(P42).set(P52).set(P62).set(P72).set(P82).set(P92)
  .set(P13).set(P23).set(P33).set(P43).set(P53).set(P63).set(P73).set(P83).set(P93);
const Bitboard Bitboard::WPromotable = Bitboard(0, 0)
  .set(P17).set(P27).set(P37).set(P47).set(P57).set(P67).set(P77).set(P87).set(P97)
  .set(P18).set(P28).set(P38).set(P48).set(P58).set(P68).set(P78).set(P88).set(P98)
  .set(P19).set(P29).set(P39).set(P49).set(P59).set(P69).set(P79).set(P89).set(P99);
const Bitboard Bitboard::BPromotable2 = Bitboard(0, 0)
  .set(P11).set(P21).set(P31).set(P41).set(P51).set(P61).set(P71).set(P81).set(P91)
  .set(P12).set(P22).set(P32).set(P42).set(P52).set(P62).set(P72).set(P82).set(P92);
const Bitboard Bitboard::WPromotable2 = Bitboard(0, 0)
  .set(P18).set(P28).set(P38).set(P48).set(P58).set(P68).set(P78).set(P88).set(P98)
  .set(P19).set(P29).set(P39).set(P49).set(P59).set(P69).set(P79).set(P89).set(P99);

std::string Bitboard::toString2D() const {
  std::ostringstream oss;
  POSITION_EACH_RD(pos) {
    oss << (check(pos) ? '1' : '0');
    if (pos.getFile() == 1) {
      oss << '\n';
    }
  }
  return oss.str();
}

} // namespace sunfish
