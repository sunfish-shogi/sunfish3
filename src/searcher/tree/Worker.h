/* Worker.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_WORKER__
#define SUNFISH_WORKER__

#include "../SearchInfo.h"
#include <atomic>
#include <thread>

namespace sunfish {

class Tree;
class Searcher;

struct Worker {

  SearchInfoBase info;
  std::thread thread;
  Searcher* psearcher;
  int workerId;
  std::atomic<int> treeId;
  std::atomic<bool> job;
  std::atomic<bool> shutdown;

  void init(int id, Searcher* ps);

  void startOnNewThread();

  void startOnCurrentThread(int tid);

  void stop();

  void setJob(int tid);

  void unsetJob();

  void swapTree(int tid);

  void waitForJob(Tree* suspendedTree);

};

} // namespace sunfish

#endif // SUNFISH_WORKER__
