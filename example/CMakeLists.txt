if (${BINOCLE_HTTP})
    message(STATUS "HTTP support enabled")
    add_definitions(-DBINOCLE_HTTP)
endif ()

if (NOT BINOCLE_DATA_DIR)
    message("Setting BINOCLE_DATA_DIR to the default value")
    set(BINOCLE_DATA_DIR "${CMAKE_SOURCE_DIR}/assets/")
endif ()

message("Binocle data dir: ${BINOCLE_DATA_DIR}")
message("Project source dir: ${PROJECT_SOURCE_DIR}")

include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(
        ${CMAKE_SOURCE_DIR}/src/deps
        ${CMAKE_SOURCE_DIR}/src/deps/chipmunk/include
        ${CMAKE_SOURCE_DIR}/src/deps/sdl/include
        ${CMAKE_SOURCE_DIR}/src/deps/sdl_mixer
        ${CMAKE_SOURCE_DIR}/src/deps/physfs
        ${CMAKE_SOURCE_DIR}/src/deps/sokol
        ${CMAKE_SOURCE_DIR}/src/binocle/core
        ${CMAKE_SOURCE_DIR}/src/example/gameplay
)

if (APPLE)
    if (IOS)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
        include_directories(${PROJECT_SOURCE_DIR}/src/deps/lua/src)
        include_directories(${PROJECT_SOURCE_DIR}/src/deps/luasocket/src)
    elseif (WATCHOS)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
        include_directories(${PROJECT_SOURCE_DIR}/src/deps/lua/src)
        include_directories(${PROJECT_SOURCE_DIR}/src/deps/luasocket/src)
    else ()
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/mac)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/glew/include)
        # we're using lua instead of luajit on macos to be able to test with valgrind
        #include_directories(${CMAKE_SOURCE_DIR}/src/deps/luajit/src)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/luasocket/src)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/newton)
    endif ()
endif ()

