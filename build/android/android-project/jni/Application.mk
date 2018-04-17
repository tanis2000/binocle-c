NDK_TOOLCHAIN_VERSION=4.9

# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
APP_STL := gnustl_static

APP_CPPFLAGS := -frtti -fexceptions
APP_ABI := armeabi armeabi-v7a 
# Note: Add "x86" to APP_ABI to add x86 CPU support above
