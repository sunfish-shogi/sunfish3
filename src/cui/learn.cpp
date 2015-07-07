/* learn.cpp
 * 
 * Kubo Ryosuke
 */

#ifndef NLEARN

#include "learn/Learn.h"
#include "logger/Logger.h"
#include <fstream>

using namespace sunfish;

int learn() {
  std::ofstream fout("learn.log", std::ios::out | std::ios::app);
  if (fout) {
    Loggers::error.addStream(fout, true, true);
    Loggers::warning.addStream(fout, true, true);
    Loggers::message.addStream(fout, true, true);
  }
  Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
  Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
  Loggers::message.addStream(std::cerr);
  Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
  Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
#ifndef NDEBUG
  Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
  Loggers::develop.addStream(std::cerr, "\x1b[37m", "\x1b[39m");
#endif // NDEBUG

  Learn learn;
  bool ok = learn.run();

  return ok ? 0 : 1;
}

int analyzeEvalBin() {
  Loggers::error.addStream(std::cerr, "\x1b[31m", "\x1b[39m");
  Loggers::warning.addStream(std::cerr, "\x1b[33m", "\x1b[39m");
  Loggers::message.addStream(std::cerr);
  Loggers::send.addStream(std::cerr, true, true, "\x1b[34m", "\x1b[39m");
  Loggers::receive.addStream(std::cerr, true, true, "\x1b[35m", "\x1b[39m");
#ifndef NDEBUG
  Loggers::debug.addStream(std::cerr, "\x1b[36m", "\x1b[39m");
  Loggers::develop.addStream(std::cerr, "\x1b[37m", "\x1b[39m");
#endif // NDEBUG

  Learn learn;
  bool ok = learn.analyze();

  return ok ? 0 : 1;
}

#endif // NLEARN
