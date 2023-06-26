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

#ifndef sdl2_compat_h
#define sdl2_compat_h

/* these types were removed from / renamed in SDL3. We need them for SDL2 APIs exported here. */

typedef SDL_AtomicInt SDL_atomic_t;

typedef SDL_Condition SDL_cond;
typedef SDL_Mutex SDL_mutex;
typedef SDL_Semaphore SDL_sem;

typedef SDL_Gamepad SDL_GameController;  /* since they're opaque types, for simplicity we just typedef it here and use the old types in sdl3_syms.h */
typedef SDL_GamepadAxis SDL_GameControllerAxis;
typedef SDL_GamepadBinding SDL_GameControllerButtonBind;
typedef SDL_GamepadButton SDL_GameControllerButton;
typedef SDL_GamepadType SDL_GameControllerType;

typedef Sint32 SDL2_JoystickID;  /* this became unsigned in SDL3, but we'll just hope we don't overflow. */
typedef Sint32 SDL2_SensorID;  /* this became unsigned in SDL3, but we'll just hope we don't overflow. */

typedef Sint64 SDL2_GestureID;

typedef struct SDL2_RWops SDL2_RWops;
typedef struct SDL2_DisplayMode SDL2_DisplayMode;


typedef struct SDL2_hid_device_info SDL2_hid_device_info;

typedef union SDL2_Event SDL2_Event;
typedef int (SDLCALL *SDL2_EventFilter) (void *userdata, SDL2_Event *event);

struct SDL_AudioCVT;
typedef void (SDLCALL * SDL_AudioFilter) (struct SDL_AudioCVT *cvt, SDL_AudioFormat format);

/**
 *  \brief Upper limit of filters in SDL_AudioCVT
 *
 *  The maximum number of SDL_AudioFilter functions in SDL_AudioCVT is
 *  currently limited to 9. The SDL_AudioCVT.filters array has 10 pointers,
 *  one of which is the terminating NULL pointer.
 */
#define SDL_AUDIOCVT_MAX_FILTERS 9

/**
 *  \struct SDL_AudioCVT
 *  \brief A structure to hold a set of audio conversion filters and buffers.
 *
 *  Note that various parts of the conversion pipeline can take advantage
 *  of SIMD operations (like SSE2, for example). SDL_AudioCVT doesn't require
 *  you to pass it aligned data, but can possibly run much faster if you
 *  set both its (buf) field to a pointer that is aligned to 16 bytes, and its
 *  (len) field to something that's a multiple of 16, if possible.
 */
#if defined(__GNUC__) && !defined(__CHERI_PURE_CAPABILITY__)
/* This structure is 84 bytes on 32-bit architectures, make sure GCC doesn't
   pad it out to 88 bytes to guarantee ABI compatibility between compilers.
   This is not a concern on CHERI architectures, where pointers must be stored
   at aligned locations otherwise they will become invalid, and thus structs
   containing pointers cannot be packed without giving a warning or error.
   vvv
   The next time we rev the ABI, make sure to size the ints and add padding.
*/
#define SDL_AUDIOCVT_PACKED __attribute__((packed))
#else
#define SDL_AUDIOCVT_PACKED
#endif
/* */
typedef struct SDL_AudioCVT
{
    int needed;                 /**< Set to 1 if conversion possible */
    SDL_AudioFormat src_format; /**< Source audio format */
    SDL_AudioFormat dst_format; /**< Target audio format */
    double rate_incr;           /**< Rate conversion increment */
    Uint8 *buf;                 /**< Buffer to hold entire audio data */
    int len;                    /**< Length of original audio buffer */
    int len_cvt;                /**< Length of converted audio buffer */
    int len_mult;               /**< buffer must be len*len_mult big */
    double len_ratio;           /**< Given len, final size is len*len_ratio */
    SDL_AudioFilter filters[SDL_AUDIOCVT_MAX_FILTERS + 1]; /**< NULL-terminated list of filter functions */
    int filter_index;           /**< Current audio conversion function */
} SDL_AUDIOCVT_PACKED SDL_AudioCVT;

typedef struct SDL2_AudioStream
{
    SDL_AudioStream *stream3;
    SDL_AudioFormat src_format;
    SDL_AudioFormat dst_format;
} SDL2_AudioStream;

typedef enum
{
  SDL2_SYSWM_UNKNOWN,
  SDL2_SYSWM_WINDOWS,
  SDL2_SYSWM_X11,
  SDL2_SYSWM_DIRECTFB,
  SDL2_SYSWM_COCOA,
  SDL2_SYSWM_UIKIT,
  SDL2_SYSWM_WAYLAND,
  SDL2_SYSWM_MIR,  /* no longer available, left for API/ABI compatibility. */
  SDL2_SYSWM_WINRT,
  SDL2_SYSWM_ANDROID,
  SDL2_SYSWM_VIVANTE,
  SDL2_SYSWM_OS2,
  SDL2_SYSWM_HAIKU,
  SDL2_SYSWM_KMSDRM,
  SDL2_SYSWM_RISCOS
} SDL2_SYSWM_TYPE;

