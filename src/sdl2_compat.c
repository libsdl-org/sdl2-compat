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
/* force SDL_DECLSPEC off...it's all internal symbols now.
   These will have actual #defines during SDL_dynapi.c only */
#ifdef SDL_DECLSPEC
#undef SDL_DECLSPEC
#endif
#define SDL_DECLSPEC
#endif

#define STRINGIFY2(V) #V
#define STRINGIFY(V) STRINGIFY2(V)

/*
 * We report the library version as
 * 2.$(SDL2_COMPAT_VERSION_MINOR).$(SDL2_COMPAT_VERSION_PATCH). This number
 * should be way ahead of what SDL2 Classic would report, so apps can
 * decide if they're running under the compat layer, if they really care.
 * The patch level changes in release cycles. The minor version starts at 90
 * to be high by default, and usually doesn't change (and maybe never changes).
 * The number might increment past 90 if there are a ton of releases.
 */
#define SDL2_COMPAT_VERSION_MINOR 30
#define SDL2_COMPAT_VERSION_PATCH 50

#ifndef SDL2COMPAT_REVISION
#define SDL2COMPAT_REVISION "SDL-2." STRINGIFY(SDL2_COMPAT_VERSION_MINOR) "." STRINGIFY(SDL2_COMPAT_VERSION_PATCH) "-no-vcs"
#endif

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
#ifdef __ANDROID__
#include <android/log.h>
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


/* SDL2 function prototypes:  */
#include "sdl2_protos.h"


#ifdef __cplusplus
extern "C" {
#endif

/** Define SDL2COMPAT_TEST_SYMS=1 to have warnings about wrong prototypes of src/sdl3_sym.h
 *  It won't compile but it helps to make sure it's sync'ed with SDL3 headers.
 */
#ifndef SDL2COMPAT_TEST_SYMS
#define SDL2COMPAT_TEST_SYMS 0
#endif
#if SDL2COMPAT_TEST_SYMS
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
    SDL_DECLSPEC rc SDLCALL SDL_##fn params { ret SDL3_##fn args; }
#define SDL3_SYM_PASSTHROUGH_BOOL(rc,fn,params,args,ret) \
    SDL_DECLSPEC SDL2_bool SDLCALL SDL_##fn params { ret (SDL3_##fn args) ? SDL2_TRUE : SDL2_FALSE; }
#define SDL3_SYM_PASSTHROUGH_RETCODE(rc,fn,params,args,ret) \
    SDL_DECLSPEC int SDLCALL SDL_##fn params { ret (SDL3_##fn args) ? 0 : -1; }
#include "sdl3_syms.h"

/* Things that were renamed and _should_ be binary compatible pass right through with the correct names... */
#define SDL3_SYM_RENAMED(rc,oldfn,newfn,params,args,ret) \
    SDL_DECLSPEC rc SDLCALL SDL_##oldfn params { ret SDL3_##newfn args; }
#define SDL3_SYM_RENAMED_BOOL(rc,oldfn,newfn,params,args,ret) \
    SDL_DECLSPEC SDL2_bool SDLCALL SDL_##oldfn params { ret (SDL3_##newfn args) ? SDL2_TRUE : SDL2_FALSE; }
#define SDL3_SYM_RENAMED_RETCODE(rc,oldfn,newfn,params,args,ret) \
    SDL_DECLSPEC int SDLCALL SDL_##oldfn params { ret (SDL3_##newfn args) ? 0 : -1; }
#include "sdl3_syms.h"


/* these are macros (etc) in the SDL headers, so make our own. */
#define SDL3_AtomicIncRef(a)  SDL3_AddAtomicInt(a, 1)
#define SDL3_AtomicDecRef(a) (SDL3_AddAtomicInt(a,-1) == 1)
#define SDL3_Unsupported()            SDL3_SetError("That operation is not supported")
#define SDL3_InvalidParamError(param) SDL3_SetError("Parameter '%s' is invalid", (param))
#define SDL3_zero(x) SDL3_memset(&(x), 0, sizeof((x)))
#define SDL3_zerop(x) SDL3_memset((x), 0, sizeof(*(x)))
#define SDL3_zeroa(x) SDL3_memset((x), 0, sizeof((x)))
#define SDL3_copyp(dst, src)                                                    \
    { SDL_COMPILE_TIME_ASSERT(SDL3_copyp, sizeof(*(dst)) == sizeof(*(src))); }  \
    SDL3_memcpy((dst), (src), sizeof(*(src)))
#define SDL_zerop   SDL3_zerop

/* for SDL_assert() : */
#define SDL_enabled_assert(condition) \
do { \
    while ( !(condition) ) { \
        static struct SDL_AssertData sdl_assert_data = { 0, 0, #condition, 0, 0, 0, 0 }; \
        const SDL_AssertState sdl_assert_state = SDL3_ReportAssertion(&sdl_assert_data, SDL_FUNCTION, SDL_FILE, SDL_LINE); \
        if (sdl_assert_state == SDL_ASSERTION_RETRY) { \
            continue; /* go again. */ \
        } else if (sdl_assert_state == SDL_ASSERTION_BREAK) { \
            SDL_AssertBreakpoint(); \
        } \
        break; /* not retrying. */ \
    } \
} while (SDL_NULL_WHILE_LOOP_CONDITION)

#ifndef SDL_DISABLE_ALLOCA
#define SDL3_stack_alloc(type, count)    (type*)alloca(sizeof(type)*(count))
#define SDL3_stack_free(data)
#else
#define SDL3_stack_alloc(type, count)    (type*)SDL3_malloc(sizeof(type)*(count))
#define SDL3_stack_free(data)            SDL3_free(data)
#endif

#ifdef _MSC_VER /* SDL_MAX_SMALL_ALLOC_STACKSIZE is smaller than _ALLOCA_S_THRESHOLD and should be generally safe */
#pragma warning(disable : 6255)
#endif
#define SDL_MAX_SMALL_ALLOC_STACKSIZE          128
#define SDL3_small_alloc(type, count, pisstack) ((*(pisstack) = ((sizeof(type) * (count)) < SDL_MAX_SMALL_ALLOC_STACKSIZE)), (*(pisstack) ? SDL3_stack_alloc(type, count) : (type *)SDL3_malloc(sizeof(type) * (count))))
#define SDL3_small_free(ptr, isstack) \
    if ((isstack)) {                 \
        SDL3_stack_free(ptr);         \
    } else {                         \
        SDL3_free(ptr);               \
    }

#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>

static bool WantDebugLogging = false;


static char *
SDL2COMPAT_stpcpy(char *dst, const char *src)
{
    while ((*dst++ = *src++) != '\0') {
        /**/;
    }
    return --dst;
}

static void
SDL2COMPAT_itoa(char *dst, int val)
{
    char *ptr, temp;

    if (val < 0) {
        *dst++ = '-';
        val = -val;
    }
    ptr = dst;

    do {
        *ptr++ = '0' + (val % 10);
        val /= 10;
    } while (val > 0);
    *ptr-- = '\0';

    /* correct the order of digits */
    do {
        temp = *dst;
        *dst++ = *ptr;
        *ptr-- = temp;
    } while (ptr > dst);
}


/* Obviously we can't use SDL_LoadObject() to load SDL3.  :)  */
/* FIXME: Updated library names after https://github.com/libsdl-org/SDL/issues/5626 solidifies.  */
static char loaderror[256];
#if defined(_WIN32)
    static HMODULE Loaded_SDL3 = NULL;
    #define DIRSEP "\\"
    #define SDL3_LIBNAME "SDL3.dll"
    #define LoadSDL3Library() ((Loaded_SDL3 = LoadLibraryA(SDL3_LIBNAME)) != NULL)
    #define LookupSDL3Sym(sym) (void *)GetProcAddress(Loaded_SDL3, sym)
    #define CloseSDL3Library() { if (Loaded_SDL3) { FreeLibrary(Loaded_SDL3); Loaded_SDL3 = NULL; } }
#elif defined(__APPLE__)
    #include <dlfcn.h>
    #include <pwd.h>
    #include <unistd.h>
    #define SDL3_LIBNAME "libSDL3.dylib"
    #define SDL3_FRAMEWORK "SDL3.framework/Versions/A/SDL3"
    static void *Loaded_SDL3 = NULL;
    #define LookupSDL3Sym(sym) dlsym(Loaded_SDL3, sym)
    #define CloseSDL3Library() { if (Loaded_SDL3) { dlclose(Loaded_SDL3); Loaded_SDL3 = NULL; } }
    static bool LoadSDL3Library(void) {
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
                return true;
            }
        }

        return false; /* didn't find it anywhere reasonable. :( */
    }
#elif defined(__unix__)
    #include <dlfcn.h>
    #define SDL3_LIBNAME "libSDL3.so.0"
    static void *Loaded_SDL3 = NULL;
    #define LoadSDL3Library() ((Loaded_SDL3 = dlopen(SDL3_LIBNAME, RTLD_LOCAL|RTLD_NOW)) != NULL)
    #define LookupSDL3Sym(sym) dlsym(Loaded_SDL3, sym)
    #define CloseSDL3Library() { if (Loaded_SDL3) { dlclose(Loaded_SDL3); Loaded_SDL3 = NULL; } }
#else
    #error Please define your platform.
#endif

#ifndef SDL3_REQUIRED_VER
#define SDL3_REQUIRED_VER SDL_VERSIONNUM(3,2,0)
#endif

#ifndef DIRSEP
#define DIRSEP "/"
#endif

/* init stuff we want to do after SDL3 is loaded but before the app has access to it. */
static int SDL2Compat_InitOnStartup(void);


