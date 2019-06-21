Building the library
====================

The whole Binocle toolchain is based on CMake and makes it quite easy to build for different architectures.
I usually suggest to use `bone` to build your project, but if you really need to compile the library by hand, here are
the steps to follow for each and every supported platform.

macOS
-----

.. code-block:: sh

    cd build/macosx/gen
    cmake -G Xcode -D DEBUG=1 ../../..


Windows
-------

I usually run the CMake GUI tool and select the Visual Studio generator there. That's pretty much all that's needed.

Android
-------

You will need the Android SDK and NDK and the correct environment variables for this to work.

.. code-block:: sh

    cd build/android/gen
    cmake -D DEBUG=1 -D ANDROID_ABI=armeabi -D ANDROID_STL=c++_static -D ANDROID_PLATFORM=android-21 -D CMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake ../../..
    make
    cmake -D DEBUG=1 -D ANDROID_ABI=armeabi-v7a -D ANDROID_STL=c++_static -D ANDROID_PLATFORM=android-21 -D CMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake ../../..
    make
    cmake -D DEBUG=1 -D ANDROID_ABI=x86_64 -D ANDROID_STL=c++_static -D ANDROID_PLATFORM=android-21 -D CMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake ../../..
    make
    cd ../android-project
    ./gradlew installDebug

iOS
---

You will need the latest Xcode and its command line tools.

.. code-block:: sh

    cd build/ios/gen
    cmake -G Xcode -D DEBUG=1 -D IOS=1 ../../..

Emscripten (web)
----------------

You need a recent version of Emscripten installed on your system.
If you're using macOS, just do a `brew install emscripten` to set it up.

.. code-block:: sh

    cd build/emscripten/gen
    emcmake cmake ../../.. -DCMAKE_BUILD_TYPE=Release
    make -j8
    cd example/src
    python -m SimpleHTTPServer 8000
    open http://localhost:8000/ExampleProject.html
