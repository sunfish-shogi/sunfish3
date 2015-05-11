/* TimeManager.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TIMEMANAGER__
#define __SUNFISH_TIMEMANAGER__

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

  int _depth;
  Data _stack[Tree::StackSize];

public:

  void init();
  void nextDepth();
  void startDepth();
  void addMove(Move move, Value value);
  bool isEasy(double limit, double elapsed);

};

} // namespace sunfish

#endif // __SUNFISH_TIMEMANAGER__
