/*
 * Config.cpp
 */

#include "Config.h"
#include "logger/Logger.h"
#include <sstream>
#include <fstream>

namespace sunfish {

	bool Config::read(const char* filename) {
		// open a configure file.
		Loggers::message << "open config file: " << filename;
		std::ifstream fin(filename);
		if (!fin) {
			Loggers::error << "open error: " << filename;
			return false;
		}
		// set default values
		init();
		// input
		for (int l = 0; ; l++) {
			char line[LINE_BUFFER_SIZE];
			fin.getline(line, sizeof(line));
			if (fin.eof()) { break; }
			if (fin.fail()) {
				Loggers::error << "read error: " << filename;
				return false;
			}
			if (!readLine(line)) {
				Loggers::error << "syntax error: " << filename << '(' << l << ')';
				Loggers::error << line;
			}
		}
		fin.close(); // close a configure file.
		return true;
	}

	void Config::init() {
		// 初期値を代入
		ConfigItem* items = itemList();
		int size = itemSize();
		for (int i = 0; i < size; i++){
			// 設定項目のデータ型毎に変換
			if (!convert(items[i], items[i].defaultValue)) {
				Loggers::error << "unknown error";
			}
		}
	}

	bool Config::readLine(const std::string& line) {
		if (line[0] == '\0' || line[0] == '#') {
			return true;
		}
		// '=' で左辺値と右辺値に分解
		auto sep = line.find_first_of('=');
		if (sep == std::string::npos) {
			return false;
		}
		std::string key = line.substr(0, sep);
		std::string value = line.substr(sep + 1);
		// 左辺値に一致する項目を探す。
		ConfigItem* items = itemList();
		int size = itemSize();
		for (int i = 0; i < size; i++){
			if (key == items[i].name) {
				// 設定項目のデータ型毎に変換
				if (convert(items[i], value)) {
					return true;
				} else {
					Loggers::error << "unknown error";
					return false;
				}
			}
		}
		return false;
	}

	bool Config::convert(ConfigItem& item, const std::string& str) {
		// 設定項目のデータ型毎に変換
		if (item.type == STRING) {
			*(std::string*)item.data = str;
		} else if (item.type == INTEGER) {
			*(int*)item.data = std::stoi(str);
		} else if (item.type == BOOL) {
			*(bool*)item.data = (bool)std::stoi(str);
		} else {
			return false;
		}
		return true;
	}

	std::string Config::toString() {
		std::ostringstream oss;
		ConfigItem* items = itemList();
		int size = itemSize();
		for (int i = 0; i < size; i++){
			oss << items[i].name << "\t: ";
			if (items[i].type == STRING) {
				oss << *(std::string*)items[i].data;
			} else if (items[i].type == INTEGER) {
				oss << *(int*)items[i].data;
			} else if (items[i].type == BOOL) {
				oss << *(bool*)items[i].data;
			} else {
				oss << "error";
			}
			oss << '\n';
		}
		return oss.str();
	}

}
