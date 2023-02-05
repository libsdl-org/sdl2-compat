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
#include "sdl2_compat.h"
#include "dynapi/SDL_dynapi.h"


#if defined(DisableScreenSaver)
/*
This breaks the build when creating SDL_ ## DisableScreenSaver
/usr/include/X11/X.h:#define DisableScreenSaver	0
*/
#undef DisableScreenSaver
#endif


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
#undef HAVE_STDIO_H
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

#ifdef __linux__
#include <unistd.h> /* for readlink() */
#endif

#if defined(__unix__) || defined(__APPLE__)
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
#define SDL2COMPAT_MAXPATH PATH_MAX
#elif defined _WIN32
#define SDL2COMPAT_MAXPATH MAX_PATH
#else
#define SDL2COMPAT_MAXPATH 1024
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Enable this to have warnings about wrong prototypes of src/sdl3_sym.h
 *  It won't compile but it helps to make sure it's sync'ed with SDL3 headers.
 */
#if 0
#define SDL3_SYM(rc,fn,params,args,ret) \
    typedef rc (SDLCALL *SDL3_##fn##_t) params; \
    static SDL3_##fn##_t SDL3_##fn = IGNORE_THIS_VERSION_OF_SDL_##fn;
#else
#define SDL3_SYM(rc,fn,params,args,ret) \
    typedef rc (SDLCALL *SDL3_##fn##_t) params; \
    static SDL3_##fn##_t SDL3_##fn = NULL;
#endif
#include "sdl3_syms.h"

/* Things that _should_ be binary compatible pass right through... */
#define SDL3_SYM_PASSTHROUGH(rc,fn,params,args,ret) \
    DECLSPEC rc SDLCALL SDL_##fn params { ret SDL3_##fn args; }
#include "sdl3_syms.h"

/* Things that were renamed and _should_ be binary compatible pass right through with the correct names... */
#define SDL3_SYM_RENAMED(rc,oldfn,newfn,params,args,ret) \
    DECLSPEC rc SDLCALL SDL_##oldfn params { ret SDL3_##newfn args; }
#include "sdl3_syms.h"


/* these are macros (etc) in the SDL headers, so make our own. */
#define SDL3_OutOfMemory() SDL3_Error(SDL_ENOMEM)
#define SDL3_Unsupported() SDL3_Error(SDL_UNSUPPORTED)
#define SDL3_InvalidParamError(param) SDL3_SetError("Parameter '%s' is invalid", (param))
#define SDL3_zero(x) SDL3_memset(&(x), 0, sizeof((x)))
#define SDL3_zerop(x) SDL3_memset((x), 0, sizeof(*(x)))
#define SDL3_zeroa(x) SDL3_memset((x), 0, sizeof((x)))
#define SDL3_copyp(dst, src)                                                    \
    { SDL_COMPILE_TIME_ASSERT(SDL3_copyp, sizeof(*(dst)) == sizeof(*(src))); }  \
    SDL3_memcpy((dst), (src), sizeof(*(src)))


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
        static char path_buf[SDL2COMPAT_MAXPATH];
        static char *base_path;
        OS_GetExeName(path_buf, SDL2COMPAT_MAXPATH);
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


/* if you change this, update also SDL2Compat_ApplyQuirks() */
static const char *
SDL2_to_SDL3_hint(const char *name)
{
    if (SDL3_strcmp(name, "SDL_VIDEODRIVER") == 0) {
        return "SDL_VIDEO_DRIVER";
    }
    if (SDL3_strcmp(name, "SDL_AUDIODRIVER") == 0) {
        return "SDL_AUDIO_DRIVER";
    }
    return name;
}

DECLSPEC SDL_bool SDLCALL
SDL_SetHintWithPriority(const char *name, const char *value, SDL_HintPriority priority)
{
    return SDL3_SetHintWithPriority(SDL2_to_SDL3_hint(name), value, priority);
}

DECLSPEC SDL_bool SDLCALL
SDL_SetHint(const char *name, const char *value)
{
    return SDL3_SetHint(SDL2_to_SDL3_hint(name), value);
}

DECLSPEC const char * SDLCALL
SDL_GetHint(const char *name)
{
    return SDL3_GetHint(SDL2_to_SDL3_hint(name));
}

DECLSPEC SDL_bool SDLCALL
SDL_ResetHint(const char *name)
{
    return SDL3_ResetHint(SDL2_to_SDL3_hint(name));
}

DECLSPEC SDL_bool SDLCALL
SDL_GetHintBoolean(const char *name, SDL_bool default_value)
{
    return SDL3_GetHintBoolean(SDL2_to_SDL3_hint(name), default_value);
}

/* FIXME: callbacks may need tweaking ... */
DECLSPEC void SDLCALL
SDL_AddHintCallback(const char *name, SDL_HintCallback callback, void *userdata)
{
    /* this returns an int of 0 or -1 in SDL3, but SDL2 it was void (even if it failed). */
    (void) SDL3_AddHintCallback(SDL2_to_SDL3_hint(name), callback, userdata);
}

DECLSPEC void SDLCALL
SDL_DelHintCallback(const char *name, SDL_HintCallback callback, void *userdata)
{
    SDL3_DelHintCallback(SDL2_to_SDL3_hint(name), callback, userdata);
}

static void
SDL2Compat_ApplyQuirks(SDL_bool force_x11)
{
    const char *exe_name = SDL2Compat_GetExeName();
    int i;

    if (WantDebugLogging) {
        SDL3_Log("This app appears to be named '%s'", exe_name);
    }

    {
        /* if you change this, update also SDL2_to_SDL3_hint() */
        /* hint/env names updated.
         * SDL_VIDEO_DRIVER (SDL2) to SDL_VIDEO_DRIVER (SDL3)
         * SDL_AUDIO_DRIVER (SDL2) to SDL_AUDIO_DRIVER (SDL3)
         */
        const char *videodriver_env = SDL3_getenv("SDL_VIDEODRIVER");
        const char *audiodriver_env = SDL3_getenv("SDL_AUDIODRIVER");
        if (videodriver_env) {
            SDL3_setenv("SDL_VIDEO_DRIVER", videodriver_env, 1);
        }
        if (audiodriver_env) {
            SDL3_setenv("SDL_AUDIO_DRIVER", audiodriver_env, 1);
        }
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
                SDL3_Log("sdl2-compat: We are forcing this app to use X11, because it probably talks to an X server directly, outside of SDL. If possible, this app should be fixed, to be compatible with Wayland, etc.");
            }
            SDL3_setenv("SDL_VIDEO_DRIVER", "x11", 1);
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


/* removed in SDL3 (which only uses SDL_WINDOW_HIDDEN now). */
#define SDL2_WINDOW_SHOWN 0x000000004
#define SDL2_WINDOW_FULLSCREEN_DESKTOP 0x00001000

/* removed in SDL3 (APIs like this were split into getter/setter functions). */
#define SDL2_QUERY   -1
#define SDL2_DISABLE  0
#define SDL2_ENABLE   1

#define SDL2_RENDERER_TARGETTEXTURE 0x00000008

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

/* these were replaced in SDL3; all their subevents became top-level events. */
/* These values are reserved in SDL3's SDL_EventType enum to help sdl2-compat. */
#define SDL2_DISPLAYEVENT 0x150
#define SDL2_WINDOWEVENT 0x200

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
    SDL2_JoystickID which;
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
    SDL2_JoystickID which;
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
    SDL2_JoystickID which;
    Uint8 hat;
    Uint8 value;
    Uint8 padding1;
    Uint8 padding2;
} SDL2_JoyHatEvent;

typedef struct SDL2_JoyButtonEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL2_JoystickID which;
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
    SDL2_JoystickID which;
    SDL_JoystickPowerLevel level;
} SDL2_JoyBatteryEvent;

typedef struct SDL2_ControllerAxisEvent
{
    Uint32 type;
    Uint32 timestamp;
    SDL2_JoystickID which;
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
    SDL2_JoystickID which;
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
    SDL2_JoystickID which;
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
    SDL2_JoystickID which;
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
    SDL2_GestureID gestureId;
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

typedef int (SDLCALL *SDL2_EventFilter) (void *userdata, SDL2_Event *event);

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
static SDL_JoystickID *joystick_list = NULL;
static int num_joysticks = 0;
static SDL_SensorID *sensor_list = NULL;
static int num_sensors = 0;

static SDL_mutex *joystick_lock = NULL;
static SDL_mutex *sensor_lock = NULL;

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

    sensor_lock = SDL3_CreateMutex();
    if (sensor_lock == NULL) {
        okay = 0;
    }

    joystick_lock = SDL3_CreateMutex();
    if (joystick_lock == NULL) {
        okay = 0;
    }

    if (!okay) {
        strcpy_fn(loaderror, "Failed to initialize sdl2-compat library.");
    }

    return okay;
}


/* obviously we have to override this so we don't report ourselves as SDL3. */
DECLSPEC void SDLCALL
SDL_GetVersion(SDL_version *ver)
{
    if (ver) {
        ver->major = 2;
        ver->minor = SDL2_COMPAT_VERSION_MINOR;
        ver->patch = SDL2_COMPAT_VERSION_PATCH;
        if (SDL3_GetHintBoolean("SDL_LEGACY_VERSION", SDL_FALSE)) {
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


DECLSPEC int SDLCALL
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
    return -1;
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

#define SDL_LOG_IMPL(name, prio)                                             \
DECLSPEC void SDLCALL                                                        \
SDL_Log##name(int category, SDL_PRINTF_FORMAT_STRING const char *fmt, ...) { \
    va_list ap; va_start(ap, fmt);                                           \
    SDL3_LogMessageV(category, SDL_LOG_PRIORITY_##prio, fmt, ap);            \
    va_end(ap);                                                              \
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
    /* mouse coords became floats in SDL3: */
    switch (event3->type) {
    case SDL_EVENT_MOUSE_MOTION:
        event2->motion.x = (Sint32)event3->motion.x;
        event2->motion.y = (Sint32)event3->motion.y;
        event2->motion.xrel = (Sint32)event3->motion.xrel;
        event2->motion.yrel = (Sint32)event3->motion.yrel;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        event2->button.x = (Sint32)event3->button.x;
        event2->button.y = (Sint32)event3->button.y;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        event2->wheel.x = (Sint32)event3->wheel.x;
        event2->wheel.y = (Sint32)event3->wheel.y;
        event2->wheel.preciseX = event3->wheel.x;
        event2->wheel.preciseY = event3->wheel.y;
        event2->wheel.mouseX = (Sint32)event3->wheel.mouseX;
        event2->wheel.mouseY = (Sint32)event3->wheel.mouseY;
        break;
    default:
        break;
    }
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
    /* mouse coords became floats in SDL3: */
    switch (event2->type) {
    case SDL_EVENT_MOUSE_MOTION:
        event3->motion.x = (float)event2->motion.x;
        event3->motion.y = (float)event2->motion.y;
        event3->motion.xrel = (float)event2->motion.xrel;
        event3->motion.yrel = (float)event2->motion.yrel;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        event3->button.x = (float)event2->button.x;
        event3->button.y = (float)event2->button.y;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        /* The preciseX|Y members were added to SDL_MouseWheelEvent in SDL2-2.0.18.
        event3->wheel.x = event2->wheel.preciseX;
        event3->wheel.y = event2->wheel.preciseY;
        */
        event3->wheel.x = (float)event2->wheel.x;
        event3->wheel.y = (float)event2->wheel.y;
        event3->wheel.mouseX = (float)event2->wheel.mouseX;
        event3->wheel.mouseY = (float)event2->wheel.mouseY;
        break;
    default:
        break;
    }
    return event3;
}

static void GestureProcessEvent(const SDL_Event *event3);

DECLSPEC int SDLCALL
SDL_PushEvent(SDL2_Event *event2)
{
    SDL_Event event3;
    return SDL3_PushEvent(Event2to3(event2, &event3));
}

static int Display_IDToIndex(SDL_DisplayID displayID);

static int SDLCALL
EventFilter3to2(void *userdata, SDL_Event *event3)
{
    SDL2_Event event2;  /* note that event filters do not receive events as const! So we have to convert or copy it for each one! */

    GestureProcessEvent(event3);  /* this might need to generate new gesture events from touch input. */

    if (EventFilter2) {
        /* !!! FIXME: this needs to not return if the filter gives its blessing, as we still have more to do. */
        return EventFilter2(EventFilterUserData2, Event3to2(event3, &event2));
    }

    if (EventWatchers2 != NULL) {
        EventFilterWrapperData *i;
        SDL3_LockMutex(EventWatchListMutex);
        for (i = EventWatchers2; i != NULL; i = i->next) {
            i->filter2(i->userdata, Event3to2(event3, &event2));
        }
        SDL3_UnlockMutex(EventWatchListMutex);
    }

    /* push new events when we need to convert something, like toplevel SDL3 events generating the SDL2 SDL_WINDOWEVENT. */

    switch (event3->type) {
        /* display events moved to the top level in SDL3. */
        case SDL_EVENT_DISPLAY_ORIENTATION:
        case SDL_EVENT_DISPLAY_CONNECTED:
        case SDL_EVENT_DISPLAY_DISCONNECTED:
            if (SDL3_EventEnabled(SDL2_DISPLAYEVENT)) {
                event2.display.type = SDL2_DISPLAYEVENT;
                event2.display.timestamp = (Uint32) SDL_NS_TO_MS(event3->display.timestamp);
                event2.display.display = Display_IDToIndex(event3->display.displayID);
                event2.display.event = (Uint8) ((event3->type - ((Uint32) SDL_EVENT_DISPLAY_ORIENTATION)) + 1);
                event2.display.padding1 = 0;
                event2.display.padding2 = 0;
                event2.display.padding3 = 0;
                event2.display.data1 = event3->display.data1;
                SDL_PushEvent(&event2);
            }
            break;

        /* window events moved to the top level in SDL3. */
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        case SDL_EVENT_WINDOW_MINIMIZED:
        case SDL_EVENT_WINDOW_MAXIMIZED:
        case SDL_EVENT_WINDOW_RESTORED:
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_TAKE_FOCUS:
        case SDL_EVENT_WINDOW_HIT_TEST:
        case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            if (SDL3_EventEnabled(SDL2_WINDOWEVENT)) {
                event2.window.type = SDL2_WINDOWEVENT;
                event2.window.timestamp = (Uint32) SDL_NS_TO_MS(event3->window.timestamp);
                event2.window.windowID = event3->window.windowID;
                event2.window.event = (Uint8) ((event3->type - ((Uint32) SDL_EVENT_WINDOW_SHOWN)) + 1);
                event2.window.padding1 = 0;
                event2.window.padding2 = 0;
                event2.window.padding3 = 0;
                event2.window.data1 = event3->window.data1;
                event2.window.data2 = event3->window.data2;
                SDL_PushEvent(&event2);
            }
            break;

        default: break;
    }

    /* !!! FIXME: Deal with device add events using instance ids instead of indices in SDL3. */

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
        return SDL3_OutOfMemory();
    }
    if (action == SDL_ADDEVENT) {
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

    SDL3_free(events3);
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

DECLSPEC void SDLCALL
SDL_AddEventWatch(SDL2_EventFilter filter2, void *userdata)
{
    /* we set up an SDL3 event filter to manage things already; we will also use it to call all added SDL2 event watchers. Put this new one in that list. */
    EventFilterWrapperData *wrapperdata = (EventFilterWrapperData *) SDL3_malloc(sizeof (EventFilterWrapperData));
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
    EventFilterWrapperData wrapperdata;
    wrapperdata.filter2 = filter2;
    wrapperdata.userdata = userdata;
    wrapperdata.next = NULL;
    SDL3_FilterEvents(EventFilterWrapper3to2, &wrapperdata);
}


/* mouse coords became floats in SDL3 */

DECLSPEC Uint32 SDLCALL
SDL_GetMouseState(int *x, int *y)
{
    float fx, fy;
    Uint32 ret = SDL3_GetMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return ret;
}

DECLSPEC Uint32 SDLCALL
SDL_GetGlobalMouseState(int *x, int *y)
{
    float fx, fy;
    Uint32 ret = SDL3_GetGlobalMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return ret;
}

DECLSPEC Uint32 SDLCALL
SDL_GetRelativeMouseState(int *x, int *y)
{
    float fx, fy;
    Uint32 ret = SDL3_GetRelativeMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return ret;
}

DECLSPEC void SDLCALL
SDL_WarpMouseInWindow(SDL_Window *window, int x, int y)
{
    SDL3_WarpMouseInWindow(window, (float)x, (float)y);
}

DECLSPEC int SDLCALL
SDL_WarpMouseGlobal(int x, int y)
{
    return SDL3_WarpMouseGlobal((float)x, (float)y);
}

DECLSPEC void SDLCALL
SDL_RenderGetViewport(SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL3_GetRenderViewport(renderer, rect);
}

DECLSPEC void SDLCALL
SDL_RenderGetClipRect(SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL3_GetRenderClipRect(renderer, rect);
}

DECLSPEC void SDLCALL
SDL_RenderGetScale(SDL_Renderer *renderer, float *scaleX, float *scaleY)
{
    SDL3_GetRenderScale(renderer, scaleX, scaleY);
}

DECLSPEC void SDLCALL
SDL_RenderWindowToLogical(SDL_Renderer *renderer,
                          int windowX, int windowY,
                          float *logicalX, float *logicalY)
{
    SDL3_RenderCoordinatesFromWindow(renderer, (float)windowX, (float)windowY, logicalX, logicalY);
}

DECLSPEC void SDLCALL
SDL_RenderLogicalToWindow(SDL_Renderer *renderer,
                          float logicalX, float logicalY,
                          int *windowX, int *windowY)
{
    float x, y;
    SDL3_RenderCoordinatesToWindow(renderer, logicalX, logicalY, &x, &y);
    if (windowX) *windowX = (int)x;
    if (windowY) *windowY = (int)y;
}


/* The SDL3 version of SDL_RWops changed, so we need to convert when necessary. */

struct SDL2_RWops
{
    Sint64 (SDLCALL * size) (struct SDL2_RWops *ctx);
    Sint64 (SDLCALL * seek) (struct SDL2_RWops *ctx, Sint64 offset, int whence);
    size_t (SDLCALL * read) (struct SDL2_RWops *ctx, void *ptr, size_t size, size_t maxnum);
    size_t (SDLCALL * write) (struct SDL2_RWops *ctx, const void *ptr, size_t size, size_t num);
    int (SDLCALL * close) (struct SDL2_RWops *ctx);
    Uint32 type;
    union
    {
        struct {
            SDL_bool autoclose;
            void *fp;
        } stdio;
        struct {
            void *data1;
            void *data2;
        } unknown;
        struct {
            SDL_RWops *rwops;
        } sdl3;
        struct {
            void *ptrs[3];  /* just so this matches SDL2's struct size. */
        } match_sdl2;
    } hidden;
};

DECLSPEC SDL2_RWops *SDLCALL
SDL_AllocRW(void)
{
    SDL2_RWops *area2 = (SDL2_RWops *)SDL3_malloc(sizeof *area2);
    if (area2 == NULL) {
        SDL3_OutOfMemory();
    } else {
        area2->type = SDL_RWOPS_UNKNOWN;
    }
    return area2;
}

DECLSPEC void SDLCALL
SDL_FreeRW(SDL2_RWops *area2)
{
    SDL3_free(area2);
}

static Sint64 SDLCALL
RWops3to2_size(SDL2_RWops *rwops2)
{
    return SDL3_RWsize(rwops2->hidden.sdl3.rwops);
}

static Sint64 SDLCALL
RWops3to2_seek(SDL2_RWops *rwops2, Sint64 offset, int whence)
{
    return SDL3_RWseek(rwops2->hidden.sdl3.rwops, offset, whence);
}

static size_t SDLCALL
RWops3to2_read(SDL2_RWops *rwops2, void *ptr, size_t size, size_t maxnum)
{
    const Sint64 br = SDL3_RWread(rwops2->hidden.sdl3.rwops, ptr, ((Sint64) size) * ((Sint64) maxnum));
    return (br <= 0) ? 0 : (size_t) br;
}

static size_t SDLCALL
RWops3to2_write(SDL2_RWops *rwops2, const void *ptr, size_t size, size_t maxnum)
{
    const Sint64 bw = SDL3_RWwrite(rwops2->hidden.sdl3.rwops, ptr, ((Sint64) size) * ((Sint64) maxnum));
    return (bw <= 0) ? 0 : (size_t) bw;
}

static int SDLCALL
RWops3to2_close(SDL2_RWops *rwops2)
{
    const int retval = SDL3_RWclose(rwops2->hidden.sdl3.rwops);
    SDL_FreeRW(rwops2);  /* !!! FIXME: _should_ we free this if SDL3_RWclose failed? */
    return retval;
}

static SDL2_RWops *
RWops3to2(SDL_RWops *rwops3)
{
    SDL2_RWops *rwops2 = NULL;
    if (rwops3) {
        rwops2 = SDL_AllocRW();
        if (!rwops2) {
            SDL3_RWclose(rwops3);  /* !!! FIXME: make sure this is still safe if things change. */
            return NULL;
        }

        SDL3_zerop(rwops2);
        rwops2->size = RWops3to2_size;
        rwops2->seek = RWops3to2_seek;
        rwops2->read = RWops3to2_read;
        rwops2->write = RWops3to2_write;
        rwops2->close = RWops3to2_close;
        rwops2->type = rwops3->type;  /* these match up for now. */
        rwops2->hidden.sdl3.rwops = rwops3;
    }
    return rwops2;
}

DECLSPEC SDL2_RWops *SDLCALL
SDL_RWFromFile(const char *file, const char *mode)
{
    return RWops3to2(SDL3_RWFromFile(file, mode));
}

DECLSPEC SDL2_RWops *SDLCALL
SDL_RWFromMem(void *mem, int size)
{
    return RWops3to2(SDL3_RWFromMem(mem, size));
}

DECLSPEC SDL2_RWops *SDLCALL
SDL_RWFromConstMem(const void *mem, int size)
{
    return RWops3to2(SDL3_RWFromConstMem(mem, size));
}

DECLSPEC Sint64 SDLCALL
SDL_RWsize(SDL2_RWops *rwops2)
{
    return rwops2->size(rwops2);
}

DECLSPEC Sint64 SDLCALL
SDL_RWseek(SDL2_RWops *rwops2, Sint64 offset, int whence)
{
    return rwops2->seek(rwops2, offset, whence);
}

DECLSPEC Sint64 SDLCALL
SDL_RWtell(SDL2_RWops *rwops2)
{
    return rwops2->seek(rwops2, 0, SDL_RW_SEEK_CUR);
}

DECLSPEC size_t SDLCALL
SDL_RWread(SDL2_RWops *rwops2, void *ptr, size_t size, size_t maxnum)
{
    return rwops2->read(rwops2, ptr, size, maxnum);
}

DECLSPEC size_t SDLCALL
SDL_RWwrite(SDL2_RWops *rwops2, const void *ptr, size_t size, size_t num)
{
    return rwops2->write(rwops2, ptr, size, num);
}

DECLSPEC int SDLCALL
SDL_RWclose(SDL2_RWops *rwops2)
{
    return rwops2->close(rwops2);
}

DECLSPEC Uint8 SDLCALL
SDL_ReadU8(SDL2_RWops *rwops2)
{
    Uint8 x = 0;
    SDL_RWread(rwops2, &x, sizeof (x), 1);
    return x;
}

DECLSPEC size_t SDLCALL
SDL_WriteU8(SDL2_RWops *rwops2, Uint8 x)
{
    return SDL_RWwrite(rwops2, &x, sizeof(x), 1);
}

#define DORWOPSENDIAN(order, bits)          \
DECLSPEC Uint##bits SDLCALL                 \
SDL_Read##order##bits(SDL2_RWops *rwops2) { \
    Uint##bits x = 0;                       \
    SDL_RWread(rwops2, &x, sizeof (x), 1);  \
    return SDL_Swap##order##bits(x);        \
} \
                                                           \
DECLSPEC size_t SDLCALL                                    \
SDL_Write##order##bits(SDL2_RWops *rwops2, Uint##bits x) { \
    x = SDL_Swap##order##bits(x);                          \
    return SDL_RWwrite(rwops2, &x, sizeof(x), 1);          \
}
DORWOPSENDIAN(LE, 16)
DORWOPSENDIAN(BE, 16)
DORWOPSENDIAN(LE, 32)
DORWOPSENDIAN(BE, 32)
DORWOPSENDIAN(LE, 64)
DORWOPSENDIAN(BE, 64)
#undef DORWOPSENDIAN

/* stdio SDL_RWops was removed from SDL3, to prevent incompatible C runtime issues */
#ifndef HAVE_STDIO_H
DECLSPEC SDL2_RWops * SDLCALL
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
stdio_size(SDL2_RWops *rwops2)
{
    Sint64 pos, size;

    pos = SDL_RWseek(rwops2, 0, SDL_RW_SEEK_CUR);
    if (pos < 0) {
        return -1;
    }
    size = SDL_RWseek(rwops2, 0, SDL_RW_SEEK_END);

    SDL_RWseek(rwops2, pos, SDL_RW_SEEK_SET);
    return size;
}

static Sint64 SDLCALL
stdio_seek(SDL2_RWops *rwops2, Sint64 offset, int whence)
{
    FILE *fp = (FILE *) rwops2->hidden.stdio.fp;
    int stdiowhence;

    switch (whence) {
    case SDL_RW_SEEK_SET:
        stdiowhence = SEEK_SET;
        break;
    case SDL_RW_SEEK_CUR:
        stdiowhence = SEEK_CUR;
        break;
    case SDL_RW_SEEK_END:
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
stdio_read(SDL2_RWops *rwops2, void *ptr, size_t size, size_t maxnum)
{
    FILE *fp = (FILE *) rwops2->hidden.stdio.fp;
    size_t nread = fread(ptr, size, maxnum, fp);
    if (nread == 0 && ferror(fp)) {
        SDL3_Error(SDL_EFREAD);
    }
    return nread;
}

static size_t SDLCALL
stdio_write(SDL2_RWops *rwops2, const void *ptr, size_t size, size_t num)
{
    FILE *fp = (FILE *) rwops2->hidden.stdio.fp;
    size_t nwrote = fwrite(ptr, size, num, fp);
    if (nwrote == 0 && ferror(fp)) {
        SDL3_Error(SDL_EFWRITE);
    }
    return nwrote;
}

static int SDLCALL
stdio_close(SDL2_RWops *rwops2)
{
    int status = 0;
    if (rwops2) {
        if (rwops2->hidden.stdio.autoclose) {
            if (fclose((FILE *)rwops2->hidden.stdio.fp) != 0) {
                status = SDL3_Error(SDL_EFWRITE);
            }
        }
        SDL_FreeRW(rwops2);
    }
    return status;
}

DECLSPEC SDL2_RWops * SDLCALL
SDL_RWFromFP(FILE *fp, SDL_bool autoclose)
{
    SDL2_RWops *rwops = SDL_AllocRW();
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

static Sint64 SDLCALL
RWops2to3_size(struct SDL_RWops *rwops3)
{
    return SDL_RWsize((SDL2_RWops *) rwops3->hidden.unknown.data1);
}

static Sint64 SDLCALL
RWops2to3_seek(struct SDL_RWops *rwops3, Sint64 offset, int whence)
{
    return SDL_RWseek((SDL2_RWops *) rwops3->hidden.unknown.data1, offset, whence);
}

static Sint64 SDLCALL
RWops2to3_read(struct SDL_RWops *rwops3, void *ptr, Sint64 size)
{
    const size_t br = SDL_RWread((SDL2_RWops *) rwops3->hidden.unknown.data1, ptr, 1, (size_t) size);
    return (Sint64) br;
}

static Sint64 SDLCALL
RWops2to3_write(struct SDL_RWops *rwops3, const void *ptr, Sint64 size)
{
    const size_t bw = SDL_RWwrite((SDL2_RWops *) rwops3->hidden.unknown.data1, ptr, 1, (size_t) size);
    return (bw == 0) ? -1 : (Sint64) bw;
}

static int SDLCALL
RWops2to3_close(struct SDL_RWops *rwops3)
{
    const int retval = SDL_RWclose((SDL2_RWops *) rwops3->hidden.unknown.data1);
    SDL3_DestroyRW(rwops3);  /* !!! FIXME: _should_ we free this if SDL_RWclose failed? */
    return retval;
}

static SDL_RWops *
RWops2to3(SDL2_RWops *rwops2)
{
    SDL_RWops *rwops3 = NULL;
    if (rwops2) {
        rwops3 = SDL3_CreateRW();
        if (!rwops3) {
            return NULL;
        }

        SDL3_zerop(rwops3);
        rwops3->size = RWops2to3_size;
        rwops3->seek = RWops2to3_seek;
        rwops3->read = RWops2to3_read;
        rwops3->write = RWops2to3_write;
        rwops3->close = RWops2to3_close;
        rwops3->type = rwops3->type;  /* these match up for now. */
        rwops3->hidden.unknown.data1 = rwops2;
    }
    return rwops3;
}

DECLSPEC void *SDLCALL
SDL_LoadFile_RW(SDL2_RWops *rwops2, size_t *datasize, int freesrc)
{
    void *retval = NULL;
    SDL_RWops *rwops3 = RWops2to3(rwops2);
    if (rwops3) {
        retval = SDL3_LoadFile_RW(rwops3, datasize, freesrc);
        if (!freesrc) {
            SDL3_DestroyRW(rwops3);  /* don't close it because that'll close the SDL2_RWops. */
        }
    } else if (rwops2) {
        if (freesrc) {
            SDL_RWclose(rwops2);
        }
    }
    return retval;
}

DECLSPEC SDL_AudioSpec *SDLCALL
SDL_LoadWAV_RW(SDL2_RWops *rwops2, int freesrc, SDL_AudioSpec *spec, Uint8 **audio_buf, Uint32 *audio_len)
{
    SDL_AudioSpec *retval = NULL;
    SDL_RWops *rwops3 = RWops2to3(rwops2);
    if (rwops3) {
        retval = SDL3_LoadWAV_RW(rwops3, freesrc, spec, audio_buf, audio_len);
        if (!freesrc) {
            SDL3_DestroyRW(rwops3);  /* don't close it because that'll close the SDL2_RWops. */
        }
    } else if (rwops2) {
        if (freesrc) {
            SDL_RWclose(rwops2);
        }
    }
    return retval;
}

DECLSPEC SDL_Surface *SDLCALL
SDL_LoadBMP_RW(SDL2_RWops *rwops2, int freesrc)
{
    SDL_Surface *retval = NULL;
    SDL_RWops *rwops3 = RWops2to3(rwops2);
    if (rwops3) {
        retval = SDL3_LoadBMP_RW(rwops3, freesrc);
        if (!freesrc) {
            SDL3_DestroyRW(rwops3);  /* don't close it because that'll close the SDL2_RWops. */
        }
    } else if (rwops2) {
        if (freesrc) {
            SDL_RWclose(rwops2);
        }
    }
    return retval;
}

DECLSPEC int SDLCALL
SDL_SaveBMP_RW(SDL_Surface *surface, SDL2_RWops *rwops2, int freedst)
{
    int retval = -1;
    SDL_RWops *rwops3 = RWops2to3(rwops2);
    if (rwops3) {
        retval = SDL3_SaveBMP_RW(surface, rwops3, freedst);
        if (!freedst) {
            SDL3_DestroyRW(rwops3);  /* don't close it because that'll close the SDL2_RWops. */
        }
    } else if (rwops2) {
        if (freedst) {
            SDL_RWclose(rwops2);
        }
    }
    return retval;
}

DECLSPEC int SDLCALL
SDL_GameControllerAddMappingsFromRW(SDL2_RWops *rwops2, int freerw)
{
    int retval = -1;
    SDL_RWops *rwops3 = RWops2to3(rwops2);
    if (rwops3) {
        retval = SDL3_AddGamepadMappingsFromRW(rwops3, freerw);
        if (!freerw) {
            SDL3_DestroyRW(rwops3);  /* don't close it because that'll close the SDL2_RWops. */
        }
    } else if (rwops2) {
        if (freerw) {
            SDL_RWclose(rwops2);
        }
    }
    return retval;
}


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
      SDL3_InvalidParamError("gamma");
      return;
    }
    if (ramp == NULL) {
      SDL3_InvalidParamError("ramp");
      return;
    }

    /* 0.0 gamma is all black */
    if (gamma == 0.0f) {
        SDL3_memset(ramp, 0, 256 * sizeof(Uint16));
        return;
    }
    if (gamma == 1.0f) {
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
                (int) (SDL3_pow((double) i / 256.0, gamma) * 65535.0 + 0.5);
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
            SDL3_memcpy(red, buf, 256 * sizeof (Uint16));
        }
        if (green && (green != buf)) {
            SDL3_memcpy(green, buf, 256 * sizeof (Uint16));
        }
        if (blue && (blue != buf)) {
            SDL3_memcpy(blue, buf, 256 * sizeof (Uint16));
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
    return SDL3_CreateSurface(width, height, SDL3_GetPixelFormatEnumForMasks(depth, Rmask, Gmask, Bmask, Amask));
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth, Uint32 format)
{
    return SDL3_CreateSurface(width, height, format);
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return SDL3_CreateSurfaceFrom(pixels, width, height, pitch, SDL3_GetPixelFormatEnumForMasks(depth, Rmask, Gmask, Bmask, Amask));
}

DECLSPEC SDL_Surface * SDLCALL
SDL_CreateRGBSurfaceWithFormatFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 format)
{
    return SDL3_CreateSurfaceFrom(pixels, width, height, pitch, format);
}

/* SDL_GetTicks is 64-bit in SDL3. Clamp it for SDL2. */
DECLSPEC Uint32 SDLCALL
SDL_GetTicks(void)
{
    return (Uint32)SDL3_GetTicks();
}

/* SDL_GetTicks64 is gone in SDL3 because SDL_GetTicks is 64-bit. */
DECLSPEC Uint64 SDLCALL
SDL_GetTicks64(void)
{
    return SDL3_GetTicks();
}

DECLSPEC SDL_bool SDLCALL
SDL_GetWindowWMInfo(SDL_Window *window, SDL_SysWMinfo *wminfo)
{
    SDL3_Unsupported();  /* !!! FIXME: write me. */
    return SDL_FALSE;
}

/* this API was removed from SDL3 since nothing supported it. Just report 0. */
DECLSPEC int SDLCALL
SDL_JoystickNumBalls(SDL_Joystick *joystick)
{
    if (SDL3_GetNumJoystickAxes(joystick) == -1) {
        return -1;  /* just to call JOYSTICK_CHECK_MAGIC on `joystick`. */
    }
    return 0;
}

/* this API was removed from SDL3 since nothing supported it. Just report failure. */
DECLSPEC int SDLCALL
SDL_JoystickGetBall(SDL_Joystick *joystick, int ball, int *dx, int *dy)
{
    if (SDL3_GetNumJoystickAxes(joystick) == -1) {
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

static GestureTouch *
GestureAddTouch(const SDL_TouchID touchId)
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

static int
GestureDelTouch(const SDL_TouchID touchId)
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

static GestureTouch *
GestureGetTouch(const SDL_TouchID touchId)
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
static void
GestureQuit(void)
{
    SDL3_free(GestureTouches);
    GestureTouches = NULL;
}

static unsigned long
GestureHashDollar(SDL_FPoint *points)
{
    unsigned long hash = 5381;
    int i;
    for (i = 0; i < GESTURE_DOLLARNPOINTS; i++) {
        hash = ((hash << 5) + hash) + (unsigned long)points[i].x;
        hash = ((hash << 5) + hash) + (unsigned long)points[i].y;
    }
    return hash;
}

static int
GestureSaveTemplate(GestureDollarTemplate *templ, SDL2_RWops *dst)
{
    if (dst == NULL) {
        return 0;
    }

    /* No Longer storing the Hash, rehash on load */
    /* if (SDL2_RWops.write(dst, &(templ->hash), sizeof(templ->hash), 1) != 1) return 0; */

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
    if (SDL_RWwrite(dst, templ->path, sizeof(templ->path[0]), GESTURE_DOLLARNPOINTS) != GESTURE_DOLLARNPOINTS) {
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

        if (SDL_RWwrite(dst, copy.path, sizeof(copy.path[0]), GESTURE_DOLLARNPOINTS) != GESTURE_DOLLARNPOINTS) {
            return 0;
        }
    }
#endif

    return 1;
}

DECLSPEC int SDLCALL
SDL_SaveAllDollarTemplates(SDL2_RWops *dst)
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
SDL_SaveDollarTemplate(SDL2_GestureID gestureId, SDL2_RWops *dst)
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
static int
GestureAddDollar_one(GestureTouch *inTouch, SDL_FPoint *path)
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

static int
GestureAddDollar(GestureTouch *inTouch, SDL_FPoint *path)
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
SDL_LoadDollarTemplates(SDL_TouchID touchId, SDL2_RWops *src)
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

        if (SDL_RWread(src, templ.path, sizeof(templ.path[0]), GESTURE_DOLLARNPOINTS) < GESTURE_DOLLARNPOINTS) {
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

static float
GestureDollarDifference(SDL_FPoint *points, SDL_FPoint *templ, float ang)
{
    /*  SDL_FPoint p[GESTURE_DOLLARNPOINTS]; */
    float dist = 0;
    SDL_FPoint p;
    int i;
    for (i = 0; i < GESTURE_DOLLARNPOINTS; i++) {
        p.x = points[i].x * SDL3_cosf(ang) - points[i].y * SDL3_sinf(ang);
        p.y = points[i].x * SDL3_sinf(ang) + points[i].y * SDL3_cosf(ang);
        dist += SDL3_sqrtf((p.x - templ[i].x) * (p.x - templ[i].x) + (p.y - templ[i].y) * (p.y - templ[i].y));
    }
    return dist / GESTURE_DOLLARNPOINTS;
}

static float
GestureBestDollarDifference(SDL_FPoint *points, SDL_FPoint *templ)
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
static int
GestureDollarNormalize(const GestureDollarPath *path, SDL_FPoint *points, SDL_bool is_recording)
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

static float
GestureDollarRecognize(const GestureDollarPath *path, int *bestTempl, GestureTouch *touch)
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

static void
GestureSendMulti(GestureTouch *touch, float dTheta, float dDist)
{
    if (SDL3_EventEnabled(SDL_MULTIGESTURE)) {
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

static void
GestureSendDollar(GestureTouch *touch, SDL2_GestureID gestureId, float error)
{
    if (SDL3_EventEnabled(SDL_DOLLARGESTURE)) {
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

static void
GestureSendDollarRecord(GestureTouch *touch, SDL2_GestureID gestureId)
{
    if (SDL3_EventEnabled(SDL_DOLLARRECORD)) {
        SDL2_Event event;
        event.type = SDL_DOLLARRECORD;
        event.common.timestamp = 0;
        event.dgesture.touchId = touch->touchId;
        event.dgesture.gestureId = gestureId;
        SDL_PushEvent(&event);
    }
}

/* These are SDL3 events coming in from sdl2-compat's event watcher. */
static void
GestureProcessEvent(const SDL_Event *event3)
{
    float x, y;
    int i, index;
    float pathDx, pathDy;
    SDL_FPoint lastP;
    SDL_FPoint lastCentroid;
    float lDist;
    float Dist;
    float dtheta;
    float dDist;

    if (event3->type == SDL_EVENT_FINGER_MOTION || event3->type == SDL_EVENT_FINGER_DOWN || event3->type == SDL_EVENT_FINGER_UP) {
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
        if (event3->type == SDL_EVENT_FINGER_UP) {
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
        } else if (event3->type == SDL_EVENT_FINGER_MOTION) {
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
        } else if (event3->type == SDL_EVENT_FINGER_DOWN) {
            inTouch->numDownFingers++;
            inTouch->centroid.x = (inTouch->centroid.x * (inTouch->numDownFingers - 1) + x) /
                                  inTouch->numDownFingers;
            inTouch->centroid.y = (inTouch->centroid.y * (inTouch->numDownFingers - 1) + y) /
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
SDL_GetRenderDriverInfo(int index, SDL_RendererInfo *info)
{
    const char *name = SDL3_GetRenderDriver(index);
    if (!name) {
        return -1;  /* assume SDL3_GetRenderDriver set the SDL error. */
    }

    SDL3_zerop(info);
    info->name = name;

    /* these are the values that SDL2 returns. */
    if ((SDL3_strcmp(name, "opengl") == 0) || (SDL3_strcmp(name, "opengles2") == 0)) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
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
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
    } else if (SDL3_strcmp(name, "direct3d11") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_RGB888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
    } else if (SDL3_strcmp(name, "direct3d12") == 0) {
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
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
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_ABGR8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
    } else if (SDL3_strcmp(name, "software") == 0) {
        info->flags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
    } else {  /* this seems reasonable if something currently-unknown shows up in SDL3. */
        info->flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
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

DECLSPEC SDL_bool SDLCALL
SDL_RenderTargetSupported(SDL_Renderer *renderer)
{
    int ret;
    SDL_RendererInfo info;
    ret = SDL_GetRendererInfo(renderer, &info);
    if (ret == 0) {
        if (info.flags & SDL2_RENDERER_TARGETTEXTURE) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

DECLSPEC int SDLCALL
SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h)
{
    if (w == 0 && h == 0) {
        return SDL3_SetRenderLogicalPresentation(renderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED, SDL_SCALEMODE_NEAREST);
    }
    return SDL3_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX, SDL_SCALEMODE_LINEAR);
}

DECLSPEC void SDLCALL
SDL_RenderGetLogicalSize(SDL_Renderer *renderer, int *w, int *h)
{
    SDL3_GetRenderLogicalPresentation(renderer, w, h, NULL, NULL);
}

DECLSPEC int SDLCALL
SDL_RenderSetIntegerScale(SDL_Renderer *renderer, SDL_bool enable)
{
    SDL_ScaleMode scale_mode;
    SDL_RendererLogicalPresentation mode;
    int w, h;
    int ret;

    ret = SDL3_GetRenderLogicalPresentation(renderer, &w, &h, &mode, &scale_mode);
    if (ret < 0) {
        return ret;
    }

    if (enable && mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
        return 0;
    }

    if (!enable && mode != SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
        return 0;
    }

    if (enable) {
        return SDL3_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE, scale_mode);
    }
    return SDL3_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_DISABLED, scale_mode);
}

DECLSPEC SDL_bool SDLCALL
SDL_RenderGetIntegerScale(SDL_Renderer *renderer)
{
    SDL_RendererLogicalPresentation mode;
    if (SDL3_GetRenderLogicalPresentation(renderer, NULL, NULL, &mode, NULL) == 0) {
        if (mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
            return SDL_TRUE;
        }
    }
    return SDL_FALSE;
}

DECLSPEC int SDLCALL
SDL_AudioInit(const char *driver_name)
{
    if (driver_name) {
        SDL3_SetHint("SDL_AUDIO_DRIVER", driver_name);
    }
    return SDL_InitSubSystem(SDL_INIT_AUDIO);
}

DECLSPEC void SDLCALL
SDL_AudioQuit(void)
{
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

DECLSPEC int SDLCALL
SDL_VideoInit(const char *driver_name)
{
    if (driver_name) {
        SDL3_SetHint("SDL_VIDEO_DRIVER", driver_name);
    }
    return SDL_InitSubSystem(SDL_INIT_VIDEO);
}

DECLSPEC void SDLCALL
SDL_VideoQuit(void)
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

DECLSPEC int SDLCALL
SDL_GL_GetSwapInterval(void)
{
    int val = 0;
    SDL3_GL_GetSwapInterval(&val);
    return val;
}

static SDL_AudioDeviceID g_audio_id = 0;
static SDL_AudioSpec g_audio_spec;

DECLSPEC int SDLCALL
SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained)
{
    SDL_AudioDeviceID id = 0;

    /* Start up the audio driver, if necessary. This is legacy behaviour! */
    if (!SDL_WasInit(SDL_INIT_AUDIO)) {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
            return -1;
        }
    }

    if (g_audio_id > 0) {
        return SDL3_SetError("Audio device is already opened");
    }

    if (obtained) {
        id = SDL_OpenAudioDevice(NULL, 0, desired, obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);

        g_audio_spec = *obtained;
    } else {
        SDL_AudioSpec _obtained;
        SDL3_zero(_obtained);
        id = SDL_OpenAudioDevice(NULL, 0, desired, &_obtained, 0);
        /* On successful open, copy calculated values into 'desired'. */
        if (id > 0) {
            desired->size = _obtained.size;
            desired->silence = _obtained.silence;
        }

        g_audio_spec = _obtained;
    }

    if (id > 0) {
        g_audio_id = id;
        return 0;
    }
    return -1;
}

/*
 * Moved here from SDL_mixer.c, since it relies on internals of an opened
 *  audio device (and is deprecated, by the way!).
 */
DECLSPEC void SDLCALL
SDL_MixAudio(Uint8 *dst, const Uint8 *src, Uint32 len, int volume)
{
    /* Mix the user-level audio format */
    if (g_audio_id > 0) {
        SDL_MixAudioFormat(dst, src, g_audio_spec.format, len, volume); /* FIXME: is this correct ?? */
    }
}

DECLSPEC Uint32 SDLCALL
SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    return SDL3_GetQueuedAudioSize(id);
}

DECLSPEC int SDLCALL
SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    return SDL3_QueueAudio(id, data, len);
}

DECLSPEC Uint32 SDLCALL
SDL_DequeueAudio(SDL_AudioDeviceID dev, void *data, Uint32 len)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    return SDL3_DequeueAudio(id, data, len);
}

DECLSPEC void SDLCALL
SDL_ClearQueuedAudio(SDL_AudioDeviceID dev)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    SDL3_ClearQueuedAudio(id);
}

DECLSPEC void SDLCALL
SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    if (pause_on) {
        SDL3_PauseAudioDevice(id);
    } else {
        SDL3_PlayAudioDevice(id);
    }
}

DECLSPEC SDL_AudioStatus SDLCALL
SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    return SDL3_GetAudioDeviceStatus(id);
}

DECLSPEC void SDLCALL
SDL_LockAudioDevice(SDL_AudioDeviceID dev)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    SDL3_LockAudioDevice(id);
}

DECLSPEC void SDLCALL
SDL_UnlockAudioDevice(SDL_AudioDeviceID dev)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    SDL3_UnlockAudioDevice(id);
}

DECLSPEC void SDLCALL
SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
    SDL_AudioDeviceID id = dev == 1 ? g_audio_id : dev;
    SDL3_CloseAudioDevice(id);
}

DECLSPEC void SDLCALL
SDL_LockAudio(void)
{
    SDL_LockAudioDevice(1);
}

DECLSPEC void SDLCALL
SDL_UnlockAudio(void)
{
    SDL_UnlockAudioDevice(1);
}

DECLSPEC void SDLCALL
SDL_CloseAudio(void)
{
    SDL_CloseAudioDevice(1);
    g_audio_id = 0;
}

DECLSPEC void SDLCALL
SDL_PauseAudio(int pause_on)
{
    SDL_PauseAudioDevice(1, pause_on);
}

DECLSPEC SDL_AudioStatus SDLCALL
SDL_GetAudioStatus(void)
{
    return SDL_GetAudioDeviceStatus(1);
}


DECLSPEC void SDLCALL
SDL_LockJoysticks(void)
{
    SDL3_LockMutex(joystick_lock);
}

DECLSPEC void SDLCALL
SDL_UnlockJoysticks(void)
{
    SDL3_UnlockMutex(joystick_lock);
}

DECLSPEC void SDLCALL
SDL_LockSensors(void)
{
    SDL3_LockMutex(sensor_lock);
}

DECLSPEC void SDLCALL
SDL_UnlockSensors(void)
{
    SDL3_UnlockMutex(sensor_lock);
}


struct SDL2_DisplayMode
{
    Uint32 format;              /**< pixel format */
    int w;                      /**< width, in screen coordinates */
    int h;                      /**< height, in screen coordinates */
    int refresh_rate;           /**< refresh rate (or zero for unspecified) */
    void *driverdata;           /**< driver-specific data, initialize to 0 */
};

static void
DisplayMode_2to3(const SDL2_DisplayMode *in, SDL_DisplayMode *out) {
    if (in && out) {
        out->format = in->format;
        out->pixel_w = in->w;
        out->pixel_h = in->h;
        out->screen_w = in->w;
        out->screen_h = in->h;
        out->refresh_rate = (float) in->refresh_rate;
        out->display_scale = 1.0f;
        out->driverdata = in->driverdata;
    }
}

static void
DisplayMode_3to2(const SDL_DisplayMode *in, SDL2_DisplayMode *out) {
    if (in && out) {
        out->format = in->format;
        out->w = in->pixel_w;
        out->h = in->pixel_h;
        out->refresh_rate = (int) SDL3_ceil(in->refresh_rate);
        out->driverdata = in->driverdata;
    }
}

static SDL_DisplayID
Display_IndexToID(int displayIndex)
{
    SDL_DisplayID displayID = 0;
    int count = 0;
    SDL_DisplayID *list = NULL;

    list = SDL3_GetDisplays(&count);

    if (list == NULL || count == 0) {
        SDL3_SetError("no displays");
        SDL_free(list);
        return 0;
    }

    if (displayIndex < 0 || displayIndex >= count) {
        SDL3_SetError("invalid displayIndex");
        SDL_free(list);
        return 0;
    }

    displayID = list[displayIndex];
    SDL_free(list);
    return displayID;
}

DECLSPEC int SDLCALL
SDL_GetNumVideoDisplays(void)
{
    int count = 0;
    SDL_DisplayID *list = NULL;
    list = SDL3_GetDisplays(&count);
    SDL_free(list);
    return count;
}

DECLSPEC int SDLCALL
SDL_GetWindowDisplayIndex(SDL_Window *window)
{
    SDL_DisplayID displayID = SDL3_GetDisplayForWindow(window);
    return Display_IDToIndex(displayID);
}

DECLSPEC int SDLCALL
SDL_GetPointDisplayIndex(const SDL_Point *point)
{
    SDL_DisplayID displayID = SDL3_GetDisplayForPoint(point);
    return Display_IDToIndex(displayID);
}

DECLSPEC int SDLCALL
SDL_GetRectDisplayIndex(const SDL_Rect *rect)
{
    SDL_DisplayID displayID = SDL3_GetDisplayForRect(rect);
    return Display_IDToIndex(displayID);
}

static int
Display_IDToIndex(SDL_DisplayID displayID)
{
    int displayIndex = 0;
    int count = 0, i;
    SDL_DisplayID *list = NULL;

    if (displayID == 0) {
        SDL3_SetError("invalid displayID");
        return -1;
    }

    list = SDL3_GetDisplays(&count);

    if (list == NULL || count == 0) {
        SDL3_SetError("no displays");
        SDL_free(list);
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (list[i] == displayID) {
            displayIndex = i;
            break;
        }
    }
    SDL_free(list);
    return displayIndex;
}

DECLSPEC const char * SDLCALL
SDL_GetDisplayName(int displayIndex)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayName(displayID);
}

DECLSPEC int SDLCALL
SDL_GetDisplayBounds(int displayIndex, SDL_Rect *rect)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayBounds(displayID, rect);
}

DECLSPEC int SDLCALL
SDL_GetNumDisplayModes(int displayIndex)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    int count = 0;
    const SDL_DisplayMode **list;
    list = SDL3_GetFullscreenDisplayModes(displayID, &count);
    SDL_free((void *)list);
    return count;
}

