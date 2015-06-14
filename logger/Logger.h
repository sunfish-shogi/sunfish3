/* Logger.h
 * 
 * Kubo Ryosuke
 */

#ifndef SUNFISH_LOGGER__
#define SUNFISH_LOGGER__

#include <iostream>
#include <vector>
#include <ctime>
#include <mutex>
#include <memory>

#define __FILE_LINE__ (__FILE__ ":" __L2STR(__LINE__))
#define __L2STR(l) L2STR__(l)
#define L2STR__(l) #l

namespace sunfish {

class LoggerUtil {
private:

  LoggerUtil();

public:

  static const char* getIso8601();

};

class Logger {
public:
  class SubLogger {
  private:
    struct Data {
      Logger* plogger;
      std::lock_guard<std::mutex> lock;
      Data(Logger* plogger, std::mutex& mutex) : plogger(plogger), lock(mutex) {
      }
      ~Data() {
        plogger->printNoLock("\n");
      }
    };
    std::shared_ptr<Data> data;

  public:
    SubLogger(const SubLogger& org) = default;
    SubLogger(Logger* plogger, std::mutex& mutex) {
      data = std::make_shared<Data>(plogger, mutex);
    }
    template <class T>
    SubLogger& operator<<(T t) {
      data->plogger->printNoLock(t);
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

  static std::mutex mutex_;
  const char* name_;
  std::vector<Stream> os_;

public:
  Logger(const char* name = nullptr) : name_(name) {
  }
  Logger(const Logger& logger) = delete;
  Logger(Logger&& logger) = delete;

  void addStream(std::ostream& o, bool timestamp, bool loggerName,
      const char* before, const char* after) {
    Stream s = { &o, timestamp, loggerName, before, after };
    os_.push_back(s);
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

  template <class T> void printNoLock(const T t, bool top = false) {
    std::vector<Stream>::iterator it;
    for (it = os_.begin(); it != os_.end(); it++) {
      if (it->before != nullptr) {
        *(it->pout) << it->before;
      }
      if (top) {
        if (it->timestamp) {
          *(it->pout) << LoggerUtil::getIso8601();
        }
        if (it->loggerName && name_) {
          *(it->pout) << name_ << ' ';
        }
      }
      *(it->pout) << t;
      if (it->after != nullptr) {
        *(it->pout) << it->after;
      }
      it->pout->flush();
    }
  }

  template <class T> void print(const T t) {
    std::lock_guard<std::mutex> lock(mutex_);
    printNoLock(t);
  }

  template <class T>
  SubLogger operator<<(const T t) {
    SubLogger s(this, mutex_);
    printNoLock(t, true);
    return s;
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

} // namespace sunfish

#endif // SUNFISH_LOGGER__
