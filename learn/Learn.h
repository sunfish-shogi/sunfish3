/* Learn.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_LEARN__
#define __SUNFISH_LEARN__

#ifndef NLEARN

#include "config/Config.h"
#include "core/board/Board.h"
#include "core/move/Move.h"
#include "searcher/Searcher.h"
#include <random>
#include <cstring>

namespace sunfish {

class FV : public Feature<float> {
public:
  void init() {
    memset(_t, 0, sizeof(*_t));
  }
};

class Learn {
private:

  std::mt19937 _rgen;

  Config _config;

  Searcher _searcher;

  uint32_t _count;

  uint32_t _miniBatchCount;

  FV _g;

  FV _w;

  FV _u;

  /**
   * 特徴抽出を行いパラメータを更新します。
   */
  bool adjust(Board, Move);

  /**
   * 棋譜ファイルを読み込んで学習します。
   */
  bool readCsa(const char* path);

public:

  /**
   * コンストラクタ
   */
  Learn();

  /**
   * 機械学習を実行します。
   */
  bool run();

};

}

#endif // NLEARN

#endif // __SUNFISH_LEARN__
