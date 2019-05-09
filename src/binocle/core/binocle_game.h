//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_GAME_H
#define BINOCLE_GAME_H

#include <sys/types.h>
#include "binocle_platform.h"

struct binocle_window;
struct binocle_input;

typedef struct game_code
{
  void* GameCodeDLL;
  time_t DLLLastWriteTime;

  game_update_and_render *UpdateAndRender;

  int32_t IsValid;
} game_code;

typedef struct binocle_game {
  game_code game_code;
  game_memory game_memory;
  bool paused;
} binocle_game;



void binocle_game_run(struct binocle_window window, struct binocle_input input);
void binocle_game_update(binocle_game *game, float dt);
void binocle_game_draw(binocle_game *game, struct binocle_window window, float dt);
void binocle_game_hotreload(binocle_game *game, char* sourceGameCodeDLLFullPath);
void binocle_unload_game_code(game_code *game);
game_code binocle_load_game_code(char* SourceDLLName);

#endif //BINOCLE_GAME_H
