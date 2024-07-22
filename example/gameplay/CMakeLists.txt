include_directories(${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/deps
        ${CMAKE_SOURCE_DIR}/src/deps/sdl/include
        ${CMAKE_SOURCE_DIR}/src/deps/sdl_mixer
)

        
file(GLOB SOURCE_FILES
        *.c
        *.h
        )

GroupSources("gameplay")

# TODO: make it shared only on the desktop and in debug mode
if(IOS OR ANDROID OR WATCHOS)
    add_library(gameplay STATIC ${SOURCE_FILES})
else()
    add_library(gameplay SHARED ${SOURCE_FILES})
endif(IOS OR ANDROID OR WATCHOS)

#target_link_libraries(gameplay ${BINOCLE_LINK_LIBRARIES})
target_link_libraries(gameplay binocle-static)

