project(luajit-static)

SET(LUAJIT_LIBRARIES luajit-5.1)
SET(WITH_SHARED_LUA OFF)
SET(WITH_OBJECT_LUA ON)
#SET(WITH_AMALG OFF)
if(APPLE)
    #SET(LUAJIT_ENABLE_GC64 ON)
endif()

include(luajit.cmake)

include_directories(src)

