/* sse.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_SSE__
#define SUNFISH_SSE__

#if !defined(USE_SSE2) && __SSE2__
# define USE_SSE2 1
#endif

#if USE_SSE2
# include <emmintrin.h>
#endif

#endif // SUNFISH_SSE__
