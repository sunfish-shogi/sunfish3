/* Wildcard.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_WILDCARD__
#define __SUNFISH_WILDCARD__

#include <string>

namespace sunfish {

	class Wildcard {
	private:
		std::string _pattern;

		static std::string optimize(const char* pattern);
		static std::string optimize(const std::string& pattern) {
			return optimize(pattern.c_str());
		}

		static bool match(const char* pp, const char* pt);

	public:
		Wildcard(const char* pattern) : _pattern(optimize(pattern)) {
		}
		Wildcard(const std::string& pattern) : _pattern(optimize(pattern)) {
		}

		bool match(const char* str) const;
		bool match(const std::string& str) const {
			return match(str);
		}

	};

}

#endif // __SUNFISH_WILDCARD__
