/* Worker.cpp
 * 
 * Kubo Ryosuke
 */

#include "Worker.h"
#include "Tree.h"
#include "../Searcher.h"
#include <functional>
#include <chrono>

namespace sunfish {

Worker::Worker() {
}

Worker::~Worker() {
  stop();
}

void Worker::init(int id, Searcher* ps) {
  this->psearcher = ps;
  this->workerId = id;
  this->treeId = Tree::InvalidId;
  memset(&info, 0, sizeof(this->info));
}

void Worker::startOnChildThread(bool sleeping) {
  this->job = false;
  this->shutdown = false;
  this->sleeping = sleeping;
  if (!thread.joinable()) {
    thread = std::thread(std::bind(std::mem_fn(&Worker::waitForJob), this, nullptr));
  }
}

void Worker::startOnCurrentThread(int tid) {
  stop();
  this->treeId = tid;
  this->job = true;
  this->shutdown = false;
  this->sleeping = false;
}

void Worker::stop() {
  if (this->thread.joinable()) {
    this->shutdown = true;
    this->thread.join();
  }
}

void Worker::setJob(int tid) {
  this->treeId = tid;
  this->job = true;
}

void Worker::unsetJob() {
  this->job = false;
}

void Worker::swapTree(int tid) {
  this->treeId = tid;
}

void Worker::sleep() {
  this->sleeping = true;
}

void Worker::waitForJob(Tree* suspendedTree) {
  if (suspendedTree != nullptr) {
    std::lock_guard<std::mutex> lock(psearcher->getSplitMutex());
    unsetJob();
    this->psearcher->addIdleWorker();
  }

  while (true) {
    if (suspendedTree != nullptr && suspendedTree->getTlp().childCount.load() == 0) {
      std::lock_guard<std::mutex> lock(this->psearcher->getSplitMutex());
      if (!this->job) {
        setJob(suspendedTree->getTlp().treeId);
        this->psearcher->reduceIdleWorker();
        return;
      }
    }

    if (this->shutdown) {
      return;
    }

    if (this->job) {
      this->psearcher->searchTlp(this->treeId);
      {
        std::lock_guard<std::mutex> lock(this->psearcher->getSplitMutex());

        psearcher->releaseTree(this->treeId);

        if (suspendedTree != nullptr && suspendedTree->getTlp().childCount.load() == 0) {
          setJob(suspendedTree->getTlp().treeId);
          return;
        }

        unsetJob();
        this->psearcher->addIdleWorker();
      }
    }

    if (this->sleeping) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    } else {
      std::this_thread::yield();
    }
  }
}

} // namespace sunfish
