#pragma once

// Platform OpenGL configuration.
//
// Linux/Windows target the latest OpenGL (4.6 core) so modern features can be
// used during development. macOS is capped by Apple at OpenGL 4.1 core, so the
// mac build downgrades the context (and GLSL) to 4.1/410. Use the standard
// `#ifdef __APPLE__` macro in code to select the macOS downgraded path.
//
// When working on a Linux/Windows machine, run `tools/check_macos_compat.sh`
// (it also runs automatically as part of `build.sh`/`build.bat`) to get warned
// about any GL 4.2+ API / GLSL feature that would break the macOS build.

#if defined(__APPLE__) && defined(__MACH__)
    #define POGLIB_GL_CONTEXT_MAJOR_VERSION 4
    #define POGLIB_GL_CONTEXT_MINOR_VERSION 1
    #define POGLIB_GLSL_VERSION_PREFIX      "#version 410 core\n"
#else
    #define POGLIB_GL_CONTEXT_MAJOR_VERSION 4
    #define POGLIB_GL_CONTEXT_MINOR_VERSION 6
    #define POGLIB_GLSL_VERSION_PREFIX      "#version 460 core\n"
#endif
