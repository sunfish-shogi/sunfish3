/* EvaluateEntity.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_EVALUATEENTITY__
#define __SUNFISH_EVALUATEENTITY__

#include "Value.h"
#include "core/def.h"
#include <cassert>

namespace sunfish {

	class EvaluateEntity {
	public:
		static CONSTEXPR int KeyLength = 18;

	private:
		static CONSTEXPR int ValueInf = 1U << (KeyLength - 1);
		static CONSTEXPR uint64_t ValueMask = (uint64_t)((1U << KeyLength) - 1U);
		static CONSTEXPR uint64_t HashMask = ~ValueMask;

		uint64_t _data;

		static int32_t convertValue(int32_t value) {
			return ValueInf - value;
		}

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
				value = convertValue((int32_t)(temp & ValueMask));
				assert(value > -ValueInf);
				assert(value < ValueInf);
				return true;
			}
			return false;
		}

		void set(uint64_t hash, const Value& value) {
			assert(value > -ValueInf);
			assert(value < ValueInf);
			uint32_t v = (uint32_t)convertValue(value.int32());
			assert((v & ~ValueMask) == 0U);
			uint64_t temp = (hash & HashMask) | v;
			_data = temp;
		}
	};

}

#endif // __SUNFISH_EVALUATEENTITY__
