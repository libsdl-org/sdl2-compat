#!/bin/sh

#set -eux

cd `dirname $0`/..

ARGSOKAY=1
if [ -z $1 ]; then
    ARGSOKAY=0
fi
if [ -z $2 ]; then
    ARGSOKAY=0
fi
if [ -z $3 ]; then
    ARGSOKAY=0
fi

if [ "x$ARGSOKAY" = "x0" ]; then
    echo "USAGE: $0 <major> <minor> <patch>" 1>&2
    exit 1
fi

MAJOR="$1"
MINOR="$2"
PATCH="$3"
NEWVERSION="${MAJOR}.${MINOR}.${PATCH}"

echo "Updating version to '$NEWVERSION' ..."

perl -w -pi -e 's/(\#define SDL_MAJOR_VERSION\s+)\d+/${1}'${MAJOR}'/;' include/SDL2/SDL_version.h
perl -w -pi -e 's/(\#define SDL_MINOR_VERSION\s+)\d+/${1}'${MINOR}'/;' include/SDL2/SDL_version.h
perl -w -pi -e 's/(\#define SDL_PATCHLEVEL\s+)\d+/${1}'${PATCH}'/;' include/SDL2/SDL_version.h

perl -w -pi -e 's/\A(project\(sdl[0-9]+_compat VERSION )[0-9.]+/${1}'$NEWVERSION'/;' CMakeLists.txt

perl -w -pi -e 's/(FILEVERSION\s+)\d+,\d+,\d+/${1}'${MAJOR}','${MINOR}','${PATCH}'/;' src/version.rc
perl -w -pi -e 's/(PRODUCTVERSION\s+)\d+,\d+,\d+/${1}'${MAJOR}','${MINOR}','${PATCH}'/;' src/version.rc
perl -w -pi -e 's/(VALUE "FileVersion", ")\d+, \d+, \d+/${1}'${MAJOR}', '${MINOR}', '${PATCH}'/;' src/version.rc
perl -w -pi -e 's/(VALUE "ProductVersion", ")\d+, \d+, \d+/${1}'${MAJOR}', '${MINOR}', '${PATCH}'/;' src/version.rc

perl -w -pi -e 's/(\#define SDL2_COMPAT_VERSION_MINOR\s+)\d+/${1}'${MINOR}'/;' src/sdl2_compat.c
perl -w -pi -e 's/(\#define SDL2_COMPAT_VERSION_PATCH\s+)\d+/${1}'${PATCH}'/;' src/sdl2_compat.c

if [[ "x$((${MINOR}%2))" = "x0" ]]; then
    SOVER="$((100 * ${MINOR})).${PATCH}"
    DYVER="$((100 * ${MINOR} + 1)).${PATCH}"
else
    SOVER="$((100 * ${MINOR} + ${PATCH})).0"
    DYVER="$((100 * ${MINOR} + ${PATCH} + 1)).0"
fi

perl -w -pi -e 's/(SHLIB\s+=\s+libSDL2-2\.0\.so\.0\.)\d+\.\d+/${1}'${SOVER}'/;' src/Makefile.linux
perl -w -pi -e 's/(-Wl,-compatibility_version,)\d+\.\d+/${1}'${DYVER}'/;' src/Makefile.darwin
perl -w -pi -e 's/(-Wl,-current_version,)\d+.\d+/${1}'${DYVER}'/;' src/Makefile.darwin

echo "Running build-scripts/test-versioning.sh to verify changes..."
./build-scripts/test-versioning.sh

echo "All done."
echo "Run 'git diff' and make sure this looks correct, before 'git commit'."

exit 0
