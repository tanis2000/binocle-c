project(zlib)

include_directories(.)

file(GLOB_RECURSE SOURCES *.c)
#file(GLOB_RECURSE REMOVE_SOURCES
#  "jmemdos.c"
#  "jmemmac.c"
#)
#list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
#message(${SOURCES})
file(GLOB_RECURSE HEADERS *.h)

add_definitions(-DNATIVE_TOOLKIT_ZLIB)

add_library(zlib OBJECT ${SOURCES})

if (APPLE)
    add_definitions(-D_LARGEFILE64_SOURCE=1)
endif ()

if (EMSCRIPTEN)
    add_definitions(-D_LARGEFILE64_SOURCE=1)
endif ()

if (ANDROID)
    add_definitions(-D_LARGEFILE64_SOURCE=1)
endif ()

if (IOS)
    set_target_properties(
            zlib
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 13.1
    )
endif(IOS)
