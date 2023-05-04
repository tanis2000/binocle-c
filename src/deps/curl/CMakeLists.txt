# curl 8.0.1
project(curl)

include_directories(include)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(lib)

file(GLOB SOURCES
        lib/vauth/*.c
        lib/vquic/*.c
        lib/vssh/*.c
        lib/vtls/*.c
        lib/altsvc.c
        lib/asyn-ares.c
        lib/asyn-thread.c
        lib/base64.c
        lib/bufref.c
        lib/c-hyper.c
        lib/cf-https-connect.c
        lib/cf-socket.c
        lib/cfilters.c
        lib/conncache.c
        lib/connect.c
        lib/content_encoding.c
        lib/cookie.c
        lib/curl_addrinfo.c
        lib/curl_des.c
        lib/curl_endian.c
        lib/curl_fnmatch.c
        lib/curl_get_line.c
        lib/curl_gethostname.c
        lib/curl_gssapi.c
        lib/curl_log.c
        lib/curl_memrchr.c
        lib/curl_multibyte.c
        lib/curl_ntlm_core.c
        lib/curl_path.c
        lib/curl_range.c
        lib/curl_rtmp.c
        lib/curl_sasl.c
        lib/curl_sspi.c
        lib/curl_threads.c
        lib/dict.c
        lib/doh.c
        lib/dynbuf.c
        lib/easy.c
        lib/easygetopt.c
        lib/easyoptions.c
        lib/escape.c
        lib/file.c
        lib/fileinfo.c
        lib/fopen.c
        lib/formdata.c
        lib/ftp.c
        lib/ftplistparser.c
        lib/getenv.c
        lib/getinfo.c
        lib/gopher.c
        lib/h2h3.c
        lib/hash.c
        lib/headers.c
        lib/hmac.c
        lib/hostasyn.c
        lib/hostip.c
        lib/hostip4.c
        lib/hostip6.c
        lib/hostsyn.c
        lib/hsts.c
        lib/http.c
        lib/http2.c
        lib/http_aws_sigv4.c
        lib/http_chunks.c
        lib/http_digest.c
        lib/http_negotiate.c
        lib/http_ntlm.c
        lib/http_proxy.c
        lib/idn.c
        lib/if2ip.c
        lib/imap.c
        lib/inet_ntop.c
        lib/inet_pton.c
        lib/krb5.c
        lib/ldap.c
        lib/llist.c
        lib/md4.c
        lib/md5.c
        lib/memdebug.c
        lib/mime.c
        lib/mprintf.c
        lib/mqtt.c
        lib/multi.c
        lib/netrc.c
        lib/nonblock.c
        lib/noproxy.c
        lib/openldap.c
        lib/parsedate.c
        lib/pingpong.c
        lib/pop3.c
        lib/progress.c
        lib/psl.c
        lib/rand.c
        lib/rename.c
        lib/rtsp.c
        lib/select.c
        lib/sendf.c
        lib/setopt.c
        lib/sha256.c
        lib/share.c
        lib/slist.c
        lib/smb.c
        lib/smtp.c
        lib/socketpair.c
        lib/socks.c
        lib/socks_gssapi.c
        lib/socks_sspi.c
        lib/speedcheck.c
        lib/splay.c
        lib/strcase.c
        lib/strdup.c
        lib/strerror.c
        lib/strtok.c
        lib/strtoofft.c
        lib/telnet.c
        lib/tftp.c
        lib/timediff.c
        lib/timeval.c
        lib/transfer.c
        lib/url.c
        lib/urlapi.c
        lib/version.c
        lib/warnless.c
        lib/ws.c
        )

set(SOURCES
        ${SOURCES}
        )


file(GLOB_RECURSE HEADERS
        include/curl/*.h
        )

add_definitions(-DCURL_DISABLE_DOH)
add_definitions(-DBUILDING_LIBCURL)
add_definitions(-DCURL_DISABLE_ALTSVC)
add_definitions(-DCURL_ENABLE_SSL)

if (APPLE)
    file(GLOB APPLE_SOURCES
#            config.c
#            lib/config-mac.h
            )
    list(APPEND SOURCES ${APPLE_SOURCES})
    add_definitions(-DOS="mac")
    add_definitions(-DSIZEOF_CURL_OFF_T=8)
    add_definitions(-DHAVE_NETINET_IN_H)
    add_definitions(-DHAVE_SYS_SOCKET_H)
    add_definitions(-DHAVE_NETDB_H)
    add_definitions(-DHAVE_ARPA_INET_H)
    add_definitions(-DHAVE_UNISTD_H)
    add_definitions(-DHAVE_NET_IF_H)
    add_definitions(-DHAVE_SYS_TYPES_H)
    add_definitions(-DHAVE_GETTIMEOFDAY)
    add_definitions(-DHAVE_FCNTL_H)
    add_definitions(-DHAVE_SYS_STAT_H)
    add_definitions(-DHAVE_STDLIB_H)
    add_definitions(-DHAVE_TIME_H)
    add_definitions(-DHAVE_UTIME_H)
    add_definitions(-DHAVE_SYS_TIME_H)
    add_definitions(-DHAVE_SYS_UTIME_H)
    add_definitions(-DHAVE_SYS_IOCTL_H)
    add_definitions(-DTIME_WITH_SYS_TIME)
    add_definitions(-DHAVE_ALARM)
    add_definitions(-DHAVE_FTRUNCATE)
    add_definitions(-DHAVE_UTIME)
    add_definitions(-DHAVE_SELECT)
    add_definitions(-DHAVE_SOCKET)
    add_definitions(-DHAVE_STRUCT_TIMEVAL)
    add_definitions(-DHAVE_SIGACTION)
    add_definitions(-DHAVE_SIGNAL_H)
    add_definitions(-DCURL_DISABLE_LDAP)
    add_definitions(-DHAVE_RAND_EGD)
    add_definitions(-DHAVE_IOCTL_FIONBIO)
    add_definitions(-DHAVE_RECV)
    add_definitions(-DHAVE_SEND)
    add_definitions(-DMACOS_SSL_SUPPORT)
    add_definitions(-DUSE_OPENSSL)

endif (APPLE)

#if (ANDROID)
#    file(GLOB ANDROID_SOURCES
#            src/audio/aaudio/*.c
#            src/audio/android/*.c
#            src/audio/openslES/*.c
#            src/core/android/*.c
#            src/filesystem/android/*.c
#            src/haptic/android/*.c
#            src/hidapi/android/hid.cpp
#            src/joystick/*.c
#            src/joystick/android/*.c
#            src/joystick/hidapi/*.c
#            src/joystick/steam/*.c
#            src/joystick/virtual/*.c
#            src/loadso/dlopen/*.c
#            src/locale/android/*.c
#            src/main/android/*.c
#            src/misc/android/*.c
#            src/power/android/*.c
#            src/sensor/android/*.c
#            src/timer/unix/*.c
#            src/video/android/*.c
#
#            src/thread/pthread/SDL_syscond.c
#            src/thread/pthread/SDL_sysmutex.c
#            src/thread/pthread/SDL_syssem.c
#            src/thread/pthread/SDL_systhread.c
#            src/thread/pthread/SDL_systls.c
#
#            src/video/SDL_egl.c
#            src/render/opengles/SDL_render_gles.c
#            src/render/opengles2/SDL_render_gles2.c
#            src/render/opengles2/SDL_shaders_gles2.c
#
#            include/SDL_config_android.h
#            )
#    file(GLOB CPP_SOURCES
#            src/hidapi/android/hid.cpp
#            )
#
#    list(APPEND SOURCES ${ANDROID_SOURCES} ${ANDROID_NDK}/sources/android/cpufeatures/cpu-features.c)
#    add_definitions(-DGL_GLEXT_PROTOTYPES)
#    add_definitions(-D__ANDROID__)
#    add_definitions(-DHX_ANDROID)
#    include_directories(include)
#    include_directories(include/configs/default)
#    include_directories(${ANDROID_NDK}/sources/android/cpufeatures)
#endif ()

if (MSVC)
    file(GLOB_RECURSE WIN32_SOURCES
            lib/system_win32.c
            lib/version_win32.c
            lib/config-win32.h
            )
    list(APPEND SOURCES ${WIN32_SOURCES})
endif ()

#if (EMSCRIPTEN)
#    file(GLOB_RECURSE EMSCRIPTEN_SOURCES
#            src/audio/emscripten/*.c
#            src/filesystem/emscripten/*.c
#            src/joystick/emscripten/*.c
#            src/power/emscripten/*.c
#            src/video/emscripten/*.c
#
#            #include/*.h
#            )
#    list(APPEND SOURCES ${EMSCRIPTEN_SOURCES})
#    add_definitions(-D__EMSCRIPTEN__)
#    add_definitions(-DHX_EMSCRIPTEN)
#    include_directories(include/configs/default)
#endif ()

# Define source files
set_source_files_properties(${SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(${CPP_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(curl OBJECT ${SOURCES})

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
            curl
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 9.0
    )
endif (IOS)


