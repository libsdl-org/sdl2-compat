# (I updated this to change '2' to '3'. There will probably be a more
#  formal FindSDL3.cmake at some point. --ryan.)
#
# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#  Copyright 2019 Amine Ben Hassouna <amine.benhassouna@gmail.com>
#  Copyright 2000-2019 Kitware, Inc. and Contributors
#  All rights reserved.

#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:

#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.

#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.

#  * Neither the name of Kitware, Inc. nor the names of Contributors
#    may be used to endorse or promote products derived from this
#    software without specific prior written permission.

#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#[=======================================================================[.rst:
FindSDL3
--------

Locate SDL3 library

This module defines the following 'IMPORTED' targets:

::

  SDL3::Core
    The SDL3 library, if found.
    Libraries should link to SDL3::Core

  SDL3::Main
    The SDL3main library, if found.
    Applications should link to SDL3::Main instead of SDL3::Core



This module will set the following variables in your project:

::

  SDL3_LIBRARIES, the name of the library to link against
  SDL3_INCLUDE_DIRS, where to find SDL.h
  SDL3_FOUND, if false, do not try to link to SDL3
  SDL3MAIN_FOUND, if false, do not try to link to SDL3main
  SDL3_VERSION_STRING, human-readable string containing the version of SDL3



This module responds to the following cache variables:

::

  SDL3_PATH
    Set a custom SDL3 Library path (default: empty)

  SDL3_NO_DEFAULT_PATH
    Disable search SDL3 Library in default path.
      If SDL3_PATH (default: ON)
      Else (default: OFF)

  SDL3_INCLUDE_DIR
    SDL3 headers path.

  SDL3_LIBRARY
    SDL3 Library (.dll, .so, .a, etc) path.

  SDL3MAIN_LIBRAY
    SDL3main Library (.a) path.

  SDL3_BUILDING_LIBRARY
    This flag is useful only when linking to SDL3_LIBRARIES insead of
    SDL3::Main. It is required only when building a library that links to
    SDL3_LIBRARIES, because only applications need main() (No need to also
    link to SDL3main).
    If this flag is defined, then no SDL3main will be added to SDL3_LIBRARIES
    and no SDL3::Main target will be created.


Don't forget to include SDLmain.h and SDLmain.m in your project for the
OS X framework based version. (Other versions link to -lSDL3main which
this module will try to find on your behalf.) Also for OS X, this
module will automatically add the -framework Cocoa on your behalf.


Additional Note: If you see an empty SDL3_LIBRARY in your project
configuration, it means CMake did not find your SDL3 library
(SDL3.dll, libsdl3.so, SDL3.framework, etc). Set SDL3_LIBRARY to point
to your SDL3 library, and  configure again. Similarly, if you see an
empty SDL3MAIN_LIBRARY, you should set this value as appropriate. These
values are used to generate the final SDL3_LIBRARIES variable and the
SDL3::Core and SDL3::Main targets, but when these values are unset,
SDL3_LIBRARIES, SDL3::Core and SDL3::Main does not get created.


$SDL3DIR is an environment variable that would correspond to the
./configure --prefix=$SDL3DIR used in building SDL3.  l.e.galup 9-20-02



Created by Amine Ben Hassouna:
  Adapt FindSDL.cmake to SDL3 (FindSDL3.cmake).
  Add cache variables for more flexibility:
    SDL3_PATH, SDL3_NO_DEFAULT_PATH (for details, see doc above).
  Mark 'Threads' as a required dependency for non-OSX systems.
  Modernize the FindSDL3.cmake module by creating specific targets:
    SDL3::Core and SDL3::Main (for details, see doc above).


Original FindSDL.cmake module:
  Modified by Eric Wing.  Added code to assist with automated building
  by using environmental variables and providing a more
  controlled/consistent search behavior.  Added new modifications to
  recognize OS X frameworks and additional Unix paths (FreeBSD, etc).
  Also corrected the header search path to follow "proper" SDL
  guidelines.  Added a search for SDLmain which is needed by some
  platforms.  Added a search for threads which is needed by some
  platforms.  Added needed compile switches for MinGW.

