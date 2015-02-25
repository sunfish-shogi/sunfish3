/* Value.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_VALUE__
#define __SUNFISH_VALUE__

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

		int32_t _value;

	public:

		static const int32_t Zero = 0;
		static const int32_t Inf = INT_MAX / 2;
		static const int32_t Mate = INT_MAX / 4;

		Value() {
		}

		Value(int32_t value) : _value(value) {
		}

		Value& operator+=(const Value& val) {
			_value += val._value;
			return *this;
		}

		Value& operator-=(const Value& val) {
			_value -= val._value;
			return *this;
		}

		Value& operator*=(const Value& val) {
			_value *= val._value;
			return *this;
		}

		Value& operator/=(const Value& val) {
			_value /= val._value;
			return *this;
		}

		Value operator+() const {
			return Value(_value);
		}

		Value operator-() const {
			return Value(-_value);
		}

		Value operator+(const Value& val) const {
			return Value(_value + val._value);
		}

		Value operator-(const Value& val) const {
			return Value(_value - val._value);
		}

		Value operator*(const Value& val) const {
			return Value(_value * val._value);
		}

		Value operator/(const Value& val) const {
			return Value(_value / val._value);
		}

		bool operator==(const Value& val) const {
			return _value == val._value;
		}

		bool operator!=(const Value& val) const {
			return _value != val._value;
		}

		bool operator>(const Value& val) const {
			return _value > val._value;
		}

		bool operator<(const Value& val) const {
			return _value < val._value;
		}

		bool operator>=(const Value& val) const {
			return _value >= val._value;
		}

		bool operator<=(const Value& val) const {
			return _value <= val._value;
		}

		explicit operator int32_t() const {
			return _value;
		}

		int32_t int32() const {
			return _value;
		}

		static Value add(const Value& a, const Value& b) {
			if (b._value > 0 && a._value >= Inf - b._value) {
				return Value(Inf);
			} else if (b._value < 0 && a._value <= -Inf + (-b._value)) {
				return Value(-Inf);
			}
			return Value(a._value + b._value);
		}

		static Value sub(const Value& a, const Value& b) {
			if (b._value > 0 && a._value <= -Inf + b._value) {
				return Value(-Inf);
			} else if (b._value < 0 && a._value >= Inf + b._value) {
				return Value(Inf);
			}
			return Value(a._value - b._value);
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

	};

}

#endif // __SUNFISH_VALUE__
