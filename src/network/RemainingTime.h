/*
 * RemainingTime.h
 */

#ifndef SUNFISH_REMAININGTIME__
#define SUNFISH_REMAININGTIME__

#include <string>
#include <sstream>

namespace sunfish {

class RemainingTime {
private:
  int total_;
  int remain_;
  int readoff_;

public:
  RemainingTime() {
  }

  RemainingTime(int total, int readoff)
      : total_(total), readoff_(readoff) {
    reset();
  }

  void init(int total, int readoff = 0) {
    total_ = total;
    readoff_ = readoff;
    reset();
  }

  void reset() {
    remain_ = total_;
  }

  int use(int sec) {
    remain_ = remain_ > sec ? remain_ - sec : 0;
    return remain_;
  }

  int usable() const {
    return remain_ + readoff_;
  }

  int isLimited() const {
    return total_ != 0 || readoff_ != 0;
  }

  int getTotal() const {
    return total_;
  }

  int getRemain() const {
    return remain_;
  }

  int getReadoff() const {
    return readoff_;
  }

  std::string toString() const {
    std::ostringstream oss;
    oss << remain_ << '/' << total_ << ' ' << readoff_;
    return oss.str();
  }

};

} // namespace sunfish

#endif // SUNFISH_REMAININGTIME__
