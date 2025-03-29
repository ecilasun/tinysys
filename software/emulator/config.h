#pragma once

#if defined(CAT_DARWIN) || defined(CAT_LINUX)
#define __assume(x) __builtin_unreachable()
#endif
