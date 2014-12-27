/* HashTable.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_HASHTABLE__
#define __SUNFISH_HASHTABLE__

#include "core/def.h"

namespace sunfish {

	/**
	 * base class for hash table
	 */
	template <class E> class HashTable {
	private:

		unsigned _size;
		unsigned _mask;
		E* _table;

	protected:

		E& _getEntity(uint64_t hash) {
			return _table[hash&_mask];
		}

		E& _getEntity(unsigned index) {
			return _table[index];
		}

	public:

		static const unsigned DefaultBits = 21;

		HashTable(unsigned bits = DefaultBits) : _size(0), _table(nullptr) {
			init(bits);
		}

		virtual ~HashTable() {
			delete [] _table;
		}

		void init(unsigned bits = 0) {
			unsigned newSize = 1 << bits;
			if (bits != 0 && _size != newSize) {
				_size = newSize;
				_mask = _size - 1;
				if (_table != nullptr) {
					delete[] _table;
				}
				_table = new E[_size];
			} else {
				for (unsigned i = 0; i < _size; i++) {
					_table[i].init(i);
				}
			}
		}

		const E& getEntity(uint64_t hash) const {
			return _table[hash&_mask];
		}

		const E& getEntity(unsigned index) const {
			return _table[index];
		}

		unsigned getSize() const {
			return _size;
		}
	};

}

#endif // __SUNFISH_HASHTABLE__
