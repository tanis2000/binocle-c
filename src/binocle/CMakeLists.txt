include_directories(${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/deps
        ${CMAKE_SOURCE_DIR}/src/deps/glew
        ${CMAKE_SOURCE_DIR}/src/deps/freetype
        ${CMAKE_SOURCE_DIR}/src/deps/miniaudio
        ${CMAKE_SOURCE_DIR}/src/deps/ogg
        ${CMAKE_SOURCE_DIR}/src/deps/zlib
        ${CMAKE_SOURCE_DIR}/src/deps/vorbis
        ${CMAKE_SOURCE_DIR}/src/deps/sdl/include
        ${CMAKE_SOURCE_DIR}/src/deps/sdl_mixer
        ${CMAKE_SOURCE_DIR}/src/deps/stb_image
        ${CMAKE_SOURCE_DIR}/src/deps/kazmath
        ${CMAKE_SOURCE_DIR}/src/deps/physfs
        ${CMAKE_SOURCE_DIR}/src/deps/sokol
        ${CMAKE_SOURCE_DIR}/src/deps/libuv
        ${CMAKE_SOURCE_DIR}/src/deps/wren
        )

if (EMSCRIPTEN)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
endif()

if (APPLE)
    if (IOS)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
    else()
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
    endif()
endif()

if (ANDROID)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
endif()

if (WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
endif()



file(GLOB CORE_SOURCE
        core/*.h
        core/*.c

        core/backend/binocle_backend.h
        core/backend/binocle_backend_types.h
        core/backend/binocle_blend.h
        core/backend/binocle_color.h
        core/backend/binocle_material.h
        core/backend/binocle_sampler_cache.h
        core/backend/binocle_vpct.h

        core/backend/binocle_blend.c
        core/backend/binocle_color.c
        core/backend/binocle_material.c
        core/backend/binocle_sampler_cache.c
        core/backend/binocle_vpct.c

        core/backend/binocle_backend.c
        )
file(GLOB REMOVE_CORE_SOURCE
        "core/binocle.c"
        )
list(REMOVE_ITEM CORE_SOURCE ${REMOVE_CORE_SOURCE})
file(GLOB UNITY_SOURCE core/binocle.c core/binocle.h)

set(GL_SOURCE
        core/backend/binocle_backend_gl.h
        core/backend/binocle_backend_gl.c
        )

set(METAL_SOURCE
        core/backend/binocle_backend_metal.h
        core/backend/binocle_backend_metal.m
        )

set(CORE_SOURCE ${CORE_SOURCE} core/binocle_miniaudio.c)

if(BINOCLE_GL)
    set(CORE_SOURCE ${CORE_SOURCE} ${GL_SOURCE})
endif()

if(BINOCLE_METAL)
    set(CORE_SOURCE ${CORE_SOURCE} ${METAL_SOURCE})
endif()

if (APPLE)
    set(PLATFORM_SOURCE core/PlayerPrefs.mm)
endif ()

set(SOURCE_FILES
        ${CORE_SOURCE}
        ${PLATFORM_SOURCE}
        )

set(UNITY_FILES
        ${UNITY_SOURCE}
        ${PLATFORM_SOURCE}
        )

GroupSources("core")

# Uncomment the following line to avoid unity builds
add_library(binocle OBJECT ${SOURCE_FILES})
# Uncomment the following line to use unity builds
#add_library(binocle ${UNITY_FILES})

if (IOS)
    set_target_properties(
            binocle
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
    )
endif(IOS)

message("${CORE_SOURCE}")