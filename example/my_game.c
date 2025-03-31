#include "my_game.h"

#include "binocle_atlas.h"
#include "binocle_fs.h"
#include "binocle_image.h"
#include "binocle_log.h"
#include "binocle_memory.h"
#include "binocle_sprite.h"
#include "binocle_viewport_adapter.h"
#include "cLDtk.h"
#include "constants.h"

game_state_t *game_state;

bool level_is_valid(level_t *level, int64_t cx, int64_t cy) {
  return cx >= 0 && cx < level->collision_layer->cWid &&
          cy >= 0 && cy <= level->collision_layer->cHei;
}

int64_t level_coord_id(level_t *level, int64_t cx, int64_t cy) {
  int64_t cy_inverted = level->collision_layer->cHei - 1 - cy;
  return cx + 1 + cy_inverted * level->collision_layer->cWid;
}

bool level_has_collision(level_t *level, int64_t cx, int64_t cy) {
  if (!level_is_valid(level, cx, cy)) {
    return true;
  }

  int64_t idx = level_coord_id(level, cx, cy);
  int value = level->collision_layer->intGrid[idx];
  if (value == 1 || value == 3) {
    return true;
  }
  return false;
}

bool level_has_wall_collision(level_t *level, int64_t cx, int64_t cy) {
  if (!level_is_valid(level, cx, cy)) {
    return true;
  }

  int64_t idx = level_coord_id(level, cx, cy);
  int value = level->collision_layer->intGrid[idx];
  if (value == 1) {
    return true;
  }
  return false;
}


void level_set_mark(level_t *level, int64_t cx, int64_t cy, int64_t value) {
  if (level_is_valid(level, cx, cy)) {
    level->marks_map[level_coord_id(level, cx, cy)] = value;
  }
}

bool load_map(const char *filename, char **buffer, size_t *size) {
  binocle_log_info("Loading map");
  if (!binocle_fs_load_text_file(filename, buffer, size)) {
    return false;
  }
  ldtk_load_json_from_memory(*buffer);
  ldtk_import_map_data();
  binocle_image_load_desc desc = {
    .filename = "assets/sheets/tilemap.png",
    .filter = SG_FILTER_LINEAR,
    .wrap = SG_WRAP_CLAMP_TO_EDGE,
    .fs = BINOCLE_FS_PHYSFS,
  };
  game_state->world.tilemap_image = binocle_image_load_with_desc(&desc);
  game_state->world.tilemap_material = binocle_material_new();
  game_state->world.tilemap_material->albedo_texture = game_state->world.tilemap_image;
  game_state->world.tilemap_material->shader = game_state->engine.default_shader;
  // 20x9 tileset
  int i = 0;
  for (int y = 0; y < 9 ; y++) {
    for (int x = 0 ; x < 20 ; x++) {
      game_state->world.tiles[i].tile_id = i;
      game_state->world.tiles[i].sprite =
        binocle_sprite_from_material(game_state->world.tilemap_material);
      game_state->world.tiles[i].sprite->subtexture = binocle_subtexture_with_texture(
        &game_state->world.tilemap_image, x * (18 + 1), (9 - 1) * (18 + 1) - y * (18 + 1), 18, 18);
      i++;
    }
 }

  const struct levels *level = ldtk_get_level("Level_0");
  struct layerInstances *collisions = ldtk_get_layer("IntGrid",level->uid);
  game_state->world.level.collision_layer = collisions;
  for (int cy = 0 ; cy < collisions->cHei ; cy++) {
    for (int cx = 0 ; cx < collisions->cWid ; cx++) {
      if (!level_has_collision(&game_state->world.level, cx, cy) && level_has_collision(&game_state->world.level, cx, cy-1)) {
        if (!level_has_collision(&game_state->world.level, cx+1, cy) || !level_has_collision(&game_state->world.level, cx+1, cy-1)) {
          level_set_mark(&game_state->world.level, cx, cy, PLATFORM_END_RIGHT);
        }
        if (!level_has_collision(&game_state->world.level, cx-1, cy) || !level_has_collision(&game_state->world.level, cx-1, cy-1)) {
          level_set_mark(&game_state->world.level, cx, cy, PLATFORM_END_LEFT);
        }
      }
    }
  }

  // for (int i = 0; i < 20*9; i++) {
  //   game_state->world.tiles[i].tile_id = i;
  //   game_state->world.tiles[i].sprite =
  //     binocle_sprite_from_material(game_state->world.tilemap_material);
  //   const int row = i / 20;
  //   game_state->world.tiles[i].sprite->subtexture = binocle_subtexture_with_texture(&game_state->world.tilemap_image, 18*(i - row * 20) + 1, row * 18 + 1, 18, 18);
  // }
  return true;
}

