/* ProgramOptions.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_PROGRAMOPTIONS__
#define __SUNFISH_PROGRAMOPTIONS__

namespace sunfish {

	class ProgramOptions {
	private:

		struct Option {
			std::string key;
			std::string shortKey;
			std::string description;
			bool arg;
			bool exists;
			std::string value;
		};

		std::vector<Option> _options;
		std::vector<std::string> _stdArgs;

		void setValue(const char* key, const char* value) {
			for (auto& option : _options) {
				if (option.key == key || option.shortKey == key) {
					option.exists = true;
					option.value = value;
					return;
				}
			}
		}

		const Option* getOption(const char* key) const {
			for (auto& option : _options) {
				if (option.key == key || option.shortKey == key) {
					return &option;
				}
			}
			return nullptr;
		}

	public:

		ProgramOptions() = default;
		ProgramOptions(const ProgramOptions&) = delete;
		ProgramOptions(ProgramOptions&&) = delete;

		void addOption(const char* key, const char* description, bool arg = false) {
			_options.push_back(std::move(Option{ key, "", description, arg, false, "" }));
		}

		void addOption(const char* key, const char* shortKey, const char* description, bool arg = false) {
			_options.push_back(std::move(Option{ key, shortKey, description, arg, false, "" }));
		}

		void parse(int argc, char** argv) {
			const char* key = nullptr;
			for (int i = 1; i <= argc; i++) {
				bool isLast = (i == argc);
				const char* arg = !isLast ? argv[i] : "";
				if (arg[0] == '-') {
					bool isFullSpell = (arg[1] == '-');
					if (key != nullptr) {
						setValue(key, "");
					}
					key = &arg[isFullSpell?2:1];
					auto opt = getOption(key);
					if (opt == nullptr || !opt->arg) {
						setValue(key, "");
						key = nullptr;
					}
				} else if (key != nullptr) {
					setValue(key, arg);
					key = nullptr;
				} else if (!isLast) {
					_stdArgs.push_back(arg);
				}
			}
		}

		bool has(const char* key) const {
			for (const auto& option : _options) {
				if (option.key == key || option.shortKey == key) {
					if (option.exists) {
  				return true;
					}
				}
			}
			return false;
		}

		const char* getValue(const char* key) const {
			for (const auto& option : _options) {
				if (option.key == key || option.shortKey == key) {
					if (option.exists) {
  				return option.value.c_str();
					}
				}
			}
			return "";
		}

		const std::vector<std::string> getStdArgs() const {
			return _stdArgs;
		}

		std::string help() const {
			std::ostringstream oss;
			oss << "Option:\n";
			for (const auto& option : _options) {
				int length = 0;
				oss << "  ";
				if (!option.shortKey.empty()) {
					oss << '-' << option.shortKey << " [ --" << option.key << ']';
					length += (int)(option.shortKey.length() + option.key.length() + 7);
				} else {
					oss << "--" << option.key;
					length += (int)(option.key.length() + 2);
				}
				if (option.arg) {
					oss << " arg";
					length += 4;
				}
				int indent = std::max(1, (int)(24 - length));
				for (int i = 0; i < indent; i++) {
					oss << ' ';
				}
				oss << option.description << '\n';
			}
			return oss.str();
		}
	};

}

#endif // __SUNFISH_PROGRAMOPTIONS__
