
add_subdirectory(freetype)
add_subdirectory(kazmath)
add_subdirectory(parson)
add_subdirectory(physfs)
add_subdirectory(rxi_map)
add_subdirectory(zlib)
add_subdirectory(libuv)
add_subdirectory(wren)

if (NOT EMSCRIPTEN)
	add_subdirectory(sdl)
    add_subdirectory(vorbis)
    add_subdirectory(ogg)
else()
    add_subdirectory(lua)
endif()

if (APPLE AND NOT IOS)
    add_subdirectory(glew)
    add_subdirectory(luajit)
endif()

if (ANDROID)
    add_subdirectory(vorbisidec)
    add_subdirectory(luajit)
endif()

if (IOS)
    add_subdirectory(lua)
endif()

if (MSVC)
    add_subdirectory(glew)
    add_subdirectory(luajit)
endif()

