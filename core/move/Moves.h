/* Moves.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_MOVES__
#define __SUNFISH_MOVES__

#include "../board/Board.h"
#include "Moves.h"
#include <algorithm>

namespace sunfish {

	template <int maxSize>
	class TempMoves {
	private:

		Move _moves[maxSize];
		int _size;

	public:

		typedef Move* iterator;
		typedef const Move* const_iterator;

		TempMoves() : _size(0) {
		}

		void clear() { _size = 0; }
		int size() const { return _size; }

		void add(const Move& move) {
			_moves[_size++] = move;
		}

		void remove(int index) {
			_moves[index] = _moves[--_size];
		}
		iterator remove(iterator ite) {
			(*ite) = _moves[--_size];
			return ite;
		}
		void removeStable(int index) {
			removeStable(begin()+index);
		}
		iterator removeStable(iterator ite) {
			for (auto itmp = ite+1; itmp != end(); itmp++) {
				*(itmp-1) = *(itmp);
			}
			_size--;
			return ite;
		}
		void removeAfter(int index) {
			_size = index;
		}
		void removeAfter(iterator ite) {
			_size = (int)(ite - _moves);
		}

		// random accessor
		Move& get(int index) { return _moves[index]; }
		const Move& get(int index) const { return _moves[index]; }
		Move& operator[](int index) { return _moves[index]; }
		const Move& operator[](int index) const { return _moves[index]; }

		// iterator
		iterator begin() { return _moves; }
		const_iterator begin() const { return _moves; }
		iterator end() { return _moves + _size; }
		const_iterator end() const { return _moves + _size; }

		iterator find(const Move& move) {
			for (auto ite = begin(); ite != end(); ite++) {
				if (ite->equals(move)) {
					return ite;
				}
			}
			return end();
		}

	};

	typedef TempMoves<1024> Moves;

}

#endif //__SUNFISH_MOVES__
