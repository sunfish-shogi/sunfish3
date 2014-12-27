/*
 * RemainingTime.h
 */

#ifndef __SUNFISH_REMAININGTIME__
#define __SUNFISH_REMAININGTIME__

#include <string>
#include <sstream>

namespace sunfish {
	class RemainingTime {
	private:
		int _total;
		int _remain;
		int _readoff;

	public:
		RemainingTime() {
		}

		RemainingTime(int total, int readoff)
				: _total(total), _readoff(readoff) {
			reset();
		}

		void init(int total, int readoff = 0) {
			_total = total;
			_readoff = readoff;
			reset();
		}

		void reset() {
			_remain = _total;
		}

		int use(int sec) {
			_remain = _remain > sec ? _remain - sec : 0;
			return _remain;
		}

		int usable() const {
			return _remain + _readoff;
		}

		int isLimited() const {
			return _total != 0 || _readoff != 0;
		}

		int getTotal() const {
			return _total;
		}

		int getRemain() const {
			return _remain;
		}

		int getReadoff() const {
			return _readoff;
		}

		std::string toString() const {
			std::ostringstream oss;
			oss << _remain << '/' << _total << ' ' << _readoff;
			return oss.str();
		}

	};
}

#endif // __SUNFISH_REMAININGTIME__
