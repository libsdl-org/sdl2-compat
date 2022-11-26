# NOTE:  If you installed SDL2 at an unusual place and FindSDL2 fails,
#        you can define SDL2_INCLUDE_DIR on the cmdline.  For example:
#        cmake -DSDL2_INCLUDE_DIR=/opt/SDL2/include/SDL2 [other stuff]

cmake_minimum_required(VERSION 3.0.0)
project(sdl2_compat
        VERSION 2.90.0
        LANGUAGES C)

option(SDL2TESTS "Enable to build SDL2 test programs" ON)
option(SDL2DEVEL "Enable installing SDL2 development headers" ON)
option(STATICDEVEL "Enable installing static link library" OFF)

if(STATICDEVEL AND NOT (CMAKE_SYSTEM_NAME MATCHES "Linux"))
  message(FATAL_ERROR "Static builds are only supported on Linux.")
endif()

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake/modules"
)

include(CheckCSourceCompiles)
include(CheckIncludeFile)
include(CheckCCompilerFlag)
include(GNUInstallDirs)

set(CMAKE_SKIP_RPATH TRUE)

if(APPLE)
# !!! FIXME: assuming we don't need this; if so, remove this.
  #set(OSX_SRCS "src/sdl2_compat_objc.m")
  #set_source_files_properties(${OSX_SRCS} PROPERTIES LANGUAGE C)

#  # !!! FIXME: what does SDL2 expect here?
#  # compatibility version set to match SDL-1.2 autotools build
#  # Xcode project file uses 1.0.0, but it's more compatible to use the
#  # higher version.
#  set(DYLIB_COMPAT_VERSION 12.0.0 CACHE STRING "library compatibility version")
#  set(DYLIB_CURRENT_VERSION 12.61.0 CACHE STRING "library current version")
  include_directories("/opt/X11/include")  # hack.
  if(CMAKE_VERSION VERSION_LESS 3.9)
  else()
    cmake_policy(SET CMP0068 NEW)  # on macOS, don't let RPATH affect install_name.
  endif()
endif()

if(WIN32)
  set(WIN32_SRCS "src/version.rc")
endif()

set(SDL2COMPAT_SRCS
  src/sdl2_compat.c
  ${OSX_SRCS}
  ${WIN32_SRCS}
)
add_library(SDL SHARED ${SDL2COMPAT_SRCS})

find_package(SDL3)
if(NOT SDL3_INCLUDE_DIRS)
  message(FATAL_ERROR "Could not find SDL3 headers")
endif()
target_include_directories(SDL PRIVATE ${SDL3_INCLUDE_DIRS})

set(EXTRA_CFLAGS )
if (CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
  set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wall")
  check_c_compiler_flag(-Wdeclaration-after-statement HAVE_WDECLARATION_AFTER_STATEMENT)
  if(HAVE_WDECLARATION_AFTER_STATEMENT)
    set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Wdeclaration-after-statement")
  endif()
  check_c_compiler_flag(-Werror=declaration-after-statement HAVE_WERROR_DECLARATION_AFTER_STATEMENT)
  if(HAVE_WERROR_DECLARATION_AFTER_STATEMENT)
    set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -Werror=declaration-after-statement")
  endif()
endif()

# !!! FIXME: is this still necessary with SDL2?
# just in case:
check_include_file("immintrin.h" HAVE_IMMINTRIN_H)
if(NOT HAVE_IMMINTRIN_H)
  set(EXTRA_CFLAGS "${EXTRA_CFLAGS} -DSDL_DISABLE_IMMINTRIN_H")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")
string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)

if(UNIX AND NOT APPLE)
    set_target_properties(SDL PROPERTIES COMPILE_DEFINITIONS "_REENTRANT")
    target_link_libraries(SDL PRIVATE ${CMAKE_DL_LIBS})
endif()
if(APPLE)
    # !!! FIXME: check OUTPUT_NAME value, and do we need AppKit now? I assume that was for SDL_main code in 1.2.
    set_target_properties(SDL PROPERTIES COMPILE_DEFINITIONS "_THREAD_SAFE")
    set_target_properties(SDL PROPERTIES LINK_FLAGS
            "-Wl,-compatibility_version,${DYLIB_COMPAT_VERSION} -Wl,-current_version,${DYLIB_CURRENT_VERSION}")
    #target_link_libraries(SDL PRIVATE "-framework AppKit")
    set_target_properties(SDL PROPERTIES
        OUTPUT_NAME "SDL2-2.0.0"
    )
