//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SPRITE_H
#define BINOCLE_SPRITE_H

#include "binocle_render_state.h"
#include "binocle_subtexture.h"
#include "binocle_vpct.h"
#include <kazmath/kazmath.h>
#include <stdbool.h>
#include <stdlib.h>

#define BINOCLE_SPRITE_MAX_FRAMES 256
#define BINOCLE_SPRITE_MAX_ANIMATIONS 16

struct binocle_camera;
struct binocle_material;
struct binocle_gd;
struct binocle_texture;

typedef enum binocle_sprite_sort_mode
{
  /// <summary>
  /// All sprites are drawing when <see cref="SpriteBatch.End"/> invokes, in order of draw call sequence. Depth is ignored.
  /// </summary>
  BINOCLE_SPRITE_SORT_MODE_DEFERRED,
  /// <summary>
  /// Each sprite is drawing at individual draw call, instead of <see cref="SpriteBatch.End"/>. Depth is ignored.
  /// </summary>
  BINOCLE_SPRITE_SORT_MODE_IMMEDIATE,
  /// <summary>
  /// Same as <see cref="SpriteSortMode.Deferred"/>, except sprites are sorted by texture prior to drawing. Depth is ignored.
  /// </summary>
  BINOCLE_SPRITE_SORT_MODE_TEXTURE,
  /// <summary>
  /// Same as <see cref="SpriteSortMode.Deferred"/>, except sprites are sorted by depth in back-to-front order prior to drawing.
  /// </summary>
  BINOCLE_SPRITE_SORT_MODE_BACK_TO_FRONT,
  /// <summary>
  /// Same as <see cref="SpriteSortMode.Deferred"/>, except sprites are sorted by depth in front-to-back order prior to drawing.
  /// </summary>
  BINOCLE_SPRITE_SORT_MODE_FRONT_TO_BACK
} binocle_sprite_sort_mode;


typedef struct binocle_sprite_frame {
  binocle_subtexture *subtexture;
  kmVec2 origin;
} binocle_sprite_frame;

typedef struct binocle_sprite_animation_frame_mapping {
  char *name;
  int original_frame;
  int real_frame;
} binocle_sprite_animation_frame_mapping;

typedef struct binocle_sprite_animation {
  bool enabled;
  int frames[BINOCLE_SPRITE_MAX_FRAMES];
  float delays[BINOCLE_SPRITE_MAX_FRAMES];
  bool looping;
  int frames_number;
  char *name;
  binocle_sprite_animation_frame_mapping
    frame_mapping[BINOCLE_SPRITE_MAX_FRAMES];
  int frame_mapping_number;
} binocle_sprite_animation;

typedef struct binocle_sprite {
  binocle_subtexture subtexture;
  struct binocle_material *material;
  kmVec2 origin;
  binocle_sprite_animation *animations; //[BINOCLE_SPRITE_MAX_ANIMATIONS];
  binocle_sprite_frame *frames; //[BINOCLE_SPRITE_MAX_FRAMES];
  int frames_number;
  bool playing;
  bool finished;
  float rate;
  int current_frame;
  binocle_sprite_animation *current_animation;
  int animations_number;
  int current_animation_id;
  int current_animation_frame;
  float timer;
} binocle_sprite;

typedef struct binocle_sprite_batch_item {
  struct binocle_texture *texture;
  binocle_vpct vertex_tl;
  binocle_vpct vertex_tr;
  binocle_vpct vertex_bl;
  binocle_vpct vertex_br;
  float sort_key;

} binocle_sprite_batch_item;

typedef struct binocle_sprite_batcher {
  uint64_t initial_batch_size;
  uint64_t max_batch_size;
  uint64_t initial_vertex_array_size;
  binocle_sprite_batch_item *batch_item_list;
  uint64_t batch_item_list_size;
  uint64_t batch_item_list_capacity;
  uint64_t *index;
  uint64_t index_size;
  uint64_t index_capacity;
  binocle_vpct *vertex_array;
  uint64_t vertex_array_size;
  uint64_t vertex_array_capacity;
} binocle_sprite_batcher;

typedef struct binocle_sprite_batch {
  binocle_sprite_batcher batcher;
  binocle_render_state render_state;
  struct binocle_gd *gd;
  bool begin_called;
  kmMat4 matrix;
  kmAABB2 temp_rect;
  kmVec2 tex_coord_tl;
  kmVec2 tex_coord_br;
  kmVec2 scaled_origin;
  kmAABB2 origin_rect;
  binocle_sprite_sort_mode sort_mode;
  kmAABB2 cull_rect;
  kmVec2 vertex_to_cull_tl;
  kmVec2 vertex_to_cull_tr;
  kmVec2 vertex_to_cull_bl;
  kmVec2 vertex_to_cull_br;
} binocle_sprite_batch;

binocle_sprite binocle_sprite_from_material(struct binocle_material *material);
void binocle_sprite_destroy(struct binocle_sprite *sprite);
void binocle_sprite_draw(binocle_sprite sprite, struct binocle_gd *gd,
                         int64_t x, int64_t y, kmAABB2 viewport, float rotation,
                         kmVec2 scale, struct binocle_camera *camera);
void binocle_sprite_add_frame(binocle_sprite *sprite,
                              binocle_sprite_frame frame);
void binocle_sprite_add_animation(binocle_sprite *sprite, int id, int frame);
void binocle_sprite_add_animation_with_frames(binocle_sprite *sprite, int id,
                                              bool looping, float delay,
                                              int frames[], int frames_count);
