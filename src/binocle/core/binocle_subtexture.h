//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_SUBTEXTURE_H
#define BINOCLE_SUBTEXTURE_H

#include <kazmath/kazmath.h>
#include <stdint.h>

struct binocle_texture;

/**
 * A subtexture is a slice of a texture. Generally used for sprite sheets.
 */
typedef struct binocle_subtexture {
  struct binocle_texture *texture;
  kmAABB2 rect;
  char name[1024];
} binocle_subtexture;

/**
 * \brief Creates a subtexture given a texture, coordinates and size
 * @param texture the texture
 * @param x the x position of the subtexture
 * @param y the y position of the subtexture
 * @param width the width of the subtexture
 * @param height the height of the subtexture
 * @return the subtexture
 */
binocle_subtexture
binocle_subtexture_with_texture(struct binocle_texture *texture, float x, float y, float width, float height);

/**
 * \brief Creates a subtexture given a subtexture, coordinates and size
 * @param subtexture the subtexture to get the texture from
 * @param x the x position of the subtexture
 * @param y the y position of the subtexture
 * @param width the width of the subtexture
 * @param height the height of the subtexture
 * @return the subtexture
 */
binocle_subtexture
binocle_subtexture_with_subtexture(binocle_subtexture *subtexture, float x, float y, float width, float height);

/**
 * \brief Gets the X coordinate of a subtexture within its texture
 * @param subtexture the subtexture
 * @return the X position
 */
uint64_t binocle_subtexture_get_x(binocle_subtexture subtexture);

/**
 * \brief Gets the Y coordinate of a subtexture within its texture
 * @param subtexture the subtexture
 * @return the Y position
 */
uint64_t binocle_subtexture_get_y(binocle_subtexture subtexture);

/**
 * \brief Gets the width of a subtexture
 * @param subtexture the subtexture
 * @return the width of the subtexture
 */
uint64_t binocle_subtexture_get_width(binocle_subtexture *subtexture);

/**
 * \brief Gets the height of a subtexture
 * @param subtexture the subtexture
 * @return the height of the subtexture
 */
uint64_t binocle_subtexture_get_height(binocle_subtexture *subtexture);

/**
 * \brief Gets the size of a subtexture
 * @param subtexture the subtexture
 * @return the size of the subtexture
 */
kmVec2 binocle_subtexture_get_size(binocle_subtexture *subtexture);

/**
 * \brief Gets the coordinates of the center of the subtexture
 * @param subtexture the subtexture
 * @return the coordinates of the center of the subtexture
 */
kmVec2 binocle_subtexture_get_center(binocle_subtexture *subtexture);

/**
 * \brief Gets a frame position from a subtexture
 * @param subtexture the subtexture
 * @param index the index of the frame
 * @param frame_width the width of the frame
 * @param frame_height the height of the frame
 * @return the position of the frame
 */
kmAABB2 binocle_subtexture_get_frame(binocle_subtexture subtexture, uint64_t index, uint64_t frame_width,
                                     uint64_t frame_height);

/**
 * \brief Gets the absolute clip rectangle of a subtexture given a relative clip rectangle
 * @param subtexture the subtexture
 * @param relative_clip_rect the relative clip rectangle
 * @return the absolute clip rectangle
 */
kmAABB2 binocle_subtexture_get_absolute_clip_rect(binocle_subtexture subtexture, kmAABB2 relative_clip_rect);

#endif //BINOCLE_SUBTEXTURE_H
