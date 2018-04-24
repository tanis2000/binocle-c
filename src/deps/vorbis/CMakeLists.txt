
include_directories(.)

file(GLOB_RECURSE SOURCES lib/*.c)
file(GLOB_RECURSE REMOVE_SOURCES  
  "lib/psytune.c"
  "lib/barkmel.c"
  "lib/tone.c"
)
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
#message(${SOURCES})
file(GLOB_RECURSE HEADERS include/vorbis/*.h)
include_directories(include)
include_directories(../ogg/include)


add_library(vorbis ${HEADERS} ${SOURCES})

if (IOS)
    set_target_properties(
            vorbis
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)

