LOCAL_PATH := $(call my-dir)

# SDL2 shared library

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := dynapi/SDL_dynapi.c sdl2_compat.c

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES -DHAVE_ALLOCA -DHAVE_ALLOCA_H -DSDL_INCLUDE_STDBOOL_H
LOCAL_CFLAGS += -Wall -Wextra -Wno-unused-parameter -Wno-unused-local-typedefs
LOCAL_LDLIBS := -ldl -llog
LOCAL_LDFLAGS := -Wl,--no-undefined

ifeq ($(NDK_DEBUG),1)
    cmd-strip :=
endif

include $(BUILD_SHARED_LIBRARY)
