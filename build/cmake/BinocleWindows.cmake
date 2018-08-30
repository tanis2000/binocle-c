include (BundleUtilities)

add_definitions (-D__WINDOWS__) # min and max definitions are clashing with glm
add_definitions (-DNOMINMAX) # min and max definitions are clashing with glm
#add_definitions (-DNOGDI) # the problem is that excluding wingdi also excludes some pieces needed by SDL
add_definitions(-DGLEW_STATIC) # This has to be defined here or the program will get the header with the shared declspec

set(BINOCLE_LINK_LIBRARIES ${BINOCLE_LINK_LIBRARIES} glew winmm.lib Opengl32.lib Ws2_32.lib Mincore.lib imm32.lib version.lib)
message("Windows list of libraries we will link against: ${BINOCLE_LINK_LIBRARIES}")