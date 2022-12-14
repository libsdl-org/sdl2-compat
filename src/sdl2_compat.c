/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

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

/* This file contains functions for backwards compatibility with SDL2 */

#include "sdl3_include_wrapper.h"

#include "dynapi/SDL_dynapi.h"

#if SDL_DYNAMIC_API
#include "dynapi/SDL_dynapi_overrides.h"
/* force DECLSPEC off...it's all internal symbols now.
   These will have actual #defines during SDL_dynapi.c only */
#ifdef DECLSPEC
#undef DECLSPEC
#endif
#define DECLSPEC
#endif

/*
 * We report the library version as
 * 2.$(SDL2_COMPAT_VERSION_MINOR).$(SDL2_COMPAT_VERSION_PATCH). This number
 * should be way ahead of what SDL2 Classic would report, so apps can
 * decide if they're running under the compat layer, if they really care.
 * The patch level changes in release cycles. The minor version starts at 90
 * to be high by default, and usually doesn't change (and maybe never changes).
 * The number might increment past 90 if there are a ton of releases.
 */
#define SDL2_COMPAT_VERSION_MINOR 90
#define SDL2_COMPAT_VERSION_PATCH 0

#include <stdarg.h>
#include <limits.h>
#include <stddef.h>
#if defined(_MSC_VER) && (_MSC_VER < 1600)
/* intptr_t already handled by stddef.h. */
#else
#include <stdint.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#define HAVE_STDIO_H 0  /* !!! FIXME: we _currently_ don't include stdio.h on windows. Should we? */
#else
#include <stdio.h> /* fprintf(), etc. */
#include <stdlib.h>    /* for abort() */
#include <string.h>
#define HAVE_STDIO_H 1
#endif

/* mingw headers may define these ... */
#undef strtod
#undef strcasecmp
#undef strncasecmp
#undef snprintf
#undef vsnprintf

#define SDL_BlitSurface SDL_UpperBlit

#ifdef __linux__
#include <unistd.h> /* for readlink() */
#endif

#if defined(__unix__) || defined(__APPLE__)
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#define SDL12_MAXPATH PATH_MAX
#elif defined _WIN32
#define SDL12_MAXPATH MAX_PATH
#elif defined __OS2__
#define SDL12_MAXPATH CCHMAXPATH
#else
#define SDL12_MAXPATH 1024
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef Sint64 SDL_GestureID;

