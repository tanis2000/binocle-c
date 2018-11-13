include_directories(include)

file(GLOB_RECURSE SOURCES 
        src/SDL.c
        src/SDL_assert.c
		src/SDL_dataqueue.c
        src/SDL_error.c
        src/SDL_hints.c
        src/SDL_log.c
        src/atomic/SDL_atomic.c
        src/atomic/SDL_spinlock.c
        src/audio/SDL_audio.c
        src/audio/SDL_audiocvt.c
        src/audio/SDL_audiotypecvt.c
        src/audio/SDL_mixer.c
        src/audio/SDL_wave.c
        src/cpuinfo/SDL_cpuinfo.c
        src/cpuinfo/SDL_simd.c
        src/dynapi/SDL_dynapi.c
        src/events/SDL_clipboardevents.c
        src/events/SDL_displayevents.c
        src/events/SDL_dropevents.c
        src/events/SDL_events.c
        src/events/SDL_gesture.c
        src/events/SDL_keyboard.c
        src/events/SDL_mouse.c
        src/events/SDL_quit.c
        src/events/SDL_touch.c
        src/events/SDL_windowevents.c
        src/file/SDL_rwops.c
        src/haptic/SDL_haptic.c
        src/joystick/SDL_gamecontroller.c
        src/joystick/SDL_joystick.c
        src/joystick/hidapi/SDL_hidapijoystick.c
        src/joystick/hidapi/SDL_hidapi_ps4.c
        src/joystick/hidapi/SDL_hidapi_switch.c
        src/joystick/hidapi/SDL_hidapi_xbox360.c
        src/joystick/hidapi/SDL_hidapi_xboxone.c
        src/joystick/steam/SDL_steamcontroller.c
        src/libm/e_atan2.c
        src/libm/e_exp.c
        src/libm/e_fmod.c
        src/libm/e_log.c
        src/libm/e_log10.c
        src/libm/e_pow.c
        src/libm/e_rem_pio2.c
        src/libm/e_sqrt.c
        src/libm/k_cos.c
        src/libm/k_rem_pio2.c
        src/libm/k_sin.c
        src/libm/k_tan.c
        src/libm/s_atan.c
        src/libm/s_copysign.c
        src/libm/s_cos.c
        src/libm/s_tan.c
        src/libm/s_fabs.c
        src/libm/s_floor.c
        src/libm/s_scalbn.c
        src/libm/s_sin.c
        src/power/SDL_power.c
        src/render/SDL_render.c
        src/render/SDL_yuv_sw.c
        src/sensor/SDL_sensor.c
        src/sensor/dummy/SDL_dummysensor.c
        src/stdlib/SDL_getenv.c
        src/stdlib/SDL_iconv.c
        src/stdlib/SDL_malloc.c
        src/stdlib/SDL_qsort.c
        src/stdlib/SDL_stdlib.c
        src/stdlib/SDL_string.c
        src/thread/SDL_thread.c
        src/timer/SDL_timer.c
        src/video/SDL_RLEaccel.c
        src/video/SDL_blit.c
        src/video/SDL_blit_0.c
        src/video/SDL_blit_1.c
        src/video/SDL_blit_A.c
        src/video/SDL_blit_N.c
        src/video/SDL_blit_auto.c
        src/video/SDL_blit_copy.c
        src/video/SDL_blit_slow.c
        src/video/SDL_bmp.c
        src/video/SDL_clipboard.c
        src/video/SDL_fillrect.c
        src/video/SDL_pixels.c
        src/video/SDL_rect.c
        src/video/SDL_shape.c
        src/video/SDL_stretch.c
        src/video/SDL_surface.c
        src/video/SDL_video.c
		src/video/SDL_vulkan_utils.c
		src/video/SDL_yuv.c
		src/video/yuv2rgb/yuv_rgb.c
        src/audio/disk/SDL_diskaudio.c
        src/loadso/dlopen/SDL_sysloadso.c
        src/render/opengl/SDL_render_gl.c
        src/render/opengl/SDL_shaders_gl.c
        src/render/software/SDL_blendfillrect.c
        src/render/software/SDL_blendline.c
        src/render/software/SDL_blendpoint.c
        src/render/software/SDL_drawline.c
        src/render/software/SDL_drawpoint.c
        src/render/software/SDL_render_sw.c
        src/render/software/SDL_rotate.c
        src/video/dummy/SDL_nullevents.c
        src/video/dummy/SDL_nullframebuffer.c
        src/video/dummy/SDL_nullvideo.c
        src/audio/dummy/SDL_dummyaudio.c	
)

