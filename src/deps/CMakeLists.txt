
add_subdirectory(freetype)
add_subdirectory(kazmath)
add_subdirectory(ogg)
add_subdirectory(sdl_mixer)
add_subdirectory(zlib)

if (NOT EMSCRIPTEN)
	add_subdirectory(sdl)
endif()

#if (NOT ANDROID)
    add_subdirectory(vorbis)
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
