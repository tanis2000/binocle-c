include_directories(.)

file(GLOB_RECURSE SOURCES *.c)
#file(GLOB_RECURSE REMOVE_SOURCES  
#  "jmemdos.c"
#  "jmemmac.c"
#)
#list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
#message(${SOURCES})
file(GLOB_RECURSE HEADERS *.h)


add_library(parson ${HEADERS} ${SOURCES})

if (IOS)
    set_target_properties(
            jsmn
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)