set(SOURCES 
	${SOURCES} 
)

	
file(GLOB_RECURSE HEADERS 
	include/*.h
	)

if(APPLE)
	if(IOS)
		file(GLOB_RECURSE IOS_SOURCES
                src/sensor/coremotion/SDL_coremotionsensor.m
                src/hidapi/ios/hid.m
            src/thread/pthread/SDL_syscond.c
            src/thread/pthread/SDL_sysmutex.c
            src/thread/pthread/SDL_syssem.c
            src/thread/pthread/SDL_systhread.c
            src/thread/pthread/SDL_systls.c
            src/timer/unix/SDL_systimer.c

            src/video/SDL_egl.c
            src/render/opengles/SDL_render_gles.c
            src/render/opengles2/SDL_render_gles2.c
            src/render/opengles2/SDL_shaders_gles2.c

            src/audio/coreaudio/SDL_coreaudio.m
            src/file/cocoa/SDL_rwopsbundlesupport.m
            src/filesystem/cocoa/SDL_sysfilesystem.m
            src/joystick/iphoneos/SDL_sysjoystick.m
            src/haptic/dummy/SDL_syshaptic.c
            src/power/uikit/SDL_syspower.m
			src/video/cocoa/SDL_cocoaopengles.m
			src/video/cocoa/SDL_cocoavulkan.m
            src/video/uikit/SDL_uikitappdelegate.m
            src/video/uikit/SDL_uikitclipboard.m
            src/video/uikit/SDL_uikitevents.m
            src/video/uikit/SDL_uikitmessagebox.m
            src/video/uikit/SDL_uikitmetalview.m
            src/video/uikit/SDL_uikitmodes.m
            src/video/uikit/SDL_uikitopengles.m
            src/video/uikit/SDL_uikitopenglview.m
            src/video/uikit/SDL_uikitvideo.m
            src/video/uikit/SDL_uikitview.m
            src/video/uikit/SDL_uikitviewcontroller.m
            src/video/uikit/SDL_uikitvulkan.m
            src/video/uikit/SDL_uikitwindow.m

            include/*.h
            include/SDL_config_iphoneos.h
			)
 	   	list(APPEND SOURCES ${IOS_SOURCES})
 	   	add_definitions(-DSFML_SYSTEM_IOS)
 	   	add_definitions(-DSFML_OPENGL_ES)
 	    add_definitions(-DGL_GLEXT_PROTOTYPES)
        add_definitions(-fobjc-arc)
        add_definitions(-DHX_IOS)
        add_definitions(-D__IPHONEOS__)
        include_directories(include)
        include_directories(include/configs/default)
	else()
		file(GLOB MAC_SOURCES
                src/sensor/coremotion/SDL_coremotionsensor.m
                src/hidapi/mac/hid.c
            src/thread/pthread/SDL_syscond.c
            src/thread/pthread/SDL_sysmutex.c
            src/thread/pthread/SDL_syssem.c
            src/thread/pthread/SDL_systhread.c
            src/thread/pthread/SDL_systls.c
            src/timer/unix/SDL_systimer.c

            src/audio/coreaudio/SDL_coreaudio.m
            src/joystick/darwin/SDL_sysjoystick.c
            src/haptic/darwin/SDL_syshaptic.c
            src/power/macosx/SDL_syspower.c
            src/file/cocoa/SDL_rwopsbundlesupport.m
            src/filesystem/cocoa/SDL_sysfilesystem.m
            src/video/SDL_egl.c
            src/video/cocoa/SDL_cocoaclipboard.m
            src/video/cocoa/SDL_cocoaevents.m
            src/video/cocoa/SDL_cocoakeyboard.m
            src/video/cocoa/SDL_cocoamessagebox.m
            src/video/cocoa/SDL_cocoamodes.m
            src/video/cocoa/SDL_cocoamouse.m
            src/video/cocoa/SDL_cocoamousetap.m
            src/video/cocoa/SDL_cocoaopengl.m
            src/video/cocoa/SDL_cocoashape.m
            src/video/cocoa/SDL_cocoavideo.m
            src/video/cocoa/SDL_cocoavulkan.m
            src/video/cocoa/SDL_cocoawindow.m

            #include/configs/mac/*.h
			)
 	   	list(APPEND SOURCES ${MAC_SOURCES})
 	   	#add_definitions(-DUSING_GENERATED_CONFIG_H)
        add_definitions(-DTARGET_API_MAC_CARBON)
        add_definitions(-DTARGET_API_MAC_OSX)
        add_definitions(-D_THREAD_SAFE)
        add_definitions(-mmmx)
        add_definitions(-msse)
        add_definitions(-msse2)
        add_definitions(-DHX_MACOS)
        include_directories(include/configs/mac)
        include_directories(src/hidapi/hidapi)
	endif()
endif()

if(ANDROID)
	file(GLOB_RECURSE ANDROID_SOURCES
            src/hidapi/android/hid.cpp
            src/sensor/android/SDL_androidsensor.c
            src/thread/pthread/SDL_syscond.c
            src/thread/pthread/SDL_sysmutex.c
            src/thread/pthread/SDL_syssem.c
            src/thread/pthread/SDL_systhread.c
            src/thread/pthread/SDL_systls.c
            src/timer/unix/SDL_systimer.c

            src/video/SDL_egl.c
            src/render/opengles/SDL_render_gles.c
            src/render/opengles2/SDL_render_gles2.c
            src/render/opengles2/SDL_shaders_gles2.c

            src/audio/android/SDL_androidaudio.c
            src/core/android/SDL_android.c
            src/joystick/android/SDL_sysjoystick.c
            src/filesystem/android/SDL_sysfilesystem.c
            #src/main/android/SDL_android_main.c
            src/power/android/SDL_syspower.c
            src/video/android/SDL_androidclipboard.c
            src/video/android/SDL_androidevents.c
            src/video/android/SDL_androidgl.c
            src/video/android/SDL_androidkeyboard.c
            src/video/android/SDL_androidtouch.c
            src/video/android/SDL_androidvideo.c
            src/video/android/SDL_androidwindow.c
            src/video/android/SDL_androidmessagebox.c
            src/video/android/SDL_androidmouse.c
            src/video/android/SDL_androidvulkan.c
            #src/haptic/dummy/SDL_syshaptic.c
			src/haptic/android/SDL_syshaptic.c

            #include/*.h
            include/SDL_config_android.h
		)
	list(APPEND SOURCES ${ANDROID_SOURCES})
    add_definitions(-DGL_GLEXT_PROTOTYPES)
    add_definitions(-D__ANDROID__)
    add_definitions(-DHX_ANDROID)
    include_directories(include)
    include_directories(include/configs/default)
endif()

if(MSVC)
	file(GLOB_RECURSE WIN32_SOURCES
            src/hidapi/hid.c
            src/video/SDL_egl.c
            src/render/opengles/SDL_render_gles.c
            src/render/opengles2/SDL_render_gles2.c
            src/render/opengles2/SDL_shaders_gles2.c

            src/thread/generic/SDL_syscond.c
            src/audio/directsound/SDL_directsound.c
            src/audio/wasapi/SDL_wasapi.c
            src/audio/wasapi/SDL_wasapi_win32.c
            src/audio/winmm/SDL_winmm.c
            src/core/windows/SDL_windows.c
            src/core/windows/SDL_xinput.c
            src/filesystem/windows/SDL_sysfilesystem.c
            #<!-- src/joystick/windows/SDL_dxjoystick.c removed in 2.0.4?-->
            src/joystick/windows/SDL_mmjoystick.c
            src/joystick/windows/SDL_dinputjoystick.c
            src/joystick/windows/SDL_windowsjoystick.c
            src/joystick/windows/SDL_xinputjoystick.c
            #<!-- src/joystick/winrt/SDL_xinputjoystick.c  removed in 2.0.4? -->
            src/main/windows/SDL_windows_main.c
            #<!-- src/haptic/windows/SDL_syshaptic.c  removed in 2.0.4?-->
            src/haptic/windows/SDL_dinputhaptic.c
            src/haptic/windows/SDL_windowshaptic.c
            src/haptic/windows/SDL_xinputhaptic.c
            src/loadso/windows/SDL_sysloadso.c
            src/power/windows/SDL_syspower.c
            src/thread/windows/SDL_sysmutex.c
            src/thread/windows/SDL_syssem.c
            src/thread/windows/SDL_systhread.c
            src/thread/windows/SDL_systls.c
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
            include/SDL_config_windows.h
		)
	list(APPEND SOURCES ${WIN32_SOURCES})
        add_definitions(-DHX_WINDOWS)
 	   	#add_definitions(-DUSING_GENERATED_CONFIG_H)
        include_directories(include/configs/windows)
        include_directories(src/video/windows)
endif()

if(EMSCRIPTEN)
    file(GLOB_RECURSE EMSCRIPTEN_SOURCES 
            src/video/emscripten/SDL_emscriptenevents.c
            src/video/emscripten/SDL_emscriptenframebuffer.c
            src/video/emscripten/SDL_emscriptenmouse.c
            src/video/emscripten/SDL_emscriptenopengles.c
            src/video/emscripten/SDL_emscriptenvideo.c
            src/power/emscripten/SDL_syspower.c
            src/joystick/emscripten/SDL_sysjoystick.c
            src/filesystem/emscripten/SDL_sysfilesystem.c
            src/audio/emscripten/SDL_emscriptenaudio.c

            include/*.h
        )
    list(APPEND SOURCES ${EMSCRIPTEN_SOURCES})
    add_definitions(-D__EMSCRIPTEN__)
    add_definitions(-DHX_EMSCRIPTEN)
    include_directories(include/configs/default)
endif()

# Define source files
set_source_files_properties (${SOURCES} PROPERTIES LANGUAGE C)

add_library(sdl ${HEADERS} ${SOURCES})
set_property(TARGET sdl PROPERTY C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

if (IOS)
    set_target_properties(
            sdl
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)


