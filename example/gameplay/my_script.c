#include <stdio.h>
#include "my_script.h"
//#include <binocle_window.c>

GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
  {
    platform_api Platform = Memory->PlatformAPI;

  struct game_state *GameState = Memory->gameState;
  if(!GameState)
  {
    //GameState = Memory->GameState = BootstrapPushStruct(game_state, TotalArena);
    GameState = Memory->gameState = (struct game_state*)Platform.Allocate(sizeof(struct game_state));
    GameState->counter = 0;
  } else {
    GameState->counter += 3;
    char str[256];
    sprintf(str, "Count: %d things", GameState->counter);
    Platform.binocle_log_debug(str);
  }


    // NOTE(casey): Transient initialization

  }
