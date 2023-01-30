# NOTE:  If you installed SDL3 at an unusual place and SDL3Config fails,
#        you can define SDL3_INCLUDE_DIRS on the cmdline.  For example:
#        cmake -DSDL3_INCLUDE_DIRS=/opt/SDL3/include [other stuff ....]

cmake_minimum_required(VERSION 3.0.0)
project(sdl2_compat VERSION 2.90.0 LANGUAGES C)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(SDL2COMPAT_SUBPROJECT OFF)
else()
  set(SDL2COMPAT_SUBPROJECT ON)
endif()

if(CMAKE_VERSION VERSION_LESS 3.16.0 OR SDL2COMPAT_SUBPROJECT)
  # - CMake versions <3.16 do not support the OBJC language
  # - When SDL is built as a subproject and when the main project does not enable OBJC,
  #   CMake fails due to missing internal CMake variables (CMAKE_OBJC_COMPILE_OBJECT)
  #   (reproduced with CMake 3.24.2)
else()
  if(APPLE)
    enable_language(OBJC)
  endif()
endif()

set(SDL_MAJOR_VERSION ${sdl2_compat_VERSION_MAJOR})
set(SDL_MINOR_VERSION ${sdl2_compat_VERSION_MINOR})
set(SDL_MICRO_VERSION ${sdl2_compat_VERSION_PATCH})
set(SDL_VERSION "${SDL_MAJOR_VERSION}.${SDL_MINOR_VERSION}.${SDL_MICRO_VERSION}")

# Calculate libtool-compatible SO version
set(SDL_SO_VERSION_MAJOR "0")
if(SDL_MINOR_VERSION MATCHES "[02468]$")
  # Stable branch, 2.24.1 -> libSDL2-2.0.so.0.2400.1
  math(EXPR SDL_SO_VERSION_MINOR "${SDL_MINOR_VERSION} * 100")
  set(SDL_SO_VERSION_MICRO ${SDL_MICRO_VERSION})
else()
  # Development branch, 2.23.1 -> libSDL2-2.0.so.0.2301.0
  math(EXPR SDL_SO_VERSION_MINOR "${SDL_MINOR_VERSION} * 100 + ${SDL_MICRO_VERSION}")
  set(SDL_SO_VERSION_MICRO 0)
endif()
set(SDL_SO_VERSION "${SDL_SO_VERSION_MAJOR}.${SDL_SO_VERSION_MINOR}.${SDL_SO_VERSION_MICRO}")

if(SDL_MINOR_VERSION MATCHES "[02468]$")
  math(EXPR SDL_DYLIB_COMPAT_VERSION_MAJOR "100 * ${SDL_MINOR_VERSION} + 1")
  set(SDL_DYLIB_COMPAT_VERSION_MINOR "0")
  math(EXPR SDL_DYLIB_CURRENT_VERSION_MAJOR "${SDL_DYLIB_COMPAT_VERSION_MAJOR}")
  set(SDL_DYLIB_CURRENT_VERSION_MINOR "${SDL_MICRO_VERSION}")
else()
  math(EXPR SDL_DYLIB_COMPAT_VERSION_MAJOR "100 * ${SDL_MINOR_VERSION} + ${SDL_MICRO_VERSION} + 1")
  set(SDL_DYLIB_COMPAT_VERSION_MINOR "0")
  math(EXPR SDL_DYLIB_CURRENT_VERSION_MAJOR "${SDL_DYLIB_COMPAT_VERSION_MAJOR}")
  set(SDL_DYLIB_CURRENT_VERSION_MINOR "0")
endif()
set(SDL_DYLIB_COMPAT_VERSION_MICRO "0")
set(SDL_DYLIB_CURRENT_VERSION_MICRO "0")

set(SDL_DYLIB_CURRENT_VERSION "${SDL_DYLIB_CURRENT_VERSION_MAJOR}.${SDL_DYLIB_CURRENT_VERSION_MINOR}.${SDL_DYLIB_CURRENT_VERSION_MICRO}")
set(SDL_DYLIB_COMPAT_VERSION "${SDL_DYLIB_COMPAT_VERSION_MAJOR}.${SDL_DYLIB_COMPAT_VERSION_MINOR}.${SDL_DYLIB_COMPAT_VERSION_MICRO}")

