/* Random.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_RANDOM__
#define __SUNFISH_RANDOM__

#include "../def.h"
#include <random>
#include <climits>

namespace sunfish {

	class Random {
	private:
		std::mt19937 rgen;
		std::uniform_int_distribution<unsigned> dstBit;
		std::uniform_int_distribution<uint16_t> dst16;
		std::uniform_int_distribution<uint32_t> dst32;
		std::uniform_int_distribution<uint64_t> dst64;

	public:
		Random() : rgen(static_cast<unsigned>(time(NULL))), dstBit(0, 1) {
		}

		uint16_t getInt16() {
			return dst16(rgen);
		}

		uint16_t getInt16(uint16_t num) {
			return getInt16() % num;
		}

		uint32_t getInt32() {
			return dst32(rgen);
		}

		uint32_t getInt32(uint32_t num) {
			return getInt32() % num;
		}

		uint64_t getInt64() {
			return dst64(rgen);
		}

		uint64_t getInt64(uint64_t num) {
			return getInt64() % num;
		}

		unsigned getBit() {
			return dstBit(rgen);
		}

		template <class T>
		void shuffle(T array[], int size) {
			for (int i = size - 1; i > 0; i--) {
				int r = getInt32(i+1);
				if (r != i) {
					T tmp = array[i];
					array[i] = array[r];
					array[r] = tmp;
				}
			}
		}
	};
}

#endif //__SUNFISH_RANDOM__
