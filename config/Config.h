/*
 * Config.h
 */

#ifndef __SUNFISH_CONFIGURE__
#define __SUNFISH_CONFIGURE__

#include <string>
#include <typeinfo>

namespace sunfish {

	enum ItemType {
		BOOL,
		INTEGER,
		STRING,
	};

	struct ConfigItem {
		const char* name;
		ItemType type;
		void* data;
		const char* defaultValue;
		ConfigItem() {
		}
		ConfigItem(const char* name, ItemType type,
				void* data, const char* defaultValue) {
			this->name = name;
			this->type = type;
			this->data = data;
			this->defaultValue = defaultValue;
		}
	};

	class Config {
	private:

		static const int LINE_BUFFER_SIZE = 1024;

		void init();

		bool readLine(const std::string& line);

		bool convert(ConfigItem& item, const std::string& str);

	protected:

		virtual ConfigItem* itemList() = 0;

		virtual int itemSize() = 0;

	public:

		Config() {}
		Config(const Config&) = delete;
		Config(Config&&) = delete;

		bool read(const char* filename);

		std::string toString();

	};

}

#endif // __SUNFISH_CONFIGURE__
