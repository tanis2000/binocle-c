Features
========

- Cross-platform: macOS, Windows, iOS, Android, Web (Linux planned)
- OpenGL API (ES 2/3 on mobile platforms)
- 2D Sprites
- Spritesheets (TexturePacker format. LibGDX format is in the works)
- Sprite batching
- Music and sound effects
- Bezier paths that can be used for anything
- BitmapFont fonts
- 2D Camera
- 2D Collisions (boxes and circles)
- Easing functions
- Entity Component System
- Timing functions
- Viewport adapters for 2D pixel perfect images
- Experimental hot code reloading for game code
- Lua scripting through LuaJIT on supported platforms

Third party libraries
---------------------

Binocle sits on the shoulders of giants. I tried to keep the amount of external libraries to a minimum.
The current libraries are the following:

- SDL by the almighty Ryan C. Gordon (OS abstraction)
- miniaudio (cross-platform audio support)
- zlib
- Vorbis by the Xiph.Org Foundation
- OGG by the Xiph.Org Foundation
- FreeType
- Dear ImGui
- glew (for Windows OpenGL support)
- Kazmath by Luke Benstead
- stbimage
- parson
- LuaJIT
- sokol_time

Coordinate system
-----------------

Binocle uses a right-handed coordinate system which is the same used by OpenGL