void binocle_sprite_play(binocle_sprite *sprite, int id, bool restart);
void binocle_sprite_play_from_frame(binocle_sprite *sprite, int id,
                                    int start_frame, bool restart);
void binocle_sprite_stop(binocle_sprite *sprite);
void binocle_sprite_update(binocle_sprite *sprite, float dt);
int binocle_sprite_get_current_frame(binocle_sprite *sprite);
void binocle_sprite_set_current_frame(binocle_sprite *sprite, int frame);
int binocle_sprite_get_current_animation(binocle_sprite *sprite);
void binocle_sprite_set_current_animation(binocle_sprite *sprite, int id);
void binocle_sprite_clear_animations(binocle_sprite *sprite);
void binocle_sprite_clear_frames(binocle_sprite *sprite);

// sequence code format:
// startFrame-endFrame:time(chance)
// time: also be set to "forever" - this will loop the sequence indefinitely
// chance: float value from 0-1, chance that the sequence will play (if not
// played, it will be skipped) time and chance can both be ignored, this will
// mean the sequence plays through once
//
// sequence code examples:
// TV: 0-1:3, 2-3:3, 4-5:4, 6-7:4, 8:3, 9:3
// Idle animation with random fidgets: 0-59, 60-69, 10-59, 0-59(.25),
// 70-129(.75) Jump animation with looping finish: 0-33, 20-33:forever
void binocle_sprite_create_animation(binocle_sprite *sprite, char *name,
                                     char *subtextures_names,
                                     char *sequence_code, bool loop,
                                     binocle_subtexture *subtextures,
                                     size_t subtextures_count);
void binocle_sprite_play_animation(binocle_sprite *sprite, char *name,
                                   bool restart);

// binocle_sprite_frame stuff
binocle_sprite_frame
binocle_sprite_frame_from_subtexture(struct binocle_subtexture *subtexture);
binocle_sprite_frame binocle_sprite_frame_from_subtexture_and_origin(
  struct binocle_subtexture *subtexture, kmVec2 origin);

binocle_sprite_batch_item binocle_sprite_batch_item_new();
void binocle_sprite_batch_item_set(binocle_sprite_batch_item *item, float x, float y, float dx, float dy, float w, float h, float sin, float cos, binocle_color color, kmVec2 tex_coord_tl, kmVec2 tex_coord_br, float depth, struct binocle_texture *texture);

binocle_sprite_batcher binocle_sprite_batcher_new();
binocle_sprite_batch_item *binocle_sprite_batcher_create_batch_item(binocle_sprite_batcher *batcher);
void binocle_sprite_batcher_ensure_array_capacity(binocle_sprite_batcher *batcher, uint64_t num_batch_items);
void binocle_sprite_batcher_draw_batch(binocle_sprite_batcher *batcher, binocle_sprite_sort_mode sort_mode, struct binocle_render_state *render_state, struct binocle_gd *gd);
void binocle_sprite_batcher_flush_vertex_array(binocle_sprite_batcher *batcher, uint64_t start, uint64_t end, struct binocle_texture *texture, struct binocle_render_state *render_state, struct binocle_gd *gd);
binocle_sprite_batch binocle_sprite_batch_new();
void binocle_sprite_batch_compute_cull_rectangle(binocle_sprite_batch *batch, kmAABB2 viewport);
void binocle_sprite_batch_begin(binocle_sprite_batch *batch, kmAABB2 viewport, binocle_sprite_sort_mode sort_mode, struct binocle_shader *shader, kmMat4 *transform_matrix);
void binocle_sprite_batch_end(binocle_sprite_batch *batch, kmAABB2 viewport);
void binocle_sprite_batch_setup(binocle_sprite_batch *batch, kmAABB2 viewport);
void binocle_sprite_batch_draw_internal(binocle_sprite_batch *batch, struct binocle_texture *texture, kmAABB2 *source_rectangle, binocle_color color, float rotation, float depth, bool auto_flush);
void binocle_sprite_batch_flush_if_needed(binocle_sprite_batch *batch);
void binocle_sprite_batch_draw(binocle_sprite_batch *batch, struct binocle_texture *texture, kmVec2 *position,
                               kmAABB2 *destination_rectangle,
                               kmAABB2 *source_rectangle, kmVec2 *origin,
                               float rotation, kmVec2 *scale, binocle_color color,
                               float layer_depth);
void binocle_sprite_batch_draw_vector_scale(binocle_sprite_batch *batch, struct binocle_texture *texture, kmVec2 *position,
                                            kmAABB2 *source_rectangle, binocle_color color,
                                            float rotation, kmVec2 origin, kmVec2 scale,
                                            float layer_depth);
void binocle_sprite_batch_draw_float_scale(binocle_sprite_batch *batch, struct binocle_texture *texture, kmVec2 position,
                                           kmAABB2 source_rectangle, binocle_color color,
                                           float rotation, kmVec2 origin, float scale,
                                           float layer_depth);
void binocle_sprite_batch_draw_position(binocle_sprite_batch *batch, struct binocle_texture *texture, kmVec2 position);
void
binocle_sprite_batch_draw_noscale(binocle_sprite_batch *batch, struct binocle_texture *texture, kmAABB2 destination_rectangle,
                                  kmAABB2 *source_rectangle, binocle_color color,
                                  float rotation, kmVec2 origin,
                                  float layer_depth);
void
binocle_sprite_draw_dst_src_color(binocle_sprite_batch *batch, struct binocle_texture *texture, kmAABB2 destination_rectangle,
                                  kmAABB2 source_rectangle, binocle_color color);

#endif // BINOCLE_SPRITE_H
