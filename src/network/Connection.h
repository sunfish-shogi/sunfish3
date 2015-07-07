/*
 * Connection.h
 */

#ifndef SUNFISH_CONNECTION__
#define SUNFISH_CONNECTION__

#include "core/def.h"

#if WIN32
# include "windows.h" // Fix for C2146
#endif
#include <queue>
#include <string>

#ifdef WIN32
# include <process.h>
# include <mmsystem.h>
# include <csignal>
# include <ctime>
#else
# include <unistd.h>
# include <csignal>
# include <dirent.h>
# include <strings.h>
# include <sched.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <sys/param.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <netdb.h>
# include <errno.h>
#endif

namespace sunfish {

class Connection {
private:
  std::string host_;
  int port_;
#ifdef WIN32
  SOCKET sock_;
#else
  int sock_;
#endif
  std::queue<std::string> received_;
  int keepalive_;
  int keepidle_;
  int keepintvl_;
  int keepcnt_;

  void init() {
    keepalive_ = 1;
    keepidle_ = 7200;
    keepintvl_ = 75;
    keepcnt_ = 3;
  }

public:
  Connection() {
    init();
  }
  Connection(const Connection&) = delete;
  Connection(Connection&&) = delete;

  Connection(const std::string& host_, int port)
      : host_(host_), port_(port) {
    init();
  }

  void setHost(const std::string& host) {
    host_ = host;
  }

  void setPort(int port) {
    port_ = port;
  }

  void setKeepalive(
      int keepalive, int keepidle,
      int keepintvl, int keepcnt) {
    keepalive_ = keepalive;
    keepidle_ = keepidle;
    keepintvl_ = keepintvl;
    keepcnt_ = keepcnt;
  }

  bool connect();

  void disconnect();

  bool receive();

  std::string getReceivedString() {
    std::string str = received_.front();
    received_.pop();
    return str;
  }

  bool send(const std::string& str);

  bool sendln(const std::string& str);
};

} // namespace sunfish

#endif // SUNFISH_CONNECTION__