DECLSPEC int SDLCALL
SDL_GetDisplayDPI(int displayIndex, float *ddpi, float *hdpi, float *vdpi)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayPhysicalDPI(displayID, ddpi, hdpi, vdpi);
}

DECLSPEC int SDLCALL
SDL_GetDisplayUsableBounds(int displayIndex, SDL_Rect *rect)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayUsableBounds(displayID, rect);
}

DECLSPEC SDL_DisplayOrientation SDLCALL
SDL_GetDisplayOrientation(int displayIndex)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayOrientation(displayID);
}

DECLSPEC int SDLCALL
SDL_GetDisplayMode(int displayIndex, int modeIndex, SDL2_DisplayMode *mode)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    const SDL_DisplayMode **list;
    int count = 0;
    int ret = -1;
    list = SDL3_GetFullscreenDisplayModes(displayID, &count);
    if (modeIndex >= 0 && modeIndex < count) {
        if (mode) {
            DisplayMode_3to2(list[modeIndex], mode);
        }
        ret = 0;
    }
    SDL_free((void *)list);
    return ret;
}

DECLSPEC int SDLCALL
SDL_GetCurrentDisplayMode(int displayIndex, SDL2_DisplayMode *mode)
{
    const SDL_DisplayMode *dp = SDL3_GetCurrentDisplayMode(Display_IndexToID(displayIndex));
    if (dp == NULL) {
        return -1;
    }
    if (mode) {
        DisplayMode_3to2(dp, mode);
    }
    return 0;
}

