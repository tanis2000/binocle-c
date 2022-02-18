project(chipmunk)

include_directories(include)

file(GLOB SOURCES
        src/*.c
        )

set(SOURCES
        ${SOURCES}
        )


file(GLOB_RECURSE HEADERS
        include/*.h
        )

# Define source files
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)

add_library(chipmunk OBJECT ${SOURCES})

if (IOS)
    set_target_properties(
            chipmunk
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
    )
endif (IOS)


