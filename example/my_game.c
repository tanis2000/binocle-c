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
  // for (int i = 0; i < 20*9; i++) {
  //   game_state->world.tiles[i].tile_id = i;
  //   game_state->world.tiles[i].sprite =
  //     binocle_sprite_from_material(game_state->world.tilemap_material);
  //   const int row = i / 20;
  //   game_state->world.tiles[i].sprite->subtexture = binocle_subtexture_with_texture(&game_state->world.tilemap_image, 18*(i - row * 20) + 1, row * 18 + 1, 18, 18);
  // }
  return true;
}

void setup_game(game_state_t *gs) {
  game_state = gs;
  game_state->world.level_data = binocle_memory_push_size(game_state->engine.main_arena, 1000000, binocle_memory_default_arena_params());
  if (!load_map("assets/maps/example.ldtk", &game_state->world.level_data, &game_state->world.level_size_in_bytes)) {
    binocle_log_error("Failed to load map");
  }
  new_player();
}

void entity_update(entity_t *e, float dt) {
  binocle_sprite_update(e->graphics.sprite, dt);
}

void update_game(float dt) {
  entity_update(&game_state->world.entities[0], dt);
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
  struct layerInstances *collisions = getLayer("IntGrid",level->uid);
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
