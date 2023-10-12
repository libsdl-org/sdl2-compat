#!/bin/sh

if test "x$CC" = "x"; then
    CC=cc
fi

machine="$($CC -dumpmachine)"
case "$machine" in
    *mingw* )
        EXEPREFIX=""
        EXESUFFIX=".exe"
        ;;
    *android* )
        EXEPREFIX="lib"
        EXESUFFIX=".so"
        LDFLAGS="$LDFLAGS -shared"
        ;;
    * )
        EXEPREFIX=""
        EXESUFFIX=""
        ;;
esac

set -e

test_static=yes
if [ "x$1" = "x--no-static" ]; then
    test_static=no
fi

# Get the canonical path of the folder containing this script
testdir=$(cd -P -- "$(dirname -- "$0")" && printf '%s\n' "$(pwd -P)")
SDL_CFLAGS="$( pkg-config sdl2_compat --cflags )"
SDL_LDFLAGS="$( pkg-config sdl2_compat --libs )"
if [ "x$test_static" = "xyes" ]; then
    SDL_STATIC_LDFLAGS="$( pkg-config sdl2_compat --libs --static )"
fi

compile_cmd="$CC -c "$testdir/main_gui.c" -o main_gui_pkgconfig.c.o $SDL_CFLAGS $CFLAGS"
link_cmd="$CC main_gui_pkgconfig.c.o -o ${EXEPREFIX}main_gui_pkgconfig${EXESUFFIX} $SDL_LDFLAGS $LDFLAGS"
if [ "x$test_static" = "xyes" ]; then
    static_link_cmd="$CC main_gui_pkgconfig.c.o -o ${EXEPREFIX}main_gui_pkgconfig_static${EXESUFFIX} $SDL_STATIC_LDFLAGS $LDFLAGS"
fi

echo "-- CC:                 $CC"
echo "-- CFLAGS:             $CFLAGS"
echo "-- LDFLASG:            $LDFLAGS"
echo "-- SDL_CFLAGS:         $SDL_CFLAGS"
echo "-- SDL_LDFLAGS:        $SDL_LDFLAGS"
if [ "x$test_static" = "xyes" ]; then
    echo "-- SDL_STATIC_LDFLAGS: $SDL_STATIC_LDFLAGS"
fi

echo "-- COMPILE:       $compile_cmd"
echo "-- LINK:          $link_cmd"
if [ "x$test_static" = "xyes" ]; then
    echo "-- STATIC_LINK:   $static_link_cmd"
fi

set -x

$compile_cmd
$link_cmd
if [ "x$test_static" = "xyes" ]; then
    $static_link_cmd
fi