if (ANDROID)
    include_directories(${PROJECT_SOURCE_DIR}/src/deps/luajit/src)
    include_directories(${PROJECT_SOURCE_DIR}/src/deps/luasocket/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
endif ()

if (WIN32)
    # we're using lua instead of luajit on macos to be able to test with valgrind
    #include_directories(${PROJECT_SOURCE_DIR}/src/deps/luajit/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/luasocket/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/windows)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/glew/include)
endif ()

if (EMSCRIPTEN)
    include_directories(${PROJECT_SOURCE_DIR}/src/deps/lua/src)
    include_directories(${PROJECT_SOURCE_DIR}/src/deps/luasocket/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
endif ()

file(GLOB SOURCE_FILES
        *.c
        *.h
        )

if (APPLE)
    #if(IOS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    file(GLOB_RECURSE RESOURCES "${CMAKE_SOURCE_DIR}/assets/*")
    foreach(RES_FILE ${RESOURCES})
        message("RES_FILE: ${RES_FILE}")
        file(RELATIVE_PATH RES_PATH "${CMAKE_SOURCE_DIR}/assets" ${RES_FILE})
        message("RES_PATH: ${RES_PATH}")
        get_filename_component(RES_FOLDER ${RES_PATH} DIRECTORY)
        message("RES_FOLDER: ${RES_FOLDER}")
        set_property(SOURCE ${RES_FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${RES_FOLDER}")
    endforeach(RES_FILE)
    #set_property(SOURCE ${RESOURCES} PROPERTY MACOSX_PACKAGE_LOCATION "Resources")
    message("Resources list: ${RESOURCES}")
    #endif()
endif ()

if (ANDROID)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include)
    add_library(ExampleProject SHARED ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/deps/sdl/src/main/android/SDL_android_main.c)
else ()
    add_executable(ExampleProject MACOSX_BUNDLE ${EXE_TYPE} ${SOURCE_FILES} ${BINOCLE_ICON} ${RESOURCES})
endif ()

if (NOT EMSCRIPTEN AND NOT ANDROID AND NOT IOS AND NOT WATCHOS)
    set(OPENSSL_USE_STATIC_LIBS TRUE)
    set(OPENSSL_MSVC_STATIC_RT TRUE)
    find_package(OpenSSL REQUIRED)
    message("OpenSSL Libraries: ${OPENSSL_LIBRARIES}")
    message("OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    if (MSVC)
        target_link_libraries(ExampleProject ${BINOCLE_LINK_LIBRARIES} ${OPENSSL_LIBRARIES} wldap32 crypt32 Ws2_32)
        message(STATUS "Target link libraries: ${BINOCLE_LINK_LIBRARIES} ${OPENSSL_LIBRARIES} wldap32 crypt32 Ws2_32")
    else()
        target_link_libraries(ExampleProject ${BINOCLE_LINK_LIBRARIES} ${OPENSSL_LIBRARIES})
        message(STATUS "Target link libraries: ${BINOCLE_LINK_LIBRARIES} ${OPENSSL_LIBRARIES}")
    endif()
else ()
    target_link_libraries(ExampleProject ${BINOCLE_LINK_LIBRARIES})
endif ()

set_property(TARGET ExampleProject PROPERTY C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")


if (APPLE)
    set_target_properties(ExampleProject PROPERTIES XCODE_ATTRIBUTE_PRODUCT_NAME "ExampleProject")
endif ()

# Build the shared library next to sdl-endless-runner
set_target_properties(ExampleProject PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_BINARY_DIR}/src
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_BINARY_DIR}/src
        XCODE_ATTRIBUTE_PRODUCT_NAME "ExampleProject"
        XCODE_ATTRIBUTE_ENABLE_BITCODE "NO"
        )

if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)

    set_target_properties(ExampleProject PROPERTIES
            MACOSX_RPATH 1
            BUILD_WITH_INSTALL_RPATH 1
            INSTALL_RPATH "@loader_path/../Frameworks"
            MACOSX_BUNDLE_GUI_IDENTIFIER "it.altralogica.exampleproject"
            )
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    #set_target_properties(${PROJECT_NAME} PROPERTIES RESOURCE "${RESOURCES}")
    # file(GLOB_RECURSE RESOURCES
    # 			${CMAKE_SOURCE_DIR}/assets/*
    # )
    # set_source_files_properties(
    # 			${RESOURCES}
    # 			PROPERTIES
    # 			MACOSX_PACKAGE_LOCATION Resources
    # )
#    set_source_files_properties(
#            ${RESOURCES}
#            PROPERTIES
#            MACOSX_PACKAGE_LOCATION Resources)
    if (IOS)
        message("Using plist file: ${CMAKE_SOURCE_DIR}/platform/ios/Info.plist")
        set_target_properties(
                ${PROJECT_NAME}
                PROPERTIES
                MACOSX_BUNDLE YES
                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/platform/ios/Info.plist"
                XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iOS Developer"
                XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "Automatic"
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${IOS_DEVELOPMENT_TEAM_ID}"
                XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 13.1
                RESOURCE "${RESOURCES}"
        )

        # Copy resources
        #add_custom_command(
        #    TARGET ${NAME}
        #    POST_BUILD
        #    COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -resolve-src-symlinks ${CMAKE_SOURCE_DIR}/assets ${APP_NAME}
        #)
    endif ()

    if (WATCHOS)
        message("Using plist file: ${CMAKE_SOURCE_DIR}/platform/ios/Info.plist")
        set_target_properties(
                ${PROJECT_NAME}
                PROPERTIES
                MACOSX_BUNDLE YES
                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/platform/ios/Info.plist"
                XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iOS Developer"
                XCODE_ATTRIBUTE_PROVISIONING_PROFILE_SPECIFIER "Automatic"
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${IOS_DEVELOPMENT_TEAM_ID}"
                XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                XCODE_ATTRIBUTE_WATCHOS_DEPLOYMENT_TARGET 7.0
                RESOURCE "${RESOURCES}"
        )

        # Copy resources
        #add_custom_command(
        #    TARGET ${NAME}
        #    POST_BUILD
        #    COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -resolve-src-symlinks ${CMAKE_SOURCE_DIR}/assets ${APP_NAME}
        #)
    endif ()
endif ()


if (MSVC)
    set(EXE_TYPE WIN32)
    #    set (SOURCE_FILES ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/cmake/Binocle.rc)
elseif (APPLE)
    #ICNS
    set(MACOSX_BUNDLE_ICON_FILE ExampleProject.icns)
    if (NOT IOS AND NOT WATCHOS)
        set(BINOCLE_ICON ${CMAKE_SOURCE_DIR}/cmake/Binocle.icns)
    endif ()
    set_source_files_properties(${BINOCLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(EXE_TYPE MACOSX_BUNDLE)
endif ()

if (MSVC)
    set(EXE_TYPE WIN32)
    #    set (SOURCE_FILES ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/build/cmake/Binocle.rc)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    file(GLOB_RECURSE RESOURCES "${BINOCLE_DATA_DIR}*")
    foreach(RES_FILE ${RESOURCES})
        message("RES_FILE: ${RES_FILE}")
        file(RELATIVE_PATH RES_PATH "${BINOCLE_DATA_DIR}" ${RES_FILE})
        message("RES_PATH: ${RES_PATH}")
        get_filename_component(RES_FOLDER ${RES_PATH} DIRECTORY)
        message("RES_FOLDER: ${RES_FOLDER}")
        add_custom_command(TARGET ExampleProject
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${RES_FILE} ${CMAKE_CURRENT_BINARY_DIR}/src/assets/${RES_PATH}
    )
    endforeach(RES_FILE)
elseif (APPLE)
    #ICNS
    set(MACOSX_BUNDLE_ICON_FILE ExampleProject.icns)
    if (NOT IOS AND NOT WATCHOS)
        set(BINOCLE_ICON ${CMAKE_SOURCE_DIR}/build/cmake/Binocle.icns)
    endif ()
    set_source_files_properties(${BINOCLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(EXE_TYPE MACOSX_BUNDLE)
endif ()

if (ANDROID)
    message("Post build will copy to ${CMAKE_CURRENT_BINARY_DIR}/../../../platform/android/android-project/app/libs/${ANDROID_ABI}/libmain.so")
    add_custom_command(TARGET ExampleProject
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ExampleProject> ${CMAKE_CURRENT_BINARY_DIR}/../../../platform/android/android-project/app/libs/${ANDROID_ABI}/libmain.so)
#    message("Post build will copy to ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libhidapi.so")
#    add_custom_command(TARGET ExampleProject
#            POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hidapi> ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libhidapi.so)
    message("android abi ${ANDROID_ABI}")
    message("android toolchain ${ANDROID_TOOLCHAIN_NAME}")
    if (ANDROID_ABI STREQUAL x86_64)
        set(ANDROID_LLVM_TOOLCHAIN_ARCH x86_64-linux-android)
    elseif(ANDROID_ABI STREQUAL "x86")
        set(ANDROID_LLVM_TOOLCHAIN_ARCH i686-linux-android)
    elseif(ANDROID_ABI STREQUAL "armeabi-v7a")
        set(ANDROID_LLVM_TOOLCHAIN_ARCH arm-linux-androideabi)
    elseif(ANDROID_ABI STREQUAL "arm64-v8a")
        set(ANDROID_LLVM_TOOLCHAIN_ARCH aarch64-linux-android)
    endif()
    message("android arch ${ANDROID_LLVM_TOOLCHAIN_ARCH}")

    add_custom_command(TARGET ExampleProject
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${ANDROID_NDK}/toolchains/llvm/prebuilt/darwin-x86_64/sysroot/usr/lib/${ANDROID_LLVM_TOOLCHAIN_ARCH}/libc++_shared.so ${CMAKE_CURRENT_BINARY_DIR}/../../../platform/android/android-project/app/libs/${ANDROID_ABI}/libc++_shared.so)
    #	file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/../android-project/libs/${ANDROID_ABI}/lib${PROJECT_NAME}.so ${CMAKE_CURRENT_BINARY_DIR}/../android-project/libs/${ANDROID_ABI}/libmain.so)
endif ()

if(NOT MSVC)
    add_subdirectory(gameplay)
endif(NOT MSVC)

# Copy OpenSSL shared libraries
#if (APPLE)
#    add_custom_command ( TARGET ExampleProject POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            ${OPENSSL_LIBRARIES} $<TARGET_FILE_DIR:ExampleProject>
#            )
#endif ()

if (APPLE)
    if (NOT IOS AND NOT WATCHOS)
    add_custom_command ( TARGET ExampleProject POST_BUILD
            COMMAND codesign -s - -f --entitlements "${CMAKE_SOURCE_DIR}/platform/macosx/entitlements.plist"
            $<TARGET_FILE:ExampleProject>
            )
    endif ()
endif ()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        "${CMAKE_SOURCE_DIR}/assets/shaders/src/*.glsl"
)

set(SHADER_COMPILER "missing-sokol-shdc-for-this-host-architecture")
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    set(SHADER_COMPILER "osx/sokol-shdc")
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
    set(SHADER_COMPILER "win32/sokol-shdc.exe")
elseif (CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
    set(SHADER_COMPILER "linux/sokol-shdc")
endif ()
message("Using shader compiler ${SHADER_COMPILER}")

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV_GL33 "${CMAKE_SOURCE_DIR}/assets/shaders/dst/gl33/${FILE_NAME}")
    set(SPIRV_GLES "${CMAKE_SOURCE_DIR}/assets/shaders/dst/gles/${FILE_NAME}")
    set(SPIRV_HLSL "${CMAKE_SOURCE_DIR}/assets/shaders/dst/hlsl/${FILE_NAME}")
    set(SPIRV_METAL_MACOS "${CMAKE_SOURCE_DIR}/assets/shaders/dst/metal-macos/${FILE_NAME}")
    set(SPIRV_METAL_IOS "${CMAKE_SOURCE_DIR}/assets/shaders/dst/metal-ios/${FILE_NAME}")
    message("Compiling shader ${GLSL} to ${SPIRV_GL33}")
    message("Compiling shader ${GLSL} to ${SPIRV_GLES}")
    message("Compiling shader ${GLSL} to ${SPIRV_HLSL}")
    message("Compiling shader ${GLSL} to ${SPIRV_METAL}")
    add_custom_command(
            OUTPUT ${SPIRV_GL33}
            OUTPUT ${SPIRV_GLES}
            OUTPUT ${SPIRV_HLSL}
            OUTPUT ${SPIRV_METAL_MACOS}
            OUTPUT ${SPIRV_METAL_IOS}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/dst"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/dst/gl33"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/dst/gles"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/dst/hlsl"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/dst/metal-macos"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/assets/shaders/dst/metal-ios"
            COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/${SHADER_COMPILER}"
            ARGS "--input" "${GLSL}" "--output" "${SPIRV_GL33}" "--slang" "glsl410" "-f" "bare"
            COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/${SHADER_COMPILER}"
            # Enable the following to get a C output of the shaders for debugging purposes
#            ARGS "--input" "${GLSL}" "--output" "${SPIRV_GL33}" "--slang" "glsl330" "-f" "sokol"
#            COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/${SHADER_COMPILER}"
            ARGS "--input" "${GLSL}" "--output" "${SPIRV_GLES}" "--slang" "glsl300es" "-f" "bare"
            COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/${SHADER_COMPILER}"
            ARGS "--input" "${GLSL}" "--output" "${SPIRV_HLSL}" "--slang" "hlsl4" "-f" "bare"
            COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/${SHADER_COMPILER}"
            ARGS "--input" "${GLSL}" "--output" "${SPIRV_METAL_MACOS}" "--slang" "metal_macos" "-f" "bare"
            COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/${SHADER_COMPILER}"
            ARGS "--input" "${GLSL}" "--output" "${SPIRV_METAL_IOS}" "--slang" "metal_ios" "-f" "bare"
    )
    list(APPEND SPIRV_GENERATED_FILES ${SPIRV_GL33} ${SPIRV_GLES} ${SPIRV_HLSL} ${SPIRV_METAL})
endforeach(GLSL)

add_custom_target(
        Shaders
        DEPENDS ${SPIRV_GENERATED_FILES}
)

add_dependencies(ExampleProject Shaders)

#if (APPLE)
#    if (NOT IOS)
#        add_custom_command(
##                OUTPUT shader_output
#                TARGET ExampleProject POST_BUILD
#                COMMAND "${CMAKE_SOURCE_DIR}/tools/sokol-shdc/osx_arm64/sokol-shdc"
#                ARGS "--input" "${CMAKE_SOURCE_DIR}/assets/shaders/src/default.glsl" "--output" "${CMAKE_SOURCE_DIR}/assets/shaders/dst/gl33/" "--slang" "glsl330" "-f" "bare"
#        )
#    endif ()
#endif ()