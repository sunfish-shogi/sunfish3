/*
 * Connection.cpp
 */

#include "Connection.h"
#include <string.h>

#if WIN32
# include <shlwapi.h>
#else
# include <unistd.h>
#endif

namespace sunfish {

	bool Connection::connect() {
		struct hostent* he;
		struct sockaddr_in sin;
#ifdef WIN32
		WORD    wVersionRequested = MAKEWORD(2,2);
		WSADATA WSAData;
		WSAStartup( wVersionRequested, &WSAData );
#endif

		if (NULL == (he = gethostbyname(_host.c_str()))) {
			return false;
		}
		if (-1 == (_sock = socket(AF_INET, SOCK_STREAM, 0))) {
			return false;
		}
#ifdef UNIX
		// keep-alive
		if (0 != setsockopt(_sock, SOL_SOCKET, SO_KEEPALIVE,
				(void*)&_keepalive, sizeof(_keepalive))) {
		}
# ifdef BSD
		int keepon = _keepidle != 0 ? 1 : 0;
		if (0 != setsockopt(_sock, SOL_SOCKET,  SO_KEEPALIVE,
				(void*)&keepon, sizeof(keepon))) {
		}
# else
		if (0 != setsockopt(_sock, IPPROTO_TCP, TCP_KEEPIDLE,
				(void*)&_keepidle, sizeof(_keepidle))) {
		}
# endif
		if (0 != setsockopt(_sock, IPPROTO_TCP, TCP_KEEPINTVL,
				(void*)&_keepintvl, sizeof(_keepintvl))) {
		}
		if (0 != setsockopt(_sock, IPPROTO_TCP, TCP_KEEPCNT,
				(void*)&_keepcnt, sizeof(_keepcnt))) {
		}
#endif
		memcpy(&sin.sin_addr, he->h_addr, sizeof(struct in_addr));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(_port);
		if (-1 == ::connect(_sock, (struct sockaddr*)(&sin), sizeof(sin))) {
			disconnect();
			return false;
		}

		return true;
	}

	void Connection::disconnect() {
#ifdef WIN32
		closesocket(_sock);
		WSACleanup();
#else
		close(_sock);
#endif
	}

	bool Connection::receive() {
		char buf[1024];

		if (!_received.empty()) {
			return true;
		}

		memset( buf, 0, sizeof(buf) );
		if (recv(_sock, buf, sizeof(buf) - 1, 0) > 0) {
			char* p = buf;
			while (p != NULL && p[0] != '\0') {
				char* p2 = strchr(p, '\n');
				if (p2 != NULL) {
					p2[0] = '\0';
					p2++;
				}
				_received.push(std::string(p));
				p = p2;
			}
			return true;
		}
		return false;
	}

	bool Connection::send(const std::string& str) {
		if (-1 != ::send(_sock, str.c_str(), str.length(), 0)) {
			return true;
		}
		return false;
	}

	bool Connection::sendln(const std::string& str) {
		std::string strln = str + '\n';
		if (-1 != ::send(_sock, strln.c_str(), strln.length(), 0)) {
			return true;
		}
		return false;
	}

}