DECLSPEC int SDLCALL
SDL_GetDesktopDisplayMode(int displayIndex, SDL2_DisplayMode *mode)
{
    const SDL_DisplayMode *dp = SDL3_GetDesktopDisplayMode(Display_IndexToID(displayIndex));
    if (dp == NULL) {
        return -1;
    }
    if (mode) {
        DisplayMode_3to2(dp, mode);
    }
    return 0;
}

DECLSPEC int SDLCALL
SDL_GetWindowDisplayMode(SDL_Window *window, SDL2_DisplayMode *mode)
{
    /* returns a pointer to the fullscreen mode to use or NULL for desktop mode */
    const SDL_DisplayMode *dp = SDL3_GetWindowFullscreenMode(window);
    if (dp == NULL) {
        /* Desktop mode */
        /* FIXME: is this correct ? */
        dp = SDL3_GetDesktopDisplayMode(SDL3_GetPrimaryDisplay());
        if (dp == NULL) {
            return -1;
        }
    }
    if (mode) {
        DisplayMode_3to2(dp, mode);
    }
    return 0;
}

static void
SDL_FinalizeDisplayMode(SDL_DisplayMode *mode)
{
    /* Make sure all the fields are set up correctly */
    if (mode->display_scale <= 0.0f) {
        if (mode->screen_w == 0 && mode->screen_h == 0) {
            mode->screen_w = mode->pixel_w;
            mode->screen_h = mode->pixel_h;
        }
        if (mode->pixel_w == 0 && mode->pixel_h == 0) {
            mode->pixel_w = mode->screen_w;
            mode->pixel_h = mode->screen_h;
        }
        if (mode->screen_w > 0) {
            mode->display_scale = (float)mode->pixel_w / mode->screen_w;
        }
    } else {
        if (mode->screen_w == 0 && mode->screen_h == 0) {
            mode->screen_w = (int)SDL_floorf(mode->pixel_w / mode->display_scale);
            mode->screen_h = (int)SDL_floorf(mode->pixel_h / mode->display_scale);
        }
        if (mode->pixel_w == 0 && mode->pixel_h == 0) {
            mode->pixel_w = (int)SDL_ceilf(mode->screen_w * mode->display_scale);
            mode->pixel_h = (int)SDL_ceilf(mode->screen_h * mode->display_scale);
        }
    }

    /* Make sure the screen width, pixel width, and display scale all match */
    if (mode->display_scale != 0.0f) {
        SDL_assert(mode->display_scale > 0.0f);
        SDL_assert(SDL_fabsf(mode->screen_w - (mode->pixel_w / mode->display_scale)) < 1.0f);
        SDL_assert(SDL_fabsf(mode->screen_h - (mode->pixel_h / mode->display_scale)) < 1.0f);
    }
}

