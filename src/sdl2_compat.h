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

#ifndef sdl2_compat_h
#define sdl2_compat_h

#include <SDL3/SDL_begin_code.h> /* for correct structure alignment, just in case */

/* #define SDL_INIT_TIMER 0x00000001u */ /* removed in SDL3. not used here. */

#ifdef __CC_ARM
/* ARM's compiler throws warnings if we use an enum: like "SDL2_bool x = a < b;" */
#define SDL2_FALSE 0
#define SDL2_TRUE 1
typedef int SDL2_bool;
#else
typedef enum
{
    SDL2_FALSE = 0,
    SDL2_TRUE = 1
} SDL2_bool;
#endif

/* removed in SDL3 (which only uses SDL_WINDOW_HIDDEN now). */
#define SDL2_WINDOW_SHOWN 0x000000004
#define SDL2_WINDOW_FULLSCREEN_DESKTOP (0x00001000 | SDL_WINDOW_FULLSCREEN)
#define SDL2_WINDOW_SKIP_TASKBAR 0x00010000

/* removed in SDL3 (APIs like this were split into getter/setter functions). */
#define SDL2_QUERY   -1
#define SDL2_DISABLE  0
#define SDL2_ENABLE   1

#define SDL2_MUTEX_TIMEDOUT 1

/* changed values in SDL3 */
#define SDL2_HAPTIC_CONSTANT   (1u<<0)
#define SDL2_HAPTIC_SINE       (1u<<1)
#define SDL2_HAPTIC_LEFTRIGHT     (1u<<2)
#define SDL2_HAPTIC_TRIANGLE   (1u<<3)
#define SDL2_HAPTIC_SAWTOOTHUP (1u<<4)
#define SDL2_HAPTIC_SAWTOOTHDOWN (1u<<5)
#define SDL2_HAPTIC_RAMP       (1u<<6)
#define SDL2_HAPTIC_SPRING     (1u<<7)
#define SDL2_HAPTIC_DAMPER     (1u<<8)
#define SDL2_HAPTIC_INERTIA    (1u<<9)
#define SDL2_HAPTIC_FRICTION   (1u<<10)
#define SDL2_HAPTIC_CUSTOM     (1u<<11)
#define SDL2_HAPTIC_GAIN       (1u<<12)
#define SDL2_HAPTIC_AUTOCENTER (1u<<13)
#define SDL2_HAPTIC_STATUS     (1u<<14)
#define SDL2_HAPTIC_PAUSE      (1u<<15)

#define SDL2_RENDERER_SOFTWARE      0x00000001
#define SDL2_RENDERER_ACCELERATED   0x00000002
#define SDL2_RENDERER_PRESENTVSYNC  0x00000004
#define SDL2_RENDERER_TARGETTEXTURE 0x00000008


/* SDL3 added a bus_type field, we need to workaround. */
typedef struct SDL2_hid_device_info
{
    char *path;
    unsigned short vendor_id;
    unsigned short product_id;
    wchar_t *serial_number;
    unsigned short release_number;
    wchar_t *manufacturer_string;
    wchar_t *product_string;
    unsigned short usage_page;
    unsigned short usage;
    int interface_number;
    int interface_class;
    int interface_subclass;
    int interface_protocol;
    struct SDL2_hid_device_info *next;
} SDL2_hid_device_info;


/* these types were removed from / renamed in SDL3. We need them for SDL2 APIs exported here. */

typedef int SDL2_Keymod; /* actually this is an enum in real SDL2 */

typedef enum SDL_errorcode
{
    SDL_ENOMEM,
    SDL_EFREAD,
    SDL_EFWRITE,
    SDL_EFSEEK,
    SDL_UNSUPPORTED,
    SDL_LASTERROR
} SDL_errorcode;

typedef enum SDL2_LogPriority
{
    SDL2_LOG_PRIORITY_VERBOSE = 1,
    SDL2_LOG_PRIORITY_DEBUG,
    SDL2_LOG_PRIORITY_INFO,
    SDL2_LOG_PRIORITY_WARN,
    SDL2_LOG_PRIORITY_ERROR,
    SDL2_LOG_PRIORITY_CRITICAL,
    SDL2_NUM_LOG_PRIORITIES
} SDL2_LogPriority;

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

typedef enum
{
    SDL_JOYSTICK_POWER_UNKNOWN = -1,
    SDL_JOYSTICK_POWER_EMPTY,   /* <= 5% */
    SDL_JOYSTICK_POWER_LOW,     /* <= 20% */
    SDL_JOYSTICK_POWER_MEDIUM,  /* <= 70% */
    SDL_JOYSTICK_POWER_FULL,    /* <= 100% */
    SDL_JOYSTICK_POWER_WIRED,
    SDL_JOYSTICK_POWER_MAX
} SDL_JoystickPowerLevel;

typedef Sint32 SDL2_JoystickID;  /* this became unsigned in SDL3, but we'll just hope we don't overflow. */
typedef Sint32 SDL2_SensorID;  /* this became unsigned in SDL3, but we'll just hope we don't overflow. */

typedef Sint64 SDL2_GestureID;