#define SDL3_SYM(rc,fn,params,args,ret) \
    typedef rc (SDLCALL *SDL3_##fn##_t) params; \
    static SDL3_##fn##_t SDL3_##fn = NULL;
#include "sdl3_syms.h"

/* Things that _should_ be binary compatible pass right through... */
#define SDL3_SYM_PASSTHROUGH(rc,fn,params,args,ret) \
    DECLSPEC rc SDLCALL SDL_##fn params { ret SDL3_##fn args; }
#include "sdl3_syms.h"


/* these are macros (etc) in the SDL headers, so make our own. */
#define SDL3_OutOfMemory() SDL3_Error(SDL_ENOMEM)
#define SDL3_Unsupported() SDL3_Error(SDL_UNSUPPORTED)
#define SDL3_InvalidParamError(param) SDL3_SetError("Parameter '%s' is invalid", (param))
#define SDL3_zero(x) SDL3_memset(&(x), 0, sizeof((x)))
#define SDL3_zerop(x) SDL3_memset((x), 0, sizeof(*(x)))
#define SDL3_copyp(dst, src)                                                                 \
    { SDL_COMPILE_TIME_ASSERT(SDL3_copyp, sizeof (*(dst)) == sizeof (*(src))); }             \
    SDL3_memcpy((dst), (src), sizeof (*(src)))


static SDL_bool WantDebugLogging = SDL_FALSE;
static Uint32 LinkedSDL3VersionInt = 0;


/* Obviously we can't use SDL_LoadObject() to load SDL3.  :)  */
/* FIXME: Updated library names after https://github.com/libsdl-org/SDL/issues/5626 solidifies.  */
static char loaderror[256];
#if defined(_WIN32)
    #define DIRSEP "\\"
    #define SDL3_LIBNAME "SDL3.dll"
    static HMODULE Loaded_SDL3 = NULL;
    #define LoadSDL3Library() ((Loaded_SDL3 = LoadLibraryA(SDL3_LIBNAME)) != NULL)
    #define LookupSDL3Sym(sym) (void *)GetProcAddress(Loaded_SDL3, sym)
    #define CloseSDL3Library() { if (Loaded_SDL3) { FreeLibrary(Loaded_SDL3); Loaded_SDL3 = NULL; } }
    #define strcpy_fn  lstrcpyA
    #define sprintf_fn wsprintfA
#elif defined(__APPLE__)
    #include <dlfcn.h>
    #include <pwd.h>
    #include <unistd.h>
    #define SDL3_LIBNAME "libSDL3.dylib"
    #define SDL3_FRAMEWORK "SDL3.framework/Versions/A/SDL3"
    #define strcpy_fn  strcpy
    #define sprintf_fn sprintf
    static void *Loaded_SDL3 = NULL;
    #define LookupSDL3Sym(sym) dlsym(Loaded_SDL3, sym)
    #define CloseSDL3Library() { if (Loaded_SDL3) { dlclose(Loaded_SDL3); Loaded_SDL3 = NULL; } }
    static SDL_bool LoadSDL3Library(void) {
        /* I don't know if this is the _right_ order to try, but this seems reasonable */
        static const char * const dylib_locations[] = {
            "@loader_path/" SDL3_LIBNAME, /* MyApp.app/Contents/MacOS/libSDL3.dylib */
            "@loader_path/../Frameworks/" SDL3_FRAMEWORK, /* MyApp.app/Contents/Frameworks/SDL2.framework */
            "@executable_path/" SDL3_LIBNAME, /* MyApp.app/Contents/MacOS/libSDL3.dylib */
            "@executable_path/../Frameworks/" SDL3_FRAMEWORK, /* MyApp.app/Contents/Frameworks/SDL2.framework */
            NULL,  /* /Users/username/Library/Frameworks/SDL2.framework */
            "/Library/Frameworks" SDL3_FRAMEWORK, /* /Library/Frameworks/SDL2.framework */
            SDL3_LIBNAME /* oh well, anywhere the system can see the .dylib (/usr/local/lib or whatever) */
        };

        int i;
        for (i = 0; i < (int) SDL_arraysize(dylib_locations); i++) {
            const char *location = dylib_locations[i];
            if (location) {
                Loaded_SDL3 = dlopen(location, RTLD_LOCAL|RTLD_NOW);
            } else { /* hack to mean "try homedir" */
                const char *homedir = NULL;
                struct passwd *pwent = getpwuid(getuid());
                if (pwent) {
                    homedir = pwent->pw_dir;
                }
                if (!homedir) {
                    homedir = getenv("HOME");
                }
                if (homedir) {
                    char framework[512];
                    const int rc = snprintf(framework, sizeof (framework), "%s/Library/Frameworks/" SDL3_FRAMEWORK, homedir);
                    if ((rc > 0) && (rc < (int) sizeof(framework))) {
                        Loaded_SDL3 = dlopen(framework, RTLD_LOCAL|RTLD_NOW);
                    }
                }
            }

            if (Loaded_SDL3) {
                return SDL_TRUE;
            }
        }

        return SDL_FALSE; /* didn't find it anywhere reasonable. :( */
    }
#elif defined(__unix__)
    #include <dlfcn.h>
    #define SDL3_LIBNAME "libSDL3.so.0"
    static void *Loaded_SDL3 = NULL;
    #define LoadSDL3Library() ((Loaded_SDL3 = dlopen(SDL3_LIBNAME, RTLD_LOCAL|RTLD_NOW)) != NULL)
    #define LookupSDL3Sym(sym) dlsym(Loaded_SDL3, sym)
    #define CloseSDL3Library() { if (Loaded_SDL3) { dlclose(Loaded_SDL3); Loaded_SDL3 = NULL; } }
    #define strcpy_fn  strcpy
    #define sprintf_fn sprintf
#else
    #error Please define your platform.
#endif

#ifndef SDL3_REQUIRED_VER
#define SDL3_REQUIRED_VER SDL_VERSIONNUM(3,0,0)
#endif

#ifndef DIRSEP
#define DIRSEP "/"
#endif

/* init stuff we want to do after SDL3 is loaded but before the app has access to it. */
static int SDL2Compat_InitOnStartup(void);


static void *
LoadSDL3Symbol(const char *fn, int *okay)
{
    void *retval = NULL;
    if (*okay) { /* only bother trying if we haven't previously failed. */
        retval = LookupSDL3Sym(fn);
        if (retval == NULL) {
            sprintf_fn(loaderror, "%s missing in SDL3 library.", fn);
            *okay = 0;
        }
    }
    return retval;
}

static void
UnloadSDL3(void)
{
    #define SDL3_SYM(rc,fn,params,args,ret) SDL3_##fn = NULL;
    #include "sdl3_syms.h"
    CloseSDL3Library();
}

typedef struct QuirkEntryType
{
    const char *exe_name;
    const char *hint_name;
    const char *hint_value;
} QuirkEntryType;

static QuirkEntryType quirks[] = {
    /* TODO: Add any quirks needed for various systems. */
    {"", "", "0"} /* A dummy entry to keep compilers happy. */
};

#ifdef __linux__
static void OS_GetExeName(char *buf, const unsigned maxpath) {
    buf[0] = '\0';
    readlink("/proc/self/exe", buf, maxpath);
}
#elif defined(_WIN32)
static void OS_GetExeName(char *buf, const unsigned maxpath) {
    buf[0] = '\0';
    GetModuleFileNameA(NULL, buf, maxpath);
}
#elif defined(__APPLE__) || defined(__FREEBSD__)
static void OS_GetExeName(char *buf, const unsigned maxpath) {
    const char *progname = getprogname();
    if (progname != NULL) {
        strlcpy(buf, progname, maxpath);
    } else {
        buf[0] = '\0';
    }
}
#else
#warning Please implement this for your platform.
static void OS_GetExeName(char *buf, const unsigned maxpath) {
    buf[0] = '\0';
    (void)maxpath;
}
#endif

static const char *
SDL2Compat_GetExeName(void)
{
    static const char *exename = NULL;
    if (exename == NULL) {
        static char path_buf[SDL12_MAXPATH];
        static char *base_path;
        OS_GetExeName(path_buf, SDL12_MAXPATH);
        base_path = SDL3_strrchr(path_buf, *DIRSEP);
        if (base_path) {
            /* We have a '\\' component. */
            exename = base_path + 1;
        } else {
            /* No slashes, return the whole module filanem. */
            exename = path_buf;
        }
    }
    return exename;
}

static const char *
SDL2Compat_GetHint(const char *name)
{
    return SDL3_getenv(name);
}

static SDL_bool
SDL2Compat_GetHintBoolean(const char *name, SDL_bool default_value)
{
    const char *val = SDL2Compat_GetHint(name);

    if (!val) {
        return default_value;
    }

    return (SDL3_atoi(val) != 0) ? SDL_TRUE : SDL_FALSE;
}

static void
SDL2Compat_ApplyQuirks(SDL_bool force_x11)
{
    const char *exe_name = SDL2Compat_GetExeName();
    int i;

    if (WantDebugLogging) {
        SDL3_Log("This app appears to be named '%s'", exe_name);
    }

    #ifdef __linux__
    if (force_x11) {
        const char *videodriver_env = SDL3_getenv("SDL_VIDEODRIVER");
        if (videodriver_env && (SDL3_strcmp(videodriver_env, "x11") != 0)) {
            if (WantDebugLogging) {
                SDL3_Log("This app looks like it requires X11, but the SDL_VIDEODRIVER environment variable is set to \"%s\". If you have issues, try setting SDL_VIDEODRIVER=x11", videodriver_env);
            }
        } else {
            if (WantDebugLogging) {
                SDL3_Log("sdl12-compat: We are forcing this app to use X11, because it probably talks to an X server directly, outside of SDL. If possible, this app should be fixed, to be compatible with Wayland, etc.");
            }
            SDL3_setenv("SDL_VIDEODRIVER", "x11", 1);
        }
    }
    #endif

    if (*exe_name == '\0') {
        return;
    }
    for (i = 0; i < (int) SDL_arraysize(quirks); i++) {
        if (!SDL3_strcmp(exe_name, quirks[i].exe_name)) {
            if (!SDL3_getenv(quirks[i].hint_name)) {
                if (WantDebugLogging) {
                    SDL3_Log("Applying compatibility quirk %s=\"%s\" for \"%s\"", quirks[i].hint_name, quirks[i].hint_value, exe_name);
                }
                SDL3_setenv(quirks[i].hint_name, quirks[i].hint_value, 1);
            } else {
                if (WantDebugLogging) {
                    SDL3_Log("Not applying compatibility quirk %s=\"%s\" for \"%s\" due to environment variable override (\"%s\")\n",
                            quirks[i].hint_name, quirks[i].hint_value, exe_name, SDL3_getenv(quirks[i].hint_name));
                }
            }
        }
    }
}

static int
LoadSDL3(void)
{
    int okay = 1;
    if (!Loaded_SDL3) {
        SDL_bool force_x11 = SDL_FALSE;

        #ifdef __linux__
        void *global_symbols = dlopen(NULL, RTLD_LOCAL|RTLD_NOW);

        /* Use linked libraries to detect what quirks we are likely to need */
        if (global_symbols != NULL) {
            if (dlsym(global_symbols, "glxewInit") != NULL) {  /* GLEW (e.g. Frogatto, SLUDGE) */
                force_x11 = SDL_TRUE;
            } else if (dlsym(global_symbols, "cgGLEnableProgramProfiles") != NULL) {  /* NVIDIA Cg (e.g. Awesomenauts, Braid) */
                force_x11 = SDL_TRUE;
            } else if (dlsym(global_symbols, "_Z7ssgInitv") != NULL) {  /* ::ssgInit(void) in plib (e.g. crrcsim) */
                force_x11 = SDL_TRUE;
            }
            dlclose(global_symbols);
        }
        #endif

        okay = LoadSDL3Library();
        if (!okay) {
            strcpy_fn(loaderror, "Failed loading SDL3 library.");
        } else {
            #define SDL3_SYM(rc,fn,params,args,ret) SDL3_##fn = (SDL3_##fn##_t) LoadSDL3Symbol("SDL_" #fn, &okay);
            #include "sdl3_syms.h"
            if (okay) {
                SDL_version v;
                SDL3_GetVersion(&v);
                LinkedSDL3VersionInt = SDL_VERSIONNUM(v.major, v.minor, v.patch);
                okay = (LinkedSDL3VersionInt >= SDL3_REQUIRED_VER);
                if (!okay) {
                    sprintf_fn(loaderror, "SDL3 %d.%d.%d library is too old.", v.major, v.minor, v.patch);
                } else {
                    WantDebugLogging = SDL2Compat_GetHintBoolean("SDL2COMPAT_DEBUG_LOGGING", SDL_FALSE);
                    if (WantDebugLogging) {
                        #if defined(__DATE__) && defined(__TIME__)
                        SDL3_Log("sdl2-compat 2.%d.%d, built on " __DATE__ " at " __TIME__ ", talking to SDL3 %d.%d.%d", SDL2_COMPAT_VERSION_MINOR, SDL2_COMPAT_VERSION_PATCH, v.major, v.minor, v.patch);
                        #else
                        SDL3_Log("sdl2-compat 2.%d.%d, talking to SDL3 %d.%d.%d", SDL2_COMPAT_VERSION_MINOR, SDL2_COMPAT_VERSION_PATCH, v.major, v.minor, v.patch);
                        #endif
                    }
                    SDL2Compat_ApplyQuirks(force_x11);  /* Apply and maybe print a list of any enabled quirks. */
                }
            }
            if (okay) {
                okay = SDL2Compat_InitOnStartup();
            }
            if (!okay) {
                UnloadSDL3();
            }
        }
    }
    return okay;
}

#if defined(_MSC_VER) && defined(_M_IX86)
#include "x86_msvc.h"
#endif

#if defined(_WIN32)
static void error_dialog(const char *errorMsg)
{
    MessageBoxA(NULL, errorMsg, "Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
}
#elif defined(__APPLE__)
extern void error_dialog(const char *errorMsg);
#else
static void error_dialog(const char *errorMsg)
{
    fprintf(stderr, "%s\n", errorMsg);
}
#endif

#if defined(__GNUC__) && !defined(_WIN32)
static void dllinit(void) __attribute__((constructor));
static void dllinit(void)
{
    if (!LoadSDL3()) {
        error_dialog(loaderror);
        abort();
    }
}
static void dllquit(void) __attribute__((destructor));
static void dllquit(void)
{
    UnloadSDL3();
}

#elif defined(_WIN32) && (defined(_MSC_VER) || defined(__MINGW32__) || defined(__WATCOMC__))
#if defined(_MSC_VER) && !defined(__FLTUSED__)
#define __FLTUSED__
__declspec(selectany) int _fltused = 1;
#endif
#if defined(__MINGW32__)
#define _DllMainCRTStartup DllMainCRTStartup
#endif
#if defined(__WATCOMC__)
#define _DllMainCRTStartup LibMain
#endif
BOOL WINAPI _DllMainCRTStartup(HANDLE dllhandle, DWORD reason, LPVOID reserved)
{
    (void) dllhandle;
    (void) reserved;
    switch (reason) {
    case DLL_PROCESS_DETACH:
        UnloadSDL3();
        break;

    case DLL_PROCESS_ATTACH: /* init once for each new process */
        if (!LoadSDL3()) {
            error_dialog(loaderror);
            #if 0
            TerminateProcess(GetCurrentProcess(), 42);
            ExitProcess(42);
            #endif
            return FALSE;
        }
        break;

    case DLL_THREAD_ATTACH: /* thread-specific init. */
    case DLL_THREAD_DETACH: /* thread-specific cleanup */
        break;
    }
    return TRUE;
}

#else
    #error Please define an init procedure for your platform.
#endif


/* this enum changed in SDL3. */
typedef enum
{
    SDL2_SYSWM_UNKNOWN,
    SDL2_SYSWM_WINDOWS,
    SDL2_SYSWM_X11,
    SDL2_SYSWM_DIRECTFB,
    SDL2_SYSWM_COCOA,
    SDL2_SYSWM_UIKIT,
    SDL2_SYSWM_WAYLAND,
    SDL2_SYSWM_MIR,
    SDL2_SYSWM_WINRT,
    SDL2_SYSWM_ANDROID,
    SDL2_SYSWM_VIVANTE,
    SDL2_SYSWM_OS2,
    SDL2_SYSWM_HAIKU,
    SDL2_SYSWM_KMSDRM,
    SDL2_SYSWM_RISCOS
} SDL2_SYSWM_TYPE;


/* Events changed in SDL3; notably, the `timestamp` field moved from
   32 bit milliseconds to 64-bit nanoseconds, and the padding of the union
   changed, so all the SDL2 structs have to be reproduced here. */

/* Note that SDL_EventType _currently_ lines up; although some types have
   come and gone in SDL3, so we don't manage an SDL2 copy here atm. */

typedef struct SDL2_CommonEvent
{
    Uint32 type;
    Uint32 timestamp;
} SDL2_CommonEvent;

/**
 *  \brief Display state change event data (event.display.*)
 */
typedef struct SDL2_DisplayEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 display;
    Uint8 event;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint32 data1;
} SDL2_DisplayEvent;

typedef struct SDL2_WindowEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint8 event;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint32 data1;
    Sint32 data2;
} SDL2_WindowEvent;

