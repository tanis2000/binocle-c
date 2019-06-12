include (BundleUtilities)

add_definitions (-DIOS -D__IPHONEOS__)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-offsetof -std=gnu++0x")

set (CMAKE_OSX_ARCHITECTURES $(ARCHS_STANDARD))
#set (CMAKE_OSX_ARCHITECTURES "arm64 arm64e x86_64")
set (CMAKE_XCODE_EFFECTIVE_PLATFORMS -iphoneos -iphonesimulator)

# Set Base SDK to "Latest iOS"
set (CMAKE_OSX_SYSROOT iphoneos)

# Obtain iOS sysroot path
execute_process (COMMAND xcodebuild -version -sdk ${CMAKE_OSX_SYSROOT} Path OUTPUT_VARIABLE IOS_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)

set (CMAKE_FIND_ROOT_PATH ${IOS_SYSROOT})

set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework AudioToolbox -framework AVFoundation -framework CoreAudio -framework CoreGraphics -framework CoreMotion -framework Foundation -framework GameController -framework Metal -framework MobileCoreServices -framework OpenGLES -framework QuartzCore -framework UIKit")

# Check https://ceres-solver.googlesource.com/ceres-solver/+/refs/heads/master/cmake/iOS.cmake for ideas