elseif(UNIX AND NOT ANDROID)
    # !!! FIXME: check OUTPUT_NAME value
    set_target_properties(SDL PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "0"
        OUTPUT_NAME "SDL2-2.0")
elseif(WIN32)
    # avoid DLL having 'lib' prefix with MinGW
    set(CMAKE_SHARED_LIBRARY_PREFIX "")
    set_target_properties(SDL PROPERTIES COMPILE_DEFINITIONS "DLL_EXPORT")
    set_target_properties(SDL PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "0"
        OUTPUT_NAME "SDL2")
elseif(OS2)
    set_target_properties(SDL PROPERTIES COMPILE_DEFINITIONS "BUILD_SDL") # for DECLSPEC
    set_target_properties(SDL PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "0"
        OUTPUT_NAME "SDL2")
else()
    set_target_properties(SDL PROPERTIES
        VERSION "${PROJECT_VERSION}"
        SOVERSION "0"
        OUTPUT_NAME "SDL2")
endif()

if(MINGW)
    set_target_properties(SDL PROPERTIES LINK_FLAGS "-nostdlib")
    target_link_libraries(SDL PRIVATE "-static-libgcc -lgcc") # libgcc is needed for 32 bit (x86) builds
endif()
if(MSVC)
    # Don't try to link with the default set of libraries.
    set(MSVC_FLAGS "/GS-")
    check_c_source_compiles("int main(void) {
#ifndef _M_IX86
#error not x86
#endif
return 0; }" IS_X86)
    if(IS_X86)  # don't emit SSE2 in x86 builds
      set(MSVC_FLAGS "${MSVC_FLAGS} /arch:SSE")
    endif()
    set_target_properties(SDL PROPERTIES COMPILE_FLAGS ${MSVC_FLAGS})
    set_target_properties(SDL PROPERTIES LINK_FLAGS "/NODEFAULTLIB")
    # Make sure /RTC1 is disabled: (from SDL2 CMake)
    foreach(flag_var
      CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
      string(REGEX REPLACE "/RTC(su|[1su])" "" ${flag_var} "${${flag_var}}")
    endforeach(flag_var)
endif()

# !!! FIXME: what needs to be done for sdl2-compat? Look into this.
## SDLmain library...
#if(APPLE)
#    add_library(SDLmain STATIC src/SDLmain/macosx/SDLMain.m)
#    set_source_files_properties(src/SDLmain/macosx/SDLMain.m PROPERTIES LANGUAGE C)
#elseif(WIN32)
#    add_library(SDLmain STATIC src/SDLmain/win32/SDL_win32_main.c)
#    set_target_properties(SDLmain PROPERTIES COMPILE_DEFINITIONS "_CRT_SECURE_NO_WARNINGS;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE")  # !!! FIXME: don't use C runtime? We fixed this in SDL2.
#else()
    add_library(SDLmain STATIC src/SDLmain/dummy/SDL_dummy_main.c)
#endif()
target_include_directories(SDLmain PRIVATE "include/SDL2")

# SDLtest library...
add_library(SDLtest STATIC
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
target_include_directories(SDLtest PRIVATE "include/SDL2")

if(SDL2TESTS)
    if(NOT (WIN32 OR APPLE OR CYGWIN OR HAIKU OR BEOS))
      find_library(MATH_LIBRARY m)
    endif()

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
        if(MINGW)
          target_link_libraries(${_NAME} mingw32 SDLmain SDLtest SDL)
        else()
          target_link_libraries(${_NAME} SDLmain SDLtest SDL)
        endif()
        # Turn off MSVC's aggressive C runtime warnings for the old test programs.
        if(MSVC)
          set_target_properties(${_NAME} PROPERTIES COMPILE_DEFINITIONS "${HAVE_OPENGL_DEFINE};_CRT_SECURE_NO_WARNINGS;_CRT_SECURE_NO_DEPRECATE;_CRT_NONSTDC_NO_DEPRECATE")
        elseif(APPLE)
          set_target_properties(${_NAME} PROPERTIES COMPILE_DEFINITIONS "${HAVE_OPENGL_DEFINE};GL_SILENCE_DEPRECATION=1")
          target_link_libraries(${_NAME} "-Wl,-framework,Cocoa")
        else()
          set_target_properties(${_NAME} PROPERTIES COMPILE_DEFINITIONS "${HAVE_OPENGL_DEFINE}")
        endif()
        if(MATH_LIBRARY)
          target_link_libraries(${_NAME} ${MATH_LIBRARY})
        endif()
    endmacro()

    test_program(checkkeys "test/checkkeys.c")
    test_program(checkkeysthreads "test/checkkeysthreads.c")
    test_program(controllermap "test/controllermap.c")
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
    test_program(testgamecontroller "test/testgamecontroller.c")
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
    test_program(testnative "test/testnative.c;test/testutils.c")
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
    test_program(testgles "test/testgles.c")
    test_program(testgles2 "test/testgles2.c")
    test_program(testgles2_sdf "test/testgles2_sdf.c;test/testutils.c")

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
        target_link_libraries(testgl2 ${OPENGL_gl_LIBRARY})
      else()
        target_link_libraries(testgl2 ${OPENGL_opengl_LIBRARY})
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

install(TARGETS SDL SDLmain SDLtest
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(SDL2DEVEL)
  install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  if(NOT MSVC)
    if(WIN32)
      set(SDL_CFLAGS "")
      set(SDL_RLD_FLAGS "")
      set(SDL_LIBS "-lmingw32 -lSDLmain -lSDL -mwindows")
      set(SDL_STATIC_LIBS "")
    elseif(APPLE)
      set(SDL_CFLAGS "-D_THREAD_SAFE")
      set(SDL_LIBS "-lSDLmain -lSDL -Wl,-framework,Cocoa")
      set(SDL_STATIC_LIBS "")
      set(SDL_RLD_FLAGS "")  # !!! FIXME: this forces rpath, which we might want?
    else() # unix
      set(SDL_CFLAGS "-D_GNU_SOURCE=1 -D_REENTRANT")
      set(SDL_RLD_FLAGS "")  # !!! FIXME: this forces rpath, which we might want?
      set(SDL_LIBS "-lSDL")
      set(SDL_STATIC_LIBS "")
      foreach(lib ${CMAKE_DL_LIBS})
          set(SDL_STATIC_LIBS "-l${lib}")
      endforeach()
      if(NOT STATICDEVEL)
        set(SDL_STATIC_LIBS "")
      endif()
    endif()

    # !!! FIXME: do we _want_ static builds?
    if(STATICDEVEL)
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
  get_target_property(SONAME SDL OUTPUT_NAME)
  if(UNIX AND NOT ANDROID)
    install(CODE "
      execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
        \"lib${SONAME}${SOPOSTFIX}${SOEXT}\" \"libSDL${SOPOSTFIX}${SOEXT}\"
        WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\")")
    install(FILES ${CMAKE_BINARY_DIR}/libSDL${SOPOSTFIX}${SOEXT} DESTINATION "${CMAKE_INSTALL_LIBDIR}")
  endif()

  install(FILES "${CMAKE_SOURCE_DIR}/sdl.m4" DESTINATION "${CMAKE_INSTALL_FULL_DATAROOTDIR}/aclocal")
endif()

if(STATICDEVEL AND SDL2DEVEL)
  add_library(SDL-static STATIC ${SDL2COMPAT_SRCS})
  target_include_directories(SDL-static PRIVATE ${SDL2_INCLUDE_DIRS})
  set_target_properties(SDL-static PROPERTIES COMPILE_DEFINITIONS "_REENTRANT")
  target_link_libraries(SDL-static PRIVATE ${CMAKE_DL_LIBS})
  set_target_properties(SDL-static PROPERTIES
          VERSION "${PROJECT_VERSION}"
        OUTPUT_NAME "SDL")

  install(TARGETS SDL-static
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )
endif()
