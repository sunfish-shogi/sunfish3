/* BatchLearning.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_BATCHLEARNING__
#define SUNFISH_BATCHLEARNING__

#ifndef NLEARN

#include "./FV.h"
#include "core/util/Timer.h"
#include "core/util/Random.h"
#include "searcher/Searcher.h"
#include <fstream>
#include <queue>
#include <vector>
#include <thread>
#include <string>
#include <atomic>
#include <mutex>
#include <functional>
#include <utility>
#include <cstdint>

namespace sunfish {

class Config;
class Board;
class Move;

class BatchLearning {
private:

  enum class JobType {
    GenerateTrainingData,
    GenerateGradient,
    OverlapParam,
    UpdateParam,
  };

  static CONSTEXPR_CONST uint32_t InvalidWorkerNumber = -1;

  struct Job {
    JobType type;
    std::function<void(int)> method;
    uint32_t wn;
  };

  Timer timer_;

  const Config& config_;

  Evaluator eval_;

  FVM gm_;

  FV g_;

  std::queue<Job> jobQueue_;

  std::atomic<uint32_t> completedJobs_;

  uint32_t totalJobs_;

  std::atomic<uint32_t> totalMoves_;

  std::atomic<uint32_t> outOfWindLoss_;

  float loss_;

  Evaluator::ValueType max_;

  uint64_t magnitude_;

  struct ThreadObject {
    std::thread thread;
    std::unique_ptr<Searcher> searcher;
    std::unique_ptr<Random> rand;
    std::unique_ptr<std::ofstream> outTrainingData;
  };

  std::vector<ThreadObject> threadObjects_;

  uint32_t nt_;

  std::atomic<bool> shutdown_;

	std::atomic<int> activeCount_;

  std::mutex mutex_;

  void updateProgress();
  void closeProgress();

  void work(uint32_t wn);
  void waitForWorkers();

  void generateTrainingData(uint32_t wn, Board board, Move move0);
  void generateTrainingDataOnWorker(uint32_t wn, const std::string& path);
  bool generateTrainingData();
  bool generateGradient(uint32_t wn);
  bool generateGradient();
  void updateParameter(uint32_t wn, FV::ValueType& g, Evaluator::ValueType& e,
      Evaluator::ValueType& max, uint64_t& magnitude);
  void overlapParameters(int index1, int index2);
  void overlapParameters(uint32_t wn);
  void overlapParameters();
  void updateParameters(uint32_t wn);
  void updateParameters();
  void updateMaterial();
  bool iterate();

public:

  BatchLearning(const Config& config)
    : config_(config) {
  }

  /**
   * 機械学習を実行します。
   */
  bool run();

};

} // namespace sunfish

#endif // NLEARN

#endif // SUNFISH_BATCHLEARNING__
