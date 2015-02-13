/* def.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_DEF__
#define __SUNFISH_DEF__

// windows
#ifdef _MSC_VER
# ifndef WIN32
#  define WIN32 1
# endif
// gcc family
#elif defined(__GNUC__) && ( defined(__i386__) || defined(__x86_64__) ) && !defined(__MINGW32__)
# ifndef POSIX
#  define POSIX 1
# endif
// unknown
#else
# define UNKNOWN_API 1
#endif

#if defined(WIN32)
# define CONSTEXPR const
#else
# define CONSTEXPR constexpr
#endif

#define ARRAY_SIZE(arr)										((int)(sizeof(arr) / sizeof((arr)[0])))

#endif // __SUNFISH_DEF__
