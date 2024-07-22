project(luasocket)

include_directories(.)
include_directories(../lua/src)

file(GLOB SOURCES
        auxiliar.c
        buffer.c
        compat.c
        except.c
        inet.c
        io.c
        luasocket.c
        mime.c
        options.c
        select.c

        tcp.c
        timeout.c
        udp.c


)
set(SOURCES
        ${SOURCES}
)

file(GLOB HEADERS *.h)

if (APPLE OR IOS OR ANDROID OR EMSCRIPTEN)
    file(GLOB_RECURSE MACOS_SOURCES
            serial.c
            unix.c
            unixdgram.c
            unixstream.c
            usocket.c

    )
    list(APPEND SOURCES ${MACOS_SOURCES})

endif ()

if (MSVC)
    file(GLOB_RECURSE WIN32_SOURCES
            wsocket.c
    )
    list(APPEND SOURCES ${WIN32_SOURCES})

endif ()


add_library(luasocket OBJECT ${SOURCES})

if (IOS)
    set_target_properties(
            luasocket
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 13.1
    )
endif (IOS)
