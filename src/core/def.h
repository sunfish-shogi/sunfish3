/* def.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_DEF__
#define SUNFISH_DEF__

#if !defined(UNIX) && !defined(WIN32)
// windows
# ifdef _MSC_VER
#  ifndef WIN32
#   define WIN32 1
#  endif
// gcc family
# elif defined(__GNUC__) && ( defined(__i386__) || defined(__x86_64__) ) && !defined(__MINGW32__)
#  ifndef UNIX
#   define UNIX 1
#  endif
// unknown
# else
#  define UNKNOWN_API 1
# endif
#endif

#if defined(WIN32)
# define NOMINMAX
# define CONSTEXPR const
# define NOEXCEPT
# define ALIGNAS(a) __declspec(align(a))
#else
# define CONSTEXPR constexpr
# define NOEXCEPT noexcept
# define ALIGNAS(a) alignas(a)
#endif

#define ARRAY_SIZE(arr) ((int)(sizeof(arr) / sizeof((arr)[0])))

#endif // SUNFISH_DEF__
