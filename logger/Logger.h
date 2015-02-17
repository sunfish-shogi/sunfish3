/* logger.h
 * 
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_LOGGER__
#define __SUNFISH_LOGGER__

#include <iostream>
#include <vector>
#include <ctime>
#include <mutex>

#define __THIS__			__FILE__ << '(' << __LINE__ << ')'

namespace sunfish {

	class Logger {
	public:
		class SubLogger {
		private:
			Logger& logger;
			int* pcount;

		public:
			SubLogger(const SubLogger& org) : logger(org.logger), pcount(org.pcount) {
				(*pcount)++;
			}
			SubLogger(Logger& logger) : logger(logger), pcount(new int(1)) {
			}
			~SubLogger() {
				(*pcount)--;
				if (*pcount == 0) {
					delete pcount;
					logger.print("\n");
				}
			}

			template <class T>
			SubLogger& operator<<(T t) {
				logger.print(t);
				return *this;
			}
		};

	private:
		struct Stream {
			std::ostream* pout;
			bool timestamp;
			bool loggerName;
			const char* before;
			const char* after;
		};

		static std::mutex _mutex;
		const char* _name;
		std::vector<Stream> _os;

	public:
		Logger(const char* name = nullptr) : _name(name) {
		}
		Logger(const Logger& logger) = delete;
		Logger(Logger&& logger) = delete;

		void addStream(std::ostream& o, bool timestamp, bool loggerName,
				const char* before, const char* after) {
			Stream s = { &o, timestamp, loggerName, before, after };
			_os.push_back(s);
		}
		void addStream(std::ostream& o, bool timestamp, bool loggerName) {
			addStream(o, timestamp, loggerName, nullptr, nullptr);
		}
		void addStream(std::ostream& o, const char* before, const char* after) {
			addStream(o, false, false, before, after);
		}
		void addStream(std::ostream& o) {
			addStream(o, false, false, nullptr, nullptr);
		}

		template <class T> void print(const T t, bool top = false) {
			std::lock_guard<std::mutex> lock(_mutex);
			std::vector<Stream>::iterator it;
			for (it = _os.begin(); it != _os.end(); it++) {
				if (it->before != nullptr) {
					*(it->pout) << it->before;
				}
				if (top) {
					if (it->timestamp) {
						time_t t = time(nullptr);
#if defined(_MSC_VER)
						auto& lt = *localtime(&t);
#else
						struct tm lt;
						localtime_r(&t, &lt);
#endif
						char tstr[32];
						strftime(tstr, sizeof(tstr)-1, "%Y-%m-%dT%H:%M:%S\t", &lt);
						*(it->pout) << tstr;
					}
					if (it->loggerName && _name) {
						*(it->pout) << '[' << _name << ']';
					}
				}
				*(it->pout) << t;
				if (it->after != nullptr) {
					*(it->pout) << it->after;
				}
				it->pout->flush();
			}
		}

		template <class T>
		SubLogger operator<<(const T t) {
			print(t, true);
			return SubLogger(*this);
		}

	};

	class Loggers {
	public:
		static Logger error;
		static Logger warning;
		static Logger message;
		static Logger send;
		static Logger receive;
#ifndef NDEBUG
		static Logger debug;
		static Logger test;
		static Logger develop;
#endif //NDEBUG
	};

}

#endif // __SUNFISH_LOGGER__
