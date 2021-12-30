//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SPRITE_H
#define BINOCLE_SPRITE_H

#include "binocle_render_state.h"
#include "binocle_subtexture.h"
#include "backend/binocle_vpct.h"
#include <kazmath/kazmath.h>
#include <stdbool.h>
#include <stdlib.h>

#define BINOCLE_SPRITE_MAX_FRAMES 256
#define BINOCLE_SPRITE_MAX_ANIMATIONS 16

struct binocle_camera;
struct binocle_material;
struct binocle_gd;
struct binocle_image;

/**
 * The sort mode used when running through the sprite batcher
 */
typedef enum binocle_sprite_sort_mode
{
  /// All sprites gets drawn when \see binocle_sprite_batch_end is being invoked, in order of draw call sequence.
  /// Depth is ignored.
  BINOCLE_SPRITE_SORT_MODE_DEFERRED,
  /// Each sprite is being drawn with an individual draw call, instead of \see binocle_sprite_batch_end. Depth is ignored.
  BINOCLE_SPRITE_SORT_MODE_IMMEDIATE,
  /// Same as \see binocle_sprite_batch_end, except sprites are sorted by texture prior to drawing. Depth is ignored.
  BINOCLE_SPRITE_SORT_MODE_TEXTURE,
  /// Same as \see binocle_sprite_batch_end, except sprites are sorted by depth in back-to-front order prior to drawing.
  BINOCLE_SPRITE_SORT_MODE_BACK_TO_FRONT,
  /// Same as \see binocle_sprite_batch_end, except sprites are sorted by depth in front-to-back order prior to drawing.
  BINOCLE_SPRITE_SORT_MODE_FRONT_TO_BACK
} binocle_sprite_sort_mode;

/**
 * The single frame of a sprite, mainly used within animations
 */
typedef struct binocle_sprite_frame {
  binocle_subtexture *subtexture;
  kmVec2 origin;
} binocle_sprite_frame;

/**
 * The mapping of a frame within an animation
 */
typedef struct binocle_sprite_animation_frame_mapping {
  char *name;
  int original_frame;
  int real_frame;
} binocle_sprite_animation_frame_mapping;

/**
 * A sprite animation
 */
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

/**
 * A sprite
 */
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

/**
 * An item of the sprite batch
 */
typedef struct binocle_sprite_batch_item {
  struct binocle_image *texture;
  binocle_vpct vertex_tl;
  binocle_vpct vertex_tr;
  binocle_vpct vertex_bl;
  binocle_vpct vertex_br;
  float sort_key;

} binocle_sprite_batch_item;

/**
 * The sprite batcher
 */
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

/**
 * A sprite batch
 */
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

/**
 * \brief Creates a sprite from a material
 * @param material the material
 * @return the sprite
 */
binocle_sprite *binocle_sprite_from_material(struct binocle_material *material);

/**
 * \brief Releases all the resources of a sprite
 * @param sprite the sprite
 */
void binocle_sprite_destroy(struct binocle_sprite *sprite);

/**
 * \brief Draws a sprite in immediate mode.
 * The sprite gets drawn as soon as this function is called.
 * @param sprite the sprite
 * @param gd the graphics device instance
 * @param x the X coordinate
 * @param y the Y coordinate
 * @param viewport the viewport to apply
 * @param rotation the rotation
 * @param scale the scale
 * @param camera the camera to apply
 */
void binocle_sprite_draw(binocle_sprite *sprite, struct binocle_gd *gd, int64_t x, int64_t y, kmAABB2 *viewport, float rotation,
                         kmVec2 *scale, struct binocle_camera *camera);

/**
 * \brief Adds a frame of an animation to a sprite
 * @param sprite the sprite
 * @param frame the frame of the animation to add
 */
void binocle_sprite_add_frame(binocle_sprite *sprite,
                              binocle_sprite_frame frame);

/**
 * \brief Adds an animation with just one frame to a sprite
 * @param sprite the sprite
 * @param id the ID of the animation
 * @param frame the frame
 */
void binocle_sprite_add_animation(binocle_sprite *sprite, int id, int frame);

/**
 * \brief Adds an animation with more than one frame to a sprite
 * @param sprite the sprite
 * @param id the ID of the animation
 * @param looping true if the animation is looping
 * @param delay the delay between each frame
 * @param frames the frames IDs
 * @param frames_count the number of frames
 */
