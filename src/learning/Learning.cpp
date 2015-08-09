/* Learning.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "./Learning.h"
#include "./LearningConfig.h"
#include "./BatchLearning.h"
#include "./OnlineLearning.h"
#include "config/Config.h"
#include "logger/Logger.h"

#define CONFPATH                "learn.conf"

namespace sunfish {

bool Learning::run() {
  Config config;
  config.addDef(LCONF_MODE, LCONF_MODE_BATCH);
  config.addDef(LCONF_KIFU, "");
  config.addDef(LCONF_DEPTH, "3");
  config.addDef(LCONF_THREADS, "1");
  config.addDef(LCONF_ITERATION, "10");

  // 設定読み込み
  if (!config.read(CONFPATH)) {
    return false;
  }
  Loggers::message << config.toString();

  if (config.getString(LCONF_MODE) == LCONF_MODE_BATCH) {
    BatchLearning learning(config);
    learning.run();

  } else if (config.getString(LCONF_MODE) == LCONF_MODE_ONLINE) {
    OnlineLearning learning(config);
    learning.run();

  } else {
    Loggers::error << "Invalid learning mode [" << config.getString(LCONF_MODE) << "]";
    return false;

  }

  return true;
}

} // namespace sunfish

#endif // NLEARN
