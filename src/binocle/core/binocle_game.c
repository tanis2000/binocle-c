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
#include <x86intrin.h>

#include "binocle_sdl.h"
#include "binocle_game.h"
#include "binocle_platform.h"
#include "binocle_log.h"

void binocle_game_run() {
  binocle_game game = {};
  game_memory gameMemory = {};
  game_code gameCode = {};
  char *sourceGameCodeDLLFullPath = "/Users/tanis/Documents/binocle-c/cmake-build-debug/example/gameplay/libgameplay.dylib";
  game.gameCode = gameCode;
  gameMemory.gameState = NULL;
  gameMemory.PlatformAPI = (struct platform_api){ .binocle_log_debug = NULL, .Allocate = NULL};
  gameMemory.PlatformAPI.binocle_log_debug = binocle_log_debug;
  gameMemory.PlatformAPI.Allocate = SDL_malloc;

  while (1 != 0) {
    binocle_game_update(&gameCode, &gameMemory);
    binocle_game_hotreload(&gameCode, &gameMemory, sourceGameCodeDLLFullPath);
    SDL_Delay(1000);
  }
}

void binocle_game_update(game_code *game, game_memory *memory) {
  if (game->UpdateAndRender != 0) {
    game->UpdateAndRender(memory);
  }
}

void binocle_game_hotreload(game_code *game, game_memory *memory, char* sourceGameCodeDLLFullPath) {
  time_t NewDLLWriteTime = binocle_sdl_get_last_write_time(sourceGameCodeDLLFullPath);
  int32_t executableNeedsToBeReloaded = NewDLLWriteTime != game->DLLLastWriteTime;

  memory->executableReloaded = -1;

  if(executableNeedsToBeReloaded)
  {
    binocle_unload_game_code(game);
    *game = binocle_load_game_code(sourceGameCodeDLLFullPath);

    memory->executableReloaded = 0;
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