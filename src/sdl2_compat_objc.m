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

/* This file contains some macOS-specific support code */

#if defined(__APPLE__)
#include <Cocoa/Cocoa.h>

#if __GNUC__ >= 4
#define SDL2_PRIVATE __attribute__((visibility("hidden")))
#else
#define SDL2_PRIVATE __private_extern__
#endif

#ifndef MAC_OS_X_VERSION_10_12
#define NSAlertStyleCritical NSCriticalAlertStyle
#endif

SDL2_PRIVATE void error_dialog(const char *errorMsg)
{
    NSAlert *alert;

    if (NSApp == nil) {
        ProcessSerialNumber psn = { 0, kCurrentProcess };
        TransformProcessType(&psn, kProcessTransformToForegroundApplication);
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
        [NSApp finishLaunching];
    }

    [NSApp activateIgnoringOtherApps:YES];
    alert = [[[NSAlert alloc] init] autorelease];
    alert.alertStyle = NSAlertStyleCritical;
    alert.messageText = @"Fatal error! Cannot continue!";
    alert.informativeText = [NSString stringWithCString:errorMsg encoding:NSASCIIStringEncoding];
    [alert runModal];
}
#endif

/* vi: set ts=4 sw=4 expandtab: */

