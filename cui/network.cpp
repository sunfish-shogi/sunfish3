/* network.cpp
 * 
 * Kubo Ryosuke
 */

#include "network/CsaClient.h"
#include <fstream>

using namespace sunfish;

int network() {

  // logger settings
  std::ofstream fout("network.log", std::ios::out | std::ios::app);
  if (fout) {
    Loggers::error.addStream(fout, true, true);
    Loggers::warning.addStream(fout, true, true);
    Loggers::message.addStream(fout, true, true);
    Loggers::send.addStream(fout, true, true);
    Loggers::receive.addStream(fout, true, true);
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

  CsaClient csaClient;

  csaClient.execute();

  return 0;

}
