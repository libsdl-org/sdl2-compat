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

function(test_program NAME)
  cmake_parse_arguments(args "NONINTERACTIVE" "TIMEOUT" "SRC" ${ARGN})
  if(ANDROID)
    add_library(${NAME} SHARED ${args_SRC})
  else()
    add_executable(${NAME} ${args_SRC})
  endif()
  SDL_AddCommonCompilerFlags(${NAME} WERROR "${SDL2COMPAT_WERROR}")
  target_link_libraries(${NAME} PRIVATE SDL2::SDL2main SDL2::SDL2test SDL2::SDL2)
  # Turn off MSVC's aggressive C runtime warnings for the old test programs.
  if(MSVC)
    target_compile_definitions(${NAME} PRIVATE ${HAVE_OPENGL_DEFINE} _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE _CRT_NONSTDC_NO_DEPRECATE)
  elseif(APPLE)
    target_compile_definitions(${NAME} PRIVATE ${HAVE_OPENGL_DEFINE} GL_SILENCE_DEPRECATION=1)
    target_link_libraries(${NAME} PRIVATE "-Wl,-framework,Cocoa")
  else()
    target_compile_definitions(${NAME} PRIVATE ${HAVE_OPENGL_DEFINE})
  endif()
  if(MATH_LIBRARY)
    target_link_libraries(${NAME} PRIVATE ${MATH_LIBRARY})
  endif()
  if(args_NONINTERACTIVE)
    add_test(
      NAME ${NAME}
      COMMAND ${NAME}
      WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    )
    set(timeout 10)
    if(args_TIMEOUT)
      set(timeout ${args_TIMEOUT})
    endif()
    set_tests_properties(${NAME}
      PROPERTIES
      ENVIRONMENT "SDL_AUDIODRIVER=dummy;SDL_VIDEODRIVER=dummy"
      TIMEOUT ${timeout}
    )
  endif()
endfunction()

test_program(checkkeys SRC "checkkeys.c")
test_program(checkkeysthreads SRC "checkkeysthreads.c")
test_program(controllermap SRC "controllermap.c" "testutils.c")
test_program(loopwave SRC "loopwave.c" "testutils.c")
test_program(loopwavequeue SRC "loopwavequeue.c" "testutils.c")
test_program(testatomic NONINTERACTIVE SRC "testatomic.c")
test_program(testaudiocapture SRC "testaudiocapture.c")
test_program(testaudiohotplug SRC "testaudiohotplug.c" "testutils.c")
test_program(testaudioinfo SRC "testaudioinfo.c")
test_program(testbounds SRC "testbounds.c")
test_program(testcustomcursor SRC "testcustomcursor.c")
test_program(testdisplayinfo SRC "testdisplayinfo.c")
test_program(testdraw2 SRC "testdraw2.c")
test_program(testdrawchessboard SRC "testdrawchessboard.c")
test_program(testdropfile SRC "testdropfile.c")
test_program(testerror NONINTERACTIVE SRC "testerror.c")
test_program(testevdev NONINTERACTIVE SRC "testevdev.c")
test_program(testfile NONINTERACTIVE SRC "testfile.c")
test_program(testfilesystem NONINTERACTIVE SRC "testfilesystem.c")
test_program(testgamecontroller SRC "testgamecontroller.c" "testutils.c")
test_program(testgeometry SRC "testgeometry.c" "testutils.c")
test_program(testgesture SRC "testgesture.c")
test_program(testhaptic SRC "testhaptic.c")
test_program(testhittesting SRC "testhittesting.c")
test_program(testhotplug SRC "testhotplug.c")
test_program(testiconv SRC "testiconv.c" "testutils.c")
test_program(testime SRC "testime.c" "testutils.c")
test_program(testintersections SRC "testintersections.c")
test_program(testjoystick SRC "testjoystick.c")
test_program(testkeys SRC "testkeys.c")
test_program(testloadso SRC "testloadso.c")
test_program(testlocale NONINTERACTIVE SRC "testlocale.c")
test_program(testlock SRC "testlock.c")
test_program(testmessage SRC "testmessage.c")
test_program(testmouse SRC "testmouse.c")
test_program(testmultiaudio SRC "testmultiaudio.c" "testutils.c")
if(APPLE)
  set(TESTNATIVE_EXTRA "testnativecocoa.m")
elseif(WIN32)
  set(TESTNATIVE_EXTRA "testnativew32.c")
elseif(UNIX AND NOT ANDROID)
  find_package(X11 REQUIRED)
  set(TESTNATIVE_EXTRA "testnativex11.c")
