/* Move.cpp
 *
 * Kubo Ryosuke
 */

#include "Move.h"
#include "../board/Board.h"
#include <sstream>

namespace sunfish {

	uint16_t Move::serialize16(const Move& obj) {
		if (obj.isEmpty()) {
			return S16_EMPTY;
		} else if (obj.isHand()) {
			assert(!(obj.piece().isPromoted()));
			uint32_t masked = obj._move & (TO | PIECE);
			uint32_t shifted = (masked >> S16_HAND_SHIFT);
			assert(shifted < 0x0800);
			uint16_t data = (uint16_t)(shifted | S16_HAND);
			assert(((uint32_t)data & ~S16_HAND) < 0x0800);
			return data;
		} else {
			uint32_t masked = obj._move & (FROM | TO | PROMOTE);
			assert(masked < S16_HAND);
			uint16_t data = (uint16_t)masked;
			assert(!(data & S16_HAND));
			return data;
		}
	}

	Move Move::deserialize16(uint16_t value, const Board& board) {
		if (value == S16_EMPTY) {
			return empty();
		} else if (value & S16_HAND) {
			Move obj;
			uint32_t masked = (uint32_t)value & ~S16_HAND;
			assert(masked < 0x0800);
			obj._move = masked << S16_HAND_SHIFT;
			assert(!obj.piece().isPromoted());
			return obj;
		} else {
			Move obj;
			obj._move = value;
			obj.setPiece(board.getBoardPiece(obj.from()));
			return obj;
		}
	}

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
