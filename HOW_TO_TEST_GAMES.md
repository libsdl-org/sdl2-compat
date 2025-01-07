# Testing games with sdl2-compat

sdl2-compat is here to make sure older games not only work well, but work
_correctly_, and for that, we need an army of testers.

Here's how to test games with sdl2-compat.


## The general idea

- Build [SDL3](https://github.com/libsdl-org/SDL) or find a prebuilt binary.
- Build sdl2-compat (documentation is in README.md)
- Find a game to test.
- Make sure a game uses sdl2-compat instead of classic SDL2.
- See if game works or blows up, report back.


## Find a game to test

Unlike SDL 1.2, where we didn't build the compatibility layer until almost
a decade in and most major projects had already migrated, there are _tons_
of SDL2 games at the time of this writing; almost any Linux game on Steam
in 2025 is using SDL2, not to mention almost any game that is in a Linux
distribution's package manager, etc.

As such, while we kept a spreadsheet for known SDL 1.2 titles, we don't
have an authoritative source of them for SDL2, but they're everywhere
you look!


## Make sure the game works with real SDL2 first!

You'd be surprised how quickly games can bitrot! If it doesn't work with
real SDL2 anymore, it's not a bug if sdl2-compat doesn't work either.


## Force it to use sdl2-compat instead.

Either overwrite the copy of SDL2 that the game uses with sdl2-compat,
or (on Linux) export LD_LIBRARY_PATH to point to your copy, so the system will
favor it when loading libraries.

SDL2 has something called the "Dynamic API" which is some magic inside of
SDL2 itself: you can override the build of SDL2 in use, even if SDL2 is
statically linked to the app!

Just export the environment variable SDL_DYNAMIC_API with the path of your
sdl2-compat library, and it will use sdl2-compat! No LD_LIBRARY_PATH
needed!


## Watch out for setuid/setgid binaries!

On Linux, if you're testing a binary that's setgid to a "games" group (which
we ran into several times with Debian packages), or setuid root or whatever,
then the system will ignore the LD_LIBRARY_PATH variable, as a security
measure. SDL_DYNAMIC_API is not affected by this.

The reason some games are packaged like this is usually because they want to
write to a high score list in a global, shared directory. Often times the
games will just carry on if they fail to do so.

There are several ways to bypass this:

- Use SDL_DYNAMIC_API instead.
- On some distros, you can run `ld.so` directly:
  ```bash
  LD_LIBRARY_PATH=/where/i/can/find/sdl2-compat ld.so /usr/games/mygame
  ```
- You can remove the setgid bit:
  ```bash
  # (it's `u-s` for the setuid bit)
  sudo chmod g-s /usr/games/mygame
  ```
- You can install sdl2-compat system-wide, so the game uses that
  instead of SDL2 by default.
- If you don't have root access at all, you can try to copy the game 
  somewhere else or install a personal copy, or build from source code,
  but these are drastic measures.
  
Definitely read the next section ("Am I actually running sdl2-compat?") in
these scenarios to make sure you ended up with the right library!
  
## Am I actually running sdl2-compat?

The easiest way to know is to set some environment variables:

```bash
export SDL2COMPAT_DEBUG_LOGGING=1
```

If this is set, when loading sdl2-compat, it'll write something like this
to stderr (on Linux and Mac, at least)...

```
INFO: sdl2-compat, built on Sep  2 2022 at 11:27:37, talking to SDL3 3.2.0
```


## Steam

If testing a Steam game, you'll want to launch the game outside of the Steam
Client, so that Steam doesn't overwrite files you replaced and so you can
easily control environment variables.

Since you'll be using the Steam Runtime, you don't have to find your own copy
of SDL3, as Steam provides it (!!! FIXME: note to the future: SDL3 is still
in development at the time of this writing, so Steam doesn't provide it _yet_).

On Linux, Steam stores games in ~/.local/share/Steam/steamapps/common, each
in its own usually-well-named subdirectory.

You'll want to add a file named "steam_appid.txt" to the same directory as
the binary, which will encourage Steamworks to _not_ terminate the process
and have the Steam Client relaunch it. This file should just have the appid
for the game in question, which you can find from the store page.

For example, the store page for Braid is:

https://store.steampowered.com/app/26800/Braid/

See that `26800`? That's the appid.

```bash
echo 26800 > steam_appid.txt
```

For Linux, you can make sure that, from the command line, the game still
runs with the Steam Runtime and has the Steam Overlay by launching it with a
little script:

- [steamapp32](https://raw.githubusercontent.com/icculus/twisty-little-utilities/main/steamapp32) for x86 binaries.
- [steamapp64](https://raw.githubusercontent.com/icculus/twisty-little-utilities/main/steamapp64) for x86-64 binaries.

(And make sure you have a 32-bit or 64-bit build of sdl2-compat!)

And then make sure you force it to use _your_ sdl2-compat instead of the
system/Steam Runtime build:

```bash
export LD_LIBRARY_PATH=/where/i/installed/sdl2-compat
```

Putting this all together, you might run [Portal](https://store.steampowered.com/app/400/)
like this:

```bash
cd ~/.local/share/Steam/steamapps/common/Portal
export LD_LIBRARY_PATH=/where/i/installed/sdl2-compat
export SDL2COMPAT_DEBUG_LOGGING=1
echo 400 > steam_appid.txt
steamapp64 ./portal
```


## Windows

Generally, Windows games just ship with an SDL2.dll, and you just need to
overwrite it with an sdl2-compat build, then run as usual.


## macOS, etc.

(write me.)

Most of the Linux advice applies, but you might have to replace the SDL2
in a framework.


## Questions?

If something isn't clear, make a note [here](https://github.com/libsdl-org/sdl2-compat/issues/new)
and we'll update this document.

Thanks!