/* The SDL3 version of SDL_RWops (SDL_IOStream) changed, so we need to convert when necessary. */

#define SDL_HINT_APPLE_RWFROMFILE_USE_RESOURCES "SDL_APPLE_RWFROMFILE_USE_RESOURCES"

#define SDL_RWOPS_UNKNOWN   0   /**< Unknown stream type */
#define SDL_RWOPS_WINFILE   1   /**< Win32 file */
#define SDL_RWOPS_STDFILE   2   /**< Stdio file */
#define SDL_RWOPS_JNIFILE   3   /**< Android asset */
#define SDL_RWOPS_MEMORY    4   /**< Memory stream */
#define SDL_RWOPS_MEMORY_RO 5   /**< Read-Only memory stream */

#if defined(SDL_PLATFORM_WINDOWS)
#define SDL_RWOPS_PLATFORM_FILE SDL_RWOPS_WINFILE
#elif defined(SDL_PLATFORM_ANDROID)
#define SDL_RWOPS_PLATFORM_FILE SDL_RWOPS_JNIFILE
#else
#define SDL_RWOPS_PLATFORM_FILE SDL_RWOPS_STDFILE
#endif

typedef struct SDL2_RWops
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
            SDL2_bool autoclose;
            void *fp;
        } stdio;
        struct {
            void *asset;
        } androidio;
        struct {
            SDL2_bool append;
            void *h;
            struct {
                void *data;
                size_t size;
                size_t left;
            } buffer;
        } windowsio;
        struct {
            void *data1;
            void *data2;
        } unknown;
        struct {
            void *padding1;
            void *padding2;
            SDL_IOStream *iostrm;
        } sdl3;
        struct {
            void *ptrs[3];  /* just so this matches SDL2's struct size. */
        } match_sdl2;
    } hidden;
} SDL2_RWops;


typedef struct SDL2_DisplayMode
{
    Uint32 format;              /**< pixel format */
    int w;                      /**< width, in screen coordinates */
    int h;                      /**< height, in screen coordinates */
    int refresh_rate;           /**< refresh rate (or zero for unspecified) */
    void *driverdata;           /**< driver-specific data, initialize to 0 */
} SDL2_DisplayMode;