static SDL_DisplayMode *
SDL_GetClosestDisplayModeForDisplay(SDL_DisplayID displayID,
                                    const SDL_DisplayMode *mode,
                                    SDL_DisplayMode *closest)
{
    Uint32 target_format;
    float target_refresh_rate;
    int i;
    SDL_DisplayMode requested_mode;
    const SDL_DisplayMode *current, *match;
    const SDL_DisplayMode **list;
    int count = 0;

    /* Make sure all the fields are filled out in the requested mode */
    requested_mode = *mode;
    SDL_FinalizeDisplayMode(&requested_mode);
    mode = &requested_mode;

    /* Default to the desktop format */
    if (mode->format) {
        target_format = mode->format;
    } else {
/* FIXME: Desktop mode ?
        target_format = display->desktop_mode.format;*/
        target_format = mode->format;
    }

    /* Default to the desktop refresh rate */
    if (mode->refresh_rate > 0.0f) {
        target_refresh_rate = mode->refresh_rate;
    } else {
/* FIXME: Desktop mode ?
        target_refresh_rate = display->desktop_mode.refresh_rate;*/
        target_refresh_rate = mode->refresh_rate;
    }

    match = NULL;
    list = SDL3_GetFullscreenDisplayModes(displayID, &count);
    for (i = 0; i < count; ++i) {
        current = list[i];

        if (current->pixel_w && (current->pixel_w < mode->pixel_w)) {
            /* Out of sorted modes large enough here */
            break;
        }
        if (current->pixel_h && (current->pixel_h < mode->pixel_h)) {
            if (current->pixel_w && (current->pixel_w == mode->pixel_w)) {
                /* Out of sorted modes large enough here */
                break;
            }
            /* Wider, but not tall enough, due to a different
               aspect ratio. This mode must be skipped, but closer
               modes may still follow. */
            continue;
        }
        if (match == NULL || current->pixel_w < match->pixel_w || current->pixel_h < match->pixel_h) {
            match = current;
            continue;
        }
        if (current->format != match->format) {
            /* Sorted highest depth to lowest */
            if (current->format == target_format ||
                (SDL_BITSPERPIXEL(current->format) >=
                     SDL_BITSPERPIXEL(target_format) &&
                 SDL_PIXELTYPE(current->format) ==
                     SDL_PIXELTYPE(target_format))) {
                match = current;
            }
            continue;
        }
        if (current->refresh_rate != match->refresh_rate) {
            /* Sorted highest refresh to lowest */
            if (current->refresh_rate >= target_refresh_rate) {
                match = current;
                continue;
            }
        }
    }

    if (match) {
        SDL3_zerop(closest);
        if (match->format) {
            closest->format = match->format;
        } else {
            closest->format = mode->format;
        }
        if (match->screen_w && match->screen_h) {
            closest->screen_w = match->screen_w;
            closest->screen_h = match->screen_h;
        } else {
            closest->screen_w = mode->screen_w;
            closest->screen_h = mode->screen_h;
        }
        if (match->pixel_w && match->pixel_h) {
            closest->pixel_w = match->pixel_w;
            closest->pixel_h = match->pixel_h;
        } else {
            closest->pixel_w = mode->pixel_w;
            closest->pixel_h = mode->pixel_h;
        }
        if (match->refresh_rate > 0.0f) {
            closest->refresh_rate = match->refresh_rate;
        } else {
            closest->refresh_rate = mode->refresh_rate;
        }
        closest->driverdata = match->driverdata;

        /* Pick some reasonable defaults if the app and driver don't care */
        if (!closest->format) {
            closest->format = SDL_PIXELFORMAT_RGB888;
        }
        if (!closest->screen_w) {
            closest->screen_w = 640;
        }
        if (!closest->screen_h) {
            closest->screen_h = 480;
        }
        SDL_FinalizeDisplayMode(closest);
        SDL_free(list);
        return closest;
    }
    SDL_free((void *)list);
    return NULL;
}

