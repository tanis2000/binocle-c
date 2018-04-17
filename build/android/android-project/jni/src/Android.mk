LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := binocle
LOCAL_MODULE_FILENAME := binocle_static
LOCAL_SRC_FILES := ../../../gen/src/Binocle/libbinocle.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := sdl
LOCAL_MODULE_FILENAME := sdl_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/sdl/libsdl.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := sdl_mixer
LOCAL_MODULE_FILENAME := sdl_mixer_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/sdl_mixer/libsdl_mixer.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := sdl_image
LOCAL_MODULE_FILENAME := sdl_image_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/sdl_image/libsdl_image.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := flac
LOCAL_MODULE_FILENAME := flac_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/FLAC/libflac.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := freetype
LOCAL_MODULE_FILENAME := freetype_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/freetype/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := jpeg
LOCAL_MODULE_FILENAME := jpeg_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/jpeg/libjpeg.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := ogg
LOCAL_MODULE_FILENAME := ogg_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/ogg/libogg.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := overlap2d
LOCAL_MODULE_FILENAME := overlap2d_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/overlap2d/liboverlap2d.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := png
LOCAL_MODULE_FILENAME := png_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/png/libpng.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := tinyxml
LOCAL_MODULE_FILENAME := tinyxml_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/tinyxml/libtinyxml.a
include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := tmxparser
LOCAL_MODULE_FILENAME := tmxparser_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/tmxparser/libtmxparser.a
include $(PREBUILT_STATIC_LIBRARY)


#include $(CLEAR_VARS)

#LOCAL_MODULE := vorbis
#LOCAL_MODULE_FILENAME := vorbis_static
#LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/vorbis/libvorbis.a
#include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE := vorbisidec
LOCAL_MODULE_FILENAME := vorbisidec_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/vorbisidec/libvorbisidec.a
include $(PREBUILT_STATIC_LIBRARY)




include $(CLEAR_VARS)

LOCAL_MODULE := zlib
LOCAL_MODULE_FILENAME := zlib_static
LOCAL_SRC_FILES := ../../../gen/src/ThirdParty/zlib/libzlib.a
include $(PREBUILT_STATIC_LIBRARY)



include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(SDL_PATH)/include/configs/default \
	$(LOCAL_PATH)/src \
	$(LOCAL_PATH)/src/ThirdParty/sdl_image \
	$(LOCAL_PATH)/src/ThirdParty/sdl_mixer \
	$(LOCAL_PATH)/src/ThirdParty/TinyXml \
	$(LOCAL_PATH)/src/ThirdParty/tmxparser \
	$(LOCAL_PATH)/src/ThirdParty/overlap2d \
	$(LOCAL_PATH)/src/ThirdParty

# Add your application source files here...
LOCAL_SRC_FILES := \
	src/ThirdParty/sdl/src/main/android/SDL_android_main.c \
	src/ExampleProject/main.cpp \
	src/ExampleProject/MyGame.cpp
#$(SDL_PATH)/src/main/android/SDL_android_main.c \


#LOCAL_CPP_FEATURES += exceptions
#LOCAL_CPP_FEATURES += rtti
LOCAL_CFLAGS += -std=c++11 -D__ANDROID__

#LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_STATIC_LIBRARIES := binocle sdl sdl_mixer sdl_image flac freetype jpeg ogg overlap2d png tinyxml tmxparser vorbisidec zlib

LOCAL_LDLIBS := -lGLESv1_CM -lGLESv2 -llog -landroid -L$(LOCAL_PATH)/inc
LOCAL_WHOLE_STATIC_LIBRARIES := binocle sdl sdl_mixer sdl_image flac freetype jpeg ogg overlap2d png tinyxml tmxparser vorbisidec zlib libgnustl_static

include $(BUILD_SHARED_LIBRARY)

$(call import-module,cxx-stl/gnu-libstdc++)

