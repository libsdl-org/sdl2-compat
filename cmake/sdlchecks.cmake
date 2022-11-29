# Requires:
# - nada
macro(CheckOpenGLES)
  set(HAVE_OPENGLES FALSE)
  check_c_source_compiles("
      #include <GLES/gl.h>
      #include <GLES/glext.h>
      int main (int argc, char** argv) { return 0; }" HAVE_OPENGLES_V1)
  if(HAVE_OPENGLES_V1)
    set(HAVE_OPENGLES TRUE)
    set(SDL_VIDEO_OPENGL_ES 1)
    set(SDL_VIDEO_RENDER_OGL_ES 1)
  endif()
  check_c_source_compiles("
      #include <GLES2/gl2.h>
      #include <GLES2/gl2ext.h>
      int main (int argc, char** argv) { return 0; }" HAVE_OPENGLES_V2)
  if(HAVE_OPENGLES_V2)
    set(HAVE_OPENGLES TRUE)
    set(SDL_VIDEO_OPENGL_ES2 1)
    set(SDL_VIDEO_RENDER_OGL_ES2 1)
  endif()
endmacro()
