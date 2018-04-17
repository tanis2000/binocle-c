//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_BINOCLE_GAME_H
#define BINOCLE_BINOCLE_GAME_H

#include <sys/types.h>
#include "binocle_platform.h"

typedef struct game_code
{
  void* GameCodeDLL;
  time_t DLLLastWriteTime;

  game_update_and_render *UpdateAndRender;

  int32_t IsValid;
} game_code;

typedef struct binocle_game {
  game_code gameCode;
} binocle_game;



void binocle_game_run();
void binocle_game_update(game_code *game, game_memory *memory);
void binocle_game_hotreload(game_code *game, game_memory *memory, char* sourceGameCodeDLLFullPath);
void binocle_unload_game_code(game_code *game);
game_code binocle_load_game_code(char* SourceDLLName);

#endif //BINOCLE_BINOCLE_GAME_H