typedef struct SDL2_KeyboardEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint8 state;
    Uint8 repeat;
    Uint8 padding2;
    Uint8 padding3;
    SDL_Keysym keysym;
} SDL2_KeyboardEvent;

#define SDL2_TEXTEDITINGEVENT_TEXT_SIZE (32)
typedef struct SDL2_TextEditingEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    char text[SDL2_TEXTEDITINGEVENT_TEXT_SIZE];
    Sint32 start;
    Sint32 length;
} SDL2_TextEditingEvent;

typedef struct SDL2_TextEditingExtEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    char* text;
    Sint32 start;
    Sint32 length;
} SDL2_TextEditingExtEvent;

#define SDL2_TEXTINPUTEVENT_TEXT_SIZE (32)
typedef struct SDL2_TextInputEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    char text[SDL2_TEXTINPUTEVENT_TEXT_SIZE];
} SDL2_TextInputEvent;

typedef struct SDL2_MouseMotionEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Uint32 state;
    Sint32 x;
    Sint32 y;
    Sint32 xrel;
    Sint32 yrel;
} SDL2_MouseMotionEvent;

typedef struct SDL2_MouseButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Uint8 button;
    Uint8 state;
    Uint8 clicks;
    Uint8 padding1;
    Sint32 x;
    Sint32 y;
} SDL2_MouseButtonEvent;

typedef struct SDL2_MouseWheelEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Uint32 which;
    Sint32 x;
    Sint32 y;
    Uint32 direction;
    float preciseX;
    float preciseY;
    Sint32 mouseX;
    Sint32 mouseY;
} SDL2_MouseWheelEvent;

typedef struct SDL2_JoyAxisEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 axis;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;
    Uint16 padding4;
} SDL2_JoyAxisEvent;

typedef struct SDL2_JoyBallEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 ball;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 xrel;
    Sint16 yrel;
} SDL2_JoyBallEvent;

typedef struct SDL2_JoyHatEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 hat;
    Uint8 value;
    Uint8 padding1;
    Uint8 padding2;
} SDL2_JoyHatEvent;

typedef struct SDL2_JoyButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 button;
    Uint8 state;
    Uint8 padding1;
    Uint8 padding2;
} SDL2_JoyButtonEvent;

typedef struct SDL2_JoyDeviceEvent
{
    Uint32 type;
    Uint32 timestamp;
    Sint32 which;
} SDL2_JoyDeviceEvent;

typedef struct SDL2_JoyBatteryEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    SDL_JoystickPowerLevel level;
} SDL2_JoyBatteryEvent;

typedef struct SDL2_ControllerAxisEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 axis;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
    Sint16 value;
    Uint16 padding4;
} SDL2_ControllerAxisEvent;

typedef struct SDL2_ControllerButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Uint8 button;
    Uint8 state;
    Uint8 padding1;
    Uint8 padding2;
} SDL2_ControllerButtonEvent;

typedef struct SDL2_ControllerDeviceEvent
{
    Uint32 type;
    Uint32 timestamp;
    Sint32 which;
} SDL2_ControllerDeviceEvent;

typedef struct SDL2_ControllerTouchpadEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Sint32 touchpad;
    Sint32 finger;
    float x;
    float y;
    float pressure;
} SDL2_ControllerTouchpadEvent;

typedef struct SDL2_ControllerSensorEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_JoystickID which;
    Sint32 sensor;
    float data[3];
    Uint64 timestamp_us;
} SDL2_ControllerSensorEvent;

typedef struct SDL2_AudioDeviceEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 which;
    Uint8 iscapture;
    Uint8 padding1;
    Uint8 padding2;
    Uint8 padding3;
} SDL2_AudioDeviceEvent;

typedef struct SDL2_TouchFingerEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_TouchID touchId;
    SDL_FingerID fingerId;
    float x;
    float y;
    float dx;
    float dy;
    float pressure;
    Uint32 windowID;
} SDL2_TouchFingerEvent;

#define SDL_DOLLARGESTURE 0x800
#define SDL_DOLLARRECORD 0x801
#define SDL_MULTIGESTURE 0x802

typedef struct SDL2_MultiGestureEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_TouchID touchId;
    float dTheta;
    float dDist;
    float x;
    float y;
    Uint16 numFingers;
    Uint16 padding;
} SDL2_MultiGestureEvent;

typedef struct SDL2_DollarGestureEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL_TouchID touchId;
    SDL_GestureID gestureId;
    Uint32 numFingers;
    float error;
    float x;
    float y;
} SDL2_DollarGestureEvent;

typedef struct SDL2_DropEvent
{
    Uint32 type;
    Uint32 timestamp;
    char *file;
    Uint32 windowID;
} SDL2_DropEvent;

typedef struct SDL2_SensorEvent
{
    Uint32 type;
    Uint32 timestamp;
    Sint32 which;
    float data[6];
    Uint64 timestamp_us;
} SDL2_SensorEvent;

typedef struct SDL2_QuitEvent
{
    Uint32 type;
    Uint32 timestamp;
} SDL2_QuitEvent;

typedef struct SDL2_OSEvent
{
    Uint32 type;
    Uint32 timestamp;
} SDL2_OSEvent;

typedef struct SDL2_UserEvent
{
    Uint32 type;
    Uint32 timestamp;
    Uint32 windowID;
    Sint32 code;
    void *data1;
    void *data2;
} SDL2_UserEvent;

struct SDL2_SysWMmsg;
typedef struct SDL2_SysWMmsg SDL2_SysWMmsg;

typedef struct SDL2_SysWMEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL2_SysWMmsg *msg;
} SDL2_SysWMEvent;

typedef union SDL2_Event
{
    Uint32 type;
    SDL2_CommonEvent common;
    SDL2_DisplayEvent display;
    SDL2_WindowEvent window;
    SDL2_KeyboardEvent key;
    SDL2_TextEditingEvent edit;
    SDL2_TextEditingExtEvent editExt;
    SDL2_TextInputEvent text;
    SDL2_MouseMotionEvent motion;
    SDL2_MouseButtonEvent button;
    SDL2_MouseWheelEvent wheel;
    SDL2_JoyAxisEvent jaxis;
    SDL2_JoyBallEvent jball;
    SDL2_JoyHatEvent jhat;
    SDL2_JoyButtonEvent jbutton;
    SDL2_JoyDeviceEvent jdevice;
    SDL2_JoyBatteryEvent jbattery;
    SDL2_ControllerAxisEvent caxis;
    SDL2_ControllerButtonEvent cbutton;
    SDL2_ControllerDeviceEvent cdevice;
    SDL2_ControllerTouchpadEvent ctouchpad;
    SDL2_ControllerSensorEvent csensor;
    SDL2_AudioDeviceEvent adevice;
    SDL2_SensorEvent sensor;
    SDL2_QuitEvent quit;
    SDL2_UserEvent user;
    SDL2_SysWMEvent syswm;
    SDL2_TouchFingerEvent tfinger;
    SDL2_MultiGestureEvent mgesture;
    SDL2_DollarGestureEvent dgesture;
    SDL2_DropEvent drop;
    Uint8 padding[sizeof(void *) <= 8 ? 56 : sizeof(void *) == 16 ? 64 : 3 * sizeof(void *)];
} SDL2_Event;

/* Make sure we haven't broken binary compatibility */
SDL_COMPILE_TIME_ASSERT(SDL2_Event, sizeof(SDL2_Event) == sizeof(((SDL2_Event *)NULL)->padding));

typedef int (SDLCALL *SDL2_EventFilter) (void *userdata, SDL2_Event * event);

typedef struct EventFilterWrapperData
{
    SDL2_EventFilter filter2;
    void *userdata;
    struct EventFilterWrapperData *next;
} EventFilterWrapperData;


/* Some SDL2 state we need to keep... */

static SDL2_EventFilter EventFilter2 = NULL;
static void *EventFilterUserData2 = NULL;
static SDL_mutex *EventWatchListMutex = NULL;
static EventFilterWrapperData *EventWatchers2 = NULL;


/* Functions! */

static int SDLCALL EventFilter3to2(void *userdata, SDL_Event *event3);

/* this stuff _might_ move to SDL_Init later */
static int
SDL2Compat_InitOnStartup(void)
{
    int okay = 1;
    EventWatchListMutex = SDL3_CreateMutex();
    if (!EventWatchListMutex) {
        okay = 0;
    } else {
        SDL3_SetEventFilter(EventFilter3to2, NULL);
    }

    if (!okay) {
        strcpy_fn(loaderror, "Failed to initialize sdl2-compat library.");
    }
    return okay;
}


/* obviously we have to override this so we don't report ourselves as SDL3. */
DECLSPEC void SDLCALL
SDL_GetVersion(SDL_version * ver)
{
    if (ver) {
        ver->major = 2;
        ver->minor = SDL2_COMPAT_VERSION_MINOR;
        ver->patch = SDL2_COMPAT_VERSION_PATCH;
        if (SDL_GetHintBoolean("SDL_LEGACY_VERSION", SDL_FALSE)) {
            /* Prior to SDL 2.24.0, the patch version was incremented with every release */
            ver->patch = ver->minor;
            ver->minor = 0;
        }
    }
}

DECLSPEC int SDLCALL
SDL_GetRevisionNumber(void)
{
    /* After the move to GitHub, this always returned zero, since this was a
       Mercurial thing. We removed it outright in SDL3. */
    return 0;
}