DECLSPEC SDL2_DisplayMode * SDLCALL
SDL_GetClosestDisplayMode(int displayIndex, const SDL2_DisplayMode *mode, SDL2_DisplayMode *closest)
{
    SDL_DisplayMode mode3;
    SDL_DisplayMode closest3;
    SDL_DisplayMode *ret3;

    if (mode == NULL || closest == NULL) {
        SDL_InvalidParamError("mode/closest");
        return NULL;
    }

    /* silence compiler */
    SDL3_zero(mode3);
    SDL3_zero(closest3);

    DisplayMode_2to3(closest, &closest3);
    DisplayMode_2to3(mode, &mode3);
    ret3 = SDL_GetClosestDisplayModeForDisplay(Display_IndexToID(displayIndex), &mode3, &closest3);
    if (ret3 == NULL) {
        return NULL;
    }

    DisplayMode_3to2(ret3, closest);
    return closest;
}

DECLSPEC int SDLCALL
SDL_SetWindowDisplayMode(SDL_Window *window, const SDL2_DisplayMode *mode)
{
    SDL_DisplayMode dp;
    DisplayMode_2to3(mode, &dp);
    if (SDL3_SetWindowFullscreenMode(window, mode ? &dp : NULL) == 0) {
        return 0;
    } else {
        int count = 0;
        const SDL_DisplayMode **list;
        int ret = -1;

        /* FIXME: at least set a valid fullscreen mode */
        list = SDL3_GetFullscreenDisplayModes(SDL3_GetPrimaryDisplay(), &count);
        if (list && count) {
            ret = SDL3_SetWindowFullscreenMode(window, list[0]);
        }
        SDL_free((void *)list);
        return ret;
    }
}

