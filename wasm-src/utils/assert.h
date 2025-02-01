/**
 * @file assert.h
 * @brief The file contains the definition of the assert macro.
 */

#pragma once

#ifdef GK_CONF_DIST
#  define GK_ASSERT(...) ((void)0)
#else
#  include <cassert>

#  ifndef assert
#    define assert(...) ((void)0)
#  endif

#  define GK_ASSERT(x, message) assert(x&& message)
#endif