DECLSPEC void SDLCALL
SDL_SetError(const char *fmt, ...)
{
    char ch;
    char *str = NULL;
    size_t len = 0;
    va_list ap;

    va_start(ap, fmt);
    len = SDL3_vsnprintf(&ch, 1, fmt, ap);
    va_end(ap);

    str = (char *) SDL3_malloc(len + 1);
    if (!str) {
        SDL3_OutOfMemory();
    } else {
        va_start(ap, fmt);
        SDL3_vsnprintf(str, len + 1, fmt, ap);
        va_end(ap);
        SDL3_SetError("%s", str);
        SDL3_free(str);
    }
}

DECLSPEC int SDLCALL
SDL_sscanf(const char *text, const char *fmt, ...)
{
    int retval;
    va_list ap;
    va_start(ap, fmt);
    retval = (int) SDL3_vsscanf(text, fmt, ap);
    va_end(ap);
    return retval;
}

DECLSPEC int SDLCALL
SDL_snprintf(char *text, size_t maxlen, const char *fmt, ...)
{
    int retval;
    va_list ap;
    va_start(ap, fmt);
    retval = (int) SDL3_vsnprintf(text, maxlen, fmt, ap);
    va_end(ap);
    return retval;
}

DECLSPEC int SDLCALL
SDL_asprintf(char **str, SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    int retval;
    va_list ap;
    va_start(ap, fmt);
    retval = (int) SDL3_vasprintf(str, fmt, ap);
    va_end(ap);
    return retval;
}

DECLSPEC void SDLCALL
SDL_Log(SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    SDL3_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt, ap);
    va_end(ap);
}

DECLSPEC void SDLCALL
SDL_LogMessage(int category, SDL_LogPriority priority, SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    SDL3_LogMessageV(category, priority, fmt, ap);
    va_end(ap);
}

#define SDL_LOG_IMPL(name, prio) \
    DECLSPEC void SDLCALL SDL_Log##name(int category, SDL_PRINTF_FORMAT_STRING const char *fmt, ...) { \
        va_list ap; va_start(ap, fmt); \
        SDL3_LogMessageV(category, SDL_LOG_PRIORITY_##prio, fmt, ap); \
        va_end(ap); \
    }
SDL_LOG_IMPL(Verbose, VERBOSE)
SDL_LOG_IMPL(Debug, DEBUG)
SDL_LOG_IMPL(Info, INFO)
SDL_LOG_IMPL(Warn, WARN)
SDL_LOG_IMPL(Error, ERROR)
SDL_LOG_IMPL(Critical, CRITICAL)
#undef SDL_LOG_IMPL


#if 0
static SDL2_SYSWM_TYPE
SysWmType3to2(const SDL_SYSWM_TYPE typ3)
{
    switch (typ3) {
        case SDL_SYSWM_UNKNOWN: return SDL2_SYSWM_UNKNOWN;
        case SDL_SYSWM_ANDROID: return SDL2_SYSWM_ANDROID;
        case SDL_SYSWM_COCOA: return SDL2_SYSWM_COCOA;
        case SDL_SYSWM_HAIKU: return SDL2_SYSWM_HAIKU;
        case SDL_SYSWM_KMSDRM: return SDL2_SYSWM_KMSDRM;
        case SDL_SYSWM_RISCOS: return SDL2_SYSWM_RISCOS;
        case SDL_SYSWM_UIKIT: return SDL2_SYSWM_UIKIT;
        case SDL_SYSWM_VIVANTE: return SDL2_SYSWM_VIVANTE;
        case SDL_SYSWM_WAYLAND: return SDL2_SYSWM_WAYLAND;
        case SDL_SYSWM_WINDOWS: return SDL2_SYSWM_WINDOWS;
        case SDL_SYSWM_WINRT: return SDL2_SYSWM_WINRT;
        case SDL_SYSWM_X11: return SDL2_SYSWM_X11;
        default: break;
    }
    return SDL2_SYSWM_UNKNOWN;
}
#endif


/* (current) strategy for SDL_Events:
   in sdl12-compat, we built our own event queue, so when the SDL2 queue is pumped, we
   took the events we cared about and added them to the sdl12-compat queue, and otherwise
   just cleared the real SDL2 queue when we were able.
   for sdl2-compat, we're going to try to use the SDL3 queue directly, and simply convert
   individual event structs when the SDL2-based app wants to consume or produce events.
   The queue has gotten to be significantly more complex in the SDL2 era, so rather than
   try to reproduce this (or outright copy this) and work in parallel with SDL3, we'll just
   try to work _with_ it. */

/* Note: as events change type (like the SDL2 window event is broken up into several events), we'll need to
   convert and push the SDL2 equivalent into the queue, but we don't care about new SDL3 event types, as
   any app could get an unknown event type anyhow, as SDL development progressed or a library registered
   a user event, etc, so we don't bother filtering new SDL3 types out. */

static SDL2_Event *
Event3to2(const SDL_Event *event3, SDL2_Event *event2)
{
#if 0
    if (event3->type == SDL_SYSWMEVENT) {
        return SDL_FALSE;  /* !!! FIXME: figure out what to do with this. */
    }
#endif

    /* currently everything _mostly_ matches up between SDL2 and SDL3, but this might
       drift more as SDL3 development continues. */

    /* for now, the timestamp field has grown in size (and precision), everything after it is currently the same, minus padding at the end, so bump the fields down. */
    event2->common.type = event3->type;
    event2->common.timestamp = (Uint32) SDL_NS_TO_MS(event3->common.timestamp);
    SDL3_memcpy((&event2->common) + 1, (&event3->common) + 1, sizeof (SDL2_Event) - sizeof (SDL2_CommonEvent));
    return event2;
}

static SDL_Event *
Event2to3(const SDL2_Event *event2, SDL_Event *event3)
{
#if 0
    if (event2->type == SDL_SYSWMEVENT) {
        return SDL_FALSE;  /* !!! FIXME: figure out what to do with this. */
    }
#endif

    /* currently everything _mostly_ matches up between SDL2 and SDL3, but this might
       drift more as SDL3 development continues. */

    /* for now, the timestamp field has grown in size (and precision), everything after it is currently the same, minus padding at the end, so bump the fields down. */
    event3->common.type = event2->type;
    event3->common.timestamp = (Uint64) SDL_MS_TO_NS(event2->common.timestamp);
    SDL3_memcpy((&event3->common) + 1, (&event2->common) + 1, sizeof (SDL_Event) - sizeof (SDL_CommonEvent));
    return event3;
}

static void GestureProcessEvent(const SDL_Event *event3);

static int SDLCALL
EventFilter3to2(void *userdata, SDL_Event *event3)
{
    SDL2_Event event2;  /* note that event filters do not receive events as const! So we have to convert or copy it for each one! */

    GestureProcessEvent(event3);  /* this might need to generate new gesture events from touch input. */

    if (EventFilter2) {
        return EventFilter2(EventFilterUserData2, Event3to2(event3, &event2));
    }

    /* !!! FIXME: eventually, push new events when we need to convert something, like toplevel SDL3 events generating the SDL2 SDL_WINDOWEVENT. */

    if (EventWatchers2 != NULL) {
        EventFilterWrapperData *i;
        SDL3_LockMutex(EventWatchListMutex);
        for (i = EventWatchers2; i != NULL; i = i->next) {
            i->filter2(i->userdata, Event3to2(event3, &event2));
        }
        SDL3_UnlockMutex(EventWatchListMutex);
    }
    return 1;
}


DECLSPEC void SDLCALL
SDL_SetEventFilter(SDL2_EventFilter filter2, void *userdata)
{
    EventFilter2 = filter2;
    EventFilterUserData2 = userdata;
}

DECLSPEC SDL_bool SDLCALL
SDL_GetEventFilter(SDL2_EventFilter *filter2, void **userdata)
{
    if (!EventFilter2) {
        return SDL_FALSE;
    }

    if (filter2) {
        *filter2 = EventFilter2;
    }

    if (userdata) {
        *userdata = EventFilterUserData2;
    }

    return SDL_TRUE;
}

DECLSPEC int SDLCALL
SDL_PeepEvents(SDL2_Event *events2, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType)
{
    SDL_Event *events3 = (SDL_Event *) SDL3_malloc(numevents * sizeof (SDL_Event));
    int retval = 0;
    int i;

    if (!events3) {
        return SDL_OutOfMemory();
    } else if (action == SDL_ADDEVENT) {
        for (i = 0; i < numevents; i++) {
            Event2to3(&events2[i], &events3[i]);
        }
        retval = SDL3_PeepEvents(events3, numevents, action, minType, maxType);
    } else {  /* SDL2 assumes it's SDL_PEEKEVENT if it isn't SDL_ADDEVENT or SDL_GETEVENT. */
        retval = SDL3_PeepEvents(events3, numevents, action, minType, maxType);
        for (i = 0; i < retval; i++) {
            Event3to2(&events3[i], &events2[i]);
        }
    }

    SDL_free(events3);
    return retval;
}

DECLSPEC int SDLCALL
SDL_WaitEventTimeout(SDL2_Event *event2, int timeout)
{
    SDL_Event event3;
    const int retval = SDL3_WaitEventTimeout(&event3, timeout);
    if (retval == 1) {
        Event3to2(&event3, event2);
    }
    return retval;
}

DECLSPEC int SDLCALL
SDL_PollEvent(SDL2_Event *event2)
{
    return SDL_WaitEventTimeout(event2, 0);
}

DECLSPEC int SDLCALL
SDL_WaitEvent(SDL2_Event *event2)
{
    return SDL_WaitEventTimeout(event2, -1);
}

DECLSPEC int SDLCALL
SDL_PushEvent(SDL2_Event *event2)
{
    SDL_Event event3;
    return SDL3_PushEvent(Event2to3(event2, &event3));
}

