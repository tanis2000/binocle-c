include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)


file(GLOB CORE_SOURCE *.c *.h *.m)

set(SOURCE_FILES
        ${CORE_SOURCE}
        )


GroupSources("backend")

add_library(backend STATIC ${SOURCE_FILES})

