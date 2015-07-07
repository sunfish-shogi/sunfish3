/* logger.cpp
 * 
 * Kubo Ryosuke
 */

#include "Logger.h"
#include <chrono>
#include <ctime>
#include <cassert>

namespace sunfish {

const char* LoggerUtil::getIso8601() {
  using namespace std::chrono;
  thread_local char buf[22];
  std::time_t t = duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
  std::tm m;
  std::strftime(buf, sizeof(buf), "%FT%TZ ", gmtime_r(&t, &m));
  return buf;
}

std::mutex Logger::mutex_;
Logger Loggers::error("ERROR");
Logger Loggers::warning("WARNING");
Logger Loggers::message;
Logger Loggers::send("SEND");
Logger Loggers::receive("RECEIVE");
#ifndef NDEBUG
Logger Loggers::debug("DEBUG");
Logger Loggers::test("TEST");
Logger Loggers::develop("DEVELOP");
#endif //NDEBUG

} // namespace sunfish
