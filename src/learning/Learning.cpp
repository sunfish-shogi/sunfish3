/* Learning.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "./Learning.h"
#include "./LearningConfig.h"
#include "./OnlineLearning.h"
#include "config/Config.h"

#define CONFPATH                "learn.conf"

namespace sunfish {

bool Learning::run() {
  Config config;
  config.addDef(LCONF_MODE, LCONF_MODE_BATCH);
  config.addDef(LCONF_KIFU, "");
  config.addDef(LCONF_DEPTH, "3");
  config.addDef(LCONF_THREADS, "1");

  // 設定読み込み
  if (!config.read(CONFPATH)) {
    return false;
  }
  Loggers::message << config.toString();

  if (config.getString(LCONF_MODE) == LCONF_MODE_BATCH) {
    // not implemented

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