typedef enum SDL2_Scancode
{
    SDL2_SCANCODE_UNKNOWN = 0,

    /**
     *  \name Usage page 0x07
     *
     *  These values are from usage page 0x07 (USB keyboard page).
     */
    /* @{ */

    SDL2_SCANCODE_A = 4,
    SDL2_SCANCODE_B = 5,
    SDL2_SCANCODE_C = 6,
    SDL2_SCANCODE_D = 7,
    SDL2_SCANCODE_E = 8,
    SDL2_SCANCODE_F = 9,
    SDL2_SCANCODE_G = 10,
    SDL2_SCANCODE_H = 11,
    SDL2_SCANCODE_I = 12,
    SDL2_SCANCODE_J = 13,
    SDL2_SCANCODE_K = 14,
    SDL2_SCANCODE_L = 15,
    SDL2_SCANCODE_M = 16,
    SDL2_SCANCODE_N = 17,
    SDL2_SCANCODE_O = 18,
    SDL2_SCANCODE_P = 19,
    SDL2_SCANCODE_Q = 20,
    SDL2_SCANCODE_R = 21,
    SDL2_SCANCODE_S = 22,
    SDL2_SCANCODE_T = 23,
    SDL2_SCANCODE_U = 24,
    SDL2_SCANCODE_V = 25,
    SDL2_SCANCODE_W = 26,
    SDL2_SCANCODE_X = 27,
    SDL2_SCANCODE_Y = 28,
    SDL2_SCANCODE_Z = 29,

    SDL2_SCANCODE_1 = 30,
    SDL2_SCANCODE_2 = 31,
    SDL2_SCANCODE_3 = 32,
    SDL2_SCANCODE_4 = 33,
    SDL2_SCANCODE_5 = 34,
    SDL2_SCANCODE_6 = 35,
    SDL2_SCANCODE_7 = 36,
    SDL2_SCANCODE_8 = 37,
    SDL2_SCANCODE_9 = 38,
    SDL2_SCANCODE_0 = 39,

    SDL2_SCANCODE_RETURN = 40,
    SDL2_SCANCODE_ESCAPE = 41,
    SDL2_SCANCODE_BACKSPACE = 42,
    SDL2_SCANCODE_TAB = 43,
    SDL2_SCANCODE_SPACE = 44,

    SDL2_SCANCODE_MINUS = 45,
    SDL2_SCANCODE_EQUALS = 46,
    SDL2_SCANCODE_LEFTBRACKET = 47,
    SDL2_SCANCODE_RIGHTBRACKET = 48,
    SDL2_SCANCODE_BACKSLASH = 49, /**< Located at the lower left of the return
                                  *   key on ISO keyboards and at the right end
                                  *   of the QWERTY row on ANSI keyboards.
                                  *   Produces REVERSE SOLIDUS (backslash) and
                                  *   VERTICAL LINE in a US layout, REVERSE
                                  *   SOLIDUS and VERTICAL LINE in a UK Mac
                                  *   layout, NUMBER SIGN and TILDE in a UK
                                  *   Windows layout, DOLLAR SIGN and POUND SIGN
                                  *   in a Swiss German layout, NUMBER SIGN and
                                  *   APOSTROPHE in a German layout, GRAVE
                                  *   ACCENT and POUND SIGN in a French Mac
                                  *   layout, and ASTERISK and MICRO SIGN in a
                                  *   French Windows layout.
                                  */
    SDL2_SCANCODE_NONUSHASH = 50, /**< ISO USB keyboards actually use this code
                                  *   instead of 49 for the same key, but all
                                  *   OSes I've seen treat the two codes
                                  *   identically. So, as an implementor, unless
                                  *   your keyboard generates both of those
                                  *   codes and your OS treats them differently,
                                  *   you should generate SDL2_SCANCODE_BACKSLASH
                                  *   instead of this code. As a user, you
                                  *   should not rely on this code because SDL
                                  *   will never generate it with most (all?)
                                  *   keyboards.
                                  */
    SDL2_SCANCODE_SEMICOLON = 51,
    SDL2_SCANCODE_APOSTROPHE = 52,
    SDL2_SCANCODE_GRAVE = 53, /**< Located in the top left corner (on both ANSI
                              *   and ISO keyboards). Produces GRAVE ACCENT and
                              *   TILDE in a US Windows layout and in US and UK
                              *   Mac layouts on ANSI keyboards, GRAVE ACCENT
                              *   and NOT SIGN in a UK Windows layout, SECTION
                              *   SIGN and PLUS-MINUS SIGN in US and UK Mac
                              *   layouts on ISO keyboards, SECTION SIGN and
                              *   DEGREE SIGN in a Swiss German layout (Mac:
                              *   only on ISO keyboards), CIRCUMFLEX ACCENT and
                              *   DEGREE SIGN in a German layout (Mac: only on
                              *   ISO keyboards), SUPERSCRIPT TWO and TILDE in a
                              *   French Windows layout, COMMERCIAL AT and
                              *   NUMBER SIGN in a French Mac layout on ISO
                              *   keyboards, and LESS-THAN SIGN and GREATER-THAN
                              *   SIGN in a Swiss German, German, or French Mac
                              *   layout on ANSI keyboards.
                              */
    SDL2_SCANCODE_COMMA = 54,
    SDL2_SCANCODE_PERIOD = 55,
    SDL2_SCANCODE_SLASH = 56,

    SDL2_SCANCODE_CAPSLOCK = 57,

    SDL2_SCANCODE_F1 = 58,
    SDL2_SCANCODE_F2 = 59,
    SDL2_SCANCODE_F3 = 60,
    SDL2_SCANCODE_F4 = 61,
    SDL2_SCANCODE_F5 = 62,
    SDL2_SCANCODE_F6 = 63,
    SDL2_SCANCODE_F7 = 64,
    SDL2_SCANCODE_F8 = 65,
    SDL2_SCANCODE_F9 = 66,
    SDL2_SCANCODE_F10 = 67,
    SDL2_SCANCODE_F11 = 68,
    SDL2_SCANCODE_F12 = 69,

    SDL2_SCANCODE_PRINTSCREEN = 70,
    SDL2_SCANCODE_SCROLLLOCK = 71,
    SDL2_SCANCODE_PAUSE = 72,
    SDL2_SCANCODE_INSERT = 73, /**< insert on PC, help on some Mac keyboards (but
                                   does send code 73, not 117) */
    SDL2_SCANCODE_HOME = 74,
    SDL2_SCANCODE_PAGEUP = 75,
    SDL2_SCANCODE_DELETE = 76,
    SDL2_SCANCODE_END = 77,
    SDL2_SCANCODE_PAGEDOWN = 78,
    SDL2_SCANCODE_RIGHT = 79,
    SDL2_SCANCODE_LEFT = 80,
    SDL2_SCANCODE_DOWN = 81,
    SDL2_SCANCODE_UP = 82,

    SDL2_SCANCODE_NUMLOCKCLEAR = 83, /**< num lock on PC, clear on Mac keyboards
                                     */
    SDL2_SCANCODE_KP_DIVIDE = 84,
    SDL2_SCANCODE_KP_MULTIPLY = 85,
    SDL2_SCANCODE_KP_MINUS = 86,
    SDL2_SCANCODE_KP_PLUS = 87,
    SDL2_SCANCODE_KP_ENTER = 88,
    SDL2_SCANCODE_KP_1 = 89,
    SDL2_SCANCODE_KP_2 = 90,
    SDL2_SCANCODE_KP_3 = 91,
    SDL2_SCANCODE_KP_4 = 92,
    SDL2_SCANCODE_KP_5 = 93,
    SDL2_SCANCODE_KP_6 = 94,
    SDL2_SCANCODE_KP_7 = 95,
    SDL2_SCANCODE_KP_8 = 96,
    SDL2_SCANCODE_KP_9 = 97,
    SDL2_SCANCODE_KP_0 = 98,
    SDL2_SCANCODE_KP_PERIOD = 99,

    SDL2_SCANCODE_NONUSBACKSLASH = 100, /**< This is the additional key that ISO
                                        *   keyboards have over ANSI ones,
                                        *   located between left shift and Y.
                                        *   Produces GRAVE ACCENT and TILDE in a
                                        *   US or UK Mac layout, REVERSE SOLIDUS
                                        *   (backslash) and VERTICAL LINE in a
                                        *   US or UK Windows layout, and
                                        *   LESS-THAN SIGN and GREATER-THAN SIGN
                                        *   in a Swiss German, German, or French
                                        *   layout. */
    SDL2_SCANCODE_APPLICATION = 101, /**< windows contextual menu, compose */
    SDL2_SCANCODE_POWER = 102, /**< The USB document says this is a status flag,
                               *   not a physical key - but some Mac keyboards
                               *   do have a power key. */
    SDL2_SCANCODE_KP_EQUALS = 103,
    SDL2_SCANCODE_F13 = 104,
    SDL2_SCANCODE_F14 = 105,
    SDL2_SCANCODE_F15 = 106,
    SDL2_SCANCODE_F16 = 107,
    SDL2_SCANCODE_F17 = 108,
    SDL2_SCANCODE_F18 = 109,
    SDL2_SCANCODE_F19 = 110,
    SDL2_SCANCODE_F20 = 111,
    SDL2_SCANCODE_F21 = 112,
    SDL2_SCANCODE_F22 = 113,
    SDL2_SCANCODE_F23 = 114,
    SDL2_SCANCODE_F24 = 115,
    SDL2_SCANCODE_EXECUTE = 116,
    SDL2_SCANCODE_HELP = 117,    /**< AL Integrated Help Center */
    SDL2_SCANCODE_MENU = 118,    /**< Menu (show menu) */
    SDL2_SCANCODE_SELECT = 119,
    SDL2_SCANCODE_STOP = 120,    /**< AC Stop */
    SDL2_SCANCODE_AGAIN = 121,   /**< AC Redo/Repeat */
    SDL2_SCANCODE_UNDO = 122,    /**< AC Undo */
    SDL2_SCANCODE_CUT = 123,     /**< AC Cut */
    SDL2_SCANCODE_COPY = 124,    /**< AC Copy */
    SDL2_SCANCODE_PASTE = 125,   /**< AC Paste */
    SDL2_SCANCODE_FIND = 126,    /**< AC Find */
    SDL2_SCANCODE_MUTE = 127,
    SDL2_SCANCODE_VOLUMEUP = 128,
    SDL2_SCANCODE_VOLUMEDOWN = 129,
/* not sure whether there's a reason to enable these */
/*     SDL2_SCANCODE_LOCKINGCAPSLOCK = 130,  */
/*     SDL2_SCANCODE_LOCKINGNUMLOCK = 131, */
/*     SDL2_SCANCODE_LOCKINGSCROLLLOCK = 132, */
    SDL2_SCANCODE_KP_COMMA = 133,
    SDL2_SCANCODE_KP_EQUALSAS400 = 134,

    SDL2_SCANCODE_INTERNATIONAL1 = 135, /**< used on Asian keyboards, see
                                            footnotes in USB doc */
    SDL2_SCANCODE_INTERNATIONAL2 = 136,
    SDL2_SCANCODE_INTERNATIONAL3 = 137, /**< Yen */
    SDL2_SCANCODE_INTERNATIONAL4 = 138,
    SDL2_SCANCODE_INTERNATIONAL5 = 139,
    SDL2_SCANCODE_INTERNATIONAL6 = 140,
    SDL2_SCANCODE_INTERNATIONAL7 = 141,
    SDL2_SCANCODE_INTERNATIONAL8 = 142,
    SDL2_SCANCODE_INTERNATIONAL9 = 143,
    SDL2_SCANCODE_LANG1 = 144, /**< Hangul/English toggle */
    SDL2_SCANCODE_LANG2 = 145, /**< Hanja conversion */
    SDL2_SCANCODE_LANG3 = 146, /**< Katakana */
    SDL2_SCANCODE_LANG4 = 147, /**< Hiragana */
    SDL2_SCANCODE_LANG5 = 148, /**< Zenkaku/Hankaku */
    SDL2_SCANCODE_LANG6 = 149, /**< reserved */
    SDL2_SCANCODE_LANG7 = 150, /**< reserved */
    SDL2_SCANCODE_LANG8 = 151, /**< reserved */
    SDL2_SCANCODE_LANG9 = 152, /**< reserved */

    SDL2_SCANCODE_ALTERASE = 153,    /**< Erase-Eaze */
    SDL2_SCANCODE_SYSREQ = 154,
    SDL2_SCANCODE_CANCEL = 155,      /**< AC Cancel */
    SDL2_SCANCODE_CLEAR = 156,
    SDL2_SCANCODE_PRIOR = 157,
    SDL2_SCANCODE_RETURN2 = 158,
    SDL2_SCANCODE_SEPARATOR = 159,
    SDL2_SCANCODE_OUT = 160,
    SDL2_SCANCODE_OPER = 161,
    SDL2_SCANCODE_CLEARAGAIN = 162,
    SDL2_SCANCODE_CRSEL = 163,
    SDL2_SCANCODE_EXSEL = 164,

    SDL2_SCANCODE_KP_00 = 176,
    SDL2_SCANCODE_KP_000 = 177,
    SDL2_SCANCODE_THOUSANDSSEPARATOR = 178,
    SDL2_SCANCODE_DECIMALSEPARATOR = 179,
    SDL2_SCANCODE_CURRENCYUNIT = 180,
    SDL2_SCANCODE_CURRENCYSUBUNIT = 181,
    SDL2_SCANCODE_KP_LEFTPAREN = 182,
    SDL2_SCANCODE_KP_RIGHTPAREN = 183,
    SDL2_SCANCODE_KP_LEFTBRACE = 184,
    SDL2_SCANCODE_KP_RIGHTBRACE = 185,
    SDL2_SCANCODE_KP_TAB = 186,
    SDL2_SCANCODE_KP_BACKSPACE = 187,
    SDL2_SCANCODE_KP_A = 188,
    SDL2_SCANCODE_KP_B = 189,
    SDL2_SCANCODE_KP_C = 190,
    SDL2_SCANCODE_KP_D = 191,
    SDL2_SCANCODE_KP_E = 192,
    SDL2_SCANCODE_KP_F = 193,
    SDL2_SCANCODE_KP_XOR = 194,
    SDL2_SCANCODE_KP_POWER = 195,
    SDL2_SCANCODE_KP_PERCENT = 196,
    SDL2_SCANCODE_KP_LESS = 197,
    SDL2_SCANCODE_KP_GREATER = 198,
    SDL2_SCANCODE_KP_AMPERSAND = 199,
    SDL2_SCANCODE_KP_DBLAMPERSAND = 200,
    SDL2_SCANCODE_KP_VERTICALBAR = 201,
    SDL2_SCANCODE_KP_DBLVERTICALBAR = 202,
    SDL2_SCANCODE_KP_COLON = 203,
    SDL2_SCANCODE_KP_HASH = 204,
    SDL2_SCANCODE_KP_SPACE = 205,
    SDL2_SCANCODE_KP_AT = 206,
    SDL2_SCANCODE_KP_EXCLAM = 207,
    SDL2_SCANCODE_KP_MEMSTORE = 208,
    SDL2_SCANCODE_KP_MEMRECALL = 209,
    SDL2_SCANCODE_KP_MEMCLEAR = 210,
    SDL2_SCANCODE_KP_MEMADD = 211,
    SDL2_SCANCODE_KP_MEMSUBTRACT = 212,
    SDL2_SCANCODE_KP_MEMMULTIPLY = 213,
    SDL2_SCANCODE_KP_MEMDIVIDE = 214,
    SDL2_SCANCODE_KP_PLUSMINUS = 215,
    SDL2_SCANCODE_KP_CLEAR = 216,
    SDL2_SCANCODE_KP_CLEARENTRY = 217,
    SDL2_SCANCODE_KP_BINARY = 218,
    SDL2_SCANCODE_KP_OCTAL = 219,
    SDL2_SCANCODE_KP_DECIMAL = 220,
    SDL2_SCANCODE_KP_HEXADECIMAL = 221,

    SDL2_SCANCODE_LCTRL = 224,
    SDL2_SCANCODE_LSHIFT = 225,
    SDL2_SCANCODE_LALT = 226, /**< alt, option */
    SDL2_SCANCODE_LGUI = 227, /**< windows, command (apple), meta */
    SDL2_SCANCODE_RCTRL = 228,
    SDL2_SCANCODE_RSHIFT = 229,
    SDL2_SCANCODE_RALT = 230, /**< alt gr, option */
    SDL2_SCANCODE_RGUI = 231, /**< windows, command (apple), meta */

    SDL2_SCANCODE_MODE = 257,    /**< I'm not sure if this is really not covered
                                 *   by any of the above, but since there's a
                                 *   special KMOD_MODE for it I'm adding it here
                                 */

    /* @} *//* Usage page 0x07 */

    /**
     *  \name Usage page 0x0C
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     *  See https://usb.org/sites/default/files/hut1_2.pdf
     *
     *  There are way more keys in the spec than we can represent in the
     *  current scancode range, so pick the ones that commonly come up in
     *  real world usage.
     */
    /* @{ */

    SDL2_SCANCODE_AUDIONEXT = 258,
    SDL2_SCANCODE_AUDIOPREV = 259,
    SDL2_SCANCODE_AUDIOSTOP = 260,
    SDL2_SCANCODE_AUDIOPLAY = 261,
    SDL2_SCANCODE_AUDIOMUTE = 262,
    SDL2_SCANCODE_MEDIASELECT = 263,
    SDL2_SCANCODE_WWW = 264,             /**< AL Internet Browser */
    SDL2_SCANCODE_MAIL = 265,
    SDL2_SCANCODE_CALCULATOR = 266,      /**< AL Calculator */
    SDL2_SCANCODE_COMPUTER = 267,
    SDL2_SCANCODE_AC_SEARCH = 268,       /**< AC Search */
    SDL2_SCANCODE_AC_HOME = 269,         /**< AC Home */
    SDL2_SCANCODE_AC_BACK = 270,         /**< AC Back */
    SDL2_SCANCODE_AC_FORWARD = 271,      /**< AC Forward */
    SDL2_SCANCODE_AC_STOP = 272,         /**< AC Stop */
    SDL2_SCANCODE_AC_REFRESH = 273,      /**< AC Refresh */
    SDL2_SCANCODE_AC_BOOKMARKS = 274,    /**< AC Bookmarks */

    /* @} *//* Usage page 0x0C */

    /**
     *  \name Walther keys
     *
     *  These are values that Christian Walther added (for mac keyboard?).
     */
    /* @{ */

    SDL2_SCANCODE_BRIGHTNESSDOWN = 275,
    SDL2_SCANCODE_BRIGHTNESSUP = 276,
    SDL2_SCANCODE_DISPLAYSWITCH = 277, /**< display mirroring/dual display
                                           switch, video mode switch */
    SDL2_SCANCODE_KBDILLUMTOGGLE = 278,
    SDL2_SCANCODE_KBDILLUMDOWN = 279,
    SDL2_SCANCODE_KBDILLUMUP = 280,
    SDL2_SCANCODE_EJECT = 281,
    SDL2_SCANCODE_SLEEP = 282,           /**< SC System Sleep */

    SDL2_SCANCODE_APP1 = 283,
    SDL2_SCANCODE_APP2 = 284,

    /* @} *//* Walther keys */

    /**
     *  \name Usage page 0x0C (additional media keys)
     *
     *  These values are mapped from usage page 0x0C (USB consumer page).
     */
    /* @{ */

    SDL2_SCANCODE_AUDIOREWIND = 285,
    SDL2_SCANCODE_AUDIOFASTFORWARD = 286,

    /* @} *//* Usage page 0x0C (additional media keys) */

    /**
     *  \name Mobile keys
     *
     *  These are values that are often used on mobile phones.
     */
    /* @{ */

    SDL2_SCANCODE_SOFTLEFT = 287, /**< Usually situated below the display on phones and
                                      used as a multi-function feature key for selecting
                                      a software defined function shown on the bottom left
                                      of the display. */
    SDL2_SCANCODE_SOFTRIGHT = 288, /**< Usually situated below the display on phones and
                                       used as a multi-function feature key for selecting
                                       a software defined function shown on the bottom right
                                       of the display. */
    SDL2_SCANCODE_CALL = 289, /**< Used for accepting phone calls. */
    SDL2_SCANCODE_ENDCALL = 290, /**< Used for rejecting phone calls. */

    /* @} *//* Mobile keys */

    /* Add any other keys here. */

    SDL2_NUM_SCANCODES = 512 /**< not a key, just marks the number of scancodes
                                 for array bounds */
} SDL2_Scancode;