void binocle_sprite_add_animation_with_frames(binocle_sprite *sprite, int id,
                                              bool looping, float delay,
                                              int frames[], int frames_count);

/**
 * \brief Plays an animation
 * @param sprite the sprite
 * @param id the ID of the animation
 * @param restart true if the animation should restart from the beginning if it's already running
 */
void binocle_sprite_play(binocle_sprite *sprite, int id, bool restart);

/**
 * \brief Plays an animation starting from the given frame
 * @param sprite the sprite
 * @param id the ID of the animation
 * @param start_frame the index of the frame to start from
 * @param restart true if the animation should restart from the beginning if it's already running
 */
void binocle_sprite_play_from_frame(binocle_sprite *sprite, int id,
                                    int start_frame, bool restart);

/**
 * \brief Stops playing any animation if running
 * @param sprite the sprite
 */
void binocle_sprite_stop(binocle_sprite *sprite);

/**
 * \brief Updates the animation internals
 * This advances the animation as needed, given the time that has passed since the last update.
 * @param sprite the sprite
 * @param dt the delta time passed since the last call, in seconds
 */
void binocle_sprite_update(binocle_sprite *sprite, float dt);

/**
 * \brief Gets the index of the current frame being played
 * @param sprite the sprite
 * @return the index of the current frame
 */
int binocle_sprite_get_current_frame(binocle_sprite *sprite);

/**
 * \brief Sets the current frame to display
 * @param sprite the sprite
 * @param frame the index of the frame
 */
void binocle_sprite_set_current_frame(binocle_sprite *sprite, int frame);

/**
 * \brief Sets the current frame to display by name
 * @param sprite the sprite
 * @param name the name of the frame as set in its subtexture
 */
void binocle_sprite_set_current_frame_by_name(binocle_sprite *sprite, const char *name);

/**
 * \brief Gets the ID of the current animation
 * @param sprite the sprite
 * @return the ID of the current animation
 */
int binocle_sprite_get_current_animation(binocle_sprite *sprite);

/**
 * \brief Force sets the current animation
 * @param sprite the sprite
 * @param id the ID of the animation
 */
void binocle_sprite_set_current_animation(binocle_sprite *sprite, int id);

/**
 * \brief Removes all the animations of the sprite
 * @param sprite the sprite
 */
void binocle_sprite_clear_animations(binocle_sprite *sprite);

/**
 * \brief Removes all the frames of the sprite
 * @param sprite the sprite
 */
void binocle_sprite_clear_frames(binocle_sprite *sprite);

/**
 * \brief Creates a new animation
 *
 * sequence code format:
 * startFrame-endFrame:time(chance)
 * time: also be set to "forever" - this will loop the sequence indefinitely
 * chance: float value from 0-1, chance that the sequence will play (if not
 * played, it will be skipped) time and chance can both be ignored, this will
 * mean the sequence plays through once
 *
 * sequence code examples:
 * TV: 0-1:3, 2-3:3, 4-5:4, 6-7:4, 8:3, 9:3
 * Idle animation with random fidgets: 0-59, 60-69, 10-59, 0-59(.25),
 * 70-129(.75) Jump animation with looping finish: 0-33, 20-33:forever
 *
 * @param sprite the sprite
 * @param name the name of the animation
 * @param subtextures_names the names of the subtextures
 * @param sequence_code the sequence to follow
 * @param loop true if the animation is looping
 * @param subtextures the array of subtextures
 * @param subtextures_count the number of subtextures
 */
void binocle_sprite_create_animation(binocle_sprite *sprite, char *name,
                                     char *subtextures_names,
                                     char *sequence_code, bool loop,
                                     binocle_subtexture *subtextures,
                                     size_t subtextures_count);

/**
 * \brief Plays an animation given the name of the animation
 * @param sprite the sprite
 * @param name the name of the animation to play
 * @param restart true if the animation should restart from the beginning if it's already running
 */
void binocle_sprite_play_animation(binocle_sprite *sprite, char *name,
                                   bool restart);

/**
 * \brief Creates a sprite frame from a subtexture
 * The frame will be the same as the subtexture
 * @param subtexture the subtexture
 * @return the frame
 */
binocle_sprite_frame
binocle_sprite_frame_from_subtexture(struct binocle_subtexture *subtexture);

