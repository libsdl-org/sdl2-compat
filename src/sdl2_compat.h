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

#endif /* sdl2_compat_h */