DECLSPEC void SDLCALL
SDL_AddEventWatch(SDL2_EventFilter filter2, void *userdata)
{
    /* we set up an SDL3 event filter to manage things already; we will also use it to call all added SDL2 event watchers. Put this new one in that list. */
    EventFilterWrapperData *wrapperdata = (EventFilterWrapperData *) SDL_malloc(sizeof (EventFilterWrapperData));
    if (!wrapperdata) {
        return;  /* oh well. */
    }
    wrapperdata->filter2 = filter2;
    wrapperdata->userdata = userdata;
    SDL3_LockMutex(EventWatchListMutex);
    wrapperdata->next = EventWatchers2;
    EventWatchers2 = wrapperdata;
    SDL3_UnlockMutex(EventWatchListMutex);
}

DECLSPEC void SDLCALL
SDL_DelEventWatch(SDL2_EventFilter filter2, void *userdata)
{
    EventFilterWrapperData *i;
    EventFilterWrapperData *prev = NULL;
    SDL3_LockMutex(EventWatchListMutex);
    for (i = EventWatchers2; i != NULL; i = i->next) {
        if ((i->filter2 == filter2) && (i->userdata == userdata)) {
            if (prev) {
                SDL_assert(i != EventWatchers2);
                prev->next = i->next;
            } else {
                SDL_assert(i == EventWatchers2);
                EventWatchers2 = i->next;
            }
            SDL3_free(i);
            break;
        }
    }
    SDL3_UnlockMutex(EventWatchListMutex);
}

static int SDLCALL
EventFilterWrapper3to2(void *userdata, SDL_Event *event)
{
    const EventFilterWrapperData *wrapperdata = (const EventFilterWrapperData *) userdata;
    SDL2_Event event2;
    return wrapperdata->filter2(wrapperdata->userdata, Event3to2(event, &event2));
}

DECLSPEC void SDLCALL
SDL_FilterEvents(SDL2_EventFilter filter2, void *userdata)
{
    EventFilterWrapperData wrapperdata = { filter2, userdata, NULL };
    SDL3_FilterEvents(EventFilterWrapper3to2, &wrapperdata);
}


/* stdio SDL_RWops was removed from SDL3, to prevent incompatible C runtime issues */
#if !HAVE_STDIO_H
DECLSPEC SDL_RWops * SDLCALL
SDL_RWFromFP(void *fp, SDL_bool autoclose)
{
    SDL3_SetError("SDL not compiled with stdio support");
    return NULL;
}
#else

/* !!! FIXME: SDL2 has a bunch of macro salsa to try and use the most 64-bit
fseek, etc, and I'm avoiding that for now; this can change if it becomes a
problem.  --ryan. */

/* Functions to read/write stdio file pointers */

static Sint64 SDLCALL
stdio_size(SDL_RWops * context)
{
    Sint64 pos, size;

    pos = SDL3_RWseek(context, 0, RW_SEEK_CUR);
    if (pos < 0) {
        return -1;
    }
    size = SDL3_RWseek(context, 0, RW_SEEK_END);

    SDL3_RWseek(context, pos, RW_SEEK_SET);
    return size;
}

static Sint64 SDLCALL
stdio_seek(SDL_RWops * context, Sint64 offset, int whence)
{
    FILE *fp = (FILE *) context->hidden.stdio.fp;
    int stdiowhence;

    switch (whence) {
    case RW_SEEK_SET:
        stdiowhence = SEEK_SET;
        break;
    case RW_SEEK_CUR:
        stdiowhence = SEEK_CUR;
        break;
    case RW_SEEK_END:
        stdiowhence = SEEK_END;
        break;
    default:
        return SDL3_SetError("Unknown value for 'whence'");
    }

#if defined(FSEEK_OFF_MIN) && defined(FSEEK_OFF_MAX)
    if (offset < (Sint64)(FSEEK_OFF_MIN) || offset > (Sint64)(FSEEK_OFF_MAX)) {
        return SDL3_SetError("Seek offset out of range");
    }
#endif

    if (fseek(fp, (long)offset, stdiowhence) == 0) {
        Sint64 pos = ftell(fp);
        if (pos < 0) {
            return SDL3_SetError("Couldn't get stream offset");
        }
        return pos;
    }
    return SDL3_Error(SDL_EFSEEK);
}

static size_t SDLCALL
stdio_read(SDL_RWops * context, void *ptr, size_t size, size_t maxnum)
{
    FILE *fp = (FILE *) context->hidden.stdio.fp;
    size_t nread;

    nread = fread(ptr, size, maxnum, fp);
    if (nread == 0 && ferror(fp)) {
        SDL3_Error(SDL_EFREAD);
    }
    return nread;
}

static size_t SDLCALL
stdio_write(SDL_RWops * context, const void *ptr, size_t size, size_t num)
{
    FILE *fp = (FILE *) context->hidden.stdio.fp;
    size_t nwrote;

    nwrote = fwrite(ptr, size, num, fp);
    if (nwrote == 0 && ferror(fp)) {
        SDL3_Error(SDL_EFWRITE);
    }
    return nwrote;
}

static int SDLCALL
stdio_close(SDL_RWops * context)
{
    int status = 0;
    if (context) {
        if (context->hidden.stdio.autoclose) {
            if (fclose((FILE *)context->hidden.stdio.fp) != 0) {
                status = SDL3_Error(SDL_EFWRITE);
            }
        }
        SDL3_FreeRW(context);
    }
    return status;
}

DECLSPEC SDL_RWops * SDLCALL
SDL_RWFromFP(FILE *fp, SDL_bool autoclose)
{
    SDL_RWops *rwops = SDL3_AllocRW();
    if (rwops != NULL) {
        rwops->size = stdio_size;
        rwops->seek = stdio_seek;
        rwops->read = stdio_read;
        rwops->write = stdio_write;
        rwops->close = stdio_close;
        rwops->hidden.stdio.fp = fp;
        rwops->hidden.stdio.autoclose = autoclose;
        rwops->type = SDL_RWOPS_STDFILE;
    }
    return rwops;
}
#endif


/* All gamma stuff was removed from SDL3 because it affects the whole system
   in intrusive ways, and often didn't work on various platforms. These all
   just return failure now. */

DECLSPEC int SDLCALL
SDL_SetWindowBrightness(SDL_Window *window, float brightness)
{
    return SDL3_Unsupported();
}

DECLSPEC float SDLCALL
SDL_GetWindowBrightness(SDL_Window *window)
{
    return 1.0f;
}

DECLSPEC int SDLCALL
SDL_SetWindowGammaRamp(SDL_Window *window, const Uint16 *r, const Uint16 *g, const Uint16 *b)
{
    return SDL3_Unsupported();
}

DECLSPEC void SDLCALL
SDL_CalculateGammaRamp(float gamma, Uint16 *ramp)
{
    int i;

    /* Input validation */
    if (gamma < 0.0f) {
      SDL_InvalidParamError("gamma");
      return;
    }
    if (ramp == NULL) {
      SDL_InvalidParamError("ramp");
      return;
    }

    /* 0.0 gamma is all black */
    if (gamma == 0.0f) {
        SDL_memset(ramp, 0, 256 * sizeof(Uint16));
        return;
    } else if (gamma == 1.0f) {
        /* 1.0 gamma is identity */
        for (i = 0; i < 256; ++i) {
            ramp[i] = (i << 8) | i;
        }
        return;
    } else {
        /* Calculate a real gamma ramp */
        int value;
        gamma = 1.0f / gamma;
        for (i = 0; i < 256; ++i) {
            value =
                (int) (SDL_pow((double) i / 256.0, gamma) * 65535.0 + 0.5);
            if (value > 65535) {
                value = 65535;
            }
            ramp[i] = (Uint16) value;
        }
    }
}

DECLSPEC int SDLCALL
SDL_GetWindowGammaRamp(SDL_Window *window, Uint16 *red, Uint16 *blue, Uint16 *green)
{
    Uint16 *buf = red ? red : (green ? green : blue);
    if (buf) {
        SDL_CalculateGammaRamp(1.0f, buf);
        if (red && (red != buf)) {
            SDL_memcpy(red, buf, 256 * sizeof (Uint16));
        }
        if (green && (green != buf)) {
            SDL_memcpy(green, buf, 256 * sizeof (Uint16));
        }
        if (blue && (blue != buf)) {
            SDL_memcpy(blue, buf, 256 * sizeof (Uint16));
        }
    }
    return 0;
}

DECLSPEC SDL_Surface * SDLCALL
SDL_ConvertSurface(SDL_Surface *src, const SDL_PixelFormat *fmt, Uint32 flags)
{
    (void) flags; /* SDL3 removed the (unused) `flags` argument */
    return SDL3_ConvertSurface(src, fmt);
}

DECLSPEC SDL_Surface * SDLCALL
SDL_ConvertSurfaceFormat(SDL_Surface * src, Uint32 pixel_format, Uint32 flags)
{
    (void) flags; /* SDL3 removed the (unused) `flags` argument */
    return SDL3_ConvertSurfaceFormat(src, pixel_format);
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return SDL3_CreateSurface(width, height,
            SDL3_MasksToPixelFormatEnum(depth, Rmask, Gmask, Bmask, Amask));
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth, Uint32 format)
{
    return SDL3_CreateSurface(width, height, format);
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return SDL3_CreateSurfaceFrom(pixels, width, height, pitch,
            SDL3_MasksToPixelFormatEnum(depth, Rmask, Gmask, Bmask, Amask));
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurfaceWithFormatFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 format)
{
    return SDL3_CreateSurfaceFrom(pixels, width, height, pitch, format);
}

/* SDL_GetTicks is 64-bit in SDL3. Clamp it for SDL2. */
DECLSPEC Uint32
SDLCALL SDL_GetTicks(void)
{
    return (Uint32)SDL3_GetTicks();
}

