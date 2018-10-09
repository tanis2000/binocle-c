# Binocle C version

Binocle C version is a game engine written in C.

It's born out of the need for the following features:

- Cross-platform compilation (macOS, Windows, Linux, iOS, Android, Web)
- OpenGL ES 2 support
- Simple Entity-Component-System
- Sprite batching

Nothing too fancy, but still something I always need when I make 2D (but 3D as well) games. 

## Installing

The easiest way to work with Binocle is to use the CLI project manager called `bone`.
With `bone` you can initialize a new project and automate the compilation and linking scripts. It's a quite young tool so please remember to make regular backups of your projects.

### Installing `bone`

TODO

### Creating a new project

TODO

### Updating the build scripts

TODO

### Updating Binocle

TODO

### Building a project

TODO

## Third party libraries

Binocle sits on the shoulders of giants. I tried to keep the amount of external libraries to a minimum.
The current libraries are the following:

- SDL by the almighty Ryan C. Gordon
- SDL_Mixer
- zlib
- Vorbis by the Xiph.Org Foundation
- OGG by the Xiph.Org Foundation
- FreeType (http://www.freetype.org)
- Dear ImGui
- glew (for Windows OpenGL support)
- Kazmath by Luke Benstead
- stbimage

The plan for the near future is to drop `SDL_Mixer` and replace it with `soloud`, drop `GLEW` as it's probably not even worth supporting it just for the desktop platforms.

## Coordinate system

Binocle uses a right-handed coordinate system which is the default used by OpenGL 

## Contributing

You're welcome to try Binocle and contribute. I'm writing some sparse notes at the end of this README. Most of them are related to developing Binocle.

## Compiling the library

### Mac OSX

```sh
cd build/macosx/gen
cmake -G Xcode -D DEBUG=1 ../../..
```

### Android

```sh
cd build/android/gen
cmake -D DEBUG=1 -D ANDROID_ABI=armeabi -D ANDROID_STL=gnustl_static -D ANDROID_NATIVE_API_LEVEL=android-17 -D CMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake ../../..
make
cmake -D DEBUG=1 -D ANDROID_ABI=armeabi-v7a -D ANDROID_STL=gnustl_static -D ANDROID_NATIVE_API_LEVEL=android-17 -D CMAKE_TOOLCHAIN_FILE=../../cmake/android.toolchain.cmake ../../..
make
cd ../android-project
ant debug install
```

### iOS

```sh
cd build/ios/gen
cmake -G Xcode -D DEBUG=1 -D IOS=1 ../../..
```

### Emscripten

```sh
cd build/emscripten/gen
emcmake cmake ../../.. -DCMAKE_BUILD_TYPE=Release
make -j8
cd example/src
python -m SimpleHTTPServer 8000
open http://localhost:8000/ExampleProject.html
```

## Credits

Many of the concepts come from Matt Thorson's Monocle engine (the name of this project is a kind of joke around Matt's engine name as you can guess).
The ECS is based on the excellent artemis-odb.
Some of the code has been developed while Prime31 was working on its own C# engine called Nez so there will surely be similarities and lines of code that are almost the same.
The SAT2D code has been taken and adapted from Sven's Luxe Engine. 
Other pieces of code have been taken here and there on the web and I can't recall where they come from. If you see some code that looks familiar, please let me know and I'll give full credits.

## License

The MIT License (MIT)
Copyright (c) 2015-2018 Valerio Santinelli

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
