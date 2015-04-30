/*
 * Config.h
 */

#ifndef __SUNFISH_CONFIGURE__
#define __SUNFISH_CONFIGURE__

#include <string>
#include <unordered_map>
#include <cstdint>

namespace sunfish {

	class Config {
	private:
		struct Def {
			std::string defaultValue;
		};

		static const int LINE_BUFFER_SIZE = 1024;

		std::unordered_map<std::string, Def> _defs;
		std::unordered_map<std::string, std::string> _data;

		bool readLine(const std::string& line);

	public:

		Config() {}
		Config(const Config&) = delete;
		Config(Config&&) = delete;

		void addDef(const std::string& name, const std::string& defaultValue) {
			addDef(name.c_str(), defaultValue.c_str());
		}
		void addDef(const std::string& name, int32_t defaultValue) {
			addDef(name.c_str(), defaultValue);
		}
		void addDef(const std::string& name, bool defaultValue) {
			addDef(name.c_str(), defaultValue);
		}

		void addDef(const char* name, const char* defaultValue);
		void addDef(const char* name, int32_t defaultValue);
		void addDef(const char* name, bool defaultValue);

		bool read(const std::string& filename) {
			return filename.c_str();
		}
		bool read(const char* filename);

		std::string getString(const std::string& name) const {
			return getString(name.c_str());
		}

		std::string getString(const char* name) const;
		int32_t getInt(const std::string& name) const;
		double getDouble(const std::string& name) const;
		bool getBool(const std::string& name) const;

		std::string toString();

	};

}

#endif // __SUNFISH_CONFIGURE__