void entity_set_pos_grid(entity_t *e, int64_t cx, int64_t cy) {
  e->transform.cx = cx;
  e->transform.cy = cy;
  e->transform.xr = 0.5f;
  e->transform.yr = 0;
}

void setup_game(game_state_t *gs) {
  game_state = gs;
  game_state->world.level_data = binocle_memory_push_size(game_state->engine.main_arena, 1000000, binocle_memory_default_arena_params());
  if (!load_map("assets/maps/example.ldtk", &game_state->world.level_data, &game_state->world.level_size_in_bytes)) {
    binocle_log_error("Failed to load map");
  }
  game_state->world.player = new_player();
  entity_set_pos_grid(game_state->world.player, 10, 10);
}

void entity_pre_update(entity_t *e, float dt) {
  // TODO: update cooldowns
}

void entity_on_touch_wall(entity_t *e, int64_t dir) {
  // TODO: change this to be some kind of callback function or something better
}

void entity_on_land(entity_t *e) {
  // TODO: change this to be some kind of callback function or something better
}

void entity_on_pre_step_x(entity_t *e) {
  // Right collisions
  if (e->physics.has_collisions && e->transform.xr > 0.8f && level_has_wall_collision(&game_state->world.level, e->transform.cx+1, e->transform.cy)) {
    entity_on_touch_wall(e, 1);
    e->transform.xr = 0.8f;
  }

  // Left collisions
  if (e->physics.has_collisions && e->transform.xr < 0.2f && level_has_wall_collision(&game_state->world.level, e->transform.cx-1, e->transform.cy)) {
    entity_on_touch_wall(e, -1);
    e->transform.xr = 0.2f;
  }
}

void entity_on_pre_step_y(entity_t *e) {
  // Down collisions
  if (e->physics.has_collisions && e->transform.yr < 0.0f && level_has_collision(&game_state->world.level, e->transform.cx, e->transform.cy-1)) {
    e->physics.dy = 0;
    e->transform.yr = 0.0f;
    e->physics.bdx *= 0.5f;
    e->physics.bdx = 0;
    e->transform.xr = 0.8f;
    entity_on_land(e);
  }

  // Up collisions
  if (e->physics.has_collisions && e->transform.yr > 0.5f && level_has_wall_collision(&game_state->world.level, e->transform.cx, e->transform.cy+1)) {
    e->transform.yr = 0.5f;
  }
}

bool entity_on_ground(entity_t *e) {
  return level_has_wall_collision(&game_state->world.level, e->transform.cx, e->transform.cy-1) && e->transform.yr == 0 && e->physics.dy <= 0;
}

void entity_update(entity_t *e, float dt) {
  // X
  int64_t steps = (int64_t)ceilf(fabsf(e->physics.dx + e->physics.bdx) * e->physics.time_mul);
  float step = ((e->physics.dx + e->physics.bdx) * e->physics.time_mul) / steps;
  while (steps > 0) {
    e->transform.xr += (float)step;
    entity_on_pre_step_x(e);
    while (e->transform.xr > 1) {
      e->transform.xr -= 1;
      e->transform.cx += 1;
    }
    while (e->transform.xr < 0) {
      e->transform.xr += 1;
      e->transform.cx -= 1;
    }
    steps--;
  }
  e->physics.dx *= powf(e->physics.frict, e->physics.time_mul);
  e->physics.bdx *= powf(e->physics.bump_frict, e->physics.time_mul);
  if (fabsf(e->physics.dx) <= 0.0005 * e->physics.time_mul) {
    e->physics.dx = 0.0f;
  }
  if (fabsf(e->physics.bdx) <= 0.0005 * e->physics.time_mul) {
    e->physics.bdx = 0.0f;
  }

  // Y
  if (!entity_on_ground(e)) {
    e->physics.dy -= e->physics.gravity;
  }
  steps = (int64_t)ceilf(fabsf(e->physics.dy + e->physics.bdy) * e->physics.time_mul);
  step = ((e->physics.dy + e->physics.bdy) * e->physics.time_mul) / steps;
  while (steps > 0) {
    e->transform.yr += step;
    entity_on_pre_step_y(e);
    while (e->transform.yr > 1) {
      e->transform.yr -= 1;
      e->transform.cy += 1;
    }
    while (e->transform.yr < 0) {
      e->transform.yr += 1;
      e->transform.cy -= 1;
    }
    steps--;
  }
  e->physics.dy *= powf(e->physics.frict, e->physics.time_mul);
  e->physics.bdy *= powf(e->physics.bump_frict, e->physics.time_mul);
  if (fabsf(e->physics.dy) <= 0.0005 * e->physics.time_mul) {
    e->physics.dy = 0.0f;
  }
  if (fabsf(e->physics.bdy) <= 0.0005 * e->physics.time_mul) {
    e->physics.bdy = 0.0f;
  }

  // Animations
  binocle_sprite_update(e->graphics.sprite, dt);
}

