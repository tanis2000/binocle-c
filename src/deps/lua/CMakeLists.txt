project(lua)

include_directories(.)
include_directories(src)

file(GLOB_RECURSE SOURCES src/*.c)
file(GLOB_RECURSE REMOVE_SOURCES
  "src/lua.c"
  "src/luac.c"
)
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
#message(${SOURCES})
file(GLOB_RECURSE HEADERS src/*.h)

if (IOS)
    add_definitions(-DLUA_USE_MKSTEMP)
#    file(GLOB_RECURSE REMOVE_SOURCES
#            "src/loslib.c"
#            )
#    list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
endif()

add_library(lua OBJECT ${HEADERS} ${SOURCES})

if (IOS)
    set_target_properties(
            lua
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)