option(SDL2COMPAT_TESTS "Enable to build SDL2 test programs" ON)
option(SDL2COMPAT_DEVEL "Enable installing SDL2 development headers" ON)
option(SDL2COMPAT_STATICDEVEL "Enable installing static link library" OFF)
option(SDL2COMPAT_WERROR "Treat warnings as errors" ON)
set(SDL2COMPAT_VENDOR_INFO "" CACHE STRING "Vendor name and/or version to add to SDL_REVISION")
#FIXME: should SDL2-compat modify vendor_info/revision information?

if(SDL2COMPAT_STATICDEVEL AND NOT (CMAKE_SYSTEM_NAME MATCHES "Linux"))
  message(FATAL_ERROR "Static builds are only supported on Linux.")
endif()

include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckCCompilerFlag)
include(CMakePackageConfigHelpers)
include(CMakePushCheckState)
include(GNUInstallDirs)
include("${CMAKE_CURRENT_LIST_DIR}/cmake/CheckCPUArchitecture.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sdlchecks.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/cmake/sdlplatform.cmake")

SDL_DetectCMakePlatform()
CheckOpenGLES()

check_cpu_architecture(x86 SDL_CPU_X86)

if(NOT MSVC AND NOT APPLE AND NOT OPENBSD)
  cmake_push_check_state()
  set(CMAKE_REQUIRED_FLAGS "-Wl,--no-undefined")
  check_c_compiler_flag("" HAVE_NO_UNDEFINED)
  cmake_pop_check_state()
  if(HAVE_NO_UNDEFINED)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--no-undefined")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined")
  endif()
endif()

set(CMAKE_SKIP_RPATH TRUE)

if(NOT TARGET SDL3::Headers)
  find_package(SDL3 QUIET COMPONENTS Headers)
endif()