void entity_post_update(entity_t *e, float dt) {
  e->graphics.sprite_x = ((float)e->transform.cx + e->transform.xr) * GRID;
  e->graphics.sprite_y = ((float)e->transform.cy + e->transform.yr) * GRID;
  e->graphics.sprite_scale_x = e->graphics.dir * e->graphics.sprite_scale_set_x;
  e->graphics.sprite_scale_y = e->graphics.sprite_scale_set_y;
}

bool  player_is_alive(entity_t *e) {
  return e->data.player.health.current > 0;
}

void player_update(entity_t *e, float dt) {
  float spd = 2.0f;
  if (player_is_alive(e) && (binocle_input_is_key_pressed(&game_state->engine.input, KEY_LEFT) || binocle_input_is_key_pressed(&game_state->engine.input, KEY_A))) {
    e->physics.dx -= spd * dt * e->physics.time_mul;
    e->graphics.dir = -1;
  } else if (player_is_alive(e) && (binocle_input_is_key_pressed(&game_state->engine.input, KEY_RIGHT) || binocle_input_is_key_pressed(&game_state->engine.input, KEY_D))) {
    e->physics.dx += spd * dt * e->physics.time_mul;
    e->graphics.dir = 1;
  }

  if (player_is_alive(e) && (binocle_input_is_key_pressed(&game_state->engine.input, KEY_UP) || binocle_input_is_key_pressed(&game_state->engine.input, KEY_W))) {
    if (entity_on_ground(e)) {
      e->physics.dy = 0.9f;
      // TODO: play the jump sound
      // TODO: spawn the jump particles
    }
  }

  if (player_is_alive(e) && binocle_input_is_key_pressed(&game_state->engine.input, KEY_E)) {
    // TODO: implement shooting
  }
}

void update_game(float dt) {
  entity_update(&game_state->world.entities[0], dt);
  player_update(game_state->world.player, dt);
  entity_post_update(&game_state->world.entities[0], dt);
}

void draw_tiles(struct layerInstances *layer, sg_image image, int offset) {
  kmAABB2 viewport;
  viewport.min.x = 0;
  viewport.min.y = 0;
  viewport.max.x = DESIGN_WIDTH;
  viewport.max.y = DESIGN_HEIGHT;

  kmVec2 scale;
  scale.x = 1.0f;
  scale.y = 1.0f;

  sg_color white = binocle_color_white();

  for(int y=layer->autoTiles_data_ptr->count ;y-- > 0;) {
    struct autoTiles tiles = layer->autoTiles_data_ptr[y];
    binocle_sprite_draw(game_state->world.tiles[tiles.t].sprite, &game_state->engine.gd,
      tiles.x, 288-18-tiles.y, &viewport, 0, &scale, &game_state->camera, 0, &white);
  }
}

void draw_map() {
  const struct levels *level = ldtk_get_level("Level_0");
  struct layerInstances *collisions = ldtk_get_layer("IntGrid",level->uid);
  draw_tiles(collisions, game_state->world.tilemap_image, 18);
}

void draw_entity(entity_t *e) {
  kmAABB2 viewport;
  viewport.min.x = 0;
  viewport.min.y = 0;
  viewport.max.x = DESIGN_WIDTH;
  viewport.max.y = DESIGN_HEIGHT;
  kmVec2 scale = {.x = e->graphics.sprite_scale_x,
                  .y = e->graphics.sprite_scale_y};
  if (e->graphics.visible && e->graphics.sprite != NULL) {
    binocle_sprite_draw(e->graphics.sprite, &game_state->engine.gd,
      e->graphics.sprite_x, e->graphics.sprite_y, &viewport, 0, &scale,
      &game_state->camera, e->graphics.depth, NULL);
  }
}

void draw_game() {
  draw_map();
  draw_entity(&game_state->world.entities[0]);
}

