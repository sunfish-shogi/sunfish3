/* logger.cpp
 * 
 * Kubo Ryosuke
 */

#include "Logger.h"

namespace sunfish {

std::mutex Logger::_mutex;
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
