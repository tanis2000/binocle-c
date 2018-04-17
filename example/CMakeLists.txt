include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(
        ${CMAKE_SOURCE_DIR}/src/deps
        ${CMAKE_SOURCE_DIR}/src/deps/sdl/include
        ${CMAKE_SOURCE_DIR}/src/deps/sdl_mixer
        ${CMAKE_SOURCE_DIR}/src/binocle/core
        ${CMAKE_SOURCE_DIR}/src/example/gameplay
)

if (APPLE)
    if (IOS)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
    else ()
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/mac)
        include_directories(${CMAKE_SOURCE_DIR}/src/deps/glew/include)
    endif ()
endif ()

if (ANDROID)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
endif ()

if (WIN32)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/windows)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/glew/include)
endif ()

if (EMSCRIPTEN)
    include_directories(${CMAKE_SOURCE_DIR}/src/deps/sdl/include/configs/default)
endif ()

file(GLOB SOURCE_FILES
        *.c
        *.h
        )


if (NOT DEFINED BINOCLE_DATA_DIR)
    if (APPLE)
        if (IOS)
            set(BINOCLE_DATA_DIR "")
        else ()
            set(BINOCLE_DATA_DIR "../Resources/")
        endif ()
    elseif (WIN32)
        set(BINOCLE_DATA_DIR "../../../../../../data/")
    elseif (EMSCRIPTEN)
        # TODO: make this relative but also make sure that it
        # becomes relative in BinocleWeb.cmake as they have to be the same
        set(BINOCLE_DATA_DIR "/Users/tanis/Documents/binocle/data/")
    endif ()
endif ()
message("Data dir is ${BINOCLE_DATA_DIR}")


CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/example/sys_config.h.cmake ${CMAKE_SOURCE_DIR}/example/sys_config.h)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_SOURCE_DIR}/example/sys_config.h PROPERTIES GENERATED TRUE)

if (APPLE)
    #if(IOS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    file(GLOB RESOURCES_LIST ${CMAKE_SOURCE_DIR}/data/*)
    set(RESOURCES
            ${RESOURCES_LIST}
            )
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
    # 			${CMAKE_SOURCE_DIR}/data/*
    # )
    # set_source_files_properties(
    # 			${RESOURCES}
    # 			PROPERTIES
    # 			MACOSX_PACKAGE_LOCATION Resources
    # )
    set_source_files_properties(
            ${RESOURCES}
            PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources)
    if (IOS)
        set_target_properties(
                ${PROJECT_NAME}
                PROPERTIES
                MACOSX_BUNDLE YES
                MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/build/ios/Info.plist"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
                XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
                RESOURCE "${RESOURCES}"
        )

        # Copy resources
        #add_custom_command(
        #    TARGET ${NAME}
        #    POST_BUILD
        #    COMMAND /Developer/Library/PrivateFrameworks/DevToolsCore.framework/Resources/pbxcp -exclude .DS_Store -exclude CVS -exclude .svn -resolve-src-symlinks ${CMAKE_SOURCE_DIR}/data ${APP_NAME}
        #)
    endif ()
endif ()


if (MSVC)
    set(EXE_TYPE WIN32)
    #    set (SOURCE_FILES ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/build/cmake/Binocle.rc)
elseif (APPLE)
    #ICNS
    set(MACOSX_BUNDLE_ICON_FILE ExampleProject.icns)
    if (NOT IOS)
        set(BINOCLE_ICON ${CMAKE_SOURCE_DIR}/build/cmake/Binocle.icns)
    endif ()
    set_source_files_properties(${BINOCLE_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set(EXE_TYPE MACOSX_BUNDLE)
endif ()

if (ANDROID)
    message("Post build will copy to ${CMAKE_CURRENT_BINARY_DIR}/../../../android-project/libs/${ANDROID_ABI}/libmain.so")
    add_custom_command(TARGET ExampleProject
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "$<TARGET_FILE:ExampleProject> ${CMAKE_CURRENT_BINARY_DIR}/../../../android-project/libs/${ANDROID_ABI}/libmain.so")
    add_custom_command(TARGET ExampleProject
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ExampleProject> ${CMAKE_CURRENT_BINARY_DIR}/../../../android-project/libs/${ANDROID_ABI}/libmain.so)
    #	file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/../android-project/libs/${ANDROID_ABI}/lib${PROJECT_NAME}.so ${CMAKE_CURRENT_BINARY_DIR}/../android-project/libs/${ANDROID_ABI}/libmain.so)
endif ()

add_subdirectory(gameplay)