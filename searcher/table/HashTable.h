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

		uint32_t _size;
		uint32_t _mask;
		E* _table;

	protected:

		E& getEntity(uint64_t hash) {
			return _table[hash&_mask];
		}

		E& getEntity(uint32_t index) {
			return _table[index];
		}

		const E& getEntity(uint64_t hash) const {
			return _table[hash&_mask];
		}

		const E& getEntity(uint32_t index) const {
			return _table[index];
		}

	public:

		static CONSTEXPR uint32_t DefaultBits = 18;

		HashTable(uint32_t bits = DefaultBits) : _size(0), _table(nullptr) {
			init(bits);
		}
		HashTable(const HashTable&) = delete;
		HashTable(HashTable&&) = delete;

		virtual ~HashTable() {
			delete [] _table;
		}

		void init(uint32_t bits = 0) {
			uint32_t newSize = 1 << bits;
			if (bits != 0 && _size != newSize) {
				_size = newSize;
				_mask = _size - 1;
				if (_table != nullptr) {
					delete[] _table;
				}
				_table = new E[_size];
			} else {
				for (uint32_t i = 0; i < _size; i++) {
					_table[i].init(i);
				}
			}
		}

		uint32_t getSize() const {
			return _size;
		}
	};

}

#endif // __SUNFISH_HASHTABLE__