entity_t *new_entity(e_entity_type type) {
  entity_t *e = &game_state->world.entities[game_state->world.num_entities];
  game_state->world.num_entities++;
  e->transform.cx = 0;
  e->transform.cy = 0;
  e->transform.xr = 0.5f;
  e->transform.yr = 0;

  e->physics.dx = 0.0f;
  e->physics.dy = 0.0f;
  e->physics.bdx = 0.0f;
  e->physics.bdy = 0.0f;
  e->physics.gravity = 0.025f;
  e->physics.frict = 0.82f;
  e->physics.bump_frict = 0.93f;
  e->physics.hei = GRID;
  e->physics.wid = GRID;
  e->physics.radius = GRID / 2.0f;
  e->physics.time_mul = 1.0f;
  e->physics.has_collisions = true;

  e->graphics.depth = 0;
  e->graphics.pivot_x = 0.5f;
  e->graphics.pivot_y = 0;
  e->graphics.sprite = NULL;
  e->graphics.material = NULL;
  e->graphics.texture.id = 0;
  e->graphics.sprite_x = 0;
  e->graphics.sprite_y = 0;
  e->graphics.sprite_scale_x = 1.0f;
  e->graphics.sprite_scale_y = 1.0f;
  e->graphics.sprite_scale_set_x = 1.0f;
  e->graphics.sprite_scale_set_y = 1.0f;
  e->graphics.visible = true;
  e->graphics.dir = 1;

  return e;
}

entity_t *new_player() {
  entity_t *e = new_entity(PLAYER);
  e->physics.hei = 24;
  e->physics.wid = 24;
  e->graphics.depth = 1;
  e->data.player.health.max = 10;
  e->data.player.health.current = e->data.player.health.max;
  binocle_image_load_desc img_desc = {
    .filename = "assets/sheets/entities.png",
    .filter = SG_FILTER_LINEAR,
    .wrap = SG_WRAP_CLAMP_TO_EDGE,
    .fs = BINOCLE_FS_PHYSFS,
  };
  e->graphics.texture = binocle_image_load_with_desc(&img_desc);
  e->graphics.material = binocle_material_new();
  e->graphics.material->albedo_texture = e->graphics.texture;
  e->graphics.material->shader = game_state->engine.default_shader;
  e->graphics.sprite = binocle_sprite_from_material(e->graphics.material);
  binocle_subtexture subs[256];
  int subs_count = 256;
  binocle_atlas_texturepacker atlas;
  binocle_atlas_texturepacker_load_desc desc = {
    .filename = "assets/sheets/entities.json",
    .fs = BINOCLE_FS_PHYSFS,
  };
  binocle_atlas_load_texturepacker(&atlas, &desc);
  binocle_atlas_texturepacker_create_subtextures(&atlas, &e->graphics.texture, &subs[0], &subs_count);
  for (int i = 0 ; i < subs_count ; i++) {
    binocle_sprite_frame frame = binocle_sprite_frame_from_subtexture(&subs[i]);
    binocle_sprite_add_frame(e->graphics.sprite, frame);
  }
  if (atlas.meta.frame_tags != NULL) {
    for (int i = 0 ; i < atlas.meta.num_frame_tags ; i++) {
      int num_frames = atlas.meta.frame_tags[i].to - atlas.meta.frame_tags[i].from + 1;
      if (num_frames == 0) {
        continue;
      }
      char subs_names[4096] = {0};
      for (int j = 0; j < num_frames; j++) {
        int sprite_num = atlas.meta.frame_tags[i].from + j;
        if (j == 0) {
          sprintf(subs_names, "%s", e->graphics.sprite->frames[sprite_num].subtexture->name);
        } else {
          sprintf(subs_names, "%s,%s", subs_names, e->graphics.sprite->frames[sprite_num].subtexture->name);
        }
      }
      char seq[16] = {0};
      sprintf(seq, "%d-%d:%.2f", 0, atlas.meta.frame_tags[i].to - atlas.meta.frame_tags[i].from, 1.0f/6.0f);
      binocle_sprite_create_animation(e->graphics.sprite, atlas.meta.frame_tags[i].name, subs_names, seq, true,
                                      subs, subs_count);
    }
  }
  e->graphics.sprite->origin.x = e->graphics.pivot_x * e->physics.wid;
  e->graphics.sprite->origin.y = e->graphics.pivot_y * e->physics.hei;
  binocle_sprite_play_animation(e->graphics.sprite, "hero_run", false);
  return e;
}