On OSX, this will prefer the Framework version (if found) over others.
People will have to manually change the cache value of SDL3_LIBRARY to
override this selection or set the SDL3_PATH variable or the CMake
environment CMAKE_INCLUDE_PATH to modify the search paths.

Note that the header path has changed from SDL/SDL.h to just SDL.h
This needed to change because "proper" SDL convention is #include
"SDL.h", not <SDL/SDL.h>.  This is done for portability reasons
because not all systems place things in SDL/ (see FreeBSD).
#]=======================================================================]

# Define options for searching SDL3 Library in a custom path

set(SDL3_PATH "" CACHE STRING "Custom SDL3 Library path")

set(_SDL3_NO_DEFAULT_PATH OFF)
if(SDL3_PATH)
  set(_SDL3_NO_DEFAULT_PATH ON)
endif()

set(SDL3_NO_DEFAULT_PATH ${_SDL3_NO_DEFAULT_PATH}
    CACHE BOOL "Disable search SDL3 Library in default path")
unset(_SDL3_NO_DEFAULT_PATH)

set(SDL3_NO_DEFAULT_PATH_CMD)
if(SDL3_NO_DEFAULT_PATH)
  set(SDL3_NO_DEFAULT_PATH_CMD NO_DEFAULT_PATH)
endif()

# Search for the SDL3 include directory
find_path(SDL3_INCLUDE_DIR SDL.h
  HINTS
    ENV SDL3DIR
    ${SDL3_NO_DEFAULT_PATH_CMD}
  PATH_SUFFIXES SDL3
                # path suffixes to search inside ENV{SDL3DIR}
                include/SDL3 include
  PATHS ${SDL3_PATH}
  DOC "Where the SDL3 headers can be found"
)

set(SDL3_INCLUDE_DIRS "${SDL3_INCLUDE_DIR}")

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(VC_LIB_PATH_SUFFIX lib/x64)
else()
  set(VC_LIB_PATH_SUFFIX lib/x86)
endif()

# SDL-3.0 is the name used by FreeBSD ports...
# don't confuse it for the version number.
find_library(SDL3_LIBRARY
  NAMES SDL3 SDL-3.0
  HINTS
    ENV SDL3DIR
    ${SDL3_NO_DEFAULT_PATH_CMD}
  PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
  PATHS ${SDL3_PATH}
  DOC "Where the SDL3 Library can be found"
)

set(SDL3_LIBRARIES "${SDL3_LIBRARY}")

if(NOT SDL3_BUILDING_LIBRARY)
  if(NOT SDL3_INCLUDE_DIR MATCHES ".framework")
    # Non-OS X framework versions expect you to also dynamically link to
    # SDL3main. This is mainly for Windows and OS X. Other (Unix) platforms
    # seem to provide SDL3main for compatibility even though they don't
    # necessarily need it.

    if(SDL3_PATH)
      set(SDL3MAIN_LIBRARY_PATHS "${SDL3_PATH}")
    endif()

    if(NOT SDL3_NO_DEFAULT_PATH)
      set(SDL3MAIN_LIBRARY_PATHS
            /sw
            /opt/local
            /opt/csw
            /opt
            "${SDL3MAIN_LIBRARY_PATHS}"
      )
    endif()

    find_library(SDL3MAIN_LIBRARY
      NAMES SDL3main
      HINTS
        ENV SDL3DIR
        ${SDL3_NO_DEFAULT_PATH_CMD}
      PATH_SUFFIXES lib ${VC_LIB_PATH_SUFFIX}
      PATHS ${SDL3MAIN_LIBRARY_PATHS}
      DOC "Where the SDL3main library can be found"
    )
    unset(SDL3MAIN_LIBRARY_PATHS)
  endif()
