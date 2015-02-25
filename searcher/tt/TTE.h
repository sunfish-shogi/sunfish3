/* TTE.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TTE__
#define __SUNFISH_TTE__

#include "TTMoves.h"
#include "../eval/Value.h"
#include "../tree/NodeStat.h"
#include "core/def.h"
#include <cassert>

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
		static CONSTEXPR uint32_t AgeMax = 0x01 << 8;

		enum ValueType : int {
			Exact = 0,
			Upper, /* = 1 */
			Lower, /* = 2 */
			None, /* = 3 */
		};

	private:

		uint64_t _hash;
		uint32_t _checkSum;
		Value _value;
		TTMoves _moves;
		struct {
			uint32_t age : 8;
			uint32_t depth : 18;
			uint32_t valueType : 2;
			uint32_t stat : 4;
		} _;

		uint32_t generateCheckSum() const {
			return (uint32_t)_hash ^ (uint32_t)(_hash >> 32)
					^ (uint32_t)(int32_t)_value
					^ (uint32_t)_moves.getMove1()
					^ (uint32_t)_moves.getMove2()
					^ (uint32_t)_.age
					^ ((uint32_t)_.depth << 8)
					^ ((uint32_t)_.valueType << 26)
					^ ((uint32_t)_.stat << 28)
					;
		}

		bool update(uint64_t newHash,
				Value newValue,
				ValueType newValueType,
				int newDepth, int ply,
				const NodeStat& newStat,
				const Move& move,
				uint32_t newAge);

	public:
		TTE() {
			init();
		}

		void init() {
			_checkSum = generateCheckSum() + 1;
		}

		bool update(uint64_t newHash,
				Value alpha,
				Value beta,
				Value newValue,
				int newDepth, int ply,
				const NodeStat& newStat,
				const Move& move,
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
					newDepth, ply, newStat, move, newAge);

		}

		void updatePv(uint64_t newHash, int newDepth, const Move& move, uint32_t newAge);

		bool isOk() const {
			return _checkSum == generateCheckSum();
		}

		bool isBroken() const {
			return !isOk();
		}

		bool is(uint64_t _hash) const {
			return this->_hash == _hash && isOk();
		}

		bool isSuperior(int curDepth) const {
			if ((int)_.depth >= curDepth) {
				return true;
			}
			if (_value >= Value::Mate && (_.valueType == Lower || _.valueType == Exact)) {
				return true;
			}
			if (_value <= -Value::Mate && (_.valueType == Upper || _.valueType == Exact)) {
				return true;                                    
			}
			return false;
		}

		uint64_t getHash() const {
			return _hash;
		}

		Value getValue(int ply) const {
			if (_value >= Value::Mate) {
				if (_.valueType == Lower) { return _value - ply; }
			} else if (_value <= -Value::Mate) {
				if (_.valueType == Upper) { return _value + ply; }
			}
			return _value;
		}

		uint32_t getValueType() const {
			return _.valueType;
		}

		int getDepth() const {
			return (int)_.depth;
		}

		const NodeStat getStat() const {
			return NodeStat(_.stat);
		}

		const TTMoves getMoves() const {
			return _moves;
		}

		uint32_t getAge() const {
			return _.age;
		}

	};

	class TTEs {
	private:

		static CONSTEXPR uint32_t Size = 4;
		TTE _list[Size];
		volatile uint32_t _lastAccess;

	public:

		TTEs() : _lastAccess(0) {
		}

		void init(uint32_t) {
			for (uint32_t i = 0; i < Size; i++) {
				_list[i].init();
			}
		}

		TTStatus set(const TTE& entity);
		bool get(uint64_t hash, TTE& entity);

	};
}

#endif // __SUNFISH_TTE__
