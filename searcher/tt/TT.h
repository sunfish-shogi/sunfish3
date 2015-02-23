/* TT.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TT__
#define __SUNFISH_TT__

#include "TTE.h"
#include "../table/HashTable.h"

namespace sunfish {

	class TT : public HashTable<TTEs> {
	private:

		uint32_t _age;

	public:

		TT() : HashTable<TTEs>() {}
		TT(const TT&) = delete;
		TT(TT&&) = delete;

		void evolve() {
			_age = (_age + 1) % TTE::AgeMax;
		}

		bool entry(uint64_t hash,
				Value alpha, Value beta, Value value,
				int depth, int ply,
				const NodeStat& stat, const Move& move) {
			TTE e;
			TTEs& entities = getEntity(hash);
			entities.get(hash, e);
			if (e.update(hash, alpha, beta, value, depth, ply, stat, move, _age)) {
				entities.set(e);
				return true;
			}
			return false;
		}

		void entryPv(uint64_t hash, int depth, const Move& move) {
			TTE e;
			TTEs& entities = getEntity(hash);
			entities.get(hash, e);
			e.updatePv(hash, depth, move, _age);
			entities.set(e);
		}

		bool get(uint64_t hash, TTE& e) {
			return getEntity(hash).get(hash, e) && e.is(hash);
		}

	};

}

#endif // __SUNFISH_TT__
