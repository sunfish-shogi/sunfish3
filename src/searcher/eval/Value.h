/* Value.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_VALUE__
#define SUNFISH_VALUE__

#include "core/def.h"
#include <cstdint>
#include <cstdlib>
#include <climits>

#if WIN32
# undef max
# undef min
#endif

namespace sunfish {

class Value {
private:

  int32_t value_;

public:

  static CONSTEXPR_CONST int32_t Zero = 0;
  static CONSTEXPR_CONST int32_t Inf = 20000;
  static CONSTEXPR_CONST int32_t Mate = 15000;
  static CONSTEXPR_CONST int32_t PieceInf = 5000;
  static CONSTEXPR_CONST int32_t PieceInfEx = 10000;

  Value() {
  }

  Value(int32_t value) : value_(value) {
  }

  Value& operator+=(const Value& val) {
    value_ += val.value_;
    return *this;
  }

  Value& operator-=(const Value& val) {
    value_ -= val.value_;
    return *this;
  }

  Value& operator*=(const Value& val) {
    value_ *= val.value_;
    return *this;
  }

  Value& operator/=(const Value& val) {
    value_ /= val.value_;
    return *this;
  }

  Value operator+() const {
    return Value(value_);
  }

  Value operator-() const {
    return Value(-value_);
  }

  Value operator+(const Value& val) const {
    return Value(value_ + val.value_);
  }

  Value operator-(const Value& val) const {
    return Value(value_ - val.value_);
  }

  Value operator*(const Value& val) const {
    return Value(value_ * val.value_);
  }

  Value operator/(const Value& val) const {
    return Value(value_ / val.value_);
  }

  bool operator==(const Value& val) const {
    return value_ == val.value_;
  }

  bool operator!=(const Value& val) const {
    return value_ != val.value_;
  }

  bool operator>(const Value& val) const {
    return value_ > val.value_;
  }

  bool operator<(const Value& val) const {
    return value_ < val.value_;
  }

  bool operator>=(const Value& val) const {
    return value_ >= val.value_;
  }

  bool operator<=(const Value& val) const {
    return value_ <= val.value_;
  }

  explicit operator int32_t() const {
    return value_;
  }

  int32_t int32() const {
    return value_;
  }

  static Value add(const Value& a, const Value& b) {
    if (b.value_ > 0 && a.value_ >= Inf - b.value_) {
      return Value(Inf);
    } else if (b.value_ < 0 && a.value_ <= -Inf + (-b.value_)) {
      return Value(-Inf);
    }
    return Value(a.value_ + b.value_);
  }

  static Value sub(const Value& a, const Value& b) {
    if (b.value_ > 0 && a.value_ <= -Inf + b.value_) {
      return Value(-Inf);
    } else if (b.value_ < 0 && a.value_ >= Inf + b.value_) {
      return Value(Inf);
    }
    return Value(a.value_ - b.value_);
  }

  static Value (max)(const Value& a, const Value& b) {
    return a >= b ? a : b;
  }

  static Value (min)(const Value& a, const Value& b) {
    return a < b ? a : b;
  }

  static Value abs(const Value& a) {
    return a >= Value(0) ? a : -a;
  }

  static Value ave(const Value& a, const Value& b) {
    return (a + b) / 2;
  }

  static void swap(Value& val1, Value& val2) {
    Value temp = val1;
    val1 = val2;
    val2 = temp;
  }

};

} // namespace sunfish

#endif // SUNFISH_VALUE__
