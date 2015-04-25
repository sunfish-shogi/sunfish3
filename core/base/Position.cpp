/* Position.cpp
 *
 * Kubo Ryosuke
 */

#include "Position.h"
#include <sstream>

namespace sunfish {

	const int Position::DistanceTable[17][17] = {
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

#define D00 Direction::None
#define D01 Direction::Up
#define D02 Direction::Down
#define D03 Direction::Left
#define D04 Direction::Right
#define D05 Direction::LeftUp
#define D06 Direction::LeftDown
#define D07 Direction::RightUp
#define D08 Direction::RightDown
#define D09 Direction::LeftUpKnight
#define D10 Direction::LeftDownKnight
#define D11 Direction::RightUpKnight
#define D12 Direction::RightDownKnight
	const Direction Position::DirectionTable[17][17] = {
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

#define E00 DirectionEx::None
#define E01 DirectionEx::Up
#define E02 DirectionEx::Down
#define E03 DirectionEx::Left
#define E04 DirectionEx::Right
#define E05 DirectionEx::LeftUp
#define E06 DirectionEx::LeftDown
#define E07 DirectionEx::RightUp
#define E08 DirectionEx::RightDown
#define E09 DirectionEx::LeftUpKnight
#define E10 DirectionEx::LeftDownKnight
#define E11 DirectionEx::RightUpKnight
#define E12 DirectionEx::RightDownKnight
#define E13 DirectionEx::LongUp
#define E14 DirectionEx::LongDown
#define E15 DirectionEx::LongLeft
#define E16 DirectionEx::LongRight
#define E17 DirectionEx::LongLeftUp
#define E18 DirectionEx::LongLeftDown
#define E19 DirectionEx::LongRightUp
#define E20 DirectionEx::LongRightDown
	const DirectionEx Position::DirectionTableEx[17][17] = {
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

#define HNN HSideType::None
#define TP1 HSideType::Top
#define BM1 HSideType::Bottom
#define TP2 HSideType::Top2
#define BM2 HSideType::Bottom2
	const HSideType Position::HSideTypeTable[81] = {
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

#define VNN VSideType::None
#define LT1 VSideType::Left
#define RT1 VSideType::Right
#define LT2 VSideType::Left2
#define RT2 VSideType::Right2
	const VSideType Position::VSideTypeTable[81] = {
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

	std::string Position::toString() const {
		std::ostringstream oss;

		oss << getFile() << getRank();

		return oss.str();
	}

	Position Position::parse(const char* str) {
		if (str[0] >= '1' && str[0] <= '9' && str[1] >= '1' && str[1] <= '9') {
			return Position(str[0] - '0', str[1] - '0');
		}
		return Position::Invalid;
	}

}