typedef struct SDL2_Keysym
{
    SDL2_Scancode scancode;
    SDL_Keycode sym;
    SDL_Keymod mod;
    Uint16 raw;
} SDL2_Keysym;


/* Events changed in SDL3; notably, the `timestamp` field moved from
   32 bit milliseconds to 64-bit nanoseconds, and the padding of the union
   changed, so all the SDL2 structs have to be reproduced here. */

/* Note that SDL_EventType _currently_ lines up; although some types have
   come and gone in SDL3, so we don't manage an SDL2 copy here atm. */

/* these were replaced in SDL3; all their subevents became top-level events. */
/* These values are reserved in SDL3's SDL_EventType enum to help sdl2-compat. */
#define SDL2_DISPLAYEVENT 0x150
#define SDL2_WINDOWEVENT 0x200

typedef enum SDL2_WindowEventID
{
    SDL_WINDOWEVENT_NONE,           /**< Never used */
    SDL_WINDOWEVENT_SHOWN,          /**< Window has been shown */
    SDL_WINDOWEVENT_HIDDEN,         /**< Window has been hidden */
    SDL_WINDOWEVENT_EXPOSED,        /**< Window has been exposed and should be
                                         redrawn */
    SDL_WINDOWEVENT_MOVED,          /**< Window has been moved to data1, data2
                                     */
    SDL_WINDOWEVENT_RESIZED,        /**< Window has been resized to data1xdata2 */
    SDL_WINDOWEVENT_SIZE_CHANGED,   /**< The window size has changed, either as
                                         a result of an API call or through the
                                         system or user changing the window size. */
    SDL_WINDOWEVENT_MINIMIZED,      /**< Window has been minimized */
    SDL_WINDOWEVENT_MAXIMIZED,      /**< Window has been maximized */
    SDL_WINDOWEVENT_RESTORED,       /**< Window has been restored to normal size
                                         and position */
    SDL_WINDOWEVENT_ENTER,          /**< Window has gained mouse focus */
    SDL_WINDOWEVENT_LEAVE,          /**< Window has lost mouse focus */
    SDL_WINDOWEVENT_FOCUS_GAINED,   /**< Window has gained keyboard focus */
    SDL_WINDOWEVENT_FOCUS_LOST,     /**< Window has lost keyboard focus */
    SDL_WINDOWEVENT_CLOSE,          /**< The window manager requests that the window be closed */
    SDL_WINDOWEVENT_TAKE_FOCUS,     /**< Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore) */
    SDL_WINDOWEVENT_HIT_TEST,       /**< Window had a hit test that wasn't SDL_HITTEST_NORMAL. */
    SDL_WINDOWEVENT_ICCPROF_CHANGED,/**< The ICC profile of the window's display has changed. */
    SDL_WINDOWEVENT_DISPLAY_CHANGED /**< Window has been moved to display data1. */
} SDL2_WindowEventID;

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
    SDL2_Keysym keysym;
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