/* SDL_GetTicks64 is gone in SDL3 because SDL_GetTicks is 64-bit. */
DECLSPEC Uint64
SDLCALL SDL_GetTicks64(void)
{
    return SDL3_GetTicks();
}

DECLSPEC SDL_bool
SDL_GetWindowWMInfo(SDL_Window *window, SDL_SysWMinfo *wminfo)
{
    SDL3_Unsupported();  /* !!! FIXME: write me. */
    return SDL_FALSE;
}

/* this API was removed from SDL3 since nothing supported it. Just report 0. */
DECLSPEC int SDLCALL
SDL_JoystickNumBalls(SDL_Joystick *joystick)
{
    if (SDL3_JoystickNumAxes(joystick) == -1) {
        return -1;  /* just to call JOYSTICK_CHECK_MAGIC on `joystick`. */
    }
    return 0;
}

/* this API was removed from SDL3 since nothing supported it. Just report failure. */
DECLSPEC int SDLCALL
SDL_JoystickGetBall(SDL_Joystick *joystick, int ball, int *dx, int *dy)
{
    if (SDL3_JoystickNumAxes(joystick) == -1) {
        return -1;  /* just to call JOYSTICK_CHECK_MAGIC on `joystick`. */
    }
    return SDL3_SetError("Joystick only has 0 balls");
}


/* this API was removed in SDL3; use sensor event timestamps instead! */
DECLSPEC int SDLCALL
SDL_GameControllerGetSensorDataWithTimestamp(SDL_GameController *gamecontroller, SDL_SensorType type, Uint64 *timestamp, float *data, int num_values)
{
    return SDL3_Unsupported();  /* !!! FIXME: maybe try to track this from SDL3 events if something needs this? I can't imagine this was widely used. */
}

/* this API was removed in SDL3; use sensor event timestamps instead! */
DECLSPEC int SDLCALL
SDL_SensorGetDataWithTimestamp(SDL_Sensor *sensor, Uint64 *timestamp, float *data, int num_values)
{
    return SDL3_Unsupported();  /* !!! FIXME: maybe try to track this from SDL3 events if something needs this? I can't imagine this was widely used. */
}


/* Touch gestures were removed from SDL3, so this is the SDL2 implementation copied in here, and tweaked a little. */

#define GESTURE_MAX_DOLLAR_PATH_SIZE 1024
#define GESTURE_DOLLARNPOINTS 64
#define GESTURE_DOLLARSIZE 256
#define GESTURE_PHI        0.618033989

typedef struct
{
    float length;
    int numPoints;
    SDL_FPoint p[GESTURE_MAX_DOLLAR_PATH_SIZE];
} GestureDollarPath;

typedef struct
{
    SDL_FPoint path[GESTURE_DOLLARNPOINTS];
    unsigned long hash;
} GestureDollarTemplate;

typedef struct
{
    SDL_TouchID touchId;
    SDL_FPoint centroid;
    GestureDollarPath dollarPath;
    Uint16 numDownFingers;
    int numDollarTemplates;
    GestureDollarTemplate *dollarTemplate;
    SDL_bool recording;
} GestureTouch;

static GestureTouch *GestureTouches = NULL;
static int GestureNumTouches = 0;
static SDL_bool GestureRecordAll = SDL_FALSE;

static GestureTouch *GestureAddTouch(const SDL_TouchID touchId)
{
    GestureTouch *gestureTouch = (GestureTouch *)SDL3_realloc(GestureTouches, (GestureNumTouches + 1) * sizeof(GestureTouch));
    if (gestureTouch == NULL) {
        SDL3_OutOfMemory();
        return NULL;
    }

    GestureTouches = gestureTouch;
    SDL3_zero(GestureTouches[GestureNumTouches]);
    GestureTouches[GestureNumTouches].touchId = touchId;
    return &GestureTouches[GestureNumTouches++];
}

static int GestureDelTouch(const SDL_TouchID touchId)
{
    int i;
    for (i = 0; i < GestureNumTouches; i++) {
        if (GestureTouches[i].touchId == touchId) {
            break;
        }
    }

    if (i == GestureNumTouches) {
        /* not found */
        return -1;
    }

    SDL3_free(GestureTouches[i].dollarTemplate);
    SDL3_zero(GestureTouches[i]);

    GestureNumTouches--;
    if (i != GestureNumTouches) {
        SDL3_copyp(&GestureTouches[i], &GestureTouches[GestureNumTouches]);
    }
    return 0;
}

static GestureTouch *GestureGetTouch(const SDL_TouchID touchId)
{
    int i;
    for (i = 0; i < GestureNumTouches; i++) {
        /* printf("%i ?= %i\n",GestureTouches[i].touchId,touchId); */
        if (GestureTouches[i].touchId == touchId) {
            return &GestureTouches[i];
        }
    }
    return NULL;
}

DECLSPEC int SDLCALL
SDL_RecordGesture(SDL_TouchID touchId)
{
    const int numtouchdevs = SDL3_GetNumTouchDevices();
    int i;

    /* make sure we know about all the devices SDL3 knows about, since we aren't connected as tightly as we were in SDL2. */
    for (i = 0; i < numtouchdevs; i++) {
        const SDL_TouchID thistouch = SDL3_GetTouchDevice(i);
        if (!GestureGetTouch(thistouch)) {
            if (!GestureAddTouch(thistouch)) {
                return 0;  /* uhoh, out of memory */
            }
        }
    }

    if (touchId < 0) {
        GestureRecordAll = SDL_TRUE;  /* !!! FIXME: this is never set back to SDL_FALSE anywhere, that's probably a bug. */
        for (i = 0; i < GestureNumTouches; i++) {
            GestureTouches[i].recording = SDL_TRUE;
        }
    } else {
        GestureTouch *touch = GestureGetTouch(touchId);
        if (!touch) {
            return 0;  /* bogus touchid */
        }
        touch->recording = SDL_TRUE;
    }

    return 1;
}

/* !!! FIXME: we need to hook this up when we override SDL_Quit */
static void GestureQuit(void)
{
    SDL3_free(GestureTouches);
    GestureTouches = NULL;
}

static unsigned long GestureHashDollar(SDL_FPoint *points)
{
    unsigned long hash = 5381;
    int i;
    for (i = 0; i < GESTURE_DOLLARNPOINTS; i++) {
        hash = ((hash << 5) + hash) + (unsigned long)points[i].x;
        hash = ((hash << 5) + hash) + (unsigned long)points[i].y;
    }
    return hash;
}

static int GestureSaveTemplate(GestureDollarTemplate *templ, SDL_RWops *dst)
{
    if (dst == NULL) {
        return 0;
    }

    /* No Longer storing the Hash, rehash on load */
    /* if (SDL_RWops.write(dst, &(templ->hash), sizeof(templ->hash), 1) != 1) return 0; */

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    if (SDL3_RWwrite(dst, templ->path, sizeof(templ->path[0]), GESTURE_DOLLARNPOINTS) != GESTURE_DOLLARNPOINTS) {
        return 0;
    }
#else
    {
        GestureDollarTemplate copy = *templ;
        SDL_FPoint *p = copy.path;
        int i;
        for (i = 0; i < GESTURE_DOLLARNPOINTS; i++, p++) {
            p->x = SDL_SwapFloatLE(p->x);
            p->y = SDL_SwapFloatLE(p->y);
        }

        if (SDL3_RWwrite(dst, copy.path, sizeof(copy.path[0]), GESTURE_DOLLARNPOINTS) != GESTURE_DOLLARNPOINTS) {
            return 0;
        }
    }
#endif

    return 1;
}

DECLSPEC int SDLCALL
SDL_SaveAllDollarTemplates(SDL_RWops *dst)
{
    int i, j, rtrn = 0;
    for (i = 0; i < GestureNumTouches; i++) {
        GestureTouch *touch = &GestureTouches[i];
        for (j = 0; j < touch->numDollarTemplates; j++) {
            rtrn += GestureSaveTemplate(&touch->dollarTemplate[j], dst);
        }
    }
    return rtrn;
}

DECLSPEC int SDLCALL
SDL_SaveDollarTemplate(SDL_GestureID gestureId, SDL_RWops *dst)
{
    int i, j;
    for (i = 0; i < GestureNumTouches; i++) {
        GestureTouch *touch = &GestureTouches[i];
        for (j = 0; j < touch->numDollarTemplates; j++) {
            if (touch->dollarTemplate[j].hash == gestureId) {
                return GestureSaveTemplate(&touch->dollarTemplate[j], dst);
            }
        }
    }
    return SDL3_SetError("Unknown gestureId");
}

/* path is an already sampled set of points
Returns the index of the gesture on success, or -1 */
static int GestureAddDollar_one(GestureTouch *inTouch, SDL_FPoint *path)
{
    GestureDollarTemplate *dollarTemplate;
    GestureDollarTemplate *templ;
    int index;

    index = inTouch->numDollarTemplates;
    dollarTemplate = (GestureDollarTemplate *)SDL3_realloc(inTouch->dollarTemplate, (index + 1) * sizeof(GestureDollarTemplate));
    if (dollarTemplate == NULL) {
        return SDL3_OutOfMemory();
    }
    inTouch->dollarTemplate = dollarTemplate;

    templ = &inTouch->dollarTemplate[index];
    SDL3_memcpy(templ->path, path, GESTURE_DOLLARNPOINTS * sizeof(SDL_FPoint));
    templ->hash = GestureHashDollar(templ->path);
    inTouch->numDollarTemplates++;

    return index;
}