struct SDL2_SysWMinfo
{
    SDL_version version;
    SDL2_SYSWM_TYPE subsystem;
    union
    {
#if defined(SDL_ENABLE_SYSWM_WINDOWS)
      struct
      {
        HWND window;                /**< The window handle */
        HDC hdc;                    /**< The window device context */
        HINSTANCE hinstance;        /**< The instance handle */
      } win;
#endif
#if defined(SDL_ENABLE_SYSWM_WINRT)
      struct
      {
        IInspectable * window;      /**< The WinRT CoreWindow */
      } winrt;
#endif
#if defined(SDL_ENABLE_SYSWM_X11)
      struct
      {
        Display *display;           /**< The X11 display */
        Window window;              /**< The X11 window */
      } x11;
#endif
#if defined(SDL_ENABLE_SYSWM_DIRECTFB)
      struct
      {
        IDirectFB *dfb;             /**< The directfb main interface */
        IDirectFBWindow *window;    /**< The directfb window handle */
        IDirectFBSurface *surface;  /**< The directfb client surface */
      } dfb;
#endif
#if defined(SDL_ENABLE_SYSWM_COCOA)
      struct
      {
#if defined(__OBJC__) && defined(__has_feature)
#if __has_feature(objc_arc)
        NSWindow __unsafe_unretained *window; /**< The Cocoa window */
#else
        NSWindow *window;                     /**< The Cocoa window */
#endif
#else
        NSWindow *window;                     /**< The Cocoa window */
#endif
      } cocoa;
#endif
#if defined(SDL_ENABLE_SYSWM_UIKIT)
      struct
      {
#if defined(__OBJC__) && defined(__has_feature)
#if __has_feature(objc_arc)
        UIWindow __unsafe_unretained *window; /**< The UIKit window */
#else
        UIWindow *window;                     /**< The UIKit window */
#endif
#else
        UIWindow *window;                     /**< The UIKit window */
#endif
        GLuint framebuffer; /**< The GL view's Framebuffer Object. It must be bound when rendering to the screen using GL. */
        GLuint colorbuffer; /**< The GL view's color Renderbuffer Object. It must be bound when SDL_GL_SwapWindow is called. */
        GLuint resolveFramebuffer; /**< The Framebuffer Object which holds the resolve color Renderbuffer, when MSAA is used. */
      } uikit;
#endif
#if defined(SDL_ENABLE_SYSWM_WAYLAND)
      struct
      {
        struct wl_display *display;             /**< Wayland display */
        struct wl_surface *surface;             /**< Wayland surface */
        void *shell_surface;                    /**< DEPRECATED Wayland shell_surface (window manager handle) */
        struct wl_egl_window *egl_window;       /**< Wayland EGL window (native window) */
        struct xdg_surface *xdg_surface;        /**< Wayland xdg surface (window manager handle) */
        struct xdg_toplevel *xdg_toplevel;      /**< Wayland xdg toplevel role */
        struct xdg_popup *xdg_popup;            /**< Wayland xdg popup role */
        struct xdg_positioner *xdg_positioner;  /**< Wayland xdg positioner, for popup */
      } wl;
#endif

#if defined(SDL_ENABLE_SYSWM_ANDROID)
      struct
      {
        ANativeWindow *window;
        EGLSurface surface;
      } android;
#endif

#if defined(SDL_ENABLE_SYSWM_OS2)
      struct
      {
        HWND hwnd;                  /**< The window handle */
        HWND hwndFrame;             /**< The frame window handle */
      } os2;
#endif

#if defined(SDL_ENABLE_SYSWM_VIVANTE)
      struct
      {
        EGLNativeDisplayType display;
        EGLNativeWindowType window;
      } vivante;
#endif

#if defined(SDL_ENABLE_SYSWM_KMSDRM)
      struct
      {
        int dev_index;               /**< Device index (ex: the X in /dev/dri/cardX) */
        int drm_fd;                  /**< DRM FD (unavailable on Vulkan windows) */
        struct gbm_device *gbm_dev;  /**< GBM device (unavailable on Vulkan windows) */
      } kmsdrm;
#endif

      /* Make sure this union is always 64 bytes (8 64-bit pointers). */
      /* Be careful not to overflow this if you add a new target! */
      Uint8 dummy[64];
    } info;
};

typedef struct SDL2_SysWMinfo SDL2_SysWMinfo;

#endif /* sdl2_compat_h */
