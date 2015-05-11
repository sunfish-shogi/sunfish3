/* Worker.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_WORKER__
#define __SUNFISH_WORKER__

#include "../SearchInfo.h"
#include <atomic>
#include <thread>

namespace sunfish {

class Tree;
class Searcher;

struct Worker {

  std::thread thread;
  Searcher* psearcher;
  int workerId;
  std::atomic<int> treeId;
  std::atomic<bool> job;
  std::atomic<bool> shutdown;
  SearchInfoBase info;

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

#endif // __SUNFISH_WORKER__
