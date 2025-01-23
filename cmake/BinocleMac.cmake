#include (BundleUtilities)

add_definitions (-D__APPLE__)
add_definitions (-DBINOCLE_MACOS)

add_definitions (-DBINOCLE_METAL)
set (BINOCLE_METAL true)

#add_definitions (-DBINOCLE_GL)
#set(BINOCLE_GL true)

list (APPEND BINOCLE_LINK_LIBRARIES "-l iconv")

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -std=gnu++0x")

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework AppKit -framework AudioToolbox -framework AudioUnit -framework Carbon -framework Cocoa -framework CoreAudio -framework CoreHaptics -framework CoreMotion -framework CoreVideo -framework ForceFeedback -framework GameController -framework IOKit -framework OpenGL -framework CoreServices -framework Security -framework Metal -framework MetalKit -framework UniformTypeIdentifiers")
set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -framework AppKit -framework AudioToolbox -framework AudioUnit -framework Carbon -framework Cocoa -framework CoreAudio -framework CoreHaptics -framework CoreMotion -framework CoreVideo -framework ForceFeedback -framework GameController -framework IOKit -framework OpenGL -framework CoreServices -framework Security -framework Metal -framework MetalKit -framework UniformTypeIdentifiers")
#		set(EXTRA_LIBRARIES "${EXTRA_LIBRARIES}  -framework IOKit -framework AppKit -framework GLUT -framework GLKit -framework OpenGL -framework AudioToolbox -framework OpenAL -framework CoreAudio -framework AudioUnit -framework QuartzCore -framework CoreGraphics -framework CoreServices -framework ForceFeedback")

set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)

set(XCODE_IOS_PLATFORM macosx)

# If user did not specify the SDK root to use, then query xcodebuild for it.
if (NOT CMAKE_OSX_SYSROOT)
    execute_process(COMMAND xcodebuild -version -sdk ${XCODE_IOS_PLATFORM} Path
            OUTPUT_VARIABLE CMAKE_OSX_SYSROOT
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    message("Using SDK: ${CMAKE_OSX_SYSROOT} for platform: ${IOS_PLATFORM}")
endif()
if (NOT EXISTS ${CMAKE_OSX_SYSROOT})
    message(FATAL_ERROR "Invalid CMAKE_OSX_SYSROOT: ${CMAKE_OSX_SYSROOT} "
            "does not exist.")
endif()

# Find (Apple's) libtool.
message("Looking for libtool in ${CMAKE_OSX_SYSROOT}")
execute_process(COMMAND xcrun -sdk ${CMAKE_OSX_SYSROOT} -find libtool
        OUTPUT_VARIABLE IOS_LIBTOOL
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE)
message("Using libtool: ${IOS_LIBTOOL}")
# Configure libtool to be used instead of ar + ranlib to build static libraries.
# This is required on Xcode 7+, but should also work on previous versions of
# Xcode.
set(CMAKE_C_CREATE_STATIC_LIBRARY
        "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")
set(CMAKE_CXX_CREATE_STATIC_LIBRARY
        "${IOS_LIBTOOL} -static -o <TARGET> <LINK_FLAGS> <OBJECTS> ")

