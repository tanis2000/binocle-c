
add_subdirectory(freetype)
add_subdirectory(kazmath)
add_subdirectory(miniaudio)
add_subdirectory(parson)
add_subdirectory(sdl_mixer)
add_subdirectory(zlib)

if (NOT EMSCRIPTEN)
	add_subdirectory(sdl)
    add_subdirectory(vorbis)
    add_subdirectory(ogg)
endif()

#if (NOT ANDROID)
#    add_subdirectory(vorbis)
#endif ()

if (APPLE AND NOT IOS)
    add_subdirectory(glew)
endif()

if (ANDROID)
    add_subdirectory(vorbisidec)
endif()

if (MSVC)
    add_subdirectory(glew)
endif()
