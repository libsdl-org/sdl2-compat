/*
  Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
#include <stdio.h>
#include "SDL.h"
#include "SDL_test.h"

static void
print_devices(int iscapture)
{
    SDL_AudioSpec spec;
    const char *typestr = ((iscapture) ? "capture" : "output");
    int n = SDL_GetNumAudioDevices(iscapture);

    SDL_Log("Found %d %s device%s:\n", n, typestr, n != 1 ? "s" : "");

    if (n == -1) {
        SDL_Log("  Driver can't detect specific %s devices.\n\n", typestr);
    } else if (n == 0) {
        SDL_Log("  No %s devices found.\n\n", typestr);
    } else {
        int i;
        for (i = 0; i < n; i++) {
            const char *name = SDL_GetAudioDeviceName(i, iscapture);
            if (name) {
                SDL_Log("  %d: %s\n", i, name);
            } else {
                SDL_Log("  %d Error: %s\n", i, SDL_GetError());
            }

            if (SDL_GetAudioDeviceSpec(i, iscapture, &spec) == 0) {
                SDL_Log("     Sample Rate: %d\n", spec.freq);
                SDL_Log("     Channels: %d\n", spec.channels);
                SDL_Log("     SDL_AudioFormat: %X\n", spec.format);
            }
        }
        SDL_Log("\n");
    }
}

int main(int argc, char **argv)
{
    char *deviceName = NULL;
    SDL_AudioSpec spec;
    int n;
    SDLTest_CommonState *state;

    state = SDLTest_CommonCreateState(argv, SDL_INIT_AUDIO);
    if (!state) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDLTest_CommonCreateState failed: %s\n", SDL_GetError());
        return 1;
    }

    /* Enable standard application logging */
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

    if (!SDLTest_CommonDefaultArgs(state, argc, argv)) {
        return 1;
    }

    if (!SDLTest_CommonInit(state)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s\n", SDL_GetError());
        return 1;
    }

    /* Print available audio drivers */
    n = SDL_GetNumAudioDrivers();
    if (n == 0) {
        SDL_Log("No built-in audio drivers\n\n");
    } else {
        int i;
        SDL_Log("Built-in audio drivers:\n");
        for (i = 0; i < n; ++i) {
            SDL_Log("  %d: %s\n", i, SDL_GetAudioDriver(i));
        }
        SDL_Log("Select a driver with the SDL_AUDIODRIVER environment variable.\n");
    }

    SDL_Log("Using audio driver: %s\n\n", SDL_GetCurrentAudioDriver());

    print_devices(0);
    print_devices(1);

    if (SDL_GetDefaultAudioInfo(&deviceName, &spec, 0) < 0) {
        SDL_Log("Error when calling SDL_GetDefaultAudioInfo: %s\n", SDL_GetError());
    } else {
        SDL_Log("Default Output Name: %s\n", deviceName ? deviceName : "unknown");
        SDL_free(deviceName);
        SDL_Log("Sample Rate: %d\n", spec.freq);
        SDL_Log("Channels: %d\n", spec.channels);
        SDL_Log("SDL_AudioFormat: %X\n", spec.format);
    }

    if (SDL_GetDefaultAudioInfo(&deviceName, &spec, 1) < 0) {
        SDL_Log("Error when calling SDL_GetDefaultAudioInfo: %s\n", SDL_GetError());
    } else {
        SDL_Log("Default Capture Name: %s\n", deviceName ? deviceName : "unknown");
        SDL_free(deviceName);
        SDL_Log("Sample Rate: %d\n", spec.freq);
        SDL_Log("Channels: %d\n", spec.channels);
        SDL_Log("SDL_AudioFormat: %X\n", spec.format);
    }

    SDLTest_CommonQuit(state);
    return 0;
}
