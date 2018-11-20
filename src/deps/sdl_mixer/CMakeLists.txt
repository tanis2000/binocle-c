add_definitions(-DDHAVE_LIBC)

add_definitions(-DMUSIC_OGG)

include_directories(.
	../sdl/include
	../ogg/include
	../vorbis/include
	)

if (APPLE)
	if(IOS)
		include_directories(../sdl/include/configs/default)
	else()
		include_directories(../sdl/include/configs/mac)
	endif()
endif()

if(ANDROID)
	add_definitions(-D_ARM_ASSEM_)
	add_definitions(-DOGG_USE_TREMOR)
	include_directories(../sdl/include/configs/default)
	include_directories(../vorbisidec)
endif()

if (WIN32 OR EMSCRIPTEN)
	include_directories(../sdl/include/configs/default)
endif()

file(GLOB_RECURSE SOURCES 
        *.c
        *.h
)
file(GLOB REMOVE_SOURCES  
  "playmus.c"
  "playwave.c"
)
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
set(SOURCES 
	${SOURCES} 
	)

	
file(GLOB_RECURSE HEADERS 
	*.h
	)
#file(GLOB_RECURSE REMOVE_HEADERS  
#  "src/SFML/Network/Win32/*.hpp"
#  "src/SFML/Window/iOS/*.hpp"
#)
#list(REMOVE_ITEM HEADERS ${REMOVE_HEADERS})


if (APPLE) 
	if (IOS)
	else()
	endif()
endif()


add_library(sdl_mixer ${HEADERS} ${SOURCES})


if (IOS)
	set_target_properties(
			sdl_mixer
			PROPERTIES
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
	)
endif(IOS)
