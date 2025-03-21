#pragma once

#if defined(CAT_DARWIN)
#define __assume(x) __builtin_unreachable()
#endif
