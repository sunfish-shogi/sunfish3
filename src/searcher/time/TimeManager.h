/* TimeManager.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_TIMEMANAGER__
#define SUNFISH_TIMEMANAGER__

#include "../eval/Value.h"
#include "../tree/Tree.h"
#include "core/move/Move.h"

namespace sunfish {

class TimeManager {
private:

  struct Data {
    Move firstMove;
    Value firstValue;
  };

  int depth_;
  Data stack_[Tree::StackSize];

public:

  void init();
  void nextDepth();
  void startDepth();
  void addMove(Move move, Value value);
  bool isEasy(float limit, float elapsed);

};

} // namespace sunfish

#endif // SUNFISH_TIMEMANAGER__