static void *
LoadSDL3Symbol(const char *fn, bool *okay)
{
    void *retval = NULL;
    if (*okay) { /* only bother trying if we haven't previously failed. */
        retval = LookupSDL3Sym(fn);
        if (retval == NULL) {
            char *p = SDL2COMPAT_stpcpy(loaderror, fn);
            SDL2COMPAT_stpcpy(p, " missing in SDL3 library.");
            *okay = false;
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
    /*{"my_game_name", "SDL_RENDER_BATCHING", "0"},*/
    {"", "", "0"} /* A dummy entry to keep compilers happy. */
};

#ifdef __linux__
static void OS_GetExeName(char *buf, const unsigned maxpath) {
    int ret;
    buf[0] = '\0';
    ret = readlink("/proc/self/exe", buf, maxpath);
    (void)ret;
}
#elif defined(_WIN32)
static void OS_GetExeName(char *buf, const unsigned maxpath) {
    buf[0] = '\0';
    GetModuleFileNameA(NULL, buf, maxpath);
}
#elif defined(__APPLE__) || defined(SDL_PLATFORM_FREEBSD)
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

static bool
SDL2Compat_GetHintBoolean(const char *name, bool default_value)
{
    const char *val = SDL2Compat_GetHint(name);

    if (!val) {
        return default_value;
    }

    return (SDL3_atoi(val) != 0);
}


static struct {
    const char *old_hint;
    const char *new_hint;
} renamed_hints[] = {
    { "SDL_ALLOW_TOPMOST", "SDL_WINDOW_ALLOW_TOPMOST" },
    { "SDL_AUDIODRIVER", "SDL_AUDIO_DRIVER" },
    { "SDL_DIRECTINPUT_ENABLED", "SDL_JOYSTICK_DIRECTINPUT" },
    { "SDL_GDK_TEXTINPUT_DEFAULT", "SDL_GDK_TEXTINPUT_DEFAULT_TEXT" },
    { "SDL_JOYSTICK_GAMECUBE_RUMBLE_BRAKE", "SDL_JOYSTICK_HIDAPI_GAMECUBE_RUMBLE_BRAKE" },
    { "SDL_JOYSTICK_HIDAPI_PS4_RUMBLE", "SDL_JOYSTICK_ENHANCED_REPORTS" },
    { "SDL_JOYSTICK_HIDAPI_PS5_RUMBLE", "SDL_JOYSTICK_ENHANCED_REPORTS" },
    { "SDL_LINUX_DIGITAL_HATS", "SDL_JOYSTICK_LINUX_DIGITAL_HATS" },
    { "SDL_LINUX_HAT_DEADZONES", "SDL_JOYSTICK_LINUX_HAT_DEADZONES" },
    { "SDL_LINUX_JOYSTICK_CLASSIC", "SDL_JOYSTICK_LINUX_CLASSIC" },
    { "SDL_LINUX_JOYSTICK_DEADZONES", "SDL_JOYSTICK_LINUX_DEADZONES" },
    { "SDL_PS2_DYNAMIC_VSYNC", "SDL_RENDER_PS2_DYNAMIC_VSYNC" },
    { "SDL_VIDEODRIVER", "SDL_VIDEO_DRIVER" },
    { "SDL_VIDEO_WAYLAND_EMULATE_MOUSE_WARP", "SDL_MOUSE_EMULATE_WARP_WITH_RELATIVE" },
    { "SDL_VIDEO_WAYLAND_WMCLASS", "SDL_APP_ID" },
    { "SDL_VIDEO_X11_FORCE_EGL", "SDL_VIDEO_FORCE_EGL" },
    { "SDL_VIDEO_X11_WMCLASS", "SDL_APP_ID" },
    { "SDL_VIDEO_GL_DRIVER", "SDL_OPENGL_LIBRARY" },
    { "SDL_VIDEO_EGL_DRIVER", "SDL_EGL_LIBRARY" },
};

static const char *SDL2_to_SDL3_hint(const char *name)
{
    unsigned int i;

    for (i = 0; i < SDL_arraysize(renamed_hints); ++i) {
        if (SDL3_strcmp(name, renamed_hints[i].old_hint) == 0) {
            return renamed_hints[i].new_hint;
        }
    }
    return name;
}

static const char *SDL2_to_SDL3_hint_value(const char *name, const char *value, bool *free_value)
{
    if (name && value && SDL3_strcmp(name, SDL_HINT_LOGGING) == 0) {
        /* Rewrite numeric priorities for SDL3 */
        char *value3 = SDL_strdup(value);
        if (value3) {
            char *sep;
            for (sep = SDL3_strchr(value3, '='); sep; sep = SDL3_strchr(sep + 1, '=')) {
                if (SDL3_isdigit(sep[1]) && sep[1] != '0') {
                    sep[1] = '0' + SDL_atoi(&sep[1]) + 1;
                }
            }
        }
        *free_value = true;
        return value3;
    } else {
        *free_value = false;
        return value;
    }
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_SetHintWithPriority(const char *name, const char *value, SDL_HintPriority priority)
{
    bool free_value = false;
    const char *value3 = SDL2_to_SDL3_hint_value(name, value, &free_value);
    SDL2_bool result = SDL3_SetHintWithPriority(SDL2_to_SDL3_hint(name), value3, priority) ? SDL2_TRUE : SDL2_FALSE;
    if (free_value) {
        SDL3_free((void *)value3);
    }
    return result;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_SetHint(const char *name, const char *value)
{
    return SDL_SetHintWithPriority(name, value, SDL_HINT_NORMAL);
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetHint(const char *name)
{
    return SDL3_GetHint(SDL2_to_SDL3_hint(name));
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_ResetHint(const char *name)
{
    return SDL3_ResetHint(SDL2_to_SDL3_hint(name)) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GetHintBoolean(const char *name, SDL2_bool default_value)
{
    return SDL3_GetHintBoolean(SDL2_to_SDL3_hint(name), default_value) ? SDL2_TRUE : SDL2_FALSE;
}

/* FIXME: callbacks may need tweaking ... */
SDL_DECLSPEC void SDLCALL
SDL_AddHintCallback(const char *name, SDL_HintCallback callback, void *userdata)
{
    /* this returns an int of 0 or -1 in SDL3, but SDL2 it was void (even if it failed). */
    (void) SDL3_AddHintCallback(SDL2_to_SDL3_hint(name), callback, userdata);
}

SDL_DECLSPEC void SDLCALL
SDL_DelHintCallback(const char *name, SDL_HintCallback callback, void *userdata)
{
    SDL3_RemoveHintCallback(SDL2_to_SDL3_hint(name), callback, userdata);
}

SDL_DECLSPEC void SDLCALL
SDL_ClearHints(void)
{
    SDL3_ResetHints();
}

static void
SDL2Compat_ApplyQuirks(bool force_x11)
{
    const char *exe_name = SDL2Compat_GetExeName();
    const char *old_env;
    unsigned int i;

    if (WantDebugLogging) {
        SDL3_Log("This app appears to be named '%s'", exe_name);
    }

    /* if you change this, update also SDL2_to_SDL3_hint() */
    for (i = 0; i < SDL_arraysize(renamed_hints); ++i) {
        old_env = SDL3_getenv(renamed_hints[i].old_hint);
        if (old_env) {
            SDL3_setenv_unsafe(renamed_hints[i].new_hint, old_env, 1);
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
            SDL3_setenv_unsafe("SDL_VIDEO_DRIVER", "x11", 1);
        }
    }
    #endif

    if (*exe_name == '\0') {
        return;
    }
    for (i = 0; i < SDL_arraysize(quirks); i++) {
        if (!SDL3_strcmp(exe_name, quirks[i].exe_name)) {
            const char *var = SDL3_getenv(quirks[i].hint_name);
            if (!var) {
                if (WantDebugLogging) {
                    SDL3_Log("Applying compatibility quirk %s=\"%s\" for \"%s\"", quirks[i].hint_name, quirks[i].hint_value, exe_name);
                }
                SDL3_setenv_unsafe(quirks[i].hint_name, quirks[i].hint_value, 1);
            } else {
                if (WantDebugLogging) {
                    SDL3_Log("Not applying compatibility quirk %s=\"%s\" for \"%s\" due to environment variable override (\"%s\")\n",
                            quirks[i].hint_name, quirks[i].hint_value, exe_name, var);
                }
            }
        }
    }
}

static int
LoadSDL3(void)
{
    bool okay = true;
    if (!Loaded_SDL3) {
        bool force_x11 = false;

        #ifdef __linux__
        void *global_symbols = dlopen(NULL, RTLD_LOCAL|RTLD_NOW);

        /* Use linked libraries to detect what quirks we are likely to need */
        if (global_symbols != NULL) {
            if (dlsym(global_symbols, "glxewInit") != NULL) {  /* GLEW (e.g. Frogatto, SLUDGE) */
                force_x11 = true;
            } else if (dlsym(global_symbols, "cgGLEnableProgramProfiles") != NULL) {  /* NVIDIA Cg (e.g. Awesomenauts, Braid) */
                force_x11 = true;
            } else if (dlsym(global_symbols, "_Z7ssgInitv") != NULL) {  /* ::ssgInit(void) in plib (e.g. crrcsim) */
                force_x11 = true;
            }
            dlclose(global_symbols);
        }
        #endif

        okay = LoadSDL3Library();
        if (!okay) {
            SDL2COMPAT_stpcpy(loaderror, "Failed loading SDL3 library.");
        } else {
            #define SDL3_SYM(rc,fn,params,args,ret) SDL3_##fn = (SDL3_##fn##_t) LoadSDL3Symbol("SDL_" #fn, &okay);
            #include "sdl3_syms.h"
            if (okay) {
                const int sdl3version = SDL3_GetVersion();
                const int sdl3major = SDL_VERSIONNUM_MAJOR(sdl3version);
                const int sdl3minor = SDL_VERSIONNUM_MINOR(sdl3version);
                const int sdl3micro = SDL_VERSIONNUM_MICRO(sdl3version);

                okay = (sdl3version >= SDL3_REQUIRED_VER);
                if (!okay) {
                    char value[12];
                    char *p = SDL2COMPAT_stpcpy(loaderror, "SDL3 ");

                    SDL2COMPAT_itoa(value, sdl3major);
                    p = SDL2COMPAT_stpcpy(p, value); *p++ = '.';
                    SDL2COMPAT_itoa(value, sdl3minor);
                    p = SDL2COMPAT_stpcpy(p, value); *p++ = '.';
                    SDL2COMPAT_itoa(value, sdl3micro);
                    p = SDL2COMPAT_stpcpy(p, value);

                    SDL2COMPAT_stpcpy(p, " library is too old.");
                } else {
                    WantDebugLogging = SDL2Compat_GetHintBoolean("SDL2COMPAT_DEBUG_LOGGING", false);
                    if (WantDebugLogging) {
                        #if defined(__DATE__) && defined(__TIME__)
                        SDL3_Log("sdl2-compat 2.%d.%d, built on " __DATE__ " at " __TIME__ ", talking to SDL3 %d.%d.%d",
                                 SDL2_COMPAT_VERSION_MINOR, SDL2_COMPAT_VERSION_PATCH, sdl3major, sdl3minor, sdl3micro);
                        #else
                        SDL3_Log("sdl2-compat 2.%d.%d, talking to SDL3 %d.%d.%d",
                                 SDL2_COMPAT_VERSION_MINOR, SDL2_COMPAT_VERSION_PATCH, sdl3major, sdl3minor, sdl3micro);
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

#if defined(_MSC_VER)

/* NOLINTNEXTLINE(readability-redundant-declaration) */
extern void *memcpy(void *dst, const void *src, size_t len);
/* NOLINTNEXTLINE(readability-redundant-declaration) */
extern void *memset(void *dst, int c, size_t len);
#ifndef __INTEL_LLVM_COMPILER
#pragma intrinsic(memcpy)
#pragma intrinsic(memset)
#endif
#pragma function(memcpy)
#pragma function(memset)

/* NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) */
void *memcpy(void *dst, const void *src, size_t len)
{
    return SDL3_memcpy(dst, src, len);
}

/* NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name) */
void *memset(void *dst, int c, size_t len)
{
    return SDL3_memset(dst, c, len);
}
#endif  /* MSVC */

#if defined(__ICL) && defined(_WIN32)
/* The classic Intel compiler generates calls to _intel_fast_memcpy
 * and _intel_fast_memset when building an optimized SDL library */
void *_intel_fast_memcpy(void *dst, const void *src, size_t len)
{
    return SDL3_memcpy(dst, src, len);
}

void *_intel_fast_memset(void *dst, int c, size_t len)
{
    return SDL3_memset(dst, c, len);
}
#endif

#if defined(_WIN32)
static void error_dialog(const char *errorMsg)
{
    MessageBoxA(NULL, errorMsg, "Error", MB_OK | MB_SETFOREGROUND | MB_ICONSTOP);
}
#elif defined(__APPLE__)
extern void error_dialog(const char *errorMsg);
#elif defined(__ANDROID__)
static void error_dialog(const char *errorMsg)
{
    __android_log_print(ANDROID_LOG_FATAL, "SDL2COMPAT", "%s\n", errorMsg);
}
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


/* Some SDL2 state we need to keep... */

/* !!! FIXME: unify coding convention on the globals: some are MyVariableName and some are my_variable_name */
static SDL2_EventFilter EventFilter2 = NULL;
static void *EventFilterUserData2 = NULL;
static SDL_mutex *EventWatchListMutex = NULL;
static EventFilterWrapperData *EventWatchers2 = NULL;
static SDL2_bool relative_mouse_mode = SDL2_FALSE;
static SDL_JoystickID *joystick_list = NULL;
static int num_joysticks = 0;
static SDL_JoystickID *gamepad_button_swap_list = NULL;
static int num_gamepad_button_swap_list = 0;
static SDL_SensorID *sensor_list = NULL;
static int num_sensors = 0;
static SDL_HapticID *haptic_list = NULL;
static int num_haptics = 0;

static SDL_mutex *joystick_lock = NULL;
static SDL_mutex *sensor_lock = NULL;

typedef struct AudioDeviceInfo
{
    SDL_AudioDeviceID devid;
    char *name;
} AudioDeviceInfo;

typedef struct AudioDeviceList
{
    AudioDeviceInfo *devices;
    int num_devices;
} AudioDeviceList;

static SDL_Mutex *AudioDeviceLock = NULL;
static SDL2_AudioStream *AudioOpenDevices[16];  /* SDL2 had a limit of 16 simultaneous devices opens (and the first slot was for the 1.2 legacy interface). We track these as _SDL2_ audio streams. */
static AudioDeviceList AudioSDL3PlaybackDevices;
static AudioDeviceList AudioSDL3RecordingDevices;

static char **GamepadMappings = NULL;
static int NumGamepadMappings = 0;

static SDL_TouchID *TouchDevices = NULL;
static int NumTouchDevices = 0;
static SDL_TouchID TouchFingersDeviceID = 0;
static SDL_Finger **TouchFingers = NULL;
static int NumTouchFingers = 0;

static SDL_PropertiesID timers = 0;

/* Functions! */

static void
SDL2Compat_InitLogPrefixes(void)
{
    SDL3_SetLogPriorityPrefix(SDL_LOG_PRIORITY_VERBOSE, "VERBOSE: ");
    SDL3_SetLogPriorityPrefix(SDL_LOG_PRIORITY_DEBUG, "DEBUG: ");
    SDL3_SetLogPriorityPrefix(SDL_LOG_PRIORITY_INFO, "INFO: ");
    SDL3_SetLogPriorityPrefix(SDL_LOG_PRIORITY_WARN, "WARN: ");
    SDL3_SetLogPriorityPrefix(SDL_LOG_PRIORITY_ERROR, "ERROR: ");
    SDL3_SetLogPriorityPrefix(SDL_LOG_PRIORITY_CRITICAL, "CRITICAL: ");
}

/* this stuff _might_ move to SDL_Init later */
static int
SDL2Compat_InitOnStartup(void)
{
    EventWatchListMutex = SDL3_CreateMutex();
    if (!EventWatchListMutex) {
        goto fail;
    }

    sensor_lock = SDL3_CreateMutex();
    if (sensor_lock == NULL) {
        goto fail;
    }

    joystick_lock = SDL3_CreateMutex();
    if (joystick_lock == NULL) {
        goto fail;
    }

    AudioDeviceLock = SDL3_CreateMutex();
    if (AudioDeviceLock == NULL) {
        goto fail;
    }

    SDL3_SetHint("SDL_WINDOWS_DPI_AWARENESS", "unaware");
    SDL3_SetHint("SDL_BORDERLESS_WINDOWED_STYLE", "0");
    SDL3_SetHint("SDL_VIDEO_SYNC_WINDOW_OPERATIONS", "1");

    SDL2Compat_InitLogPrefixes();

    return 1;

fail:
    SDL2COMPAT_stpcpy(loaderror, "Failed to initialize sdl2-compat library.");

    if (EventWatchListMutex) {
        SDL3_DestroyMutex(EventWatchListMutex);
    }
    if (sensor_lock) {
        SDL3_DestroyMutex(sensor_lock);
    }
    if (joystick_lock) {
        SDL3_DestroyMutex(joystick_lock);
    }
    if (AudioDeviceLock) {
        SDL3_DestroyMutex(AudioDeviceLock);
    }
    return 0;
}


/* obviously we have to override this so we don't report ourselves as SDL3. */
SDL_DECLSPEC void SDLCALL
SDL_GetVersion(SDL2_version *ver)
{
    if (ver) {
        ver->major = 2;
        ver->minor = SDL2_COMPAT_VERSION_MINOR;
        ver->patch = SDL2_COMPAT_VERSION_PATCH;
        if (SDL3_GetHintBoolean("SDL_LEGACY_VERSION", false)) {
            /* Prior to SDL 2.24.0, the patch version was incremented with every release */
            ver->patch = ver->minor;
            ver->minor = 0;
        }
    }
}

SDL_DECLSPEC int SDLCALL
SDL_GetRevisionNumber(void)
{
    /* After the move to GitHub, this always returned zero, since this was a
       Mercurial thing. We removed it outright in SDL3. */
    return 0;
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetRevision(void)
{
    return SDL2COMPAT_REVISION;
}

SDL_DECLSPEC char * SDLCALL
SDL_GetErrorMsg(char *errstr, int maxlen)
{
    SDL3_strlcpy(errstr, SDL3_GetError(), maxlen);
    return errstr;
}

SDL_DECLSPEC int SDLCALL
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
    if (str) {
        va_start(ap, fmt);
        SDL3_vsnprintf(str, len + 1, fmt, ap);
        va_end(ap);
        SDL3_SetError("%s", str);
        SDL3_free(str);
    }
    return -1;
}

SDL_DECLSPEC void SDLCALL
SDL_ClearError(void)
{
    SDL3_ClearError();
}

SDL_DECLSPEC int SDLCALL
SDL_Error(SDL_errorcode code)
{
    switch (code) {
    case SDL_ENOMEM:
        SDL3_OutOfMemory();
        break;
    case SDL_EFREAD:
        SDL3_SetError("Error reading from datastream");
        break;
    case SDL_EFWRITE:
        SDL3_SetError("Error writing to datastream");
        break;
    case SDL_EFSEEK:
        SDL3_SetError("Error seeking in datastream");
        break;
    case SDL_UNSUPPORTED:
        SDL3_Unsupported();
        break;
    default:
        SDL3_SetError("Unknown SDL error");
        break;
    }
    return -1;
}

SDL_DECLSPEC char *SDLCALL
SDL_lltoa(Sint64 value, char *str, int radix)
{
    return SDL3_lltoa((long long)value, str, radix);
}

SDL_DECLSPEC char *
SDLCALL SDL_ulltoa(Uint64 value, char *str, int radix)
{
    return SDL3_ulltoa((unsigned long long)value, str, radix);
}

SDL_DECLSPEC Sint64 SDLCALL
SDL_strtoll(const char *str, char **endp, int base)
{
    return (Sint64)SDL3_strtoll(str, endp, base);
}

SDL_DECLSPEC Uint64 SDLCALL
SDL_strtoull(const char *str, char **endp, int base)
{
    return (Uint64)SDL3_strtoull(str, endp, base);
}

SDL_DECLSPEC int SDLCALL
SDL_sscanf(const char *text, const char *fmt, ...)
{
    int retval;
    va_list ap;
    va_start(ap, fmt);
    retval = (int) SDL3_vsscanf(text, fmt, ap);
    va_end(ap);
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_snprintf(char *text, size_t maxlen, const char *fmt, ...)
{
    int retval;
    va_list ap;
    va_start(ap, fmt);
    retval = (int) SDL3_vsnprintf(text, maxlen, fmt, ap);
    va_end(ap);
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_asprintf(char **str, SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    int retval;
    va_list ap;
    va_start(ap, fmt);
    retval = (int) SDL3_vasprintf(str, fmt, ap);
    va_end(ap);
    return retval;
}

static SDL2_LogPriority LogPriority3to2(SDL_LogPriority priority)
{
    switch (priority) {
    case SDL_LOG_PRIORITY_VERBOSE:
        return SDL2_LOG_PRIORITY_VERBOSE;
    case SDL_LOG_PRIORITY_DEBUG:
        return SDL2_LOG_PRIORITY_DEBUG;
    case SDL_LOG_PRIORITY_INFO:
        return SDL2_LOG_PRIORITY_INFO;
    case SDL_LOG_PRIORITY_WARN:
        return SDL2_LOG_PRIORITY_WARN;
    case SDL_LOG_PRIORITY_ERROR:
        return SDL2_LOG_PRIORITY_ERROR;
    case SDL_LOG_PRIORITY_CRITICAL:
        return SDL2_LOG_PRIORITY_CRITICAL;
    default:
        return SDL2_NUM_LOG_PRIORITIES;
    }
}

static SDL_LogPriority LogPriority2to3(SDL2_LogPriority priority)
{
    switch (priority) {
    case SDL2_LOG_PRIORITY_VERBOSE:
        return SDL_LOG_PRIORITY_VERBOSE;
    case SDL2_LOG_PRIORITY_DEBUG:
        return SDL_LOG_PRIORITY_DEBUG;
    case SDL2_LOG_PRIORITY_INFO:
        return SDL_LOG_PRIORITY_INFO;
    case SDL2_LOG_PRIORITY_WARN:
        return SDL_LOG_PRIORITY_WARN;
    case SDL2_LOG_PRIORITY_ERROR:
        return SDL_LOG_PRIORITY_ERROR;
    case SDL2_LOG_PRIORITY_CRITICAL:
        return SDL_LOG_PRIORITY_CRITICAL;
    default:
        return SDL_LOG_PRIORITY_INVALID;
    }
}

SDL_DECLSPEC void SDLCALL SDL_LogSetAllPriority(SDL2_LogPriority priority)
{
    SDL3_SetLogPriorities(LogPriority2to3(priority));
}

SDL_DECLSPEC void SDLCALL SDL_LogSetPriority(int category, SDL2_LogPriority priority)
{
    SDL3_SetLogPriority(category, LogPriority2to3(priority));
}

SDL_DECLSPEC SDL2_LogPriority SDLCALL SDL_LogGetPriority(int category)
{
    return LogPriority3to2(SDL3_GetLogPriority(category));
}

SDL_DECLSPEC void SDLCALL
SDL_Log(SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    SDL3_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, fmt, ap);
    va_end(ap);
}

SDL_DECLSPEC void SDLCALL
SDL_LogMessage(int category, SDL2_LogPriority priority, SDL_PRINTF_FORMAT_STRING const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    SDL3_LogMessageV(category, LogPriority2to3(priority), fmt, ap);
    va_end(ap);
}

SDL_DECLSPEC void SDLCALL SDL_LogMessageV(int category, SDL2_LogPriority priority, const char *fmt, va_list ap)
{
    SDL3_LogMessageV(category, LogPriority2to3(priority), fmt, ap);
}

#define SDL_LOG_IMPL(name, prio)                                             \
SDL_DECLSPEC void SDLCALL                                                        \
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

static void UpdateGamepadButtonSwap(SDL_Gamepad *gamepad)
{
    int i;
    SDL_JoystickID instance_id = SDL3_GetGamepadID(gamepad);
    bool swap_buttons = false;

    if (SDL3_GetHintBoolean("SDL_GAMECONTROLLER_USE_BUTTON_LABELS", true)) {
        if (SDL3_GetGamepadButtonLabel(gamepad, SDL_GAMEPAD_BUTTON_SOUTH) == SDL_GAMEPAD_BUTTON_LABEL_B) {
            swap_buttons = true;
        }
    }

    if (swap_buttons) {
        bool has_gamepad = false;

        for (i = 0; i < num_gamepad_button_swap_list; ++i) {
            if (gamepad_button_swap_list[i] == instance_id) {
                has_gamepad = true;
                break;
            }
        }

        if (!has_gamepad) {
            int new_count = num_gamepad_button_swap_list + 1;
            SDL_JoystickID *new_list = (SDL_JoystickID *)SDL3_realloc(gamepad_button_swap_list, new_count * sizeof(*new_list));
            if (new_list) {
                new_list[num_gamepad_button_swap_list] = instance_id;
                gamepad_button_swap_list = new_list;
                ++num_gamepad_button_swap_list;
            }
        }
    } else {
        for (i = 0; i < num_gamepad_button_swap_list; ++i) {
            if (gamepad_button_swap_list[i] == instance_id) {
                if (i < (num_gamepad_button_swap_list - 1)) {
                    SDL3_memmove(&gamepad_button_swap_list[i], &gamepad_button_swap_list[i+1], (num_gamepad_button_swap_list - i - 1) * sizeof(gamepad_button_swap_list[i]));
                }
                --num_gamepad_button_swap_list;
                break;
            }
        }
    }
}

static bool ShouldSwapGamepadButtons(SDL_JoystickID instance_id)
{
    int i;

    for (i = 0; i < num_gamepad_button_swap_list; ++i) {
        if (gamepad_button_swap_list[i] == instance_id) {
            return true;
        }
    }
    return false;
}

static Uint8 SwapGamepadButton(Uint8 button)
{
    switch (button) {
    case SDL_GAMEPAD_BUTTON_SOUTH:
        return SDL_GAMEPAD_BUTTON_EAST;
    case SDL_GAMEPAD_BUTTON_EAST:
        return SDL_GAMEPAD_BUTTON_SOUTH;
    case SDL_GAMEPAD_BUTTON_WEST:
        return SDL_GAMEPAD_BUTTON_NORTH;
    case SDL_GAMEPAD_BUTTON_NORTH:
        return SDL_GAMEPAD_BUTTON_WEST;
    default:
        return button;
    }
}

static SDL_Scancode SDL2ScancodeToSDL3Scancode(SDL2_Scancode scancode)
{
    if (scancode <= SDL2_SCANCODE_MODE) {
        // They're the same
        return (SDL_Scancode)scancode;
    }

    switch (scancode) {
    case SDL2_SCANCODE_AUDIOFASTFORWARD:
        return SDL_SCANCODE_MEDIA_FAST_FORWARD;
    case SDL2_SCANCODE_AUDIOMUTE:
        return SDL_SCANCODE_MUTE;
    case SDL2_SCANCODE_AUDIONEXT:
        return SDL_SCANCODE_MEDIA_NEXT_TRACK;
    case SDL2_SCANCODE_AUDIOPLAY:
        return SDL_SCANCODE_MEDIA_PLAY;
    case SDL2_SCANCODE_AUDIOPREV:
        return SDL_SCANCODE_MEDIA_PREVIOUS_TRACK;
    case SDL2_SCANCODE_AUDIOREWIND:
        return SDL_SCANCODE_MEDIA_REWIND;
    case SDL2_SCANCODE_AUDIOSTOP:
        return SDL_SCANCODE_MEDIA_STOP;
    case SDL2_SCANCODE_EJECT:
        return SDL_SCANCODE_MEDIA_EJECT;
    case SDL2_SCANCODE_MEDIASELECT:
        return SDL_SCANCODE_MEDIA_SELECT;
    default:
        return SDL_SCANCODE_UNKNOWN;
    }
}

static SDL2_Scancode SDL3ScancodeToSDL2Scancode(SDL_Scancode scancode)
{
    if (scancode <= SDL_SCANCODE_MODE) {
        // They're the same
        return (SDL2_Scancode)scancode;
    }

    switch (scancode) {
    case SDL_SCANCODE_MEDIA_FAST_FORWARD:
        return SDL2_SCANCODE_AUDIOFASTFORWARD;
    case SDL_SCANCODE_MUTE:
        return SDL2_SCANCODE_AUDIOMUTE;
    case SDL_SCANCODE_MEDIA_NEXT_TRACK:
        return SDL2_SCANCODE_AUDIONEXT;
    case SDL_SCANCODE_MEDIA_PLAY:
        return SDL2_SCANCODE_AUDIOPLAY;
    case SDL_SCANCODE_MEDIA_PREVIOUS_TRACK:
        return SDL2_SCANCODE_AUDIOPREV;
    case SDL_SCANCODE_MEDIA_REWIND:
        return SDL2_SCANCODE_AUDIOREWIND;
    case SDL_SCANCODE_MEDIA_STOP:
        return SDL2_SCANCODE_AUDIOSTOP;
    case SDL_SCANCODE_MEDIA_EJECT:
        return SDL2_SCANCODE_EJECT;
    case SDL_SCANCODE_MEDIA_SELECT:
        return SDL2_SCANCODE_MEDIASELECT;
    default:
        return SDL2_SCANCODE_UNKNOWN;
    }
}

static SDL_Keycode SDL3KeycodeToSDL2Keycode(SDL_Scancode scancode, SDL_Keycode keycode)
{
    /* Keys without the extended mask are passed through. */
    if (!(keycode & SDLK_EXTENDED_MASK)) {
        return keycode;
    }

    /* SDLK_TAB is an ASCII value and can't be converted directly from a scancode. */
    if (keycode == SDLK_LEFT_TAB) {
        return SDLK_TAB;
    }

    /* Convert the scancode directly to the keycode. This matches the mapping behavior
     * of SDL2 backends unless some very esoteric key remapping is being used.
     */
    return SDL_SCANCODE_TO_KEYCODE(scancode);
}

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

static int GetIndexFromJoystickInstance(SDL_JoystickID jid);

static SDL2_Event *
Event3to2(const SDL_Event *event3, SDL2_Event *event2)
{
    SDL_Renderer *renderer;
    SDL_Event cvtevent3;

#if 0
    if (event3->type == SDL_SYSWMEVENT) {
        return SDL2_FALSE;  /* !!! FIXME: figure out what to do with this. */
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
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        event2->key.keysym.scancode = SDL3ScancodeToSDL2Scancode(event3->key.scancode);
        event2->key.keysym.sym = SDL3KeycodeToSDL2Keycode(event3->key.scancode, event3->key.key);
        event2->key.keysym.mod = event3->key.mod;
        event2->key.state = event3->key.down;
        event2->key.repeat = event3->key.repeat;
        break;
    case SDL_EVENT_TEXT_INPUT:
        SDL3_strlcpy(event2->text.text, event3->text.text, sizeof(event2->text.text));
        break;
    case SDL_EVENT_TEXT_EDITING:
        if (SDL3_GetHintBoolean("SDL_IME_SUPPORT_EXTENDED_TEXT", false) &&
            SDL3_strlen(event3->edit.text) >= sizeof(event2->edit.text)) {
            /* From events/SDL_keyboard.c::SDL_SendEditingText() of SDL2 */
            event2->editExt.type = SDL2_TEXTEDITING_EXT;
            event2->editExt.windowID = event3->edit.windowID;
            event2->editExt.text = SDL3_strdup(event3->edit.text);
            event2->editExt.start = event3->edit.start;
            event2->editExt.length = event3->edit.length;
        } else {
            SDL3_strlcpy(event2->edit.text, event3->edit.text, sizeof(event2->edit.text));
            event2->edit.start = event3->edit.start;
            event2->edit.length = event3->edit.length;
        }
        break;
    case SDL_EVENT_DROP_FILE:
    case SDL_EVENT_DROP_TEXT:
        event2->drop.file = SDL3_strdup(event3->drop.data);
        SDL_FALLTHROUGH;
    case SDL_EVENT_DROP_BEGIN:
    case SDL_EVENT_DROP_COMPLETE:
        event2->drop.windowID = event3->drop.windowID;
        break;
    case SDL_EVENT_MOUSE_MOTION:
        renderer = SDL3_GetRenderer(SDL3_GetWindowFromID(event3->motion.windowID));
        if (renderer) {
            SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
            SDL3_GetRenderLogicalPresentation(renderer, NULL, NULL, &mode);
            if (mode != SDL_LOGICAL_PRESENTATION_DISABLED) {
                SDL3_memcpy(&cvtevent3, event3, sizeof (SDL_Event));
                SDL3_ConvertEventToRenderCoordinates(renderer, &cvtevent3);
                event3 = &cvtevent3;
            }
        }
        event2->motion.x = (Sint32)event3->motion.x;
        event2->motion.y = (Sint32)event3->motion.y;
        event2->motion.xrel = (Sint32)event3->motion.xrel;
        event2->motion.yrel = (Sint32)event3->motion.yrel;
        break;
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        renderer = SDL3_GetRenderer(SDL3_GetWindowFromID(event3->button.windowID));
        if (renderer) {
            SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
            SDL3_GetRenderLogicalPresentation(renderer, NULL, NULL, &mode);
            if (mode != SDL_LOGICAL_PRESENTATION_DISABLED) {
                SDL3_memcpy(&cvtevent3, event3, sizeof (SDL_Event));
                SDL3_ConvertEventToRenderCoordinates(renderer, &cvtevent3);
                event3 = &cvtevent3;
            }
        }
        event2->button.x = (Sint32)event3->button.x;
        event2->button.y = (Sint32)event3->button.y;
        break;
    case SDL_EVENT_MOUSE_WHEEL:
        renderer = SDL3_GetRenderer(SDL3_GetWindowFromID(event3->wheel.windowID));
        if (renderer) {
            SDL_RendererLogicalPresentation mode = SDL_LOGICAL_PRESENTATION_DISABLED;
            SDL3_GetRenderLogicalPresentation(renderer, NULL, NULL, &mode);
            if (mode != SDL_LOGICAL_PRESENTATION_DISABLED) {
                SDL3_memcpy(&cvtevent3, event3, sizeof (SDL_Event));
                SDL3_ConvertEventToRenderCoordinates(renderer, &cvtevent3);
                event3 = &cvtevent3;
            }
        }
        event2->wheel.x = (Sint32)event3->wheel.x;
        event2->wheel.y = (Sint32)event3->wheel.y;
        event2->wheel.preciseX = event3->wheel.x;
        event2->wheel.preciseY = event3->wheel.y;
        event2->wheel.mouseX = (Sint32)event3->wheel.mouse_x;
        event2->wheel.mouseY = (Sint32)event3->wheel.mouse_y;
        break;
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
        if (ShouldSwapGamepadButtons(event2->cbutton.which)) {
            event2->cbutton.button = SwapGamepadButton(event2->cbutton.button);
        }
        break;
    /* sensor timestamps are in nanosecond in SDL3 */
    case SDL_EVENT_GAMEPAD_SENSOR_UPDATE:
        event2->csensor.timestamp_us = SDL_NS_TO_US(event3->gsensor.sensor_timestamp);
        break;
    case SDL_EVENT_SENSOR_UPDATE:
        event2->sensor.timestamp_us = SDL_NS_TO_US(event3->sensor.sensor_timestamp);
        break;
    /* Change SDL3 InstanceID to index */
    case SDL_EVENT_JOYSTICK_ADDED:
    case SDL_EVENT_GAMEPAD_ADDED:
        event2->jaxis.which = GetIndexFromJoystickInstance(event3->jaxis.which);
        break;
    case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
        switch (event3->jbattery.state) {
        case SDL_POWERSTATE_CHARGING:
        case SDL_POWERSTATE_CHARGED:
            event2->jbattery.level = SDL_JOYSTICK_POWER_WIRED;
            break;
        case SDL_POWERSTATE_ON_BATTERY:
            if (event3->jbattery.percent > 70) {
                event2->jbattery.level = SDL_JOYSTICK_POWER_FULL;
            } else if (event3->jbattery.percent > 20) {
                event2->jbattery.level = SDL_JOYSTICK_POWER_MEDIUM;
            } else if (event3->jbattery.percent > 5) {
                event2->jbattery.level = SDL_JOYSTICK_POWER_LOW;
            } else {
                event2->jbattery.level = SDL_JOYSTICK_POWER_EMPTY;
            }
            break;
        default:
            event2->jbattery.level = SDL_JOYSTICK_POWER_UNKNOWN;
            break;
        }
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
        return SDL2_FALSE;  /* !!! FIXME: figure out what to do with this. */
    }
#endif

    /* currently everything _mostly_ matches up between SDL2 and SDL3, but this might
       drift more as SDL3 development continues. */

    /* for now, the timestamp field has grown in size (and precision), everything after it is currently the same, minus padding at the end, so bump the fields down. */
    event3->common.type = event2->type;
    event3->common.timestamp = (Uint64) SDL_MS_TO_NS(event2->common.timestamp);
    SDL3_memcpy((&event3->common) + 1, (&event2->common) + 1, sizeof (SDL2_Event) - sizeof (SDL2_CommonEvent));
    /* mouse coords became floats in SDL3: */
    switch (event2->type) {
    case SDL_EVENT_TEXT_INPUT:
        /* We shouldn't be getting text input events this direction */
        return NULL;
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        event3->key.which = 0;
        event3->key.scancode = SDL2ScancodeToSDL3Scancode(event2->key.keysym.scancode);
        event3->key.key = event2->key.keysym.sym;
        event3->key.mod = event2->key.keysym.mod;
        event3->key.raw = 0;
        event3->key.down = (event2->key.state != 0);
        event3->key.repeat = event2->key.repeat;
        break;
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
        event3->wheel.mouse_x = (float)event2->wheel.mouseX;
        event3->wheel.mouse_y = (float)event2->wheel.mouseY;
        break;
    case SDL_EVENT_JOYSTICK_BATTERY_UPDATED:
        /* This should never happen, but see Event3to2() for details */
        break;
    default:
        break;
    }
    return event3;
}

static void GestureProcessEvent(const SDL_Event *event3);

SDL_DECLSPEC int SDLCALL
SDL_PushEvent(SDL2_Event *event2)
{
    SDL_Event event3;
    if (SDL3_PushEvent(Event2to3(event2, &event3))) {
        return 1;
    } else if (*SDL_GetError() == '\0') {
        return 0;
    } else {
        return -1;
    }
}

static int Display_IDToIndex(SDL_DisplayID displayID);

static SDL2_WindowEventID
WindowEventType3To2(Uint32 event_type3)
{
    switch (event_type3) {
    case SDL_EVENT_WINDOW_SHOWN:
        return SDL_WINDOWEVENT_SHOWN;
    case SDL_EVENT_WINDOW_HIDDEN:
        return SDL_WINDOWEVENT_HIDDEN;
    case SDL_EVENT_WINDOW_EXPOSED:
        return SDL_WINDOWEVENT_EXPOSED;
    case SDL_EVENT_WINDOW_MOVED:
        return SDL_WINDOWEVENT_MOVED;
    case SDL_EVENT_WINDOW_RESIZED:
        return SDL_WINDOWEVENT_RESIZED;
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
        return SDL_WINDOWEVENT_SIZE_CHANGED;
    case SDL_EVENT_WINDOW_MINIMIZED:
        return SDL_WINDOWEVENT_MINIMIZED;
    case SDL_EVENT_WINDOW_MAXIMIZED:
        return SDL_WINDOWEVENT_MAXIMIZED;
    case SDL_EVENT_WINDOW_RESTORED:
        return SDL_WINDOWEVENT_RESTORED;
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
        return SDL_WINDOWEVENT_ENTER;
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        return SDL_WINDOWEVENT_LEAVE;
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
        return SDL_WINDOWEVENT_FOCUS_GAINED;
    case SDL_EVENT_WINDOW_FOCUS_LOST:
        return SDL_WINDOWEVENT_FOCUS_LOST;
    case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        return SDL_WINDOWEVENT_CLOSE;
    case SDL_EVENT_WINDOW_HIT_TEST:
        return SDL_WINDOWEVENT_HIT_TEST;
    case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
        return SDL_WINDOWEVENT_ICCPROF_CHANGED;
    case SDL_WINDOWEVENT_DISPLAY_CHANGED:
        return SDL_WINDOWEVENT_DISPLAY_CHANGED;
    default:
        return SDL_WINDOWEVENT_NONE;
    }
}

typedef struct RemovePendingSizeChangedAndResizedEvents_Data
{
    const SDL2_Event *new_event;
    bool saw_resized;
} RemovePendingSizeChangedAndResizedEvents_Data;

static int SDLCALL
RemovePendingSizeChangedAndResizedEvents(void *userdata, SDL2_Event *event)
{
    RemovePendingSizeChangedAndResizedEvents_Data *resizedata = (RemovePendingSizeChangedAndResizedEvents_Data *)userdata;
    const SDL2_Event *new_event = resizedata->new_event;

    if (event->type == SDL2_WINDOWEVENT &&
        (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
         event->window.event == SDL_WINDOWEVENT_RESIZED) &&
        event->window.windowID == new_event->window.windowID) {

        if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
            resizedata->saw_resized = true;
        }

        /* We're about to post a new size event, drop the old one */
        return 0;
    }
    return 1;
}

static int SDLCALL
RemoveSupercededWindowEvents(void *userdata, SDL2_Event *event)
{
    SDL2_Event *new_event = (SDL2_Event *)userdata;

    if (event->type == SDL2_WINDOWEVENT &&
        event->window.event == new_event->window.event &&
        event->window.windowID == new_event->window.windowID) {
        /* We're about to post a new move event, drop the old one */
        return 0;
    }
    return 1;
}

static bool SDLCALL
EventFilter3to2(void *userdata, SDL_Event *event3)
{
    SDL2_Event event2;  /* note that event filters do not receive events as const! So we have to convert or copy it for each one! */
    bool post_event = true;

    GestureProcessEvent(event3);  /* this might need to generate new gesture events from touch input. */

    /* Ensure joystick and haptic IDs are updated before calling Event3to2() */
    switch (event3->type) {
        case SDL_EVENT_JOYSTICK_ADDED:
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
        case SDL_EVENT_JOYSTICK_REMOVED:
            SDL_NumJoysticks(); /* Refresh */
            SDL_NumHaptics(); /* Refresh */
            break;
    }

    if (EventFilter2) {
        post_event = !!EventFilter2(EventFilterUserData2, Event3to2(event3, &event2));
    }

    if (post_event && EventWatchers2 != NULL) {
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
        case SDL_EVENT_DISPLAY_ADDED:
        case SDL_EVENT_DISPLAY_REMOVED:
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
        case SDL_EVENT_WINDOW_HIT_TEST:
        case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
            if (SDL3_EventEnabled(SDL2_WINDOWEVENT)) {


                event2.window.type = SDL2_WINDOWEVENT;
                event2.window.timestamp = (Uint32) SDL_NS_TO_MS(event3->window.timestamp);
                event2.window.windowID = event3->window.windowID;
                event2.window.event = WindowEventType3To2(event3->type);
                event2.window.padding1 = 0;
                event2.window.padding2 = 0;
                event2.window.padding3 = 0;
                event2.window.data1 = event3->window.data1;
                event2.window.data2 = event3->window.data2;

                /* Fixes queue overflow with resize events that aren't processed */
                if (event2.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    RemovePendingSizeChangedAndResizedEvents_Data resizedata;
                    resizedata.new_event = &event2;
                    resizedata.saw_resized = false;
                    SDL_FilterEvents(RemovePendingSizeChangedAndResizedEvents, &resizedata);
                    if (resizedata.saw_resized) { /* if there was a pending resize, make sure one at the new dimensions remains. */
                        event2.window.event = SDL_WINDOWEVENT_RESIZED;
                        SDL_PushEvent(&event2);
                        event2.window.event = SDL_WINDOWEVENT_SIZE_CHANGED; /* then push the actual event next. */
                    }
                } else if (event2.window.event == SDL_WINDOWEVENT_MOVED || event2.window.event == SDL_WINDOWEVENT_EXPOSED) {
                    /* Flush old move and exposure events */
                    SDL_FilterEvents(RemoveSupercededWindowEvents, &event2);
                }

                SDL_PushEvent(&event2);
            }
            break;

        default: break;
    }

    return post_event;
}

static void CheckEventFilter(void)
{
    SDL_EventFilter filter = NULL;

    if (!SDL3_GetEventFilter(&filter, NULL) || filter != EventFilter3to2) {
        SDL3_SetEventFilter(EventFilter3to2, NULL);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_SetEventFilter(SDL2_EventFilter filter2, void *userdata)
{
    CheckEventFilter();

    EventFilter2 = filter2;
    EventFilterUserData2 = userdata;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GetEventFilter(SDL2_EventFilter *filter2, void **userdata)
{
    if (!EventFilter2) {
        return SDL2_FALSE;
    }

    if (filter2) {
        *filter2 = EventFilter2;
    }
    if (userdata) {
        *userdata = EventFilterUserData2;
    }

    return SDL2_TRUE;
}

SDL_DECLSPEC int SDLCALL
SDL_PeepEvents(SDL2_Event *events2, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType)
{
    int isstack = 0;
    SDL_Event *events3 = SDL3_small_alloc(SDL_Event, numevents ? numevents : 1, &isstack);
    int retval = 0;
    int i;

    if (!events3) {
        return -1;
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

    SDL3_small_free(events3, isstack);
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_WaitEventTimeout(SDL2_Event *event2, int timeout)
{
    SDL_Event event3;
    const int retval = SDL3_WaitEventTimeout(event2 ? &event3 : NULL, timeout);
    if ((retval == 1) && event2) {
        /* Ensure joystick and haptic IDs are updated before calling Event3to2() */
        switch (event3.type) {
            case SDL_EVENT_JOYSTICK_ADDED:
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
            case SDL_EVENT_JOYSTICK_REMOVED:
                SDL_NumJoysticks(); /* Refresh */
                SDL_NumHaptics(); /* Refresh */
                break;
        }
        Event3to2(&event3, event2);
    }
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_PollEvent(SDL2_Event *event2)
{
    return SDL_WaitEventTimeout(event2, 0);
}

SDL_DECLSPEC int SDLCALL
SDL_WaitEvent(SDL2_Event *event2)
{
    return SDL_WaitEventTimeout(event2, -1);
}

SDL_DECLSPEC void SDLCALL
SDL_AddEventWatch(SDL2_EventFilter filter2, void *userdata)
{
    EventFilterWrapperData *wrapperdata;

    CheckEventFilter();

    /* we set up an SDL3 event filter to manage things already; we will also use it to call all added SDL2 event watchers. Put this new one in that list. */
    wrapperdata = (EventFilterWrapperData *) SDL3_malloc(sizeof (EventFilterWrapperData));
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

SDL_DECLSPEC void SDLCALL
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

static bool SDLCALL
EventFilterWrapper3to2(void *userdata, SDL_Event *event)
{
    const EventFilterWrapperData *wrapperdata = (const EventFilterWrapperData *) userdata;
    SDL2_Event event2;
    return wrapperdata->filter2(wrapperdata->userdata, Event3to2(event, &event2));
}

SDL_DECLSPEC void SDLCALL
SDL_FilterEvents(SDL2_EventFilter filter2, void *userdata)
{
    EventFilterWrapperData wrapperdata;
    wrapperdata.filter2 = filter2;
    wrapperdata.userdata = userdata;
    wrapperdata.next = NULL;
    SDL3_FilterEvents(EventFilterWrapper3to2, &wrapperdata);
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_RegisterEvents(int numevents)
{
    Uint32 r = SDL3_RegisterEvents(numevents);
    return r > 0 ? r : (Uint32)(-1);
}

SDL_DECLSPEC SDL2_Keymod SDLCALL
SDL_GetModState(void)
{
    return SDL3_GetModState();
}

SDL_DECLSPEC void SDLCALL
SDL_SetModState(SDL2_Keymod modstate)
{
    SDL3_SetModState((SDL_Keymod)modstate);
}

SDL_DECLSPEC SDL_Keycode SDLCALL
SDL_GetKeyFromScancode(SDL2_Scancode scancode)
{
    if (((int)scancode) < SDL2_SCANCODE_UNKNOWN || scancode >= SDL2_NUM_SCANCODES) {
        SDL3_InvalidParamError("scancode");
        return 0;
    }

    if (scancode <= SDL2_SCANCODE_MODE) {
        /* Filter out extended keycodes. This matches the key mapping behavior of SDL2 backends. */
        const SDL_Keycode keycode = SDL3_GetKeyFromScancode(SDL2ScancodeToSDL3Scancode(scancode), SDL_KMOD_NONE, true);
        if (!(keycode & SDLK_EXTENDED_MASK)) {
            return keycode;
        }
    }

    return SDL_SCANCODE_TO_KEYCODE(scancode);
}

SDL_DECLSPEC SDL2_Scancode SDLCALL
SDL_GetScancodeFromKey(SDL_Keycode key)
{
    SDL_Keymod modstate = SDL_KMOD_NONE;
    SDL_Scancode scancode = SDL3_GetScancodeFromKey(key, &modstate);

    if (scancode == SDL_SCANCODE_UNKNOWN) {
        /* Keys commonly remapped to extended keycodes in SDL3.
         * If the keymap returns no mapping, convert the key to a scancode directly.
         */
        switch (key) {
        case SDLK_LCTRL:
        case SDLK_RCTRL:
        case SDLK_LALT:
        case SDLK_RALT:
        case SDLK_LGUI:
        case SDLK_RGUI:
        case SDLK_CAPSLOCK:
        case SDLK_SCROLLLOCK:
        case SDLK_PRINTSCREEN:
        case SDLK_INSERT:
            scancode = (SDL_Scancode)(key & ~SDLK_SCANCODE_MASK);
            break;
        case SDLK_TAB:
            /* ASCII value that can't be directly converted to a scancode. */
            scancode = SDL_SCANCODE_TAB;
            break;
        default:
            break;
        }
    }

    if (modstate != SDL_KMOD_NONE) {
        return SDL2_SCANCODE_UNKNOWN;
    }
    return SDL3ScancodeToSDL2Scancode(scancode);
}

static const char *SDL2_scancode_names[] = {
    /* 258 */ "AudioNext",
    /* 259 */ "AudioPrev",
    /* 260 */ "AudioStop",
    /* 261 */ "AudioPlay",
    /* 262 */ "AudioMute",
    /* 263 */ "MediaSelect",
    /* 264 */ "WWW",
    /* 265 */ "Mail",
    /* 266 */ "Calculator",
    /* 267 */ "Computer",
    /* 268 */ "AC Search",
    /* 269 */ "AC Home",
    /* 270 */ "AC Back",
    /* 271 */ "AC Forward",
    /* 272 */ "AC Stop",
    /* 273 */ "AC Refresh",
    /* 274 */ "AC Bookmarks",
    /* 275 */ "BrightnessDown",
    /* 276 */ "BrightnessUp",
    /* 277 */ "DisplaySwitch",
    /* 278 */ "KBDIllumToggle",
    /* 279 */ "KBDIllumDown",
    /* 280 */ "KBDIllumUp",
    /* 281 */ "Eject",
    /* 282 */ "Sleep",
    /* 283 */ "App1",
    /* 284 */ "App2",
    /* 285 */ "AudioRewind",
    /* 286 */ "AudioFastForward",
    /* 287 */ "SoftLeft",
    /* 288 */ "SoftRight",
    /* 289 */ "Call",
    /* 290 */ "EndCall",
};

SDL_DECLSPEC const char *SDLCALL
SDL_GetScancodeName(SDL2_Scancode scancode)
{
    unsigned int i;

    if (scancode <= SDL2_SCANCODE_MODE) {
        // They're the same
        return SDL3_GetScancodeName(SDL2ScancodeToSDL3Scancode(scancode));
    }

    i = (unsigned int)scancode - (SDL2_SCANCODE_MODE + 1);
    if (i >= SDL_arraysize(SDL2_scancode_names)) {
        SDL3_InvalidParamError("scancode");
        return "";
    }
    return SDL2_scancode_names[i];
}

SDL_DECLSPEC SDL2_Scancode SDLCALL
SDL_GetScancodeFromName(const char *name)
{
    int i;

    if (!name || !*name) {
        SDL3_InvalidParamError("name");
        return SDL2_SCANCODE_UNKNOWN;
    }

    for (i = 0; i < SDL2_NUM_SCANCODES; ++i) {
        if (SDL3_strcasecmp(name, SDL_GetScancodeName((SDL2_Scancode)i)) == 0) {
            return (SDL2_Scancode)i;
        }
    }

    SDL3_InvalidParamError("name");
    return SDL2_SCANCODE_UNKNOWN;
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetKeyName(SDL_Keycode key)
{
    if (key & SDLK_SCANCODE_MASK) {
        return SDL_GetScancodeName((SDL2_Scancode)(key & ~SDLK_SCANCODE_MASK));
    }
    return SDL3_GetKeyName(key);
}

SDL_DECLSPEC SDL_Keycode SDLCALL
SDL_GetKeyFromName(const char *name)
{
    SDL_Keycode key;

    /* Check input */
    if (!name) {
        return SDLK_UNKNOWN;
    }

    /* If it's a single UTF-8 character, then that's the keycode itself */
    key = *(const unsigned char *)name;
    if (key >= 0xF0) {
        if (SDL_strlen(name) == 4) {
            int i = 0;
            key = (Uint16)(name[i] & 0x07) << 18;
            key |= (Uint16)(name[++i] & 0x3F) << 12;
            key |= (Uint16)(name[++i] & 0x3F) << 6;
            key |= (Uint16)(name[++i] & 0x3F);
            return key;
        }
        return SDLK_UNKNOWN;
    } else if (key >= 0xE0) {
        if (SDL_strlen(name) == 3) {
            int i = 0;
            key = (Uint16)(name[i] & 0x0F) << 12;
            key |= (Uint16)(name[++i] & 0x3F) << 6;
            key |= (Uint16)(name[++i] & 0x3F);
            return key;
        }
        return SDLK_UNKNOWN;
    } else if (key >= 0xC0) {
        if (SDL_strlen(name) == 2) {
            int i = 0;
            key = (Uint16)(name[i] & 0x1F) << 6;
            key |= (Uint16)(name[++i] & 0x3F);
            return key;
        }
        return SDLK_UNKNOWN;
    } else {
        if (SDL_strlen(name) == 1) {
            if (key >= 'A' && key <= 'Z') {
                key += 32;
            }
            return key;
        }

        /* Get the scancode for this name, and the associated keycode */
        return SDL_GetKeyFromScancode(SDL_GetScancodeFromName(name));
    }
}

SDL_DECLSPEC const Uint8 *SDLCALL
SDL_GetKeyboardState(int *numkeys)
{
    return (const Uint8 *)SDL3_GetKeyboardState(numkeys);
}

/* Several SDL3 video backends have had their names lower-cased, map to the SDL2 equivalent name. */
static const char *ReplaceVideoBackendName(const char *name)
{
    if (name) {
        #define CHECKBACKEND(sdl2name) if (SDL_strcasecmp(name, sdl2name) == 0) { return sdl2name; }
        CHECKBACKEND("KMSDRM");
        CHECKBACKEND("RPI");
        CHECKBACKEND("Android");
        CHECKBACKEND("PSP");
        CHECKBACKEND("PS2");
        CHECKBACKEND("VITA");
        #undef CHECKBACKEND
    }
    return name;
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetVideoDriver(int idx)
{
    return ReplaceVideoBackendName(SDL3_GetVideoDriver(idx));
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetCurrentVideoDriver(void)
{
    return ReplaceVideoBackendName(SDL3_GetCurrentVideoDriver());
}


/* mouse coords became floats in SDL3 */

SDL_DECLSPEC Uint32 SDLCALL
SDL_GetMouseState(int *x, int *y)
{
    float fx, fy;
    Uint32 ret = SDL3_GetMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return ret;
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_GetGlobalMouseState(int *x, int *y)
{
    float fx, fy;
    Uint32 ret = SDL3_GetGlobalMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return ret;
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_GetRelativeMouseState(int *x, int *y)
{
    float fx, fy;
    Uint32 ret = SDL3_GetRelativeMouseState(&fx, &fy);
    if (x) *x = (int)fx;
    if (y) *y = (int)fy;
    return ret;
}

SDL_DECLSPEC void SDLCALL
SDL_WarpMouseInWindow(SDL_Window *window, int x, int y)
{
    SDL3_WarpMouseInWindow(window, (float)x, (float)y);
}

SDL_DECLSPEC int SDLCALL
SDL_WarpMouseGlobal(int x, int y)
{
    return SDL3_WarpMouseGlobal((float)x, (float)y) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_SetRelativeMouseMode(SDL2_bool enabled)
{
    int retval = 0;
    SDL_Window **windows = SDL3_GetWindows(NULL);
    if (windows) {
        int i;

        for (i = 0; windows[i]; ++i) {
            if (!SDL3_SetWindowRelativeMouseMode(windows[i], enabled)) {
                retval = -1;
            }
        }

        SDL_free(windows);
    }
    if (retval == 0) {
        relative_mouse_mode = enabled;
    }
    return retval;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GetRelativeMouseMode(void)
{
    return relative_mouse_mode;
}

SDL_DECLSPEC int SDLCALL
SDL_CaptureMouse(SDL2_bool enabled)
{
    return SDL3_CaptureMouse(enabled) ? 0 : -1;
}

SDL_DECLSPEC SDL2_RWops *SDLCALL
SDL_AllocRW(void)
{
    SDL2_RWops *rwops2 = (SDL2_RWops *)SDL3_malloc(sizeof *rwops2);
    if (rwops2) {
        rwops2->type = SDL_RWOPS_UNKNOWN;
    }
    return rwops2;
}

SDL_DECLSPEC void SDLCALL
SDL_FreeRW(SDL2_RWops *rwops2)
{
    SDL3_free(rwops2);
}

static Sint64 SDLCALL
RWops3to2_size(SDL2_RWops *rwops2)
{
    return SDL3_GetIOSize(rwops2->hidden.sdl3.iostrm);
}

static Sint64 SDLCALL
RWops3to2_seek(SDL2_RWops *rwops2, Sint64 offset, int whence)
{
    return SDL3_SeekIO(rwops2->hidden.sdl3.iostrm, offset, (SDL_IOWhence) whence);
}

static size_t SDLCALL
RWops3to2_read(SDL2_RWops *rwops2, void *ptr, size_t size, size_t maxnum)
{
    size_t count = 0;
    if (size > 0 && maxnum > 0) {
        count = SDL3_ReadIO(rwops2->hidden.sdl3.iostrm, ptr, (size * maxnum)) / size;
    }
    return count;
}

static size_t SDLCALL
RWops3to2_write(SDL2_RWops *rwops2, const void *ptr, size_t size, size_t maxnum)
{
    size_t count = 0;
    if (size > 0 && maxnum > 0) {
        count = SDL3_WriteIO(rwops2->hidden.sdl3.iostrm, ptr, (size * maxnum)) / size;
    }
    return count;
}

static int SDLCALL
RWops3to2_close(SDL2_RWops *rwops2)
{
    const int retval = SDL3_CloseIO(rwops2->hidden.sdl3.iostrm) ? 0 : -1;
    SDL_FreeRW(rwops2);
    return retval;
}

static SDL2_RWops *
RWops3to2(SDL_IOStream *iostrm3, Uint32 type)
{
    SDL2_RWops *rwops2 = NULL;
    if (iostrm3) {
        rwops2 = SDL_AllocRW();
        if (!rwops2) {
            SDL3_CloseIO(iostrm3);  /* !!! FIXME: make sure this is still safe if things change. */
            return NULL;
        }

        SDL3_zerop(rwops2);
        rwops2->size = RWops3to2_size;
        rwops2->seek = RWops3to2_seek;
        rwops2->read = RWops3to2_read;
        rwops2->write = RWops3to2_write;
        rwops2->close = RWops3to2_close;
        rwops2->type = type;
        rwops2->hidden.sdl3.iostrm = iostrm3;
    }
    return rwops2;
}

SDL_DECLSPEC SDL2_RWops *SDLCALL
SDL_RWFromFile(const char *file, const char *mode)
{
    SDL2_RWops *rwops2 = NULL;

    /* match some SDL2 Apple-specific quirks that were removed from SDL3. */
    #if defined(SDL_PLATFORM_APPLE)
    char *adjusted_path = NULL;
    /* If the file mode is writable, skip all the bundle stuff because generally the bundle is read-only. */
    if (mode && (SDL3_strchr(mode, 'r') != NULL) && SDL3_GetHintBoolean(SDL_HINT_APPLE_RWFROMFILE_USE_RESOURCES, true)) {
        const char *base = SDL3_GetBasePath();
        if (base) {
            if (SDL3_asprintf(&adjusted_path, "%s%s", base, file) < 0) {
                return NULL;
            }
            rwops2 = RWops3to2(SDL3_IOFromFile(adjusted_path, mode), SDL_RWOPS_PLATFORM_FILE);
            SDL_free(adjusted_path);
        }
    }
    if (!rwops2) {
        rwops2 = RWops3to2(SDL3_IOFromFile(file, mode), SDL_RWOPS_PLATFORM_FILE);
    }
    #else
    rwops2 = RWops3to2(SDL3_IOFromFile(file, mode), SDL_RWOPS_PLATFORM_FILE);
    #endif

    if (rwops2) {
        const SDL_PropertiesID props = SDL3_GetIOProperties(rwops2->hidden.sdl3.iostrm);
        if (props) {
            void *handle = NULL;
            #if defined(SDL_PLATFORM_WINDOWS)
            if (!handle) {
                handle = SDL3_GetPointerProperty(props, SDL_PROP_IOSTREAM_WINDOWS_HANDLE_POINTER, NULL);
                if (handle) {
                    rwops2->hidden.windowsio.h = handle;
                }
            }
            #elif defined(SDL_PLATFORM_ANDROID)
            if (!handle) {
                handle = SDL3_GetPointerProperty(props, SDL_PROP_IOSTREAM_ANDROID_AASSET_POINTER, NULL);
                if (handle) {
                    rwops2->hidden.androidio.asset = handle;
                }
            }
            #endif

            if (!handle) {
                handle = SDL3_GetPointerProperty(props, SDL_PROP_IOSTREAM_STDIO_FILE_POINTER, NULL);
                if (handle) {
                    rwops2->hidden.stdio.autoclose = SDL2_FALSE;
                    rwops2->hidden.stdio.fp = handle;
                }
            }
        }
    }
    return rwops2;
}

SDL_DECLSPEC SDL2_RWops *SDLCALL
SDL_RWFromMem(void *mem, int size)
{
    if (size < 0) { /* SDL3 already checks size == 0 */
        SDL3_InvalidParamError("size");
        return NULL;
    }
    return RWops3to2(SDL3_IOFromMem(mem, size), SDL_RWOPS_MEMORY);
}

SDL_DECLSPEC SDL2_RWops *SDLCALL
SDL_RWFromConstMem(const void *mem, int size)
{
    if (size < 0) { /* SDL3 already checks size == 0 */
        SDL3_InvalidParamError("size");
        return NULL;
    }
    return RWops3to2(SDL3_IOFromConstMem(mem, size), SDL_RWOPS_MEMORY_RO);
}

SDL_DECLSPEC Sint64 SDLCALL
SDL_RWsize(SDL2_RWops *rwops2)
{
    return rwops2->size(rwops2);
}

SDL_DECLSPEC Sint64 SDLCALL
SDL_RWseek(SDL2_RWops *rwops2, Sint64 offset, int whence)
{
    return rwops2->seek(rwops2, offset, whence);
}

SDL_DECLSPEC Sint64 SDLCALL
SDL_RWtell(SDL2_RWops *rwops2)
{
    return rwops2->seek(rwops2, 0, SDL_IO_SEEK_CUR);
}

SDL_DECLSPEC size_t SDLCALL
SDL_RWread(SDL2_RWops *rwops2, void *ptr, size_t size, size_t maxnum)
{
    return rwops2->read(rwops2, ptr, size, maxnum);
}

SDL_DECLSPEC size_t SDLCALL
SDL_RWwrite(SDL2_RWops *rwops2, const void *ptr, size_t size, size_t num)
{
    return rwops2->write(rwops2, ptr, size, num);
}

SDL_DECLSPEC int SDLCALL
SDL_RWclose(SDL2_RWops *rwops2)
{
    return rwops2->close(rwops2);
}

SDL_DECLSPEC Uint8 SDLCALL
SDL_ReadU8(SDL2_RWops *rwops2)
{
    Uint8 x = 0;
    SDL_RWread(rwops2, &x, sizeof (x), 1);
    return x;
}

SDL_DECLSPEC size_t SDLCALL
SDL_WriteU8(SDL2_RWops *rwops2, Uint8 x)
{
    return SDL_RWwrite(rwops2, &x, sizeof(x), 1);
}

#define DO_RWOPS_ENDIAN(order, bits)        \
SDL_DECLSPEC Uint##bits SDLCALL             \
SDL_Read##order##bits(SDL2_RWops *rwops2) { \
    Uint##bits x = 0;                       \
    SDL_RWread(rwops2, &x, sizeof (x), 1);  \
    return SDL_Swap##bits##order(x);        \
} \
                                                           \
SDL_DECLSPEC size_t SDLCALL                                \
SDL_Write##order##bits(SDL2_RWops *rwops2, Uint##bits x) { \
    x = SDL_Swap##bits##order(x);                          \
    return SDL_RWwrite(rwops2, &x, sizeof(x), 1);          \
}
DO_RWOPS_ENDIAN(LE, 16)
DO_RWOPS_ENDIAN(BE, 16)
DO_RWOPS_ENDIAN(LE, 32)
DO_RWOPS_ENDIAN(BE, 32)
DO_RWOPS_ENDIAN(LE, 64)
DO_RWOPS_ENDIAN(BE, 64)
#undef DO_RWOPS_ENDIAN

/* stdio SDL_RWops was removed from SDL3, to prevent incompatible C runtime issues */
#ifndef HAVE_STDIO_H
SDL_DECLSPEC SDL2_RWops * SDLCALL
SDL_RWFromFP(void *fp, SDL2_bool autoclose)
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

    pos = SDL_RWseek(rwops2, 0, SDL_IO_SEEK_CUR);
    if (pos < 0) {
        return -1;
    }
    size = SDL_RWseek(rwops2, 0, SDL_IO_SEEK_END);

    SDL_RWseek(rwops2, pos, SDL_IO_SEEK_SET);
    return size;
}

static Sint64 SDLCALL
stdio_seek(SDL2_RWops *rwops2, Sint64 offset, int whence)
{
    FILE *fp = (FILE *) rwops2->hidden.stdio.fp;
    int stdiowhence;

    switch (whence) {
    case SDL_IO_SEEK_SET:
        stdiowhence = SEEK_SET;
        break;
    case SDL_IO_SEEK_CUR:
        stdiowhence = SEEK_CUR;
        break;
    case SDL_IO_SEEK_END:
        stdiowhence = SEEK_END;
        break;
    default:
        SDL3_SetError("Unknown value for 'whence'");
        return -1;
    }

#if defined(FSEEK_OFF_MIN) && defined(FSEEK_OFF_MAX)
    if (offset < (Sint64)(FSEEK_OFF_MIN) || offset > (Sint64)(FSEEK_OFF_MAX)) {
        SDL3_SetError("Seek offset out of range");
        return -1;
    }
#endif

    if (fseek(fp, (long)offset, stdiowhence) == 0) {
        Sint64 pos = ftell(fp);
        if (pos < 0) {
            SDL3_SetError("Couldn't get stream offset");
            return -1;
        }
        return pos;
    }
    return SDL_Error(SDL_EFSEEK);
}

static size_t SDLCALL
stdio_read(SDL2_RWops *rwops2, void *ptr, size_t size, size_t maxnum)
{
    FILE *fp = (FILE *) rwops2->hidden.stdio.fp;
    size_t nread = fread(ptr, size, maxnum, fp);
    if (nread == 0 && ferror(fp)) {
        SDL_Error(SDL_EFREAD);
    }
    return nread;
}

static size_t SDLCALL
stdio_write(SDL2_RWops *rwops2, const void *ptr, size_t size, size_t num)
{
    FILE *fp = (FILE *) rwops2->hidden.stdio.fp;
    size_t nwrote = fwrite(ptr, size, num, fp);
    if (nwrote == 0 && ferror(fp)) {
        SDL_Error(SDL_EFWRITE);
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
                status = SDL_Error(SDL_EFWRITE);
            }
        }
        SDL_FreeRW(rwops2);
    }
    return status;
}

SDL_DECLSPEC SDL2_RWops * SDLCALL
SDL_RWFromFP(void *fp, SDL2_bool autoclose)
{
    SDL2_RWops *rwops = SDL_AllocRW();
    if (rwops != NULL) {
        rwops->size = stdio_size;
        rwops->seek = stdio_seek;
        rwops->read = stdio_read;
        rwops->write = stdio_write;
        rwops->close = stdio_close;
        rwops->hidden.stdio.fp = (FILE *) fp;
        rwops->hidden.stdio.autoclose = autoclose;
        rwops->type = SDL_RWOPS_STDFILE;
    }
    return rwops;
}
#endif

static Sint64 SDLCALL
RWops2to3_size(void *userdata)
{
    return SDL_RWsize((SDL2_RWops *) userdata);
}

static Sint64 SDLCALL
RWops2to3_seek(void *userdata, Sint64 offset, SDL_IOWhence whence)
{
    return SDL_RWseek((SDL2_RWops *) userdata, offset, whence);
}

static size_t SDLCALL
RWops2to3_read(void *userdata, void *ptr, size_t size, SDL_IOStatus *status)
{
    return SDL_RWread((SDL2_RWops *) userdata, ptr, 1, size);
}

static size_t SDLCALL
RWops2to3_write(void *userdata, const void *ptr, size_t size, SDL_IOStatus *status)
{
    return SDL_RWwrite((SDL2_RWops *) userdata, ptr, 1, size);
}

static bool SDLCALL
RWops2to3_close(void *userdata)
{
    /* Never close the SDL2_RWops here! This is just a wrapper to talk to SDL3 APIs. We will manually close the rwops2 if appropriate. */
    /*return SDL3_CloseIO((SDL2_RWops *) userdata) ? 0 : -1;*/
    return true;
}

static SDL_IOStream *
RWops2to3(SDL2_RWops *rwops2)
{
    SDL_IOStream *iostrm3 = NULL;
    if (rwops2) {
        SDL_IOStreamInterface iface;
        SDL_INIT_INTERFACE(&iface);
        iface.size = RWops2to3_size;
        iface.seek = RWops2to3_seek;
        iface.read = RWops2to3_read;
        iface.write = RWops2to3_write;
        iface.close = RWops2to3_close;

        iostrm3 = SDL3_OpenIO(&iface, rwops2);
        if (!iostrm3) {
            return NULL;
        }
    }
    return iostrm3;
}

SDL_DECLSPEC void *SDLCALL
SDL_LoadFile_RW(SDL2_RWops *rwops2, size_t *datasize, int freesrc)
{
    void *retval = NULL;

    SDL_IOStream *iostrm3 = RWops2to3(rwops2);
    if (iostrm3) {
        retval = SDL3_LoadFile_IO(iostrm3, datasize, true);  /* always close the iostrm3 bridge object. */
    }

    if (rwops2 && freesrc) {
        SDL_RWclose(rwops2);
    }

    return retval;
}

SDL_DECLSPEC SDL2_AudioSpec *SDLCALL
SDL_LoadWAV_RW(SDL2_RWops *rwops2, int freesrc, SDL2_AudioSpec *spec2, Uint8 **audio_buf, Uint32 *audio_len)
{
    SDL2_AudioSpec *retval = NULL;

    if (spec2 == NULL) {
        SDL3_InvalidParamError("spec");
    } else {
        SDL_IOStream *iostrm3 = RWops2to3(rwops2);
        if (iostrm3) {
            SDL_AudioSpec spec3;
            const bool rc = SDL3_LoadWAV_IO(iostrm3, true, &spec3, audio_buf, audio_len);   /* always close the iostrm3 bridge object. */
            SDL3_zerop(spec2);
            if (rc) {
                spec2->format = spec3.format;
                spec2->channels = spec3.channels;
                spec2->freq = spec3.freq;
                spec2->samples = 4096; /* This is what SDL2 hardcodes, also. */
                spec2->silence = SDL3_GetSilenceValueForFormat(spec3.format);
                retval = spec2;
            }
        }
    }

    if (rwops2 && freesrc) {
        SDL_RWclose(rwops2);
    }

    return retval;
}

#define PROP_SURFACE2 "sdl2-compat.surface2"

static void SDLCALL CleanupSurface2(void *userdata, void *value)
{
    SDL2_Surface *surface = (SDL2_Surface *)value;

    if (surface->format) {
        SDL_SetPixelFormatPalette(surface->format, NULL);
        SDL_FreeFormat(surface->format);
        surface->format = NULL;
    }
    SDL_free(surface);
}

static SDL2_Surface *CreateSurface2from3(SDL_Surface *surface3)
{
    /* Allocate the surface */
    SDL2_Surface *surface = (SDL2_Surface *)SDL3_calloc(1, sizeof(*surface));
    if (!surface) {
        SDL3_OutOfMemory();
        return NULL;
    }

    /* Link the surfaces */
    surface->map = (SDL_BlitMap *)surface3;
    SDL3_SetPointerPropertyWithCleanup(SDL3_GetSurfaceProperties(surface3), PROP_SURFACE2, surface, CleanupSurface2, NULL);

    surface->format = SDL_AllocFormat(surface3->format);
    if (!surface->format) {
        SDL_FreeSurface(surface);
        return NULL;
    }
    surface->flags = (surface3->flags & SHARED_SURFACE_FLAGS);
    surface->w = surface3->w;
    surface->h = surface3->h;
    surface->pixels = surface3->pixels;
    surface->pitch = surface3->pitch;

    SDL3_GetSurfaceClipRect(surface3, &surface->clip_rect);

    if (SDL_ISPIXELFORMAT_INDEXED(surface3->format)) {
        SDL_Palette *palette = SDL3_GetSurfacePalette(surface3);
        if (!palette) {
            palette = SDL3_CreateSurfacePalette(surface3);
            if (!palette) {
                SDL_FreeSurface(surface);
                return NULL;
            }
        }
        surface->format->palette = palette;
        ++palette->refcount;
    }

    /* The surface is ready to go */
    surface->refcount = 1;
    return surface;
}

static SDL2_Surface *Surface3to2(SDL_Surface *surface)
{
    SDL2_Surface *surface2 = NULL;

    if (surface) {
        surface2 = (SDL2_Surface *)SDL3_GetPointerProperty(SDL3_GetSurfaceProperties(surface), PROP_SURFACE2, NULL);
        if (!surface2) {
            surface2 = CreateSurface2from3(surface);
        }
    }
    return surface2;
}

static SDL_Surface *Surface2to3(SDL2_Surface *surface)
{
    SDL_Surface *surface3 = NULL;

    if (surface) {
        surface3 = (SDL_Surface *)surface->map;
        SDL_assert(surface3 != NULL);

        /* Synchronize any changes made by the application to the SDL2 surface
         * The application might have changed memory allocation, e.g.:
         * https://github.com/libsdl-org/SDL_ttf/blob/7e4a456bf463b887b94c191030dd742d7654d6ff/SDL_ttf.c#L1476-L1478
         */
        surface3->w = surface->w;
        surface3->h = surface->h;
        surface3->flags &= ~(surface->flags & SHARED_SURFACE_FLAGS);
        surface3->flags |= (surface->flags & SHARED_SURFACE_FLAGS);
        surface3->pixels = surface->pixels;
        surface3->pitch = surface->pitch;
    }
    return surface3;
}

SDL_DECLSPEC SDL2_Surface *SDLCALL
SDL_LoadBMP_RW(SDL2_RWops *rwops2, int freesrc)
{
    SDL_Surface *retval = NULL;
    SDL_IOStream *iostrm3 = RWops2to3(rwops2);
    if (iostrm3) {
        retval = SDL3_LoadBMP_IO(iostrm3, true);   /* always close the iostrm3 bridge object. */
    }
    if (rwops2 && freesrc) {
        SDL_RWclose(rwops2);
    }
    return Surface3to2(retval);
}

SDL_DECLSPEC int SDLCALL
SDL_SaveBMP_RW(SDL2_Surface *surface, SDL2_RWops *rwops, int freedst)
{
    int retval = -1;
    SDL_IOStream *iostream = RWops2to3(rwops);
    if (iostream) {
        retval = SDL3_SaveBMP_IO(Surface2to3(surface), iostream, true) ? 0 : -1;    /* always close the iostrm3 bridge object. */
    }
    if (rwops && freedst) {
        SDL_RWclose(rwops);
    }
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_GameControllerAddMappingsFromRW(SDL2_RWops *rwops2, int freerw)
{
    int retval = -1;
    SDL_IOStream *iostrm3 = RWops2to3(rwops2);
    if (iostrm3) {
        retval = SDL3_AddGamepadMappingsFromIO(iostrm3, true);    /* always close the iostrm3 bridge object. */
    }
    if (rwops2 && freerw) {
        SDL_RWclose(rwops2);
    }
    return retval;
}


/* All gamma stuff was removed from SDL3 because it affects the whole system
   in intrusive ways, and often didn't work on various platforms. These all
   just return failure now. */

SDL_DECLSPEC int SDLCALL
SDL_SetWindowBrightness(SDL_Window *window, float brightness)
{
    SDL3_Unsupported();
    return -1;
}

SDL_DECLSPEC float SDLCALL
SDL_GetWindowBrightness(SDL_Window *window)
{
    if (!window) {
        SDL3_SetError("Invalid window");
    }
    return 1.0f;
}

SDL_DECLSPEC int SDLCALL
SDL_SetWindowGammaRamp(SDL_Window *window, const Uint16 *r, const Uint16 *g, const Uint16 *b)
{
    SDL3_Unsupported();
    return -1;
}

SDL_DECLSPEC void SDLCALL
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

SDL_DECLSPEC int SDLCALL
SDL_GetWindowGammaRamp(SDL_Window *window, Uint16 *red, Uint16 *blue, Uint16 *green)
{
    Uint16 *buf;

    if (!window) {
        SDL3_SetError("Invalid window");
        return -1;
    }

    buf = red ? red : (green ? green : blue);
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

SDL_DECLSPEC int SDLCALL
SDL_GetWindowOpacity(SDL_Window *window, float *out_opacity)
{
    float opacity = SDL3_GetWindowOpacity(window);
    if (opacity < 0.0f) {
        return -1;
    }
    if (out_opacity) {
        *out_opacity = opacity;
    }
    return 0;
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_ConvertSurface(SDL2_Surface *surface, const SDL2_PixelFormat *format, Uint32 flags)
{
    (void) flags; /* SDL3 removed the (unused) `flags` argument */

    if (!surface) {
        SDL3_InvalidParamError("surface");
        return NULL;
    }
    if (!format) {
        SDL3_InvalidParamError("format");
        return NULL;
    }

    return Surface3to2(SDL3_ConvertSurfaceAndColorspace(Surface2to3(surface), (SDL_PixelFormat)format->format, format->palette, SDL_COLORSPACE_SRGB, 0));
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_DuplicateSurface(SDL2_Surface *surface)
{
    return Surface3to2(SDL3_DuplicateSurface(Surface2to3(surface)));
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_ConvertSurfaceFormat(SDL2_Surface *surface, Uint32 pixel_format, Uint32 flags)
{
    (void) flags; /* SDL3 removed the (unused) `flags` argument */
    return Surface3to2(SDL3_ConvertSurface(Surface2to3(surface), (SDL_PixelFormat)pixel_format));
}

#define SDL_YUV_SD_THRESHOLD 576

static SDL_YUV_CONVERSION_MODE SDL_YUV_ConversionMode = SDL_YUV_CONVERSION_BT601;

SDL_DECLSPEC void SDLCALL
SDL_SetYUVConversionMode(SDL_YUV_CONVERSION_MODE mode)
{
    SDL_YUV_ConversionMode = mode;
}

SDL_DECLSPEC SDL_YUV_CONVERSION_MODE SDLCALL
SDL_GetYUVConversionMode(void)
{
    return SDL_YUV_ConversionMode;
}

SDL_DECLSPEC SDL_YUV_CONVERSION_MODE SDLCALL
SDL_GetYUVConversionModeForResolution(int width, int height)
{
    SDL_YUV_CONVERSION_MODE mode = SDL_GetYUVConversionMode();
    if (mode == SDL_YUV_CONVERSION_AUTOMATIC) {
        if (height <= SDL_YUV_SD_THRESHOLD) {
            mode = SDL_YUV_CONVERSION_BT601;
        } else {
            mode = SDL_YUV_CONVERSION_BT709;
        }
    }
    return mode;
}

static SDL_Colorspace GetColorspaceForFormatAndSize(Uint32 format, int width, int height)
{
    if (SDL_ISPIXELFORMAT_FOURCC(format)) {
        switch (SDL_GetYUVConversionModeForResolution(width, height)) {
        case SDL_YUV_CONVERSION_JPEG:
            return SDL_COLORSPACE_BT601_FULL;
        case SDL_YUV_CONVERSION_BT601:
            return SDL_COLORSPACE_BT601_LIMITED;
        case SDL_YUV_CONVERSION_BT709:
            return SDL_COLORSPACE_BT709_LIMITED;
        default:
            break;
        }
    }
    return SDL_COLORSPACE_SRGB;
}

SDL_DECLSPEC int SDLCALL
SDL_ConvertPixels(int width, int height, Uint32 src_format, const void *src, int src_pitch, Uint32 dst_format, void *dst, int dst_pitch)
{
    SDL_Colorspace src_colorspace = GetColorspaceForFormatAndSize(src_format, width, height);
    SDL_Colorspace dst_colorspace = GetColorspaceForFormatAndSize(dst_format, width, height);
    return SDL3_ConvertPixelsAndColorspace(width, height, (SDL_PixelFormat)src_format, src_colorspace, 0, src, src_pitch, (SDL_PixelFormat)dst_format, dst_colorspace, 0, dst, dst_pitch) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_PremultiplyAlpha(int width, int height, Uint32 src_format, const void * src, int src_pitch, Uint32 dst_format, void * dst, int dst_pitch)
{
    return SDL3_PremultiplyAlpha(width, height, (SDL_PixelFormat)src_format, src, src_pitch, (SDL_PixelFormat)dst_format, dst, dst_pitch, false) ? 0 : -1;
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_CreateRGBSurface(Uint32 flags, int width, int height, int depth, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return Surface3to2(SDL3_CreateSurface(width, height, SDL3_GetPixelFormatForMasks(depth, Rmask, Gmask, Bmask, Amask)));
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_CreateRGBSurfaceWithFormat(Uint32 flags, int width, int height, int depth, Uint32 format)
{
    return Surface3to2(SDL3_CreateSurface(width, height, (SDL_PixelFormat)format));
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return Surface3to2(SDL3_CreateSurfaceFrom(width, height, SDL3_GetPixelFormatForMasks(depth, Rmask, Gmask, Bmask, Amask), pixels, pitch));
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_CreateRGBSurfaceWithFormatFrom(void *pixels, int width, int height, int depth, int pitch, Uint32 format)
{
    return Surface3to2(SDL3_CreateSurfaceFrom(width, height, (SDL_PixelFormat)format, pixels, pitch));
}

SDL_DECLSPEC void SDLCALL
SDL_FreeSurface(SDL2_Surface *surface)
{
    if (!surface) {
        return;
    }

    if (--surface->refcount > 0) {
        return;
    }

    SDL3_DestroySurface(Surface2to3(surface));
}

SDL_DECLSPEC int SDLCALL
SDL_FillRect(SDL2_Surface *dst, const SDL_Rect *rect, Uint32 color)
{
    return SDL3_FillSurfaceRect(Surface2to3(dst), rect, color) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_FillRects(SDL2_Surface *dst, const SDL_Rect *rects, int count, Uint32 color)
{
    return SDL3_FillSurfaceRects(Surface2to3(dst), rects, count, color) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_UpperBlit(SDL2_Surface *src, const SDL_Rect *srcrect, SDL2_Surface *dst, SDL_Rect *dstrect)
{
    SDL_Rect r_src, r_dst;

    /* Make sure the surfaces aren't locked */
    if (!src) {
        SDL3_InvalidParamError("src");
        return -1;
    } else if (!dst) {
        SDL3_InvalidParamError("dst");
        return -1;
    } else if (src->locked || dst->locked) {
        SDL3_SetError("Surfaces must not be locked during blit");
        return -1;
    }

    /* Full src surface */
    r_src.x = 0;
    r_src.y = 0;
    r_src.w = src->w;
    r_src.h = src->h;

    if (dstrect) {
        r_dst.x = dstrect->x;
        r_dst.y = dstrect->y;
    } else {
        r_dst.x = 0;
        r_dst.y = 0;
    }

    /* clip the source rectangle to the source surface */
    if (srcrect) {
        SDL_Rect tmp;
        if (!SDL3_GetRectIntersection(srcrect, &r_src, &tmp)) {
            goto end;
        }

        /* Shift dstrect, if srcrect origin has changed */
        r_dst.x += tmp.x - srcrect->x;
        r_dst.y += tmp.y - srcrect->y;

        /* Update srcrect */
        r_src = tmp;
    }

    /* There're no dstrect.w/h parameters. It's the same as srcrect */
    r_dst.w = r_src.w;
    r_dst.h = r_src.h;

    /* clip the destination rectangle against the clip rectangle */
    {
        SDL_Rect tmp;
        if (!SDL3_GetRectIntersection(&r_dst, &dst->clip_rect, &tmp)) {
            goto end;
        }

        /* Shift srcrect, if dstrect has changed */
        r_src.x += tmp.x - r_dst.x;
        r_src.y += tmp.y - r_dst.y;
        r_src.w = tmp.w;
        r_src.h = tmp.h;

        /* Update dstrect */
        r_dst = tmp;
    }

    if (r_dst.w > 0 && r_dst.h > 0) {
        if (dstrect) { /* update output parameter */
            *dstrect = r_dst;
        }
        return SDL_LowerBlit(src, &r_src, dst, &r_dst);
    }

end:
    if (dstrect) { /* update output parameter */
        dstrect->w = dstrect->h = 0;
    }
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_LowerBlit(SDL2_Surface *src, SDL_Rect *srcrect, SDL2_Surface *dst, SDL_Rect *dstrect)
{
    return SDL3_BlitSurfaceUnchecked(Surface2to3(src), srcrect, Surface2to3(dst), dstrect) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_UpperBlitScaled(SDL2_Surface *src, const SDL_Rect *srcrect, SDL2_Surface *dst, SDL_Rect *dstrect)
{
    double src_x0, src_y0, src_x1, src_y1;
    double dst_x0, dst_y0, dst_x1, dst_y1;
    SDL_Rect final_src, final_dst;
    double scaling_w, scaling_h;
    int src_w, src_h;
    int dst_w, dst_h;

    /* Make sure the surfaces aren't locked */
    if (!src || !dst) {
        SDL3_InvalidParamError("SDL_UpperBlitScaled(): src/dst");
        return -1;
    }
    if (src->locked || dst->locked) {
        SDL3_SetError("Surfaces must not be locked during blit");
        return -1;
    }

    if (!srcrect) {
        src_w = src->w;
        src_h = src->h;
    } else {
        src_w = srcrect->w;
        src_h = srcrect->h;
    }

    if (!dstrect) {
        dst_w = dst->w;
        dst_h = dst->h;
    } else {
        dst_w = dstrect->w;
        dst_h = dstrect->h;
    }

    if (dst_w == src_w && dst_h == src_h) {
        /* No scaling, defer to regular blit */
        return SDL_UpperBlit(src, srcrect, dst, dstrect);
    }

    scaling_w = (double)dst_w / src_w;
    scaling_h = (double)dst_h / src_h;

    if (!dstrect) {
        dst_x0 = 0;
        dst_y0 = 0;
        dst_x1 = dst_w;
        dst_y1 = dst_h;
    } else {
        dst_x0 = dstrect->x;
        dst_y0 = dstrect->y;
        dst_x1 = dst_x0 + dst_w;
        dst_y1 = dst_y0 + dst_h;
    }

    if (!srcrect) {
        src_x0 = 0;
        src_y0 = 0;
        src_x1 = src_w;
        src_y1 = src_h;
    } else {
        src_x0 = srcrect->x;
        src_y0 = srcrect->y;
        src_x1 = src_x0 + src_w;
        src_y1 = src_y0 + src_h;

        /* Clip source rectangle to the source surface */

        if (src_x0 < 0) {
            dst_x0 -= src_x0 * scaling_w;
            src_x0 = 0;
        }

        if (src_x1 > src->w) {
            dst_x1 -= (src_x1 - src->w) * scaling_w;
            src_x1 = src->w;
        }

        if (src_y0 < 0) {
            dst_y0 -= src_y0 * scaling_h;
            src_y0 = 0;
        }

        if (src_y1 > src->h) {
            dst_y1 -= (src_y1 - src->h) * scaling_h;
            src_y1 = src->h;
        }
    }

    /* Clip destination rectangle to the clip rectangle */

    /* Translate to clip space for easier calculations */
    dst_x0 -= dst->clip_rect.x;
    dst_x1 -= dst->clip_rect.x;
    dst_y0 -= dst->clip_rect.y;
    dst_y1 -= dst->clip_rect.y;

    if (dst_x0 < 0) {
        src_x0 -= dst_x0 / scaling_w;
        dst_x0 = 0;
    }

    if (dst_x1 > dst->clip_rect.w) {
        src_x1 -= (dst_x1 - dst->clip_rect.w) / scaling_w;
        dst_x1 = dst->clip_rect.w;
    }

    if (dst_y0 < 0) {
        src_y0 -= dst_y0 / scaling_h;
        dst_y0 = 0;
    }

    if (dst_y1 > dst->clip_rect.h) {
        src_y1 -= (dst_y1 - dst->clip_rect.h) / scaling_h;
        dst_y1 = dst->clip_rect.h;
    }

    /* Translate back to surface coordinates */
    dst_x0 += dst->clip_rect.x;
    dst_x1 += dst->clip_rect.x;
    dst_y0 += dst->clip_rect.y;
    dst_y1 += dst->clip_rect.y;

    final_src.x = (int)SDL_round(src_x0);
    final_src.y = (int)SDL_round(src_y0);
    final_src.w = (int)SDL_round(src_x1 - src_x0);
    final_src.h = (int)SDL_round(src_y1 - src_y0);

    final_dst.x = (int)SDL_round(dst_x0);
    final_dst.y = (int)SDL_round(dst_y0);
    final_dst.w = (int)SDL_round(dst_x1 - dst_x0);
    final_dst.h = (int)SDL_round(dst_y1 - dst_y0);

    /* Clip again */
    {
        SDL_Rect tmp;
        tmp.x = 0;
        tmp.y = 0;
        tmp.w = src->w;
        tmp.h = src->h;
        SDL3_GetRectIntersection(&tmp, &final_src, &final_src);
    }

    /* Clip again */
    SDL3_GetRectIntersection(&dst->clip_rect, &final_dst, &final_dst);

    if (dstrect) {
        *dstrect = final_dst;
    }

    if (final_dst.w == 0 || final_dst.h == 0 ||
        final_src.w <= 0 || final_src.h <= 0) {
        /* No-op. */
        return 0;
    }

    return SDL_LowerBlitScaled(src, &final_src, dst, &final_dst);
}

SDL_DECLSPEC int SDLCALL
SDL_LowerBlitScaled(SDL2_Surface *src, SDL_Rect *srcrect, SDL2_Surface *dst, SDL_Rect *dstrect)
{
    return SDL3_BlitSurfaceUncheckedScaled(Surface2to3(src), srcrect, Surface2to3(dst), dstrect, SDL_SCALEMODE_NEAREST) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_SoftStretch(SDL2_Surface *src, const SDL_Rect *srcrect, SDL2_Surface *dst, const SDL_Rect *dstrect)
{
    return SDL3_BlitSurfaceScaled(Surface2to3(src), srcrect, Surface2to3(dst), dstrect, SDL_SCALEMODE_NEAREST) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_SoftStretchLinear(SDL2_Surface *src, const SDL_Rect *srcrect, SDL2_Surface *dst, const SDL_Rect *dstrect)
{
    return SDL3_BlitSurfaceScaled(Surface2to3(src), srcrect, Surface2to3(dst), dstrect, SDL_SCALEMODE_LINEAR) ? 0 : -1;
}

/* SDL_GetTicks is 64-bit in SDL3. Clamp it for SDL2. */
SDL_DECLSPEC Uint32 SDLCALL
SDL_GetTicks(void)
{
    return (Uint32)SDL3_GetTicks();
}

/* SDL_GetTicks64 is gone in SDL3 because SDL_GetTicks is 64-bit. */
SDL_DECLSPEC Uint64 SDLCALL
SDL_GetTicks64(void)
{
    return SDL3_GetTicks();
}

SDL_DECLSPEC SDL2_bool SDLCALL SDL_GetWindowWMInfo(SDL_Window *window, SDL_SysWMinfo *info)
{
    const char *driver = SDL3_GetCurrentVideoDriver();
    SDL_PropertiesID props;

    if (!driver) {
        return SDL2_FALSE;
    }
    if (!window) {
        SDL3_InvalidParamError("window");
        return SDL2_FALSE;
    }
    if (!info) {
        SDL3_InvalidParamError("info");
        return SDL2_FALSE;
    }

    props = SDL3_GetWindowProperties(window);

    if (SDL_strcmp(driver, "android") == 0) {
        info->subsystem = SDL2_SYSWM_ANDROID;
        info->info.android.window = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_WINDOW_POINTER, NULL);
        info->info.android.surface = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_ANDROID_SURFACE_POINTER, NULL);
    } else if (SDL_strcmp(driver, "cocoa") == 0) {
        info->subsystem = SDL2_SYSWM_COCOA;
        info->info.cocoa.window = (NSWindow *)SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, NULL);
    } else if (SDL_strcmp(driver, "kmsdrm") == 0) {
        info->subsystem = SDL2_SYSWM_KMSDRM;
        info->info.kmsdrm.dev_index = (int)SDL3_GetNumberProperty(props, SDL_PROP_WINDOW_KMSDRM_DEVICE_INDEX_NUMBER, 0);
        info->info.kmsdrm.drm_fd = (int)SDL3_GetNumberProperty(props, SDL_PROP_WINDOW_KMSDRM_DRM_FD_NUMBER, -1);
        info->info.kmsdrm.gbm_dev = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_KMSDRM_GBM_DEVICE_POINTER, NULL);
    } else if (SDL_strcmp(driver, "uikit") == 0) {
        info->subsystem = SDL2_SYSWM_UIKIT;
        info->info.uikit.window = (UIWindow *)SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, NULL);
        info->info.uikit.colorbuffer = 0;
        info->info.uikit.framebuffer = 0;
        info->info.uikit.resolveFramebuffer = 0;
    } else if (SDL_strcmp(driver, "vivante") == 0) {
        info->subsystem = SDL2_SYSWM_VIVANTE;
        info->info.vivante.display = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_VIVANTE_DISPLAY_POINTER, NULL);
        info->info.vivante.window = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_VIVANTE_WINDOW_POINTER, NULL);
    } else if (SDL_strcmp(driver, "wayland") == 0) {
        Uint32 version2 = SDL_VERSIONNUM((Uint32)info->version.major,
                                         (Uint32)info->version.minor,
                                         (Uint32)info->version.patch);

        /* Before 2.0.6, it was possible to build an SDL with Wayland support
         * (SDL_SysWMinfo will be large enough to hold Wayland info), but build
         * your app against SDL headers that didn't have Wayland support
         * (SDL_SysWMinfo could be smaller than Wayland needs. This would lead
         * to an app properly using SDL_GetWindowWMInfo() but we'd accidentally
         * overflow memory on the stack or heap. To protect against this, we've
         * padded out the struct unconditionally in the headers and Wayland will
         * just return an error for older apps using this function. Those apps
         * will need to be recompiled against newer headers or not use Wayland,
         * maybe by forcing SDL_VIDEODRIVER=x11.
         */
        if (version2 < SDL_VERSIONNUM(2, 0, 6)) {
            info->subsystem = SDL2_SYSWM_UNKNOWN;
            SDL3_SetError("Version must be 2.0.6 or newer");
            return SDL2_FALSE;
        }

        info->subsystem = SDL2_SYSWM_WAYLAND;
        info->info.wl.display = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, NULL);
        info->info.wl.surface = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, NULL);
        info->info.wl.shell_surface = NULL; /* Deprecated */

        if (version2 >= SDL_VERSIONNUM(2, 0, 15)) {
            info->info.wl.egl_window = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_EGL_WINDOW_POINTER, NULL);
            info->info.wl.xdg_surface = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_XDG_SURFACE_POINTER, NULL);
            if (version2 >= SDL_VERSIONNUM(2, 0, 17)) {
                info->info.wl.xdg_toplevel = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_XDG_TOPLEVEL_POINTER, NULL);
                if (version2 >= SDL_VERSIONNUM(2, 0, 22)) {
                    info->info.wl.xdg_popup = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_XDG_POPUP_POINTER, NULL);
                    info->info.wl.xdg_positioner = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_XDG_POSITIONER_POINTER, NULL);
                }
            }
        }
    } else if (SDL_strcmp(driver, "windows") == 0) {
        info->subsystem = SDL2_SYSWM_WINDOWS;
        info->info.win.window = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);
        info->info.win.hdc = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HDC_POINTER, NULL);
        info->info.win.hinstance = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_INSTANCE_POINTER, NULL);
    } else if (SDL_strcmp(driver, "x11") == 0) {
        info->subsystem = SDL2_SYSWM_X11;
        info->info.x11.display = SDL3_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, NULL);
        info->info.x11.window = (unsigned long)SDL3_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
    } else {
        SDL3_SetError("Video driver '%s' has no mapping to SDL_SysWMinfo", driver);
        info->subsystem = SDL2_SYSWM_UNKNOWN;
        return SDL2_FALSE;
    }

    return SDL2_TRUE;
}


SDL_DECLSPEC int SDLCALL
SDL_GameControllerSetSensorEnabled(SDL_GameController *gamecontroller, SDL_SensorType type, SDL2_bool enabled)
{
    return SDL3_SetGamepadSensorEnabled(gamecontroller, type, enabled) ? 0 : -1;
}

/* this API was removed in SDL3; use sensor event timestamps instead! */
SDL_DECLSPEC int SDLCALL
SDL_GameControllerGetSensorDataWithTimestamp(SDL_GameController *gamecontroller, SDL_SensorType type, Uint64 *timestamp, float *data, int num_values)
{
    SDL3_Unsupported();  /* !!! FIXME: maybe try to track this from SDL3 events if something needs this? I can't imagine this was widely used. */
    return -1;
}

/* this API was removed in SDL3; use sensor event timestamps instead! */
SDL_DECLSPEC int SDLCALL
SDL_SensorGetDataWithTimestamp(SDL_Sensor *sensor, Uint64 *timestamp, float *data, int num_values)
{
    SDL3_Unsupported();  /* !!! FIXME: maybe try to track this from SDL3 events if something needs this? I can't imagine this was widely used. */
    return -1;
}

SDL_DECLSPEC int SDLCALL
SDL_GetNumTouchDevices(void)
{
    SDL3_free(TouchDevices);
    TouchDevices = SDL3_GetTouchDevices(&NumTouchDevices);
    return NumTouchDevices;
}

SDL_DECLSPEC SDL_TouchID SDLCALL
SDL_GetTouchDevice(int idx)
{
    if ((idx < 0) || (idx >= NumTouchDevices)) {
        SDL3_SetError("Unknown touch device index %d", idx);
        return 0;
    }
    return TouchDevices[idx];
}

SDL_DECLSPEC const char* SDLCALL
SDL_GetTouchName(int idx)
{
    SDL_TouchID tid = SDL_GetTouchDevice(idx);
    return tid ? SDL3_GetTouchDeviceName(tid) : NULL;
}

SDL_DECLSPEC int SDLCALL
SDL_GetNumTouchFingers(SDL_TouchID touchID)
{
    SDL3_free(TouchFingers);
    TouchFingersDeviceID = touchID;
    TouchFingers = SDL3_GetTouchFingers(touchID, &NumTouchFingers);
    return NumTouchFingers;
}

SDL_DECLSPEC SDL_Finger * SDLCALL
SDL_GetTouchFinger(SDL_TouchID touchID, int idx)
{
    if (touchID != TouchFingersDeviceID) {
        SDL_GetNumTouchFingers(touchID);
    }
    if ((idx < 0) || (idx >= NumTouchFingers)) {
        SDL3_SetError("Unknown touch finger");
        return NULL;
    }
    return TouchFingers[idx];
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
    Sint64 hash;
} GestureDollarTemplate;

typedef struct
{
    SDL_TouchID touchId;
    SDL_FPoint centroid;
    GestureDollarPath dollarPath;
    Uint16 numDownFingers;
    int numDollarTemplates;
    GestureDollarTemplate *dollarTemplate;
    SDL2_bool recording;
} GestureTouch;

static GestureTouch *GestureTouches = NULL;
static int GestureNumTouches = 0;
static bool GestureRecordAll = false;

static GestureTouch *
GestureAddTouch(const SDL_TouchID touchId)
{
    GestureTouch *gestureTouch = (GestureTouch *)SDL3_realloc(GestureTouches, (GestureNumTouches + 1) * sizeof(GestureTouch));
    if (gestureTouch == NULL) {
        return NULL;
    }

    GestureTouches = gestureTouch;
    SDL3_zero(GestureTouches[GestureNumTouches]);
    GestureTouches[GestureNumTouches].touchId = touchId;
    return &GestureTouches[GestureNumTouches++];
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

SDL_DECLSPEC int SDLCALL
SDL_RecordGesture(SDL_TouchID touchId)
{
    int numtouchdevs = 0;
    SDL_TouchID *touchdevs = SDL3_GetTouchDevices(&numtouchdevs);
    int i;

    /* make sure we know about all the devices SDL3 knows about, since we aren't connected as tightly as we were in SDL2. */
    for (i = 0; i < numtouchdevs; i++) {
        SDL_TouchID thistouch = touchdevs[i];
        if (!GestureGetTouch(thistouch)) {
            if (!GestureAddTouch(thistouch)) {
                SDL_free(touchdevs);
                return 0;  /* uhoh, out of memory */
            }
        }
    }
    SDL_free(touchdevs);

    if (touchId == (SDL_TouchID)-1) {
        GestureRecordAll = true;  /* !!! FIXME: this is never set back to SDL2_FALSE anywhere, that's probably a bug. */
        for (i = 0; i < GestureNumTouches; i++) {
            GestureTouches[i].recording = SDL2_TRUE;
        }
    } else {
        GestureTouch *touch = GestureGetTouch(touchId);
        if (!touch) {
            return 0;  /* bogus touchid */
        }
        touch->recording = SDL2_TRUE;
    }

    return 1;
}

static void
GestureQuit(void)
{
    int i;
    for (i = 0; i < GestureNumTouches; i++) {
        SDL3_free(GestureTouches[i].dollarTemplate);
    }
    SDL3_free(GestureTouches);
    GestureTouches = NULL;
    GestureNumTouches = 0;
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

SDL_DECLSPEC int SDLCALL
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

SDL_DECLSPEC int SDLCALL
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
    SDL3_SetError("Unknown gestureId");
    return -1;
}

/* path is an already sampled set of points
Returns the index of the gesture on success, or -1 */
static int
GestureAddDollar_one(GestureTouch *inTouch, SDL_FPoint *path)
{
    GestureDollarTemplate *dollarTemplate;
    GestureDollarTemplate *templ;
    int idx;

    idx = inTouch->numDollarTemplates;
    dollarTemplate = (GestureDollarTemplate *)SDL3_realloc(inTouch->dollarTemplate, (idx + 1) * sizeof(GestureDollarTemplate));
    if (dollarTemplate == NULL) {
        return -1;
    }
    inTouch->dollarTemplate = dollarTemplate;

    templ = &inTouch->dollarTemplate[idx];
    SDL3_memcpy(templ->path, path, GESTURE_DOLLARNPOINTS * sizeof(SDL_FPoint));
    templ->hash = GestureHashDollar(templ->path);
    inTouch->numDollarTemplates++;

    return idx;
}

static int
GestureAddDollar(GestureTouch *inTouch, SDL_FPoint *path)
{
    if (inTouch == NULL) {
        int i, idx;
        if (GestureNumTouches == 0) {
            SDL3_SetError("no gesture touch devices registered");
            return -1;
        }
        for (i = 0, idx = -1; i < GestureNumTouches; i++) {
            inTouch = &GestureTouches[i];
            idx = GestureAddDollar_one(inTouch, path);
            if (idx < 0) {
                return -1;
            }
        }
        /* Use the index of the last one added. */
        return idx;
    }
    return GestureAddDollar_one(inTouch, path);
}

SDL_DECLSPEC int SDLCALL
SDL_LoadDollarTemplates(SDL_TouchID touchId, SDL2_RWops *src)
{
    int i, loaded = 0;
    GestureTouch *touch = NULL;
    if (src == NULL) {
        return 0;
    }
    if (touchId != (SDL_TouchID)-1) {
        for (i = 0; i < GestureNumTouches; i++) {
            if (GestureTouches[i].touchId == touchId) {
                touch = &GestureTouches[i];
            }
        }
        if (touch == NULL) {
            SDL3_SetError("given touch id not found");
            return -1;
        }
    }

    while (1) {
        GestureDollarTemplate templ;

        if (SDL_RWread(src, templ.path, sizeof(templ.path[0]), GESTURE_DOLLARNPOINTS) < GESTURE_DOLLARNPOINTS) {
            if (loaded == 0) {
                SDL3_SetError("could not read any dollar gesture from rwops");
                return -1;
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

        if (touchId != (SDL_TouchID)-1) {
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
GestureDollarNormalize(const GestureDollarPath *path, SDL_FPoint *points, SDL2_bool is_recording)
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

    GestureDollarNormalize(path, points, SDL2_FALSE);

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
    int i, idx;
    float pathDx, pathDy;
    SDL_FPoint lastP;
    SDL_FPoint lastCentroid;
    float lDist;
    float Dist;
    float dtheta;
    float dDist;

    if (event3->type == SDL_EVENT_FINGER_MOTION || event3->type == SDL_EVENT_FINGER_DOWN || event3->type == SDL_EVENT_FINGER_UP) {
        GestureTouch *inTouch = GestureGetTouch(event3->tfinger.touchID);

        if (inTouch == NULL) {  /* we maybe didn't see this one before. */
            inTouch = GestureAddTouch(event3->tfinger.touchID);
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
                inTouch->recording = SDL2_FALSE;
                GestureDollarNormalize(&inTouch->dollarPath, path, SDL2_TRUE);
                /* PrintPath(path); */
                if (GestureRecordAll) {
                    idx = GestureAddDollar(NULL, path);
                    for (i = 0; i < GestureNumTouches; i++) {
                        GestureTouches[i].recording = SDL2_FALSE;
                    }
                } else {
                    idx = GestureAddDollar(inTouch, path);
                }

                if (idx >= 0) {
                    GestureSendDollarRecord(inTouch, inTouch->dollarTemplate[idx].hash);
                } else {
                    GestureSendDollarRecord(inTouch, -1);
                }
            } else {
                int bestTempl = -1;
                const float error = GestureDollarRecognize(&inTouch->dollarPath, &bestTempl, inTouch);
                if (bestTempl >= 0) {
                    /* Send Event */
                    Sint64 gestureId = inTouch->dollarTemplate[bestTempl].hash;
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


/* REQUIRES that bitmap point to a w-by-h bitmap with ppb pixels-per-byte. */
static void SDL_CalculateShapeBitmap(SDL_WindowShapeMode mode, SDL2_Surface *shape, Uint32 *pixels, int pitch)
{
    int x = 0;
    int y = 0;
    Uint8 r = 0, g = 0, b = 0, alpha = 0;
    Uint8 *pixel = NULL;
    Uint32 pixel_value = 0, mask_value = 0;
    SDL_Color key;

    for (y = 0; y < shape->h; y++) {
        for (x = 0; x < shape->w; x++) {
            alpha = 0;
            pixel_value = 0;
            pixel = (Uint8 *)(shape->pixels) + (y * shape->pitch) + (x * shape->format->BytesPerPixel);
            switch (shape->format->BytesPerPixel) {
            case 1:
                pixel_value = *pixel;
                break;
            case 2:
                pixel_value = *(Uint16 *)pixel;
                break;
            case 3:
                pixel_value = *(Uint32 *)pixel & (~shape->format->Amask);
                break;
            case 4:
                pixel_value = *(Uint32 *)pixel;
                break;
            }
            SDL_GetRGBA(pixel_value, shape->format, &r, &g, &b, &alpha);
            switch (mode.mode) {
            case (ShapeModeDefault):
                mask_value = (alpha >= 1 ? 0xFFFFFFFF : 0);
                break;
            case (ShapeModeBinarizeAlpha):
                mask_value = (alpha >= mode.parameters.binarizationCutoff ? 0xFFFFFFFF : 0);
                break;
            case (ShapeModeReverseBinarizeAlpha):
                mask_value = (alpha <= mode.parameters.binarizationCutoff ? 0xFFFFFFFF : 0);
                break;
            case (ShapeModeColorKey):
                key = mode.parameters.colorKey;
                mask_value = ((key.r != r || key.g != g || key.b != b) ? 0xFFFFFFFF : 0);
                break;
            }
            pixels[x] = mask_value;
        }
        pixels = (Uint32 *)((Uint8 *)pixels + pitch);
    }
}


SDL_DECLSPEC SDL_Window * SDLCALL
SDL_CreateShapedWindow(const char *title, unsigned int x, unsigned int y, unsigned int w, unsigned int h, Uint32 flags)
{
    flags |= (SDL_WINDOW_HIDDEN | SDL_WINDOW_TRANSPARENT);

    return SDL_CreateWindow(title, (int)x, (int)y, (int)w, (int)h, flags);
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_IsShapedWindow(const SDL_Window *window)
{
    if (SDL3_GetWindowFlags((SDL_Window *)window) & SDL_WINDOW_TRANSPARENT) {
        return SDL2_TRUE;
    }
    return SDL2_FALSE;
}

#define PROP_WINDOW_SHAPE_MODE_POINTER "sdl2-compat.window.shape_mode"

static void SDLCALL CleanupFreeableProperty(void *userdata, void *value)
{
    SDL3_free(value);
}

SDL_DECLSPEC int SDLCALL
SDL_SetWindowShape(SDL_Window *window, SDL2_Surface *shape, SDL_WindowShapeMode *shape_mode)
{
    SDL_Surface *surface;
    int result;

    if (!SDL_IsShapedWindow(window)) {
        return SDL_NONSHAPEABLE_WINDOW;
    }

    if (shape == NULL) {
        return SDL_INVALID_SHAPE_ARGUMENT;
    }

    if (shape_mode == NULL) {
        return SDL_INVALID_SHAPE_ARGUMENT;
    }

    surface = SDL3_CreateSurface(shape->w, shape->h, SDL_PIXELFORMAT_ARGB32);
    if (!surface) {
        return -1;
    }

    SDL_CalculateShapeBitmap(*shape_mode, shape, (Uint32 *)surface->pixels, surface->pitch);

    result = SDL3_SetWindowShape(window, surface) ? 0 : -1;
    if (result == 0) {
        SDL_WindowShapeMode *property = (SDL_WindowShapeMode *)SDL3_malloc(sizeof(*property));
        if (property) {
            SDL3_copyp(property, shape_mode);
            result = SDL3_SetPointerPropertyWithCleanup(SDL3_GetWindowProperties(window), PROP_WINDOW_SHAPE_MODE_POINTER, property, CleanupFreeableProperty, NULL) ? 0 : -1;
        } else {
            result = -1;
        }
    }

    SDL3_DestroySurface(surface);

    if (result == 0) {
        SDL3_ShowWindow(window);
    }

    return result;
}

SDL_DECLSPEC int SDLCALL
SDL_GetShapedWindowMode(SDL_Window *window, SDL_WindowShapeMode *shape_mode)
{
    SDL_WindowShapeMode *property = (SDL_WindowShapeMode *)SDL3_GetPointerProperty(SDL3_GetWindowProperties(window), PROP_WINDOW_SHAPE_MODE_POINTER, NULL);
    if (!property) {
        return SDL_NONSHAPEABLE_WINDOW;
    }

    if (shape_mode) {
        SDL3_copyp(shape_mode, property);
    }
    return 0;
}


SDL_DECLSPEC int SDLCALL
SDL_GetRendererInfo(SDL_Renderer *renderer, SDL2_RendererInfo *info)
{
    const char *name;
    SDL_PropertiesID props;
    unsigned int i;
    const SDL_PixelFormat *formats;

    if (!info) {
        SDL3_InvalidParamError("info");
        return -1;
    }

    name = SDL3_GetRendererName(renderer);
    if (!name) {
        return -1;
    }

    SDL3_zerop(info);

    props = SDL3_GetRendererProperties(renderer);

    info->name = name;
    info->max_texture_width = (int)SDL3_GetNumberProperty(props, SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 0);
    info->max_texture_height = info->max_texture_width;

    if (SDL3_strcmp(info->name, SDL_SOFTWARE_RENDERER) == 0) {
        info->flags = SDL2_RENDERER_SOFTWARE | SDL2_RENDERER_TARGETTEXTURE;
    } else {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_TARGETTEXTURE;
    }
    if (SDL3_GetNumberProperty(props, SDL_PROP_RENDERER_VSYNC_NUMBER, 0)) {
        info->flags |= SDL2_RENDERER_PRESENTVSYNC;
    }

    formats = (const SDL_PixelFormat *)SDL3_GetPointerProperty(props, SDL_PROP_RENDERER_TEXTURE_FORMATS_POINTER, NULL);
    for (i = 0; i < 16 && formats[i]; ++i) {
        info->texture_formats[i] = formats[i];
    }
    info->num_texture_formats = i;

    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_GetRenderDriverInfo(int idx, SDL2_RendererInfo *info)
{
    const char *name = SDL3_GetRenderDriver(idx);
    if (!name) {
        return -1;  /* assume SDL3_GetRenderDriver set the SDL error. */
    }

    SDL3_zerop(info);
    info->name = name;

    /* these are the values that SDL2 returns. */
    if ((SDL3_strcmp(name, "opengl") == 0) || (SDL3_strcmp(name, "opengles2") == 0)) {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 4;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_ABGR8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_XRGB8888;
        info->texture_formats[3] = SDL_PIXELFORMAT_XBGR8888;
    } else if (SDL3_strcmp(name, "opengles") == 0) {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ABGR8888;
    } else if (SDL3_strcmp(name, "direct3d") == 0) {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
    } else if (SDL3_strcmp(name, "direct3d11") == 0) {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_XRGB8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
    } else if (SDL3_strcmp(name, "direct3d12") == 0) {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_XRGB8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
        info->max_texture_width = 16384;
        info->max_texture_height = 16384;
    } else if (SDL3_strcmp(name, "metal") == 0) {
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 6;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
        info->texture_formats[1] = SDL_PIXELFORMAT_ABGR8888;
        info->texture_formats[2] = SDL_PIXELFORMAT_YV12;
        info->texture_formats[3] = SDL_PIXELFORMAT_IYUV;
        info->texture_formats[4] = SDL_PIXELFORMAT_NV12;
        info->texture_formats[5] = SDL_PIXELFORMAT_NV21;
    } else if (SDL3_strcmp(name, "software") == 0) {
        info->flags = SDL2_RENDERER_SOFTWARE | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
    } else {  /* this seems reasonable if something currently-unknown shows up in SDL3. */
        info->flags = SDL2_RENDERER_ACCELERATED | SDL2_RENDERER_PRESENTVSYNC | SDL2_RENDERER_TARGETTEXTURE;
        info->num_texture_formats = 1;
        info->texture_formats[0] = SDL_PIXELFORMAT_ARGB8888;
    }
    return 0;
}

#define PROP_RENDERER_BATCHING "sdl2-compat.renderer.batching"

static int FlushRendererIfNotBatching(SDL_Renderer *renderer)
{
    const SDL_PropertiesID props = SDL3_GetRendererProperties(renderer);
    if (!SDL3_GetBooleanProperty(props, PROP_RENDERER_BATCHING, false)) {
        return SDL3_FlushRenderer(renderer) ? 0 : -1;
    }
    return 0;
}

/* Second parameter changed from an index to a string in SDL3. */
SDL_DECLSPEC SDL_Renderer *SDLCALL
SDL_CreateRenderer(SDL_Window *window, int idx, Uint32 flags)
{
    SDL_PropertiesID props;
    SDL_Renderer *renderer;
    const char *name = NULL;

    if (idx != -1) {
        name = SDL3_GetRenderDriver(idx);
        if (!name) {
            return NULL;  /* assume SDL3_GetRenderDriver set the SDL error. */
        }
    }

    if (flags & SDL2_RENDERER_ACCELERATED) {
        if (name && SDL3_strcmp(name, SDL_SOFTWARE_RENDERER) == 0) {
            SDL3_SetError("Couldn't find matching render driver");
            return NULL;
        }
    }

    if (flags & SDL2_RENDERER_SOFTWARE) {
        if (name && SDL3_strcmp(name, SDL_SOFTWARE_RENDERER) != 0) {
            SDL3_SetError("Couldn't find matching render driver");
            return NULL;
        }
        name = SDL_SOFTWARE_RENDERER;
    }

    renderer = SDL3_CreateRenderer(window, name);
    props = SDL3_GetRendererProperties(renderer);
    if (props) {
        SDL3_SetBooleanProperty(props, PROP_RENDERER_BATCHING, SDL2Compat_GetHintBoolean("SDL_RENDER_BATCHING", (name == NULL)));
    }
    if (flags & SDL2_RENDERER_PRESENTVSYNC) {
        SDL3_SetRenderVSync(renderer, 1);
    }

    return renderer;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_RenderTargetSupported(SDL_Renderer *renderer)
{
    /* All SDL3 renderers support target textures */
    return SDL2_TRUE;
}

SDL_DECLSPEC void SDLCALL
SDL_RenderGetViewport(SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL3_GetRenderViewport(renderer, rect);
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_SetClipRect(SDL2_Surface *surface, const SDL_Rect *rect)
{
    SDL_Rect full_rect;

    /* Don't do anything if there's no surface to act on */
    if (!surface) {
        return SDL2_FALSE;
    }

    SDL3_SetSurfaceClipRect(Surface2to3(surface), rect);

    /* Set up the full surface rectangle */
    full_rect.x = 0;
    full_rect.y = 0;
    full_rect.w = surface->w;
    full_rect.h = surface->h;

    /* Set the clipping rectangle */
    if (!rect) {
        surface->clip_rect = full_rect;
        return SDL2_TRUE;
    }
    return SDL3_GetRectIntersection(rect, &full_rect, &surface->clip_rect) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC void SDLCALL
SDL_RenderGetClipRect(SDL_Renderer *renderer, SDL_Rect *rect)
{
    SDL3_GetRenderClipRect(renderer, rect);
}

SDL_DECLSPEC void SDLCALL
SDL_RenderGetScale(SDL_Renderer *renderer, float *scaleX, float *scaleY)
{
    SDL3_GetRenderScale(renderer, scaleX, scaleY);
}

SDL_DECLSPEC void SDLCALL
SDL_RenderWindowToLogical(SDL_Renderer *renderer,
                          int windowX, int windowY,
                          float *logicalX, float *logicalY)
{
    SDL3_RenderCoordinatesFromWindow(renderer, (float)windowX, (float)windowY, logicalX, logicalY);
}

SDL_DECLSPEC void SDLCALL
SDL_RenderLogicalToWindow(SDL_Renderer *renderer,
                          float logicalX, float logicalY,
                          int *windowX, int *windowY)
{
    float x, y;
    SDL3_RenderCoordinatesToWindow(renderer, logicalX, logicalY, &x, &y);
    if (windowX) *windowX = (int)x;
    if (windowY) *windowY = (int)y;
}

SDL_DECLSPEC int SDLCALL
SDL_RenderSetLogicalSize(SDL_Renderer *renderer, int w, int h)
{
    int retval;
    if (w == 0 && h == 0) {
        retval = SDL3_SetRenderLogicalPresentation(renderer, 0, 0, SDL_LOGICAL_PRESENTATION_DISABLED) ? 0 : -1;
    } else {
        retval = SDL3_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_LETTERBOX) ? 0 : -1;
    }
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC void SDLCALL
SDL_RenderGetLogicalSize(SDL_Renderer *renderer, int *w, int *h)
{
    SDL3_GetRenderLogicalPresentation(renderer, w, h, NULL);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderSetIntegerScale(SDL_Renderer *renderer, SDL2_bool enable)
{
    SDL_RendererLogicalPresentation mode;
    int w, h;
    int retval;

    retval = SDL3_GetRenderLogicalPresentation(renderer, &w, &h, &mode) ? 0 : -1;
    if (retval < 0) {
        return retval;
    }

    if (enable && mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
        return 0;
    }

    if (!enable && mode != SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
        return 0;
    }

    if (enable) {
        retval = SDL3_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) ? 0 : -1;
    } else {
        retval = SDL3_SetRenderLogicalPresentation(renderer, w, h, SDL_LOGICAL_PRESENTATION_DISABLED) ? 0 : -1;
    }
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_RenderGetIntegerScale(SDL_Renderer *renderer)
{
    SDL_RendererLogicalPresentation mode;
    if (SDL3_GetRenderLogicalPresentation(renderer, NULL, NULL, &mode)) {
        if (mode == SDL_LOGICAL_PRESENTATION_INTEGER_SCALE) {
            return SDL2_TRUE;
        }
    }
    return SDL2_FALSE;
}

SDL_DECLSPEC int SDLCALL
SDL_RenderSetViewport(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    const int retval = SDL3_SetRenderViewport(renderer, rect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderSetClipRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    const int retval = SDL3_SetRenderClipRect(renderer, rect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderClear(SDL_Renderer *renderer)
{
    const int retval = SDL3_RenderClear(renderer) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawPointF(SDL_Renderer *renderer, float x, float y)
{
    int retval;
    SDL_FPoint fpoint;
    fpoint.x = x;
    fpoint.y = y;
    retval = SDL3_RenderPoints(renderer, &fpoint, 1) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawPoint(SDL_Renderer *renderer, int x, int y)
{
    return SDL_RenderDrawPointF(renderer, (float)x, (float)y);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawPoints(SDL_Renderer *renderer,
                     const SDL_Point *points, int count)
{
    SDL_FPoint *fpoints;
    int i;
    int retval;

    if (points == NULL) {
        SDL3_InvalidParamError("points");
        return -1;
    }

    fpoints = (SDL_FPoint *) SDL3_malloc(sizeof (SDL_FPoint) * count);
    if (fpoints == NULL) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        fpoints[i].x = (float)points[i].x;
        fpoints[i].y = (float)points[i].y;
    }

    retval = SDL3_RenderPoints(renderer, fpoints, count) ? 0 : -1;

    SDL3_free(fpoints);

    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawPointsF(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    const int retval = SDL3_RenderPoints(renderer, points, count) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawLineF(SDL_Renderer *renderer, float x1, float y1, float x2, float y2)
{
    int retval;
    SDL_FPoint points[2];
    points[0].x = (float)x1;
    points[0].y = (float)y1;
    points[1].x = (float)x2;
    points[1].y = (float)y2;
    retval = SDL3_RenderLines(renderer, points, 2) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2)
{
    return SDL_RenderDrawLineF(renderer, (float) x1, (float) y1, (float) x2, (float) y2);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawLines(SDL_Renderer *renderer, const SDL_Point *points, int count)
{
    SDL_FPoint *fpoints;
    int i;
    int retval;

    if (points == NULL) {
        SDL3_InvalidParamError("points");
        return -1;
    }
    if (count < 2) {
        return 0;
    }

    fpoints = (SDL_FPoint *) SDL3_malloc(sizeof (SDL_FPoint) * count);
    if (fpoints == NULL) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        fpoints[i].x = (float)points[i].x;
        fpoints[i].y = (float)points[i].y;
    }

    retval = SDL3_RenderLines(renderer, fpoints, count) ? 0 : -1;

    SDL3_free(fpoints);

    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawLinesF(SDL_Renderer *renderer, const SDL_FPoint *points, int count)
{
    const int retval = SDL3_RenderLines(renderer, points, count) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    int retval;
    SDL_FRect frect;
    SDL_FRect *prect = NULL;

    if (rect) {
        frect.x = (float)rect->x;
        frect.y = (float)rect->y;
        frect.w = (float)rect->w;
        frect.h = (float)rect->h;
        prect = &frect;
    }

    retval = SDL3_RenderRect(renderer, prect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawRects(SDL_Renderer *renderer, const SDL_Rect *rects, int count)
{
    int i;

    if (rects == NULL) {
        SDL3_InvalidParamError("rects");
        return -1;
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

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawRectF(SDL_Renderer *renderer, const SDL_FRect *rect)
{
    const int retval = SDL3_RenderRect(renderer, rect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderDrawRectsF(SDL_Renderer *renderer, const SDL_FRect *rects, int count)
{
    const int retval = SDL3_RenderRects(renderer, rects, count) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderFillRect(SDL_Renderer *renderer, const SDL_Rect *rect)
{
    int retval;
    SDL_FRect frect;
    if (rect) {
        frect.x = (float)rect->x;
        frect.y = (float)rect->y;
        frect.w = (float)rect->w;
        frect.h = (float)rect->h;
        return SDL3_RenderFillRect(renderer, &frect) ? 0 : -1;
    }
    retval = SDL3_RenderFillRect(renderer, NULL) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderFillRects(SDL_Renderer *renderer, const SDL_Rect *rects, int count)
{
    SDL_FRect *frects;
    int i;
    int retval;

    if (rects == NULL) {
        SDL3_InvalidParamError("rects");
        return -1;
    }
    if (count < 1) {
        return 0;
    }

    frects = (SDL_FRect *) SDL3_malloc(sizeof (SDL_FRect) * count);
    if (frects == NULL) {
        return -1;
    }

    for (i = 0; i < count; ++i) {
        frects[i].x = (float)rects[i].x;
        frects[i].y = (float)rects[i].y;
        frects[i].w = (float)rects[i].w;
        frects[i].h = (float)rects[i].h;
    }

    retval = SDL3_RenderFillRects(renderer, frects, count) ? 0 : -1;

    SDL3_free(frects);

    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderFillRectF(SDL_Renderer *renderer, const SDL_FRect *rect)
{
    const int retval = SDL3_RenderFillRect(renderer, rect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderFillRectsF(SDL_Renderer *renderer, const SDL_FRect *rects, int count)
{
    const int retval = SDL3_RenderFillRects(renderer, rects, count) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderCopy(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_Rect *dstrect)
{
    int retval;
    SDL_FRect srcfrect;
    SDL_FRect *psrcfrect = NULL;
    SDL_FRect dstfrect;
    SDL_FRect *pdstfrect = NULL;
    if (srcrect) {
        srcfrect.x = (float)srcrect->x;
        srcfrect.y = (float)srcrect->y;
        srcfrect.w = (float)srcrect->w;
        srcfrect.h = (float)srcrect->h;
        psrcfrect = &srcfrect;
    }
    if (dstrect) {
        dstfrect.x = (float)dstrect->x;
        dstfrect.y = (float)dstrect->y;
        dstfrect.w = (float)dstrect->w;
        dstfrect.h = (float)dstrect->h;
        pdstfrect = &dstfrect;
    }
    retval = SDL3_RenderTexture(renderer, texture, psrcfrect, pdstfrect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderCopyF(SDL_Renderer *renderer, SDL_Texture *texture, const SDL_Rect *srcrect, const SDL_FRect *dstrect)
{
    int retval;
    SDL_FRect srcfrect;
    SDL_FRect *psrcfrect = NULL;
    if (srcrect) {
        srcfrect.x = (float)srcrect->x;
        srcfrect.y = (float)srcrect->y;
        srcfrect.w = (float)srcrect->w;
        srcfrect.h = (float)srcrect->h;
        psrcfrect = &srcfrect;
    }
    retval = SDL3_RenderTexture(renderer, texture, psrcfrect, dstrect) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderCopyEx(SDL_Renderer *renderer, SDL_Texture *texture,
                 const SDL_Rect *srcrect, const SDL_Rect *dstrect,
                 const double angle, const SDL_Point *center, const SDL_FlipMode flip)
{
    int retval;
    SDL_FRect srcfrect;
    SDL_FRect *psrcfrect = NULL;
    SDL_FRect dstfrect;
    SDL_FRect *pdstfrect = NULL;
    SDL_FPoint fcenter;
    SDL_FPoint *pfcenter = NULL;

    if (srcrect) {
        srcfrect.x = (float)srcrect->x;
        srcfrect.y = (float)srcrect->y;
        srcfrect.w = (float)srcrect->w;
        srcfrect.h = (float)srcrect->h;
        psrcfrect = &srcfrect;
    }

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

    retval = SDL3_RenderTextureRotated(renderer, texture, psrcfrect, pdstfrect, angle, pfcenter, flip) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderCopyExF(SDL_Renderer *renderer, SDL_Texture *texture,
                  const SDL_Rect *srcrect, const SDL_FRect *dstrect,
                  const double angle, const SDL_FPoint *center, const SDL_FlipMode flip)
{
    int retval;
    SDL_FRect srcfrect;
    SDL_FRect *psrcfrect = NULL;

    if (srcrect) {
        srcfrect.x = (float)srcrect->x;
        srcfrect.y = (float)srcrect->y;
        srcfrect.w = (float)srcrect->w;
        srcfrect.h = (float)srcrect->h;
        psrcfrect = &srcfrect;
    }

    retval = SDL3_RenderTextureRotated(renderer, texture, psrcfrect, dstrect, angle, center, flip) ? 0 : -1;
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderGeometry(SDL_Renderer *renderer, SDL_Texture *texture, const SDL2_Vertex *vertices, int num_vertices, const int *indices, int num_indices)
{
    if (vertices) {
        const float *xy = &vertices->position.x;
        int xy_stride = sizeof(SDL2_Vertex);
        const SDL_Color *color = &vertices->color;
        int color_stride = sizeof(SDL2_Vertex);
        const float *uv = &vertices->tex_coord.x;
        int uv_stride = sizeof(SDL2_Vertex);
        int size_indices = 4;
        return SDL_RenderGeometryRaw(renderer, texture, xy, xy_stride, color, color_stride, uv, uv_stride, num_vertices, indices, num_indices, size_indices);
    }
    SDL3_InvalidParamError("vertices");
    return -1;
}

SDL_DECLSPEC int SDLCALL
SDL_RenderGeometryRaw(SDL_Renderer *renderer, SDL_Texture *texture, const float *xy, int xy_stride, const SDL_Color *color, int color_stride, const float *uv, int uv_stride, int num_vertices, const void *indices, int num_indices, int size_indices)
{
    int i, retval, isstack;
    const char *color2 = (const char *) color;
    SDL_FColor *color3;

    if (num_vertices <= 0) {
        SDL3_InvalidParamError("num_vertices");
        return -1;
    }
    if (!color) {
        SDL3_InvalidParamError("color");
        return -1;
    }

    color3 = (SDL_FColor *) SDL3_small_alloc(SDL_FColor, num_vertices, &isstack);
    if (!color3) {
        SDL3_OutOfMemory();
        return -1;
    }
    for (i = 0; i < num_vertices; ++i) {
        color3[i].r = color->r / 255.0f;
        color3[i].g = color->g / 255.0f;
        color3[i].b = color->b / 255.0f;
        color3[i].a = color->a / 255.0f;
        color2 += color_stride;
        color = (const SDL_Color *) color2;
    }

    color_stride = sizeof(SDL_FColor);
    retval = SDL3_RenderGeometryRaw(renderer, texture, xy, xy_stride, color3, color_stride, uv, uv_stride, num_vertices, indices, num_indices, size_indices) ? 0 : -1;
    SDL3_small_free(color3, isstack);
    return retval < 0 ? retval : FlushRendererIfNotBatching(renderer);
}

SDL_DECLSPEC int SDLCALL
SDL_RenderReadPixels(SDL_Renderer * renderer, const SDL_Rect * rect, Uint32 format, void *pixels, int pitch)
{
    int result = -1;

    SDL_Surface *surface = SDL3_RenderReadPixels(renderer, rect);
    if (!surface) {
        return -1;
    }

    if (SDL3_ConvertPixelsAndColorspace(surface->w, surface->h, surface->format, SDL3_GetSurfaceColorspace(surface), SDL3_GetSurfaceProperties(surface), surface->pixels, surface->pitch, (SDL_PixelFormat)format, SDL_COLORSPACE_SRGB, 0, pixels, pitch)) {
        result = 0;
    }

    SDL3_DestroySurface(surface);

    return result;
}

SDL_DECLSPEC void SDLCALL
SDL_RenderPresent(SDL_Renderer *renderer)
{
    SDL3_RenderPresent(renderer);
}

static SDL_ScaleMode SDL_GetScaleMode(void)
{
    const char *hint = SDL3_GetHint("SDL_RENDER_SCALE_QUALITY");

    if (!hint || SDL3_strcasecmp(hint, "nearest") == 0) {
        return SDL_SCALEMODE_NEAREST;
    } else if (SDL3_strcasecmp(hint, "linear") == 0 ||
               SDL3_strcasecmp(hint, "best") == 0) {
        return SDL_SCALEMODE_LINEAR;
    } else {
        return (SDL_ScaleMode)SDL3_atoi(hint);
    }
}

SDL_DECLSPEC SDL_Texture * SDLCALL
SDL_CreateTexture(SDL_Renderer * renderer, Uint32 format, int access, int w, int h)
{
    SDL_Texture *texture = SDL3_CreateTexture(renderer, (SDL_PixelFormat)format, (SDL_TextureAccess)access, w, h);
    if (texture) {
        SDL3_SetTextureBlendMode(texture, SDL_BLENDMODE_NONE);
        SDL3_SetTextureScaleMode(texture, SDL_GetScaleMode());
    }
    return texture;
}

SDL_DECLSPEC SDL_Texture * SDLCALL
SDL_CreateTextureFromSurface(SDL_Renderer *renderer, SDL2_Surface *surface)
{
    SDL_Texture *texture = SDL3_CreateTextureFromSurface(renderer, Surface2to3(surface));
    if (texture) {
        SDL3_SetTextureScaleMode(texture, SDL_GetScaleMode());
    }
    return texture;
}

SDL_DECLSPEC int SDLCALL
SDL_QueryTexture(SDL_Texture *texture, Uint32 *format, int *access, int *w, int *h)
{
    SDL_PropertiesID props = SDL3_GetTextureProperties(texture);
    if (!props) {
        return -1;
    }

    if (format) {
        *format = (Uint32)SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_FORMAT_NUMBER, SDL_PIXELFORMAT_UNKNOWN);
    }
    if (access) {
        *access = (int)SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_ACCESS_NUMBER, SDL_TEXTUREACCESS_STATIC);
    }
    if (w) {
        *w = (int)SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_WIDTH_NUMBER, 0);
    }
    if (h) {
        *h = (int)SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_HEIGHT_NUMBER, 0);
    }
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_SetTextureScaleMode(SDL_Texture *texture, SDL_ScaleMode scaleMode)
{
    if (scaleMode > SDL_SCALEMODE_LINEAR) {
        scaleMode = SDL_SCALEMODE_LINEAR;
    }
    return SDL3_SetTextureScaleMode(texture, scaleMode) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_LockMutex(SDL_Mutex *a)
{
    SDL3_LockMutex(a);
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_TryLockMutex(SDL_Mutex *a)
{
    if (SDL3_TryLockMutex(a)) {
        return 0;
    } else {
        return SDL2_MUTEX_TIMEDOUT;
    }
}

SDL_DECLSPEC int SDLCALL
SDL_UnlockMutex(SDL_Mutex *a)
{
    SDL3_UnlockMutex(a);
    return 0;
}

#define PROP_TIMER_CALLBACK_POINTER "sdl2-compat.timer.callback"
#define PROP_TIMER_USERDATA_POINTER "sdl2-compat.timer.userdata"

static void SDLCALL CleanupTimerProperties(void *userdata, void *value)
{
    SDL_PropertiesID props = (SDL_PropertiesID)(uintptr_t)value;
    SDL3_DestroyProperties(props);
}

static Uint32 SDLCALL CompatTimerCallback(void *userdata, SDL_TimerID timerID, Uint32 interval)
{
    SDL_PropertiesID props = (SDL_PropertiesID)(uintptr_t)userdata;
    SDL2_TimerCallback callback = (SDL2_TimerCallback)SDL3_GetPointerProperty(props, PROP_TIMER_CALLBACK_POINTER, NULL);
    void *param = SDL3_GetPointerProperty(props, PROP_TIMER_USERDATA_POINTER, NULL);
    if (callback) {
        return callback(interval, param);
    } else {
        return 0;
    }
}

SDL_DECLSPEC SDL2_TimerID SDLCALL
SDL_AddTimer(Uint32 interval, SDL2_TimerCallback callback, void *param)
{
    SDL_TimerID timerID;
    SDL_PropertiesID props;

    if (!timers) {
        timers = SDL3_CreateProperties();
        if (!timers) {
            return 0;
        }
    }

    props = SDL3_CreateProperties();
    if (!props) {
        return 0;
    }
    SDL3_SetPointerProperty(props, PROP_TIMER_CALLBACK_POINTER, (void *)callback);
    SDL3_SetPointerProperty(props, PROP_TIMER_USERDATA_POINTER, param);
    timerID = SDL3_AddTimer(interval, CompatTimerCallback, (void *)(uintptr_t)props);
    if (timerID) {
        char name[32];

        SDL3_snprintf(name, sizeof(name), "%" SDL_PRIu32, timerID);
        SDL3_SetPointerPropertyWithCleanup(timers, name, (void *)(uintptr_t)props, CleanupTimerProperties, NULL);
    } else {
        SDL3_DestroyProperties(props);
    }
    return (SDL2_TimerID)timerID;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_RemoveTimer(SDL2_TimerID id)
{
    char name[32];

    SDL3_snprintf(name, sizeof(name), "%" SDL_PRIu32, id);
    SDL3_ClearProperty(timers, name);

    return SDL3_RemoveTimer((SDL_TimerID)id) ? SDL2_TRUE : SDL2_FALSE;
}


SDL_DECLSPEC int SDLCALL
SDL_AudioInit(const char *driver_name)
{
    if (driver_name) {
        SDL3_SetHint("SDL_AUDIO_DRIVER", driver_name);
    }
    return SDL3_InitSubSystem(SDL_INIT_AUDIO) ? 0 : -1;
}

SDL_DECLSPEC void SDLCALL
SDL_AudioQuit(void)
{
    SDL3_QuitSubSystem(SDL_INIT_AUDIO);
}

SDL_DECLSPEC int SDLCALL
SDL_VideoInit(const char *driver_name)
{
    int ret;
    if (driver_name) {
        SDL3_SetHint("SDL_VIDEO_DRIVER", driver_name);
    }

    ret = SDL3_InitSubSystem(SDL_INIT_VIDEO) ? 0 : -1;

    /* default SDL2 GL attributes */
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 3);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 3);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 2);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);

    return ret;
}

SDL_DECLSPEC void SDLCALL
SDL_VideoQuit(void)
{
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

SDL_DECLSPEC int SDLCALL
SDL_Init(Uint32 flags)
{
    int ret;

    ret = SDL3_InitSubSystem(flags) ? 0 : -1;
    if (flags & SDL_INIT_VIDEO) {
        /* default SDL2 GL attributes */
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 3);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 3);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 2);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    }

    return ret;
}

SDL_DECLSPEC int SDLCALL
SDL_InitSubSystem(Uint32 flags)
{
    int ret;

    SDL2Compat_InitLogPrefixes();

    /* Update IME UI hint */
    if (flags & SDL_INIT_VIDEO) {
        const char *old_hint;
        const char *hint;

#if defined(SDL_PLATFORM_WIN32)
        old_hint = SDL_GetHint("SDL_IME_SHOW_UI");
        if (old_hint && *old_hint == '1') {
            hint = "composition";
        } else {
            hint = "composition,candidates";
        }
#else
        old_hint = SDL_GetHint("SDL_IME_INTERNAL_EDITING");
        if (old_hint && *old_hint == '1') {
            hint = "0";
        } else {
            hint = "composition";
        }
#endif
        SDL_SetHint(SDL_HINT_IME_IMPLEMENTED_UI, hint);
    }

    ret = SDL3_InitSubSystem(flags) ? 0 : -1;
    if (flags & SDL_INIT_VIDEO) {
        /* default SDL2 GL attributes */
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 3);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 3);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 2);
        SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
    }
    return ret;
}

SDL_DECLSPEC void SDLCALL
SDL_Quit(void)
{
    int i;
    SDL_LogPriority priorities[SDL_LOG_CATEGORY_CUSTOM];
    relative_mouse_mode = SDL2_FALSE;

    if (SDL3_WasInit(SDL_INIT_VIDEO)) {
        GestureQuit();
    }

    if (joystick_list) {
        SDL3_free(joystick_list);
        joystick_list = NULL;
    }
    num_joysticks = 0;

    if (sensor_list) {
        SDL3_free(sensor_list);
        sensor_list = NULL;
    }
    num_sensors = 0;

    if (haptic_list) {
        SDL3_free(haptic_list);
        haptic_list = NULL;
    }
    num_haptics = 0;

    if (gamepad_button_swap_list) {
        SDL3_free(gamepad_button_swap_list);
        gamepad_button_swap_list = NULL;
    }
    num_gamepad_button_swap_list = 0;

    SDL3_free(GamepadMappings);
    GamepadMappings = NULL;
    NumGamepadMappings = 0;

    SDL3_free(TouchDevices);
    TouchDevices = NULL;
    NumTouchDevices = 0;
    TouchFingersDeviceID = 0;
    SDL3_free(TouchFingers);
    TouchFingers = NULL;
    NumTouchFingers = 0;

    if (timers) {
        SDL3_DestroyProperties(timers);
        timers = 0;
    }

    for (i = 0; i < SDL_LOG_CATEGORY_CUSTOM; i++) {
        priorities[i] = SDL3_GetLogPriority(i);
    }

    SDL3_Quit();

    for (i = 0; i < SDL_LOG_CATEGORY_CUSTOM; i++) {
        SDL3_SetLogPriority(i, priorities[i]);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_QuitSubSystem(Uint32 flags)
{
    if (flags & SDL_INIT_VIDEO) {
        GestureQuit();
    }

    // !!! FIXME: there's cleanup in SDL_Quit that probably needs to be done here, too.

    SDL3_QuitSubSystem(flags);
}

SDL_DECLSPEC int SDLCALL
SDL_GL_GetSwapInterval(void)
{
    int val = 0;
    SDL3_GL_GetSwapInterval(&val);
    return val;
}

static Uint16 GetDefaultSamplesFromFreq(int freq)
{
    /* Pick a default of ~46 ms at desired frequency */
    /* !!! FIXME: remove this when the non-Po2 resampling is in. */
    const Uint16 max_sample = (freq / 1000) * 46;
    Uint16 current_sample = 1;
    while (current_sample < max_sample) {
        current_sample *= 2;
    }
    return current_sample;
}

static int GetNumAudioDevices(int iscapture)
{
    AudioDeviceList newlist;
    AudioDeviceList *list = iscapture ? &AudioSDL3RecordingDevices : &AudioSDL3PlaybackDevices;
    SDL_AudioDeviceID *devices;
    int num_devices;
    int i;

    /* SDL_GetNumAudioDevices triggers a device redetect in SDL2, so we'll just build our list from here. */
    devices = iscapture ? SDL3_GetAudioRecordingDevices(&num_devices) : SDL3_GetAudioPlaybackDevices(&num_devices);
    if (!devices) {
        return list->num_devices;  /* just return the existing one for now. Oh well. */
    }

    SDL3_zero(newlist);
    if (num_devices > 0) {
        newlist.num_devices = num_devices;
        newlist.devices = (AudioDeviceInfo *) SDL3_malloc(sizeof (AudioDeviceInfo) * num_devices);
        if (!newlist.devices) {
            SDL_free(devices);
            return list->num_devices;  /* just return the existing one for now. Oh well. */
        }

        for (i = 0; i < num_devices; i++) {
            const char *newname = SDL3_GetAudioDeviceName(devices[i]);
            char *fullname = NULL;
            if (newname == NULL) {
                /* ugh, whatever, just make up a name. */
                newname = "Unidentified device";
            }

            /* Device names must be unique in SDL2, as that's how we open them.
               SDL2 took serious pains to try to add numbers to the end of duplicate device names ("SoundBlaster Pro" and then "SoundBlaster Pro (2)"),
               but here we're just putting the actual SDL3 instance id at the end of everything. Good enough. I hope. */
            if (!newname || (SDL3_asprintf(&fullname, "%s (id=%u)", newname, (unsigned int) devices[i]) < 0)) {
                /* we're in real trouble now.  :/  */
                int j;
                for (j = 0; j < i; j++) {
                    SDL3_free(newlist.devices[i].name);
                }
                SDL3_free(fullname);
                SDL_free(devices);
                return list->num_devices;  /* just return the existing one for now. Oh well. */
            }

            newlist.devices[i].devid = devices[i];
            newlist.devices[i].name = fullname;
        }
    }

    for (i = 0; i < list->num_devices; i++) {
        SDL3_free(list->devices[i].name);
    }
    SDL3_free(list->devices);
    SDL_free(devices);

    SDL3_memcpy(list, &newlist, sizeof (AudioDeviceList));
    return num_devices;
}

SDL_DECLSPEC int SDLCALL
SDL_GetNumAudioDevices(int iscapture)
{
    int retval;

    if (!SDL3_GetCurrentAudioDriver()) {
        SDL3_SetError("Audio subsystem is not initialized");
        return -1;
    }

    SDL3_LockMutex(AudioDeviceLock);
    retval = GetNumAudioDevices(iscapture);
    SDL3_UnlockMutex(AudioDeviceLock);
    return retval;
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetAudioDeviceName(int idx, int iscapture)
{
    const char *retval = NULL;
    AudioDeviceList *list;

    if (!SDL3_GetCurrentAudioDriver()) {
        SDL3_SetError("Audio subsystem is not initialized");
        return NULL;
    }

    SDL3_LockMutex(AudioDeviceLock);
    list = iscapture ? &AudioSDL3RecordingDevices : &AudioSDL3PlaybackDevices;
    if ((idx < 0) || (idx >= list->num_devices)) {
        SDL3_InvalidParamError("index");
    } else {
        retval = list->devices[idx].name;
    }
    SDL3_UnlockMutex(AudioDeviceLock);

    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_GetAudioDeviceSpec(int idx, int iscapture, SDL2_AudioSpec *spec2)
{
    AudioDeviceList *list;
    SDL_AudioSpec spec3;
    int retval = -1;

    if (spec2 == NULL) {
        SDL3_InvalidParamError("spec");
        return -1;
    } else if (!SDL3_GetCurrentAudioDriver()) {
        SDL3_SetError("Audio subsystem is not initialized");
        return -1;
    }

    SDL3_LockMutex(AudioDeviceLock);
    list = iscapture ? &AudioSDL3RecordingDevices : &AudioSDL3PlaybackDevices;
    if ((idx < 0) || (idx >= list->num_devices)) {
        SDL3_InvalidParamError("index");
    } else {
        retval = SDL3_GetAudioDeviceFormat(list->devices[idx].devid, &spec3, NULL) ? 0 : -1;
    }
    SDL3_UnlockMutex(AudioDeviceLock);

    if (retval == 0) {
        SDL3_zerop(spec2);
        spec2->format = spec3.format;
        spec2->channels = spec3.channels;
        spec2->freq = spec3.freq;
    }

    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_GetDefaultAudioInfo(char **name, SDL2_AudioSpec *spec2, int iscapture)
{
    SDL_AudioSpec spec3;
    int retval;

    if (spec2 == NULL) {
        SDL3_InvalidParamError("spec");
        return -1;
    } else if (!SDL3_GetCurrentAudioDriver()) {
        SDL3_SetError("Audio subsystem is not initialized");
        return -1;
    }

    retval = SDL3_GetAudioDeviceFormat(iscapture ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec3, NULL);
    if (retval == 0) {
        if (name) {
            *name = SDL3_strdup("System default");  /* the default device can change to different physical hardware on-the-fly in SDL3, so we don't provide a name for it. */
            if (*name == NULL) {
                return -1;
            }
        }

        SDL3_zerop(spec2);
        spec2->format = spec3.format;
        spec2->channels = spec3.channels;
        spec2->freq = spec3.freq;
    }

    return retval;
}

static SDL2_AudioFormat ParseAudioFormat(const char *string)
{
#define CHECK_FMT_STRING(x) if (SDL3_strcmp(string, #x) == 0) { return SDL_AUDIO_##x; }
#define CHECK_FMT_STRING2(x, y) if (SDL3_strcmp(string, #x) == 0) { return y; }
    CHECK_FMT_STRING(U8);
    CHECK_FMT_STRING(S8);
    CHECK_FMT_STRING2(S16LE, SDL_AUDIO_S16LE);
    CHECK_FMT_STRING2(S16BE, SDL_AUDIO_S16BE);
    CHECK_FMT_STRING2(S16SYS, SDL_AUDIO_S16);
    CHECK_FMT_STRING2(S16, SDL_AUDIO_S16LE);
    CHECK_FMT_STRING2(U16LE, SDL_AUDIO_S16LE & ~SDL_AUDIO_MASK_SIGNED);
    CHECK_FMT_STRING2(U16BE, SDL_AUDIO_S16BE & ~SDL_AUDIO_MASK_SIGNED);
    CHECK_FMT_STRING2(U16SYS, SDL_AUDIO_S16 & ~SDL_AUDIO_MASK_SIGNED);
    CHECK_FMT_STRING2(U16, SDL_AUDIO_S16LE & ~SDL_AUDIO_MASK_SIGNED);
    CHECK_FMT_STRING2(S32LE, SDL_AUDIO_S32LE);
    CHECK_FMT_STRING2(S32BE, SDL_AUDIO_S32BE);
    CHECK_FMT_STRING2(S32SYS, SDL_AUDIO_S32);
    CHECK_FMT_STRING2(S32, SDL_AUDIO_S32LE);
    CHECK_FMT_STRING2(F32LE, SDL_AUDIO_F32LE);
    CHECK_FMT_STRING2(F32BE, SDL_AUDIO_F32BE);
    CHECK_FMT_STRING2(F32SYS, SDL_AUDIO_F32);
    CHECK_FMT_STRING2(F32, SDL_AUDIO_F32LE);
#undef CHECK_FMT_STRING
    return 0;
}

static int PrepareAudiospec(const SDL2_AudioSpec *orig2, SDL2_AudioSpec *prepared2)
{
    SDL3_memcpy(prepared2, orig2, sizeof(*orig2));

    if (orig2->freq == 0) {
        static const int DEFAULT_FREQ = 22050;
        const char *env = SDL3_getenv("SDL_AUDIO_FREQUENCY");
        if (env != NULL) {
            int freq = SDL3_atoi(env);
            prepared2->freq = freq != 0 ? freq : DEFAULT_FREQ;
        } else {
            prepared2->freq = DEFAULT_FREQ;
        }
    }

    if (orig2->format == 0) {
        const char *env = SDL3_getenv("SDL_AUDIO_FORMAT");
        if (env != NULL) {
            const SDL2_AudioFormat format = ParseAudioFormat(env);
            prepared2->format = format != 0 ? format : SDL_AUDIO_S16;
        } else {
            prepared2->format = SDL_AUDIO_S16;
        }
    }

    if (orig2->channels == 0) {
        const char *env = SDL3_getenv("SDL_AUDIO_CHANNELS");
        if (env != NULL) {
            Uint8 channels = (Uint8)SDL3_atoi(env);
            prepared2->channels = channels != 0 ? channels : 2;
        } else {
            prepared2->channels = 2;
        }
    } else if (orig2->channels > 8) {
        SDL3_SetError("Unsupported number of audio channels.");
        return 0;
    }

    if (orig2->samples == 0) {
        const char *env = SDL3_getenv("SDL_AUDIO_SAMPLES");
        if (env != NULL) {
            Uint16 samples = (Uint16)SDL3_atoi(env);
            prepared2->samples = samples != 0 ? samples : GetDefaultSamplesFromFreq(prepared2->freq);
        } else {
            prepared2->samples = GetDefaultSamplesFromFreq(prepared2->freq);
        }
    }

    /* Calculate the silence and size of the audio specification */
    if ((prepared2->format == SDL_AUDIO_U8) || (prepared2->format == (SDL_AUDIO_S16LE & ~SDL_AUDIO_MASK_SIGNED)) || (prepared2->format == (SDL_AUDIO_S16BE & ~SDL_AUDIO_MASK_SIGNED))) {
        prepared2->silence = 0x80;
    } else {
        prepared2->silence = 0x00;
    }

    prepared2->size = SDL_AUDIO_BITSIZE(prepared2->format) / 8;
    prepared2->size *= prepared2->channels;
    prepared2->size *= prepared2->samples;

    return 1;
}

static void SDLCALL SDL2AudioDeviceQueueingCallback(void *userdata, SDL_AudioStream *stream3, int approx_amount, int total_amount)
{
    SDL2_AudioStream *stream2 = (SDL2_AudioStream *) userdata;
    Uint8 *buffer;

    SDL_assert(stream2 != NULL);
    SDL_assert(stream3 == stream2->stream3);
    SDL_assert(stream2->dataqueue3 != NULL);

    if (approx_amount == 0) {
        return;  /* nothing to do right now. */
    }

    buffer = (Uint8 *) SDL3_malloc(approx_amount);
    if (!buffer) {
        return;  /* oh well */
    }

    if (stream2->iscapture) {
        const int br = SDL_AudioStreamGet(stream2, buffer, approx_amount);
        if (br > 0) {
            SDL3_PutAudioStreamData(stream2->dataqueue3, buffer, br);
        }
    } else {
        const int br = SDL3_GetAudioStreamData(stream2->dataqueue3, buffer, approx_amount);
        if (br > 0) {
            SDL_AudioStreamPut(stream2, buffer, br);
        }
    }

    SDL3_free(buffer);
}

static void SDLCALL SDL2AudioDeviceCallbackBridge(void *userdata, SDL_AudioStream *stream3, int approx_amount, int total_amount)
{
    SDL2_AudioStream *stream2 = (SDL2_AudioStream *) userdata;
    Uint8 *buffer;

    if (approx_amount == 0) {
        return;  /* nothing to do right now. */
    }

    SDL_assert(stream2 != NULL);
    SDL_assert(stream3 == stream2->stream3);
    SDL_assert(stream2->dataqueue3 == NULL);

    buffer = (Uint8 *) SDL3_malloc(approx_amount);
    if (!buffer) {
        return;  /* oh well */
    }

    if (stream2->iscapture) {
        const int br = SDL_AudioStreamGet(stream2, buffer, approx_amount);
        stream2->callback2(stream2->callback2_userdata, buffer, br);
    } else {
        stream2->callback2(stream2->callback2_userdata, buffer, approx_amount);
        SDL_AudioStreamPut(stream2, buffer, approx_amount);
    }

    SDL3_free(buffer);
}

static SDL_AudioDeviceID OpenAudioDeviceLocked(const char *devicename, int iscapture,
                                               const SDL2_AudioSpec *desired2, SDL2_AudioSpec *obtained2,
                                               int allowed_changes, int min_id)
{
    /* we use an SDL2 audio stream for this, since it'll use an SDL3 audio stream behind the scenes, but also support the removed-from-SDL3 U16 audio format. */
    SDL2_AudioStream *stream2;
    SDL_AudioDeviceID device3 = 0;
    SDL_AudioSpec spec3;
    int id;

    SDL_assert(obtained2 != NULL);  /* we checked this before locking. */

    /* Find an available device ID... */
    for (id = min_id - 1; id < (int)SDL_arraysize(AudioOpenDevices); id++) {
        if (AudioOpenDevices[id] == NULL) {
            break;
        }
    }

    if (id == SDL_arraysize(AudioOpenDevices)) {
        SDL3_SetError("Too many open audio devices");
        return 0;
    }

    /* (also note that SDL2 doesn't check if `desired2` is NULL before dereferencing, either.) */
    if (!PrepareAudiospec(desired2, obtained2)) {
        return 0;
    }

    /* If app doesn't care about a specific device, let the user override. */
    if (devicename == NULL) {
        devicename = SDL3_getenv("SDL_AUDIO_DEVICE_NAME");
    }

    if (devicename == NULL) {
        device3 = iscapture ? SDL_AUDIO_DEVICE_DEFAULT_RECORDING : SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK;
    } else {
        AudioDeviceList *list = iscapture ? &AudioSDL3RecordingDevices : &AudioSDL3PlaybackDevices;
        const int total = list->num_devices;
        int i;
        for (i = 0; i < total; i++) {
            if (SDL3_strcmp(list->devices[i].name, devicename) == 0) {
                device3 = list->devices[i].devid;
                break;
            }
        }
        if (device3 == 0) {
            SDL3_SetError("No such device.");
            return 0;
        }
    }

    spec3.format = (SDL_AudioFormat)obtained2->format;
    spec3.channels = obtained2->channels;
    spec3.freq = obtained2->freq;

    /* Don't try to open the SDL3 audio device with the (abandoned) U16 formats... */
    if (spec3.format == SDL2_AUDIO_U16LSB) {
        spec3.format = SDL_AUDIO_S16LE;
    } else if (spec3.format == SDL2_AUDIO_U16MSB) {
        spec3.format = SDL_AUDIO_S16BE;
    }

    device3 = SDL3_OpenAudioDevice(device3, &spec3);
    if (device3 == 0) {
        return 0;
    }

    SDL3_PauseAudioDevice(device3);  /* they start paused in SDL2 */
    SDL3_GetAudioDeviceFormat(device3, &spec3, NULL);

    if ((spec3.format != (SDL_AudioFormat)obtained2->format) && (allowed_changes & SDL2_AUDIO_ALLOW_FORMAT_CHANGE)) {
        obtained2->format = (SDL2_AudioFormat)spec3.format;
    }
    if ((spec3.channels != obtained2->channels) && (allowed_changes & SDL2_AUDIO_ALLOW_CHANNELS_CHANGE)) {
        obtained2->freq = spec3.channels;
    }
    if ((spec3.freq != obtained2->freq) && (allowed_changes & SDL2_AUDIO_ALLOW_FREQUENCY_CHANGE)) {
        obtained2->freq = spec3.freq;
    }

    if (iscapture) {
        stream2 = SDL_NewAudioStream((SDL2_AudioFormat)spec3.format, spec3.channels, spec3.freq, obtained2->format, obtained2->channels, obtained2->freq);
    } else {
        stream2 = SDL_NewAudioStream(obtained2->format, obtained2->channels, obtained2->freq, (SDL2_AudioFormat)spec3.format, spec3.channels, spec3.freq);
    }

    if (!stream2) {
        SDL3_CloseAudioDevice(device3);
        return 0;
    }

    if (desired2->callback) {
        stream2->callback2 = desired2->callback;
        stream2->callback2_userdata = desired2->userdata;
        if (iscapture) {
            SDL3_SetAudioStreamPutCallback(stream2->stream3, SDL2AudioDeviceCallbackBridge, stream2);
        } else {
            SDL3_SetAudioStreamGetCallback(stream2->stream3, SDL2AudioDeviceCallbackBridge, stream2);
        }
    } else {
        /* SDL2 treats this as a simple data queue that doesn't convert before the audio callback gets it,
           so just make a second audiostream with no conversion and let it work like a flexible buffer. */
        stream2->dataqueue3 = SDL3_CreateAudioStream(&spec3, &spec3);
        if (!stream2->dataqueue3) {
            SDL3_CloseAudioDevice(device3);
            SDL_FreeAudioStream(stream2);
        }
        if (iscapture) {
            SDL3_SetAudioStreamPutCallback(stream2->stream3, SDL2AudioDeviceQueueingCallback, stream2);
        } else {
            SDL3_SetAudioStreamGetCallback(stream2->stream3, SDL2AudioDeviceQueueingCallback, stream2);
        }
    }

    if (!SDL3_BindAudioStream(device3, stream2->stream3)) {
        SDL_FreeAudioStream(stream2);
        SDL3_CloseAudioDevice(device3);
        return 0;
    }

    stream2->iscapture = iscapture ? SDL2_TRUE : SDL2_FALSE;
    AudioOpenDevices[id] = stream2;

    return id + 1;
}

static SDL_AudioDeviceID OpenAudioDevice(const char *devicename, int iscapture,
                                         const SDL2_AudioSpec *desired2, SDL2_AudioSpec *obtained2,
                                         int allowed_changes, int min_id)
{
    SDL2_AudioSpec _obtained2;
    SDL_AudioDeviceID retval;

    if (!SDL3_GetCurrentAudioDriver()) {
        SDL3_SetError("Audio subsystem is not initialized");
        return 0;
    }

    if (!obtained2) {
        obtained2 = &_obtained2;
    }

    SDL3_LockMutex(AudioDeviceLock);
    retval = OpenAudioDeviceLocked(devicename, iscapture, desired2, obtained2, allowed_changes, min_id);
    SDL3_UnlockMutex(AudioDeviceLock);

    return retval;
}


SDL_DECLSPEC SDL_AudioDeviceID SDLCALL
SDL_OpenAudioDevice(const char *device, int iscapture, const SDL2_AudioSpec *desired2, SDL2_AudioSpec *obtained2, int allowed_changes)
{
    return OpenAudioDevice(device, iscapture, desired2, obtained2, allowed_changes, 2);
}

SDL_DECLSPEC int SDLCALL
SDL_OpenAudio(SDL2_AudioSpec *desired2, SDL2_AudioSpec *obtained2)
{
    SDL_AudioDeviceID id = 0;

    /* Start up the audio driver, if necessary. This is legacy behaviour! */
    if (!SDL3_WasInit(SDL_INIT_AUDIO)) {
        if (!SDL3_InitSubSystem(SDL_INIT_AUDIO)) {
            return -1;
        }
    }

    if (AudioOpenDevices[0] != NULL) {
        SDL3_SetError("Audio device is already opened");
        return -1;
    }

    if (obtained2) {
        id = OpenAudioDevice(NULL, 0, desired2, obtained2, SDL2_AUDIO_ALLOW_ANY_CHANGE, 1);
    } else {
        SDL2_AudioSpec _obtained2;
        SDL3_zero(_obtained2);
        id = OpenAudioDevice(NULL, 0, desired2, &_obtained2, 0, 1);

        /* On successful open, copy calculated values into 'desired'. */
        if (id > 0) {
            desired2->size = _obtained2.size;
            desired2->silence = _obtained2.silence;
        }
    }

    if (id > 0) {
        return 0;
    }
    return -1;
}

/* `src` and `dst` can be the same pointer. The buffer size does not change. */
static void AudioUi16LSBToSi16Sys(Sint16 *dst, const Uint16 *src, const size_t num_samples)
{
    size_t i;
    for (i = 0; i < num_samples; i++) {
        dst[i] = (Sint16) (SDL_Swap16LE(src[i]) ^ 0x8000);
    }
}

/* `src` and `dst` can be the same pointer. The buffer size does not change. */
static void AudioUi16MSBToSi16Sys(Sint16 *dst, const Uint16 *src, const size_t num_samples)
{
    size_t i;
    for (i = 0; i < num_samples; i++) {
        dst[i] = (Sint16) (SDL_Swap16BE(src[i]) ^ 0x8000);
    }
}

/* `src` and `dst` can be the same pointer. The buffer size does not change. */
static void AudioSi16SysToUi16LSB(Uint16 *dst, const Sint16 *src, const size_t num_samples)
{
    size_t i;
    for (i = 0; i < num_samples; i++) {
        dst[i] = SDL_Swap16LE(((Uint16) src[i]) ^ 0x8000);
    }
}

/* `src` and `dst` can be the same pointer. The buffer size does not change. */
static void AudioSi16SysToUi16MSB(Uint16 *dst, const Sint16 *src, const size_t num_samples)
{
    size_t i;
    for (i = 0; i < num_samples; i++) {
        dst[i] = SDL_Swap16BE(((Uint16) src[i]) ^ 0x8000);
    }
}

SDL_DECLSPEC SDL2_AudioStream * SDLCALL
SDL_NewAudioStream(const SDL2_AudioFormat real_src_format, const Uint8 src_channels, const int src_rate, const SDL2_AudioFormat real_dst_format, const Uint8 dst_channels, const int dst_rate)
{
    SDL2_AudioFormat src_format = real_src_format;
    SDL2_AudioFormat dst_format = real_dst_format;
    SDL2_AudioStream *retval = (SDL2_AudioStream *) SDL3_calloc(1, sizeof (SDL2_AudioStream));
    SDL_AudioSpec srcspec3, dstspec3;

    if (!retval) {
        return NULL;
    }

    /* SDL3 removed U16 audio formats. Convert to S16SYS. */
    if ((src_format == SDL2_AUDIO_U16LSB) || (src_format == SDL2_AUDIO_U16MSB)) {
        src_format = SDL_AUDIO_S16;
    }
    if ((dst_format == SDL2_AUDIO_U16LSB) || (dst_format == SDL2_AUDIO_U16MSB)) {
        dst_format = SDL_AUDIO_S16;
    }

    srcspec3.format = (SDL_AudioFormat)src_format;
    srcspec3.channels = src_channels;
    srcspec3.freq = src_rate;
    dstspec3.format = (SDL_AudioFormat)dst_format;
    dstspec3.channels = dst_channels;
    dstspec3.freq = dst_rate;
    retval->stream3 = SDL3_CreateAudioStream(&srcspec3, &dstspec3);
    if (retval->stream3 == NULL) {
        SDL3_free(retval);
        return NULL;
    }

    retval->src_format = real_src_format;
    retval->dst_format = real_dst_format;
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_AudioStreamPut(SDL2_AudioStream *stream2, const void *buf, int len)
{
    int retval;

    /* SDL3 removed U16 audio formats. Convert to S16SYS. */
    if (stream2 && buf && len && ((stream2->src_format == SDL2_AUDIO_U16LSB) || (stream2->src_format == SDL2_AUDIO_U16MSB))) {
        const Uint32 tmpsamples = len / sizeof (Uint16);
        Sint16 *tmpbuf = (Sint16 *) SDL3_malloc(len);
        if (!tmpbuf) {
            return -1;
        }
        if (stream2->src_format == SDL2_AUDIO_U16LSB) {
            AudioUi16LSBToSi16Sys(tmpbuf, (const Uint16 *) buf, tmpsamples);
        } else if (stream2->src_format == SDL2_AUDIO_U16MSB) {
            AudioUi16MSBToSi16Sys(tmpbuf, (const Uint16 *) buf, tmpsamples);
        }
        retval = SDL3_PutAudioStreamData(stream2->stream3, tmpbuf, len) ? 0 : -1;
        SDL3_free(tmpbuf);
    } else {
        retval = SDL3_PutAudioStreamData(stream2->stream3, buf, len) ? 0 : -1;
    }

    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_AudioStreamGet(SDL2_AudioStream *stream2, void *buf, int len)
{
    const int retval = stream2 ? SDL3_GetAudioStreamData(stream2->stream3, buf, len) : SDL3_InvalidParamError("stream");

    if (retval > 0) {
        /* SDL3 removed U16 audio formats. Convert to S16SYS. */
        SDL_assert(stream2 != NULL);
        SDL_assert(buf != NULL);
        SDL_assert(len > 0);
        if ((stream2->dst_format == SDL2_AUDIO_U16LSB) || (stream2->dst_format == SDL2_AUDIO_U16MSB)) {
            if (stream2->dst_format == SDL2_AUDIO_U16LSB) {
                AudioSi16SysToUi16LSB((Uint16 *) buf, (const Sint16 *) buf, retval / sizeof (Sint16));
            } else if (stream2->dst_format == SDL2_AUDIO_U16MSB) {
                AudioSi16SysToUi16MSB((Uint16 *) buf, (const Sint16 *) buf, retval / sizeof (Sint16));
            }
        }
    }

    return retval;
}

SDL_DECLSPEC void SDLCALL
SDL_AudioStreamClear(SDL2_AudioStream *stream2)
{
    SDL3_ClearAudioStream(stream2 ? stream2->stream3 : NULL);
}

SDL_DECLSPEC int SDLCALL
SDL_AudioStreamAvailable(SDL2_AudioStream *stream2)
{
    return SDL3_GetAudioStreamAvailable(stream2 ? stream2->stream3 : NULL);
}

SDL_DECLSPEC void SDLCALL
SDL_FreeAudioStream(SDL2_AudioStream *stream2)
{
    if (stream2) {
        SDL3_DestroyAudioStream(stream2->stream3);
        SDL3_DestroyAudioStream(stream2->dataqueue3);
        SDL3_free(stream2);
    }
}

SDL_DECLSPEC int SDLCALL
SDL_AudioStreamFlush(SDL2_AudioStream *stream2)
{
    return SDL3_FlushAudioStream(stream2 ? stream2->stream3 : NULL) ? 0 : -1;
}

#define SDL2_MIX_MAXVOLUME 128.0f
SDL_DECLSPEC void SDLCALL
SDL_MixAudioFormat(Uint8 *dst, const Uint8 *src, SDL2_AudioFormat format, Uint32 len, int volume)
{
    const float fvolume = volume / SDL2_MIX_MAXVOLUME;

    /* SDL3 removed U16 audio formats. Convert to S16SYS. */
    if ((format == SDL2_AUDIO_U16LSB) || (format == SDL2_AUDIO_U16MSB)) {
        const Uint32 tmpsamples = len / sizeof (Uint16);
        Sint16 *tmpbuf = (Sint16 *) SDL3_malloc(len);
        if (tmpbuf) {  /* if malloc fails, oh well, no mixed audio for you. */
            if (format == SDL2_AUDIO_U16LSB) {
                AudioUi16LSBToSi16Sys(tmpbuf, (const Uint16 *) src, tmpsamples);
            } else if (format == SDL2_AUDIO_U16MSB) {
                AudioUi16MSBToSi16Sys(tmpbuf, (const Uint16 *) src, tmpsamples);
            }
            SDL3_MixAudio(dst, (const Uint8 *) tmpbuf, SDL_AUDIO_S16, tmpsamples * sizeof (Sint16), fvolume);
            SDL3_free(tmpbuf);
        }
    } else {
        SDL3_MixAudio(dst, src, (SDL_AudioFormat)format, len, fvolume);
    }
}

static SDL2_AudioStream *GetOpenAudioDevice(SDL_AudioDeviceID id)
{
    id--;
    if ((id >= SDL_arraysize(AudioOpenDevices)) || (AudioOpenDevices[id] == NULL)) {
        SDL3_SetError("Invalid audio device ID");
        return NULL;
    }
    return AudioOpenDevices[id];
}

SDL_DECLSPEC void SDLCALL
SDL_MixAudio(Uint8 *dst, const Uint8 *src, Uint32 len, int volume)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(1);
    if (stream2 != NULL) {
        SDL_MixAudioFormat(dst, src, stream2->dst_format, len, volume);  /* call the sdl2-compat version, since it needs to handle U16 audio data. */
    }
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    return (stream2 && (stream2->dataqueue3 != NULL)) ? SDL3_GetAudioStreamAvailable(stream2->dataqueue3) : 0;
}

SDL_DECLSPEC int SDLCALL
SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (!stream2) {
        return -1;
    } else if (stream2->iscapture) {
        SDL3_SetError("This is a capture device, queueing not allowed");
        return -1;
    } else if (stream2->dataqueue3 == NULL) {
        SDL3_SetError("Audio device has a callback, queueing not allowed");
        return -1;
    } else if (len == 0) {
        return 0;  /* nothing to do. */
    }

    SDL_assert(stream2->dataqueue3 != NULL);
    return SDL3_PutAudioStreamData(stream2->dataqueue3, data, len) ? 0 : -1;
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_DequeueAudio(SDL_AudioDeviceID dev, void *data, Uint32 len)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if ((len == 0) ||                      /* nothing to do? */
        (!stream2) ||                      /* called with bogus device id */
        (!stream2->iscapture) ||           /* playback devices can't dequeue */
        (stream2->dataqueue3 == NULL)) {   /* not set for queueing */
        return 0;                          /* just report zero bytes dequeued. */
    }

    SDL_assert(stream2->dataqueue3 != NULL);
    return SDL3_GetAudioStreamData(stream2->dataqueue3, data, len);
}

SDL_DECLSPEC void SDLCALL
SDL_ClearQueuedAudio(SDL_AudioDeviceID dev)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (stream2 && stream2->dataqueue3) {
        SDL3_ClearAudioStream(stream2->dataqueue3);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (stream2) {
        const SDL_AudioDeviceID device3 = SDL3_GetAudioStreamDevice(stream2->stream3);
        SDL3_ClearAudioStream(stream2->stream3);
        if (device3) {
            if (pause_on) {
                SDL3_PauseAudioDevice(device3);
            } else {
                SDL3_ResumeAudioDevice(device3);
            }
        }
    }
}

SDL_DECLSPEC SDL2_AudioStatus SDLCALL
SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev)
{
    SDL2_AudioStatus retval = SDL2_AUDIO_STOPPED;
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (stream2) {
        const SDL_AudioDeviceID device3 = SDL3_GetAudioStreamDevice(stream2->stream3);
        if (device3) {
            retval = SDL3_AudioDevicePaused(device3) ? SDL2_AUDIO_PAUSED : SDL2_AUDIO_PLAYING;
        }
    }
    return retval;
}

SDL_DECLSPEC void SDLCALL
SDL_LockAudioDevice(SDL_AudioDeviceID dev)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (stream2) {
        SDL3_LockAudioStream(stream2->stream3);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_UnlockAudioDevice(SDL_AudioDeviceID dev)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (stream2) {
        SDL3_UnlockAudioStream(stream2->stream3);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
    SDL2_AudioStream *stream2 = GetOpenAudioDevice(dev);
    if (stream2) {
        SDL3_CloseAudioDevice(SDL3_GetAudioStreamDevice(stream2->stream3));
        SDL_FreeAudioStream(stream2);
        AudioOpenDevices[dev - 1] = NULL;  /* this doesn't hold a lock in SDL2, either; the lock only prevents two racing opens from getting the same id. We can NULL it whenever, though. */
    }
}

SDL_DECLSPEC void SDLCALL
SDL_LockAudio(void)
{
    SDL_LockAudioDevice(1);
}

SDL_DECLSPEC void SDLCALL
SDL_UnlockAudio(void)
{
    SDL_UnlockAudioDevice(1);
}

SDL_DECLSPEC void SDLCALL
SDL_CloseAudio(void)
{
    SDL_CloseAudioDevice(1);
}

SDL_DECLSPEC void SDLCALL
SDL_PauseAudio(int pause_on)
{
    SDL_PauseAudioDevice(1, pause_on);
}

SDL_DECLSPEC SDL2_AudioStatus SDLCALL
SDL_GetAudioStatus(void)
{
    return SDL_GetAudioDeviceStatus(1);
}


SDL_DECLSPEC void SDLCALL
SDL_LockJoysticks(void)
{
    SDL3_LockMutex(joystick_lock);
}

SDL_DECLSPEC void SDLCALL
SDL_UnlockJoysticks(void)
{
    SDL3_UnlockMutex(joystick_lock);
}

SDL_DECLSPEC void SDLCALL
SDL_LockSensors(void)
{
    SDL3_LockMutex(sensor_lock);
}

SDL_DECLSPEC void SDLCALL
SDL_UnlockSensors(void)
{
    SDL3_UnlockMutex(sensor_lock);
}

static void
DisplayMode_3to2(const SDL_DisplayMode *in, SDL2_DisplayMode *out) {
    if (in && out) {
        out->format = in->format;
        out->w = in->w;
        out->h = in->h;
        out->refresh_rate = (int) SDL3_lroundf(in->refresh_rate);
        out->driverdata = in->internal;
    }
}

static SDL_DisplayID
Display_IndexToID(int displayIndex)
{
    SDL_DisplayID displayID;
    int count = 0;
    const SDL_DisplayID *list;

    list = SDL3_GetDisplays(&count);

    if (list == NULL || count == 0) {
        SDL3_SetError("no displays");
        return 0;
    }

    if (displayIndex < 0 || displayIndex >= count) {
        SDL3_SetError("invalid displayIndex");
        return 0;
    }

    displayID = list[displayIndex];
    return displayID;
}

SDL_DECLSPEC int SDLCALL
SDL_GetNumVideoDisplays(void)
{
    int count = 0;
    SDL3_GetDisplays(&count);
    return count;
}

SDL_DECLSPEC int SDLCALL
SDL_GetWindowDisplayIndex(SDL_Window *window)
{
    SDL_DisplayID displayID = SDL3_GetDisplayForWindow(window);
    return Display_IDToIndex(displayID);
}

SDL_DECLSPEC int SDLCALL
SDL_GetPointDisplayIndex(const SDL_Point *point)
{
    SDL_DisplayID displayID = SDL3_GetDisplayForPoint(point);
    return Display_IDToIndex(displayID);
}

SDL_DECLSPEC int SDLCALL
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
    const SDL_DisplayID *list;

    if (displayID == 0) {
        SDL3_SetError("invalid displayID");
        return -1;
    }

    list = SDL3_GetDisplays(&count);

    if (list == NULL || count == 0) {
        SDL3_SetError("no displays");
        return -1;
    }

    for (i = 0; i < count; i++) {
        if (list[i] == displayID) {
            displayIndex = i;
            break;
        }
    }
    return displayIndex;
}

static const SDL_DisplayMode **SDL_GetDisplayModeList(SDL_DisplayID displayID, int *count)
{
    int c = 0;
    const SDL_DisplayMode **modes = (const SDL_DisplayMode **)SDL3_GetFullscreenDisplayModes(displayID, &c);
    if (!modes) {
        return NULL;
    }

    if (c == 0) {
        /* Return the desktop mode, if no exclusive fullscreen modes are available. */
        modes[0] = SDL3_GetDesktopDisplayMode(displayID);
        c = 1;
    }
    if (count) {
        *count = c;
    }
    return modes;
}

SDL_DECLSPEC const char * SDLCALL
SDL_GetDisplayName(int displayIndex)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayName(displayID);
}

SDL_DECLSPEC int SDLCALL
SDL_GetDisplayBounds(int displayIndex, SDL_Rect *rect)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayBounds(displayID, rect) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_GetNumDisplayModes(int displayIndex)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    int count = 0;
    const SDL_DisplayMode **list;
    list = SDL_GetDisplayModeList(displayID, &count);
    if (list) {
        SDL3_free((void *) list);
        return count;
    }
    return -1;
}

SDL_DECLSPEC int SDLCALL
SDL_GetDisplayDPI(int displayIndex, float *ddpi, float *hdpi, float *vdpi)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    const SDL_DisplayMode *dp = SDL3_GetDesktopDisplayMode(displayID);
    float pixel_density = dp ? dp->pixel_density : 1.0f;
    float content_scale = SDL3_GetDisplayContentScale(displayID);
    float dpi;

    if (content_scale == 0.0f) {
        content_scale = 1.0f;
    }

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS)
    dpi = pixel_density * content_scale * 160.0f;
#else
    dpi = pixel_density * content_scale * 96.0f;
#endif

    if (hdpi) {
        *hdpi = dpi;
    }
    if (vdpi) {
        *vdpi = dpi;
    }
    if (ddpi) {
        *ddpi = dpi;
    }
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_GetDisplayUsableBounds(int displayIndex, SDL_Rect *rect)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetDisplayUsableBounds(displayID, rect) ? 0 : -1;
}

SDL_DECLSPEC SDL_DisplayOrientation SDLCALL
SDL_GetDisplayOrientation(int displayIndex)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    return SDL3_GetCurrentDisplayOrientation(displayID);
}

SDL_DECLSPEC int SDLCALL
SDL_GetDisplayMode(int displayIndex, int modeIndex, SDL2_DisplayMode *mode)
{
    SDL_DisplayID displayID = Display_IndexToID(displayIndex);
    const SDL_DisplayMode **list;
    int count = 0;
    int ret = -1;
    list = SDL_GetDisplayModeList(displayID, &count);
    if (modeIndex >= 0 && modeIndex < count) {
        if (mode) {
            DisplayMode_3to2(list[modeIndex], mode);
        }
        ret = 0;
    }
    SDL3_free((void *) list);
    return ret;
}

SDL_DECLSPEC int SDLCALL
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

SDL_DECLSPEC int SDLCALL
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

#define PROP_WINDOW_FULLSCREEN_MODE "sdl2-compat.window.fullscreen-mode"

static int
ApplyFullscreenMode(SDL_Window *window)
{
    SDL2_DisplayMode *property = (SDL2_DisplayMode *) SDL3_GetPointerProperty(SDL3_GetWindowProperties(window), PROP_WINDOW_FULLSCREEN_MODE, NULL);
    SDL_DisplayMode mode;
    SDL3_zero(mode);
    if (property) {
        /* Always try to enter fullscreen on the current display */
        const SDL_DisplayID displayID = SDL3_GetDisplayForWindow(window);

        /* The SDL2 refresh rate is rounded off, and SDL3 checks that the mode parameters match exactly, so try to find the closest matching SDL3 mode. */
        SDL3_GetClosestFullscreenDisplayMode(displayID, property->w, property->h, (float)property->refresh_rate, false, &mode);
    }
    if (mode.displayID && SDL3_SetWindowFullscreenMode(window, &mode)) {
        return 0;
    } else {
        int count = 0;
        SDL_DisplayMode **list;
        SDL_DisplayID displayID;
        int ret;

        displayID = SDL3_GetDisplayForWindow(window);
        if (!displayID) {
            displayID = SDL3_GetPrimaryDisplay();
        }

        /* FIXME: at least set a valid fullscreen mode */
        list = SDL3_GetFullscreenDisplayModes(displayID, &count);
        if (list && count) {
            ret = SDL3_SetWindowFullscreenMode(window, list[0]) ? 0 : -1;
        } else {
            /* If no exclusive modes, use the fullscreen desktop mode. */
            ret = SDL3_SetWindowFullscreenMode(window, NULL) ? 0 : -1;
        }
        SDL_free(list);
        return ret;
    }
}

SDL_DECLSPEC int SDLCALL
SDL_GetWindowDisplayMode(SDL_Window *window, SDL2_DisplayMode *mode)
{
    /* returns a pointer to the fullscreen mode to use or NULL for desktop mode */
    const SDL2_DisplayMode *dp;

    if (!window) {
        SDL3_SetError("Invalid window");
        return -1;
    }

    if (!mode) {
        SDL3_InvalidParamError("mode");
        return -1;
    }

    dp = (SDL2_DisplayMode *) SDL3_GetPointerProperty(SDL3_GetWindowProperties(window), PROP_WINDOW_FULLSCREEN_MODE, NULL);
    if (dp) {
        SDL3_copyp(mode, dp);
    } else {
        const SDL_DisplayMode *dp3;
        SDL_DisplayID displayID = SDL3_GetDisplayForWindow(window);
        if (!displayID) {
            displayID = SDL3_GetPrimaryDisplay();
        }

        /* Desktop mode */
        /* FIXME: is this correct ? */
        dp3 = SDL3_GetDesktopDisplayMode(displayID);
        if (dp3 == NULL) {
            return -1;
        }
        DisplayMode_3to2(dp3, mode);

        /* When returning the desktop mode, make sure the refresh is some nonzero value. */
        if (mode->refresh_rate == 0) {
            mode->refresh_rate = 60;
        }
    }

    return 0;
}

SDL_DECLSPEC SDL2_DisplayMode * SDLCALL
SDL_GetClosestDisplayMode(int displayIndex, const SDL2_DisplayMode *mode, SDL2_DisplayMode *closest)
{
    SDL_DisplayMode closest3;
    const SDL_DisplayMode *ret3 = NULL;
    const SDL_DisplayID displayID = Display_IndexToID(displayIndex);

    if (mode == NULL || closest == NULL) {
        SDL3_InvalidParamError("mode/closest");
        return NULL;
    }

    if (SDL3_GetClosestFullscreenDisplayMode(displayID, mode->w, mode->h, (float)mode->refresh_rate, false, &closest3)) {
        ret3 = &closest3;
    } else {
        /* Try the desktop mode */
        ret3 = SDL3_GetDesktopDisplayMode(displayID);
        if (!ret3 || ret3->w < mode->w || ret3->h < mode->h) {
            return NULL;
        }
    }

    DisplayMode_3to2(ret3, closest);

    /* Make sure the returned refresh rate and format are something valid */
    if (!closest->refresh_rate) {
        closest->refresh_rate = mode->refresh_rate ? mode->refresh_rate : 60;
    }
    if (!closest->format) {
        closest->format = SDL_PIXELFORMAT_XRGB8888;
    }
    return closest;
}

SDL_DECLSPEC int SDLCALL
SDL_SetWindowDisplayMode(SDL_Window *window, const SDL2_DisplayMode *mode)
{
    int result;

    /* Store the requested mode in case we aren't in full-screen exclusive mode yet (or ever) */
    if (mode) {
        SDL2_DisplayMode *property = (SDL2_DisplayMode *) SDL3_malloc(sizeof(*property));
        if (property) {
            SDL3_copyp(property, mode);
            result = SDL3_SetPointerPropertyWithCleanup(SDL3_GetWindowProperties(window), PROP_WINDOW_FULLSCREEN_MODE, property, CleanupFreeableProperty, NULL) ? 0 : -1;
        } else {
            result = -1;
        }
    } else {
        result = SDL3_SetPointerProperty(SDL3_GetWindowProperties(window), PROP_WINDOW_FULLSCREEN_MODE, NULL) ? 0 : -1;
    }

    /* If're we're in full-screen exclusive mode now, apply the new display mode */
    if ((SDL3_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) && SDL3_GetWindowFullscreenMode(window)) {
        result = ApplyFullscreenMode(window);
    }

    return result;
}

/* this came out of SDL2 directly. */
static SDL2_bool SDL_Vulkan_GetInstanceExtensions_Helper(unsigned int *userCount,
                                                 const char **userNames,
                                                 unsigned int nameCount,
                                                 const char *const *names)
{
    if (userNames) {
        unsigned i;

        if (*userCount < nameCount) {
            SDL3_SetError("Output array for SDL_Vulkan_GetInstanceExtensions needs to be at least %d big", nameCount);
            return SDL2_FALSE;
        }

        for (i = 0; i < nameCount; i++) {
            userNames[i] = names[i];
        }
    }
    *userCount = nameCount;
    return SDL2_TRUE;
}


/* SDL3 simplified SDL_Vulkan_GetInstanceExtensions() extensively. */
SDL_DECLSPEC SDL2_bool SDLCALL
SDL_Vulkan_GetInstanceExtensions(SDL_Window *window, unsigned int *puiCount, const char **pNames)
{
    Uint32 ui32count = 0;
    char const* const* extensions = NULL;

    (void) window;  /* Strictly speaking, SDL2 would report failure if this window was invalid, but we'll just go on until someone complains. */

    if (puiCount == NULL) {
        SDL3_InvalidParamError("count");
        return SDL2_FALSE;
    }

    extensions = SDL3_Vulkan_GetInstanceExtensions(&ui32count);
    if (!extensions) {
        return SDL2_FALSE;
    }

    return SDL_Vulkan_GetInstanceExtensions_Helper(puiCount, pNames, (unsigned int) ui32count, extensions);
}

/* SDL3 added a VkAllocationCallbacks* argument; SDL2 always uses the default (NULL) allocator */
/* SDL3 also changed the return type from SDL2_bool to int (with usual 0==success, -1==error semantics) */
SDL_DECLSPEC SDL2_bool SDLCALL
SDL_Vulkan_CreateSurface(SDL_Window *window, VkInstance vkinst, VkSurfaceKHR *psurf)
{
    return SDL3_Vulkan_CreateSurface(window, vkinst, NULL, psurf) ? SDL2_TRUE : SDL2_FALSE;
}


/* SDL3 doesn't have 3dNow or RDTSC. */
static int
CPU_haveCPUID(void)
{
    int has_CPUID = 0;

/* *INDENT-OFF* */
#ifndef SDL_CPUINFO_DISABLED
#if (defined(__GNUC__) || defined(__llvm__)) && defined(__i386__)
    __asm__ (
"        pushfl                      # Get original EFLAGS             \n"
"        popl    %%eax                                                 \n"
"        movl    %%eax,%%ecx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushl   %%eax               # Save new EFLAGS value on stack  \n"
"        popfl                       # Replace current EFLAGS value    \n"
"        pushfl                      # Get new EFLAGS                  \n"
"        popl    %%eax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
    : "=m" (has_CPUID)
    :
    : "%eax", "%ecx"
    );
#elif (defined(__GNUC__) || defined(__llvm__)) && defined(__x86_64__)
/* Technically, if this is being compiled under __x86_64__ then it has
   CPUid by definition.  But it's nice to be able to prove it.  :)      */
    __asm__ (
"        pushfq                      # Get original EFLAGS             \n"
"        popq    %%rax                                                 \n"
"        movq    %%rax,%%rcx                                           \n"
"        xorl    $0x200000,%%eax     # Flip ID bit in EFLAGS           \n"
"        pushq   %%rax               # Save new EFLAGS value on stack  \n"
"        popfq                       # Replace current EFLAGS value    \n"
"        pushfq                      # Get new EFLAGS                  \n"
"        popq    %%rax               # Store new EFLAGS in EAX         \n"
"        xorl    %%ecx,%%eax         # Can not toggle ID bit,          \n"
"        jz      1f                  # Processor=80486                 \n"
"        movl    $1,%0               # We have CPUID support           \n"
"1:                                                                    \n"
    : "=m" (has_CPUID)
    :
    : "%rax", "%rcx"
    );
#elif (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)
    __asm {
        pushfd                      ; Get original EFLAGS
        pop     eax
        mov     ecx, eax
        xor     eax, 200000h        ; Flip ID bit in EFLAGS
        push    eax                 ; Save new EFLAGS value on stack
        popfd                       ; Replace current EFLAGS value
        pushfd                      ; Get new EFLAGS
        pop     eax                 ; Store new EFLAGS in EAX
        xor     eax, ecx            ; Can not toggle ID bit,
        jz      done                ; Processor=80486
        mov     has_CPUID,1         ; We have CPUID support
done:
    }
#elif defined(_MSC_VER) && defined(_M_X64)
    has_CPUID = 1;
#elif defined(__sun) && defined(__i386)
    __asm (
"       pushfl                 \n"
"       popl    %eax           \n"
"       movl    %eax,%ecx      \n"
"       xorl    $0x200000,%eax \n"
"       pushl   %eax           \n"
"       popfl                  \n"
"       pushfl                 \n"
"       popl    %eax           \n"
"       xorl    %ecx,%eax      \n"
"       jz      1f             \n"
"       movl    $1,-8(%ebp)    \n"
"1:                            \n"
    );
#elif defined(__sun) && defined(__amd64)
    __asm (
"       pushfq                 \n"
"       popq    %rax           \n"
"       movq    %rax,%rcx      \n"
"       xorl    $0x200000,%eax \n"
"       pushq   %rax           \n"
"       popfq                  \n"
"       pushfq                 \n"
"       popq    %rax           \n"
"       xorl    %ecx,%eax      \n"
"       jz      1f             \n"
"       movl    $1,-8(%rbp)    \n"
"1:                            \n"
    );
#endif
#endif
/* *INDENT-ON* */
    return has_CPUID;
}

#if (defined(__GNUC__) || defined(__llvm__)) && defined(__i386__)
#define cpuid(func, a, b, c, d) \
    __asm__ __volatile__ ( \
"        pushl %%ebx        \n" \
"        xorl %%ecx,%%ecx   \n" \
"        cpuid              \n" \
"        movl %%ebx, %%esi  \n" \
"        popl %%ebx         \n" : \
            "=a" (a), "=S" (b), "=c" (c), "=d" (d) : "a" (func))
#elif (defined(__GNUC__) || defined(__llvm__)) && defined(__x86_64__)
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
#elif (defined(_MSC_VER) && defined(_M_X64))
/* Use __cpuidex instead of __cpuid because ICL does not clear ecx register */
#include <intrin.h>
#define cpuid(func, a, b, c, d) \
{ \
    int CPUInfo[4]; \
    __cpuidex(CPUInfo, func, 0); \
    a = CPUInfo[0]; \
    b = CPUInfo[1]; \
    c = CPUInfo[2]; \
    d = CPUInfo[3]; \
}
#else
#define cpuid(func, a, b, c, d) \
    do { a = b = c = d = 0; (void) a; (void) b; (void) c; (void) d; } while (0)
#endif

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_Has3DNow(void)
{
    /* if there's no MMX, presumably there's no 3DNow. */
    /* This lets SDL3 deal with the check for CPUID support _at all_ and eliminates non-AMD CPUs. */
    if (SDL3_HasMMX()) {
        int a, b, c, d;
        cpuid(0x80000000, a, b, c, d);
        if ((unsigned int)a >= 0x80000001) {
            cpuid(0x80000001, a, b, c, d);
            return (d & 0x80000000) ? SDL2_TRUE : SDL2_FALSE;
        }
    }
    return SDL2_FALSE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_HasRDTSC(void)
{
    static SDL2_bool checked = SDL2_FALSE;
    static SDL2_bool has_RDTSC = SDL2_FALSE;
    if (!checked) {
        checked = SDL2_TRUE;
        if (CPU_haveCPUID()) {
            int a, b, c, d;
            cpuid(0, a, b, c, d);
            if (a /* CPUIDMaxFunction */ >= 1) {
                cpuid(1, a, b, c, d);
                if (d & 0x00000010) {
                    has_RDTSC = SDL2_TRUE;
                }
            }
        }
    }
    return has_RDTSC;
}


/* This was always a basic wrapper over SDL_free; SDL3 removed it and says use SDL_free directly. */
SDL_DECLSPEC void SDLCALL
SDL_FreeWAV(Uint8 *audio_buf)
{
    SDL3_free(audio_buf);
}

/* SDL_WINDOW_FULLSCREEN_DESKTOP has been removed, and you can call
 * SDL_GetWindowFullscreenMode() to see whether an exclusive fullscreen
 * mode will be used or the fullscreen desktop mode will be used when
 * the window is fullscreen. */
/* SDL3 removed SDL_WINDOW_SHOWN as redundant to SDL_WINDOW_HIDDEN. */
/* SDL3 removed SDL_WINDOW_SKIP_TASKBAR as redundant to SDL_WINDOW_UTILITY. */
SDL_DECLSPEC Uint32 SDLCALL
SDL_GetWindowFlags(SDL_Window *window)
{
    Uint32 flags3 = (Uint32) SDL3_GetWindowFlags(window);
    Uint32 flags = (flags3 & ~(SDL2_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN | SDL2_WINDOW_FULLSCREEN_DESKTOP | SDL2_WINDOW_SKIP_TASKBAR));

    if ((flags3 & SDL_WINDOW_HIDDEN) == 0) {
        flags |= SDL2_WINDOW_SHOWN;
    }
    if ((flags3 & SDL_WINDOW_FULLSCREEN)) {
        if (SDL3_GetWindowFullscreenMode(window)) {
            flags |= SDL_WINDOW_FULLSCREEN;
        } else {
            flags |= SDL2_WINDOW_FULLSCREEN_DESKTOP;
        }
    }
    if (flags3 & SDL_WINDOW_UTILITY) {
        flags |= SDL2_WINDOW_SKIP_TASKBAR;
    }
    return flags;
}

#define PROP_WINDOW_PARENT_POINTER "sdl2-compat.window.parent"

SDL_DECLSPEC void* SDLCALL
SDL_GetWindowData(SDL_Window * window, const char *name)
{
    if (!window) {
        SDL3_SetError("Invalid window");
        return NULL;
    }

    if (name == NULL || name[0] == '\0') {
        SDL3_InvalidParamError("name");
        return NULL;
    }

    return SDL3_GetPointerProperty(SDL3_GetWindowProperties(window), name, NULL);
}

SDL_DECLSPEC void* SDLCALL
SDL_SetWindowData(SDL_Window * window, const char *name, void *userdata)
{
    void *prev;

    if (!window) {
        SDL3_SetError("Invalid window");
        return NULL;
    }

    if (name == NULL || name[0] == '\0') {
        SDL3_InvalidParamError("name");
        return NULL;
    }

    prev = SDL_GetWindowData(window, name);
    SDL3_SetPointerProperty(SDL3_GetWindowProperties(window), name, userdata);
    return prev;
}

#define PROP_TEXTURE_USERDATA_POINTER "sdl2-compat.texture.userdata"

SDL_DECLSPEC int SDLCALL
SDL_SetTextureUserData(SDL_Texture * texture, void *userdata)
{
    return SDL3_SetPointerProperty(SDL3_GetTextureProperties(texture), PROP_TEXTURE_USERDATA_POINTER, userdata) ? 0 : -1;
}

SDL_DECLSPEC void * SDLCALL
SDL_GetTextureUserData(SDL_Texture * texture)
{
    return SDL3_GetPointerProperty(SDL3_GetTextureProperties(texture), PROP_TEXTURE_USERDATA_POINTER, NULL);
}

static void
WindowPos2To3(int *x, int *y)
{
    /* Convert display indices to display IDs */
    if (SDL_WINDOWPOS_ISUNDEFINED(*x) || SDL_WINDOWPOS_ISCENTERED(*x)) {
      const int displayIndex = *x & 0xFFFF;
      const SDL_DisplayID displayID = Display_IndexToID(displayIndex);

      *x = (*x & 0xFFFF0000) | (0xFFFF & displayID);
    }
    if (SDL_WINDOWPOS_ISUNDEFINED(*y) || SDL_WINDOWPOS_ISCENTERED(*y)) {
      const int displayIndex = *y & 0xFFFF;
      const SDL_DisplayID displayID = Display_IndexToID(displayIndex);

      *y = (*y & 0xFFFF0000) | (0xFFFF & displayID);
    }
}

SDL_DECLSPEC SDL_Window * SDLCALL
SDL_CreateWindow(const char *title, int x, int y, int w, int h, Uint32 flags)
{
    SDL_Window *window = NULL;
    const Uint32 is_popup = flags & (SDL_WINDOW_POPUP_MENU | SDL_WINDOW_TOOLTIP);

    CheckEventFilter();

    if (flags & SDL2_WINDOW_FULLSCREEN_DESKTOP) {
        flags &= ~SDL2_WINDOW_FULLSCREEN_DESKTOP;
        flags |= SDL_WINDOW_FULLSCREEN; /* This is fullscreen desktop for new windows */
    }
    if (flags & SDL2_WINDOW_SKIP_TASKBAR) {
        flags &= ~SDL2_WINDOW_SKIP_TASKBAR;
        flags |= SDL_WINDOW_UTILITY;
    }

    if (!is_popup) {
        SDL_PropertiesID props = SDL3_CreateProperties();

        WindowPos2To3(&x, &y);
        if (title && *title) {
            SDL3_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, title);
        }
        SDL3_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, x);
        SDL3_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, y);
        SDL3_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, w);
        SDL3_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, h);
        SDL3_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_FLAGS_NUMBER, flags);
        SDL3_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_EXTERNAL_GRAPHICS_CONTEXT_BOOLEAN, SDL3_GetHintBoolean("SDL_VIDEO_EXTERNAL_CONTEXT", false));

        window = SDL3_CreateWindowWithProperties(props);
        SDL3_DestroyProperties(props);
    } else {
        SDL_Window *parent = SDL3_GetMouseFocus();
        if (!parent) {
            parent = SDL3_GetKeyboardFocus();
        }

        if (parent) {
            /* UE5 mixes the utility flag with popup flags, which SDL3 does not allow. */
            flags &= ~SDL_WINDOW_UTILITY;

            window = SDL3_CreatePopupWindow(parent, x, y, w, h, flags);
            SDL_SetWindowData(window, PROP_WINDOW_PARENT_POINTER, parent);
        }
    }

    #if !defined(SDL_PLATFORM_IOS) && !defined(SDL_PLATFORM_ANDROID)  // (and maybe others...?)
    SDL3_StartTextInput(window);
    #endif

    return window;
}

SDL_DECLSPEC int SDLCALL
SDL_CreateWindowAndRenderer(int width, int height, Uint32 window_flags,
                            SDL_Window **window, SDL_Renderer **renderer)
{
    // This function's code is exactly what SDL2 does (including not checking that `window` and `renderer` aren't NULL),
    //  but we want to make sure these go through the sdl2-compat CreateWindow and CreateRenderer functions
    //  to do some compatibility magic, instead of just calling the SDL3 equivalent of this function.

    *window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED,
                               width, height, window_flags);
    if (!*window) {
        *renderer = NULL;
        return -1;
    }

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if (!*renderer) {
        return -1;
    }

    return 0;
}

SDL_DECLSPEC SDL_Window * SDLCALL
SDL_CreateWindowFrom(const void *data)
{
    SDL_Window *window;
    const char *hint;
    SDL_PropertiesID props;

    CheckEventFilter();

    props = SDL3_CreateProperties();
    if (!props) {
        return NULL;
    }

    hint = SDL3_GetHint("SDL_VIDEO_WINDOW_SHARE_PIXEL_FORMAT");
    if (hint) {
        /* This hint is a pointer (in string form) of the address of
           the window to share a pixel format with
        */
        void *otherWindow = NULL; /* SDL_Window* */
        (void)SDL3_sscanf(hint, "%p", &otherWindow);
        SDL3_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_PIXEL_FORMAT_HWND_POINTER, SDL3_GetPointerProperty(SDL3_GetWindowProperties((SDL_Window *)otherWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL));
        SDL3_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    }

    if (SDL3_GetHintBoolean("SDL_VIDEO_FOREIGN_WINDOW_OPENGL", false)) {
        SDL3_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    }
    if (SDL3_GetHintBoolean("SDL_VIDEO_FOREIGN_WINDOW_VULKAN", false)) {
        SDL3_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
    }

    SDL3_SetPointerProperty(props, "sdl2-compat.external_window", (void *)data);
    window = SDL3_CreateWindowWithProperties(props);
    SDL3_DestroyProperties(props);
    return window;
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_GetWindowPixelFormat(SDL_Window *window)
{
    return (Uint32)SDL3_GetWindowPixelFormat(window);
}

SDL_DECLSPEC int SDLCALL
SDL_SetWindowFullscreen(SDL_Window *window, Uint32 flags)
{
    int ret = 0;

    if (flags == SDL2_WINDOW_FULLSCREEN_DESKTOP) {
        ret = SDL3_SetWindowFullscreenMode(window, NULL) ? 0 : -1;
    } else if (flags == SDL_WINDOW_FULLSCREEN) {
        ret = ApplyFullscreenMode(window);
    }

    if (ret == 0) {
        ret = SDL3_SetWindowFullscreen(window, (flags & SDL2_WINDOW_FULLSCREEN_DESKTOP) != 0) ? 0 : -1;
    }

    return ret;
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowTitle(SDL_Window *window, const char *title)
{
    SDL3_SetWindowTitle(window, title);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowIcon(SDL_Window *window, SDL2_Surface *icon)
{
    SDL3_SetWindowIcon(window, Surface2to3(icon));
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowSize(SDL_Window *window, int w, int h)
{
    SDL3_SetWindowSize(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowMinimumSize(SDL_Window *window, int min_w, int min_h)
{
    if (!window) {
        SDL3_SetError("Invalid window");
        return;
    }
    if (min_w <= 0) {
        SDL3_InvalidParamError("min_w");
        return;
    }
    if (min_h <= 0) {
        SDL3_InvalidParamError("min_h");
        return;
    }
    SDL3_SetWindowMinimumSize(window, min_w, min_h);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowMaximumSize(SDL_Window *window, int max_w, int max_h)
{
    if (!window) {
        SDL3_SetError("Invalid window");
        return;
    }
    if (max_w <= 0) {
        SDL3_InvalidParamError("max_w");
        return;
    }
    if (max_h <= 0) {
        SDL3_InvalidParamError("max_h");
        return;
    }
    SDL3_SetWindowMaximumSize(window, max_w, max_h);
}

SDL_DECLSPEC int SDLCALL
SDL_SetWindowModalFor(SDL_Window *modal_window, SDL_Window *parent_window)
{
    if (!modal_window) {
        SDL3_SetError("Invalid window");
        return -1;
    }
    if (SDL3_GetWindowFlags(modal_window) & SDL_WINDOW_MODAL) {
        SDL3_SetWindowModal(modal_window, false);
    }
    if (SDL3_SetWindowParent(modal_window, parent_window)) {
        int ret = 0;
        if (parent_window) {
            ret = SDL3_SetWindowModal(modal_window, true) ? 0 : -1;
        }
        return ret;
    }

    return -1;
}

SDL_DECLSPEC void SDLCALL
SDL_JoystickSetPlayerIndex(SDL_Joystick *joystick, int player_index)
{
    SDL3_SetJoystickPlayerIndex(joystick, player_index);
}

SDL_DECLSPEC void SDLCALL
SDL_StartTextInput(void)
{
    SDL_Window **windows = SDL3_GetWindows(NULL);
    if (windows) {
        int i;
        SDL_PropertiesID props = SDL3_CreateProperties();

        SDL3_SetNumberProperty(props, SDL_PROP_TEXTINPUT_TYPE_NUMBER, SDL_TEXTINPUT_TYPE_TEXT);
        SDL3_SetNumberProperty(props, SDL_PROP_TEXTINPUT_CAPITALIZATION_NUMBER, SDL_CAPITALIZE_NONE);
        SDL3_SetBooleanProperty(props, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);

        for (i = 0; windows[i]; ++i) {
            SDL3_StartTextInputWithProperties(windows[i], props);
        }
        SDL3_DestroyProperties(props);

        SDL_free(windows);
    }
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_IsTextInputActive(void)
{
    SDL2_bool result = SDL2_FALSE;
    SDL_Window **windows = SDL3_GetWindows(NULL);
    if (windows) {
        int i;

        for (i = 0; windows[i]; ++i) {
            if (SDL3_TextInputActive(windows[i])) {
                result = SDL2_TRUE;
                break;
            }
        }
        SDL_free(windows);
    }
    return result;
}

SDL_DECLSPEC void SDLCALL
SDL_StopTextInput(void)
{
    SDL_Window **windows = SDL3_GetWindows(NULL);
    if (windows) {
        int i;

        for (i = 0; windows[i]; ++i) {
            SDL3_StopTextInput(windows[i]);
        }
        SDL_free(windows);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_ClearComposition(void)
{
    SDL_Window **windows = SDL3_GetWindows(NULL);
    if (windows) {
        int i;

        for (i = 0; windows[i]; ++i) {
            SDL3_ClearComposition(windows[i]);
        }
        SDL_free(windows);
    }
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_IsTextInputShown()
{
    return SDL_IsTextInputActive();
}

SDL_DECLSPEC void SDLCALL
SDL_SetTextInputRect(const SDL_Rect *rect)
{
    SDL_Window **windows;

    if (!rect) {
        SDL3_InvalidParamError("rect");
        return;
    }

    windows = SDL3_GetWindows(NULL);
    if (windows) {
        int i;

        for ( i = 0; windows[i]; ++i ) {
            SDL3_SetTextInputArea(windows[i], rect, 0);
        }
        SDL_free(windows);
    }
}

SDL_DECLSPEC void SDLCALL
SDL_SetCursor(SDL_Cursor * cursor)
{
    SDL3_SetCursor(cursor);
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_PixelFormatEnumToMasks(Uint32 format, int *bpp, Uint32 * Rmask, Uint32 * Gmask, Uint32 * Bmask, Uint32 * Amask)
{
    return SDL3_GetMasksForPixelFormat((SDL_PixelFormat)format, bpp, Rmask, Gmask, Bmask, Amask) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_MasksToPixelFormatEnum(int bpp, Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
    return (Uint32)SDL3_GetPixelFormatForMasks(bpp, Rmask, Gmask, Bmask, Amask);
}

SDL_DECLSPEC const char *
SDL_GetPixelFormatName(Uint32 format)
{
    switch (format) {
    case SDL_PIXELFORMAT_XRGB8888:
        return "SDL_PIXELFORMAT_RGB888";
    case SDL_PIXELFORMAT_XBGR8888:
        return "SDL_PIXELFORMAT_BGR888";
    case SDL_PIXELFORMAT_XRGB4444:
        return "SDL_PIXELFORMAT_RGB444";
    case SDL_PIXELFORMAT_XBGR4444:
        return "SDL_PIXELFORMAT_BGR444";
    case SDL_PIXELFORMAT_XRGB1555:
        return "SDL_PIXELFORMAT_RGB555";
    case SDL_PIXELFORMAT_XBGR1555:
        return "SDL_PIXELFORMAT_BGR555";
    default:
        return SDL3_GetPixelFormatName((SDL_PixelFormat)format);
    }
}

SDL_DECLSPEC SDL2_PixelFormat * SDLCALL
SDL_AllocFormat(Uint32 pixel_format)
{
    SDL2_PixelFormat *format;

    const SDL_PixelFormatDetails *details = SDL3_GetPixelFormatDetails((SDL_PixelFormat)pixel_format);
    if (!details) {
        return NULL;
    }

    /* Allocate an empty pixel format structure, and initialize it */
    format = (SDL2_PixelFormat *)SDL3_calloc(1, sizeof(*format));
    format->format = details->format;
    format->BitsPerPixel = details->bits_per_pixel;
    format->BytesPerPixel = details->bytes_per_pixel;
    format->Rmask = details->Rmask;
    format->Gmask = details->Gmask;
    format->Bmask = details->Bmask;
    format->Amask = details->Amask;
    format->Rloss = (8 - details->Rbits);
    format->Gloss = (8 - details->Gbits);
    format->Bloss = (8 - details->Bbits);
    format->Aloss = (8 - details->Abits);
    format->Rshift = details->Rshift;
    format->Gshift = details->Gshift;
    format->Bshift = details->Bshift;
    format->Ashift = details->Ashift;
    format->refcount = 1;

    return format;
}

SDL_DECLSPEC void SDLCALL
SDL_FreeFormat(SDL2_PixelFormat *format)
{
    if (!format) {
        SDL3_InvalidParamError("format");
        return;
    }

    if (format->palette) {
        SDL_FreePalette(format->palette);
    }
    SDL_free(format);
}

SDL_DECLSPEC void SDLCALL
SDL_FreePalette(SDL_Palette *palette)
{
    SDL3_DestroyPalette(palette);
}

SDL_DECLSPEC void SDLCALL
SDL_UnionRect(const SDL_Rect *A, const SDL_Rect *B, SDL_Rect *result)
{
    SDL3_GetRectUnion(A, B, result);
}

SDL_DECLSPEC void SDLCALL
SDL_UnionFRect(const SDL_FRect *A, const SDL_FRect *B, SDL_FRect *result)
{
    SDL3_GetRectUnionFloat(A, B, result);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowPosition(SDL_Window *window, int x, int y)
{
    /* Popup windows need to be transformed from global to relative coordinates. */
    if (SDL3_GetWindowFlags(window) & (SDL_WINDOW_TOOLTIP | SDL_WINDOW_POPUP_MENU)) {
        SDL_Window *parent = (SDL_Window *) SDL_GetWindowData(window, PROP_WINDOW_PARENT_POINTER);

        while (parent) {
            int x_off, y_off;
            SDL3_GetWindowPosition(parent, &x_off, &y_off);

            x -= x_off;
            y -= y_off;

            parent = (SDL_Window *) SDL_GetWindowData(parent, PROP_WINDOW_PARENT_POINTER);
        }
    } else {
        WindowPos2To3(&x, &y);
    }

    SDL3_SetWindowPosition(window, x, y);
}

SDL_DECLSPEC void SDLCALL
SDL_GetWindowPosition(SDL_Window *window, int *x, int *y)
{
    SDL3_GetWindowPosition(window, x, y);

    /* Popup windows need to be transformed from relative to global coordinates. */
    if (SDL3_GetWindowFlags(window) & (SDL_WINDOW_TOOLTIP | SDL_WINDOW_POPUP_MENU)) {
        SDL_Window *parent = (SDL_Window *) SDL_GetWindowData(window, PROP_WINDOW_PARENT_POINTER);

        while (parent) {
            int x_off, y_off;
            SDL3_GetWindowPosition(parent, &x_off, &y_off);

            *x += x_off;
            *y += y_off;

            parent = (SDL_Window *) SDL_GetWindowData(parent, PROP_WINDOW_PARENT_POINTER);
        }
    }
}

SDL_DECLSPEC void SDLCALL
SDL_GetWindowSize(SDL_Window *window, int *w, int *h)
{
    SDL3_GetWindowSize(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_GetWindowSizeInPixels(SDL_Window *window, int *w, int *h)
{
    SDL3_GetWindowSizeInPixels(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_GetWindowMinimumSize(SDL_Window *window, int *w, int *h)
{
    SDL3_GetWindowMinimumSize(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_GetWindowMaximumSize(SDL_Window *window, int *w, int *h)
{
    SDL3_GetWindowMaximumSize(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowBordered(SDL_Window *window, SDL2_bool bordered)
{
    SDL3_SetWindowBordered(window, bordered);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowResizable(SDL_Window *window, SDL2_bool resizable)
{
    SDL3_SetWindowResizable(window, resizable);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowAlwaysOnTop(SDL_Window *window, SDL2_bool on_top)
{
    SDL3_SetWindowAlwaysOnTop(window, on_top);
}

SDL_DECLSPEC void SDLCALL
SDL_ShowWindow(SDL_Window *window)
{
    SDL3_ShowWindow(window);
}

SDL_DECLSPEC void SDLCALL
SDL_HideWindow(SDL_Window *window)
{
    SDL3_HideWindow(window);
}

SDL_DECLSPEC void SDLCALL
SDL_RaiseWindow(SDL_Window *window)
{
    SDL3_RaiseWindow(window);
}

SDL_DECLSPEC int SDLCALL
SDL_SetWindowInputFocus(SDL_Window *window)
{
    return SDL3_RaiseWindow(window);
}

SDL_DECLSPEC void SDLCALL
SDL_MaximizeWindow(SDL_Window *window)
{
    SDL3_MaximizeWindow(window);
}

SDL_DECLSPEC void SDLCALL
SDL_MinimizeWindow(SDL_Window *window)
{
    SDL3_MinimizeWindow(window);
}

SDL_DECLSPEC void SDLCALL
SDL_RestoreWindow(SDL_Window *window)
{
    SDL3_RestoreWindow(window);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowGrab(SDL_Window *window, SDL2_bool grabbed)
{
    SDL3_SetWindowMouseGrab(window, grabbed);
    if (SDL3_GetHintBoolean("SDL_GRAB_KEYBOARD", false)) {
        SDL3_SetWindowKeyboardGrab(window, grabbed);
    }
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GetWindowGrab(SDL_Window *window)
{
    return SDL3_GetWindowKeyboardGrab(window) || SDL3_GetWindowMouseGrab(window) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowKeyboardGrab(SDL_Window *window, SDL2_bool grabbed)
{
    SDL3_SetWindowKeyboardGrab(window, grabbed);
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowMouseGrab(SDL_Window *window, SDL2_bool grabbed)
{
    SDL3_SetWindowMouseGrab(window, grabbed);
}

/* SDL3 added a return value and renamed this. We just throw the value away for SDL2. */
SDL_DECLSPEC void SDLCALL
SDL_GL_DeleteContext(SDL_GLContext context)
{
    (void) SDL3_GL_DestroyContext(context);
}

SDL_DECLSPEC void SDLCALL
SDL_EnableScreenSaver(void)
{
    SDL3_EnableScreenSaver();
}

SDL_DECLSPEC void SDLCALL
SDL_DisableScreenSaver(void)
{
    SDL3_DisableScreenSaver();
}

/* SDL3 added a return value. We just throw it away for SDL2. */
SDL_DECLSPEC void SDLCALL
SDL_GL_SwapWindow(SDL_Window *window)
{
    (void) SDL3_GL_SwapWindow(window);
}

typedef void (GLAPIENTRY *openglfn_glEnable_t)(GLenum what);
typedef void (GLAPIENTRY *openglfn_glDisable_t)(GLenum what);
typedef void (GLAPIENTRY *openglfn_glActiveTexture_t)(GLenum what);
typedef void (GLAPIENTRY *openglfn_glBindTexture_t)(GLenum target, GLuint name);
typedef openglfn_glActiveTexture_t openglfn_glActiveTextureARB_t;

static void *getglfn(const char *fn, bool *okay)
{
    void *retval = NULL;
    if (*okay) {
        retval = SDL3_GL_GetProcAddress(fn);
        if (retval == NULL) {
            *okay = false;
            SDL3_SetError("Failed to find GL proc address of %s", fn);
        }
    }
    return retval;
}

#define GLFN(fn) openglfn_##fn##_t p##fn = (openglfn_##fn##_t) getglfn(#fn, &okay)


SDL_DECLSPEC int SDLCALL
SDL_GL_BindTexture(SDL_Texture *texture, float *texw, float *texh)
{
    SDL_PropertiesID props;
    SDL_Renderer *renderer;
    Sint64 tex;

    /* SDL3_GetRendererFromTexture will do all the CHECK_TEXTURE_MAGIC stuff. */
    renderer = SDL3_GetRendererFromTexture(texture);
    if (!renderer) {
        return -1;
    }

    props = SDL3_GetTextureProperties(texture);
    if (!props) {
        return -1;
    }

    /* always flush the renderer here; good enough. SDL2 only flushed if the texture might have changed, but we'll be conservative. */
    SDL3_FlushRenderer(renderer);

    if ((tex = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER, -1)) != -1) {  // opengl renderer.
        const Sint64 target = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_TARGET_NUMBER, 0);
        const Sint64 uv = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_UV_NUMBER, 0);
        const Sint64 u = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_U_NUMBER, 0);
        const Sint64 v = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_V_NUMBER, 0);

        bool okay = true;
        GLFN(glEnable);
        GLFN(glActiveTextureARB);
        GLFN(glBindTexture);

        if (!okay) {
            return -1;
        }

        pglEnable((GLenum) target);

        if (u && v) {
            pglActiveTextureARB(GL_TEXTURE2_ARB);
            pglBindTexture((GLenum) target, (GLuint) v);
            pglActiveTextureARB(GL_TEXTURE1_ARB);
            pglBindTexture((GLenum) target, (GLuint) u);
            pglActiveTextureARB(GL_TEXTURE0_ARB);
        } else if (uv) {
            pglActiveTextureARB(GL_TEXTURE1_ARB);
            pglBindTexture((GLenum) target, (GLuint) uv);
            pglActiveTextureARB(GL_TEXTURE0_ARB);
        }
        pglBindTexture((GLenum) target, (GLuint) tex);

        if (texw) {
            *texw = SDL3_GetFloatProperty(props, SDL_PROP_TEXTURE_OPENGL_TEX_W_FLOAT, 1.0f);
        }
        if (texh) {
            *texh = SDL3_GetFloatProperty(props, SDL_PROP_TEXTURE_OPENGL_TEX_H_FLOAT, 1.0f);
        }
    } else if ((tex = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_NUMBER, -1)) != -1) {  // opengles2 renderer.
        const Sint64 target = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_TARGET_NUMBER, 0);
        const Sint64 uv = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_UV_NUMBER, 0);
        const Sint64 u = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_U_NUMBER, 0);
        const Sint64 v = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_V_NUMBER, 0);

        bool okay = true;
        GLFN(glActiveTexture);
        GLFN(glBindTexture);

        if (!okay) {
            return -1;
        }

        if (u && v) {
            pglActiveTexture(GL_TEXTURE2);
            pglBindTexture((GLenum) target, (GLuint) v);
            pglActiveTexture(GL_TEXTURE1);
            pglBindTexture((GLenum) target, (GLuint) u);
            pglActiveTexture(GL_TEXTURE0);
        } else if (uv) {
            pglActiveTexture(GL_TEXTURE1);
            pglBindTexture((GLenum) target, (GLuint) uv);
            pglActiveTexture(GL_TEXTURE0);
        }
        pglBindTexture((GLenum) target, (GLuint) tex);

        if (texw) {
            *texw = 1.0f;
        }
        if (texh) {
            *texh = 1.0f;
        }
    }

    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_GL_UnbindTexture(SDL_Texture *texture)
{
    SDL_PropertiesID props;
    SDL_Renderer *renderer;
    Sint64 tex;

    /* SDL3_GetRendererFromTexture will do all the CHECK_TEXTURE_MAGIC stuff. */
    renderer = SDL3_GetRendererFromTexture(texture);
    if (!renderer) {
        return -1;
    }

    props = SDL3_GetTextureProperties(texture);
    if (!props) {
        return -1;
    }

    if ((tex = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_NUMBER, -1)) != -1) {  // opengl renderer.
        const Sint64 target = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_TARGET_NUMBER, 0);
        const Sint64 uv = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_UV_NUMBER, 0);
        const Sint64 u = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_U_NUMBER, 0);
        const Sint64 v = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGL_TEXTURE_V_NUMBER, 0);

        bool okay = true;
        GLFN(glDisable);
        GLFN(glActiveTextureARB);
        GLFN(glBindTexture);

        if (!okay) {
            return -1;
        }

        if (u && v) {
            pglActiveTextureARB(GL_TEXTURE2_ARB);
            pglBindTexture((GLenum) target, 0);
            pglDisable((GLenum) target);
            pglActiveTextureARB(GL_TEXTURE1_ARB);
            pglBindTexture((GLenum) target, 0);
            pglDisable((GLenum) target);
            pglActiveTextureARB(GL_TEXTURE0_ARB);
        } else if (uv) {
            pglActiveTextureARB(GL_TEXTURE1_ARB);
            pglBindTexture((GLenum) target, 0);
            pglDisable((GLenum) target);
            pglActiveTextureARB(GL_TEXTURE0_ARB);
        }
        pglBindTexture((GLenum) target, 0);
        pglDisable((GLenum) target);
    } else if ((tex = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_NUMBER, -1)) != -1) {  // opengles2 renderer.
        const Sint64 target = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_TARGET_NUMBER, 0);
        const Sint64 uv = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_UV_NUMBER, 0);
        const Sint64 u = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_U_NUMBER, 0);
        const Sint64 v = SDL3_GetNumberProperty(props, SDL_PROP_TEXTURE_OPENGLES2_TEXTURE_V_NUMBER, 0);

        bool okay = true;
        GLFN(glActiveTexture);
        GLFN(glBindTexture);

        if (!okay) {
            return -1;
        }

        if (u && v) {
            pglActiveTexture(GL_TEXTURE2);
            pglBindTexture((GLenum) target, 0);
            pglActiveTexture(GL_TEXTURE1);
            pglBindTexture((GLenum) target, 0);
            pglActiveTexture(GL_TEXTURE0);
        } else if (uv) {
            pglActiveTexture(GL_TEXTURE1);
            pglBindTexture((GLenum) target, 0);
            pglActiveTexture(GL_TEXTURE0);
        }
        pglBindTexture((GLenum) target, 0);
    }

    /* always flush the renderer here, in case of app shenanigans. */
    SDL3_FlushRenderer(renderer);

    return 0;
}

#undef GLFN

SDL_DECLSPEC void SDLCALL
SDL_GetClipRect(SDL2_Surface *surface, SDL_Rect *rect)
{
    SDL3_GetSurfaceClipRect(Surface2to3(surface), rect);
}

SDL_DECLSPEC SDL_Cursor * SDLCALL
SDL_CreateColorCursor(SDL2_Surface *surface, int hot_x, int hot_y)
{
    return SDL3_CreateColorCursor(Surface2to3(surface), hot_x, hot_y);
}

SDL_DECLSPEC int SDLCALL
SDL_SetPixelFormatPalette(SDL2_PixelFormat *format, SDL_Palette *palette)
{
    if (!format) {
        SDL3_InvalidParamError("SDL_SetPixelFormatPalette(): format");
        return -1;
    }

    if (palette && palette->ncolors > (1 << format->BitsPerPixel)) {
        SDL3_SetError("SDL_SetPixelFormatPalette() passed a palette that doesn't match the format");
        return -1;
    }

    if (format->palette == palette) {
        return 0;
    }

    if (format->palette) {
        SDL_FreePalette(format->palette);
    }

    format->palette = palette;

    if (format->palette) {
        ++format->palette->refcount;
    }

    return 0;
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_MapRGB(const SDL2_PixelFormat *format2, Uint8 r, Uint8 g, Uint8 b)
{
    const SDL_PixelFormatDetails *format = SDL3_GetPixelFormatDetails((SDL_PixelFormat)format2->format);
    if (!format) {
        return 0;
    }
    return SDL3_MapRGB(format, format2->palette, r, g, b);
}

SDL_DECLSPEC Uint32 SDLCALL
SDL_MapRGBA(const SDL2_PixelFormat *format2, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
    const SDL_PixelFormatDetails *format = SDL3_GetPixelFormatDetails((SDL_PixelFormat)format2->format);
    if (!format) {
        return 0;
    }
    return SDL3_MapRGBA(format, format2->palette, r, g, b, a);
}

SDL_DECLSPEC void SDLCALL
SDL_GetRGB(Uint32 pixel, const SDL2_PixelFormat *format2, Uint8 * r, Uint8 * g, Uint8 * b)
{
    const SDL_PixelFormatDetails *format = SDL3_GetPixelFormatDetails((SDL_PixelFormat)format2->format);
    if (!format) {
        *r = *g = *b = 0;
        return;
    }
    SDL3_GetRGB(pixel, format, format2->palette, r, g, b);
}

SDL_DECLSPEC void SDLCALL
SDL_GetRGBA(Uint32 pixel, const SDL2_PixelFormat *format2, Uint8 * r, Uint8 * g, Uint8 * b, Uint8 *a)
{
    const SDL_PixelFormatDetails *format = SDL3_GetPixelFormatDetails((SDL_PixelFormat)format2->format);
    if (!format) {
        *r = *g = *b = *a = 0;
        return;
    }
    SDL3_GetRGBA(pixel, format, format2->palette, r, g, b, a);
}

SDL_DECLSPEC SDL_Renderer * SDLCALL
SDL_CreateSoftwareRenderer(SDL2_Surface *surface)
{
    return SDL3_CreateSoftwareRenderer(Surface2to3(surface));
}

SDL_DECLSPEC int SDLCALL
SDL_SetSurfacePalette(SDL2_Surface *surface, SDL_Palette *palette)
{
    if (!surface) {
        SDL3_InvalidParamError("SDL_SetSurfacePalette(): surface");
        return -1;
    }
    if (SDL_SetPixelFormatPalette(surface->format, palette) < 0) {
        return -1;
    }
    if (!SDL3_SetSurfacePalette(Surface2to3(surface), palette)) {
        return -1;
    }
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_LockSurface(SDL2_Surface *surface)
{
    if (!surface->locked) {
        /* Perform the lock */
        SDL_Surface *surface3 = Surface2to3(surface);
        if (!SDL3_LockSurface(surface3)) {
            return -1;
        }
        surface->pixels = surface3->pixels;
        surface->pitch = surface3->pitch;
    }

    /* Increment the surface lock count, for recursive locks */
    ++surface->locked;

    /* Ready to go.. */
    return 0;
}

SDL_DECLSPEC void SDLCALL
SDL_UnlockSurface(SDL2_Surface *surface)
{
    SDL_Surface *surface3 = Surface2to3(surface);

    /* Only perform an unlock if we are locked */
    if (!surface->locked || (--surface->locked > 0)) {
        return;
    }

    SDL3_UnlockSurface(surface3);

    surface->pixels = surface3->pixels;
    surface->pitch = surface3->pitch;
}

SDL_DECLSPEC int SDLCALL
SDL_SetSurfaceRLE(SDL2_Surface *surface, int flag)
{
    SDL_Surface *surface3 = Surface2to3(surface);
    if (!surface3) {
        SDL3_InvalidParamError("surface");
        return -1;
    }

    if (!SDL3_SetSurfaceRLE(surface3, flag ? true : false)) {
        return -1;
    }

    if (SDL3_SurfaceHasRLE(surface3)) {
        surface->flags |= SDL_RLEACCEL;
    } else {
        surface->flags &= ~SDL_RLEACCEL;
    }
    return 0;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_HasSurfaceRLE(SDL2_Surface *surface)
{
    return SDL3_SurfaceHasRLE(Surface2to3(surface)) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC int SDLCALL
SDL_SetColorKey(SDL2_Surface *surface, int flag, Uint32 key)
{
    if (flag & SDL_RLEACCEL) {
        SDL_SetSurfaceRLE(surface, 1);
    }
    return SDL3_SetSurfaceColorKey(Surface2to3(surface), flag ? true : false, key) ? 0 : -1;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_HasColorKey(SDL2_Surface *surface)
{
    return SDL3_SurfaceHasColorKey(Surface2to3(surface)) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC int SDLCALL
SDL_GetColorKey(SDL2_Surface *surface, Uint32 *key)
{
    return SDL3_GetSurfaceColorKey(Surface2to3(surface), key) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_SetSurfaceColorMod(SDL2_Surface *surface, Uint8 r, Uint8 g, Uint8 b)
{
    return SDL3_SetSurfaceColorMod(Surface2to3(surface), r, g, b) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_GetSurfaceColorMod(SDL2_Surface *surface, Uint8 *r, Uint8 *g, Uint8 *b)
{
    return SDL3_GetSurfaceColorMod(Surface2to3(surface), r, g, b) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_SetSurfaceAlphaMod(SDL2_Surface *surface, Uint8 alpha)
{
    return SDL3_SetSurfaceAlphaMod(Surface2to3(surface), alpha) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_GetSurfaceAlphaMod(SDL2_Surface *surface, Uint8 *alpha)
{
    return SDL3_GetSurfaceAlphaMod(Surface2to3(surface), alpha) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_SetSurfaceBlendMode(SDL2_Surface *surface, SDL_BlendMode blendMode)
{
    return SDL3_SetSurfaceBlendMode(Surface2to3(surface), blendMode) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_GetSurfaceBlendMode(SDL2_Surface *surface, SDL_BlendMode *blendMode)
{
    return SDL3_GetSurfaceBlendMode(Surface2to3(surface), blendMode) ? 0 : -1;
}

SDL_DECLSPEC SDL2_Surface * SDLCALL
SDL_GetWindowSurface(SDL_Window *window)
{
    return Surface3to2(SDL3_GetWindowSurface(window));
}

SDL_DECLSPEC int SDLCALL
SDL_LockTextureToSurface(SDL_Texture *texture, const SDL_Rect *rect, SDL2_Surface **surface)
{
    SDL_Surface *surface3 = NULL;
    if (!SDL3_LockTextureToSurface(texture, rect, &surface3)) {
        return -1;
    }
    *surface = Surface3to2(surface3);
    return 0;
}


SDL_DECLSPEC void SDLCALL
SDL_GameControllerSetPlayerIndex(SDL_GameController *gamecontroller, int player_index)
{
    SDL3_SetGamepadPlayerIndex(gamecontroller, player_index);
}

SDL_DECLSPEC SDL_GUID SDLCALL
SDL_JoystickGetGUIDFromString(const char *pchGUID)
{
    return SDL3_StringToGUID(pchGUID);
}

SDL_DECLSPEC void SDLCALL
SDL_JoystickGetGUIDString(SDL_GUID guid, char *pszGUID, int cbGUID)
{
    SDL3_GUIDToString(guid, pszGUID, cbGUID);
}

/* SDL3 split this into getter/setter functions. */
SDL_DECLSPEC Uint8 SDLCALL
SDL_EventState(Uint32 type, int state)
{
    const int retval = SDL3_EventEnabled(type) ? SDL2_ENABLE : SDL2_DISABLE;
    if (state == SDL2_ENABLE) {
        SDL3_SetEventEnabled(type, true);
    } else if (state == SDL2_DISABLE) {
        SDL3_SetEventEnabled(type, false);
    }
    return retval;
}

/* SDL3 split this into getter/setter functions. */
SDL_DECLSPEC int SDLCALL
SDL_ShowCursor(int state)
{
    int retval = SDL3_CursorVisible() ? SDL2_ENABLE : SDL2_DISABLE;
    if ((state == SDL2_ENABLE) && !SDL3_ShowCursor()) {
        retval = -1;
    } else if ((state == SDL2_DISABLE) && !SDL3_HideCursor()) {
        retval = -1;
    }
    return retval;
}

/* SDL3 split this into getter/setter functions. */
SDL_DECLSPEC int SDLCALL
SDL_GameControllerEventState(int state)
{
    int retval = state;
    if (state == SDL2_ENABLE) {
        SDL3_SetGamepadEventsEnabled(true);
    } else if (state == SDL2_DISABLE) {
        SDL3_SetGamepadEventsEnabled(false);
    } else {
        retval = SDL3_GamepadEventsEnabled() ? SDL2_ENABLE : SDL2_DISABLE;
    }
    return retval;
}

/* SDL3 split this into getter/setter functions. */
SDL_DECLSPEC int SDLCALL
SDL_JoystickEventState(int state)
{
    int retval = state;
    if (state == SDL2_ENABLE) {
        SDL3_SetJoystickEventsEnabled(true);
    } else if (state == SDL2_DISABLE) {
        SDL3_SetJoystickEventsEnabled(false);
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

/* !!! FIXME: put a mutex on the joystick and sensor lists. Strictly speaking, this will break if you multithread it, but it doesn't have to crash. */

SDL_DECLSPEC int SDLCALL
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
    if (jid != 0) {
        int i;
        for (i = 0; i < num_joysticks; i++) {
            if (joystick_list[i] == jid) {
                return i;
            }
        }
    }
    return -1;
}


SDL_DECLSPEC SDL_GUID SDLCALL
SDL_JoystickGetDeviceGUID(int idx)
{
    SDL_GUID guid;
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    if (!jid) {
        SDL3_zero(guid);
    } else {
        guid = SDL3_GetJoystickGUIDForID(jid);
    }
    return guid;
}

SDL_DECLSPEC const char* SDLCALL
SDL_JoystickNameForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickNameForID(jid) : NULL;
}

SDL_DECLSPEC SDL_Joystick* SDLCALL
SDL_JoystickOpen(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_OpenJoystick(jid) : NULL;
}

SDL_DECLSPEC Uint16 SDLCALL
SDL_JoystickGetDeviceVendor(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickVendorForID(jid) : 0;
}

SDL_DECLSPEC Uint16 SDLCALL
SDL_JoystickGetDeviceProduct(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickProductForID(jid) : 0;
}

SDL_DECLSPEC Uint16 SDLCALL
SDL_JoystickGetDeviceProductVersion(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickProductVersionForID(jid) : 0;
}

SDL_DECLSPEC SDL_JoystickType SDLCALL
SDL_JoystickGetDeviceType(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickTypeForID(jid) : SDL_JOYSTICK_TYPE_UNKNOWN;
}

SDL_DECLSPEC SDL2_JoystickID SDLCALL
SDL_JoystickGetDeviceInstanceID(int idx)
{
    /* this counts on a Uint32 not overflowing an Sint32. */
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    if (!jid) {
        return -1;
    }
    return (SDL2_JoystickID)jid;
}

SDL_DECLSPEC Uint8 SDLCALL
SDL_JoystickGetButton(SDL_Joystick *joystick, int button)
{
    return SDL3_GetJoystickButton(joystick, button);
}

SDL_DECLSPEC SDL2_JoystickID SDLCALL
SDL_JoystickInstanceID(SDL_Joystick *joystick)
{
    const SDL_JoystickID jid = SDL3_GetJoystickID(joystick);
    if (!jid) {
        return -1;
    }
    return (SDL2_JoystickID)jid;
}

SDL_DECLSPEC SDL_Joystick* SDLCALL
SDL_JoystickFromInstanceID(SDL2_JoystickID jid)
{
    return SDL3_GetJoystickFromID((SDL_JoystickID)jid);
}

SDL_DECLSPEC SDL_GameController* SDLCALL
SDL_GameControllerFromInstanceID(SDL2_JoystickID jid)
{
    return SDL3_GetGamepadFromID((SDL_JoystickID)jid);
}

SDL_DECLSPEC int SDLCALL
SDL_JoystickGetDevicePlayerIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickPlayerIndexForID(jid) : -1;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_JoystickIsVirtual(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return SDL3_IsJoystickVirtual(jid) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC const char* SDLCALL
SDL_JoystickPathForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetJoystickPathForID(jid) : NULL;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_JoystickHasLED(SDL_Joystick *joystick)
{
    return SDL3_GetBooleanProperty(SDL3_GetJoystickProperties(joystick), SDL_PROP_JOYSTICK_CAP_RGB_LED_BOOLEAN, false) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_JoystickHasRumble(SDL_Joystick *joystick)
{
    return SDL3_GetBooleanProperty(SDL3_GetJoystickProperties(joystick), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_JoystickHasRumbleTriggers(SDL_Joystick *joystick)
{
    return SDL3_GetBooleanProperty(SDL3_GetJoystickProperties(joystick), SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN, false) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC SDL_JoystickPowerLevel SDLCALL
SDL_JoystickCurrentPowerLevel(SDL_Joystick *joystick)
{
    SDL_JoystickConnectionState connection_state;

    connection_state = SDL3_GetJoystickConnectionState(joystick);
    if (connection_state == SDL_JOYSTICK_CONNECTION_WIRELESS) {
        int percent = -1;
        SDL_PowerState state = SDL3_GetJoystickPowerInfo(joystick, &percent);
        if (state == SDL_POWERSTATE_ON_BATTERY) {
            if (percent > 70) {
                return SDL_JOYSTICK_POWER_FULL;
            } else if (percent > 20) {
                return SDL_JOYSTICK_POWER_MEDIUM;
            } else if (percent > 5) {
                return SDL_JOYSTICK_POWER_LOW;
            } else {
                return SDL_JOYSTICK_POWER_EMPTY;
            }
        } else {
            return SDL_JOYSTICK_POWER_UNKNOWN;
        }
    } else if (connection_state == SDL_JOYSTICK_CONNECTION_WIRED) {
        return SDL_JOYSTICK_POWER_WIRED;
    } else {
        return SDL_JOYSTICK_POWER_UNKNOWN;
    }
}

SDL_DECLSPEC char* SDLCALL
SDL_GameControllerMappingForDeviceIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadMappingForID(jid) : NULL;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_IsGameController(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return SDL3_IsGamepad(jid) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC const char* SDLCALL
SDL_GameControllerNameForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadNameForID(jid) : NULL;
}

SDL_DECLSPEC int SDLCALL
SDL_GameControllerNumMappings(void)
{
    SDL3_free(GamepadMappings);
    GamepadMappings = SDL3_GetGamepadMappings(&NumGamepadMappings);
    return NumGamepadMappings;
}

SDL_DECLSPEC char* SDLCALL
SDL_GameControllerMappingForIndex(int idx)
{
    char *retval = NULL;
    if ((idx < 0) || (idx >= NumGamepadMappings)) {
        SDL3_SetError("Mapping not available");
    } else {
        retval = SDL3_strdup(GamepadMappings[idx]);
    }
    return retval;
}

SDL_DECLSPEC SDL_GameController* SDLCALL
SDL_GameControllerOpen(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    SDL_GameController *gamepad = jid ? SDL3_OpenGamepad(jid) : NULL;
    if (gamepad) {
        UpdateGamepadButtonSwap(gamepad);
    }
    return gamepad;
}

static SDL_GameControllerType SDLCALL
SDL2COMPAT_GetGamepadTypeForID(const SDL_JoystickID jid)
{
    const Uint16 vid = SDL3_GetJoystickVendorForID(jid);
    const Uint16 pid = SDL3_GetJoystickProductForID(jid);
    if (SDL3_IsJoystickVirtual(jid)) {
        return SDL_CONTROLLER_TYPE_VIRTUAL;
    }
    if ((vid == 0x1949 && pid == 0x0419) ||
        (vid == 0x0171 && pid == 0x0419)) {
        return SDL_CONTROLLER_TYPE_AMAZON_LUNA;
    }
    if (vid == 0x18d1 && pid == 0x9400) {
        return SDL_CONTROLLER_TYPE_GOOGLE_STADIA;
    }
    if (vid == 0x0955 && (pid == 0x7210 || pid == 0x7214)) {
        return SDL_CONTROLLER_TYPE_NVIDIA_SHIELD;
    }
    switch (SDL3_GetGamepadTypeForID(jid)) {
    case SDL_GAMEPAD_TYPE_XBOX360:
        return SDL_CONTROLLER_TYPE_XBOX360;
    case SDL_GAMEPAD_TYPE_XBOXONE:
        return SDL_CONTROLLER_TYPE_XBOXONE;
    case SDL_GAMEPAD_TYPE_PS3:
        return SDL_CONTROLLER_TYPE_PS3;
    case SDL_GAMEPAD_TYPE_PS4:
        return SDL_CONTROLLER_TYPE_PS4;
    case SDL_GAMEPAD_TYPE_PS5:
        return SDL_CONTROLLER_TYPE_PS5;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
        return SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
        return SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
        return SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
        return SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR;
    default:
        return SDL_CONTROLLER_TYPE_UNKNOWN;
    }
}

SDL_DECLSPEC SDL_GameControllerType SDLCALL
SDL_GameControllerGetType(SDL_GameController *controller)
{
    return SDL2COMPAT_GetGamepadTypeForID(SDL3_GetGamepadID(controller));
}

SDL_DECLSPEC SDL_GameControllerType SDLCALL
SDL_GameControllerTypeForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL2COMPAT_GetGamepadTypeForID(jid) : SDL_CONTROLLER_TYPE_UNKNOWN;
}

SDL_DECLSPEC const char* SDLCALL
SDL_GameControllerPathForIndex(int idx)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(idx);
    return jid ? SDL3_GetGamepadPathForID(jid) : NULL;
}

SDL_DECLSPEC Uint8 SDLCALL SDL_GameControllerGetButton(SDL_GameController *controller, SDL_GameControllerButton button)
{
    SDL_JoystickID instance_id = SDL3_GetGamepadID(controller);

    if (ShouldSwapGamepadButtons(instance_id)) {
        button = (SDL_GameControllerButton)SwapGamepadButton(button);
    }
    return SDL3_GetGamepadButton(controller, button);
}

SDL_DECLSPEC int SDLCALL
SDL_GameControllerGetTouchpadFinger(SDL_GameController *gamecontroller, int touchpad, int finger, Uint8 *state, float *x, float *y, float *pressure)
{
    return SDL3_GetGamepadTouchpadFinger(gamecontroller, touchpad, finger, (bool *)state, x, y, pressure) ? 0 : -1;
}

SDL_DECLSPEC SDL_GameControllerButtonBind SDLCALL
SDL_GameControllerGetBindForAxis(SDL_GameController *controller,
                                 SDL_GameControllerAxis axis)
{
    SDL_GameControllerButtonBind bind;

    SDL3_zero(bind);

    if (axis != SDL_GAMEPAD_AXIS_INVALID) {
        SDL_GamepadBinding **bindings = SDL3_GetGamepadBindings(controller, NULL);
        if (bindings) {
            int i;
            for (i = 0; bindings[i]; ++i) {
                const SDL_GamepadBinding *binding = bindings[i];
                if (binding->output_type == SDL_GAMEPAD_BINDTYPE_AXIS && binding->output.axis.axis == axis) {
                    bind.bindType = binding->input_type;
                    if (binding->input_type == SDL_GAMEPAD_BINDTYPE_AXIS) {
                        /* FIXME: There might be multiple axes bound now that we have axis ranges... */
                        bind.value.axis = binding->input.axis.axis;
                    } else if (binding->input_type == SDL_GAMEPAD_BINDTYPE_BUTTON) {
                        bind.value.button = binding->input.button;
                    } else if (binding->input_type == SDL_GAMEPAD_BINDTYPE_HAT) {
                        bind.value.hat.hat = binding->input.hat.hat;
                        bind.value.hat.hat_mask = binding->input.hat.hat_mask;
                    }
                    break;
                }
            }
            SDL_free(bindings);
        }
    }

    return bind;
}

SDL_DECLSPEC SDL_GameControllerButtonBind SDLCALL
SDL_GameControllerGetBindForButton(SDL_GameController *controller,
                                   SDL_GameControllerButton button)
{
    SDL_GameControllerButtonBind bind;

    SDL3_zero(bind);

    if (button != SDL_GAMEPAD_BUTTON_INVALID) {
        SDL_GamepadBinding **bindings = SDL3_GetGamepadBindings(controller, NULL);
        if (bindings) {
            int i;
            for (i = 0; bindings[i]; ++i) {
                const SDL_GamepadBinding *binding = bindings[i];
                if (binding->output_type == SDL_GAMEPAD_BINDTYPE_BUTTON && binding->output.button == button) {
                    bind.bindType = binding->input_type;
                    if (binding->input_type == SDL_GAMEPAD_BINDTYPE_AXIS) {
                        bind.value.axis = binding->input.axis.axis;
                    } else if (binding->input_type == SDL_GAMEPAD_BINDTYPE_BUTTON) {
                        bind.value.button = binding->input.button;
                    } else if (binding->input_type == SDL_GAMEPAD_BINDTYPE_HAT) {
                        bind.value.hat.hat = binding->input.hat.hat;
                        bind.value.hat.hat_mask = binding->input.hat.hat_mask;
                    }
                    break;
                }
            }
            SDL_free(bindings);
        }
    }

    return bind;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GameControllerHasLED(SDL_Gamepad *gamepad)
{
    return SDL3_GetBooleanProperty(SDL3_GetGamepadProperties(gamepad), SDL_PROP_GAMEPAD_CAP_RGB_LED_BOOLEAN, false) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GameControllerHasRumble(SDL_Gamepad *gamepad)
{
    return SDL3_GetBooleanProperty(SDL3_GetGamepadProperties(gamepad), SDL_PROP_GAMEPAD_CAP_RUMBLE_BOOLEAN, false) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_GameControllerHasRumbleTriggers(SDL_Gamepad *gamepad)
{
    return SDL3_GetBooleanProperty(SDL3_GetGamepadProperties(gamepad), SDL_PROP_GAMEPAD_CAP_TRIGGER_RUMBLE_BOOLEAN, false) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC int SDLCALL
SDL_JoystickAttachVirtual(SDL_JoystickType type, int naxes, int nbuttons, int nhats)
{
    SDL_JoystickID jid;
    SDL_VirtualJoystickDesc desc3;

    SDL_INIT_INTERFACE(&desc3);
    desc3.type = (Uint16)type;
    desc3.naxes = (Uint16)naxes;
    desc3.nbuttons = (Uint16)nbuttons;
    desc3.nhats = (Uint16)nhats;

    jid = SDL3_AttachVirtualJoystick(&desc3);

    SDL_NumJoysticks(); /* Refresh */
    return GetIndexFromJoystickInstance(jid);
}

static void SDLCALL VirtualJoystick_Update(void *userdata)
{
    SDL2_VirtualJoystickDesc *desc = (SDL2_VirtualJoystickDesc *)userdata;
    desc->Update(desc->userdata);
}

static void SDLCALL VirtualJoystick_SetPlayerIndex(void *userdata, int player_index)
{
    SDL2_VirtualJoystickDesc *desc = (SDL2_VirtualJoystickDesc *)userdata;
    desc->SetPlayerIndex(desc->userdata, player_index);
}

static bool SDLCALL VirtualJoystick_Rumble(void *userdata, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble)
{
    SDL2_VirtualJoystickDesc *desc = (SDL2_VirtualJoystickDesc *)userdata;
    return desc->Rumble(desc->userdata, low_frequency_rumble, high_frequency_rumble);
}

static bool SDLCALL VirtualJoystick_RumbleTriggers(void *userdata, Uint16 left_rumble, Uint16 right_rumble)
{
    SDL2_VirtualJoystickDesc *desc = (SDL2_VirtualJoystickDesc *)userdata;
    return desc->RumbleTriggers(desc->userdata, left_rumble, right_rumble);
}

static bool SDLCALL VirtualJoystick_SetLED(void *userdata, Uint8 red, Uint8 green, Uint8 blue)
{
    SDL2_VirtualJoystickDesc *desc = (SDL2_VirtualJoystickDesc *)userdata;
    return desc->SetLED(desc->userdata, red, green, blue);
}

static bool SDLCALL VirtualJoystick_SendEffect(void *userdata, const void *data, int size)
{
    SDL2_VirtualJoystickDesc *desc = (SDL2_VirtualJoystickDesc *)userdata;
    return desc->SendEffect(desc->userdata, data, size);
}

static void SDLCALL VirtualJoystick_Cleanup(void *userdata)
{
    SDL_free(userdata);
}

SDL_DECLSPEC int SDLCALL
SDL_JoystickAttachVirtualEx(const SDL2_VirtualJoystickDesc *desc2)
{
    SDL_JoystickID jid;
    SDL_VirtualJoystickDesc desc3;

    if (!desc2) {
        SDL3_InvalidParamError("desc");
        return -1;
    } else if (desc2->version != SDL_VIRTUAL_JOYSTICK_DESC_VERSION) { /* SDL2 only had one version. */
        SDL3_SetError("Unsupported virtual joystick description version %u", desc2->version);
        return -1;
    }

    SDL_INIT_INTERFACE(&desc3);
    #define SETDESCFIELD(x) desc3.x = desc2->x
    SETDESCFIELD(type);
    SETDESCFIELD(naxes);
    SETDESCFIELD(nbuttons);
    SETDESCFIELD(nhats);
    SETDESCFIELD(vendor_id);
    SETDESCFIELD(product_id);
    SETDESCFIELD(button_mask);
    SETDESCFIELD(axis_mask);
    SETDESCFIELD(name);
    #undef SETDESCFIELD

    desc3.userdata = SDL_malloc(sizeof(*desc2));
    if (!desc3.userdata) {
        SDL3_OutOfMemory();
        return -1;
    }
    SDL_memcpy(desc3.userdata, desc2, sizeof(*desc2));

    if (desc2->Update) {
        desc3.Update = VirtualJoystick_Update;
    }
    if (desc2->SetPlayerIndex) {
        desc3.SetPlayerIndex = VirtualJoystick_SetPlayerIndex;
    }
    if (desc2->Rumble) {
        desc3.Rumble = VirtualJoystick_Rumble;
    }
    if (desc2->RumbleTriggers) {
        desc3.RumbleTriggers = VirtualJoystick_RumbleTriggers;
    }
    if (desc2->SetLED) {
        desc3.SetLED = VirtualJoystick_SetLED;
    }
    if (desc2->SendEffect) {
        desc3.SendEffect = VirtualJoystick_SendEffect;
    }
    desc3.Cleanup = VirtualJoystick_Cleanup;

    jid = SDL3_AttachVirtualJoystick(&desc3);
    SDL_NumJoysticks(); /* Refresh */
    return GetIndexFromJoystickInstance(jid);
}

SDL_DECLSPEC int SDLCALL
SDL_JoystickSetVirtualButton(SDL_Joystick *joystick, int button, Uint8 value)
{
    return SDL3_SetJoystickVirtualButton(joystick, button, (value != 0)) ? 0 : -1;
}

SDL_DECLSPEC int SDLCALL
SDL_JoystickDetachVirtual(int device_index)
{
    const SDL_JoystickID jid = GetJoystickInstanceFromIndex(device_index);
    if (jid && SDL3_DetachVirtualJoystick(jid)) {
        return 0;
    } else {
        return -1;
    }
}


static SDL_SensorID
GetSensorInstanceFromIndex(int idx)
{
    if ((idx < 0) || (idx >= num_sensors)) {
        SDL3_SetError("There are %d sensors available", num_sensors);
        return 0;
    }
    return sensor_list[idx];
}

SDL_DECLSPEC int SDLCALL
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

SDL_DECLSPEC const char* SDLCALL
SDL_SensorGetDeviceName(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_GetSensorNameForID(sid) : NULL;
}

SDL_DECLSPEC SDL_SensorType SDLCALL
SDL_SensorGetDeviceType(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_GetSensorTypeForID(sid) : SDL_SENSOR_INVALID;
}

SDL_DECLSPEC int SDLCALL
SDL_SensorGetDeviceNonPortableType(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_GetSensorNonPortableTypeForID(sid) : -1;
}

SDL_DECLSPEC SDL2_SensorID SDLCALL
SDL_SensorGetDeviceInstanceID(int idx)
{
    /* this counts on a Uint32 not overflowing an Sint32. */
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    if (!sid) {
        return -1;
    }
    return (SDL2_SensorID)sid;
}

SDL_DECLSPEC SDL2_SensorID SDLCALL
SDL_SensorGetInstanceID(SDL_Sensor *sensor)
{
    const SDL_SensorID sid = SDL3_GetSensorID(sensor);
    if (!sid) {
        return -1;
    }
    return (SDL2_SensorID)sid;
}

SDL_DECLSPEC SDL_Sensor* SDLCALL
SDL_SensorFromInstanceID(SDL2_SensorID sid)
{
    return SDL3_GetSensorFromID((SDL_SensorID)sid);
}

SDL_DECLSPEC SDL_Sensor* SDLCALL
SDL_SensorOpen(int idx)
{
    const SDL_SensorID sid = GetSensorInstanceFromIndex(idx);
    return sid ? SDL3_OpenSensor(sid) : NULL;
}


static SDL_HapticID
GetHapticInstanceFromIndex(int idx)
{
    if ((idx < 0) || (idx >= num_haptics)) {
        SDL3_SetError("There are %d haptics available", num_haptics);
        return 0;
    }
    return haptic_list[idx];
}

SDL_DECLSPEC int SDLCALL
SDL_NumHaptics(void)
{
    SDL3_free(haptic_list);
    haptic_list = SDL3_GetHaptics(&num_haptics);
    if (haptic_list == NULL) {
        num_haptics = 0;
        return -1;
    }
    return num_haptics;
}

SDL_DECLSPEC const char * SDLCALL
SDL_HapticName(int device_index)
{
    const SDL_HapticID instance_id = GetHapticInstanceFromIndex(device_index);
    return instance_id ? SDL3_GetHapticNameForID(instance_id) : NULL;
}

SDL_DECLSPEC
SDL_Haptic * SDLCALL
SDL_HapticOpen(int device_index)
{
    const SDL_HapticID instance_id = GetHapticInstanceFromIndex(device_index);
    return SDL3_OpenHaptic(instance_id);
}

SDL_DECLSPEC int SDLCALL
SDL_HapticOpened(int device_index)
{
    const SDL_HapticID instance_id = GetHapticInstanceFromIndex(device_index);
    if (SDL3_GetHapticFromID(instance_id) != NULL) {
        return 1;
    }
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_HapticIndex(SDL_Haptic *haptic)
{
    const SDL_HapticID instance_id = SDL3_GetHapticID(haptic);
    int i;

    for (i = 0; i < num_haptics; ++i) {
        if (instance_id == haptic_list[i]) {
            return i;
        }
    }
    SDL3_SetError("Haptic: Invalid haptic device identifier");
    return -1;
}

SDL_DECLSPEC int SDLCALL
SDL_HapticRumbleSupported(SDL_Haptic *haptic)
{
    if (SDL3_GetHapticID(haptic) == 0) {
        return -1;
    }
    return SDL3_HapticRumbleSupported(haptic) ? SDL2_TRUE : SDL2_FALSE;
}

static Uint16 HapticFeatures3to2(Uint32 features)
{
    Uint16 features2 = 0;

    /* We could be clever and shift bits around, but let's be clear instead */
    if (features & SDL_HAPTIC_CONSTANT) {
        features2 |= SDL2_HAPTIC_CONSTANT;
    }
    if (features & SDL_HAPTIC_SINE) {
        features2 |= SDL2_HAPTIC_SINE;
    }
    if (features & SDL_HAPTIC_LEFTRIGHT) {
        features2 |= SDL2_HAPTIC_LEFTRIGHT;
    }
    if (features & SDL_HAPTIC_TRIANGLE) {
        features2 |= SDL2_HAPTIC_TRIANGLE;
    }
    if (features & SDL_HAPTIC_SAWTOOTHUP) {
        features2 |= SDL2_HAPTIC_SAWTOOTHUP;
    }
    if (features & SDL_HAPTIC_SAWTOOTHDOWN) {
        features2 |= SDL2_HAPTIC_SAWTOOTHDOWN;
    }
    if (features & SDL_HAPTIC_RAMP) {
        features2 |= SDL2_HAPTIC_RAMP;
    }
    if (features & SDL_HAPTIC_SPRING) {
        features2 |= SDL2_HAPTIC_SPRING;
    }
    if (features & SDL_HAPTIC_DAMPER) {
        features2 |= SDL2_HAPTIC_DAMPER;
    }
    if (features & SDL_HAPTIC_INERTIA) {
        features2 |= SDL2_HAPTIC_INERTIA;
    }
    if (features & SDL_HAPTIC_FRICTION) {
        features2 |= SDL2_HAPTIC_FRICTION;
    }
    if (features & SDL_HAPTIC_CUSTOM) {
        features2 |= SDL2_HAPTIC_CUSTOM;
    }
    if (features & SDL_HAPTIC_GAIN) {
        features2 |= SDL2_HAPTIC_GAIN;
    }
    if (features & SDL_HAPTIC_AUTOCENTER) {
        features2 |= SDL2_HAPTIC_AUTOCENTER;
    }
    if (features & SDL_HAPTIC_STATUS) {
        features2 |= SDL2_HAPTIC_STATUS;
    }
    if (features & SDL_HAPTIC_PAUSE) {
        features2 |= SDL2_HAPTIC_PAUSE;
    }
    return features2;
}

static Uint32 HapticFeatures2to3(Uint16 features)
{
    Uint32 features3 = 0;

    /* We could be clever and shift bits around, but let's be clear instead */
    if (features & SDL2_HAPTIC_CONSTANT) {
        features3 |= SDL_HAPTIC_CONSTANT;
    }
    if (features & SDL2_HAPTIC_SINE) {
        features3 |= SDL_HAPTIC_SINE;
    }
    if (features & SDL2_HAPTIC_LEFTRIGHT) {
        features3 |= SDL_HAPTIC_LEFTRIGHT;
    }
    if (features & SDL2_HAPTIC_TRIANGLE) {
        features3 |= SDL_HAPTIC_TRIANGLE;
    }
    if (features & SDL2_HAPTIC_SAWTOOTHUP) {
        features3 |= SDL_HAPTIC_SAWTOOTHUP;
    }
    if (features & SDL2_HAPTIC_SAWTOOTHDOWN) {
        features3 |= SDL_HAPTIC_SAWTOOTHDOWN;
    }
    if (features & SDL2_HAPTIC_RAMP) {
        features3 |= SDL_HAPTIC_RAMP;
    }
    if (features & SDL2_HAPTIC_SPRING) {
        features3 |= SDL_HAPTIC_SPRING;
    }
    if (features & SDL2_HAPTIC_DAMPER) {
        features3 |= SDL_HAPTIC_DAMPER;
    }
    if (features & SDL2_HAPTIC_INERTIA) {
        features3 |= SDL_HAPTIC_INERTIA;
    }
    if (features & SDL2_HAPTIC_FRICTION) {
        features3 |= SDL_HAPTIC_FRICTION;
    }
    if (features & SDL2_HAPTIC_CUSTOM) {
        features3 |= SDL_HAPTIC_CUSTOM;
    }
    if (features & SDL2_HAPTIC_GAIN) {
        features3 |= SDL_HAPTIC_GAIN;
    }
    if (features & SDL2_HAPTIC_AUTOCENTER) {
        features3 |= SDL_HAPTIC_AUTOCENTER;
    }
    if (features & SDL2_HAPTIC_STATUS) {
        features3 |= SDL_HAPTIC_STATUS;
    }
    if (features & SDL2_HAPTIC_PAUSE) {
        features3 |= SDL_HAPTIC_PAUSE;
    }
    return features3;
}

static void HapticEffect2to3(const SDL_HapticEffect *effect2, SDL_HapticEffect *effect3)
{
    SDL3_copyp(effect3, effect2);
    effect3->type = (Uint16)HapticFeatures2to3(effect2->type);
}

SDL_DECLSPEC unsigned int SDLCALL
SDL_HapticQuery(SDL_Haptic *haptic)
{
    return HapticFeatures3to2(SDL3_GetHapticFeatures(haptic));
}

SDL_DECLSPEC int SDLCALL
SDL_HapticEffectSupported(SDL_Haptic *haptic, SDL_HapticEffect *effect)
{
    SDL_HapticEffect effect3;

    if (!effect) {
        return SDL2_FALSE;
    }
    HapticEffect2to3(effect, &effect3);
    return SDL3_HapticEffectSupported(haptic, &effect3);
}

SDL_DECLSPEC int SDLCALL
SDL_HapticNewEffect(SDL_Haptic *haptic, SDL_HapticEffect *effect)
{
    SDL_HapticEffect effect3;

    if (!effect) {
        SDL3_InvalidParamError("effect");
        return -1;
    }
    HapticEffect2to3(effect, &effect3);
    return SDL3_CreateHapticEffect(haptic, &effect3);
}

SDL_DECLSPEC int SDLCALL
SDL_HapticUpdateEffect(SDL_Haptic *haptic, int effect, SDL_HapticEffect *data)
{
    SDL_HapticEffect effect3;

    if (!data) {
        SDL3_InvalidParamError("data");
        return -1;
    }
    HapticEffect2to3(data, &effect3);
    if (!SDL3_UpdateHapticEffect(haptic, effect, &effect3)) {
        return -1;
    }
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_HapticGetEffectStatus(SDL_Haptic * haptic, int effect)
{
    SDL3_ClearError();

    if (SDL3_GetHapticEffectStatus(haptic, effect)) {
        return 1;
    } else if (*SDL3_GetError() == '\0') {
        return 0;
    } else {
        return -1;
    }
}

SDL_DECLSPEC int SDLCALL
SDL_SemWait(SDL_sem *sem)
{
    SDL3_WaitSemaphore(sem);
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_SemTryWait(SDL_sem *sem)
{
    if (SDL3_TryWaitSemaphore(sem)) {
        return 0;
    } else {
        return SDL2_MUTEX_TIMEDOUT;
    }
}

SDL_DECLSPEC int SDLCALL
SDL_SemWaitTimeout(SDL_sem *sem, Uint32 ms)
{
    if (SDL3_WaitSemaphoreTimeout(sem, (Sint32)ms)) {
        return 0;
    } else {
        return SDL2_MUTEX_TIMEDOUT;
    }
}

SDL_DECLSPEC int SDLCALL SDL_CondSignal(SDL_cond *cond)
{
    SDL3_SignalCondition(cond);
    return 0;
}

SDL_DECLSPEC int SDLCALL SDL_CondBroadcast(SDL_cond *cond)
{
    SDL3_BroadcastCondition(cond);
    return 0;
}

SDL_DECLSPEC int SDLCALL SDL_CondWait(SDL_cond *cond, SDL_mutex *mutex)
{
    SDL3_WaitCondition(cond, mutex);
    return 0;
}

SDL_DECLSPEC int SDLCALL
SDL_CondWaitTimeout(SDL_cond *cond, SDL_mutex *mutex, Uint32 ms)
{
    if (SDL3_WaitConditionTimeout(cond, mutex, (Sint32)ms)) {
        return 0;
    } else {
        return SDL2_MUTEX_TIMEDOUT;
    }
}

SDL_DECLSPEC int SDLCALL
SDL_SemPost(SDL_sem *sem)
{
    SDL3_SignalSemaphore(sem);
    return 0;
}

SDL_DECLSPEC void * SDLCALL
SDL_SIMDAlloc(const size_t len)
{
    return SDL3_aligned_alloc(SDL3_GetSIMDAlignment(), len);
}

SDL_DECLSPEC void * SDLCALL
SDL_SIMDRealloc(void *mem, const size_t len)
{
    const size_t alignment = SDL3_GetSIMDAlignment();
    const size_t padding = (alignment - (len % alignment)) % alignment;
    Uint8 *retval = (Uint8 *)mem;
    void *oldmem = mem;
    size_t memdiff = 0, ptrdiff;
    Uint8 *ptr;
    size_t to_allocate;

    /* alignment + padding + sizeof (void *) is bounded (a few hundred
     * bytes max), so no need to check for overflow within that argument */
    if (!SDL_size_add_check_overflow(len, alignment + padding + sizeof(void *), &to_allocate)) {
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

SDL_DECLSPEC void SDLCALL
SDL_SIMDFree(void *ptr)
{
    SDL3_aligned_free(ptr);
}


/* !!! FIXME: move this all up with the other audio functions */
static SDL2_bool
SDL_IsSupportedAudioFormat(const SDL2_AudioFormat fmt)
{
    switch (fmt) {
    case SDL_AUDIO_U8:
    case SDL_AUDIO_S8:
    case SDL2_AUDIO_U16LSB:
    case SDL_AUDIO_S16LE:
    case SDL2_AUDIO_U16MSB:
    case SDL_AUDIO_S16BE:
    case SDL_AUDIO_S32LE:
    case SDL_AUDIO_S32BE:
    case SDL_AUDIO_F32LE:
    case SDL_AUDIO_F32BE:
        return SDL2_TRUE; /* supported. */

    default:
        break;
    }

    return SDL2_FALSE; /* unsupported. */
}

static bool SDL_IsSupportedChannelCount(const int channels)
{
    return ((channels >= 1) && (channels <= 8));
}


typedef struct {
    SDL2_AudioFormat src_format;
    Uint8 src_channels;
    int src_rate;
    SDL2_AudioFormat dst_format;
    Uint8 dst_channels;
    int dst_rate;
} AudioParam;

#define RESAMPLER_BITS_PER_SAMPLE           16
#define RESAMPLER_SAMPLES_PER_ZERO_CROSSING (1 << ((RESAMPLER_BITS_PER_SAMPLE / 2) + 1))


SDL_DECLSPEC int SDLCALL
SDL_BuildAudioCVT(SDL_AudioCVT *cvt,
                  SDL2_AudioFormat src_format,
                  Uint8 src_channels,
                  int src_rate,
                  SDL2_AudioFormat dst_format,
                  Uint8 dst_channels,
                  int dst_rate)
{
    /* Sanity check target pointer */
    if (cvt == NULL) {
        SDL3_InvalidParamError("cvt");
        return -1;
    }

    /* Make sure we zero out the audio conversion before error checking */
    SDL3_zerop(cvt);

    if (!SDL_IsSupportedAudioFormat(src_format)) {
        SDL3_SetError("Invalid source format");
        return -1;
    }
    if (!SDL_IsSupportedAudioFormat(dst_format)) {
        SDL3_SetError("Invalid destination format");
        return -1;
    }
    if (!SDL_IsSupportedChannelCount(src_channels)) {
        SDL3_SetError("Invalid source channels");
        return -1;
    }
    if (!SDL_IsSupportedChannelCount(dst_channels)) {
        SDL3_SetError("Invalid destination channels");
        return -1;
    }
    if (src_rate <= 0) {
        SDL3_SetError("Source rate is equal to or less than zero");
        return -1;
    }
    if (dst_rate <= 0) {
        SDL3_SetError("Destination rate is equal to or less than zero");
        return -1;
    }
    if (src_rate >= SDL_MAX_SINT32 / RESAMPLER_SAMPLES_PER_ZERO_CROSSING) {
        SDL3_SetError("Source rate is too high");
        return -1;
    }
    if (dst_rate >= SDL_MAX_SINT32 / RESAMPLER_SAMPLES_PER_ZERO_CROSSING) {
        SDL3_SetError("Destination rate is too high");
        return -1;
    }

#ifdef DEBUG_CONVERT
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

        if (src_channels < dst_channels) {
            cvt->len_mult = ((cvt->len_mult * dst_channels) + (src_channels - 1)) / src_channels;
        }

        if (src_rate < dst_rate) {
            const double mult = ((double)dst_rate / (double)src_rate);
            cvt->len_mult *= (int)SDL_ceil(mult);
            cvt->len_ratio *= mult;
        } else {
            const double divisor = ((double)src_rate / (double)dst_rate);
            cvt->len_ratio /= divisor;
        }
    }

    return cvt->needed;
}

SDL_DECLSPEC int SDLCALL
SDL_ConvertAudio(SDL_AudioCVT *cvt)
{
    SDL2_AudioStream *stream2;
    SDL2_AudioFormat src_format, dst_format;
    int src_channels, src_rate;
    int dst_channels, dst_rate;

    int src_len, dst_len, real_dst_len;
    int src_samplesize;

    /* Sanity check target pointer */
    if (cvt == NULL) {
        SDL3_InvalidParamError("cvt");
        return -1;
    }

    { /* Fetch from the end of filters[], aligned */
        AudioParam ap;

        SDL3_memcpy(
            &ap,
            (Uint8 *)&cvt->filters[SDL_AUDIOCVT_MAX_FILTERS + 1] - (sizeof(AudioParam) & ~3),
            sizeof(ap));

        src_format = ap.src_format;
        src_channels = ap.src_channels;
        src_rate = ap.src_rate;
        dst_format = ap.dst_format;
        dst_channels = ap.dst_channels;
        dst_rate = ap.dst_rate;
    }

    /* don't use the SDL3 stream directly or even SDL_ConvertAudioSamples; we want the U16 support in the sdl2-compat layer */
    stream2 = SDL_NewAudioStream(src_format, src_channels, src_rate,
                                 dst_format, dst_channels, dst_rate);
    if (stream2 == NULL) {
        goto failure;
    }

    src_samplesize = (SDL_AUDIO_BITSIZE(src_format) / 8) * src_channels;

    src_len = cvt->len & ~(src_samplesize - 1);
    dst_len = cvt->len * cvt->len_mult;

    /* Run the audio converter */
    if (SDL_AudioStreamPut(stream2, cvt->buf, src_len) < 0 ||
        SDL_AudioStreamFlush(stream2) < 0) {
        goto failure;
    }

    /* Get back in the same buffer */
    real_dst_len = SDL_AudioStreamGet(stream2, cvt->buf, dst_len);
    if (real_dst_len < 0) {
        goto failure;
    }

    cvt->len_cvt = real_dst_len;

    SDL_FreeAudioStream(stream2);

    return 0;

failure:
    SDL_FreeAudioStream(stream2);
    return -1;
}

SDL_DECLSPEC void SDLCALL
SDL_GL_GetDrawableSize(SDL_Window *window, int *w, int *h)
{
    SDL_GetWindowSizeInPixels(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_Vulkan_GetDrawableSize(SDL_Window *window, int *w, int *h)
{
    SDL_GetWindowSizeInPixels(window, w, h);
}

SDL_DECLSPEC void SDLCALL
SDL_Metal_GetDrawableSize(SDL_Window *window, int *w, int *h)
{
    SDL_GetWindowSizeInPixels(window, w, h);
}

SDL_DECLSPEC char * SDLCALL
SDL_GetBasePath(void)
{
    const char *retval = SDL3_GetBasePath();
    return retval ? SDL3_strdup(retval) : NULL;
}

SDL_DECLSPEC SDL_Locale *SDL_GetPreferredLocales(void)
{
    SDL_Locale *retval = NULL;
    int i, count;
    SDL_Locale **locales = SDL3_GetPreferredLocales(&count);
    if (locales) {
        /* the string pointers in these structs are allocated as part of
           `locales`, so count out the strings, allocate space for them,
            and copy them to the new return value so they don't go away
            with the temporary memory */
        size_t allocation = (count + 1) * sizeof (*retval);
        size_t strallocation = 0;
        for (i = 0; i < count; ++i) {
            if (locales[i]->language) {
                strallocation += SDL3_strlen(locales[i]->language) + 1;
            }
            if (locales[i]->country) {
                strallocation += SDL3_strlen(locales[i]->country) + 1;
            }
        }
        allocation += strallocation;
        retval = (SDL_Locale *)SDL3_calloc(1, allocation);
        if (retval) {
            char *strs = (char *) (retval + (count + 1));
            for (i = 0; i < count; ++i) {
                if (locales[i]->language) {
                    const size_t len = SDL3_strlcpy(strs, locales[i]->language, strallocation);
                    retval[i].language = strs;
                    strs += len + 1;
                    strallocation -= len + 1;
                }
                if (locales[i]->country) {
                    const size_t len = SDL3_strlcpy(strs, locales[i]->country, strallocation);
                    retval[i].country = strs;
                    strs += len + 1;
                    strallocation -= len + 1;
                }
            }
        }
        SDL_free(locales);
    }
    return retval;
}

SDL_DECLSPEC SDL_hid_device * SDLCALL
SDL_hid_open_path(const char *path, int bExclusive)
{
    (void) bExclusive;
    return SDL3_hid_open_path(path);
}

SDL_DECLSPEC void SDLCALL
SDL_hid_close(SDL_hid_device * dev)
{
    SDL3_hid_close(dev);
}

SDL_DECLSPEC void SDLCALL
SDL_hid_free_enumeration(SDL2_hid_device_info *devs)
{
    while (devs) {
        struct SDL2_hid_device_info *next = devs->next;
        SDL3_free(devs->path);
        SDL3_free(devs->serial_number);
        SDL3_free(devs->manufacturer_string);
        SDL3_free(devs->product_string);
        SDL3_free(devs);
        devs = next;
    }
}

SDL_DECLSPEC SDL2_hid_device_info * SDLCALL
SDL_hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
    /* the struct is slightly different in SDL3, convert it. */
    SDL2_hid_device_info *retval = NULL;
    SDL_hid_device_info *list3 = SDL3_hid_enumerate(vendor_id, product_id);

    if (list3 != NULL) {
        SDL2_hid_device_info *tail = NULL;
        SDL_hid_device_info *i;
        for (i = list3; i != NULL; i = i->next) {
            SDL2_hid_device_info *info = (SDL2_hid_device_info *) SDL3_calloc(1, sizeof (SDL2_hid_device_info));
            char *path = SDL3_strdup(i->path);
            wchar_t *serial_number = SDL3_wcsdup(i->serial_number);
            wchar_t *manufacturer_string = SDL3_wcsdup(i->manufacturer_string);
            wchar_t *product_string = SDL3_wcsdup(i->product_string);
            if (!info || !path || !serial_number || !manufacturer_string || !product_string) {
                SDL_hid_free_enumeration(retval);
                SDL3_free(info);
                SDL3_free(path);
                SDL3_free(serial_number);
                SDL3_free(manufacturer_string);
                SDL3_free(product_string);
                return NULL;
            }
            if (tail) {
                tail->next = info;
            } else {
                retval = info;
            }
            info->path = path;
            info->vendor_id = i->vendor_id;
            info->product_id = i->product_id;
            info->serial_number = serial_number;
            info->release_number = i->release_number;
            info->manufacturer_string = manufacturer_string;
            info->product_string = product_string;
            info->usage_page = i->usage_page;
            info->usage = i->usage;
            info->interface_number = i->interface_number;
            info->interface_class = i->interface_class;
            info->interface_subclass = i->interface_subclass;
            info->interface_protocol = i->interface_protocol;
            info->next = NULL;
            tail = info;
        }
        SDL3_hid_free_enumeration(list3);
    }

    return retval;
}


/* this is gone in SDL3; the TLS get/set functions will create a new TLSID if necessary. */
SDL_DECLSPEC SDL2_TLSID SDLCALL
SDL_TLSCreate(void)
{
    SDL_TLSID tlsid;
    tlsid.value = 0;  /* zero so SDL3 creates a new TLSID. */

    /* this will create the new ID when it sees a zero, and just set a
       harmless NULL in the slot for this thread, which is what SDL_TLSGet
       will return if nothing was ever set. */
    if (!SDL3_SetTLS(&tlsid, NULL, NULL)) {
        return 0; /* uhoh, ran out of memory, probably. */
    }

    return (SDL2_TLSID) tlsid.value;  /* no need to atomic-get this, it's a stack local, so nothing competing for this. :) */
}

SDL_DECLSPEC void* SDLCALL
SDL_TLSGet(SDL2_TLSID tlsid2)
{
    void *retval = NULL;
    if (tlsid2 != 0) {  /* tlsid==0 doesn't set an error here in SDL2, it just returns NULL. */
        SDL_TLSID tlsid3;
        tlsid3.value = (int) tlsid2;  /* no need to atomic-set; we don't need to compete for a stack-local variable. */
        retval = SDL3_GetTLS(&tlsid3);
    }
    return retval;
}

SDL_DECLSPEC int SDLCALL
SDL_TLSSet(SDL2_TLSID tlsid2, const void *value, SDL_TLSDestructorCallback destructor)
{
    SDL_TLSID tlsid3;
    if (tlsid2 == 0) {
        SDL3_InvalidParamError("id");
        return -1;
    }
    tlsid3.value = (int) tlsid2;  /* no need to atomic-set; we don't need to compete for a stack-local variable. */
    return SDL3_SetTLS(&tlsid3, value, destructor);
}

static SDL_Thread *SDL2_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char *name, const size_t stacksize, void *userdata, SDL_FunctionPointer pfnBeginThread, SDL_FunctionPointer pfnEndThread)
{
    const SDL_PropertiesID props = SDL3_CreateProperties();
    SDL_Thread *thread;
    SDL3_SetPointerProperty(props, SDL_PROP_THREAD_CREATE_ENTRY_FUNCTION_POINTER, (void *) fn);
    SDL3_SetStringProperty(props, SDL_PROP_THREAD_CREATE_NAME_STRING, name);
    SDL3_SetPointerProperty(props, SDL_PROP_THREAD_CREATE_USERDATA_POINTER, userdata);
    SDL3_SetNumberProperty(props, SDL_PROP_THREAD_CREATE_STACKSIZE_NUMBER, (Sint64) stacksize);
    thread = SDL3_CreateThreadWithPropertiesRuntime(props, pfnBeginThread, pfnEndThread);
    SDL3_DestroyProperties(props);
    return thread;
}

static SDL_Thread *SDL2_CreateThread(SDL_ThreadFunction fn, const char *name, void *userdata, SDL_FunctionPointer pfnBeginThread, SDL_FunctionPointer pfnEndThread)
{
    size_t stacksize = 0;
    const char *hint = SDL3_GetHint("SDL_THREAD_STACK_SIZE");
    if (hint) {
        stacksize = (size_t)SDL3_strtoul(hint, NULL, 0);
    }
    return SDL2_CreateThreadWithStackSize(fn, name, stacksize, userdata, pfnBeginThread, pfnEndThread);
}

#if defined(SDL_PLATFORM_WINDOWS)

SDL_DECLSPEC SDL_Thread *SDLCALL
SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data,
                 pfnSDL_CurrentBeginThread pfnBeginThread, pfnSDL_CurrentEndThread pfnEndThread)
{
    return SDL2_CreateThread(fn, name, data, (SDL_FunctionPointer) pfnBeginThread, (SDL_FunctionPointer) pfnEndThread);
}

SDL_DECLSPEC SDL_Thread *SDLCALL
SDL_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char *name, const size_t stacksize, void *data,
                              pfnSDL_CurrentBeginThread pfnBeginThread, pfnSDL_CurrentEndThread pfnEndThread)
{
    return SDL2_CreateThreadWithStackSize(fn, name, stacksize, data, (SDL_FunctionPointer) pfnBeginThread, (SDL_FunctionPointer) pfnEndThread);
}

#else

SDL_DECLSPEC SDL_Thread *SDLCALL
SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    return SDL2_CreateThread(fn, name, data, NULL, NULL);
}

SDL_DECLSPEC SDL_Thread *SDLCALL
SDL_CreateThreadWithStackSize(SDL_ThreadFunction fn, const char *name, const size_t stacksize, void *data)
{
    return SDL2_CreateThreadWithStackSize(fn, name, stacksize, data, NULL, NULL);
}

#endif /* SDL_PLATFORM_WINDOWS */

SDL_DECLSPEC unsigned long SDLCALL
SDL_ThreadID(void)
{
    return (unsigned long)SDL3_GetCurrentThreadID();
}

SDL_DECLSPEC unsigned long SDLCALL
SDL_GetThreadID(SDL_Thread *thread)
{
    return (unsigned long)SDL3_GetThreadID(thread);
}

SDL_DECLSPEC void SDLCALL SDL_hid_ble_scan(SDL2_bool active)
{
    SDL3_hid_ble_scan(active);
}


/* SDL_loadso Replaced void * with SDL_SharedObject * in SDL3. */

SDL_DECLSPEC void * SDLCALL
SDL_LoadObject(const char *soname)
{
    return (void *) SDL3_LoadObject(soname);
}

SDL_DECLSPEC void * SDLCALL
SDL_LoadFunction(void *lib, const char *sym)
{
    return (void *) SDL3_LoadFunction((SDL_SharedObject *) lib, sym);
}

SDL_DECLSPEC void SDLCALL
SDL_UnloadObject(void *lib)
{
    SDL3_UnloadObject((SDL_SharedObject *) lib);
}


#if defined(SDL_PLATFORM_WIN32) || defined(SDL_PLATFORM_GDK)
static SDL2_WindowsMessageHook g_WindowsMessageHook = NULL;

static bool SDLCALL SDL3to2_WindowsMessageHook(void *userdata, MSG *msg)
{
    g_WindowsMessageHook(userdata, msg->hwnd, msg->message, msg->wParam, msg->lParam);
    return true;
}

SDL_DECLSPEC void SDLCALL
SDL_SetWindowsMessageHook(SDL2_WindowsMessageHook callback, void *userdata)
{
    SDL_WindowsMessageHook callback3 = (callback != NULL) ? SDL3to2_WindowsMessageHook : NULL;
    g_WindowsMessageHook = callback;
    SDL3_SetWindowsMessageHook(callback3, userdata);
}
#endif

#if defined(SDL_PLATFORM_WIN32) || defined(SDL_PLATFORM_WINGDK)
SDL_DECLSPEC int SDLCALL
SDL_Direct3D9GetAdapterIndex(int displayIndex)
{
    return SDL3_GetDirect3D9AdapterIndex(Display_IndexToID(displayIndex));
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_DXGIGetOutputInfo(int displayIndex, int *adapterIndex, int *outputIndex)
{
    return SDL3_GetDXGIOutputInfo(Display_IndexToID(displayIndex), adapterIndex, outputIndex) ? SDL2_TRUE : SDL2_FALSE;
}

SDL_DECLSPEC IDirect3DDevice9* SDLCALL SDL_RenderGetD3D9Device(SDL_Renderer *renderer)
{
    return (IDirect3DDevice9 *)SDL3_GetPointerProperty(SDL3_GetRendererProperties(renderer),
                                                SDL_PROP_RENDERER_D3D9_DEVICE_POINTER, NULL);
}

SDL_DECLSPEC ID3D11Device* SDLCALL SDL_RenderGetD3D11Device(SDL_Renderer *renderer)
{
    return (ID3D11Device *)SDL3_GetPointerProperty(SDL3_GetRendererProperties(renderer),
                                            SDL_PROP_RENDERER_D3D11_DEVICE_POINTER, NULL);
}

SDL_DECLSPEC ID3D12Device* SDLCALL SDL_RenderGetD3D12Device(SDL_Renderer *renderer)
{
    return (ID3D12Device *)SDL3_GetPointerProperty(SDL3_GetRendererProperties(renderer),
                                            SDL_PROP_RENDERER_D3D12_DEVICE_POINTER, NULL);
}
#endif

#if defined(SDL_PLATFORM_GDK)
SDL_DECLSPEC int SDLCALL
SDL_GDKRunApp(SDL_main_func mainFunction, void *reserved)
{
    return SDL3_RunApp(0, NULL, mainFunction, reserved);
}
#endif

#ifdef SDL_PLATFORM_IOS
SDL_DECLSPEC int SDLCALL
SDL_UIKitRunApp(int argc, char *argv[], SDL_main_func mainFunction)
{
    return SDL3_RunApp(argc, argv, mainFunction, NULL);
}

SDL_DECLSPEC void SDLCALL
SDL_iPhoneSetEventPump(SDL2_bool enabled)
{
    SDL3_SetiOSEventPump(enabled);
}
#endif

#ifdef SDL_PLATFORM_ANDROID
SDL_DECLSPEC int SDLCALL
SDL_AndroidGetExternalStorageState(void)
{
    return (int)SDL3_GetAndroidExternalStorageState();
}

static void SDLCALL AndroidRequestPermissionBlockingCallback(void *userdata, const char *permission, bool granted)
{
    SDL3_SetAtomicInt((SDL_AtomicInt *) userdata, granted ? 1 : -1);
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_AndroidRequestPermission(const char *permission)
{
    SDL_AtomicInt response;
    SDL3_SetAtomicInt(&response, 0);

    if (!SDL3_RequestAndroidPermission(permission, AndroidRequestPermissionBlockingCallback, &response)) {
        return SDL2_FALSE;
    }

    /* Wait for the request to complete */
    while (SDL3_GetAtomicInt(&response) == 0) {
        SDL3_Delay(10);
    }

    return (SDL3_GetAtomicInt(&response) < 0) ? SDL2_FALSE : SDL2_TRUE;
}

SDL_DECLSPEC SDL2_bool SDLCALL
SDL_IsAndroidTV(void)
{
    return SDL3_IsTV() ? SDL2_TRUE : SDL2_FALSE;
}
#endif

#ifdef __cplusplus
}
#endif