if(NOT TARGET SDL3::Headers)
  find_path(SDL3_INCLUDE_DIRS
    NAMES "SDL3/SDL.h"
  )
  if(NOT SDL3_INCLUDE_DIRS)
    message(FATAL_ERROR "Cannot find SDL3 headers: configure with -DCMAKE_PREFIX_PATH=\"/root/to/SDL3\" or -DSDL3_INCLUDE_DIRS=\"/root/to/SDL3/include\"")
  endif()
  add_library(SDL3::Headers INTERFACE IMPORTED)
  set_property(TARGET SDL3::Headers PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${SDL3_INCLUDE_DIRS}")
endif()

find_package(Git)
if(Git_FOUND)
  execute_process(COMMAND
      "${GIT_EXECUTABLE}" describe --always --tags --long
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      RESULT_VARIABLE GIT_REVISION_STATUS
      OUTPUT_VARIABLE GIT_REVISION
      ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  set(GIT_REVISION_STATUS 1)
  set(GIT_REVISION "")
endif()

if(GIT_REVISION_STATUS EQUAL 0)
  if(GIT_REVISION MATCHES "^[0-9a-f]+$")
    # Just a truncated sha1, so prefix it with the version number
    set(SDL_REVISION "SDL-${SDL_VERSION}-g${GIT_REVISION}")
  else()
    # e.g. release-2.24.0-542-g96361fc47
    set(SDL_REVISION "SDL-${GIT_REVISION}")
  endif()
else()
  set(SDL_REVISION "SDL-${SDL_VERSION}-no-vcs")
endif()

configure_file(include/SDL2/SDL_revision.h.cmake include/SDL2/SDL_revision.h @ONLY)

set(SDL2COMPAT_SRCS
  src/sdl2_compat.c
  src/dynapi/SDL_dynapi.c
)

if(APPLE)
  list(APPEND SDL2COMPAT_SRCS
    "src/sdl2_compat_objc.m"
  )

  find_package(X11)
  if(X11_FOUND)
    include_directories("${X11_INCLUDE_DIR}")
  endif()
  if(POLICY CMP0068)
    cmake_policy(SET CMP0068 NEW)  # on macOS, don't let RPATH affect install_name.
  endif()
endif()

if(WIN32)
  list(APPEND SDL2COMPAT_SRCS
    "src/version.rc"
  )
endif()

if(APPLE)
  foreach(SOURCE_FILE ${SDL2COMPAT_SRCS})
    get_filename_component(FILE_EXTENSION ${SOURCE_FILE} EXT)
    if(FILE_EXTENSION STREQUAL "m")
      set_property(SOURCE ${SOURCE_FILE} APPEND_STRING PROPERTY COMPILE_FLAGS " -x objective-c")
    endif()
  endforeach()
endif()

add_library(SDL2 SHARED ${SDL2COMPAT_SRCS})
add_library(SDL2::SDL2 ALIAS SDL2)
target_link_libraries(SDL2 PRIVATE $<BUILD_INTERFACE:SDL3::Headers>)
target_include_directories(SDL2
  INTERFACE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/SDL2>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/SDL2>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
)

set(EXTRA_CFLAGS )
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wall")
  check_c_compiler_flag(-fvisibility=hidden HAVE_FVISIBILITY_HIDDEN)
  if(HAVE_FVISIBILITY_HIDDEN AND (APPLE OR UNIX))
    set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden")
  endif()
  check_c_compiler_flag(-Wdeclaration-after-statement HAVE_WDECLARATION_AFTER_STATEMENT)
  if(HAVE_WDECLARATION_AFTER_STATEMENT)
    set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wdeclaration-after-statement")
  endif()
  if(SDL2COMPAT_WERROR)
    check_c_compiler_flag(-Werror=declaration-after-statement HAVE_WERROR_DECLARATION_AFTER_STATEMENT)
    if(HAVE_WERROR_DECLARATION_AFTER_STATEMENT)
      set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Werror=declaration-after-statement")
    endif()
  endif()
endif()

# This isn't needed for compat libSDL2 who use SDL3 headers.
# SDL2_test and SDL2_main (and the tes programs) still need
# this, because they use SDL2 headers.
check_include_file("immintrin.h" HAVE_IMMINTRIN_H)
if(NOT HAVE_IMMINTRIN_H)
  set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -DSDL_DISABLE_IMMINTRIN_H")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)

if(UNIX AND NOT APPLE)
  target_compile_definitions(SDL2 PRIVATE "_REENTRANT")
  target_link_libraries(SDL2 PRIVATE ${CMAKE_DL_LIBS})
endif()
if(APPLE)
  # !!! FIXME: check OUTPUT_NAME value.
  set_target_properties(SDL2 PROPERTIES COMPILE_DEFINITIONS "_THREAD_SAFE")
  target_link_libraries(SDL2 PRIVATE
    "-Wl,-compatibility_version,${SDL_DYLIB_COMPAT_VERSION}"
    "-Wl,-current_version,${SDL_DYLIB_CURRENT_VERSION}"
    "-Wl,-framework,AppKit"
  )
  set_target_properties(SDL2 PROPERTIES
      OUTPUT_NAME "SDL2-2.0"
      SOVERSION "0"
  )
elseif(UNIX AND NOT ANDROID)
  # !!! FIXME: check OUTPUT_NAME value
  set_target_properties(SDL2 PROPERTIES
    OUTPUT_NAME "SDL2-2.0"
    VERSION "${SDL_SO_VERSION}"
    SOVERSION "${SDL_SO_VERSION_MAJOR}"
  )
elseif(WIN32)
  set_target_properties(SDL2 PROPERTIES DEFINE_SYMBOL "DLL_EXPORT")
  # avoid DLL having 'lib' prefix with MinGW
  set_target_properties(SDL2 PROPERTIES
    OUTPUT_NAME "SDL2"
    PREFIX ""
  )
else()
  set_target_properties(SDL2 PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "0"
    OUTPUT_NAME "SDL2"
  )
endif()

if(MINGW)
  target_link_libraries(SDL2 PRIVATE -nostdlib)
  target_link_libraries(SDL2 PRIVATE -static-libgcc -lgcc) # libgcc is needed for 32 bit (x86) builds
endif()
if(MSVC)
  # Don't try to link with the default set of libraries.
  target_compile_options(SDL2 PRIVATE /GS-)
  if(SDL_CPU_X86)  # don't emit SSE2 in x86 builds
    target_compile_options(SDL2 PRIVATE /arch:SSE)
  endif()
  set_target_properties(SDL2 PROPERTIES LINK_FLAGS "/NODEFAULTLIB")
  # Make sure /RTC1 is disabled: (from SDL2 CMake)
  foreach(flag_var
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    string(REGEX REPLACE "/RTC(su|[1su])" "" ${flag_var} "${${flag_var}}")
  endforeach(flag_var)
endif()

# !!! FIXME: what needs to be done for sdl2-compat? Look into this.
## SDL2main library...
#if(APPLE)
#    add_library(SDL2main STATIC src/SDLmain/macosx/SDLMain.m)
#    set_source_files_properties(src/SDLmain/macosx/SDLMain.m PROPERTIES LANGUAGE C)
#elseif(WIN32)
#    add_library(SDL2main STATIC src/SDLmain/win32/SDL_win32_main.c)
#    set_target_properties(SDL2main PROPERTIES COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE")  # !!! FIXME: don't use C runtime? We fixed this in SDL2.
#else()
    add_library(SDL2main STATIC src/SDLmain/dummy/SDL_dummy_main.c)
#endif()
add_library(SDL2::SDL2main ALIAS SDL2main)
target_include_directories(SDL2main PRIVATE "include/SDL2")
if(MINGW OR CYGWIN)
  if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    target_link_libraries(SDL2main PUBLIC "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:-Wl,--undefined=_WinMain@16>")
  else()
    target_link_libraries(SDL2main PUBLIC "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:-Wl,--undefined=WinMain>")
  endif()
endif()

# SDLtest library...
add_library(SDL2_test STATIC
  src/test/SDL_test_assert.c
  src/test/SDL_test_common.c
  src/test/SDL_test_compare.c
  src/test/SDL_test_crc32.c
  src/test/SDL_test_font.c
  src/test/SDL_test_fuzzer.c
  src/test/SDL_test_harness.c
  src/test/SDL_test_imageBlit.c
  src/test/SDL_test_imageBlitBlend.c
  src/test/SDL_test_imageFace.c
  src/test/SDL_test_imagePrimitives.c
  src/test/SDL_test_imagePrimitivesBlend.c
  src/test/SDL_test_log.c
  src/test/SDL_test_md5.c
  src/test/SDL_test_memory.c
  src/test/SDL_test_random.c
)
add_library(SDL2::SDL2test ALIAS SDL2_test)
target_include_directories(SDL2_test
  PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/SDL2>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/SDL2>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
)

if(SDL2COMPAT_TESTS)
  if(NOT (WIN32 OR APPLE OR CYGWIN OR HAIKU OR BEOS))
    find_library(MATH_LIBRARY m)
  endif()

  set(OpenGL_GL_PREFERENCE GLVND)
  find_package(OpenGL COMPONENTS OpenGL)
  if(OPENGL_FOUND)
    set(HAVE_OPENGL_DEFINE "HAVE_OPENGL")
    if(WIN32)
      set(OPENGL_gl_LIBRARY "opengl32")
      set(OPENGL_opengl_LIBRARY "opengl32")
    elseif(APPLE)
      set(OPENGL_gl_LIBRARY "-Wl,-framework,OpenGL")
      set(OPENGL_opengl_LIBRARY "-Wl,-framework,OpenGL")
    endif()
  endif()

  macro(test_program _NAME _SRCS)
    add_executable(${_NAME} ${_SRCS})
    target_include_directories(${_NAME} PRIVATE "include/SDL2")
    target_link_libraries(${_NAME} PRIVATE SDL2::SDL2main SDL2::SDL2test SDL2::SDL2)
    # Turn off MSVC's aggressive C runtime warnings for the old test programs.
    if(MSVC)
      target_compile_definitions(${_NAME} PRIVATE ${HAVE_OPENGL_DEFINE} _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
    elseif(APPLE)
      target_compile_definitions(${_NAME} PRIVATE ${HAVE_OPENGL_DEFINE} GL_SILENCE_DEPRECATION=1)
      target_link_libraries(${_NAME} PRIVATE "-Wl,-framework,Cocoa")
    else()
      target_compile_definitions(${_NAME} PRIVATE ${HAVE_OPENGL_DEFINE})
    endif()
    if(MATH_LIBRARY)
      target_link_libraries(${_NAME} PRIVATE ${MATH_LIBRARY})
    endif()
  endmacro()

  test_program(checkkeys "test/checkkeys.c")
  test_program(checkkeysthreads "test/checkkeysthreads.c")
  test_program(controllermap "test/controllermap.c;test/testutils.c")
  test_program(loopwave "test/loopwave.c;test/testutils.c")
  test_program(loopwavequeue "test/loopwavequeue.c;test/testutils.c")
  test_program(testatomic "test/testatomic.c")
  test_program(testaudiocapture "test/testaudiocapture.c")
  test_program(testaudiohotplug "test/testaudiohotplug.c;test/testutils.c")
  test_program(testaudioinfo "test/testaudioinfo.c")
  test_program(testbounds "test/testbounds.c")
  test_program(testcustomcursor "test/testcustomcursor.c")
  test_program(testdisplayinfo "test/testdisplayinfo.c")
  test_program(testdraw2 "test/testdraw2.c")
  test_program(testdrawchessboard "test/testdrawchessboard.c")
  test_program(testdropfile "test/testdropfile.c")
  test_program(testerror "test/testerror.c")
  test_program(testevdev "test/testevdev.c")
  test_program(testfile "test/testfile.c")
  test_program(testfilesystem "test/testfilesystem.c")
  test_program(testgamecontroller "test/testgamecontroller.c;test/testutils.c")
  test_program(testgeometry "test/testgeometry.c;test/testutils.c")
  test_program(testgesture "test/testgesture.c")
  test_program(testhaptic "test/testhaptic.c")
  test_program(testhittesting "test/testhittesting.c")
  test_program(testhotplug "test/testhotplug.c")
  test_program(testiconv "test/testiconv.c;test/testutils.c")
  test_program(testime "test/testime.c;test/testutils.c")
  test_program(testintersections "test/testintersections.c")
  test_program(testjoystick "test/testjoystick.c")
  test_program(testkeys "test/testkeys.c")
  test_program(testloadso "test/testloadso.c")
  test_program(testlocale "test/testlocale.c")
  test_program(testlock "test/testlock.c")
  test_program(testmessage "test/testmessage.c")
  test_program(testmouse "test/testmouse.c")
  test_program(testmultiaudio "test/testmultiaudio.c;test/testutils.c")
  if(APPLE)
    set(TESTNATIVE_EXTRA "test/testnativecocoa.m")
  elseif(WIN32)
    set(TESTNATIVE_EXTRA "test/testnativew32.c")
  else()
    find_package(X11 REQUIRED)
    set(TESTNATIVE_EXTRA "test/testnativex11.c")
  endif()
  test_program(testnative "test/testnative.c;test/testutils.c;${TESTNATIVE_EXTRA}")
  test_program(testoverlay2 "test/testoverlay2.c;test/testutils.c;test/testyuv_cvt.c")
  test_program(testplatform "test/testplatform.c")
  test_program(testpower "test/testpower.c")
  test_program(testqsort "test/testqsort.c")
  test_program(testrelative "test/testrelative.c")
  test_program(testrendercopyex "test/testrendercopyex.c;test/testutils.c")
  test_program(testrendertarget "test/testrendertarget.c;test/testutils.c")
  test_program(testresample "test/testresample.c")
  test_program(testrumble "test/testrumble.c")
  test_program(testscale "test/testscale.c;test/testutils.c")
  test_program(testsem "test/testsem.c")
  test_program(testsensor "test/testsensor.c")
  test_program(testshape "test/testshape.c")
  test_program(testsprite2 "test/testsprite2.c;test/testutils.c")
  test_program(testspriteminimal "test/testspriteminimal.c;test/testutils.c")
  test_program(teststreaming "test/teststreaming.c;test/testutils.c")
  test_program(testsurround "test/testsurround.c")
  test_program(testthread "test/testthread.c")
  test_program(testtimer "test/testtimer.c")
  test_program(testurl "test/testurl.c")
  test_program(testver "test/testver.c")
  test_program(testviewport "test/testviewport.c;test/testutils.c")
  test_program(testvulkan "test/testvulkan.c")
  test_program(testwm2 "test/testwm2.c")
  test_program(testyuv "test/testyuv.c;test/testyuv_cvt.c")
  test_program(torturethread "test/torturethread.c")
  test_program(testgl2 "test/testgl2.c")
  if(HAVE_OPENGLES)
    test_program(testgles "test/testgles.c")
  endif()
  if(HAVE_OPENGLES_V2)
    test_program(testgles2 "test/testgles2.c")
    test_program(testgles2_sdf "test/testgles2_sdf.c;test/testutils.c")
  endif()

  list(APPEND TESTAUTOMATION_SRCS "test/testautomation.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_audio.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_guid.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_main.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_platform.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_sdltest.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_syswm.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_hints.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_math.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_rect.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_stdlib.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_timer.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_clipboard.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_joystick.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_mouse.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_render.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_video.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_events.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_keyboard.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_pixels.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_rwops.c")
  list(APPEND TESTAUTOMATION_SRCS "test/testautomation_surface.c")
  test_program(testautomation "${TESTAUTOMATION_SRCS}")

  if(OPENGL_FOUND)
    if(CMAKE_VERSION VERSION_LESS 3.10 OR NOT OPENGL_opengl_LIBRARY)
      target_link_libraries(testgl2 PRIVATE ${OPENGL_gl_LIBRARY})
    else()
      target_link_libraries(testgl2 PRIVATE ${OPENGL_opengl_LIBRARY})
    endif()
  endif()

  foreach(fname
    "axis.bmp"
    "button.bmp"
    "controllermap.bmp"
    "controllermap_back.bmp"
    "icon.bmp"
    "moose.dat"
    "sample.bmp"
    "sample.wav"
    "testgles2_sdf_img_normal.bmp"
    "testgles2_sdf_img_sdf.bmp"
    "testyuv.bmp"
    "unifont-13.0.06.hex"
    "utf8.txt")
      file(COPY "${CMAKE_SOURCE_DIR}/test/${fname}" DESTINATION "${CMAKE_BINARY_DIR}")
  endforeach()
endif()

set(installed_targets SDL2)

if(SDL2COMPAT_STATICDEVEL AND SDL2COMPAT_DEVEL)
  add_library(SDL2-static STATIC ${SDL2COMPAT_SRCS})
  add_library(SDL2::SDL2-static ALIAS SDL2-static)
  target_link_libraries(SDL2-static PRIVATE $<BUILD_INTERFACE:SDL3::Headers>)
  target_include_directories(SDL2-static
    INTERFACE
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
      "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/SDL2>"
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>"
      "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include/SDL2>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/SDL2>"
  )
  target_compile_definitions(SDL2-static PRIVATE _REENTRANT)
  target_link_libraries(SDL2-static PRIVATE ${CMAKE_DL_LIBS})
  set_target_properties(SDL2-static PROPERTIES
    VERSION "${PROJECT_VERSION}"
    OUTPUT_NAME "SDL2"
  )
  if(CMAKE_STATIC_LIBRARY_PREFIX STREQUAL "" AND CMAKE_STATIC_LIBRARY_SUFFIX STREQUAL ".lib")
    set_target_properties(SDL2-static PROPERTIES
      OUTPUT_NAME "SDL2-static"
    )
  endif()

  list(APPEND installed_targets SDL2-static)
endif()

if(SDL2COMPAT_DEVEL)
  if(WIN32 AND NOT MINGW)
    set(SDL23COMPAT_INSTALL_CMAKEDIR_DEFAULT "cmake")
    set(LICENSES_PREFIX "licenses/SDL2")
  else()
    set(SDL23COMPAT_INSTALL_CMAKEDIR_DEFAULT "${CMAKE_INSTALL_LIBDIR}/cmake/SDL2")
    set(LICENSES_PREFIX "${CMAKE_INSTALL_DATAROOTDIR}/licenses/SDL2")
  endif()
  set(SDL23COMPAT_INSTALL_CMAKEDIR "${SDL23COMPAT_INSTALL_CMAKEDIR_DEFAULT}" CACHE STRING "Location where to install SDL2Config.cmake")

  list(APPEND installed_targets SDL2main SDL2_test)
  foreach(target ${installed_targets})
    install(TARGETS ${target} EXPORT ${target}Targets
      ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
      RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )
    install(EXPORT ${target}Targets
      FILE ${target}Targets.cmake
      NAMESPACE SDL2::
      DESTINATION "${SDL23COMPAT_INSTALL_CMAKEDIR}"
    )
  endforeach()
  configure_package_config_file(SDL2Config.cmake.in "${CMAKE_CURRENT_BINARY_DIR}/SDL2Config.cmake"
    PATH_VARS CMAKE_INSTALL_PREFIX CMAKE_INSTALL_FULL_BINDIR CMAKE_INSTALL_FULL_INCLUDEDIR CMAKE_INSTALL_FULL_LIBDIR
    INSTALL_DESTINATION "${SDL23COMPAT_INSTALL_CMAKEDIR}"
  )
  write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/SDL2ConfigVersion.cmake"
    VERSION ${SDL_VERSION}
    COMPATIBILITY AnyNewerVersion
  )
  install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/SDL2Config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/SDL2ConfigVersion.cmake
    DESTINATION "${SDL23COMPAT_INSTALL_CMAKEDIR}"
  )

  file(GLOB SDL2_INCLUDE_FILES ${PROJECT_SOURCE_DIR}/include/SDL2/*.h)
  set(SDL2_COPIED_INCLUDE_FILES)
  foreach(_hdr IN LISTS SDL2_INCLUDE_FILES)
    if(_hdr MATCHES ".*(SDL_config).*")
      # FIXME: how to handle SDL_config.h? Generate a new one from SDL_config.h.cmake? Copy SDL_config.h + SDL_config_xxxxx.h?
      install(FILES "${_hdr}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL2")
    elseif(_hdr MATCHES ".*(SDL_revision).*")
      list(REMOVE_ITEM SDL2_INCLUDE_FILES "${_hdr}")
    else()
      install(FILES "${_hdr}" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL2")
    endif()
  endforeach()
  install(FILES "${PROJECT_BINARY_DIR}/include/SDL2/SDL_revision.h" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/SDL2")

  if(NOT MSVC)
    if(WIN32)
      set(SDL_CFLAGS "")
      set(SDL_RLD_FLAGS "")
      set(SDL_LIBS "-lmingw32 -lSDL2main -lSDL2 -mwindows")
      set(SDL_STATIC_LIBS "")
    elseif(APPLE)
      set(SDL_CFLAGS "-D_THREAD_SAFE")
      set(SDL_LIBS "-lSDL2main -lSDL2 -Wl,-framework,Cocoa")
      set(SDL_STATIC_LIBS "")
      set(SDL_RLD_FLAGS "")  # !!! FIXME: this forces rpath, which we might want?
    else() # unix
      set(SDL_CFLAGS "-D_GNU_SOURCE=1 -D_REENTRANT")
      set(SDL_RLD_FLAGS "")  # !!! FIXME: this forces rpath, which we might want?
      set(SDL_LIBS "-lSDL2")
      set(SDL_STATIC_LIBS "")
      foreach(lib ${CMAKE_DL_LIBS})
        set(SDL_STATIC_LIBS "-l${lib}")
      endforeach()
      if(NOT SDL2COMPAT_STATICDEVEL)
        set(SDL_STATIC_LIBS "")
      endif()
    endif()

    # !!! FIXME: do we _want_ static builds?
    if(SDL2COMPAT_STATICDEVEL)
      set(ENABLE_STATIC_TRUE "")
      set(ENABLE_STATIC_FALSE "#")
    else()
      set(ENABLE_STATIC_TRUE "#")
      set(ENABLE_STATIC_FALSE "")
    endif()
    set(ENABLE_SHARED_TRUE "")
    set(ENABLE_SHARED_FALSE "#")

    configure_file(sdl2_compat.pc.in sdl2_compat.pc @ONLY)
    install(FILES ${CMAKE_BINARY_DIR}/sdl2_compat.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        )

    configure_file("${CMAKE_SOURCE_DIR}/sdl2-config.in" "${CMAKE_BINARY_DIR}/sdl2-config" @ONLY)
    install(PROGRAMS "${CMAKE_BINARY_DIR}/sdl2-config" DESTINATION bin)
  endif()

  set(SOEXT ${CMAKE_SHARED_LIBRARY_SUFFIX})
  get_target_property(SONAME SDL2 OUTPUT_NAME)
  if(UNIX AND NOT ANDROID)
    install(CODE "
      execute_process(COMMAND \"${CMAKE_COMMAND}\" -E create_symlink
        \"lib${SONAME}${SOPOSTFIX}${SOEXT}\" \"libSDL2${SOPOSTFIX}${SOEXT}\"
        WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\")")
    install(FILES ${CMAKE_BINARY_DIR}/libSDL2${SOPOSTFIX}${SOEXT} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  endif()

  install(FILES "${PROJECT_SOURCE_DIR}/sdl2.m4" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/aclocal")
endif()
