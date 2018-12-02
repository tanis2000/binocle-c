add_definitions(-Wno-warn-absolute-paths)

# -std=gnu++0x
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu99")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")

set (CMAKE_EXECUTABLE_SUFFIX ".html")
# Add -s ALLOW_MEMORY_GROWTH=1 to avoid memory issues at the cost of some optimizations
if (DEBUG)
    message("Compiling for EMSCRIPTEN in Debug mode")
    #it might be worth disabling WASM for debugging (-s WASM=0) but I'm not even sure it can be of any help today
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g4 -s BINARYEN_TRAP_MODE='clamp' -s ASSERTIONS=2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_OGG=1 -s TOTAL_MEMORY=1073741824 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 --preload-file /Users/tanis/Documents/binocle-c/data --use-preload-plugins -Wno-invalid-offsetof -std=gnu++0x")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g4 -s BINARYEN_TRAP_MODE='clamp' -s ASSERTIONS=2 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_OGG=1 -s TOTAL_MEMORY=1073741824 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 --preload-file /Users/tanis/Documents/binocle-c/data --use-preload-plugins")
else()
    message("Compiling for EMSCRIPTEN in Release mode")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O -s BINARYEN_TRAP_MODE='clamp' -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_OGG=1 -s TOTAL_MEMORY=1073741824 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 --preload-file /Users/tanis/Documents/binocle-c/data --use-preload-plugins -Wno-invalid-offsetof -std=gnu++0x")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O -s BINARYEN_TRAP_MODE='clamp' -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='[\"png\"]' -s USE_ZLIB=1 -s USE_LIBPNG=1 -s USE_VORBIS=1 -s USE_OGG=1 -s TOTAL_MEMORY=1073741824 -s DISABLE_EXCEPTION_CATCHING=0 -s NO_EXIT_RUNTIME=1 --preload-file /Users/tanis/Documents/binocle-c/data --use-preload-plugins")
endif ()


set (BINOCLE_LINK_LIBRARIES ${BINOCLE_LINK_LIBRARIES})