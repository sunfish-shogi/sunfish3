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
#include "core/util/Timer.h"
#include "searcher/Searcher.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <random>
#include <queue>
#include <vector>
#include <thread>
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

  struct Job {
    Board board;
    Move move;
  };

  Timer _timer;

  Config _config;

  Evaluator _eval;

  std::vector<std::mt19937> _rgens;

  std::vector<std::unique_ptr<Searcher>> _searchers;

  uint32_t _count;

  uint32_t _miniBatchCount;

  FV _g;

  FV _w;

  FV _u;

  std::queue<Job> _jobQueue;

  std::vector<std::thread> _threads;

  std::atomic<bool> _shutdown;

	std::atomic<int> _activeCount;

  std::mutex _mutex;

  void genGradient(int wn, Board, Move);

  void work(int wn);

  bool putJob(Board, Move);

  /**
   * 棋譜ファイルを読み込んで学習します。
   */
  bool readCsa(size_t count, size_t total, const char* path);

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
