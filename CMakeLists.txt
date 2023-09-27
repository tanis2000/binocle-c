cmake_minimum_required(VERSION 3.1)

project(binocle)

# Policies
if("${CMAKE_VERSION}" VERSION_GREATER 3.1)
	cmake_policy(SET CMP0054 NEW)
	cmake_policy(SET CMP0025 NEW)
endif()

# cmake modules directory
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Enables CMake database for YCM
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Enables CMake debugging with VSCode
include(CMakeToolsHelpers OPTIONAL)

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

# Add our own DEBUG define for Debug build types
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEBUG")

# Registers build options that are exposed to cmake
include(CMakeOptions.txt)

if(BINOCLE_LUAJIT)
	add_definitions(-DBINOCLE_LUAJIT)
endif ()

if(BINOCLE_SHOW_CONSOLE)
	add_definitions(-DBINOCLE_SHOW_CONSOLE)
endif()

if(BINOCLE_LOG_MEMORY_ALLOCATIONS)
	add_definitions(-DBINOCLE_LOG_MEMORY_ALLOCATIONS)
endif()

include(BinocleUtils)

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "2")
SET(VERSION_PATCH "1")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

set(BINOCLE_LINK_LIBRARIES binocle-static)

#if (EMSCRIPTEN)
#	set(BINOCLE_LINK_LIBRARIES binocle-static lua)
#endif()
#
#if (APPLE AND NOT IOS)
#	set(BINOCLE_LINK_LIBRARIES binocle-static luajit-5.1)
#endif()
#
#if (IOS)
#	set(BINOCLE_LINK_LIBRARIES binocle-static lua)
#endif()
#
#if (ANDROID)
#	set(BINOCLE_LINK_LIBRARIES binocle-static luajit-5.1)
#endif()
#
#if (MSVC)
#	set(BINOCLE_LINK_LIBRARIES binocle-static luajit-5.1)
#endif()

if (MSVC)
	message("Including Windows defines")
	include(BinocleWindows)
elseif(APPLE AND NOT ANDROID)
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
	message("Setting BINOCLE_DATA_DIR before including web settings")
	set(BINOCLE_DATA_DIR "${CMAKE_SOURCE_DIR}/assets/")
	message("Including Emscripten defines")
	include(BinocleWeb)
endif()

message("Including documentation defines")
include(BinocleDocs)
if(BUILD_DOC)
	return()
endif()

add_subdirectory(src)

if (${BUILD_EXAMPLE})
	message("Building example project is enabled")
	add_subdirectory(example)
endif()

message("Linking with the following libraries: ${BINOCLE_LINK_LIBRARIES}")
