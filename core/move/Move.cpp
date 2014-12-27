/* Move.cpp
 *
 * Kubo Ryosuke
 */

#include "Move.h"
#include "../board/Board.h"
#include <sstream>

namespace sunfish {

	std::string Move::toString() const {
		std::ostringstream oss;

		oss << to().toString();
		oss << piece().toString();
		if (promote()) {
			oss << '+';
		}
		oss << '(';
		if (isHand()) {
			oss << "00";
		} else {
			oss << from().toString();
		}
		oss << ')';

		return oss.str();
	}

	std::string Move::toStringCsa(bool black) const {
		std::ostringstream oss;

		oss << (black ? '+' : '-');

		if (isHand()) {
			oss << "00";
		} else {
			oss << from().toString();
		}

		oss << to().toString();

		if (promote()) {
			oss << piece().promote().toStringCsa(true);
		} else {
			oss << piece().toStringCsa(true);
		}

		return oss.str();
	}

}