static int GestureAddDollar(GestureTouch *inTouch, SDL_FPoint *path)
{
    int index = -1;
    int i = 0;
    if (inTouch == NULL) {
        if (GestureNumTouches == 0) {
            return SDL3_SetError("no gesture touch devices registered");
        }
        for (i = 0; i < GestureNumTouches; i++) {
            inTouch = &GestureTouches[i];
            index = GestureAddDollar_one(inTouch, path);
            if (index < 0) {
                return -1;
            }
        }
        /* Use the index of the last one added. */
        return index;
    }
    return GestureAddDollar_one(inTouch, path);
}

DECLSPEC int SDLCALL
SDL_LoadDollarTemplates(SDL_TouchID touchId, SDL_RWops *src)
{
    int i, loaded = 0;
    GestureTouch *touch = NULL;
    if (src == NULL) {
        return 0;
    }
    if (touchId >= 0) {
        for (i = 0; i < GestureNumTouches; i++) {
            if (GestureTouches[i].touchId == touchId) {
                touch = &GestureTouches[i];
            }
        }
        if (touch == NULL) {
            return SDL3_SetError("given touch id not found");
        }
    }

    while (1) {
        GestureDollarTemplate templ;

        if (SDL3_RWread(src, templ.path, sizeof(templ.path[0]), GESTURE_DOLLARNPOINTS) < GESTURE_DOLLARNPOINTS) {
            if (loaded == 0) {
                return SDL3_SetError("could not read any dollar gesture from rwops");
            }
            break;
        }

#if SDL_BYTEORDER != SDL_LIL_ENDIAN
        for (i = 0; i < GESTURE_DOLLARNPOINTS; i++) {
            SDL_FPoint *p = &templ.path[i];
            p->x = SDL_SwapFloatLE(p->x);
            p->y = SDL_SwapFloatLE(p->y);
        }
#endif

        if (touchId >= 0) {
            /* printf("Adding loaded gesture to 1 touch\n"); */
            if (GestureAddDollar(touch, templ.path) >= 0) {
                loaded++;
            }
        } else {
            /* printf("Adding to: %i touches\n",GestureNumTouches); */
            for (i = 0; i < GestureNumTouches; i++) {
                touch = &GestureTouches[i];
                /* printf("Adding loaded gesture to + touches\n"); */
                /* TODO: What if this fails? */
                GestureAddDollar(touch, templ.path);
            }
            loaded++;
        }
    }

    return loaded;
}

static float GestureDollarDifference(SDL_FPoint *points, SDL_FPoint *templ, float ang)
{
    /*  SDL_FPoint p[GESTURE_DOLLARNPOINTS]; */
    float dist = 0;
    SDL_FPoint p;
    int i;
    for (i = 0; i < GESTURE_DOLLARNPOINTS; i++) {
        p.x = points[i].x * SDL3_cosf(ang) - points[i].y * SDL_sinf(ang);
        p.y = points[i].x * SDL3_sinf(ang) + points[i].y * SDL_cosf(ang);
        dist += SDL_sqrtf((p.x - templ[i].x) * (p.x - templ[i].x) + (p.y - templ[i].y) * (p.y - templ[i].y));
    }
    return dist / GESTURE_DOLLARNPOINTS;
}

static float GestureBestDollarDifference(SDL_FPoint *points, SDL_FPoint *templ)
{
    /*------------BEGIN DOLLAR BLACKBOX------------------
      -TRANSLATED DIRECTLY FROM PSUDEO-CODE AVAILABLE AT-
      -"http://depts.washington.edu/aimgroup/proj/dollar/"
    */
    double ta = -SDL_PI_D / 4;
    double tb = SDL_PI_D / 4;
    double dt = SDL_PI_D / 90;
    float x1 = (float)(GESTURE_PHI * ta + (1 - GESTURE_PHI) * tb);
    float f1 = GestureDollarDifference(points, templ, x1);
    float x2 = (float)((1 - GESTURE_PHI) * ta + GESTURE_PHI * tb);
    float f2 = GestureDollarDifference(points, templ, x2);
    while (SDL3_fabs(ta - tb) > dt) {
        if (f1 < f2) {
            tb = x2;
            x2 = x1;
            f2 = f1;
            x1 = (float)(GESTURE_PHI * ta + (1 - GESTURE_PHI) * tb);
            f1 = GestureDollarDifference(points, templ, x1);
        } else {
            ta = x1;
            x1 = x2;
            f1 = f2;
            x2 = (float)((1 - GESTURE_PHI) * ta + GESTURE_PHI * tb);
            f2 = GestureDollarDifference(points, templ, x2);
        }
    }
    /*
      if (f1 <= f2)
          printf("Min angle (x1): %f\n",x1);
      else if (f1 >  f2)
          printf("Min angle (x2): %f\n",x2);
    */
    return SDL_min(f1, f2);
}

/* `path` contains raw points, plus (possibly) the calculated length */
static int GestureDollarNormalize(const GestureDollarPath *path, SDL_FPoint *points, SDL_bool is_recording)
{
    int i;
    float interval;
    float dist;
    int numPoints = 0;
    SDL_FPoint centroid;
    float xmin, xmax, ymin, ymax;
    float ang;
    float w, h;
    float length = path->length;

    /* Calculate length if it hasn't already been done */
    if (length <= 0) {
        for (i = 1; i < path->numPoints; i++) {
            const float dx = path->p[i].x - path->p[i - 1].x;
            const float dy = path->p[i].y - path->p[i - 1].y;
            length += SDL3_sqrtf(dx * dx + dy * dy);
        }
    }

    /* Resample */
    interval = length / (GESTURE_DOLLARNPOINTS - 1);
    dist = interval;

    centroid.x = 0;
    centroid.y = 0;

    /* printf("(%f,%f)\n",path->p[path->numPoints-1].x,path->p[path->numPoints-1].y); */
    for (i = 1; i < path->numPoints; i++) {
        const float d = SDL3_sqrtf((path->p[i - 1].x - path->p[i].x) * (path->p[i - 1].x - path->p[i].x) + (path->p[i - 1].y - path->p[i].y) * (path->p[i - 1].y - path->p[i].y));
        /* printf("d = %f dist = %f/%f\n",d,dist,interval); */
        while (dist + d > interval) {
            points[numPoints].x = path->p[i - 1].x +
                                  ((interval - dist) / d) * (path->p[i].x - path->p[i - 1].x);
            points[numPoints].y = path->p[i - 1].y +
                                  ((interval - dist) / d) * (path->p[i].y - path->p[i - 1].y);
            centroid.x += points[numPoints].x;
            centroid.y += points[numPoints].y;
            numPoints++;

            dist -= interval;
        }
        dist += d;
    }
    if (numPoints < GESTURE_DOLLARNPOINTS - 1) {
        if (is_recording) {
            SDL3_SetError("ERROR: NumPoints = %i", numPoints);
        }
        return 0;
    }
    /* copy the last point */
    points[GESTURE_DOLLARNPOINTS - 1] = path->p[path->numPoints - 1];
    numPoints = GESTURE_DOLLARNPOINTS;

    centroid.x /= numPoints;
    centroid.y /= numPoints;

    /* printf("Centroid (%f,%f)",centroid.x,centroid.y); */
    /* Rotate Points so point 0 is left of centroid and solve for the bounding box */
    xmin = centroid.x;
    xmax = centroid.x;
    ymin = centroid.y;
    ymax = centroid.y;

    ang = SDL3_atan2f(centroid.y - points[0].y, centroid.x - points[0].x);

    for (i = 0; i < numPoints; i++) {
        const float px = points[i].x;
        const float py = points[i].y;
        points[i].x = (px - centroid.x) * SDL3_cosf(ang) - (py - centroid.y) * SDL3_sinf(ang) + centroid.x;
        points[i].y = (px - centroid.x) * SDL3_sinf(ang) + (py - centroid.y) * SDL3_cosf(ang) + centroid.y;

        if (points[i].x < xmin) {
            xmin = points[i].x;
        }
        if (points[i].x > xmax) {
            xmax = points[i].x;
        }
        if (points[i].y < ymin) {
            ymin = points[i].y;
        }
        if (points[i].y > ymax) {
            ymax = points[i].y;
        }
    }

    /* Scale points to GESTURE_DOLLARSIZE, and translate to the origin */
    w = xmax - xmin;
    h = ymax - ymin;

    for (i = 0; i < numPoints; i++) {
        points[i].x = (points[i].x - centroid.x) * GESTURE_DOLLARSIZE / w;
        points[i].y = (points[i].y - centroid.y) * GESTURE_DOLLARSIZE / h;
    }
    return numPoints;
}

static float GestureDollarRecognize(const GestureDollarPath *path, int *bestTempl, GestureTouch *touch)
{
    SDL_FPoint points[GESTURE_DOLLARNPOINTS];
    int i;
    float bestDiff = 10000;

    SDL3_memset(points, 0, sizeof(points));

    GestureDollarNormalize(path, points, SDL_FALSE);

    /* PrintPath(points); */
    *bestTempl = -1;
    for (i = 0; i < touch->numDollarTemplates; i++) {
        const float diff = GestureBestDollarDifference(points, touch->dollarTemplate[i].path);
        if (diff < bestDiff) {
            bestDiff = diff;
            *bestTempl = i;
        }
    }
    return bestDiff;
}

static void GestureSendMulti(GestureTouch *touch, float dTheta, float dDist)
{
    if (SDL3_GetEventState(SDL_MULTIGESTURE) == SDL_ENABLE) {
        SDL2_Event event;
        event.type = SDL_MULTIGESTURE;
        event.common.timestamp = 0;
        event.mgesture.touchId = touch->touchId;
        event.mgesture.x = touch->centroid.x;
        event.mgesture.y = touch->centroid.y;
        event.mgesture.dTheta = dTheta;
        event.mgesture.dDist = dDist;
        event.mgesture.numFingers = touch->numDownFingers;
        SDL_PushEvent(&event);
    }
}

