#pragma once
#include "backend/binocle_material.h"
#include "binocle_audio.h"
#include "binocle_camera.h"
#include "binocle_gd.h"
#include "binocle_input.h"
#include "binocle_sprite.h"
struct binocle_memory_arena;
struct binocle_window;
struct binocle_viewport_adapter;
struct binocle_bitmapfont;
struct binocle_material;
struct binocle_sprite;

typedef struct game_state_t {
  struct {
    struct binocle_memory_arena *main_arena;
    struct binocle_memory_arena *frame_arena;

    struct binocle_window *window;
    binocle_input input;
    struct binocle_viewport_adapter *adapter;
    binocle_gd gd;

    // Default font
    struct binocle_bitmapfont *font;
    sg_image font_texture;
    struct binocle_material *font_material;
    struct binocle_sprite *font_sprite;

    float running_time;

    binocle_audio audio;
    binocle_audio_sound sound;
    binocle_audio_music music;

    char *binocle_data_dir;

    sg_shader default_shader;
    sg_shader screen_shader;
  } engine;

  struct {
    const char *level_data;
    size_t level_size_in_bytes;

    sg_image tilemap_image;
    binocle_material *tilemap_material;
    struct {
      int tile_id;
      binocle_sprite *sprite;
    }tiles[256];
  } world;

  binocle_camera camera;
  struct binocle_sprite *player;
  kmVec2 player_pos;
  sg_image wabbit_image;
} game_state_t;

void setup_game(game_state_t *gs);
void draw_game();
