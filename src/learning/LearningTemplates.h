/* LearningTemplates.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_LEARNINGTEMPLATES__
#define SUNFISH_LEARNINGTEMPLATES__

#ifndef NLEARN

#include "searcher/eval/Evaluator.h"

namespace sunfish {

class LearningTemplates {
private:

  LearningTemplates();

public:

  template <class Type, class Func>
  static void symmetrize(Feature<Type>& fv, Func&& f) {
    // king-king-piece
    SQUARE_EACH(bking0) {
      Square bking1 = bking0.sym();
      if (bking0.index() > bking1.index()) {
        continue;
      }

      SQUARE_EACH(wking0) {
        if (bking0.index() == wking0.index()) {
          continue;
        }
        Square wking1 = wking0.sym();
        if (bking0.index() == bking1.index() && wking0.index() > wking1.index()) {
          continue;
        }

        for (int index0 = 0; index0 < KKP_MAX; index0++) {
          int index1 = symmetrizeKkpIndex(index0);
          if (bking0.index() == bking1.index() && wking0.index() > wking1.index() && index0 >= index1) {
            continue;
          }
          f(fv.t_->kkp[bking0.index()][wking0.index()][index0], fv.t_->kkp[bking1.index()][wking1.index()][index1]);
        }
      }
    }
  }

};

} // namespace sunfish

#endif // NLEARN

#endif // SUNFISH_LEARNINGTEMPLATES__
