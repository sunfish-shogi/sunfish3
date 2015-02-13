/* TTE.cpp
 *
 * Kubo Ryosuke
 */

#include "TTE.h"

namespace sunfish {

	bool TTE::update(uint64_t newHash,
			Value newValue,
			int newValueType,
			int newDepth, int ply,
			const NodeStat& newStat,
			const Move& move,
			uint32_t newAge) {

		assert(newAge < AgeMax);
		assert(newDepth < (1<<20));
		assert(newValueType < (1<<2));
		assert((uint32_t)newStat < (1<<4));

		if (newDepth < 0) {
			newDepth = 0;
		}

		if (isOk()) {
			assert(_hash == newHash);
			// 深さが劣るものは登録させない。
			if (newDepth < (int)_.depth && _.age == newAge) {
				return false;
			}
		} else {
			_hash = newHash;
			_moves.init();
		}

		if (_value >= Value::Mate) {
			if (newValueType == Lower) {
				if (_value < Value::Inf - ply) {
					_value += ply;
				} else {
					_value = Value::Inf;
				}
			}
		} else if (_value <= -Value::Mate) {
			if (newValueType == Upper) {
				if (_value > -Value::Inf + ply) {
					_value -= ply;
				} else {
					_value = -Value::Inf;
				}
			}
		}

		_value = newValue;
		_.valueType = newValueType;
		_.depth = (uint32_t)newDepth;
		_.stat = (uint32_t)newStat;
		if (!move.isEmpty()) {
			_moves.update(move);
		}
		_.age = newAge;
		_checkSum = generateCheckSum();

		return true;

	}

	void TTEs::set(const TTE& entity) {

		uint32_t l = lastAccess;
		for (uint32_t i = 0; i < Size; i++) {
			const uint32_t index = (l + i) % Size;
			if (list[index].getHash() == entity.getHash()) {
				list[index] = entity;
				lastAccess = index;
				return;
			}
		}
		l++;
		for (uint32_t i = 0; i < Size; i++) {
			const uint32_t index = (l + i) % Size;
			if (list[index].isBroken() ||
					list[index].getAge() != entity.getAge()) {
				list[index] = entity;
				lastAccess = index;
				return;
			}
		}
		const uint32_t index = l % Size;
		list[index] = entity;
		lastAccess = index;

	}

	bool TTEs::get(uint64_t hash, TTE& entity) {

		uint32_t l = lastAccess;
		for (uint32_t i = 0; i < Size; i++) {
			const uint32_t index = (l + i) % Size;
			if (list[index].getHash() == hash) {
				entity = list[index];
				lastAccess = index;
				return true;
			}
		}
		return false;

	}
}