endif()

# SDL3 may require threads on your system.
# The Apple build may not need an explicit flag because one of the
# frameworks may already provide it.
# But for non-OSX systems, I will use the CMake Threads package.
if(NOT APPLE)
  find_package(Threads QUIET)
  if(NOT Threads_FOUND)
    set(SDL3_THREADS_NOT_FOUND "Could NOT find Threads (Threads is required by SDL3).")
    if(SDL3_FIND_REQUIRED)
      message(FATAL_ERROR ${SDL3_THREADS_NOT_FOUND})
    else()
        if(NOT SDL3_FIND_QUIETLY)
          message(STATUS ${SDL3_THREADS_NOT_FOUND})
        endif()
      return()
    endif()
    unset(SDL3_THREADS_NOT_FOUND)
  endif()
endif()

# MinGW needs an additional link flag, -mwindows
# It's total link flags should look like -lmingw32 -lSDL3main -lSDL3 -mwindows
if(MINGW)
  set(MINGW32_LIBRARY mingw32 "-mwindows" CACHE STRING "link flags for MinGW")
endif()

if(SDL3_LIBRARY)
  # For SDL3main
  if(SDL3MAIN_LIBRARY AND NOT SDL3_BUILDING_LIBRARY)
    list(FIND SDL3_LIBRARIES "${SDL3MAIN_LIBRARY}" _SDL3_MAIN_INDEX)
    if(_SDL3_MAIN_INDEX EQUAL -1)
      set(SDL3_LIBRARIES "${SDL3MAIN_LIBRARY}" ${SDL3_LIBRARIES})
    endif()
    unset(_SDL3_MAIN_INDEX)
  endif()

  # For OS X, SDL3 uses Cocoa as a backend so it must link to Cocoa.
  # CMake doesn't display the -framework Cocoa string in the UI even
  # though it actually is there if I modify a pre-used variable.
  # I think it has something to do with the CACHE STRING.
  # So I use a temporary variable until the end so I can set the
  # "real" variable in one-shot.
  if(APPLE)
    set(SDL3_LIBRARIES ${SDL3_LIBRARIES} -framework Cocoa)
  endif()

  # For threads, as mentioned Apple doesn't need this.
  # In fact, there seems to be a problem if I used the Threads package
  # and try using this line, so I'm just skipping it entirely for OS X.
  if(NOT APPLE)
    set(SDL3_LIBRARIES ${SDL3_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
  endif()

  # For MinGW library
  if(MINGW)
    set(SDL3_LIBRARIES ${MINGW32_LIBRARY} ${SDL3_LIBRARIES})
  endif()

endif()

# Read SDL3 version
if(SDL3_INCLUDE_DIR AND EXISTS "${SDL3_INCLUDE_DIR}/SDL_version.h")
  file(STRINGS "${SDL3_INCLUDE_DIR}/SDL_version.h" SDL3_VERSION_MAJOR_LINE REGEX "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL3_INCLUDE_DIR}/SDL_version.h" SDL3_VERSION_MINOR_LINE REGEX "^#define[ \t]+SDL_MINOR_VERSION[ \t]+[0-9]+$")
  file(STRINGS "${SDL3_INCLUDE_DIR}/SDL_version.h" SDL3_VERSION_PATCH_LINE REGEX "^#define[ \t]+SDL_PATCHLEVEL[ \t]+[0-9]+$")
  string(REGEX REPLACE "^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL3_VERSION_MAJOR "${SDL3_VERSION_MAJOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$" "\\1" SDL3_VERSION_MINOR "${SDL3_VERSION_MINOR_LINE}")
  string(REGEX REPLACE "^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$" "\\1" SDL3_VERSION_PATCH "${SDL3_VERSION_PATCH_LINE}")
  set(SDL3_VERSION_STRING ${SDL3_VERSION_MAJOR}.${SDL3_VERSION_MINOR}.${SDL3_VERSION_PATCH})
  unset(SDL3_VERSION_MAJOR_LINE)
  unset(SDL3_VERSION_MINOR_LINE)
  unset(SDL3_VERSION_PATCH_LINE)
  unset(SDL3_VERSION_MAJOR)
  unset(SDL3_VERSION_MINOR)
  unset(SDL3_VERSION_PATCH)
endif()

include(FindPackageHandleStandardArgs)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL3
                                  REQUIRED_VARS SDL3_LIBRARY SDL3_INCLUDE_DIR
                                  VERSION_VAR SDL3_VERSION_STRING)

if(SDL3MAIN_LIBRARY)
  set(FPHSA_NAME_MISMATCHED 1)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDL3main
                                    REQUIRED_VARS SDL3MAIN_LIBRARY SDL3_INCLUDE_DIR
                                    VERSION_VAR SDL3_VERSION_STRING)
endif()


mark_as_advanced(SDL3_PATH
                 SDL3_NO_DEFAULT_PATH
                 SDL3_LIBRARY
                 SDL3MAIN_LIBRARY
                 SDL3_INCLUDE_DIR
                 SDL3_BUILDING_LIBRARY)


# SDL3:: targets (SDL3::Core and SDL3::Main)
if(SDL3_FOUND)

  # SDL3::Core target
  if(SDL3_LIBRARY AND NOT TARGET SDL3::Core)
    add_library(SDL3::Core UNKNOWN IMPORTED)
    set_target_properties(SDL3::Core PROPERTIES
                          IMPORTED_LOCATION "${SDL3_LIBRARY}"
                          INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIR}")

    if(APPLE)
      # For OS X, SDL3 uses Cocoa as a backend so it must link to Cocoa.
      # For more details, please see above.
      set_property(TARGET SDL3::Core APPEND PROPERTY
                   INTERFACE_LINK_OPTIONS -framework Cocoa)
    else()
      # For threads, as mentioned Apple doesn't need this.
      # For more details, please see above.
      set_property(TARGET SDL3::Core APPEND PROPERTY
                   INTERFACE_LINK_LIBRARIES Threads::Threads)
    endif()
  endif()

  # SDL3::Main target
  # Applications should link to SDL3::Main instead of SDL3::Core
  # For more details, please see above.
  if(NOT SDL3_BUILDING_LIBRARY AND NOT TARGET SDL3::Main)

    if(SDL3_INCLUDE_DIR MATCHES ".framework" OR NOT SDL3MAIN_LIBRARY)
      add_library(SDL3::Main INTERFACE IMPORTED)
      set_property(TARGET SDL3::Main PROPERTY
                   INTERFACE_LINK_LIBRARIES SDL3::Core)
    elseif(SDL3MAIN_LIBRARY)
      # MinGW requires that the mingw32 library is specified before the
      # libSDL3main.a static library when linking.
      # The SDL3::MainInternal target is used internally to make sure that
      # CMake respects this condition.
      add_library(SDL3::MainInternal UNKNOWN IMPORTED)
      set_property(TARGET SDL3::MainInternal PROPERTY
                   IMPORTED_LOCATION "${SDL3MAIN_LIBRARY}")
      set_property(TARGET SDL3::MainInternal PROPERTY
                   INTERFACE_LINK_LIBRARIES SDL3::Core)

      add_library(SDL3::Main INTERFACE IMPORTED)

      if(MINGW)
        # MinGW needs an additional link flag '-mwindows' and link to mingw32
        set_property(TARGET SDL3::Main PROPERTY
                     INTERFACE_LINK_LIBRARIES "mingw32" "-mwindows")
      endif()

      set_property(TARGET SDL3::Main APPEND PROPERTY
                   INTERFACE_LINK_LIBRARIES SDL3::MainInternal)
    endif()

  endif()
endif()
