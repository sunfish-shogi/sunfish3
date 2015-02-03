/* Tree.cpp
 * 
 * Kubo Ryosuke
 */

#include "Tree.h"

namespace sunfish {

	void Tree::sort(const Moves::iterator begin) {
		auto& moves = _stack[_ply].moves;
		auto& values = _sortValues;
		auto beginIndex = begin - moves.begin();
		auto endIndex = moves.size();

		assert(beginIndex >= 0);
		assert(endIndex >= beginIndex);

		values[endIndex] = INT_MIN;
		for (int i = (int)endIndex - 2; i >= beginIndex; i--) {
			auto tmove = moves[i];
			auto tvalue = _sortValues[i];
			int j = i + 1;
    	for (; _sortValues[j] > tvalue; j++) {
				moves[j-1] = moves[j];
				values[j-1] = values[j];
    	}
			moves[j-1] = tmove;
			values[j-1] = tvalue;
		}
	}

}
