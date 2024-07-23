#include (BundleUtilities)

set (CMAKE_SYSTEM_NAME iOS)

add_definitions (-DIOS -D__IPHONEOS__)
#add_definitions (-DBINOCLE_METAL)
#set(BINOCLE_METAL true)

add_definitions (-DBINOCLE_GL)
set(BINOCLE_GL true)
add_definitions (-DBINOCLE_GLES2)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -std=gnu++0x")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -D_DEBUG_ -D_DEBUG -g")

# force to use the Objective-C compiler even for .c files to fix an issue with miniaudio
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -x objective-c")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -D_DEBUG_ -D_DEBUG -g")

# Force unset of OS X-specific deployment target (otherwise autopopulated),
# required as of cmake 2.8.10.
set(CMAKE_OSX_DEPLOYMENT_TARGET "13.1" CACHE STRING
        "The deployment target." FORCE)

#set (CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD))
if(BINOCLE_IOS_ARCH)
    set (CMAKE_OSX_ARCHITECTURES ${BINOCLE_IOS_ARCH})
else()
    set (CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

set (CMAKE_XCODE_EFFECTIVE_PLATFORMS "-iphoneos;-iphonesimulator")
set (CMAKE_CONFIGURATION_TYPES Debug Release)

if(NOT BINOCLE_IOS_SDK)
    # Set Base SDK to "Latest iOS" for the device
    set (BINOCLE_IOS_SDK "iphoneos")
endif()

# Obtain iOS sysroot path
execute_process (COMMAND "xcodebuild" -version -sdk ${BINOCLE_IOS_SDK} Path
        OUTPUT_VARIABLE IOS_SYSROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE SYSROOT_RESULT)

message("iOS sysroot path found: ${IOS_SYSROOT}")
message("iOS sysroot search result: ${SYSROOT_RESULT}")
set (CMAKE_FIND_ROOT_PATH ${IOS_SYSROOT})
set (CMAKE_OSX_SYSROOT ${IOS_SYSROOT})

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework AudioToolbox -framework AVFoundation -framework CoreAudio -framework CoreBluetooth -framework CoreGraphics -framework CoreMotion -framework Foundation -framework GameController -framework Metal -framework MobileCoreServices -framework OpenGLES -framework QuartzCore -framework UIKit -framework MetalKit -framework CoreHaptics")

# globally silence the GLES deprecation warning
add_definitions(-DGLES_SILENCE_DEPRECATION)

set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH "YES")
# Needed for Xcode 12+ on Apple Silicon M1
set(CMAKE_XCODE_ATTRIBUTE_EXCLUDED_ARCHS[sdk=iphonesimulator*] arm64)

# set the build type to use
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Compile Type" FORCE)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

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

message("Building with architecture ${CMAKE_OSX_ARCHITECTURES}")

# Check https://ceres-solver.googlesource.com/ceres-solver/+/refs/heads/master/cmake/iOS.cmake for ideas