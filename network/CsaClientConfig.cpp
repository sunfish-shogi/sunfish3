/*
 * CsaClientConfig.cpp
 */

#include "CsaClientConfig.h"
#include "logger/Logger.h"
#include <fstream>

namespace sunfish {

	CsaClientConfig::CsaClientConfig(const char* filename) {
		int i = 0;
		_items[i++] = ConfigItem("host", STRING, &_host, "localhost");
		_items[i++] = ConfigItem("port", INTEGER, &_port, "4081");
		_items[i++] = ConfigItem("user", STRING, &_user, "test");
		_items[i++] = ConfigItem("pass", STRING, &_pass, "");
		_items[i++] = ConfigItem("depth", INTEGER, &_depth, "32");
		_items[i++] = ConfigItem("limit", INTEGER, &_limit, "10");
		_items[i++] = ConfigItem("repeat", INTEGER, &_repeat, "1");
		_items[i++] = ConfigItem("worker", INTEGER, &_worker, "1");
		_items[i++] = ConfigItem("enemy", BOOL, &_enemy, "1");
		_items[i++] = ConfigItem("keepalive", INTEGER, &_keepalive, "1");
		_items[i++] = ConfigItem("keepidle", INTEGER, &_keepidle, "120");
		_items[i++] = ConfigItem("keepintvl", INTEGER, &_keepintvl, "60");
		_items[i++] = ConfigItem("keepcnt", INTEGER, &_keepcnt, "10");
		_items[i++] = ConfigItem("floodgate", BOOL, &_floodgate, "0");
		_items[i++] = ConfigItem("kifu", STRING, &_kifu, "Kifu");
		_items[i++] = ConfigItem("monitor", STRING, &_monitor, "");

		if (filename != NULL) {
			read(filename);
		}
	}

}
