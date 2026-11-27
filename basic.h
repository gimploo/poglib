#pragma once
#ifdef _WIN64
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

//NOTE: mostly used to know whether its mac since it only has support till 4.1
#if defined(__linux__) || defined(_WIN64)
    #define MODERN_OPENGL_SUPPORTED_PLATFORM 1
#else
    #define MODERN_OPENGL_SUPPORTED_PLATFORM 0
#endif

#include "basic/dbg.h"
#include "basic/test.h"
#include "basic/common.h"
#include "basic/ds.h"
#include "basic/file.h"
#include "basic/color.h"
#include "basic/str.h"
#include "basic/util.h"
#include "basic/concurrency.h"
#include "basic/arena.h"
#include "basic/runtime-ctx.h"
