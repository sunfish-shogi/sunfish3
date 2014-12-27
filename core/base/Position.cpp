/* Position.cpp
 *
 * Kubo Ryosuke
 */

#include "Position.h"
#include <sstream>

namespace sunfish {

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
