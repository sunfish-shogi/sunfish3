/*
 * Connection.h
 */

#ifndef __SUNFISH_CONNECTION__
#define __SUNFISH_CONNECTION__

#include "core/def.h"

#if WIN32
# include "windows.h" // Fix for C2146
#endif
#include <queue>
#include <string>

#ifdef WIN32
#	include <process.h>
#	include <mmsystem.h>
#	include <csignal>
#	include <ctime>
#else
#	include <unistd.h>
#	include <csignal>
#	include <dirent.h>
#	include <strings.h>
#	include <sched.h>
#	include <sys/types.h>
#	include <sys/socket.h>
# include <sys/param.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <netdb.h>
#	include <errno.h>
#endif

namespace sunfish {

	class Connection {
	private:
		std::string _host;
		int _port;
#ifdef WIN32
		SOCKET _sock;
#else
		int _sock;
#endif
		std::queue<std::string> _received;
		int _keepalive;
		int _keepidle;
		int _keepintvl;
		int _keepcnt;

		void init() {
			_keepalive = 1;
			_keepidle = 7200;
			_keepintvl = 75;
			_keepcnt = 3;
		}

	public:
		Connection() {
			init();
		}
		Connection(const Connection&) = delete;
		Connection(Connection&&) = delete;

		Connection(const std::string& _host, int port)
				: _host(_host), _port(port) {
			init();
		}

		void setHost(const std::string& host) {
			_host = host;
		}

		void setPort(int port) {
			_port = port;
		}

		void setKeepalive(
				int keepalive, int keepidle,
				int keepintvl, int keepcnt) {
			_keepalive = keepalive;
			_keepidle = keepidle;
			_keepintvl = keepintvl;
			_keepcnt = keepcnt;
		}

		bool connect();

		void disconnect();

		bool receive();

		std::string getReceivedString() {
			std::string str = _received.front();
			_received.pop();
			return str;
		}

		bool send(const std::string& str);

		bool sendln(const std::string& str);
	};

}

#endif // __SUNFISH_CONNECTION__
