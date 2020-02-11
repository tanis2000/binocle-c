project(sdl)

include_directories(include)
include_directories(src)

file(GLOB_RECURSE SOURCES 
        src/fs-poll.c
        src/inet.c
		src/threadpool.c
        src/uv-common.c
        src/version.c
)

set(SOURCES 
	${SOURCES} 
)

	
file(GLOB_RECURSE HEADERS 
	include/*.h
	)

if(MSVC)
	file(GLOB_RECURSE WIN32_SOURCES
			src/win/async.c
			src/win/core.c
			src/win/detect-wakeup.c
			src/win/dl.c
			src/win/error.c
			src/win/fs.c
			src/win/fs-event.c
			src/win/getaddrinfo.c
			src/win/getnameinfo.c
			src/win/handle.c
		)
	list(APPEND SOURCES ${WIN32_SOURCES})
        include_directories(src/win)
else()
	file(GLOB_RECURSE UNIX_SOURCES
			src/unix/async.c
			src/unix/core.c
			src/unix/dl.c
			src/unix/fs.c
			src/unix/getaddrinfo.c
			src/unix/getnameinfo.c
			#src/unix/kqueue.c
			src/unix/loop.c
			src/unix/loop-watcher.c
			src/unix/pipe.c
			src/unix/poll.c
			src/unix/process.c
			src/unix/proctitle.c
			#src/unix/pthread-barrier.c
			#src/unix/pthread-fixes.c
			src/unix/signal.c
			src/unix/stream.c
			src/unix/tcp.c
			src/unix/thread.c
			src/unix/timer.c
			src/unix/tty.c
			src/unix/udp.c
			)
	list(APPEND SOURCES ${UNIX_SOURCES})
	include_directories(src/unix)
endif()

if (APPLE)
	file(GLOB_RECURSE APPLE_SOURCES
			src/unix/darwin.c
			src/unix/darwin-proctitle.c
			src/unix/fsevents.c
			)
	list(APPEND SOURCES ${APPLE_SOURCES})
	include_directories(src/unix)
endif()

if (EMSCRIPTEN)
	file(GLOB_RECURSE EMSCRIPTEN_SOURCES
			#src/unix/linux-core.c
			#src/unix/linux-inotify.c
			#src/unix/linux-syscalls.c

			#include/uv-linux.h
			#include/uv-unix.h
			)
	list(APPEND SOURCES ${EMSCRIPTEN_SOURCES})
	add_definitions(-D_POSIX_SOURCE)
	add_definitions(-D_GNU_SOURCE)
endif()

# Define source files
set_source_files_properties (${SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties (${CPP_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(libuv OBJECT ${SOURCES})

#set_property(TARGET sdl PROPERTY C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (IOS)
    set_target_properties(
            libuv
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)


