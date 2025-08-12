#pragma once

#include "targetver.h"

#ifdef _DEBUG
// ReSharper disable once CppInconsistentNaming
#   define _CRTDBG_MAP_ALLOC
#   include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#define _SILENCE_CXX20_REL_OPS_DEPRECATION_WARNING

#include <windows.h>
