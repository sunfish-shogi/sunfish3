/* TTE.cpp
 *
 * Kubo Ryosuke
 */

#include "TTE.h"
#include "core/move/Move.h"

namespace sunfish {

	bool TTE::update(uint64_t newHash,
			Value newValue,
			ValueType newValueType,
			int newDepth, int ply,
			uint16_t move,
			uint32_t newAge,
			const NodeStat& stat) {

		assert(newAge < AgeMax);
		assert(newValueType < (ValueType)(1<<TT_VTYPE_WIDTH));

		if (newDepth < 0) {
			newDepth = 0;
		} else if (newDepth > (1<<TT_DEPTH_WIDTH)-1) {
			newDepth = (1<<TT_DEPTH_WIDTH)-1;
		}

		if (checkHash(newHash)) {
			// 深さが劣るものは登録させない。
			if (newDepth < (int)_2.depth && _1.age == newAge &&
					newValue < Value::Mate && newValue > -Value::Mate) {
				return false;
			}
		} else {
			_1.hash = TT_ENC_HASH(newHash);
			_2.move = Move::S16_EMPTY;
		}

		if (newValue >= Value::Mate) {
			if (newValueType == Lower) {
				if (newValue < Value::Inf - ply) {
					newValue += ply;
				} else {
					newValue = Value::Inf;
				}
			}
		} else if (newValue <= -Value::Mate) {
			if (newValueType == Upper) {
				if (newValue > -Value::Inf + ply) {
					newValue -= ply;
				} else {
					newValue = -Value::Inf;
				}
			}
		}

		assert(newValue >= -Value::Inf);
		assert(newValue <= Value::Inf);
		int32_t value = TT_ENC_VALUE(newValue);
		assert(value >= 0);
		assert(value < (1<<TT_VALUE_WIDTH));
		_2.value = value;
		_2.valueType = newValueType;
		_2.depth = (uint32_t)newDepth;
		if (move != Move::S16_EMPTY) {
			_2.move = move;
		}
		_1.mateThreat = stat.isMateThreat();
		_1.age = newAge;

		return true;

	}

	void TTE::updatePv(uint64_t newHash, int newDepth, uint16_t move, uint32_t newAge) {
		if (newDepth < 0) {
			newDepth = 0;
		}

		if (checkHash(newHash)) {
			if (newDepth >= (int)_2.depth || _1.age != newAge) {
				_2.valueType = None;
				_2.depth = (uint32_t)newDepth;
			}
		} else {
			_1.hash = TT_ENC_HASH(newHash);
			_2.move = Move::S16_EMPTY;
			_2.valueType = None;
			_2.depth = (uint32_t)newDepth;
		}

		if (move != Move::S16_EMPTY) {
			_2.move = move;
		}
		_1.mateThreat = false;
		_1.age = newAge;
	}

	TTStatus TTEs::set(const TTE& entity) {
		// ハッシュ値が一致するスロットを探す
		uint32_t l = _lastAccess % Size;
		for (uint32_t i = 0; i < Size; i++) {
			const uint32_t index = (l + i) % Size;
			if (_slots[index].getHash() == entity.getHash()) {
				_slots[index] = entity;
				_lastAccess = index;
				return TTStatus::Update;
			}
		}

		// 空きスロットを探す
		l++;
		for (uint32_t i = 0; i < Size; i++) {
			const uint32_t index = (l + i) % Size;
			if (_slots[index].getAge() != entity.getAge()) {
				_slots[index] = entity;
				_lastAccess = index;
				return TTStatus::New;
			}
		}

		// 上書きする
		const uint32_t index = l % Size;
		_slots[index] = entity;
		_lastAccess = index;
		return TTStatus::Collide;

	}

	bool TTEs::get(uint64_t hash, TTE& entity) {

		uint32_t l = _lastAccess % Size;
		for (uint32_t i = 0; i < Size; i++) {
			const uint32_t index = (l + i) % Size;
			if (_slots[index].checkHash(hash)) {
				entity = _slots[index];
				_lastAccess = index;
				return true;
			}
		}
		return false;

	}
}
