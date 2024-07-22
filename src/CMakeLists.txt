if (${BINOCLE_HTTP})
    message(STATUS "HTTP support enabled")
    add_definitions(-DBINOCLE_HTTP)
endif ()

add_subdirectory(deps)
add_subdirectory(binocle)
add_subdirectory(binocle/core/backend)

set(BINOCLE_STATIC_LIBS
        $<TARGET_OBJECTS:binocle>
        $<TARGET_OBJECTS:chipmunk>
        $<TARGET_OBJECTS:freetype>
        $<TARGET_OBJECTS:kazmath>
        $<TARGET_OBJECTS:parson>
        $<TARGET_OBJECTS:physfs>
        $<TARGET_OBJECTS:rxi_map>
        $<TARGET_OBJECTS:libuv>
        $<TARGET_OBJECTS:wren>
        )

if (NOT EMSCRIPTEN AND NOT ANDROID AND NOT IOS AND BINOCLE_HTTP)
    find_package(OpenSSL REQUIRED)
    set(OPENSSL_USE_STATIC_LIBS true)
endif ()

if (NOT EMSCRIPTEN)
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:SDL2::SDL2>
            $<TARGET_OBJECTS:ogg>
            $<TARGET_OBJECTS:vorbis>
            $<TARGET_OBJECTS:zlib>
            )
else()
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:lua>
            $<TARGET_OBJECTS:luasocket>
            )
endif()

if (APPLE AND NOT IOS)
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:glew>
            # we're using lua instead of luajit on macos to be able to test with valgrind
            #$<TARGET_OBJECTS:luajit-5.1>
#            $<TARGET_OBJECTS:lua>
#            $<TARGET_OBJECTS:newton>
            )
    if (BINOCLE_HTTP)
        set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
                $<TARGET_OBJECTS:curl>
                )
    endif ()
    if (BINOCLE_LUAJIT)
        set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
                $<TARGET_OBJECTS:libluajit>
                $<TARGET_OBJECTS:luasocket>
                )
    else ()
        set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
                $<TARGET_OBJECTS:lua>
                $<TARGET_OBJECTS:luasocket>
                )
    endif()
endif()

if (IOS)
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:lua>
            $<TARGET_OBJECTS:luasocket>
            )
endif()

if (WATCHOS)
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:lua>
            $<TARGET_OBJECTS:luasocket>
    )
endif()

if (ANDROID)
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:vorbisidec>
            #$<TARGET_OBJECTS:luajit-5.1>
            $<TARGET_OBJECTS:lua>
            $<TARGET_OBJECTS:luasocket>
            )
endif()

if (MSVC)
    set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
            $<TARGET_OBJECTS:glew>
            # we're using lua instead of luajit on macos to be able to test with valgrind
            #$<TARGET_OBJECTS:luajit-5.1>
            $<TARGET_OBJECTS:lua>
            $<TARGET_OBJECTS:luasocket>
            )
    if (BINOCLE_HTTP)
        set(BINOCLE_STATIC_LIBS ${BINOCLE_STATIC_LIBS}
                $<TARGET_OBJECTS:curl>
                )
    endif ()
endif()

message("List of static libraries for Binocle: ${BINOCLE_STATIC_LIBS}")

# This is required to work around an issue with the XCode generator that cannot include libraries with objects only.
# It requires at least one (empty) source file
# See: https://gitlab.kitware.com/cmake/cmake/issues/17457
file(GLOB SOURCE_FILES dummy.c)

add_library(binocle-static STATIC
        ${BINOCLE_STATIC_LIBS}
        ${SOURCE_FILES}
        )
