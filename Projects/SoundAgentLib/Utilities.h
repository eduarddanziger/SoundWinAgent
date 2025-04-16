#pragma once

#include <mmdeviceapi.h>
#include <set>
#include <sstream>

#ifdef NDEBUG
#undef NDEBUG
#include <cassert>
#define NDEBUG  // NOLINT(clang-diagnostic-unused-macros)
#else
#include <cassert>
#endif

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }
