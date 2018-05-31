//
//  Binocle
//  Copyright(C)2015-2018 Valerio Santinelli
//

#include <stdio.h>
#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif
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
#include "binocle_bitmapfont.h"
#include "sys_config.h"

//#define GAMELOOP 1

binocle_window window;
binocle_input input;
binocle_viewport_adapter adapter;
binocle_camera camera;
binocle_sprite player;
kmVec2 player_pos;
binocle_gd gd;
binocle_bitmapfont font;
binocle_image font_image;
binocle_texture font_texture;
binocle_material font_material;
binocle_sprite font_sprite;
kmVec2 font_sprite_pos;

void main_loop() {
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
  char fps_str[256];
  sprintf(fps_str, "FPS: %d", binocle_window_get_fps(&window));
  binocle_bitmapfont_draw_string(font, fps_str, 32, &gd, 0, window.height - 32, adapter.viewport);
  //binocle_sprite_draw(font_sprite, &gd, (uint64_t)font_sprite_pos.x, (uint64_t)font_sprite_pos.y, adapter.viewport);
  binocle_window_refresh(&window);
  binocle_window_end_frame(&window);
  //binocle_log_info("FPS: %d", binocle_window_get_fps(&window));
}

int main(int argc, char *argv[])
{
  binocle_sdl_init();
  window = binocle_window_new(320, 240, "Binocle Test Game");
  binocle_window_set_background_color(&window, binocle_color_azure());
  adapter = binocle_viewport_adapter_new(window, BINOCLE_VIEWPORT_ADAPTER_KIND_SCALING, BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_PIXEL_PERFECT, window.original_width, window.original_height, window.original_width, window.original_height);
  camera = binocle_camera_new(&adapter);
  input = binocle_input_new();
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
  player = binocle_sprite_from_material(&material);
  player_pos.x = 50;
  player_pos.y = 50;

  char font_filename[1024];
  sprintf(font_filename, "%s%s", BINOCLE_DATA_DIR, "font.fnt");
  font = binocle_bitmapfont_from_file(font_filename, true);

  char font_image_filename[1024];
  sprintf(font_image_filename, "%s%s", BINOCLE_DATA_DIR, "font.png");
  font_image = binocle_image_load(font_image_filename);
  font_texture = binocle_texture_from_image(font_image);
  font_material = binocle_material_new();
  font_material.texture = &font_texture;
  font_material.shader = &shader;
  font.material = &font_material;
  font_sprite = binocle_sprite_from_material(&font_material);
  font_sprite_pos.x = 0;
  font_sprite_pos.y = -256;

  gd = binocle_gd_new();
  binocle_gd_init(&gd);
#ifdef GAMELOOP
  binocle_game_run(window, input);
#else
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(main_loop, 0, 1);
#else
  while (!input.quit_requested) {
    main_loop();
  }
#endif
  binocle_log_info("Quit requested");
#endif
  binocle_sdl_exit();
}


