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

		TT() : HashTable<TTEs>(TT_INDEX_WIDTH), _age(1) {}
		TT(const TT&) = delete;
		TT(TT&&) = delete;

		void evolve() {
			_age = _age % (TTE::AgeMax-1) + 1;
			assert(_age != TTE::InvalidAge);
		}

		TTStatus entry(uint64_t hash,
				Value alpha, Value beta, Value value,
				int depth, int ply, uint16_t move) {
			TTE e;
			TTEs& entities = getEntity(hash);
			entities.get(hash, e);
			if (e.update(hash, alpha, beta, value, depth, ply, move, _age)) {
				return entities.set(e);
			}
			return TTStatus::Reject;
		}

		TTStatus entryPv(uint64_t hash, int depth, uint16_t move) {
			TTE e;
			TTEs& entities = getEntity(hash);
			entities.get(hash, e);
			e.updatePv(hash, depth, move, _age);
			return entities.set(e);
		}

		bool get(uint64_t hash, TTE& e) {
			return getEntity(hash).get(hash, e) && e.checkHash(hash);
		}

	};

}

#endif // __SUNFISH_TT__
