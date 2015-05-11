/* timer.h
 *
 * Kubo Ryosuke
 */

#ifndef __SUNFISH_TIMER__
#define __SUNFISH_TIMER__

#ifdef WIN32
# include <windows.h>
#elif __MACH__
# include <mach/mach_time.h>
#endif
#include <ctime>

namespace sunfish {

class Timer {
private:

#ifdef WIN32
  LARGE_INTEGER time_b;
#elif __MACH__
  mach_timebase_info_data_t tb;
  uint64_t time_b;
#else
  struct timespec time_b;
#endif

public:

  /**
   * set current time to base
   */
  void set() {

#ifdef WIN32

    // windows
    QueryPerformanceCounter(&time_b);

#elif __MACH__

    // mac os x
    memset(&tb, 0, sizeof(tb));
    mach_timebase_info(&tb);
    time_b = mach_absolute_time();

#else

    // posix
# if defined(CLOCK_MONOTONIC_RAW)
    clock_gettime(CLOCK_MONOTONIC_RAW, &time_b);
# elif defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &time_b);
# else
    clock_gettime(CLOCK_REALTIME, &time_b);
# endif

#endif
  }

  /**
   * get current time(sec) from base
   */
  double get() const {

#ifdef WIN32

    // windows
    LARGE_INTEGER time_n, freq;
    QueryPerformanceCounter(&time_n);
    QueryPerformanceFrequency(&freq);
    return (time_n.QuadPart - time_b.QuadPart + 1) / (double)freq.QuadPart;

#elif __MACH__

    // mac os x
    double time_n = mach_absolute_time();
    return (time_n - time_b) * ((double)tb.numer / tb.denom) * 1.0e-9;

#else

    // posix
    struct timespec time_n;
# if defined(CLOCK_MONOTONIC_RAW)
    clock_gettime(CLOCK_MONOTONIC_RAW, &time_n);
# elif defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &time_n);
# else
    clock_gettime(CLOCK_REALTIME, &time_n);
# endif
    return (time_n.tv_sec - time_b.tv_sec) + (time_n.tv_nsec - time_b.tv_nsec) * 1.0e-9;

#endif

  }

};

} // namespace sunfish

#endif // __SUNFISH_TIMER__
