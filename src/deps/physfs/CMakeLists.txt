project(physfs)

include_directories(.)

file(GLOB_RECURSE SOURCES 
        physfs.c
		physfs_archiver_7z.c
		physfs_archiver_dir.c
		physfs_archiver_grp.c
		physfs_archiver_hog.c
		physfs_archiver_iso9660.c
		physfs_archiver_mvl.c
		physfs_archiver_qpak.c
		physfs_archiver_slb.c
		physfs_archiver_unpacked.c
		physfs_archiver_vdf.c
		physfs_archiver_wad.c
		physfs_archiver_zip.c
		physfs_byteorder.c
		physfs_platform_posix.c
		physfs_platform_unix.c
		physfs_platform_windows.c
		physfs_platform_os2.c
		physfs_platform_qnx.c
		physfs_unicode.c
)

set(SOURCES 
	${SOURCES} 
)

file(GLOB_RECURSE HEADERS *.h)

if(APPLE)
	file(GLOB APPLE_SOURCES
		physfs_platform_apple.m
	)
	list(APPEND SOURCES ${APPLE_SOURCES})
endif()

if(ANDROID)
	file(GLOB ANDROID_SOURCES
			physfs_platform_posix.c
			)
	list(APPEND SOURCES ${ANDROID_SOURCES})
endif()

if(MSVC)
	file(GLOB WINDOWS_SOURCES
			physfs_platform_windows.c
			)
	list(APPEND SOURCES ${WINDOWS_SOURCES})
endif()

if(MSVC)
	file(GLOB WINDOWS_SOURCES
			physfs_platform_windows.c
			)
	list(APPEND SOURCES ${WINDOWS_SOURCES})
endif()

if(EMSCRIPTEN)
	file(GLOB EMSCRIPTEN_SOURCES
			physfs_platform_unix.c
			)
	list(APPEND SOURCES ${EMSCRIPTEN_SOURCES})
endif()

# Define source files
set_source_files_properties (${SOURCES} PROPERTIES LANGUAGE C)

add_library(physfs OBJECT ${HEADERS} ${SOURCES})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

if (IOS)
    set_target_properties(
            physfs
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)


