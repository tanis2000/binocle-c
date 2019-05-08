
include_directories(.)

file(GLOB_RECURSE SOURCES *.c)
#set(SOURCES
#  GLOB
#  *.c
#  )
set(HEADERS
	dr_flac.h
	dr_mp3.h
	dr_wav.h
	jar_mod.h
	jar_xm.h
	miniaudio.h
)

add_library(miniaudio ${HEADERS} ${SOURCES})

if (IOS)
	set_target_properties(
			miniaudio
			PROPERTIES
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
	)
endif(IOS)