DECLSPEC int SDLCALL
SDL_RenderDrawPoint(SDL_Renderer *renderer, int x, int y)
{
    SDL_FPoint fpoint;
    fpoint.x = (float)x;
    fpoint.y = (float)y;
    return SDL3_RenderPoints(renderer, &fpoint, 1);
}

DECLSPEC int SDLCALL
SDL_RenderDrawPoints(SDL_Renderer *renderer,
                     const SDL_Point *points, int count)
{
    SDL_FPoint *fpoints;
    int i;
    int retval;

    if (points == NULL) {
        return SDL3_InvalidParamError("SDL_RenderPoints(): points");
    }

    fpoints = (SDL_FPoint *) SDL3_malloc(sizeof (SDL_FPoint) * count);
    if (fpoints == NULL) {
        return SDL3_OutOfMemory();
    }

    for (i = 0; i < count; ++i) {
        fpoints[i].x = (float)points[i].x;
        fpoints[i].y = (float)points[i].y;
    }

    retval = SDL3_RenderPoints(renderer, fpoints, count);

    SDL3_free(fpoints);

    return retval;
}

DECLSPEC int SDLCALL
SDL_RenderLine(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)
{
    SDL_FPoint points[2];
    points[0].x = (float)x1;
    points[0].y = (float)y1;
    points[1].x = (float)x2;
    points[1].y = (float)y2;
    return SDL3_RenderLines(renderer, points, 2);
}

DECLSPEC int SDLCALL
SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
{
    SDL_FPoint points[2];
    points[0].x = (float)x1;
    points[0].y = (float)y1;
    points[1].x = (float)x2;
    points[1].y = (float)y2;
    return SDL3_RenderLines(renderer, points, 2);
}


DECLSPEC int SDLCALL
SDL_RenderDrawLines(SDL_Renderer *renderer, const SDL_Point *points, int count)
{
    SDL_FPoint *fpoints;
    int i;
    int retval;

    if (points == NULL) {
        return SDL3_InvalidParamError("SDL_RenderLines(): points");
    }
    if (count < 2) {
        return 0;
    }

    fpoints = (SDL_FPoint *) SDL3_malloc(sizeof (SDL_FPoint) * count);
    if (fpoints == NULL) {
        return SDL3_OutOfMemory();
    }

    for (i = 0; i < count; ++i) {
        fpoints[i].x = (float)points[i].x;
        fpoints[i].y = (float)points[i].y;
    }

    retval = SDL3_RenderLines(renderer, fpoints, count);

    SDL3_free(fpoints);

    return retval;
}

DECLSPEC int SDLCALL
SDL_RenderDrawRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    SDL_FRect frect;
    SDL_FRect *prect = NULL;

    if (rect) {
        frect.x = (float)rect->x;
        frect.y = (float)rect->y;
        frect.w = (float)rect->w;
        frect.h = (float)rect->h;
        prect = &frect;
    }

    return SDL3_RenderRect(renderer, prect);
}

DECLSPEC int SDLCALL
SDL_RenderDrawRects(SDL_Renderer *renderer, const SDL_Rect *rects, int count)
{
    int i;

    if (rects == NULL) {
        return SDL3_InvalidParamError("SDL_RenderRectsFloat(): rects");
    }
    if (count < 1) {
        return 0;
    }

    for (i = 0; i < count; ++i) {
        if (SDL_RenderDrawRect(renderer, &rects[i]) < 0) {
            return -1;
        }
    }
    return 0;
}

DECLSPEC int SDLCALL
SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    SDL_FRect frect;
    if (rect) {
        frect.x = (float)rect->x;
        frect.y = (float)rect->y;
        frect.w = (float)rect->w;
        frect.h = (float)rect->h;
        return SDL3_RenderFillRect(renderer, &frect);
    }
    return SDL3_RenderFillRect(renderer, NULL);
}

DECLSPEC int SDLCALL
SDL_RenderFillRects(SDL_Renderer *renderer, const SDL_FRect *rects, int count)
{
    SDL_FRect *frects;
    int i;
    int retval;

    if (rects == NULL) {
        return SDL3_InvalidParamError("SDL_RenderFillRectsFloat(): rects");
    }
    if (count < 1) {
        return 0;
    }

    frects = (SDL_FRect *) SDL3_malloc(sizeof (SDL_FRect) * count);
    if (frects == NULL) {
        return SDL3_OutOfMemory();
    }
    for (i = 0; i < count; ++i) {
        frects[i].x = rects[i].x;
        frects[i].y = rects[i].y;
        frects[i].w = rects[i].w;
        frects[i].h = rects[i].h;
    }

    retval = SDL3_RenderFillRects(renderer, frects, count);

    SDL3_free(frects);

    return retval;
}

DECLSPEC int SDLCALL
SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect)
{
    SDL_FRect dstfrect;
    SDL_FRect *pdstfrect = NULL;
    if (dstrect) {
        dstfrect.x = (float)dstrect->x;
        dstfrect.y = (float)dstrect->y;
        dstfrect.w = (float)dstrect->w;
        dstfrect.h = (float)dstrect->h;
        pdstfrect = &dstfrect;
    }
    return SDL3_RenderTexture(renderer, texture, srcrect, pdstfrect);
}

DECLSPEC int SDLCALL
SDL_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture,
                     const SDL_Rect *srcrect, const SDL_Rect *dstrect,
                     const double angle, const SDL_Point *center, const SDL_RendererFlip flip)
{
    SDL_FRect dstfrect;
    SDL_FRect *pdstfrect = NULL;
    SDL_FPoint fcenter;
    SDL_FPoint *pfcenter = NULL;

    if (dstrect) {
        dstfrect.x = (float)dstrect->x;
        dstfrect.y = (float)dstrect->y;
        dstfrect.w = (float)dstrect->w;
        dstfrect.h = (float)dstrect->h;
        pdstfrect = &dstfrect;
    }

    if (center) {
        fcenter.x = (float)center->x;
        fcenter.y = (float)center->y;
        pfcenter = &fcenter;
    }

    return SDL3_RenderTextureRotated(renderer, texture, srcrect, pdstfrect, angle, pfcenter, flip);
}

/* SDL3 removed window parameter from SDL_Vulkan_GetInstanceExtensions() */
DECLSPEC SDL_bool SDLCALL
SDL_Vulkan_GetInstanceExtensions(SDL_Window *window, unsigned int *pCount, const char **pNames)
{
    (void) window;
    return SDL3_Vulkan_GetInstanceExtensions(pCount, pNames);
}


/* SDL3 doesn't have 3dNow. */
#if defined(__GNUC__) && defined(__i386__)
#define cpuid(func, a, b, c, d) \
    __asm__ __volatile__ ( \
"        pushl %%ebx        \n" \
"        xorl %%ecx,%%ecx   \n" \
"        cpuid              \n" \
"        movl %%ebx, %%esi  \n" \
"        popl %%ebx         \n" : \
            "=a" (a), "=S" (b), "=c" (c), "=d" (d) : "a" (func))
#elif defined(__GNUC__) && defined(__x86_64__)
#define cpuid(func, a, b, c, d) \
    __asm__ __volatile__ ( \
"        pushq %%rbx        \n" \
"        xorq %%rcx,%%rcx   \n" \
"        cpuid              \n" \
"        movq %%rbx, %%rsi  \n" \
"        popq %%rbx         \n" : \
            "=a" (a), "=S" (b), "=c" (c), "=d" (d) : "a" (func))
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
#define cpuid(func, a, b, c, d) \
    __asm { \
        __asm mov eax, func \
        __asm xor ecx, ecx \
        __asm cpuid \
        __asm mov a, eax \
        __asm mov b, ebx \
        __asm mov c, ecx \
        __asm mov d, edx \
}
#elif defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#define cpuid(func, a, b, c, d) \
{ \
    int CPUInfo[4]; \
    __cpuid(CPUInfo, func); \
    a = CPUInfo[0]; \
    b = CPUInfo[1]; \
    c = CPUInfo[2]; \
    d = CPUInfo[3]; \
}
#else
#define cpuid(func, a, b, c, d) \
    do { a = b = c = d = 0; (void) a; (void) b; (void) c; (void) d; } while (0)
