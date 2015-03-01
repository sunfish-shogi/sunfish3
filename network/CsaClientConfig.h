/*
 * CsaClientConfig.h
 */

#ifndef __SUNFISH_CSACLIENTCONFIG__
#define __SUNFISH_CSACLIENTCONFIG__

#include "config/Config.h"

namespace sunfish {

	class CsaClientConfig : public Config {
	private:

		ConfigItem _items[16];

		std::string _host;
		int _port;
		std::string _user;
		std::string _pass;

		int _depth;
		int _limit;
		int _repeat;
		int _worker;
		bool _enemy;

		int _keepalive;
		int _keepidle;
		int _keepintvl;
		int _keepcnt;

		bool _floodgate;

		std::string _kifu;
		std::string _monitor;

	protected:

		virtual ConfigItem* itemList() override final {
			return _items;
		}

		virtual int itemSize() override final {
			return sizeof(_items)/sizeof(_items[0]);
		}

	public:

		CsaClientConfig(const char* filename = NULL);
		CsaClientConfig(const CsaClientConfig&) = delete;
		CsaClientConfig(CsaClientConfig&&) = delete;
		
		const std::string& getHost() const {
			return _host;
		}

		int getPort() const {
			return _port;
		}

		const std::string& getUser() const {
			return _user;
		}

		const std::string& getPass() const {
			return _pass;
		}

		int getDepth() const {
			return _depth;
		}

		int getLimit() const {
			return _limit;
		}

		int getRepeat() const {
			return _repeat;
		}

		int getWorker() const {
			return _worker;
		}

		bool getEnemy() const {
			return _enemy;
		}

		int getKeepalive() const {
			return _keepalive;
		}

		int getKeepidle() const {
			return _keepidle;
		}

		int getKeepintvl() const {
			return _keepintvl;
		}

		int getKeepcnt() const {
			return _keepcnt;
		}

		bool getFloodgate() const {
			return _floodgate;
		}

		std::string getKifu() const {
			if (_kifu.length() > 0 && _kifu[_kifu.length()-1] == '/') {
				return _kifu;
			} else {
				return _kifu + '/';
			}
		}

		const std::string& getMonitor() const {
			return _monitor;
		}

	};

}

#endif // __SUNFISH_CSACLIENTCONFIG__
