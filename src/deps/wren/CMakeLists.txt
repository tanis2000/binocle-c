project(wren)

include_directories(src/cli)
include_directories(src/include)
include_directories(src/module)
include_directories(src/optional)
include_directories(src/vm)
include_directories(../libuv/include)

file(GLOB_RECURSE SOURCES 
        src/cli/modules.c
        src/cli/path.c
        src/cli/vm.c
        src/module/io.c
        src/module/os.c
        src/module/repl.c
        src/module/scheduler.c
        src/module/timer.c
        src/optional/wren_opt_meta.c
        src/optional/wren_opt_random.c
        src/vm/wren_compiler.c
        src/vm/wren_core.c
        src/vm/wren_debug.c
        src/vm/wren_primitive.c
        src/vm/wren_utils.c
        src/vm/wren_value.c
        src/vm/wren_vm.c
)

set(SOURCES 
	${SOURCES} 
)

	
file(GLOB_RECURSE HEADERS 
	src/**/*.h
	)

# Define source files
set_source_files_properties (${SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties (${CPP_SOURCES} PROPERTIES LANGUAGE CXX)

add_library(wren OBJECT ${SOURCES})

#set_property(TARGET wren PROPERTY C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (IOS)
    set_target_properties(
            wren
            PROPERTIES
            XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET 8.0
    )
endif(IOS)