#endif

DECLSPEC SDL_bool SDLCALL
SDL_Has3DNow(void)
{
    /* if there's no MMX, presumably there's no 3DNow. */
    /* This lets SDL3 deal with the check for CPUID support _at all_ and eliminates non-AMD CPUs. */
    if (SDL3_HasMMX()) {
        int a, b, c, d;
        cpuid(0x80000000, a, b, c, d);
        if ((unsigned int)a >= 0x80000001) {
            cpuid(0x80000001, a, b, c, d);
            return (d & 0x80000000) ? SDL_TRUE : SDL_FALSE;
        }
    }
    return SDL_FALSE;
}

/* This was always a basic wrapper over SDL_free; SDL3 removed it and says use SDL_free directly. */
DECLSPEC void SDLCALL
SDL_FreeWAV(Uint8 *audio_buf)
{
    SDL3_free(audio_buf);
}

/* SDL_WINDOW_FULLSCREEN_DESKTOP has been removed, and you can call
 * SDL_GetWindowFullscreenMode() to see whether an exclusive fullscreen
 * mode will be used or the fullscreen desktop mode will be used when
 * the window is fullscreen. */
/* SDL3 removed SDL_WINDOW_SHOWN as redundant to SDL_WINDOW_HIDDEN. */
DECLSPEC Uint32 SDLCALL
SDL_GetWindowFlags(SDL_Window *window)
{
    Uint32 flags = SDL3_GetWindowFlags(window);
    if ((flags & SDL_WINDOW_HIDDEN) == 0) {
        flags |= SDL2_WINDOW_SHOWN;
    }
    if ((flags & SDL_WINDOW_FULLSCREEN)) {
        if (SDL3_GetWindowFullscreenMode(window)) {
            flags |= SDL_WINDOW_FULLSCREEN;
        } else {
            flags |= SDL2_WINDOW_FULLSCREEN_DESKTOP;
        }
    }
    return flags;
}

DECLSPEC SDL_Window * SDLCALL
SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
{
    flags &= ~SDL2_WINDOW_SHOWN;
    if (flags & SDL2_WINDOW_FULLSCREEN_DESKTOP) {
        flags &= ~SDL2_WINDOW_FULLSCREEN_DESKTOP;
        flags |= SDL_WINDOW_FULLSCREEN; /* FIXME  force fullscreen desktop ? */
    }
    return SDL3_CreateWindow(title, x, y, w, h, flags);
}

DECLSPEC int SDLCALL
SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags)
{
    int flags3 = SDL_FALSE;

    if (flags & SDL2_WINDOW_FULLSCREEN_DESKTOP) {
        flags3 = SDL_TRUE;
    }
    if (flags & SDL_WINDOW_FULLSCREEN) {
        flags3 = SDL_TRUE;
    }

    return SDL3_SetWindowFullscreen(window, flags3);
}

/* SDL3 added a return value. We just throw it away for SDL2. */
DECLSPEC void SDLCALL
SDL_GL_SwapWindow(SDL_Window *window)
{
    (void) SDL3_GL_SwapWindow(window);
}

/* SDL3 split this into getter/setter functions. */
DECLSPEC Uint8 SDLCALL
SDL_EventState(Uint32 type, int state)
{
    const int retval = SDL3_EventEnabled(type) ? SDL2_ENABLE : SDL2_DISABLE;
    if (state == SDL2_ENABLE) {
        SDL3_SetEventEnabled(type, SDL_TRUE);
    } else if (state == SDL2_DISABLE) {
        SDL3_SetEventEnabled(type, SDL_FALSE);
    }
    return retval;
}

/* SDL3 split this into getter/setter functions. */
DECLSPEC int SDLCALL
SDL_ShowCursor(int state)
{
    int retval = SDL3_CursorVisible() ? SDL2_ENABLE : SDL2_DISABLE;
    if ((state == SDL2_ENABLE) && (SDL3_ShowCursor() < 0)) {
        retval = -1;
    } else if ((state == SDL2_DISABLE) && (SDL3_HideCursor() < 0)) {
        retval = -1;
    }
    return retval;
}

/* SDL3 split this into getter/setter functions. */
DECLSPEC int SDLCALL
SDL_GameControllerEventState(int state)
{
    int retval = state;
    if (state == SDL2_ENABLE) {
        SDL3_SetGamepadEventsEnabled(SDL_TRUE);
    } else if (state == SDL2_DISABLE) {
        SDL3_SetGamepadEventsEnabled(SDL_FALSE);
    } else {
        retval = SDL3_GamepadEventsEnabled() ? SDL2_ENABLE : SDL2_DISABLE;
    }
    return retval;
}

/* SDL3 split this into getter/setter functions. */
DECLSPEC int SDLCALL
SDL_JoystickEventState(int state)
{
    int retval = state;
    if (state == SDL2_ENABLE) {
        SDL3_SetJoystickEventsEnabled(SDL_TRUE);
    } else if (state == SDL2_DISABLE) {
        SDL3_SetJoystickEventsEnabled(SDL_FALSE);
    } else {
        retval = SDL3_JoystickEventsEnabled() ? SDL2_ENABLE : SDL2_DISABLE;
    }
    return retval;
}


/* SDL3 dumped the index/instance difference for various devices. */

static SDL_JoystickID
GetJoystickInstanceFromIndex(int idx)
{
    if ((idx < 0) || (idx >= num_joysticks)) {
        SDL3_SetError("There are %d joysticks available", num_joysticks);
        return 0;
    }
    return joystick_list[idx];
}

/* !!! FIXME: when we override SDL_Quit(), we need to free/reset joystick_list and friends*/
/* !!! FIXME: put a mutex on the joystick and sensor lists. Strictly speaking, this will break if you multithread it, but it doesn't have to crash. */

DECLSPEC int SDLCALL
SDL_NumJoysticks(void)
{
    SDL3_free(joystick_list);
    joystick_list = SDL3_GetJoysticks(&num_joysticks);
    if (joystick_list == NULL) {
        num_joysticks = 0;
        return -1;
    }
    return num_joysticks;
}

static int
GetIndexFromJoystickInstance(SDL_JoystickID jid) {
    int i;

    /* Refresh */
    SDL_NumJoysticks();

    for (i = 0; i < num_joysticks; i++) {
        if (joystick_list[i] == jid) {
            return i;
        }
    }
    return -1;
}


DECLSPEC SDL_JoystickGUID SDLCALL
SDL_JoystickGetDeviceGUID(int idx)
{
    SDL_JoystickGUID guid;
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    if (!jid) {
        SDL3_zero(guid);
    } else {
        guid = SDL3_GetJoystickInstanceGUID(jid);
    }
    return guid;
}

DECLSPEC const char* SDLCALL
SDL_JoystickNameForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstanceName(jid) : NULL;
}

DECLSPEC SDL_Joystick* SDLCALL
SDL_JoystickOpen(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_OpenJoystick(jid) : NULL;
}

DECLSPEC Uint16 SDLCALL
SDL_JoystickGetDeviceVendor(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstanceVendor(jid) : 0;
}

DECLSPEC Uint16 SDLCALL
SDL_JoystickGetDeviceProduct(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstanceProduct(jid) : 0;
}

DECLSPEC Uint16 SDLCALL
SDL_JoystickGetDeviceProductVersion(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstanceProductVersion(jid) : 0;
}

DECLSPEC SDL_JoystickType SDLCALL
SDL_JoystickGetDeviceType(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstanceType(jid) : SDL_JOYSTICK_TYPE_UNKNOWN;
}

DECLSPEC SDL2_JoystickID SDLCALL
SDL_JoystickGetDeviceInstanceID(int idx)
{
    /* this counts on a Uint32 not overflowing an Sint32. */
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    if (!jid) {
        return -1;
    }
    return (SDL2_JoystickID)jid;
}

DECLSPEC SDL2_JoystickID SDLCALL
SDL_JoystickInstanceID(SDL_Joystick *joystick)
{
    const SDL_JoystickID jid = SDL3_GetJoystickInstanceID(joystick);
    if (!jid) {
        return -1;
    }
    return (SDL2_JoystickID)jid;
}

DECLSPEC int SDLCALL
SDL_JoystickGetDevicePlayerIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstancePlayerIndex(jid) : -1;
}

DECLSPEC SDL_bool SDLCALL
SDL_JoystickIsVirtual(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_IsJoystickVirtual(jid) : SDL_FALSE;
}

DECLSPEC const char* SDLCALL
SDL_JoystickPathForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickInstancePath(jid) : NULL;
}

DECLSPEC char* SDLCALL
SDL_GameControllerMappingForDeviceIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadInstanceMapping(jid) : NULL;
}

DECLSPEC SDL_bool SDLCALL
SDL_IsGameController(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_IsGamepad(jid) : SDL_FALSE;
}

DECLSPEC const char* SDLCALL
SDL_GameControllerNameForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadInstanceName(jid) : NULL;
}

DECLSPEC SDL_GameController* SDLCALL
SDL_GameControllerOpen(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_OpenGamepad(jid) : NULL;
}

DECLSPEC SDL_GameControllerType SDLCALL
SDL_GameControllerTypeForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadInstanceType(jid) : SDL_GAMEPAD_TYPE_UNKNOWN;
}

DECLSPEC const char* SDLCALL
SDL_GameControllerPathForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadInstancePath(jid) : NULL;
}

DECLSPEC int SDLCALL
SDL_JoystickAttachVirtual(SDL_JoystickType type, int naxes, int nbuttons, int nhats)
{
    SDL_JoystickID jid = SDL3_AttachVirtualJoystick(type, naxes, nbuttons, nhats);
    return GetIndexFromJoystickInstance(jid);
}

DECLSPEC int SDLCALL
SDL_JoystickAttachVirtualEx(const SDL_VirtualJoystickDesc *desc)
{
    SDL_JoystickID jid = SDL3_AttachVirtualJoystickEx(desc);
    return GetIndexFromJoystickInstance(jid);
}

DECLSPEC int SDLCALL
SDL_JoystickDetachVirtual(int device_index)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(device_index);
    return jid ? SDL3_DetachVirtualJoystick(jid) : -1;
}


/* !!! FIXME: when we override SDL_Quit(), we need to free/reset sensor_list */

static SDL_SensorID
GetSensorInstanceFromIndex(int idx)
{
    if ((idx < 0) || (idx >= num_sensors)) {
        SDL3_SetError("There are %d sensors available", num_sensors);
        return 0;
    }
    return sensor_list[idx];
}

DECLSPEC int SDLCALL
SDL_NumSensors(void)
{
    SDL3_free(sensor_list);
    sensor_list = SDL3_GetSensors(&num_sensors);
    if (sensor_list == NULL) {
        num_sensors = 0;
        return -1;
    }
    return num_sensors;
}

DECLSPEC const char* SDLCALL
SDL_SensorGetDeviceName(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_GetSensorInstanceName(sid) : NULL;
}

DECLSPEC SDL_SensorType SDLCALL
SDL_SensorGetDeviceType(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_GetSensorInstanceType(sid) : SDL_SENSOR_INVALID;
}

