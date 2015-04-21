/* SeeTable.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_SEETABLE__
#define __SUNFISH_SEETABLE__

#include "SeeEntity.h"
#include "../table/HashTable.h"

namespace sunfish {

	template <int KeyLength>
	class SeeTable : public HashTable<SeeEntity<KeyLength>> {
	public:
		using BaseType = HashTable<SeeEntity<KeyLength>>;

		SeeTable() : BaseType(KeyLength) {
		}
		SeeTable(const SeeTable&) = delete;
		SeeTable(SeeTable&&) = delete;

		bool get(uint64_t hash, Value& value, Value alpha, Value beta) const {
			return BaseType::getEntity(hash).get(hash, value, alpha, beta);
		}

		void set(uint64_t hash, const Value& value, Value alpha, Value beta) {
			BaseType::getEntity(hash).set(hash, value, alpha, beta);
		}
	};

}

#endif // __SUNFISH_SEETABLE__
