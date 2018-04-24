
add_definitions(-DHAVE_LROUND)
add_definitions(-DVERSION="1.3.1")
if(APPLE OR EMSCRIPTEN)
	add_definitions(-D__GNUC__=6)
endif()

if(WIN32)
	add_definitions(-DFLAC__HAS_OGG)
	add_definitions(-DFLAC__CPU_IA32)
	add_definitions(-DFLAC__SSE_OS)
	add_definitions(-DFLAC__HAS_NASM)
	add_definitions(-DFLAC__HAS_X86INTRIN)
	add_definitions(-DFLAC__ALIGN_MALLOC_DATA)
	add_definitions(-DFLAC__NO_DLL)
	add_definitions(-DFLAC__OVERFLOW_DETECT)
endif()

file(GLOB_RECURSE SOURCES 
  src/libFLAC/*.c 
  include/*.h
  src/libFLAC/include/*.h
  src/libFLAC/include/private/*.h
  src/libFLAC/include/protected/*.h
  )
file(GLOB REMOVE_SOURCES  
#  "src/libFLAC/bitreader.c"
#  "src/libFLAC/bitwriter.c"
  "src/libFLAC/cpu.c"
#  "src/libFLAC/fixed.c"
#  "src/libFLAC/fixed_intrin_sse2.c"
#  "src/libFLAC/fixed_intrin_ssse3.c"
#  "src/libFLAC/format.c"
#  "src/libFLAC/metadata_iterators.c"
#  "src/libFLAC/ogg_decoder_aspect.c"
)
list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})
set(SOURCES 
	${SOURCES} 
	)



file(GLOB_RECURSE HEADERS 
  include/*.h
  src/libFLAC/include/*.h
  src/libFLAC/include/private/*.h
  src/libFLAC/include/protected/*.h
  )
#file(GLOB REMOVE_HEADERS  
#  "src/SFML/Network/Win32/*.hpp"
#)
#list(REMOVE_ITEM SOURCES ${REMOVE_SOURCES})

if(APPLE OR ANDROID)
  set(SOURCES
    ${SOURCES}
    "src/libFLAC/cpu.c"
    )
endif()


include_directories(include)
include_directories(src)
include_directories(src/libFLAC/include)
include_directories(../ogg/include)

add_library(flac ${HEADERS} ${SOURCES})

if (IOS)
	set_target_properties(
		flac
		PROPERTIES
		XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
	)
endif(IOS)
