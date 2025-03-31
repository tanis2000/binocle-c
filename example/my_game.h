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
struct layerInstances;

#define GRID 18
#define MAX_ENTITIES 256
#define PLATFORM_END_RIGHT 1
#define PLATFORM_END_LEFT 2

typedef enum e_entity_type {
  PLAYER,
  MOB,
} e_entity_type;

typedef struct transform_t {
  int64_t cx, cy;
  float xr, yr;
} transform_t;

typedef struct physics_t {
  float dx, dy;
  float bdx, bdy;
  float gravity;
  float frict;
  float bump_frict;
  float hei, wid;
  float radius;
  float time_mul;
  bool has_collisions;
} physics_t;

typedef struct graphics_t {
  float depth;
  float pivot_x, pivot_y;
  binocle_sprite *sprite;
  binocle_material *material;
  sg_image texture;
  float sprite_x, sprite_y;
  float sprite_scale_x, sprite_scale_y;
  float sprite_scale_set_x, sprite_scale_set_y;
  bool visible;
  int8_t dir;
} graphics_t;

typedef struct health_t {
  float max;
  float current;
} health_t;

typedef struct player_t {
  health_t health;
} player_t;

typedef struct mob_t {
} mob_t;

typedef struct entity_t {
  e_entity_type type;
  transform_t transform;
  physics_t physics;
  graphics_t graphics;
  union {
    player_t player;
    mob_t mob;
  } data;
} entity_t;

typedef struct level_t {
  struct layerInstances *collision_layer;
  int64_t marks_map[1024*1024];
} level_t;

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
    } tiles[256];
    entity_t entities[MAX_ENTITIES];
    uint64_t num_entities;
    entity_t *player;
    level_t level;
  } world;

  binocle_camera camera;
  struct binocle_sprite *player;
  kmVec2 player_pos;
  sg_image wabbit_image;
} game_state_t;

void setup_game(game_state_t *gs);
void update_game(float dt);
void draw_game();
entity_t *new_player();
