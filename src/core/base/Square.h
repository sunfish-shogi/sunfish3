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
  P91 = 0, P92, P93, P94, P95, P96, P97, P98, P99,
  P81, P82, P83, P84, P85, P86, P87, P88, P89,
  P71, P72, P73, P74, P75, P76, P77, P78, P79,
  P61, P62, P63, P64, P65, P66, P67, P68, P69,
  P51, P52, P53, P54, P55, P56, P57, P58, P59,
  P41, P42, P43, P44, P45, P46, P47, P48, P49,
  P31, P32, P33, P34, P35, P36, P37, P38, P39,
  P21, P22, P23, P24, P25, P26, P27, P28, P29,
  P11, P12, P13, P14, P15, P16, P17, P18, P19,
};

enum class Direction : int8_t {
  None,
  Up, Down, Left, Right,
  LeftUp, LeftDown,
  RightUp, RightDown,
  LeftUpKnight, LeftDownKnight,
  RightUpKnight, RightDownKnight,
};

enum class DirectionEx : int8_t {
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

enum class HSideType : int8_t {
  None,
  Top,
  Bottom,
  Top2,
  Bottom2,
};

enum class VSideType : int8_t {
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

  static const int8_t DistanceTable[17][17];
  static const Direction DirectionTable[17][17];
  static const DirectionEx DirectionTableEx[17][17];
  static const HSideType HSideTypeTable[81];
  static const VSideType VSideTypeTable[81];

  int8_t index_;

public:

  static CONSTEXPR int8_t Invalid = -1;
  static CONSTEXPR int8_t N = 81;
  static CONSTEXPR int8_t RankN = 9;
  static CONSTEXPR int8_t FileN = 9;
  static CONSTEXPR int8_t Begin = 0;
  static CONSTEXPR int8_t End = Begin + N;

  Square() : index_(Invalid) {
  }
  Square(const Square&) = default;
  Square(Square&&) = default;
  Square(int8_t index) : index_(index) {
  }
  explicit Square(int8_t file, int8_t rank) {
    set(file, rank);
  }

  Square& operator=(const Square&) = default;
  Square& operator=(Square&&) = default;

  bool operator==(const Square& sq) const {
    return index_ == sq.index_;
  }
  bool operator!=(const Square& sq) const {
    return index_ != sq.index_;
  }

  bool operator==(int8_t i) const {
    return index_ == i;
  }
  bool operator!=(int8_t i) const {
    return index_ != i;
  }

  explicit operator int8_t() const {
    return index_;
  }
  int8_t index() const {
    return index_;
  }

  Square& set(int8_t i) {
    index_ = i;
    return *this;
  }
  Square& set(int8_t file, int8_t rank) {
    index_ = (9 - file) * RankN + rank - 1;
    return *this;
  }

  int8_t getFile() const {
    assert(index_ >= Begin);
    assert(index_ < End);
    return 9 - (index_ / RankN);
  }
  int8_t getRank() const {
    assert(index_ >= Begin);
    assert(index_ < End);
    return index_ % RankN + 1;
  }

