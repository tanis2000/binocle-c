project(rxi_map)

include_directories(.)

file(GLOB_RECURSE SOURCES *.c)
#file(GLOB_RECURSE REMOVE_SOURCES  
#  "jmemdos.c"
#  "jmemmac.c"
#)
#list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
#message(${SOURCES})
file(GLOB_RECURSE HEADERS *.h)


add_library(rxi_map OBJECT ${HEADERS} ${SOURCES})

if (IOS)
    set_target_properties(
            rxi_map
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 13.1
    )
endif(IOS)