/**
 * \brief Creates a sprite frame from a subtexture and assigns an origin
 * The frame will have the same size as the subtexture but its origin will be set to the one passed as parameter.
 * @param subtexture the subtexture
 * @param origin the origin of the sprite
 * @return the frame
 */
binocle_sprite_frame binocle_sprite_frame_from_subtexture_and_origin(
  struct binocle_subtexture *subtexture, kmVec2 origin);

/**
 * \brief Creates a new sprite batch item
 * @return the sprite batch item
 */
binocle_sprite_batch_item binocle_sprite_batch_item_new();

/**
 * \brief Sets the values of a sprite batch item
 * @param item the sprite batch item
 * @param x the X position
 * @param y the Y position
 * @param dx the delta X position
 * @param dy the delta Y position
 * @param w the width
 * @param h the height
 * @param sin the sin component of the rotation
 * @param cos the cos component of the rotation
 * @param color the color
 * @param tex_coord_tl the texture coordinates of the top-left corner
 * @param tex_coord_br the texture coordinates of the bottom-right corner
 * @param depth the depth of the sprite
 * @param texture the texture
 */
void binocle_sprite_batch_item_set(binocle_sprite_batch_item *item, float x, float y, float dx, float dy, float w, float h, float sin, float cos, binocle_color color, kmVec2 tex_coord_tl, kmVec2 tex_coord_br, float depth, struct binocle_image *texture);

/**
 * \brief Creates a new sprite batcher
 * @return the sprite batcher
 */
binocle_sprite_batcher binocle_sprite_batcher_new();

/**
 * \brief Creates a new sprite batch item within the given sprite batcher
 * @param batcher the sprite batcher
 * @return the sprite batch item
 */
binocle_sprite_batch_item *binocle_sprite_batcher_create_batch_item(binocle_sprite_batcher *batcher);

/**
 * \brief Makes sure that the sprite batcher has enough room for the given number of sprite batch items.
 * If there's not enough room, it will allocate the necessary memory and initialize it as needed.
 * @param batcher the sprite batcher
 * @param num_batch_items the number of sprite batch items
 */
void binocle_sprite_batcher_ensure_array_capacity(binocle_sprite_batcher *batcher, uint64_t num_batch_items);

/**
 * \brief Tells the sprite batcher to draw its batch of items
 * @param batcher the sprite batcher
 * @param sort_mode the sorting mode \see binocle_sprite_sort_mode
 * @param render_state the current render state
 * @param gd the graphics device
 */
void binocle_sprite_batcher_draw_batch(binocle_sprite_batcher *batcher, binocle_sprite_sort_mode sort_mode, struct binocle_render_state *render_state, struct binocle_gd *gd);

/**
 * \brief Tells the sprite batcher to flush an array of vertices
 * @param batcher the sprite batcher
 * @param start the index of the vertex to start from
 * @param end the index of the last vertex
 * @param texture the texture to use
 * @param render_state the current render state
 * @param gd the graphics device
 */
void binocle_sprite_batcher_flush_vertex_array(binocle_sprite_batcher *batcher, uint64_t start, uint64_t end, struct binocle_image *texture, struct binocle_render_state *render_state, struct binocle_gd *gd);

/**
 * \brief Creates a new sprite batch
 * @return the sprite batch
 */
binocle_sprite_batch binocle_sprite_batch_new();

/**
 * \brief Computes the cull rectangle of a prite batch
 * @param batch the sprite batch
 * @param viewport the viewport
 */
void binocle_sprite_batch_compute_cull_rectangle(binocle_sprite_batch *batch, kmAABB2 viewport);

/**
 * \brief Starts the sprite batch
 * This should be called before adding any sprite to the batcher
 * @param batch the sprite batch
 * @param viewport the viewport
 * @param sort_mode the sorting mode
 * @param shader the shader
 * @param transform_matrix the transformation matrix
 */
void binocle_sprite_batch_begin(binocle_sprite_batch *batch, kmAABB2 viewport, binocle_sprite_sort_mode sort_mode, struct binocle_shader *shader, kmMat4 *transform_matrix);

/**
 * \brief Ends a sprite batch
 * @param batch the sprite batch
 * @param viewport the viewport
 */
void binocle_sprite_batch_end(binocle_sprite_batch *batch, kmAABB2 viewport);