#define SDL2_TEXTEDITING_EXT 0x305
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
typedef struct SDL2_SysWMEvent
{
    Uint32 type;
    Uint32 timestamp;
    struct SDL2_SysWMmsg *msg;
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

typedef SDL_EventAction SDL_eventaction;
typedef int (SDLCALL *SDL2_EventFilter) (void *userdata, SDL2_Event *event);

typedef struct EventFilterWrapperData
{
    SDL2_EventFilter filter2;
    void *userdata;
    struct EventFilterWrapperData *next;
} EventFilterWrapperData;

/* removed in SDL3 (no U16 audio formats supported) */
#define SDL2_AUDIO_U16LSB 0x0010  /* Unsigned 16-bit samples */
#define SDL2_AUDIO_U16MSB 0x1010  /* As above, but big-endian byte order */

typedef Uint16 SDL2_AudioFormat;

struct SDL_AudioCVT;
typedef void (SDLCALL * SDL_AudioFilter) (struct SDL_AudioCVT *cvt, SDL2_AudioFormat format);

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
    SDL2_AudioFormat format;     /**< Audio data format */
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
*/
#define SDL_AUDIOCVT_PACKED __attribute__((packed))
#else
#define SDL_AUDIOCVT_PACKED
#endif
/* */
typedef struct SDL_AudioCVT
{
    int needed;                 /**< Set to 1 if conversion possible */
    SDL2_AudioFormat src_format; /**< Source audio format */
    SDL2_AudioFormat dst_format; /**< Target audio format */
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
    SDL2_AudioFormat src_format;
    SDL2_AudioFormat dst_format;

    /* these are used when this stream is powering an opened audio device, and not when just used as an SDL2 audio stream. */
    SDL2_AudioCallback callback2;
    void *callback2_userdata;
    SDL_AudioStream *dataqueue3;
    SDL2_bool iscapture;
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
#if defined(SDL_PLATFORM_WIN32) || defined(SDL_PLATFORM_GDK)
typedef void (SDLCALL * SDL2_WindowsMessageHook)(void *userdata, void *hWnd, unsigned int message, Uint64 wParam, Sint64 lParam);
#endif

#if defined(SDL_PLATFORM_WINDOWS)
typedef uintptr_t (__cdecl * pfnSDL_CurrentBeginThread)
                        (void *, unsigned, unsigned (__stdcall *func)(void *), void *, unsigned, unsigned *);
typedef void (__cdecl * pfnSDL_CurrentEndThread) (unsigned);
#endif

typedef struct SDL2_version
{
    Uint8 major;
    Uint8 minor;
    Uint8 patch;
} SDL2_version;

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

typedef struct SDL_SysWMinfo
{
    SDL2_version version;
    SDL2_SYSWM_TYPE subsystem;
    union
    {
      struct {
        void *window;
        void *hdc;
        void *hinstance;
      } win;

      struct {
        void *window;
      } winrt;

      struct {
        void *display;
        unsigned long window;
      } x11;

      struct {
#if defined(__OBJC__) && defined(__has_feature)
#if __has_feature(objc_arc)
        NSWindow __unsafe_unretained *window;
#endif
#else
        NSWindow *window;
#endif
      } cocoa;

      struct {
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

      struct {
        void *display;
        void *surface;
        void *shell_surface;
        void *egl_window;
        void *xdg_surface;
        void *xdg_toplevel;
        void *xdg_popup;
        void *xdg_positioner;
      } wl;

      struct {
        void *window;
        void *surface;
      } android;

      struct {
        void *display;
        void *window;
      } vivante;

      struct {
        int dev_index;
        int drm_fd;
        void *gbm_dev;
      } kmsdrm;

      /* Make sure this union is always 64 bytes (8 64-bit pointers). */
      /* Be careful not to overflow this if you add a new target! */
      Uint8 dummy[64];
    } info;
} SDL_SysWMinfo;


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


typedef struct SDL2_Vertex {
    SDL_FPoint position;
    SDL_Color  color;
    SDL_FPoint tex_coord;
} SDL2_Vertex;

typedef enum
{
    SDL_YUV_CONVERSION_JPEG,        /**< Full range JPEG */
    SDL_YUV_CONVERSION_BT601,       /**< BT.601 (the default) */
    SDL_YUV_CONVERSION_BT709,       /**< BT.709 */
    SDL_YUV_CONVERSION_AUTOMATIC    /**< BT.601 for SD content, BT.709 for HD content */
} SDL_YUV_CONVERSION_MODE;


typedef struct SDL2_RendererInfo
{
    const char *name;           /**< The name of the renderer */
    Uint32 flags;               /**< Supported ::SDL_RendererFlags */
    Uint32 num_texture_formats; /**< The number of available texture formats */
    Uint32 texture_formats[16]; /**< The available texture formats */
    int max_texture_width;      /**< The maximum texture width */
    int max_texture_height;     /**< The maximum texture height */
} SDL2_RendererInfo;


typedef struct SDL2_PixelFormat
{
    Uint32 format;
    SDL_Palette *palette;
    Uint8 BitsPerPixel;
    Uint8 BytesPerPixel;
    Uint8 padding[2];
    Uint32 Rmask;
    Uint32 Gmask;
    Uint32 Bmask;
    Uint32 Amask;
    Uint8 Rloss;
    Uint8 Gloss;
    Uint8 Bloss;
    Uint8 Aloss;
    Uint8 Rshift;
    Uint8 Gshift;
    Uint8 Bshift;
    Uint8 Ashift;
    int refcount;
    struct SDL2_PixelFormat *next;
} SDL2_PixelFormat;

#define SDL_PREALLOC                0x00000001u /**< Surface uses preallocated memory */
#define SDL_RLEACCEL                0x00000002u /**< Surface is RLE encoded */
#define SDL_DONTFREE                0x00000004u /**< Surface is referenced internally */
#define SDL_SIMD_ALIGNED            0x00000008u /**< Surface uses aligned memory */
#define SHARED_SURFACE_FLAGS        (SDL_PREALLOC | SDL_RLEACCEL | SDL_SIMD_ALIGNED)

typedef struct SDL_BlitMap SDL_BlitMap;

typedef struct SDL2_Surface
{
    Uint32 flags;               /**< Read-only */
    SDL2_PixelFormat *format;   /**< Read-only */
    int w, h;                   /**< Read-only */
    int pitch;                  /**< Read-only */
    void *pixels;               /**< Read-write */

    /** Application data associated with the surface */
    void *userdata;             /**< Read-write */

    /** information needed for surfaces requiring locks */
    int locked;                 /**< Read-only */

    /** list of BlitMap that hold a reference to this surface */
    void *list_blitmap;         /**< Private */

    /** clipping information */
    SDL_Rect clip_rect;         /**< Read-only */

    /** info for fast blit mapping to other surfaces */
    SDL_BlitMap *map;           /**< Private */

    /** Reference count -- used when freeing surface */
    int refcount;               /**< Read-mostly */
} SDL2_Surface;

#define SDL_VIRTUAL_JOYSTICK_DESC_VERSION 1

typedef struct SDL2_VirtualJoystickDesc
{
    Uint16 version;     /**< `SDL_VIRTUAL_JOYSTICK_DESC_VERSION` */
    Uint16 type;        /**< `SDL_JoystickType` */
    Uint16 naxes;       /**< the number of axes on this joystick */
    Uint16 nbuttons;    /**< the number of buttons on this joystick */
    Uint16 nhats;       /**< the number of hats on this joystick */
    Uint16 vendor_id;   /**< the USB vendor ID of this joystick */
    Uint16 product_id;  /**< the USB product ID of this joystick */
    Uint16 padding;     /**< unused */
    Uint32 button_mask; /**< A mask of which buttons are valid for this controller
                             e.g. (1 << SDL_GAMEPAD_BUTTON_SOUTH) */
    Uint32 axis_mask;   /**< A mask of which axes are valid for this controller
                             e.g. (1 << SDL_GAMEPAD_AXIS_LEFTX) */
    const char *name;   /**< the name of the joystick */

    void *userdata;     /**< User data pointer passed to callbacks */
    void (SDLCALL *Update)(void *userdata); /**< Called when the joystick state should be updated */
    void (SDLCALL *SetPlayerIndex)(void *userdata, int player_index); /**< Called when the player index is set */
    int (SDLCALL *Rumble)(void *userdata, Uint16 low_frequency_rumble, Uint16 high_frequency_rumble); /**< Implements SDL_RumbleJoystick() */
    int (SDLCALL *RumbleTriggers)(void *userdata, Uint16 left_rumble, Uint16 right_rumble); /**< Implements SDL_RumbleJoystickTriggers() */
    int (SDLCALL *SetLED)(void *userdata, Uint8 red, Uint8 green, Uint8 blue); /**< Implements SDL_SetJoystickLED() */
    int (SDLCALL *SendEffect)(void *userdata, const void *data, int size); /**< Implements SDL_SendJoystickEffect() */
} SDL2_VirtualJoystickDesc;

typedef int SDL2_TimerID;
typedef Uint32 (SDLCALL * SDL2_TimerCallback) (Uint32 interval, void *param);

typedef unsigned int SDL2_TLSID;

#include <SDL3/SDL_close_code.h>

#endif /* sdl2_compat_h */
