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
#define DEMOLOOP
//#define TWODLOOP

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
float time;
binocle_shader quad_shader;
binocle_shader sdf_shader;
binocle_shader fxaa_shader;
binocle_shader dof_shader;
binocle_shader bloom_shader;

#ifdef TWODLOOP
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
#endif

#ifdef DEMOLOOP

binocle_render_target g_buffer;
binocle_render_target dof_buffer;
binocle_render_target dof_far_buffer1;
binocle_render_target dof_far_buffer2;
binocle_render_target dof_far_buffer3;
binocle_render_target dof_near_buffer1;
binocle_render_target dof_near_buffer2;
binocle_render_target bloom_depth_buffer;
binocle_render_target bloom_buffer1;
binocle_render_target bloom_buffer2;
binocle_render_target masking_buffer;
binocle_render_target aberration_buffer;
binocle_render_target compose_buffer;
binocle_render_target fxaa_buffer;

void init_render_targets() {
  g_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  dof_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  dof_far_buffer1 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  dof_far_buffer2 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  dof_far_buffer3 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  dof_near_buffer1 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  dof_near_buffer2 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  bloom_depth_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  bloom_buffer1 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  bloom_buffer2 = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  masking_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  aberration_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  compose_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
  fxaa_buffer = binocle_gd_create_render_target(window.width, window.height, false, GL_RGBA);
}

void draw_scene() {
  binocle_gd_apply_shader(&gd, sdf_shader);
  binocle_gd_set_uniform_float(sdf_shader, "time", time);
  binocle_gd_set_uniform_float2(sdf_shader, "resolution", window.width, window.height);
  // TODO: set all other uniforms
  binocle_gd_draw_quad(sdf_shader); // Draws the current vertices buffer to the render target
}

void gbuffer_pass() {
  binocle_gd_set_render_target(g_buffer);
  kmAABB2 vp = {.min.x = 0, .min.y = 0, .max.x = window.width, .max.y = window.height};
  binocle_gd_apply_viewport(vp);
  binocle_color clear_color = {.a = 1.0, .r = 0.0, .g = 1.0, .b = 0.0};
  binocle_gd_clear(clear_color);
  draw_scene();
}

void fxaa_pass() {
  binocle_gd_set_render_target(fxaa_buffer);
  binocle_gd_apply_shader(&gd, fxaa_shader);
  binocle_gd_set_uniform_float(fxaa_shader, "time", time);
  binocle_gd_set_uniform_float2(fxaa_shader, "resolution", window.width, window.height);
  binocle_gd_set_uniform_render_target_as_texture(fxaa_shader, "texture", g_buffer);
  binocle_gd_draw_quad(fxaa_shader);
}

void dof_pass() {
  binocle_gd_set_render_target(dof_buffer);
  binocle_gd_apply_shader(&gd, dof_shader);
  binocle_gd_set_uniform_float(dof_shader, "time", time);
  binocle_gd_set_uniform_float2(dof_shader, "resolution", window.width, window.height);
  binocle_gd_set_uniform_float2(dof_shader, "uv", 1.0, 1.0);
  binocle_gd_set_uniform_render_target_as_texture(dof_shader, "tInput", g_buffer);
  binocle_gd_set_uniform_render_target_as_texture(dof_shader, "tBias", g_buffer);
  binocle_gd_set_uniform_float(dof_shader, "radius", 0.5);
  binocle_gd_set_uniform_float(dof_shader, "amount", 1.0);
  binocle_gd_set_uniform_float(dof_shader, "focalDistance", 2.0);
  binocle_gd_set_uniform_float(dof_shader, "aperture", 2.0);
  binocle_gd_draw_quad(dof_shader);
}

void bloom_pass() {
  binocle_gd_set_render_target(bloom_buffer1);
  binocle_gd_apply_shader(&gd, bloom_shader);
  binocle_gd_set_uniform_float(bloom_shader, "time", time);
  binocle_gd_set_uniform_float2(bloom_shader, "resolution", window.width, window.height);
  binocle_gd_set_uniform_float2(bloom_shader, "uv", 1.0, 1.0);
  binocle_gd_set_uniform_render_target_as_texture(bloom_shader, "tInput", g_buffer);
  binocle_gd_draw_quad(bloom_shader);
}

void compose_pass() {

}

void masking_pass() {

}

void aberration_pass() {

}

void tonemap_pass() {

}

void screen_pass() {
  //binocle_gd_set_render_target(g_buffer);
  binocle_gd_apply_shader(&gd, quad_shader);
  binocle_gd_set_uniform_float(quad_shader, "time", time);
  binocle_gd_set_uniform_float2(quad_shader, "resolution", window.width, window.height);
  if (binocle_input_is_key_pressed(input, KEY_1)) {
    binocle_gd_draw_quad_to_screen(quad_shader, fxaa_buffer);
  } else if (binocle_input_is_key_pressed(input, KEY_2)) {
    binocle_gd_draw_quad_to_screen(quad_shader, dof_buffer);
  } else if (binocle_input_is_key_pressed(input, KEY_3)) {
    binocle_gd_draw_quad_to_screen(quad_shader, bloom_buffer1);
  } else {
    binocle_gd_draw_quad_to_screen(quad_shader, g_buffer);
  }
}

void render_loop() {
  init_render_targets();
  gbuffer_pass();
  fxaa_pass();
  dof_pass();
  bloom_pass();
  compose_pass();
  masking_pass();
  aberration_pass();
  tonemap_pass();
  screen_pass();
}

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


  binocle_window_clear(&window);

  render_loop();
  time += (binocle_window_get_frame_time(&window) / 1000.0);
  char fps_str[256];
  sprintf(fps_str, "FPS: %d", binocle_window_get_fps(&window));
  binocle_bitmapfont_draw_string(font, fps_str, 32, &gd, 0, window.height - 32, adapter.viewport);

  binocle_window_refresh(&window);
  binocle_window_end_frame(&window);
}
#endif

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

  // Load the default quad shader
  sprintf(vert, "%s%s", BINOCLE_DATA_DIR, "screen.vert");
  sprintf(frag, "%s%s", BINOCLE_DATA_DIR, "screen.frag");
  quad_shader = binocle_shader_load_from_file(vert, frag);

  // Load the SDF shader
  sprintf(vert, "%s%s", BINOCLE_DATA_DIR, "test.vert");
  sprintf(frag, "%s%s", BINOCLE_DATA_DIR, "test2.frag");
  sdf_shader = binocle_shader_load_from_file(vert, frag);

  // Load the FXAA shader
  sprintf(vert, "%s%s", BINOCLE_DATA_DIR, "test.vert");
  sprintf(frag, "%s%s", BINOCLE_DATA_DIR, "fxaa.frag");
  fxaa_shader = binocle_shader_load_from_file(vert, frag);

  // Load the DOF shader
  sprintf(vert, "%s%s", BINOCLE_DATA_DIR, "dof.vert");
  sprintf(frag, "%s%s", BINOCLE_DATA_DIR, "dof.frag");
  dof_shader = binocle_shader_load_from_file(vert, frag);

  // Load the bloom shader
  sprintf(vert, "%s%s", BINOCLE_DATA_DIR, "dof.vert");
  sprintf(frag, "%s%s", BINOCLE_DATA_DIR, "bloom2.frag");
  bloom_shader = binocle_shader_load_from_file(vert, frag);

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
  time = 0;
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


