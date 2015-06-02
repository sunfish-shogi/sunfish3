/* TT.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_TT__
#define SUNFISH_TT__

#include "TTE.h"
#include "../table/HashTable.h"

namespace sunfish {

class TT : public HashTable<TTEs> {
private:

  uint32_t age_;

public:

  TT() : HashTable<TTEs>(TT_INDEX_WIDTH), age_(1) {}
  TT(const TT&) = delete;
  TT(TT&&) = delete;

  void evolve() {
    age_ = age_ % (TTE::AgeMax-1) + 1;
    assert(age_ != TTE::InvalidAge);
  }

  TTStatus entry(uint64_t hash,
      Value alpha, Value beta, Value value,
      int depth, int ply, uint16_t move,
      const NodeStat& stat) {
    TTE e;
    TTEs& entities = getEntity(hash);
    entities.get(hash, e);
    if (e.update(hash, alpha, beta, value, depth, ply, move, age_, stat)) {
      return entities.set(e);
    }
    return TTStatus::Reject;
  }

  TTStatus entryPV(uint64_t hash, int depth, uint16_t move) {
    TTE e;
    TTEs& entities = getEntity(hash);
    entities.get(hash, e);
    e.updatePV(hash, depth, move, age_);
    return entities.set(e);
  }

  bool get(uint64_t hash, TTE& e) {
    return getEntity(hash).get(hash, e) && e.checkHash(hash);
  }

};

} // namespace sunfish

#endif // SUNFISH_TT__
