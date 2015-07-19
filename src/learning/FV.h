/* FV.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_FV__
#define SUNFISH_FV__


#include "searcher/eval/Evaluator.h"

namespace sunfish {

class FV : public Feature<float> {
public:
  void init() {
    memset(t_, 0, sizeof(*t_));
  }
};

} // namespace sunfish

#endif // SUNFISH_FV__
