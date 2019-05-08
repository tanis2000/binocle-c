project(binocle)

cmake_minimum_required(VERSION 2.8.12)

# Policies
if("${CMAKE_VERSION}" VERSION_GREATER 3.1)
	cmake_policy(SET CMP0054 NEW)
	cmake_policy(SET CMP0025 NEW)
endif()

# cmake modules directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/cmake")

# Enables CMake database for YCM
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Enables CMake debugging with VSCode
include(CMakeToolsHelpers OPTIONAL)

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

# Enables CMake debugging with VSCode
include(CMakeToolsHelpers OPTIONAL)

# Add our own DEBUG define for Debug build types
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

include(BinocleUtils)

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "1")
SET(VERSION_PATCH "0")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set(BINOCLE_LINK_LIBRARIES binocle freetype kazmath parson)

if (NOT EMSCRIPTEN)
	set(BINOCLE_LINK_LIBRARIES ${BINOCLE_LINK_LIBRARIES} sdl ogg vorbis zlib)
endif()

if (MSVC)
	message("Including Windows defines")
	include(BinocleWindows)
elseif(APPLE)
	if(IOS)
		message("Including iOS defines")
		include(BinocleIOS)
	else()
		message("Including Mac defines")
		include(BinocleMac)
	endif()
elseif(ANDROID)
	message("Including Android defines")
	include(BinocleAndroid)
elseif(EMSCRIPTEN)
	message("Including Emscripten defines")
	include(BinocleWeb)
endif()

add_subdirectory(src)
add_subdirectory(example)

message("Linking with the following libraries: ${BINOCLE_LINK_LIBRARIES}")
