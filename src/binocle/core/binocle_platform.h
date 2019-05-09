//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_PLATFORM_H
#define BINOCLE_PLATFORM_H

#include "stdint.h"

typedef void binocle_log_debug_t(const char *msg, ...);
typedef void *binocle_allocate_t(size_t size);

typedef struct platform_api {
  binocle_log_debug_t *binocle_log_debug;
  binocle_allocate_t *Allocate;
} platform_api;

typedef struct game_memory
{
  struct game_state *gameState;
  //struct transient_state *TransientState;

  int32_t executableReloaded;
  platform_api PlatformAPI;
} game_memory;

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory/*, game_input *Input, game_render_commands *RenderCommands*/)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#endif //BINOCLE_PLATFORM_H
