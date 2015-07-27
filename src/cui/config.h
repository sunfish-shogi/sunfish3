/* config.h
 * 
 * Kubo Ryosuke
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include "core/def.h"

#if WIN32
# define ENABLE_ESC_SEQ_COLOR 0
#else
# define ENABLE_ESC_SEQ_COLOR 1
#endif

#if ENABLE_ESC_SEQ_COLOR
# define ESC_SEQ_COLOR_BLACK   "\x1b[30m"
# define ESC_SEQ_COLOR_RED     "\x1b[31m"
# define ESC_SEQ_COLOR_GREEN   "\x1b[32m"
# define ESC_SEQ_COLOR_YELLOW  "\x1b[33m"
# define ESC_SEQ_COLOR_BLUE    "\x1b[34m"
# define ESC_SEQ_COLOR_MAGENTA "\x1b[35m"
# define ESC_SEQ_COLOR_CYAN    "\x1b[36m"
# define ESC_SEQ_COLOR_WHITE   "\x1b[37m"
# define ESC_SEQ_COLOR_RESET   "\x1b[39m"
#else
# define ESC_SEQ_COLOR_BLACK   nullptr
# define ESC_SEQ_COLOR_RED     nullptr
# define ESC_SEQ_COLOR_GREEN   nullptr
# define ESC_SEQ_COLOR_YELLOW  nullptr
# define ESC_SEQ_COLOR_BLUE    nullptr
# define ESC_SEQ_COLOR_MAGENTA nullptr
# define ESC_SEQ_COLOR_CYAN    nullptr
# define ESC_SEQ_COLOR_WHITE   nullptr
# define ESC_SEQ_COLOR_RESET   nullptr
#endif

#endif // CONFIG_H_