DECLSPEC int SDLCALL
SDL_SensorGetDeviceNonPortableType(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_GetSensorInstanceNonPortableType(sid) : -1;
}

DECLSPEC SDL2_SensorID SDLCALL
SDL_SensorGetDeviceInstanceID(int idx)
{
    /* this counts on a Uint32 not overflowing an Sint32. */
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    if (!sid) {
        return -1;
    }
    return (SDL2_SensorID)sid;
}

DECLSPEC SDL2_SensorID SDLCALL
SDL_SensorGetInstanceID(SDL_Sensor *sensor)
{
    const SDL_SensorID sid = SDL3_GetSensorInstanceID(sensor);
    if (!sid) {
        return -1;
    }
    return (SDL2_SensorID)sid;
}

DECLSPEC SDL_Sensor* SDLCALL
SDL_SensorOpen(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_OpenSensor(sid) : NULL;
}


DECLSPEC void * SDLCALL
SDL_SIMDAlloc(const size_t len)
{
    return SDL3_aligned_alloc(SDL3_SIMDGetAlignment(), len);
}

DECLSPEC void * SDLCALL
SDL_SIMDRealloc(void *mem, const size_t len)
{
    const size_t alignment = SDL3_SIMDGetAlignment();
    const size_t padding = (alignment - (len % alignment)) % alignment;
    Uint8 *retval = (Uint8 *)mem;
    void *oldmem = mem;
    size_t memdiff = 0, ptrdiff;
    Uint8 *ptr;
    size_t to_allocate;

    /* alignment + padding + sizeof (void *) is bounded (a few hundred
     * bytes max), so no need to check for overflow within that argument */
    if (SDL_size_add_overflow(len, alignment + padding + sizeof(void *), &to_allocate)) {
        return NULL;
    }

    if (mem) {
        mem = *(((void **)mem) - 1);

        /* Check the delta between the real pointer and user pointer */
        memdiff = ((size_t)oldmem) - ((size_t)mem);
    }

    ptr = (Uint8 *)SDL3_realloc(mem, to_allocate);

    if (ptr == NULL) {
        return NULL; /* Out of memory, bail! */
    }

    /* Store the actual allocated pointer right before our aligned pointer. */
    retval = ptr + sizeof(void *);
    retval += alignment - (((size_t)retval) % alignment);

    /* Make sure the delta is the same! */
    if (mem) {
        ptrdiff = ((size_t)retval) - ((size_t)ptr);
        if (memdiff != ptrdiff) { /* Delta has changed, copy to new offset! */
            oldmem = (void *)(((uintptr_t)ptr) + memdiff);

            /* Even though the data past the old `len` is undefined, this is the
             * only length value we have, and it guarantees that we copy all the
             * previous memory anyhow.
             */
            SDL3_memmove(retval, oldmem, len);
        }
    }

    /* Actually store the allocated pointer, finally. */
    *(((void **)retval) - 1) = ptr;
    return retval;
}

DECLSPEC void SDLCALL
SDL_SIMDFree(void *ptr)
{
    SDL3_aligned_free(ptr);
}


static SDL_bool
SDL_IsSupportedAudioFormat(const SDL_AudioFormat fmt)
{
    switch (fmt) {
    case AUDIO_U8:
    case AUDIO_S8:
    case AUDIO_U16LSB:
    case AUDIO_S16LSB:
    case AUDIO_U16MSB:
    case AUDIO_S16MSB:
    case AUDIO_S32LSB:
    case AUDIO_S32MSB:
    case AUDIO_F32LSB:
    case AUDIO_F32MSB:
        return SDL_TRUE; /* supported. */

    default:
        break;
    }

    return SDL_FALSE; /* unsupported. */
}

static SDL_bool SDL_IsSupportedChannelCount(const int channels)
{
    return ((channels >= 1) && (channels <= 8)) ? SDL_TRUE : SDL_FALSE;
}


typedef struct {
    SDL_AudioFormat src_format;
    Uint8 src_channels;
    int src_rate;
    SDL_AudioFormat dst_format;
    Uint8 dst_channels;
    int dst_rate;
} AudioParam;

#define RESAMPLER_BITS_PER_SAMPLE           16
#define RESAMPLER_SAMPLES_PER_ZERO_CROSSING (1 << ((RESAMPLER_BITS_PER_SAMPLE / 2) + 1))


DECLSPEC int SDLCALL
SDL_BuildAudioCVT(SDL_AudioCVT *cvt,
                  SDL_AudioFormat src_format,
                  Uint8 src_channels,
                  int src_rate,
                  SDL_AudioFormat dst_format,
                  Uint8 dst_channels,
                  int dst_rate)
{
    /* Sanity check target pointer */
    if (cvt == NULL) {
        return SDL_InvalidParamError("cvt");
    }

    /* Make sure we zero out the audio conversion before error checking */
    SDL3_zerop(cvt);

    if (!SDL_IsSupportedAudioFormat(src_format)) {
        return SDL_SetError("Invalid source format");
    }
    if (!SDL_IsSupportedAudioFormat(dst_format)) {
        return SDL_SetError("Invalid destination format");
    }
    if (!SDL_IsSupportedChannelCount(src_channels)) {
        return SDL_SetError("Invalid source channels");
    }
    if (!SDL_IsSupportedChannelCount(dst_channels)) {
        return SDL_SetError("Invalid destination channels");
    }
    if (src_rate <= 0) {
        return SDL_SetError("Source rate is equal to or less than zero");
    }
    if (dst_rate <= 0) {
        return SDL_SetError("Destination rate is equal to or less than zero");
    }
    if (src_rate >= SDL_MAX_SINT32 / RESAMPLER_SAMPLES_PER_ZERO_CROSSING) {
        return SDL_SetError("Source rate is too high");
    }
    if (dst_rate >= SDL_MAX_SINT32 / RESAMPLER_SAMPLES_PER_ZERO_CROSSING) {
        return SDL_SetError("Destination rate is too high");
    }

#if DEBUG_CONVERT
    SDL_Log("SDL_AUDIO_CONVERT: Build format %04x->%04x, channels %u->%u, rate %d->%d\n",
            src_format, dst_format, src_channels, dst_channels, src_rate, dst_rate);
#endif

    /* Start off with no conversion necessary */
    cvt->src_format = src_format;
    cvt->dst_format = dst_format;
    cvt->needed = 0;
    cvt->filter_index = 0;
    SDL3_zeroa(cvt->filters);
    cvt->len_mult = 1;
    cvt->len_ratio = 1.0;
    cvt->rate_incr = ((double)dst_rate) / ((double)src_rate);

    { /* Use the filters[] to store some data ... */
        AudioParam ap;
        ap.src_format = src_format;
        ap.src_channels = src_channels;
        ap.src_rate = src_rate;
        ap.dst_format = dst_format;
        ap.dst_channels = dst_channels;
        ap.dst_rate = dst_rate;

        /* Store at the end of filters[], aligned */
        SDL3_memcpy(
            (Uint8 *)&cvt->filters[SDL_AUDIOCVT_MAX_FILTERS + 1] - (sizeof(AudioParam) & ~3),
            &ap,
            sizeof(ap));

        cvt->filters[0] = NULL;
        cvt->needed = 1;
        if (src_format == dst_format && src_rate == dst_rate && src_channels == dst_channels) {
            cvt->needed = 0;
        }

        if (src_rate < dst_rate) {
            const int mult = (dst_rate / src_rate);
            cvt->len_mult *= mult;
            cvt->len_ratio *= mult;
        } else {
            const int div = (src_rate / dst_rate);
            cvt->len_ratio /= div;
        }

        if (src_channels < dst_channels) {
            cvt->len_mult = ((cvt->len_mult * dst_channels) + (src_channels - 1)) / src_channels;
        }
    }

    return cvt->needed;
}


DECLSPEC int SDLCALL
SDL_ConvertAudio(SDL_AudioCVT *cvt)
{

    SDL_AudioStream *stream;
    SDL_AudioFormat src_format, dst_format;
    int src_channels, src_rate;
    int dst_channels, dst_rate;

    int src_len, dst_len, real_dst_len;
    int src_samplesize, dst_samplesize;

    /* Sanity check target pointer */
    if (cvt == NULL) {
        return SDL_InvalidParamError("cvt");
    }

    { /* Fetch from the end of filters[], aligned */
        AudioParam ap;

        SDL3_memcpy(
            &ap,
            (Uint8 *)&cvt->filters[SDL_AUDIOCVT_MAX_FILTERS + 1] - (sizeof(AudioParam) & ~3),
            sizeof(ap));

        src_format = ap.dst_format;
        src_channels = ap.src_channels;
        src_rate = ap.src_rate;
        dst_format = ap.dst_format;
        dst_channels = ap.dst_channels;
        dst_rate = ap.dst_rate;
    }

    stream = SDL3_CreateAudioStream(src_format, src_channels, src_rate,
                                    dst_format, dst_channels, dst_rate);
    if (stream == NULL) {
        goto failure;
    }

    src_samplesize = (SDL_AUDIO_BITSIZE(src_format) / 8) * src_channels;
    dst_samplesize = (SDL_AUDIO_BITSIZE(dst_format) / 8) * dst_channels;

    src_len = cvt->len & ~(src_samplesize - 1);
    dst_len = dst_samplesize * (src_len / src_samplesize);
    if (src_rate < dst_rate) {
        const double mult = ((double)dst_rate) / ((double)src_rate);
        dst_len *= (int) SDL3_ceil(mult);
    }

    /* Run the audio converter */
    if (SDL3_PutAudioStreamData(stream, cvt->buf, src_len) < 0 ||
        SDL3_FlushAudioStream(stream) < 0) {
        goto failure;
    }

    dst_len = SDL_min(dst_len, cvt->len * cvt->len_mult);
    dst_len = dst_len & ~(dst_samplesize - 1);

    /* Get back in the same buffer */
    real_dst_len = SDL3_GetAudioStreamData(stream, cvt->buf, dst_len);
    if (real_dst_len < 0) {
        goto failure;
    }

    cvt->len_cvt = real_dst_len;

    SDL3_DestroyAudioStream(stream);
    return 0;

failure:
    SDL3_DestroyAudioStream(stream);
    return -1;
}

DECLSPEC void SDLCALL
SDL_GL_GetDrawableSize(SDL_Window *window, int *w, int *h)
{
    SDL_GetWindowSizeInPixels(window, w, h);
}

DECLSPEC void SDLCALL
SDL_Vulkan_GetDrawableSize(SDL_Window *window, int *w, int *h)
{
    SDL_GetWindowSizeInPixels(window, w, h);
}

DECLSPEC void SDLCALL
SDL_Metal_GetDrawableSize(SDL_Window *window, int *w, int *h)
{
    SDL_GetWindowSizeInPixels(window, w, h);
}

#ifdef __WINRT__
DECLSPEC int SDLCALL
SDL_WinRTRunApp(SDL_main_func mainFunction, void *reserved)
{
    return SDL3_RunApp(0, NULL, mainFunction, reserved);
}
#endif

#if defined(__GDK__)
DECLSPEC int SDLCALL
SDL_GDKRunApp(SDL_main_func mainFunction, void *reserved)
{
    return SDL3_RunApp(0, NULL, mainFunction, reserved);
}
#endif

#ifdef __IOS__
DECLSPEC int SDLCALL
SDL_UIKitRunApp(int argc, char *argv[], SDL_main_func mainFunction)
{
    return SDL3_RunApp(argc, argv, mainFunction, NULL);
}
#endif

#ifdef __cplusplus
}
#endif

/* vi: set ts=4 sw=4 expandtab: */
