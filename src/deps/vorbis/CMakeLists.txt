project(vorbis)

include_directories(.)

file(GLOB_RECURSE SOURCES lib/*.c)
file(GLOB_RECURSE REMOVE_SOURCES  
  "lib/psytune.c"
  "lib/barkmel.c"
  "lib/tone.c"
  "lib/lookup.c"
)
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
#message(${SOURCES})
file(GLOB_RECURSE HEADERS include/vorbis/*.h lib/*.h)
include_directories(include)
include_directories(lib)
include_directories(../ogg/include)

if (NOT WIN32)
	add_definitions(-DHAVE_ALLOCA_H)
endif()

add_library(vorbis OBJECT ${HEADERS} ${SOURCES})

if (IOS)
    set_target_properties(
            vorbis
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 13.1
    )
endif(IOS)

