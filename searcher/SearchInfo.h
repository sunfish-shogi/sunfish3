/* SearchInfo.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SEARCHINFO__
#define SUNFISH_SEARCHINFO__

#include "eval/Value.h"
#include "core/move/Move.h"
#include "tree/PV.h"
#include <cstdint>

namespace sunfish {

struct SearchInfoBase {
  uint64_t failHigh;
  uint64_t failHighFirst;
  uint64_t failHighIsHash;
  uint64_t failHighIsKiller1;
  uint64_t failHighIsKiller2;
  uint64_t hashProbed;
  uint64_t hashHit;
  uint64_t hashExact;
  uint64_t hashLower;
  uint64_t hashUpper;
  uint64_t hashStore;
  uint64_t hashNew;
  uint64_t hashUpdate;
  uint64_t hashCollision;
  uint64_t hashReject;
  uint64_t mateProbed;
  uint64_t mateHit;
  uint64_t expand;
  uint64_t expandHashMove;
  uint64_t shekProbed;
  uint64_t shekSuperior;
  uint64_t shekInferior;
  uint64_t shekEqual;
  uint64_t nullMovePruning;
  uint64_t nullMovePruningTried;
  uint64_t futilityPruning;
  uint64_t extendedFutilityPruning;
  uint64_t moveCountPruning;
  uint64_t razoring;
  uint64_t razoringTried;
  uint64_t probcut;
  uint64_t probcutTried;
  uint64_t singular;
  uint64_t singularChecked;
  uint64_t expanded;
  uint64_t checkExtension;
  uint64_t onerepExtension;
  uint64_t recapExtension;
  uint64_t split;
  uint64_t node;
  uint64_t qnode;
};

struct SearchInfo : public SearchInfoBase {
  float time;
  float nps;
  Move move;
  Value eval;
  int32_t lastDepth;
  PV pv;
};

} // namespace sunfish

#endif // SUNFISH_SEARCHINFO__
