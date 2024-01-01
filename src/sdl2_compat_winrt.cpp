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

/* This file contains some WinRT-specific support code */

#include <windows.h>

#include <windows.ui.popups.h>
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::UI::Popups;

static String ^ WINRT_PlatformString(const WCHAR *wstr)
{
    String ^ rtstr = ref new String(wstr);
    return rtstr;
}

static String ^ WINRT_UTF8ToPlatformString(const char *str)
{
    WCHAR wstr[256];
    unsigned int i;
    for (i = 0; i < (ARRAYSIZE(wstr) - 1) && str[i]; i++) {
        wstr[i] = (WCHAR) str[i]; /* low-ASCII maps to WCHAR directly. */
    }
    wstr[i] = 0;
    return WINRT_PlatformString(wstr);
}

extern "C"
void error_dialog(const char *errorMsg)
{
    /* Build a MessageDialog object and its buttons */
    MessageDialog ^ dialog = ref new MessageDialog(WINRT_UTF8ToPlatformString(errorMsg));
    dialog->Title = WINRT_PlatformString(L"Error");
    UICommand ^ button = ref new UICommand(WINRT_PlatformString(L"OK"));
    button->Id = IntPtr(0);
    dialog->Commands->Append(button);
    dialog->CancelCommandIndex = 0;
    dialog->DefaultCommandIndex = 0;
    /* Display the MessageDialog, then wait for it to be closed */
    auto operation = dialog->ShowAsync();
    while (operation->Status == Windows::Foundation::AsyncStatus::Started) {
      /* do anything here? */ ;
    }
}

/* vi: set ts=4 sw=4 expandtab: */
