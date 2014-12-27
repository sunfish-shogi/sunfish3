/* Pv.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_PV__
#define __SUNFISH_PV__

#include "core/move/Move.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace sunfish {

	class Pv {
	private:

		static const int MaxDepth = 64;

		Move _moves[MaxDepth];
		int _num;

	public:
		Pv() {
			init();
		}

		Pv(const Pv& pv) {
			copy(pv);
		}

		void copy(const Pv& pv) {
			_num = pv._num;
			memcpy(_moves, pv._moves, sizeof(Move) * _num);
		}

		void init() {
			_num = 0;
		}

		int size() const {
			return _num;
		}

		int set(const Move& move, const Pv& pv) {
			_moves[0] = move;
			_num = std::min(pv._num + 1, int(MaxDepth));
			memcpy(&_moves[1], pv._moves, sizeof(Move) * (_num - 1));
			return _num;
		}

		int set(const Move& move) {
			_moves[0] = move;
			_num = 1;
			return _num;
		}

		const Move* getTop() const {
			if (_num > 0) {
				return &_moves[0];
			} else {
				return nullptr;
			}
		}

		const Move& get(int depth) const {
			return _moves[depth];
		}

		std::string toString(int beginIndex = 0) const {
			std::ostringstream oss;
			for (int i = beginIndex; i < _num; i++) {
				oss << _moves[i].toString() << ' ';
			}
			return oss.str();
		}

		std::string toStringCsa(bool black, int beginIndex = 0) const {
			std::ostringstream oss;
			for (int i = beginIndex; i < _num; i++) {
				oss << _moves[i].toStringCsa((i%2)^black) << ' ';
			}
			return oss.str();
		}

	};

}

#endif // __SUNFISH_PV__
