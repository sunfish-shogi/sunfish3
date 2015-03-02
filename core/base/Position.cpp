/* Position.cpp
 *
 * Kubo Ryosuke
 */

#include "Position.h"
#include <sstream>

namespace sunfish {

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
