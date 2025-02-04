/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef SDL_config_unix_h_
#define SDL_config_unix_h_

#include "SDL_platform.h"

/* C datatypes */
#if defined(__LP64__) || defined(_LP64) || defined(_WIN64)
#define SIZEOF_VOIDP 8
#else
#define SIZEOF_VOIDP 4
#endif

#define HAVE_GCC_ATOMICS 1

/* Assume that any reasonable Unix platform has Standard C headers */
#define STDC_HEADERS 1
#define HAVE_CTYPE_H 1
#define HAVE_FLOAT_H 1
#define HAVE_INTTYPES_H 1
#define HAVE_LIMITS_H 1
#define HAVE_MATH_H 1
#define HAVE_SIGNAL_H 1
#define HAVE_STDARG_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDIO_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRING_H 1
#define HAVE_WCHAR_H 1

/* Assume that any reasonable Unix platform has POSIX headers */
#define HAVE_ICONV_H 1
#define HAVE_STRINGS_H 1
#define HAVE_SYS_TYPES_H 1

/* Non-standardized, but we assume they exist anyway */
#define HAVE_ALLOCA_H 1
#define HAVE_MALLOC_H 1
#define HAVE_MEMORY_H 1

#define SDL_VIDEO_DRIVER_X11 1

#if defined(__LINUX__)
#define SDL_VIDEO_DRIVER_KMSDRM 1
#define SDL_VIDEO_DRIVER_WAYLAND 1
#endif

/* Enable OpenGL support */
#define SDL_VIDEO_OPENGL 1
#define SDL_VIDEO_OPENGL_ES 1

/* Enable Vulkan support */
#if defined(__LINUX__)
#define SDL_VIDEO_VULKAN 1
#endif

#endif /* SDL_config_unix_h_ */
