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

#ifndef SDL_config_h_
#define SDL_config_h_

#include "SDL_platform.h"

/* WIKI CATEGORY: - */

/* Add any platform that doesn't build using the configure system. */
#if defined(__WIN32__)
#include "SDL_config_windows.h"
#elif defined(__WINRT__)
#include "SDL_config_winrt.h"
#elif defined(__WINGDK__)
#include "SDL_config_wingdk.h"
#elif defined(__XBOXONE__) || defined(__XBOXSERIES__)
#include "SDL_config_xbox.h"
#elif defined(__MACOSX__)
#include "SDL_config_macosx.h"
#elif defined(__IPHONEOS__)
#include "SDL_config_iphoneos.h"
#elif defined(__ANDROID__)
#include "SDL_config_android.h"
#elif defined(__OS2__)
#include "SDL_config_os2.h"
#elif defined(__EMSCRIPTEN__)
#include "SDL_config_emscripten.h"
#elif defined(__NGAGE__)
#include "SDL_config_ngage.h"
#else
/* This is a minimal configuration just to get SDL running on new platforms. */
#include "SDL_config_minimal.h"
#endif /* platform config */

#ifdef USING_GENERATED_CONFIG_H
#error Wrong SDL_config.h, check your include path?
#endif


/* DEFINES ADDED BY SDL2-COMPAT */

/* Some programs (incorrectly, probably) check defines that aren't available
   with an SDL2 that doesn't have a configure-generated SDL_config.h, so force
   a few that might be important. */
#ifndef SDL_VIDEO_OPENGL
#define SDL_VIDEO_OPENGL 1
#endif
#if defined(__WIN32__)
#define SDL_VIDEO_DRIVER_WINDOWS 1
#endif
#if defined (__LINUX__) || defined(__FREEBSD__) || defined(__NETBSD__) || defined(__OPENBSD__) || defined(__SOLARIS__)
#define SDL_VIDEO_DRIVER_X11 1
#endif
#if defined(__LINUX__)
#define SDL_VIDEO_DRIVER_KMSDRM 1
#endif
#if defined(__MACOSX__)
#define SDL_VIDEO_DRIVER_COCOA 1
#endif
#if defined(__IPHONEOS__) || defined(__TVOS__)
#define SDL_VIDEO_DRIVER_UIKIT 1
#endif
#if defined(__ANDROID__)
#define SDL_VIDEO_DRIVER_ANDROID 1
#endif

/* END DEFINES ADDED BY SDL2-COMPAT */


#endif /* SDL_config_h_ */
