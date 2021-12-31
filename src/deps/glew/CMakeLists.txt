project(glew)

include_directories(.)

file(GLOB_RECURSE SOURCES 
    src/glew.c
)
#file(GLOB REMOVE_SOURCES  
#  "src/SFML/Main/*.cpp"
#)
#list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
set(SOURCES 
	${SOURCES} 
	)

	
file(GLOB_RECURSE HEADERS 
	include/*.h
	)
#file(GLOB_RECURSE REMOVE_HEADERS  
#  "src/SFML/Network/Win32/*.hpp"
#  "src/SFML/Window/iOS/*.hpp"
#)
#list(REMOVE_ITEM HEADERS ${REMOVE_HEADERS})

if(APPLE)
    add_definitions(-DHAVE_LIBC)
	if(IOS)
        add_definitions(-DHX_IOS)
        add_definitions(-D__IPHONEOS__)
        include_directories(include)
	else()
        add_definitions(-DHX_MACOS)
        include_directories(include)
	endif()
endif()

if(ANDROID)
        add_definitions(-DHAVE_LIBC)
        add_definitions(-DHX_ANDROID)
        add_definitions(-D__ANDROID__)
        include_directories(include)
endif()

if(MSVC)
        add_definitions(-DHX_WINDOWS)
        add_definitions(-D__WINDOWS__)
        include_directories(include)
endif()




include_directories(include)
include_directories(src)

# Define source files
set_source_files_properties (${SOURCES} PROPERTIES LANGUAGE C)

add_library(glew OBJECT ${HEADERS} ${SOURCES})

if (IOS)
    set_target_properties(
            glew
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
    )
endif(IOS)

