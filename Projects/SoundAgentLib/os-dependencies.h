#pragma once

#include "targetver.h"

#ifdef _DEBUG
// ReSharper disable once CppInconsistentNaming
#   define _CRTDBG_MAP_ALLOC
#   include <crtdbg.h>
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