/**
 * \brief Performs the setup of a sprite batch
 * @param batch the sprite batch
 * @param viewport the viewport
 */
void binocle_sprite_batch_setup(binocle_sprite_batch *batch, kmAABB2 viewport);

/**
 * \brief Draws a sprite (Used internally)
 * @param batch the sprite batch
 * @param texture the texture
 * @param source_rectangle the source rectangle
 * @param color the color
 * @param rotation the rotation angle
 * @param depth the depth of the sprite
 * @param auto_flush true if this should be flushed as soon as the draw call is issued
 */
void binocle_sprite_batch_draw_internal(binocle_sprite_batch *batch, struct binocle_image *texture, kmAABB2 *source_rectangle, binocle_color color, float rotation, float depth, bool auto_flush);

/**
 * \brief Flushes the batch if the requirements are met
 * @param batch the sprite batch
 */
void binocle_sprite_batch_flush_if_needed(binocle_sprite_batch *batch);

/**
 * \brief Draws a sprite in the batch
 * @param batch the sprite batch
 * @param texture the texture
 * @param position the position
 * @param destination_rectangle the destination rectangle
 * @param source_rectangle the source rectangle
 * @param origin the origin position
 * @param rotation the rotation angle
 * @param scale the scale
 * @param color the color
 * @param layer_depth the depth of the sprite
 */
void binocle_sprite_batch_draw(binocle_sprite_batch *batch, struct binocle_image *texture, kmVec2 *position,
                               kmAABB2 *destination_rectangle,
                               kmAABB2 *source_rectangle, kmVec2 *origin,
                               float rotation, kmVec2 *scale, binocle_color color,
                               float layer_depth);

/**
 * \brief Draws a sprite to the batch
 * @param batch the sprite batch
 * @param texture the texture
 * @param position the position
 * @param source_rectangle the source rectangle
 * @param color the color
 * @param rotation the rotation angle
 * @param origin the origin
 * @param scale the scale
 * @param layer_depth the depth of the sprite
 */
void binocle_sprite_batch_draw_vector_scale(binocle_sprite_batch *batch, struct binocle_image *texture, kmVec2 *position,
                                            kmAABB2 *source_rectangle, binocle_color color,
                                            float rotation, kmVec2 origin, kmVec2 scale,
                                            float layer_depth);

/**
 * \brief Draws a sprite to the batch
 * @param batch the sprite batch
 * @param texture the texture
 * @param position the position
 * @param source_rectangle the source rectangle
 * @param color the color
 * @param rotation the rotation angle
 * @param origin the origin
 * @param scale the scale
 * @param layer_depth the depth of the sprite
 */
void binocle_sprite_batch_draw_float_scale(binocle_sprite_batch *batch, struct binocle_image *texture, kmVec2 position,
                                           kmAABB2 source_rectangle, binocle_color color,
                                           float rotation, kmVec2 origin, float scale,
                                           float layer_depth);

/**
 * \brief Draws a sprite to the batch
 * @param batch the sprite batch
 * @param texture the texture
 * @param position the position
 */
void binocle_sprite_batch_draw_position(binocle_sprite_batch *batch, struct binocle_image *texture, kmVec2 position);

/**
 * \brief Draws a sprite to the batch
 * @param batch the sprite batch
 * @param texture the texture
 * @param destination_rectangle the destination rectangle
 * @param source_rectangle the source rectangle
 * @param color the color
 * @param rotation the rotation angle
 * @param origin the origin
 * @param layer_depth the depth of the sprite
 */
void
binocle_sprite_batch_draw_noscale(binocle_sprite_batch *batch, struct binocle_image *texture, kmAABB2 destination_rectangle,
                                  kmAABB2 *source_rectangle, binocle_color color,
                                  float rotation, kmVec2 origin,
                                  float layer_depth);

/**
 * \brief Draws a sprite to the batch
 * @param batch the sprite batch
 * @param texture the texture
 * @param destination_rectangle the destination rectangle
 * @param source_rectangle the source rectangle
 * @param color the color
 */
void
binocle_sprite_draw_dst_src_color(binocle_sprite_batch *batch, struct binocle_image *texture, kmAABB2 destination_rectangle,
                                  kmAABB2 source_rectangle, binocle_color color);

#endif // BINOCLE_SPRITE_H