static void GestureSendDollar(GestureTouch *touch, SDL_GestureID gestureId, float error)
{
    if (SDL3_GetEventState(SDL_DOLLARGESTURE) == SDL_ENABLE) {
        SDL2_Event event;
        event.type = SDL_DOLLARGESTURE;
        event.common.timestamp = 0;
        event.dgesture.touchId = touch->touchId;
        event.dgesture.x = touch->centroid.x;
        event.dgesture.y = touch->centroid.y;
        event.dgesture.gestureId = gestureId;
        event.dgesture.error = error;
        /* A finger came up to trigger this event. */
        event.dgesture.numFingers = touch->numDownFingers + 1;
        SDL_PushEvent(&event);
    }
}

static void GestureSendDollarRecord(GestureTouch *touch, SDL_GestureID gestureId)
{
    if (SDL3_GetEventState(SDL_DOLLARRECORD) == SDL_ENABLE) {
        SDL2_Event event;
        event.type = SDL_DOLLARRECORD;
        event.common.timestamp = 0;
        event.dgesture.touchId = touch->touchId;
        event.dgesture.gestureId = gestureId;
        SDL_PushEvent(&event);
    }
}

/* These are SDL3 events coming in from sdl2-compat's event watcher. */
static void GestureProcessEvent(const SDL_Event *event3)
{
    float x, y;
    int index;
    int i;
    float pathDx, pathDy;
    SDL_FPoint lastP;
    SDL_FPoint lastCentroid;
    float lDist;
    float Dist;
    float dtheta;
    float dDist;

    if (event3->type == SDL_FINGERMOTION || event3->type == SDL_FINGERDOWN || event3->type == SDL_FINGERUP) {
        GestureTouch *inTouch = GestureGetTouch(event3->tfinger.touchId);

        if (inTouch == NULL) {  /* we maybe didn't see this one before. */
            inTouch = GestureAddTouch(event3->tfinger.touchId);
            if (!inTouch) {
                return;  /* oh well. */
            }
        }

        x = event3->tfinger.x;
        y = event3->tfinger.y;

        /* Finger Up */
        if (event3->type == SDL_FINGERUP) {
            SDL_FPoint path[GESTURE_DOLLARNPOINTS];
            inTouch->numDownFingers--;

            if (inTouch->recording) {
                inTouch->recording = SDL_FALSE;
                GestureDollarNormalize(&inTouch->dollarPath, path, SDL_TRUE);
                /* PrintPath(path); */
                if (GestureRecordAll) {
                    index = GestureAddDollar(NULL, path);
                    for (i = 0; i < GestureNumTouches; i++) {
                        GestureTouches[i].recording = SDL_FALSE;
                    }
                } else {
                    index = GestureAddDollar(inTouch, path);
                }

                if (index >= 0) {
                    GestureSendDollarRecord(inTouch, inTouch->dollarTemplate[index].hash);
                } else {
                    GestureSendDollarRecord(inTouch, -1);
                }
            } else {
                int bestTempl = -1;
                const float error = GestureDollarRecognize(&inTouch->dollarPath, &bestTempl, inTouch);
                if (bestTempl >= 0) {
                    /* Send Event */
                    const unsigned long gestureId = inTouch->dollarTemplate[bestTempl].hash;
                    GestureSendDollar(inTouch, gestureId, error);
                    /* printf ("%s\n",);("Dollar error: %f\n",error); */
                }
            }

            /* inTouch->gestureLast[j] = inTouch->gestureLast[inTouch->numDownFingers]; */
            if (inTouch->numDownFingers > 0) {
                inTouch->centroid.x = (inTouch->centroid.x * (inTouch->numDownFingers + 1) - x) / inTouch->numDownFingers;
                inTouch->centroid.y = (inTouch->centroid.y * (inTouch->numDownFingers + 1) - y) / inTouch->numDownFingers;
            }
        } else if (event3->type == SDL_FINGERMOTION) {
            const float dx = event3->tfinger.dx;
            const float dy = event3->tfinger.dy;
            GestureDollarPath *path = &inTouch->dollarPath;
            if (path->numPoints < GESTURE_MAX_DOLLAR_PATH_SIZE) {
                path->p[path->numPoints].x = inTouch->centroid.x;
                path->p[path->numPoints].y = inTouch->centroid.y;
                pathDx = (path->p[path->numPoints].x - path->p[path->numPoints - 1].x);
                pathDy = (path->p[path->numPoints].y - path->p[path->numPoints - 1].y);
                path->length += (float)SDL3_sqrt(pathDx * pathDx + pathDy * pathDy);
                path->numPoints++;
            }

            lastP.x = x - dx;
            lastP.y = y - dy;
            lastCentroid = inTouch->centroid;

            inTouch->centroid.x += dx / inTouch->numDownFingers;
            inTouch->centroid.y += dy / inTouch->numDownFingers;
            /* printf("Centrid : (%f,%f)\n",inTouch->centroid.x,inTouch->centroid.y); */
            if (inTouch->numDownFingers > 1) {
                SDL_FPoint lv; /* Vector from centroid to last x,y position */
                SDL_FPoint v;  /* Vector from centroid to current x,y position */
                /* lv = inTouch->gestureLast[j].cv; */
                lv.x = lastP.x - lastCentroid.x;
                lv.y = lastP.y - lastCentroid.y;
                lDist = SDL3_sqrtf(lv.x * lv.x + lv.y * lv.y);
                /* printf("lDist = %f\n",lDist); */
                v.x = x - inTouch->centroid.x;
                v.y = y - inTouch->centroid.y;
                /* inTouch->gestureLast[j].cv = v; */
                Dist = SDL3_sqrtf(v.x * v.x + v.y * v.y);
                /* SDL3_cosf(dTheta) = (v . lv)/(|v| * |lv|) */

                /* Normalize Vectors to simplify angle calculation */
                lv.x /= lDist;
                lv.y /= lDist;
                v.x /= Dist;
                v.y /= Dist;
                dtheta = SDL3_atan2f(lv.x * v.y - lv.y * v.x, lv.x * v.x + lv.y * v.y);

                dDist = (Dist - lDist);
                if (lDist == 0) {
                    /* To avoid impossible values */
                    dDist = 0;
                    dtheta = 0;
                }

                /* inTouch->gestureLast[j].dDist = dDist;
                inTouch->gestureLast[j].dtheta = dtheta;

                printf("dDist = %f, dTheta = %f\n",dDist,dtheta);
                gdtheta = gdtheta*.9 + dtheta*.1;
                gdDist  =  gdDist*.9 +  dDist*.1
                knob.r += dDist/numDownFingers;
                knob.ang += dtheta;
                printf("thetaSum = %f, distSum = %f\n",gdtheta,gdDist);
                printf("id: %i dTheta = %f, dDist = %f\n",j,dtheta,dDist); */
                GestureSendMulti(inTouch, dtheta, dDist);
            } else {
                /* inTouch->gestureLast[j].dDist = 0;
                inTouch->gestureLast[j].dtheta = 0;
                inTouch->gestureLast[j].cv.x = 0;
                inTouch->gestureLast[j].cv.y = 0; */
            }
            /* inTouch->gestureLast[j].f.p.x = x;
            inTouch->gestureLast[j].f.p.y = y;
            break;
            pressure? */
        } else if (event3->type == SDL_FINGERDOWN) {
            inTouch->numDownFingers++;
            inTouch->centroid.x = (inTouch->centroid.x * (inTouch->numDownFingers - 1) +
                                   x) /
                                  inTouch->numDownFingers;
            inTouch->centroid.y = (inTouch->centroid.y * (inTouch->numDownFingers - 1) +
                                   y) /
                                  inTouch->numDownFingers;
            /* printf("Finger Down: (%f,%f). Centroid: (%f,%f\n",x,y,
                 inTouch->centroid.x,inTouch->centroid.y); */

            inTouch->dollarPath.length = 0;
            inTouch->dollarPath.p[0].x = x;
            inTouch->dollarPath.p[0].y = y;
            inTouch->dollarPath.numPoints = 1;
        }
    }
}


DECLSPEC int SDLCALL
SDL_GetRenderDriverInfo(int index, SDL_RendererInfo * info)
{
    const char *name = SDL3_GetRenderDriver(index);
    if (!name) {
        return -1;  /* assume SDL3_GetRenderDriver set the SDL error. */
    }

    SDL_zerop(info);
    info->name = name;

    /* these are the values that SDL2 returns. */
    if ((SDL3_strcmp(name, "opengl") == 0) || (SDL3_strcmp(name, "opengles2") == 0)) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 4;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_ABGR8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_RGB888;
        info->texture_formats[3] = SDL_PIXELFORMAT_BGR888;
    } else if (SDL3_strcmp(name, "opengles") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ABGR8888;
    } else if (SDL3_strcmp(name, "direct3d") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
    } else if (SDL3_strcmp(name, "direct3d11") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_RGB888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
    } else if (SDL3_strcmp(name, "direct3d12") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_RGB888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
        info->max_texture_width = 16384;
        info->max_texture_height = 16384;
    } else if (SDL3_strcmp(name, "metal") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_ABGR8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
    } else if (SDL3_strcmp(name, "software") == 0) {
        info->flags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
    } else {  /* this seems reasonable if something currently-unknown shows up in SDL3. */
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
    }
    return 0;
}

/* Second parameter changed from an index to a string in SDL3. */
DECLSPEC SDL_Renderer *SDLCALL
SDL_CreateRenderer(SDL_Window *window, int index, Uint32 flags)
{
    const char *name = NULL;
    if (index != -1) {
        name = SDL3_GetRenderDriver(index);
        if (!name) {
            return NULL;  /* assume SDL3_GetRenderDriver set the SDL error. */
        }
    }
    return SDL3_CreateRenderer(window, name, flags);
}

#ifdef __cplusplus
}
#endif

/* vi: set ts=4 sw=4 expandtab: */
