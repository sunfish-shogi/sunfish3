/* def.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_DEF__
#define __SUNFISH_DEF__

#include <memory>
#include <string>

#ifdef _MSC_VER
# ifndef WIN32
#  define WIN32 1
# endif
# ifndef VC6
#  define VC6 1
# endif
#elif defined(__GNUC__) && ( defined(__i386__) || defined(__x86_64__) ) && !defined(__MINGW32__)
# ifndef POSIX
#  define POSIX 1
# endif
# if GCC_VERSION < 40600 && !defined(nullptr)
#  define nullptr NULL
# endif
#else
# define UNKNOWN_API 1
#endif

#define ARRAY_SIZE(arr)										((int)(sizeof(arr) / sizeof((arr)[0])))

namespace sunfish {

	// 64 bit integer
#ifdef _MSC_VER
	typedef __int64 int64;
#	define ULLONG_MAX						_UI64_MAX
#	define LLONG_MAX						_I64_MAX
#endif

}

#endif // __SUNFISH_DEF__
