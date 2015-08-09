/* Square.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SQUARE__
#define SUNFISH_SQUARE__

#include "../def.h"
#include <string>
#include <cstdint>
#include <cassert>

namespace sunfish {

enum {
  S91 = 0, S92, S93, S94, S95, S96, S97, S98, S99,
  S81, S82, S83, S84, S85, S86, S87, S88, S89,
  S71, S72, S73, S74, S75, S76, S77, S78, S79,
  S61, S62, S63, S64, S65, S66, S67, S68, S69,
  S51, S52, S53, S54, S55, S56, S57, S58, S59,
  S41, S42, S43, S44, S45, S46, S47, S48, S49,
  S31, S32, S33, S34, S35, S36, S37, S38, S39,
  S21, S22, S23, S24, S25, S26, S27, S28, S29,
  S11, S12, S13, S14, S15, S16, S17, S18, S19,
};

enum class Direction : int32_t {
  None,
  Up, Down, Left, Right,
  LeftUp, LeftDown,
  RightUp, RightDown,
  LeftUpKnight, LeftDownKnight,
  RightUpKnight, RightDownKnight,
};

enum class DirectionEx : int32_t {
  None,
  Up, Down, Left, Right,
  LeftUp, LeftDown,
  RightUp, RightDown,
  LeftUpKnight, LeftDownKnight,
  RightUpKnight, RightDownKnight,
  LongUp, LongDown, LongLeft, LongRight,
  LongLeftUp, LongLeftDown,
  LongRightUp, LongRightDown,
};

enum class HSideType : int32_t {
  None,
  Top,
  Bottom,
  Top2,
  Bottom2,
};

enum class VSideType : int32_t {
  None,
  Left,
  Right,
  Left2,
  Right2,
};

inline Direction getReversedDir(Direction dir) {
  switch (dir) {
    case Direction::Up:              return Direction::Down;
    case Direction::Down:            return Direction::Up;
    case Direction::Left:            return Direction::Right;
    case Direction::Right:           return Direction::Left;
    case Direction::LeftUp:          return Direction::RightDown;
    case Direction::LeftDown:        return Direction::RightUp;
    case Direction::RightUp:         return Direction::LeftDown;
    case Direction::RightDown:       return Direction::LeftUp;
    case Direction::LeftUpKnight:    return Direction::RightDownKnight;
    case Direction::LeftDownKnight:  return Direction::RightUpKnight;
    case Direction::RightUpKnight:   return Direction::LeftDownKnight;
    case Direction::RightDownKnight: return Direction::LeftUpKnight;
    default: return Direction::None;
  }
}

class Square {
private:

  int32_t index_;

  enum {
    D00 = static_cast<int8_t>(Direction::None),
    D01 = static_cast<int8_t>(Direction::Up),
    D02 = static_cast<int8_t>(Direction::Down),
    D03 = static_cast<int8_t>(Direction::Left),
    D04 = static_cast<int8_t>(Direction::Right),
    D05 = static_cast<int8_t>(Direction::LeftUp),
    D06 = static_cast<int8_t>(Direction::LeftDown),
    D07 = static_cast<int8_t>(Direction::RightUp),
    D08 = static_cast<int8_t>(Direction::RightDown),
    D09 = static_cast<int8_t>(Direction::LeftUpKnight),
    D10 = static_cast<int8_t>(Direction::LeftDownKnight),
    D11 = static_cast<int8_t>(Direction::RightUpKnight),
    D12 = static_cast<int8_t>(Direction::RightDownKnight),
  };

  enum {
    E00 = static_cast<int8_t>(DirectionEx::None),
    E01 = static_cast<int8_t>(DirectionEx::Up),
    E02 = static_cast<int8_t>(DirectionEx::Down),
    E03 = static_cast<int8_t>(DirectionEx::Left),
    E04 = static_cast<int8_t>(DirectionEx::Right),
    E05 = static_cast<int8_t>(DirectionEx::LeftUp),
    E06 = static_cast<int8_t>(DirectionEx::LeftDown),
    E07 = static_cast<int8_t>(DirectionEx::RightUp),
    E08 = static_cast<int8_t>(DirectionEx::RightDown),
    E09 = static_cast<int8_t>(DirectionEx::LeftUpKnight),
    E10 = static_cast<int8_t>(DirectionEx::LeftDownKnight),
    E11 = static_cast<int8_t>(DirectionEx::RightUpKnight),
    E12 = static_cast<int8_t>(DirectionEx::RightDownKnight),
    E13 = static_cast<int8_t>(DirectionEx::LongUp),
    E14 = static_cast<int8_t>(DirectionEx::LongDown),
    E15 = static_cast<int8_t>(DirectionEx::LongLeft),
    E16 = static_cast<int8_t>(DirectionEx::LongRight),
    E17 = static_cast<int8_t>(DirectionEx::LongLeftUp),
    E18 = static_cast<int8_t>(DirectionEx::LongLeftDown),
    E19 = static_cast<int8_t>(DirectionEx::LongRightUp),
    E20 = static_cast<int8_t>(DirectionEx::LongRightDown),
  };

  enum {
    HNN = static_cast<int8_t>(HSideType::None),
    TP1 = static_cast<int8_t>(HSideType::Top),
    BM1 = static_cast<int8_t>(HSideType::Bottom),
    TP2 = static_cast<int8_t>(HSideType::Top2),
    BM2 = static_cast<int8_t>(HSideType::Bottom2),
  };

  enum {
    VNN = static_cast<int8_t>(VSideType::None),
    LT1 = static_cast<int8_t>(VSideType::Left),
    RT1 = static_cast<int8_t>(VSideType::Right),
    LT2 = static_cast<int8_t>(VSideType::Left2),
    RT2 = static_cast<int8_t>(VSideType::Right2),
  };

public:

  static CONSTEXPR_CONST int32_t Invalid = -1;
  static CONSTEXPR_CONST int32_t N = 81;
  static CONSTEXPR_CONST int32_t RankN = 9;
  static CONSTEXPR_CONST int32_t FileN = 9;
  static CONSTEXPR_CONST int32_t Begin = 0;
  static CONSTEXPR_CONST int32_t End = Begin + N;

  static CONSTEXPR int32_t index(int file, int rank) {
    return (9 - file) * RankN + rank - 1;
  }

  CONSTEXPR Square() : index_(Invalid) {
  }
  CONSTEXPR Square(const Square&) = default;
  CONSTEXPR Square(Square&&) = default;
  CONSTEXPR Square(int32_t index) : index_(index) {
  }
  explicit CONSTEXPR Square(int32_t file, int32_t rank) :
    index_(index(file, rank)) {
  }

  Square& operator=(const Square&) = default;
  Square& operator=(Square&&) = default;

  CONSTEXPR bool operator==(const Square& sq) const {
    return index_ == sq.index_;
  }
  CONSTEXPR bool operator!=(const Square& sq) const {
    return index_ != sq.index_;
  }

  CONSTEXPR bool operator==(int32_t i) const {
    return index_ == i;
  }
  CONSTEXPR bool operator!=(int32_t i) const {
    return index_ != i;
  }

  explicit CONSTEXPR operator int32_t() const {
    return index_;
  }
  CONSTEXPR int32_t index() const {
    return index_;
  }

  Square& set(int32_t i) {
    index_ = i;
    return *this;
  }
  Square& set(int32_t file, int32_t rank) {
    index_ = index(file, rank);
    return *this;
  }

  CONSTEXPR int32_t getFile() const {
    return 9 - (index_ / RankN);
  }
  CONSTEXPR int32_t getRank() const {
    return index_ % RankN + 1;
  }

  CONSTEXPR bool isValid() const {
    return index_ != Invalid;
  }
  CONSTEXPR bool isInvalid() const {
    return !isValid();
  }
  CONSTEXPR bool isStrictValid() const {
    return index_ >= 0 && index_ < End;
  }
  static CONSTEXPR bool isValidFile(int32_t file) {
    return file >= 1 && file <= 9;
  }
  static CONSTEXPR bool isValidRank(int32_t rank) {
    return rank >= 1 && rank <= 9;
  }
  template<bool black>
  CONSTEXPR bool isPromotable() const {
    return black ? getRank() - 1 <= 2 : getRank() - 1 >= 6;
  }
  template<bool black>
  CONSTEXPR bool isPawnMovable() const {
    return black ? getRank() - 1 != 0 : getRank() - 1 != 8;
  }
  template<bool black>
  CONSTEXPR bool isPawnSignficant() const {
    return black ? getRank() - 1 >= 3 : getRank() - 1 <= 5;
  }
  template<bool black>
  CONSTEXPR bool isLanceMovable() const {
    return isPawnMovable<black>();
  }
  template<bool black>
  CONSTEXPR bool isLanceSignficant() const {
    return black ? getRank() - 1 >= 2 : getRank() - 1 <= 6;
  }
  template<bool black>
  CONSTEXPR bool isKnightMovable() const {
    return black ? getRank() - 1 >= 2 : getRank() - 1 <= 6;
  }
  CONSTEXPR Square reverse() const {
    return N - 1 - index_;
  }
  CONSTEXPR Square sym() const {
    return Square(FileN + 1 - getFile(), getRank());
  }
  CONSTEXPR Square up(int32_t distance = 1) const {
    return index_ - distance;
  }
  CONSTEXPR Square down(int32_t distance = 1) const {
    return index_ + distance;
  }
  CONSTEXPR Square left(int32_t distance = 1) const {
    return index_ - distance * RankN;
  }
  CONSTEXPR Square right(int32_t distance = 1) const {
    return index_ + distance * RankN;
  }
  CONSTEXPR Square leftUp(int32_t distance = 1) const {
    return (*this).left(distance).up(distance);
  }
  CONSTEXPR Square leftDown(int32_t distance = 1) const {
    return (*this).left(distance).down(distance);
  }
  CONSTEXPR Square rightUp(int32_t distance = 1) const {
    return (*this).right(distance).up(distance);
  }
  CONSTEXPR Square rightDown(int32_t distance = 1) const {
    return (*this).right(distance).down(distance);
  }
  CONSTEXPR Square leftUpKnight() const {
    return (*this).left().up(2);
  }
  CONSTEXPR Square leftDownKnight() const {
    return (*this).left().down(2);
  }
  CONSTEXPR Square rightUpKnight() const {
    return (*this).right().up(2);
  }
  CONSTEXPR Square rightDownKnight() const {
    return (*this).right().down(2);
  }
  CONSTEXPR Square safetyUp(int32_t distance = 1) const {
    return (!isInvalid() && getRank() - distance >= 1)
      ? Square(getFile(), getRank() - distance) : Square(Invalid);
  }
  CONSTEXPR Square safetyDown(int32_t distance = 1) const {
    return (!isInvalid() && getRank() + distance <= 9)
      ? Square(getFile(), getRank() + distance) : Square(Invalid);
  }
  CONSTEXPR Square safetyLeft(int32_t distance = 1) const {
    return (!isInvalid() && getFile() + distance <= 9)
      ? Square(getFile() + distance, getRank()) : Square(Invalid);
  }
  CONSTEXPR Square safetyRight(int32_t distance = 1) const {
    return (!isInvalid() && getFile() - distance >= 1)
      ? Square(getFile() - distance, getRank()) : Square(Invalid);
  }
  CONSTEXPR Square safetyLeftUp(int32_t distance = 1) const {
    return (*this).safetyLeft(distance).safetyUp(distance);
  }
  CONSTEXPR Square safetyLeftDown(int32_t distance = 1) const {
    return (*this).safetyLeft(distance).safetyDown(distance);
  }
  CONSTEXPR Square safetyRightUp(int32_t distance = 1) const {
    return (*this).safetyRight(distance).safetyUp(distance);
  }
  CONSTEXPR Square safetyRightDown(int32_t distance = 1) const {
    return (*this).safetyRight(distance).safetyDown(distance);
  }
  CONSTEXPR Square safetyLeftUpKnight() const {
    return (*this).safetyLeft().safetyUp(2);
  }
  CONSTEXPR Square safetyLeftDownKnight() const {
    return (*this).safetyLeft().safetyDown(2);
  }
  CONSTEXPR Square safetyRightUpKnight() const {
    return (*this).safetyRight().safetyUp(2);
  }
  CONSTEXPR Square safetyRightDownKnight() const {
    return (*this).safetyRight().safetyDown(2);
  }
  CONSTEXPR Square next() const {
    return Square(index_ + 1);
  }
  Square nextRightDown() const {
    int32_t file = index_ / RankN;
    if (file == 8) {
      int32_t rank = index_ % RankN;
      if (rank == RankN - 1) {
        return End;
      }
      return index_ - (FileN - 1) * RankN + 1;
    }
    return index_ + RankN;
  }
  int32_t distance(const Square& to) const {
    static CONSTEXPR_CONST int8_t t[17][17] = {
      { 8, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 8, },
      { 0, 7, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7, 0, },
      { 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, },
      { 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, },
      { 0, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, },
      { 0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, },
      { 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 0, 0, 0, 0, 0, 0, },
      { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, },
      { 8, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, },
      { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, },
      { 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 0, 0, 0, 0, 0, 0, },
      { 0, 0, 0, 0, 0, 3, 0, 0, 3, 0, 0, 3, 0, 0, 0, 0, 0, },
      { 0, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 4, 0, 0, 0, 0, },
      { 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, 0, 5, 0, 0, 0, },
      { 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 6, 0, 0, },
      { 0, 7, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 7, 0, },
      { 8, 0, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 8, },
    };
    int x = (int)to.index_ % RankN - (int)index_ % RankN + 8;
    int y = (int)to.index_ / RankN - (int)index_ / RankN + 8;
    return t[x][y];
  }
  Direction dir(const Square& to) const {
    static CONSTEXPR_CONST int8_t t[17][17] = {
      { D05, D00, D00, D00, D00, D00, D00, D00, D01, D00, D00, D00, D00, D00, D00, D00, D07, },
      { D00, D05, D00, D00, D00, D00, D00, D00, D01, D00, D00, D00, D00, D00, D00, D07, D00, },
      { D00, D00, D05, D00, D00, D00, D00, D00, D01, D00, D00, D00, D00, D00, D07, D00, D00, },
      { D00, D00, D00, D05, D00, D00, D00, D00, D01, D00, D00, D00, D00, D07, D00, D00, D00, },
      { D00, D00, D00, D00, D05, D00, D00, D00, D01, D00, D00, D00, D07, D00, D00, D00, D00, },
      { D00, D00, D00, D00, D00, D05, D00, D00, D01, D00, D00, D07, D00, D00, D00, D00, D00, },
      { D00, D00, D00, D00, D00, D00, D05, D09, D01, D11, D07, D00, D00, D00, D00, D00, D00, },
      { D00, D00, D00, D00, D00, D00, D00, D05, D01, D07, D00, D00, D00, D00, D00, D00, D00, },
      { D03, D03, D03, D03, D03, D03, D03, D03, D00, D04, D04, D04, D04, D04, D04, D04, D04, },
      { D00, D00, D00, D00, D00, D00, D00, D06, D02, D08, D00, D00, D00, D00, D00, D00, D00, },
      { D00, D00, D00, D00, D00, D00, D06, D10, D02, D12, D08, D00, D00, D00, D00, D00, D00, },
      { D00, D00, D00, D00, D00, D06, D00, D00, D02, D00, D00, D08, D00, D00, D00, D00, D00, },
      { D00, D00, D00, D00, D06, D00, D00, D00, D02, D00, D00, D00, D08, D00, D00, D00, D00, },
      { D00, D00, D00, D06, D00, D00, D00, D00, D02, D00, D00, D00, D00, D08, D00, D00, D00, },
      { D00, D00, D06, D00, D00, D00, D00, D00, D02, D00, D00, D00, D00, D00, D08, D00, D00, },
      { D00, D06, D00, D00, D00, D00, D00, D00, D02, D00, D00, D00, D00, D00, D00, D08, D00, },
      { D06, D00, D00, D00, D00, D00, D00, D00, D02, D00, D00, D00, D00, D00, D00, D00, D08, },
    };
    int x = (int)to.index_ % RankN - (int)index_ % RankN + 8;
    int y = (int)to.index_ / RankN - (int)index_ / RankN + 8;
    return static_cast<Direction>(t[x][y]);
  }
  DirectionEx dirEx(const Square& to) const {
    static CONSTEXPR_CONST int8_t t[17][17] = {
      { E17, E00, E00, E00, E00, E00, E00, E00, E13, E00, E00, E00, E00, E00, E00, E00, E19, },
      { E00, E17, E00, E00, E00, E00, E00, E00, E13, E00, E00, E00, E00, E00, E00, E19, E00, },
      { E00, E00, E17, E00, E00, E00, E00, E00, E13, E00, E00, E00, E00, E00, E19, E00, E00, },
      { E00, E00, E00, E17, E00, E00, E00, E00, E13, E00, E00, E00, E00, E19, E00, E00, E00, },
      { E00, E00, E00, E00, E17, E00, E00, E00, E13, E00, E00, E00, E19, E00, E00, E00, E00, },
      { E00, E00, E00, E00, E00, E17, E00, E00, E13, E00, E00, E19, E00, E00, E00, E00, E00, },
      { E00, E00, E00, E00, E00, E00, E17, E09, E13, E11, E19, E00, E00, E00, E00, E00, E00, },
      { E00, E00, E00, E00, E00, E00, E00, E05, E01, E07, E00, E00, E00, E00, E00, E00, E00, },
      { E15, E15, E15, E15, E15, E15, E15, E03, E00, E04, E16, E16, E16, E16, E16, E16, E16, },
      { E00, E00, E00, E00, E00, E00, E00, E06, E02, E08, E00, E00, E00, E00, E00, E00, E00, },
      { E00, E00, E00, E00, E00, E00, E18, E10, E14, E12, E20, E00, E00, E00, E00, E00, E00, },
      { E00, E00, E00, E00, E00, E18, E00, E00, E14, E00, E00, E20, E00, E00, E00, E00, E00, },
      { E00, E00, E00, E00, E18, E00, E00, E00, E14, E00, E00, E00, E20, E00, E00, E00, E00, },
      { E00, E00, E00, E18, E00, E00, E00, E00, E14, E00, E00, E00, E00, E20, E00, E00, E00, },
      { E00, E00, E18, E00, E00, E00, E00, E00, E14, E00, E00, E00, E00, E00, E20, E00, E00, },
      { E00, E18, E00, E00, E00, E00, E00, E00, E14, E00, E00, E00, E00, E00, E00, E20, E00, },
      { E18, E00, E00, E00, E00, E00, E00, E00, E14, E00, E00, E00, E00, E00, E00, E00, E20, },
    };
    int x = (int)to.index_ % RankN - (int)index_ % RankN + 8;
    int y = (int)to.index_ / RankN - (int)index_ / RankN + 8;
    return static_cast<DirectionEx>(t[x][y]);
  }
  HSideType sideTypeH() const {
    static CONSTEXPR_CONST int8_t t[81] = {
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
      TP1, TP2, HNN, HNN, HNN, HNN, HNN, BM2, BM1,
    };
    return static_cast<HSideType>(t[index_]);
  }
  VSideType sideTypeV() const {
    static CONSTEXPR_CONST int8_t t[81] = {
      LT1, LT1, LT1, LT1, LT1, LT1, LT1, LT1, LT1,
      LT2, LT2, LT2, LT2, LT2, LT2, LT2, LT2, LT2,
      VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN,
      VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN,
      VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN,
      VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN,
      VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN, VNN,
      RT2, RT2, RT2, RT2, RT2, RT2, RT2, RT2, RT2,
      RT1, RT1, RT1, RT1, RT1, RT1, RT1, RT1, RT1,
    };
    return static_cast<VSideType>(t[index_]);
  }

  std::string toString() const;
  static Square parse(const char* str);

};

} // namespace sunfish

#define SQUARE_EACH(sq)    for (sunfish::Square (sq) = sunfish::Square::Begin; (sq) != sunfish::Square::End; (sq) = (sq).next())

// scanning right-down
#define SQUARE_EACH_RD(sq) for (sunfish::Square (sq) = S91; (sq) != sunfish::Square::End; (sq) = (sq).nextRightDown())

inline bool operator==(uint8_t index, const sunfish::Square& square) {
  return index == square.index();
}

inline bool operator!=(uint8_t index, const sunfish::Square& square) {
  return index != square.index();
}

inline std::ostream& operator<<(std::ostream& os, const sunfish::Square& square) {
  os << square.index();
  return os;
}

#endif //SUNFISH_SQUARE__
