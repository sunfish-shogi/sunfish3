/* bmi.h
 *
 * Kubo Ryosuke
 */

#ifndef SUNFISH_BMI__
#define SUNFISH_BMI__

#if !defined(USE_BMI2)
# define USE_BMI2 0
#endif

#if USE_BMI2
# include <immintrin.h>
#endif

#endif // SUNFISH_BMI__
