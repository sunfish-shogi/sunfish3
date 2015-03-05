/* TTE.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TTE__
#define __SUNFISH_TTE__

#include "../eval/Value.h"
#include "core/def.h"
#include <cassert>

#define TT_INDEX_WIDTH 20

// 1st word
#define TT_HASH_WIDTH  54
#define TT_AGE_WIDTH   3  // 2^3 = 8 [0, 7]
#define TT_DEPTH_WIDTH 7  // 2^7 = 128 [0, 127]

// 2nd word
#define TT_VALUE_WIDTH 16 // 2^16
#define TT_VTYPE_WIDTH 2  // 2^2 = 4 [0, 3]
#define TT_MOVE1_WIDTH 16
#define TT_MOVE2_WIDTH 16

#define TT_VALUE_OFFSET ((1<<TT_VALUE_WIDTH)/2)
#define TT_ENC_VALUE(value) ((value.int32()) + TT_VALUE_OFFSET)
#define TT_DEC_VALUE(value) ((int32_t)(value) - TT_VALUE_OFFSET)
#define TT_ENC_HASH(hash) ((hash) >> (64 - TT_HASH_WIDTH))

static_assert((TT_INDEX_WIDTH > 64 - TT_HASH_WIDTH), "invalid hash length");

namespace sunfish {

	enum class TTStatus : int {
		None,
		Reject,
		New,
		Update,
		Collide,
	};

	class TTE {
	public:
		static CONSTEXPR uint32_t InvalidAge = 0x00;
		static CONSTEXPR uint32_t AgeMax = 0x01 << TT_AGE_WIDTH;

		enum ValueType : int {
			Exact = 0,
			Upper, /* = 1 */
			Lower, /* = 2 */
			None, /* = 3 */
		};

	private:

		struct {
			uint64_t hash : TT_HASH_WIDTH;
			uint32_t age : TT_AGE_WIDTH;
			uint32_t depth : TT_DEPTH_WIDTH;
			uint32_t value : TT_VALUE_WIDTH;
			uint32_t valueType : TT_VTYPE_WIDTH;
			uint16_t move1 : TT_MOVE1_WIDTH;
			uint16_t move2 : TT_MOVE2_WIDTH;
		} _;

		bool update(uint64_t newHash,
				Value newValue,
				ValueType newValueType,
				int newDepth, int ply,
				uint16_t move,
				uint32_t newAge);

	public:
		TTE() {
			init();
		}

		void init() {
			_.age = InvalidAge;
		}

		bool update(uint64_t newHash,
				Value alpha,
				Value beta,
				Value newValue,
				int newDepth, int ply,
				uint16_t move,
				uint32_t newAge) {

			ValueType newValueType;
			if (newValue >= beta) {
				newValueType = Lower;
			} else if (newValue <= alpha) {
				newValueType = Upper;
			} else {
				newValueType = Exact;
			}

			return update(newHash, newValue, newValueType,
					newDepth, ply, move, newAge);

		}

		void updatePv(uint64_t newHash, int newDepth, uint16_t move, uint32_t newAge);

		bool checkHash(uint64_t hash) const {
			return _.hash == TT_ENC_HASH(hash);
		}

		uint64_t getHash() const {
			return _.hash;
		}

		Value getValue(int ply) const {
			Value value = TT_DEC_VALUE(_.value);
			assert(value >= -Value::Inf);
			assert(value <= Value::Inf);
			if (value >= Value::Mate) {
				if (_.valueType == Lower) { return value - ply; }
			} else if (value <= -Value::Mate) {
				if (_.valueType == Upper) { return value + ply; }
			}
			return value;
		}

		uint32_t getValueType() const {
			return _.valueType;
		}

		int getDepth() const {
			return (int)_.depth;
		}

		uint16_t getMove1() const {
			return _.move1;
		}

		uint16_t getMove2() const {
			return _.move2;
		}

		uint32_t getAge() const {
			return _.age;
		}

	};

	class TTEs {
	private:

		static CONSTEXPR uint32_t Size = 4;
		TTE _slots[Size];
		volatile uint32_t _lastAccess;

	public:

		TTEs() : _lastAccess(0) {
		}

		void init(uint32_t) {
			for (uint32_t i = 0; i < Size; i++) {
				_slots[i].init();
			}
		}

		TTStatus set(const TTE& entity);
		bool get(uint64_t hash, TTE& entity);

	};
}

#endif // __SUNFISH_TTE__
