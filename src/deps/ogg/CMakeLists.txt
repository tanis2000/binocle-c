
include_directories(.)

file(GLOB_RECURSE SOURCES src/*.c)
#set(SOURCES
#  GLOB
#  *.c
#  )
set(HEADERS
	include/ogg/config_types.h
	include/ogg/ogg.h
	include/ogg/os_types.h
)

include_directories(include)

add_library(ogg ${HEADERS} ${SOURCES})

if (IOS)
	set_target_properties(
			ogg
			PROPERTIES
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
	)
endif(IOS)

