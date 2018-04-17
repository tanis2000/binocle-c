include (BundleUtilities)

add_definitions (-D__ANDROID__)

# Omitted: -stdlib=libc++ 
# Might try using -std=gnu++0x instead of -std=c++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -Wall")
set(BINOCLE_LINK_LIBRARIES ${BINOCLE_LINK_LIBRARIES} EGL GLESv1_CM GLESv2 android m stdc++ log dl vorbisidec "ogg" "-frtti" "-Wl,--no-undefined" "-Wl,-z,noexecstack" "-shared")
