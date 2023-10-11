# SDL 2.0.12
# TODO: check iOS, Android, Windows, Emscripten changes from previous version
project(sdl)

include_directories(include)
include_directories(src/hidapi/hidapi)

file(GLOB SOURCES
        src/*.c
        src/atomic/*.c
        src/audio/*.c
        src/audio/dummy/*.c
        src/audio/disk/*.c
        src/cpuinfo/*.c
        src/dynapi/*.c
        src/events/*.c
        src/file/*.c
        src/haptic/*.c
        src/hidapi/*.c
        src/libm/*.c
        src/locale/*.c
        src/misc/*.c
        src/power/*.c
        src/render/*.c
        src/render/*/*.c
        src/sensor/*.c
        src/sensor/dummy/*.c
        src/stdlib/*.c
        src/thread/*.c
        src/timer/*.c
        src/video/*.c
        src/video/dummy/*.c
        src/video/offscreen/*.c
        src/video/yuv2rgb/*.c
        )

set(SOURCES
        ${SOURCES}
        )


file(GLOB_RECURSE HEADERS
        include/*.h
        )

if (BINOCLE_GL)
    file(GLOB_RECURSE GL_SOURCES
            src/render/opengl/SDL_render_gl.c
            src/render/opengl/SDL_shaders_gl.c
            )
    list(APPEND SOURCES ${GL_SOURCES})
elseif (BINOCLE_METAL)
    file(GLOB_RECURSE METAL_SOURCES
            src/render/metal/SDL_render_metal.m
            )
    list(APPEND SOURCES ${METAL_SOURCES})
endif (BINOCLE_GL)

if (APPLE)
    file(GLOB MFI_JOYSTICK_SOURCES src/joystick/iphoneos/*.m)
    if (IOS)
        file(GLOB IOS_SOURCES
                src/audio/coreaudio/*.m
                src/file/cocoa/*.m
                src/filesystem/cocoa/*.m
                src/joystick/steam/*.c
                src/haptic/dummy/*.c
                src/hidapi/ios/hid.m
                src/loadso/dlopen/*.c
                src/locale/macosx/*.m
                src/main/uikit/*.m
                src/misc/ios/*.m
                src/power/uikit/*.m
                src/render/opengles/SDL_render_gles.c
                src/render/opengles2/SDL_render_gles2.c
                src/render/opengles2/SDL_shaders_gles2.c
                src/render/metal/SDL_render_metal.m
                src/sensor/coremotion/*.m
                src/thread/pthread/SDL_syscond.c
                src/thread/pthread/SDL_sysmutex.c
                src/thread/pthread/SDL_syssem.c
                src/thread/pthread/SDL_systhread.c
                src/thread/pthread/SDL_systls.c
                src/timer/unix/*.c
                src/power/uikit/SDL_syspower.m
                src/video/SDL_egl.c
                src/video/cocoa/SDL_cocoaopengles.m
                src/video/cocoa/SDL_cocoavulkan.m
                src/video/uikit/*.m

                #include/*.h
                #include/SDL_config_iphoneos.h
                )
        list(APPEND SOURCES ${IOS_SOURCES} ${MFI_JOYSTICK_SOURCES})
        add_definitions(-DSFML_SYSTEM_IOS)
        add_definitions(-DSFML_OPENGL_ES)
        add_definitions(-DGL_GLEXT_PROTOTYPES)
        add_definitions(-fobjc-arc)
        add_definitions(-DHX_IOS)
        add_definitions(-D__IPHONEOS__)
        include_directories(include)
        include_directories(include/configs/default)
    else ()
        file(GLOB MAC_SOURCES
                src/audio/coreaudio/*.m
                src/core/unix/SDL_poll.c
                src/file/cocoa/*.m
                src/filesystem/cocoa/*.m
                src/haptic/darwin/*.c
                src/joystick/*.c
                src/joystick/darwin/*.c
                src/joystick/hidapi/*.c
                src/joystick/iphoneos/*.m
                src/joystick/virtual/*.c
                src/loadso/dlopen/*.c
                src/locale/macosx/*.m
                src/misc/macosx/*.m
                src/power/macosx/*.c
                src/sensor/coremotion/*.m
                src/thread/pthread/SDL_syscond.c
                src/thread/pthread/SDL_sysmutex.c
                src/thread/pthread/SDL_syssem.c
                src/thread/pthread/SDL_systhread.c
                src/thread/pthread/SDL_systls.c
                src/timer/unix/*.c
                src/video/SDL_egl.c
                src/video/cocoa/SDL_cocoaclipboard.m
                src/video/cocoa/SDL_cocoaevents.m
                src/video/cocoa/SDL_cocoakeyboard.m
                src/video/cocoa/SDL_cocoamessagebox.m
                src/video/cocoa/SDL_cocoametalview.m
                src/video/cocoa/SDL_cocoamodes.m
                src/video/cocoa/SDL_cocoamouse.m
                src/video/cocoa/SDL_cocoaopengl.m
                src/video/cocoa/SDL_cocoaopengles.m
                src/video/cocoa/SDL_cocoashape.m
                src/video/cocoa/SDL_cocoavideo.m
                src/video/cocoa/SDL_cocoavulkan.m
                src/video/cocoa/SDL_cocoawindow.m

                #include/SDL_config_macosx.h
                )
        set_source_files_properties(${MFI_JOYSTICK_SOURCES} PROPERTIES COMPILE_FLAGS -fobjc-weak)
        set_source_files_properties(${MFI_JOYSTICK_SOURCES} PROPERTIES LANGUAGE C)
        list(APPEND SOURCES ${MAC_SOURCES})

        if (BINOCLE_GL)
            file(GLOB MAC_GL_SOURCES
                    #					src/video/SDL_egl.c
                    #					src/video/cocoa/SDL_cocoaopengl.m
                    )
            list(APPEND SOURCES ${MAC_GL_SOURCES})
            add_definitions(-DSDL_VIDEO_RENDER_METAL=0)
            add_definitions(-DSDL_VIDEO_METAL=0)
        elseif (BINOCLE_METAL)
            file(GLOB MAC_METAL_SOURCES
                    #					src/video/cocoa/SDL_cocoametalview.m
                    )
            list(APPEND SOURCES ${MAC_METAL_SOURCES})
            add_definitions(-DSDL_VIDEO_RENDER_METAL=1)
            add_definitions(-DSDL_VIDEO_METAL=1)
        endif ()

        #add_definitions(-DUSING_GENERATED_CONFIG_H)
        add_definitions(-DTARGET_API_MAC_CARBON)
        add_definitions(-DTARGET_API_MAC_OSX)
        add_definitions(-D_THREAD_SAFE)
        add_definitions(-DSDL_VIDEO_RENDER_OGL_ES2=0)
        add_definitions(-DSDL_VIDEO_VULKAN=0)
        add_definitions(-DSDL_VIDEO_OPENGL_ES2=0)
        add_definitions(-DSDL_VIDEO_OPENGL_EGL=0)
        if (CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
            add_definitions(-mmmx)
            add_definitions(-msse)
            add_definitions(-msse2)
        endif ()
        add_definitions(-DHX_MACOS)
        include_directories(include/configs/mac)
        include_directories(src/dynapi)
        include_directories(src/hidapi/hidapi)
    endif ()
endif ()

if (ANDROID)
    file(GLOB ANDROID_SOURCES
            src/audio/aaudio/*.c
            src/audio/android/*.c
            src/audio/openslES/*.c
            src/core/android/*.c
            src/filesystem/android/*.c
            src/haptic/android/*.c
            src/hidapi/android/hid.cpp
            src/joystick/*.c
            src/joystick/android/*.c
            src/joystick/hidapi/*.c
            src/joystick/steam/*.c
            src/joystick/virtual/*.c
            src/loadso/dlopen/*.c
            src/locale/android/*.c
            src/main/android/*.c
            src/misc/android/*.c
            src/power/android/*.c
            src/sensor/android/*.c
            src/timer/unix/*.c
            src/video/android/*.c

            src/thread/pthread/SDL_syscond.c
            src/thread/pthread/SDL_sysmutex.c
            src/thread/pthread/SDL_syssem.c
            src/thread/pthread/SDL_systhread.c
            src/thread/pthread/SDL_systls.c

            src/video/SDL_egl.c
            src/render/opengles/SDL_render_gles.c
            src/render/opengles2/SDL_render_gles2.c
            src/render/opengles2/SDL_shaders_gles2.c

            include/SDL_config_android.h
            )
    file(GLOB CPP_SOURCES
            src/hidapi/android/hid.cpp
            )

    list(APPEND SOURCES ${ANDROID_SOURCES} ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
    add_definitions(-DGL_GLEXT_PROTOTYPES)
    add_definitions(-D__ANDROID__)
    add_definitions(-DHX_ANDROID)
    include_directories(include)
    include_directories(include/configs/default)
    include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
endif ()

if (MSVC)
    file(GLOB_RECURSE WIN32_SOURCES
            #src/hidapi/hid.c
            src/hidapi/windows/hid.c
            src/video/SDL_egl.c
            src/render/opengles/SDL_render_gles.c
            src/render/opengles2/SDL_render_gles2.c
            src/render/opengles2/SDL_shaders_gles2.c

            src/thread/generic/SDL_syscond.c
            src/audio/directsound/*.c
            src/audio/wasapi/*.c
            src/audio/winmm/*.c
            src/core/windows/*.c
            src/filesystem/windows/SDL_sysfilesystem.c
            src/haptic/windows/*.c
            src/joystick/*.c
            src/joystick/windows/SDL_dinputjoystick.c
            src/joystick/windows/SDL_rawinputjoystick.c
            src/joystick/windows/SDL_windows_gaming_input.c
            src/joystick/windows/SDL_windowsjoystick.c
            src/joystick/windows/SDL_xinputjoystick.c
            src/locale/windows/*.c
            src/main/windows/SDL_windows_main.c
            src/haptic/windows/SDL_dinputhaptic.c
            src/haptic/windows/SDL_windowshaptic.c
            src/haptic/windows/SDL_xinputhaptic.c
            src/loadso/windows/SDL_sysloadso.c
            src/misc/windows/*.c
            src/power/windows/SDL_syspower.c
            src/sensor/windows/*.c
            src/thread/windows/*.c
            src/timer/windows/SDL_systimer.c
            src/video/windows/SDL_windowsclipboard.c
            src/video/windows/SDL_windowsevents.c
            src/video/windows/SDL_windowsframebuffer.c
            src/video/windows/SDL_windowskeyboard.c
            src/video/windows/SDL_windowsmessagebox.c
            src/video/windows/SDL_windowsmodes.c
            src/video/windows/SDL_windowsmouse.c
            src/video/windows/SDL_windowsopengl.c
            src/video/windows/SDL_windowsopengles.c
            src/video/windows/SDL_windowsshape.c
            src/video/windows/SDL_windowsvideo.c
            src/video/windows/SDL_windowsvulkan.c
            src/video/windows/SDL_windowswindow.c
            src/render/SDL_d3dmath.c
            src/render/direct3d/SDL_render_d3d.c
            src/render/direct3d/SDL_shaders_d3d.c
            src/render/direct3d11/SDL_render_d3d11.c
            src/render/direct3d11/SDL_shaders_d3d11.c

            #include/*.h
            #            include/SDL_config_windows.h
            )
    list(APPEND SOURCES ${WIN32_SOURCES})
    add_definitions(-DHX_WINDOWS)
    add_definitions(-DSDL_JOYSTICK_HIDAPI)
    add_definitions(-DSDL_JOYSTICK_XINPUT)
    add_definitions(-DSDL_JOYSTICK_RAWINPUT)
    add_definitions(-DSDL_TIMER_WINDOWS)
    #add_definitions(-DUSING_GENERATED_CONFIG_H)
    include_directories(include/configs/windows)
    include_directories(src/video/windows)
    include_directories(src/core/windows)
endif ()

if (EMSCRIPTEN)
    file(GLOB_RECURSE EMSCRIPTEN_SOURCES
            src/audio/emscripten/*.c
            src/filesystem/emscripten/*.c
            src/joystick/emscripten/*.c
            src/power/emscripten/*.c
            src/video/emscripten/*.c

            #include/*.h
            )
    list(APPEND SOURCES ${EMSCRIPTEN_SOURCES})
    add_definitions(-D__EMSCRIPTEN__)
    add_definitions(-DHX_EMSCRIPTEN)
    include_directories(include/configs/default)
endif ()

# Define source files
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${CPP_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(sdl OBJECT ${SOURCES})

#if (ANDROID)
#    add_library(hidapi SHARED ${CPP_SOURCES})
#    target_link_libraries(hidapi log)
#endif (ANDROID)

#set_property(TARGET sdl PROPERTY C_STANDARD 11)
if (NOT MSVC)
    # gnu11 because typeof is a GNU extension
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu11")
endif ()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

if (IOS)
    set_target_properties(
            sdl
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
    )
endif (IOS)


