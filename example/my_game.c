#include "my_game.h"

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
}

void update_game() {

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

void draw_game() {
  draw_map();
}





