//
// Created by Valerio Santinelli on 13/04/18.
//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <glob.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#if defined(__WINDOWS__)
#include <x86intrin.h>
#endif
#include "binocle_sdl.h"
#include "binocle_game.h"
#include "binocle_platform.h"
#include "binocle_log.h"
#include "binocle_window.h"
#include "binocle_input.h"
#include "binocle_math.h"

void binocle_game_run(binocle_window window, binocle_input input) {
  binocle_game game = {};
  game_memory game_memory = {};
  game_code game_code = {};
  char *sourceGameCodeDLLFullPath = "/Users/tanis/Documents/binocle-c/cmake-build-debug/example/gameplay/libgameplay.dylib";
  game.game_code = game_code;
  game.game_memory = game_memory;
  game.game_memory.gameState = NULL;
  game.game_memory.PlatformAPI = (struct platform_api){ .binocle_log_debug = NULL, .Allocate = NULL};
  game.game_memory.PlatformAPI.binocle_log_debug = binocle_log_debug;
  game.game_memory.PlatformAPI.Allocate = SDL_malloc;

  while (!input.quit_requested) {
    float delta = (float)binocle_window_get_delta(&window) / 1000.0f;

    binocle_game_update(&game, delta);
    binocle_game_draw(&game, window, delta);
    binocle_game_hotreload(&game, sourceGameCodeDLLFullPath);
    if (!game.paused) {
      binocle_window_refresh(&window);
    }
    binocle_window_delay_framerate_if_needed(&window);
  }
}

void binocle_game_update(binocle_game *game, float dt) {
  if (game->game_code.UpdateAndRender != 0) {
    game->game_code.UpdateAndRender(&game->game_memory);
  }
}

void binocle_game_draw(binocle_game *game, binocle_window window, float dt) {
  // TODO: split UpdateAndRender into Update and Render
  if (game->paused) return;
  binocle_window_clear(&window);
}

void binocle_game_hotreload(binocle_game *game, char* sourceGameCodeDLLFullPath) {
  time_t NewDLLWriteTime = binocle_sdl_get_last_write_time(sourceGameCodeDLLFullPath);
  int32_t executableNeedsToBeReloaded = NewDLLWriteTime != game->game_code.DLLLastWriteTime;

  game->game_memory.executableReloaded = -1;

  if(executableNeedsToBeReloaded)
  {
    binocle_unload_game_code(&game->game_code);
    game->game_code = binocle_load_game_code(sourceGameCodeDLLFullPath);

    game->game_memory.executableReloaded = 0;
  }

}

void binocle_unload_game_code(game_code *game) {
  if(game->GameCodeDLL)
  {
    dlclose(game->GameCodeDLL);
    game->GameCodeDLL = 0;
  }

  game->IsValid = false;
  game->UpdateAndRender = 0;
}

game_code binocle_load_game_code(char* SourceDLLName) {
  game_code Result = {};

  Result.DLLLastWriteTime = binocle_sdl_get_last_write_time(SourceDLLName);

  if(Result.DLLLastWriteTime)
  {
    Result.GameCodeDLL = dlopen(SourceDLLName, RTLD_LAZY);
    if(Result.GameCodeDLL)
    {
      Result.UpdateAndRender = (game_update_and_render *)
        dlsym(Result.GameCodeDLL, "GameUpdateAndRender");

      Result.IsValid = (Result.UpdateAndRender != 0);
    }
    else
    {
      fprintf(stderr, "ERROR: Cannot Load game code. %s\n", dlerror());
    }
  }

  if(!Result.IsValid)
  {
    Result.UpdateAndRender = 0;
  }

  return(Result);

}
