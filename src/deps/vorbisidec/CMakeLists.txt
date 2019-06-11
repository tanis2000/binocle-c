add_definitions(-DDHAVE_LIBC)

add_definitions(-DOGG_MUSIC)

include_directories(.
	../ogg/include
	../vorbis/include
	)

if (APPLE)
	if(IOS)
	else()
	endif()
endif()

if(ANDROID)
endif()

file(GLOB_RECURSE SOURCES 
        *.c
        *.h
)
#file(GLOB REMOVE_SOURCES  
#  "src/SFML/Main/*.cpp"
#)
#list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
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

add_library(vorbisidec OBJECT ${HEADERS} ${SOURCES})

if (IOS)
	set_target_properties(
			vorbisidec
			PROPERTIES
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
	)
endif(IOS)

