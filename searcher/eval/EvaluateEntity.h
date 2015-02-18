/* EvaluateEntity.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_EVALUATEENTITY__
#define __SUNFISH_EVALUATEENTITY__

#include "Value.h"

namespace sunfish {

	class EvaluateEntity {
	private:

		static const int ValueInfl = 1U << (21 - 1); // TODO: fix magic number
		static const uint64_t ValueMask = (uint64_t)((1U << 21) - 1U); // TODO: fix magic number
		static const uint64_t HashMask = ~ValueMask;

		uint64_t _data;

	public:

		EvaluateEntity() {
			init();
		}

		void init() {
			_data = 0ull;
		}

		void init(unsigned) {
			init();
		}

		bool get(uint64_t hash, Value& value) const {
			uint64_t temp = _data;
			if ((temp & HashMask) == (hash & HashMask)) {
				value = (int)(temp & ValueMask) - ValueInfl;
				return true;
			}
			return false;
		}

		void set(uint64_t hash, const Value& value) {
			uint64_t temp = (hash & HashMask) | ((value.int32() + ValueInfl) & ValueMask);
			_data = temp;
		}
	};

}

#endif // __SUNFISH_EVALUATEENTITY__
