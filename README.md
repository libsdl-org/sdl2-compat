# Simple DirectMedia Layer (SDL) sdl2-compat

https://www.libsdl.org/

This is the Simple DirectMedia Layer, a general API that provides low
level access to audio, keyboard, mouse, joystick, 3D hardware via OpenGL,
and 2D framebuffer across multiple platforms.

This code is a compatibility layer; it provides a binary and source
compatible API for programs written against SDL2, but it uses SDL3
behind the scenes. If you are writing new code, please target SDL3
directly and do not use this layer.

If you absolutely must have the real SDL2 ("SDL 2 Classic"), please use
the `SDL2` branch at https://github.com/libsdl-org/SDL, which occasionally
gets bug fixes (and eventually, no new formal releases). But we strongly
encourage you not to do that.

# How to use:

- Build the library. This will need access to SDL3's headers,
[CMake](https://cmake.org/) and the build tools of your choice. Once built, you
will have a drop-in replacement that can be used with any existing binary
that relies on SDL2. You can copy this library over the existing SDL2 build,
or force it to take priority over a system copy with LD_LIBRARY_PATH, etc.
At runtime, sdl2-compat needs to be able to find a copy of SDL3, so plan to
include it with the library if necessary.

- If you want to build an SDL2 program from source code, we have included
compatibility headers, so that sdl2-compat can completely replace SDL2
at all points. Note that sdl2-compat itself does not use these headers,
so if you just want the library, you don't need them.

# Building the library:

These are quick-start instructions; there isn't anything out of the ordinary
here if you're used to using CMake. 

You'll need to use CMake to build sdl2-compat. Download at
[cmake.org](https://cmake.org/) or install from your package manager
(`sudo apt-get install cmake` on Ubuntu, etc).

Please refer to the [CMake documentation](https://cmake.org/documentation/)
for complete details, as platform and build tool details vary.

You'll need a copy of SDL3 to build sdl2-compat, because we need the
SDL3 headers. You can build this from source or install from a package
manager. Windows and Mac users can download prebuilt binaries from
[SDL's download page](https://libsdl.org/download-3.0.php); make sure you
get the "development libraries" and not "runtime binaries" there.

Linux users might need some packages from their Linux distribution. On Ubuntu,
you might need to do (!!! FIXME: this won't work until SDL3 is further in
development!) ...

```bash
sudo apt-get install build-essential cmake libsdl3-dev libgl-dev
```

Now just point CMake at sdl2-compat's directory. Here's a command-line
example:

```bash
cd sdl2-compat
cmake -Bbuild -DCMAKE_BUILD_TYPE=Release .
cmake --build build
```

On Windows or macOS, you might prefer to use CMake's GUI, but it's the same
idea: give it the directory where sdl2-compat is located, click "Configure,"
choose your favorite compiler, then click "Generate." Now you have project
files! Click "Open Project" to launch your development environment. Then you
can build however you like with Visual Studio, Xcode, etc.

If necessary, you might have to fill in the location of the SDL3 headers
when using CMake. sdl2-compat does not need SDL3's library to _build_,
just its headers (although it may complain about the missing library,
you can ignore that). From the command line, add
`-DSDL3_INCLUDE_DIRS=/path/to/SDL3/include`, or find this in the CMake
GUI and set it appropriately, click "Configure" again, and then "Generate."

When the build is complete, you'll have a shared library you can drop in
as a replacement for an existing SDL2 build. This will also build
the original SDL2 test apps, so you can verify the library is working.


# Building for older CPU architectures on Linux:

There are a lot of binaries from many years ago that used SDL2, which is
to say they are for CPU architectures that are likely not your current
system's.

If you want to build a 32-bit x86 library on an x86-64 Linux machine, for
compatibility with older games, you should install some basic 32-bit
development libraries for your distribution. On Ubuntu, this would be:

(!!! FIXME: this won't work until SDL3 is further in
development!) 

```bash
sudo apt-get install gcc-multilib libsdl3-dev:i386
```

...and then add `-m32` to your build options:


```bash
cd sdl2-compat
cmake -Bbuild32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS=-m32
cmake --build build32
```


# Building for older CPU architectures on macOS:

macOS users can try adding `-DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'` instead
of `-DCMAKE_C_FLAGS=-m32` to make a Universal Binary for both 64-bit Intel and
Apple Silicon machines.


# Building for older CPU architectures on Windows:

Windows users just select a 32-bit version of Visual Studio when running
CMake, when it asks you what compiler to target in the CMake GUI.


# Configuration options:

sdl2-compat has a number of configuration options which can be used to work
around issues with individual applications, or to better fit your system or
preferences.

These options are all specified as environment variables, and can be set by
running your application with them set on the command-line, for example:
```
SDL2COMPAT_DEBUG_LOGGING=1 SDL2COMPAT_ALLOW_SYSWM=0 %command%
```
will run `%command%` with high-dpi monitor support enabled, but OpenGL
scaling support disabled.

(While these environment variables are checked at various times throughout
the lifetime of the app, sdl2-compat expects these to be set before the
process starts and not change during the life of the process, and any
places where changing it later might affect operation is purely accidental
and might change. That is to say: don't write an SDL2-based app with
plans to tweak these values on the fly!)

The available options are:

- SDL2COMPAT_DEBUG_LOGGING: (checked at startup)
  If enabled, print debugging messages to stderr.  These messages are
  mostly useful to developers, or when trying to track down a specific
  bug.

- SDL2COMPAT_ALLOW_SYSWM: (checked during SDL_Init)
  Enabled by default.
  If disabled, SDL_SYSWMEVENT events will not be delivered to the app, and
  SDL_GetWMInfo() will fail; this is useful if you have a program that
  tries to access X11 directly through SDL's interfaces, but can survive
  without it, becoming compatible with, for example, Wayland, or perhaps
  just avoiding a bug in target-specific code.


# Compatibility issues with applications directly accessing underlying APIs

Some applications combine the use of SDL with direct access to the underlying
OS or window system. When running these applications on the same OS and SDL
video driver (e.g. a program written for X11 on Linux is run on X11 on Linux),
sdl2-compat is usually compatible.

However, if you wish to run an application on a different video driver, the
application will be unable to access the underlying API it is expecting, and
may fail. This often occurs trying to run applications written for X11 under
Wayland, and particularly affects a number of popular OpenGL extension loaders.

In this case, the best workaround is to run under a compatibility layer like
XWayland, and set the SDL_VIDEODRIVER environment variable to the driver the
program is expecting:
```
SDL_VIDEODRIVER=x11
```
