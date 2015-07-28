/* Square.cpp
 *
 * Kubo Ryosuke
 */

#include "Square.h"
#include <sstream>

namespace sunfish {

const int8_t Square::DistanceTable[17][17] = {
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

#define D00 static_cast<int8_t>(Direction::None)
#define D01 static_cast<int8_t>(Direction::Up)
#define D02 static_cast<int8_t>(Direction::Down)
#define D03 static_cast<int8_t>(Direction::Left)
#define D04 static_cast<int8_t>(Direction::Right)
#define D05 static_cast<int8_t>(Direction::LeftUp)
#define D06 static_cast<int8_t>(Direction::LeftDown)
#define D07 static_cast<int8_t>(Direction::RightUp)
#define D08 static_cast<int8_t>(Direction::RightDown)
#define D09 static_cast<int8_t>(Direction::LeftUpKnight)
#define D10 static_cast<int8_t>(Direction::LeftDownKnight)
#define D11 static_cast<int8_t>(Direction::RightUpKnight)
#define D12 static_cast<int8_t>(Direction::RightDownKnight)
const int8_t Square::DirectionTable[17][17] = {
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

#define E00 static_cast<int8_t>(DirectionEx::None)
#define E01 static_cast<int8_t>(DirectionEx::Up)
#define E02 static_cast<int8_t>(DirectionEx::Down)
#define E03 static_cast<int8_t>(DirectionEx::Left)
#define E04 static_cast<int8_t>(DirectionEx::Right)
#define E05 static_cast<int8_t>(DirectionEx::LeftUp)
#define E06 static_cast<int8_t>(DirectionEx::LeftDown)
#define E07 static_cast<int8_t>(DirectionEx::RightUp)
#define E08 static_cast<int8_t>(DirectionEx::RightDown)
#define E09 static_cast<int8_t>(DirectionEx::LeftUpKnight)
#define E10 static_cast<int8_t>(DirectionEx::LeftDownKnight)
#define E11 static_cast<int8_t>(DirectionEx::RightUpKnight)
#define E12 static_cast<int8_t>(DirectionEx::RightDownKnight)
#define E13 static_cast<int8_t>(DirectionEx::LongUp)
#define E14 static_cast<int8_t>(DirectionEx::LongDown)
#define E15 static_cast<int8_t>(DirectionEx::LongLeft)
#define E16 static_cast<int8_t>(DirectionEx::LongRight)
#define E17 static_cast<int8_t>(DirectionEx::LongLeftUp)
#define E18 static_cast<int8_t>(DirectionEx::LongLeftDown)
#define E19 static_cast<int8_t>(DirectionEx::LongRightUp)
#define E20 static_cast<int8_t>(DirectionEx::LongRightDown)
const int8_t Square::DirectionTableEx[17][17] = {
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

#define HNN static_cast<int8_t>(HSideType::None)
#define TP1 static_cast<int8_t>(HSideType::Top)
#define BM1 static_cast<int8_t>(HSideType::Bottom)
#define TP2 static_cast<int8_t>(HSideType::Top2)
#define BM2 static_cast<int8_t>(HSideType::Bottom2)
const int8_t Square::HSideTypeTable[81] = {
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

#define VNN static_cast<int8_t>(VSideType::None)
#define LT1 static_cast<int8_t>(VSideType::Left)
#define RT1 static_cast<int8_t>(VSideType::Right)
#define LT2 static_cast<int8_t>(VSideType::Left2)
#define RT2 static_cast<int8_t>(VSideType::Right2)
const int8_t Square::VSideTypeTable[81] = {
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

std::string Square::toString() const {
  std::ostringstream oss;

  oss << (int)getFile() << (int)getRank();

  return oss.str();
}

Square Square::parse(const char* str) {
  if (str[0] >= '1' && str[0] <= '9' && str[1] >= '1' && str[1] <= '9') {
    return Square(str[0] - '0', str[1] - '0');
  }
  return Square::Invalid;
}

} // namespace sunfish