endif()
test_program(testnative SRC "testnative.c" "testutils.c" "${TESTNATIVE_EXTRA}")
test_program(testoverlay2 SRC "testoverlay2.c" "testutils.c" "testyuv_cvt.c")
test_program(testplatform NONINTERACTIVE SRC "testplatform.c")
test_program(testpower NONINTERACTIVE SRC "testpower.c")
test_program(testqsort NONINTERACTIVE SRC "testqsort.c")
test_program(testrelative SRC "testrelative.c")
test_program(testrendercopyex SRC "testrendercopyex.c" "testutils.c")
test_program(testrendertarget SRC "testrendertarget.c" "testutils.c")
test_program(testresample SRC "testresample.c")
test_program(testrumble SRC "testrumble.c")
test_program(testscale SRC "testscale.c" "testutils.c")
test_program(testsem SRC "testsem.c")
test_program(testsensor SRC "testsensor.c")
test_program(testshape SRC "testshape.c")
test_program(testsprite2 SRC "testsprite2.c" "testutils.c")
test_program(testspriteminimal SRC "testspriteminimal.c" "testutils.c")
test_program(teststreaming SRC "teststreaming.c" "testutils.c")
test_program(testsurround SRC "testsurround.c")
test_program(testthread NONINTERACTIVE TIMEOUT 40 SRC "testthread.c")
test_program(testtimer NONINTERACTIVE TIMEOUT 60 SRC "testtimer.c")
test_program(testurl SRC "testurl.c")
test_program(testver NONINTERACTIVE SRC "testver.c")
test_program(testviewport SRC "testviewport.c" "testutils.c")
test_program(testvulkan SRC "testvulkan.c")
test_program(testwm2 SRC "testwm2.c")
test_program(testyuv SRC "testyuv.c" "testyuv_cvt.c")
test_program(torturethread SRC "torturethread.c")
test_program(testgl2 SRC "testgl2.c")
if(HAVE_OPENGLES)
  test_program(testgles SRC "testgles.c")
  if(ANDROID)
    target_link_libraries(testgles PRIVATE GLESv1_CM)
  endif()
endif()
if(HAVE_OPENGLES_V2)
  test_program(testgles2 SRC "testgles2.c")
  test_program(testgles2_sdf SRC "testgles2_sdf.c" "testutils.c")
  if(ANDROID)
    target_link_libraries(testgles2 PRIVATE GLESv2)
    target_link_libraries(testgles2_sdf PRIVATE GLESv2)
  endif()
endif()

test_program(testautomation NONINTERACTIVE TIMEOUT 40
  SRC
    "testautomation.c"
    "testautomation_audio.c"
    "testautomation_guid.c"
    "testautomation_main.c"
    "testautomation_platform.c"
    "testautomation_sdltest.c"
    "testautomation_syswm.c"
    "testautomation_hints.c"
    "testautomation_math.c"
    "testautomation_rect.c"
    "testautomation_stdlib.c"
    "testautomation_subsystems.c"
    "testautomation_timer.c"
    "testautomation_clipboard.c"
    "testautomation_joystick.c"
    "testautomation_mouse.c"
    "testautomation_render.c"
    "testautomation_video.c"
    "testautomation_events.c"
    "testautomation_keyboard.c"
    "testautomation_pixels.c"
    "testautomation_rwops.c"
    "testautomation_surface.c"
)

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
  file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${fname}" DESTINATION "${CMAKE_BINARY_DIR}")
endforeach()

check_c_compiler_flag(-Wformat HAVE_WFORMAT)
if(HAVE_WFORMAT)
  target_compile_definitions(testautomation PRIVATE HAVE_WFORMAT)
endif()

cmake_push_check_state()
if(HAVE_WFORMAT)
  # Some compilers ignore -Wformat-extra-args without -Wformat
  string(APPEND CMAKE_REQUIRED_FLAGS " -Wformat")
endif()
check_c_compiler_flag(-Wformat-extra-args HAVE_WFORMAT_EXTRA_ARGS)
cmake_pop_check_state()
if(HAVE_WFORMAT_EXTRA_ARGS)
  target_compile_definitions(testautomation PRIVATE HAVE_WFORMAT_EXTRA_ARGS)
endif()

add_custom_target(sdl2compat-build-tests)

check_language(CXX)
if(CMAKE_CXX_COMPILER)
  enable_language(CXX)
  add_library(test_sdl3_prototypes STATIC EXCLUDE_FROM_ALL "test_sdl3_syms.cpp")
  target_link_libraries(test_sdl3_prototypes PRIVATE SDL3::Headers)
  SDL_AddCommonCompilerFlags(test_sdl3_prototypes WERROR TRUE)
  set_target_properties(test_sdl3_prototypes PROPERTIES SUFFIX ".do_not_use")
  # Make sure test_sdl3_prototypes is always out-of-date
  add_custom_command(TARGET test_sdl3_prototypes POST_BUILD COMMAND "${CMAKE_COMMAND}" -E touch "${CMAKE_CURRENT_SOURCE_DIR}/test_sdl3_syms.cpp")
  add_dependencies(sdl2compat-build-tests test_sdl3_prototypes)
endif()
