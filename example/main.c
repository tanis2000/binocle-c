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
#include <binocle_image.h>
#include <binocle_texture.h>
#include <binocle_sprite.h>
#include <binocle_shader.h>
#include <binocle_material.h>
#define BINOCLE_MATH_IMPL
#include "binocle_math.h"
#include "binocle_gd.h"
#include "binocle_log.h"
#include "sys_config.h"

//#define GAMELOOP 1

int main(int argc, char *argv[])
{
  binocle_sdl_init();
  binocle_window window = binocle_window_new(320, 240, "Binocle Test Game");
  binocle_window_set_background_color(&window, binocle_color_azure());
  binocle_viewport_adapter adapter = binocle_viewport_adapter_new(window, BINOCLE_VIEWPORT_ADAPTER_KIND_SCALING, BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_PIXEL_PERFECT, window.original_width, window.original_height, window.original_width, window.original_height);
  binocle_camera camera = binocle_camera_new(&adapter);
  //binocle_window_clear(&window);
  //binocle_window_refresh(&window);
  binocle_input input = binocle_input_new();
  char filename[1024];
  sprintf(filename, "%s%s", BINOCLE_DATA_DIR, "wabbit_alpha.png");
  binocle_image image = binocle_image_load(filename);
  binocle_texture texture = binocle_texture_from_image(image);
  char vert[1024];
  sprintf(vert, "%s%s", BINOCLE_DATA_DIR, "default.vert");
  char frag[1024];
  sprintf(frag, "%s%s", BINOCLE_DATA_DIR, "default.frag");
  binocle_shader shader = binocle_shader_load_from_file(vert, frag);
  binocle_material material = binocle_material_new();
  material.texture = &texture;
  material.shader = &shader;
  binocle_sprite player = binocle_sprite_from_material(&material);
  kmVec2 player_pos = {.x = 50, .y = 50};
  binocle_gd gd = binocle_gd_new();
  binocle_gd_init(&gd);
#ifdef GAMELOOP
  binocle_game_run(window, input);
#else
  while (!input.quit_requested) {
    binocle_window_begin_frame(&window);
    binocle_input_update(&input);

    if (input.resized) {
      kmVec2 oldWindowSize = {.x = window.width, .y = window.height};
      window.width = input.newWindowSize.x;
      window.height = input.newWindowSize.y;
      binocle_viewport_adapter_reset(&adapter, oldWindowSize, input.newWindowSize);
      input.resized = false;
    }


    if (binocle_input_is_key_pressed(input, KEY_RIGHT)) {
      player_pos.x += 50 * (1.0/window.frame_time);
    } else if (binocle_input_is_key_pressed(input, KEY_LEFT)) {
      player_pos.x -= 50 * (1.0/window.frame_time);
    }

    if (binocle_input_is_key_pressed(input, KEY_UP)) {
      player_pos.y += 50 * (1.0/window.frame_time);
    } else if (binocle_input_is_key_pressed(input, KEY_DOWN)) {
      player_pos.y -= 50 * (1.0/window.frame_time);
    }

    binocle_window_clear(&window);
    binocle_sprite_draw(player, &gd, (uint64_t)player_pos.x, (uint64_t)player_pos.y, adapter.viewport);
    binocle_window_refresh(&window);
    binocle_window_end_frame(&window);
    //binocle_log_info("FPS: %d", binocle_window_get_fps(&window));
  }
  binocle_log_info("Quit requested");
#endif
  binocle_sdl_exit();
}
