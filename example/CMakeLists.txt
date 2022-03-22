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
    else ()
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/mac)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/glew/include)
        # we're using lua instead of luajit on macos to be able to test with valgrind
        #include_directories(${CMAKE_SOURCE_DIR}/src/deps/luajit/src)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/newton)
    endif ()
endif ()

if (ANDROID)
    include_directories(${PROJECT_SOURCE_DIR}/src/deps/luajit/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
endif ()

if (WIN32)
    # we're using lua instead of luajit on macos to be able to test with valgrind
    #include_directories(${PROJECT_SOURCE_DIR}/src/deps/luajit/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/lua/src)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/windows)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/glew/include)
endif ()

if (EMSCRIPTEN)
    include_directories(${PROJECT_SOURCE_DIR}/src/deps/lua/src)
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

target_link_libraries(ExampleProject ${BINOCLE_LINK_LIBRARIES})

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
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
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
    if (NOT IOS)
        set(BINOCLE_ICON ${CMAKE_SOURCE_DIR}/cmake/Binocle.icns)
    endif ()
    set_source_files_properties(${BINOCLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(EXE_TYPE MACOSX_BUNDLE)
endif ()

if (ANDROID)
    message("Post build will copy to ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libmain.so")
    add_custom_command(TARGET ExampleProject
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ExampleProject> ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libmain.so)
#    message("Post build will copy to ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libhidapi.so")
#    add_custom_command(TARGET ExampleProject
#            POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:hidapi> ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libhidapi.so)
    add_custom_command(TARGET ExampleProject
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${ANDROID_NDK}/sources/cxx-stl/llvm-libc++/libs/${ANDROID_ABI}/libc++_shared.so ${CMAKE_CURRENT_BINARY_DIR}/../../platform/android/android-project/app/libs/${ANDROID_ABI}/libc++_shared.so)
    #	file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/../android-project/libs/${ANDROID_ABI}/lib${PROJECT_NAME}.so ${CMAKE_CURRENT_BINARY_DIR}/../android-project/libs/${ANDROID_ABI}/libmain.so)
endif ()

if(NOT MSVC)
    add_subdirectory(gameplay)
endif(NOT MSVC)
