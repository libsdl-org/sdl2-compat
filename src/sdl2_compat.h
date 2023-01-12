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

/* these types were removed from SDL3, but we need them for SDL2 APIs exported here. */

typedef SDL_Gamepad SDL_GameController;  /* since they're opaque types, for simplicity we just typedef it here and use the old types in sdl3_syms.h */
typedef SDL_GamepadAxis SDL_GameControllerAxis;
typedef SDL_GamepadBinding SDL_GameControllerButtonBind;
typedef SDL_GamepadButton SDL_GameControllerButton;
typedef SDL_GamepadType SDL_GameControllerType;

typedef Sint32 SDL2_JoystickID;  /* this became unsigned in SDL3, but we'll just hope we don't overflow. */
typedef Sint32 SDL2_SensorID;  /* this became unsigned in SDL3, but we'll just hope we don't overflow. */

typedef Sint64 SDL2_GestureID;

#endif /* sdl2_compat_h */
