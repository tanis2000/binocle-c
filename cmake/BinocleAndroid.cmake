#include (BundleUtilities)

add_definitions (-D__ANDROID__)
add_definitions (-DBINOCLE_GL)
set(BINOCLE_GL true)
add_definitions (-DBINOCLE_GLES2)

#set(CMAKE_SYSTEM_NAME Android)
#set(CMAKE_SYSTEM_VERSION 17)
#set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)
#set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_ROOT})
#set(CMAKE_ANDROID_STL_TYPE gnustl_static)

# Omitted: -stdlib=libc++ 
# Might try using -std=gnu++0x instead of -std=c++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")
set(BINOCLE_LINK_LIBRARIES ${BINOCLE_LINK_LIBRARIES} android dl EGL GLESv1_CM GLESv2 GLESv3 log m stdc++ vorbisidec ogg OpenSLES "-frtti" "-Wl,--no-undefined" "-Wl,-z,noexecstack" "-shared")
