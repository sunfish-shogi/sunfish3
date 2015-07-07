/* Worker.cpp
 * 
 * Kubo Ryosuke
 */

#include "Worker.h"
#include "Tree.h"
#include "../Searcher.h"
#include <functional>

namespace sunfish {

void Worker::init(int id, Searcher* ps) {
  psearcher = ps;
  workerId = id;
  treeId.store(Tree::InvalidId);
  memset(&info, 0, sizeof(info));
}

void Worker::startOnNewThread() {
  job.store(false);
  shutdown.store(false);
  thread = std::thread(std::bind(std::mem_fn(&Worker::waitForJob), this, nullptr));
}

void Worker::startOnCurrentThread(int tid) {
  treeId.store(tid);
  job.store(true);
  shutdown.store(false);
}

void Worker::stop() {
  shutdown.store(true);
  thread.join();
}

void Worker::setJob(int tid) {
  treeId.store(tid);
  job.store(true);
}

void Worker::unsetJob() {
  job.store(false);
}

void Worker::swapTree(int tid) {
  treeId.store(tid);
}

void Worker::waitForJob(Tree* suspendedTree) {
  if (suspendedTree != nullptr) {
    std::lock_guard<std::mutex> lock(psearcher->getSplitMutex());
    unsetJob();
    psearcher->addIdleWorker();
  }

  while (true) {
    if (suspendedTree != nullptr && suspendedTree->getTlp().childCount.load() == 0) {
      std::lock_guard<std::mutex> lock(psearcher->getSplitMutex());
      if (!job.load()) {
        setJob(suspendedTree->getTlp().treeId);
        psearcher->reduceIdleWorker();
        return;
      }
    }

    if (shutdown.load()) {
      return;
    }

    if (job.load()) {
      psearcher->searchTlp(treeId.load());
      {
        std::lock_guard<std::mutex> lock(psearcher->getSplitMutex());

        psearcher->releaseTree(treeId.load());

        if (suspendedTree != nullptr && suspendedTree->getTlp().childCount.load() == 0) {
          setJob(suspendedTree->getTlp().treeId);
          return;
        }

        unsetJob();
        psearcher->addIdleWorker();
      }
    }

    std::this_thread::yield();
  }
}

} // namespace sunfish
