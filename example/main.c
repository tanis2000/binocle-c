//
//  Binocle
//  Copyright(C)2015-2017 Valerio Santinelli
//

#include <stdio.h>
#include "binocle_sdl.h"
#include "binocle_color.h"
#include "binocle_window.h"
#include "binocle_game.h"
#include "binocle_viewport_adapter.h"
#include "binocle_camera.h"
#include <binocle_input.h>

int main(int argc, char *argv[])
{
  binocle_sdl_init();
  binocle_window window = binocle_window_new(320, 240, "Binocle Test Game");
  binocle_window_set_background_color(&window, binocle_color_white());
  binocle_viewport_adapter adapter = binocle_viewport_adapter_new(window, BINOCLE_VIEWPORT_ADAPTER_KIND_SCALING, BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_PIXEL_PERFECT, window.original_width, window.original_height, window.original_width, window.original_height);
  binocle_camera camera = binocle_camera_new(&adapter);
  //binocle_window_clear(&window);
  //binocle_window_refresh(&window);
  binocle_input input = binocle_input_new();
  binocle_game_run(window, input);
  binocle_sdl_exit();
}
