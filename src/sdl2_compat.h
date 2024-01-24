/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2024 Sam Lantinga <slouken@libsdl.org>

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

typedef SDL_GamepadBindingType SDL_GameControllerBindType;

typedef struct SDL_GameControllerButtonBind
{
    SDL_GameControllerBindType bindType;
    union
    {
        int button;
        int axis;
        struct {
            int hat;
            int hat_mask;
        } hat;
    } value;

} SDL_GameControllerButtonBind;

typedef enum
{
    SDL_CONTROLLER_TYPE_UNKNOWN = 0,
    SDL_CONTROLLER_TYPE_XBOX360,
    SDL_CONTROLLER_TYPE_XBOXONE,
    SDL_CONTROLLER_TYPE_PS3,
    SDL_CONTROLLER_TYPE_PS4,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO,
    SDL_CONTROLLER_TYPE_VIRTUAL,
    SDL_CONTROLLER_TYPE_PS5,
    SDL_CONTROLLER_TYPE_AMAZON_LUNA,
    SDL_CONTROLLER_TYPE_GOOGLE_STADIA,
    SDL_CONTROLLER_TYPE_NVIDIA_SHIELD,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_LEFT,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT,
    SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_JOYCON_PAIR
} SDL_GameControllerType;

typedef SDL_Gamepad SDL_GameController;  /* since they're opaque types, for simplicity we just typedef it here and use the old types in sdl3_syms.h */
typedef SDL_GamepadAxis SDL_GameControllerAxis;
typedef SDL_GamepadButton SDL_GameControllerButton;

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

typedef void (SDLCALL * SDL2_AudioCallback) (void *userdata, Uint8 * stream, int len);

typedef enum
{
    SDL2_AUDIO_STOPPED = 0,
    SDL2_AUDIO_PLAYING,
    SDL2_AUDIO_PAUSED
} SDL2_AudioStatus;

typedef struct SDL2_AudioSpec
{
    int freq;                    /**< DSP frequency -- samples per second */
    SDL_AudioFormat format;      /**< Audio data format */
    Uint8 channels;              /**< Number of channels: 1 mono, 2 stereo */
    Uint8 silence;               /**< Audio buffer silence value (calculated) */
    Uint16 samples;              /**< Audio buffer size in sample FRAMES (total samples divided by channel count) */
    Uint16 padding;              /**< Necessary for some compile environments */
    Uint32 size;                 /**< Audio buffer size in bytes (calculated) */
    SDL2_AudioCallback callback; /**< Callback that feeds the audio device (NULL to use SDL_QueueAudio()). */
    void *userdata;              /**< Userdata passed to callback (ignored for NULL callbacks). */
} SDL2_AudioSpec;

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

    /* these are used when this stream is powering an opened audio device, and not when just used as an SDL2 audio stream. */
    SDL2_AudioCallback callback2;
    void *callback2_userdata;
    SDL_AudioStream *dataqueue3;
    SDL_bool iscapture;
} SDL2_AudioStream;

#define SDL2_AUDIO_ALLOW_FREQUENCY_CHANGE    0x00000001
#define SDL2_AUDIO_ALLOW_FORMAT_CHANGE       0x00000002
#define SDL2_AUDIO_ALLOW_CHANNELS_CHANGE     0x00000004
#define SDL2_AUDIO_ALLOW_SAMPLES_CHANGE      0x00000008
#define SDL2_AUDIO_ALLOW_ANY_CHANGE          (SDL2_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL2_AUDIO_ALLOW_FORMAT_CHANGE|SDL2_AUDIO_ALLOW_CHANNELS_CHANGE|SDL2_AUDIO_ALLOW_SAMPLES_CHANGE)

/* Prototypes for D3D devices */
#if defined(SDL_PLATFORM_WIN32) || defined(SDL_PLATFORM_WINGDK)
typedef struct IDirect3DDevice9 IDirect3DDevice9;
typedef struct ID3D11Device ID3D11Device;
typedef struct ID3D12Device ID3D12Device;
#endif

/* SDL2 SysWM mapping */
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

#ifdef __OBJC__
@class NSWindow;
@class UIWindow;
#else
typedef struct _NSWindow NSWindow;
typedef struct _UIWindow UIWindow;
#endif

struct SDL_SysWMinfo
{
    SDL_version version;
    SDL2_SYSWM_TYPE subsystem;
    union
    {
      struct
      {
        void *window;
        void *hdc;
        void *hinstance;
      } win;

      struct
      {
        void *window;
      } winrt;

      struct
      {
        void *display;
        unsigned long window;
      } x11;

      struct
      {
#if defined(__OBJC__) && defined(__has_feature)
#if __has_feature(objc_arc)
        NSWindow __unsafe_unretained *window;
#endif
#else
        NSWindow *window;
#endif
      } cocoa;

      struct
      {
#if defined(__OBJC__) && defined(__has_feature)
#if __has_feature(objc_arc)
        UIWindow __unsafe_unretained *window;
#endif
#else
        UIWindow *window;
#endif
        Uint32 framebuffer;
        Uint32 colorbuffer;
        Uint32 resolveFramebuffer;
      } uikit;

      struct
      {
        void *display;
        void *surface;
        void *shell_surface;
        void *egl_window;
        void *xdg_surface;
        void *xdg_toplevel;
        void *xdg_popup;
        void *xdg_positioner;
      } wl;

      struct
      {
        void *window;
        void *surface;
      } android;

      struct
      {
        void *display;
        void *window;
      } vivante;

      struct
      {
        int dev_index;
        int drm_fd;
        void *gbm_dev;
      } kmsdrm;

      /* Make sure this union is always 64 bytes (8 64-bit pointers). */
      /* Be careful not to overflow this if you add a new target! */
      Uint8 dummy[64];
    } info;
};

typedef struct SDL_SysWMinfo SDL_SysWMinfo;


#define SDL_NONSHAPEABLE_WINDOW -1
#define SDL_INVALID_SHAPE_ARGUMENT -2
#define SDL_WINDOW_LACKS_SHAPE -3

typedef enum {
    ShapeModeDefault,
    ShapeModeBinarizeAlpha,
    ShapeModeReverseBinarizeAlpha,
    ShapeModeColorKey
} WindowShapeMode;

#define SDL_SHAPEMODEALPHA(mode) (mode == ShapeModeDefault || mode == ShapeModeBinarizeAlpha || mode == ShapeModeReverseBinarizeAlpha)
typedef union {
    Uint8 binarizationCutoff;
    SDL_Color colorKey;
} SDL_WindowShapeParams;

typedef struct SDL_WindowShapeMode {
    WindowShapeMode mode;
    SDL_WindowShapeParams parameters;
} SDL_WindowShapeMode;

#endif /* sdl2_compat_h */