  bool isValid() const {
    return index_ != Invalid;
  }
  bool isInvalid() const {
    return !isValid();
  }
  bool isStrictValid() const {
    return index_ >= 0 && index_ < End;
  }
  static bool isValidFile(int8_t file) {
    return file >= 1 && file <= 9;
  }
  static bool isValidRank(int8_t rank) {
    return rank >= 1 && rank <= 9;
  }
  template<bool black>
  bool isPromotable() const {
    assert(isValid());
    if (black) {
      return getRank() - 1 <= 2;
    } else {
      return getRank() - 1 >= 6;
    }
  }
  template<bool black>
  bool isPawnMovable() const {
    assert(isValid());
    if (black) {
      return getRank() - 1 != 0;
    } else {
      return getRank() - 1 != 8;
    }
  }
  template<bool black>
  bool isPawnSignficant() const {
    assert(isValid());
    if (black) {
      return getRank() - 1 >= 3;
    } else {
      return getRank() - 1 <= 5;
    }
  }
  template<bool black>
  bool isLanceMovable() const {
    assert(isValid());
    return isPawnMovable<black>();
  }
  template<bool black>
  bool isLanceSignficant() const {
    assert(isValid());
    if (black) {
      return getRank() - 1 >= 2;
    } else {
      return getRank() - 1 <= 6;
    }
  }
  template<bool black>
  bool isKnightMovable() const {
    assert(isValid());
    if (black) {
      return getRank() - 1 >= 2;
    } else {
      return getRank() - 1 <= 6;
    }
  }
  Square reverse() const {
    assert(isValid());
    return N - 1 - index_;
  }
  Square up(int8_t distance = 1) const {
    assert(isValid());
    assert(getRank() != 1);
    return index_ - distance;
  }
  Square down(int8_t distance = 1) const {
    assert(isValid());
    assert(getRank() != 9);
    return index_ + distance;
  }
  Square left(int8_t distance = 1) const {
    assert(isValid());
    assert(getFile() != 9);
    return index_ - distance * RankN;
  }
  Square right(int8_t distance = 1) const {
    assert(isValid());
    assert(getFile() != 1);
    return index_ + distance * RankN;
  }
  Square leftUp(int8_t distance = 1) const {
    assert(isValid());
    assert(getRank() != 1);
    assert(getFile() != 9);
    return (*this).left(distance).up(distance);
  }
  Square leftDown(int8_t distance = 1) const {
    assert(isValid());
    assert(getRank() != 9);
    assert(getFile() != 9);
    return (*this).left(distance).down(distance);
  }
  Square rightUp(int8_t distance = 1) const {
    assert(isValid());
    assert(getRank() != 1);
    assert(getFile() != 1);
    return (*this).right(distance).up(distance);
  }
  Square rightDown(int8_t distance = 1) const {
    assert(isValid());
    assert(getRank() != 9);
    assert(getFile() != 1);
    return (*this).right(distance).down(distance);
  }
  Square leftUpKnight() const {
    assert(isValid());
    assert(getRank() >= 3);
    assert(getFile() != 9);
    return (*this).left().up(2);
  }
  Square leftDownKnight() const {
    assert(isValid());
    assert(getRank() <= 7);
    assert(getFile() != 9);
    return (*this).left().down(2);
  }
  Square rightUpKnight() const {
    assert(isValid());
    assert(getRank() >= 3);
    assert(getFile() != 1);
    return (*this).right().up(2);
  }
  Square rightDownKnight() const {
    assert(isValid());
    assert(getRank() <= 7);
    assert(getFile() != 1);
    return (*this).right().down(2);
  }
  Square safetyUp(int8_t distance = 1) const {
    if (isInvalid()) { return Square(Invalid); }
    int8_t file = getFile();
    int8_t rank = getRank() - distance;
    assert(file >= 1);
    assert(file <= 9);
    assert(rank <= 9);
    return rank >= 1 ? Square(file, rank) : Square(Invalid);
  }
  Square safetyDown(int8_t distance = 1) const {
    if (isInvalid()) { return Square(Invalid); }
    int8_t file = getFile();
    int8_t rank = getRank() + distance;
    assert(file >= 1);
    assert(file <= 9);
    assert(rank >= 1);
    return rank <= 9 ? Square(file, rank) : Square(Invalid);
  }
  Square safetyLeft(int8_t distance = 1) const {
    if (isInvalid()) { return Square(Invalid); }
    int8_t file = getFile() + distance;
    int8_t rank = getRank();
    assert(file >= 1);
    assert(rank >= 1);
    assert(rank <= 9);
    return file <= 9 ? Square(file, rank) : Square(Invalid);
  }
  Square safetyRight(int8_t distance = 1) const {
    if (isInvalid()) { return Square(Invalid); }
    int8_t file = getFile() - distance;
    int8_t rank = getRank();
    assert(file <= 9);
    assert(rank >= 1);
    assert(rank <= 9);
    return file >= 1 ? Square(file, rank) : Square(Invalid);
  }
  Square safetyLeftUp(int8_t distance = 1) const {
    return (*this).safetyLeft(distance).safetyUp(distance);
  }
  Square safetyLeftDown(int8_t distance = 1) const {
    return (*this).safetyLeft(distance).safetyDown(distance);
  }
  Square safetyRightUp(int8_t distance = 1) const {
    return (*this).safetyRight(distance).safetyUp(distance);
  }
  Square safetyRightDown(int8_t distance = 1) const {
    return (*this).safetyRight(distance).safetyDown(distance);
  }
  Square safetyLeftUpKnight() const {
    return (*this).safetyLeft().safetyUp(2);
  }
  Square safetyLeftDownKnight() const {
    return (*this).safetyLeft().safetyDown(2);
  }
  Square safetyRightUpKnight() const {
    return (*this).safetyRight().safetyUp(2);
  }
  Square safetyRightDownKnight() const {
    return (*this).safetyRight().safetyDown(2);
  }
  Square next() const {
    return index_ + 1;
  }
  Square nextRightDown() const {
    int8_t file = index_ / RankN;
    if (file == 8) {
      int8_t rank = index_ % RankN;
      if (rank == RankN - 1) {
        return End;
      }
      return index_ - (FileN - 1) * RankN + 1;
    }
    return index_ + RankN;
  }
  int8_t distance(const Square& to) const {
    int rank = (int)to.index_ % RankN - (int)index_ % RankN + 8;
    int file = (int)to.index_ / RankN - (int)index_ / RankN + 8;
    assert(rank >= 0);
    assert(rank <= 16);
    assert(file >= 0);
    assert(file <= 16);
    return DistanceTable[rank][file];
  }
  Direction dir(const Square& to) const {
    int rank = (int)to.index_ % RankN - (int)index_ % RankN + 8;
    int file = (int)to.index_ / RankN - (int)index_ / RankN + 8;
    assert(rank >= 0);
    assert(rank <= 16);
    assert(file >= 0);
    assert(file <= 16);
    return DirectionTable[rank][file];
  }
  DirectionEx dirEx(const Square& to) const {
    int rank = (int)to.index_ % RankN - (int)index_ % RankN + 8;
    int file = (int)to.index_ / RankN - (int)index_ / RankN + 8;
    assert(rank >= 0);
    assert(rank <= 16);
    assert(file >= 0);
    assert(file <= 16);
    return DirectionTableEx[rank][file];
  }
  HSideType sideTypeH() const {
    assert(index_ != Invalid);
    return HSideTypeTable[index_];
  }
  VSideType sideTypeV() const {
    assert(index_ != Invalid);
    return VSideTypeTable[index_];
  }

  std::string toString() const;
  static Square parse(const char* str);

};

} // namespace sunfish

#define SQUARE_EACH(sq)    for (sunfish::Square (sq) = sunfish::Square::Begin; (sq) != sunfish::Square::End; (sq) = (sq).next())

// scanning right-down
#define SQUARE_EACH_RD(sq) for (sunfish::Square (sq) = P91; (sq) != sunfish::Square::End; (sq) = (sq).nextRightDown())

#endif //SUNFISH_SQUARE__
