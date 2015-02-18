/* EvaluateTable.h
 *
 *  Created on: 2012/07/09
 *      Author: ryosuke
 */

#ifndef __SUNFISH_EVALUATETABLE__
#define __SUNFISH_EVALUATETABLE__

#include "EvaluateEntity.h"
#include "../table/HashTable.h"

namespace sunfish {

	class EvaluateTable : public HashTable<EvaluateEntity> {
	public:
		static const int Bits = 21; // TODO: fix magic number

		EvaluateTable() : HashTable<EvaluateEntity>(Bits) {
		}
		EvaluateTable(const EvaluateTable&) = delete;
		EvaluateTable(EvaluateTable&&) = delete;

		bool get(uint64_t hash, Value& value) const {
			return getEntity(hash).get(hash, value);
		}

		void set(uint64_t hash, const Value& value) {
			_getEntity(hash).set(hash, value);
		}
	};

}

#endif // __SUNFISH_EVALUATETABLE__
