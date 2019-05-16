//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_COLLISION_H
#define BINOCLE_COLLISION_H

#include <stdbool.h>
#include <kazmath/kazmath.h>
#include <inttypes.h>
#include <khash/khash.h>

typedef enum binocle_collider_type {
  BINOCLE_COLLIDER_CIRCLE,
  BINOCLE_COLLIDER_HITBOX
} binocle_collider_type;

typedef enum binocle_point_sectors {
  BINOCLE_POINT_SECTOR_CENTER = 0,
  BINOCLE_POINT_SECTOR_TOP = 1,
  BINOCLE_POINT_SECTOR_BOTTOM = 2,
  BINOCLE_POINT_SECTOR_TOP_LEFT = 9,
  BINOCLE_POINT_SECTOR_TOP_RIGHT = 5,
  BINOCLE_POINT_SECTOR_LEFT = 8,
  BINOCLE_POINT_SECTOR_RIGHT = 4,
  BINOCLE_POINT_SECTOR_BOTTOM_LEFT = 10,
  BINOCLE_POINT_SECTOR_BOTTOM_RIGHT = 6
} binocle_point_sectors;


typedef struct binocle_collider_circle {
  float radius;
  kmVec2 center;
} binocle_collider_circle;

typedef struct binocle_collider_hitbox {
  kmAABB2 aabb;
} binocle_collider_hitbox;

typedef struct binocle_collider {
  binocle_collider_circle *circle;
  binocle_collider_hitbox *hitbox;
} binocle_collider;

typedef struct binocle_spatial_hash_cell {
  binocle_collider *colliders;
  uint32_t size;
  uint32_t capacity;
} binocle_spatial_hash_cell;

#define binocle_spatial_hash_get_key(key) (uint64_t)((int64_t)((key).x)<<32 | (uint32_t)((key).y))
#define binocle_spatial_hash_equal(a, b) ((a).x == (b).x && (a).y == (b).y)
KHASH_INIT(spatial_hash_cell_map_t, kmVec2, binocle_spatial_hash_cell, 1, binocle_spatial_hash_get_key, binocle_spatial_hash_equal)

typedef struct binocle_spatial_hash {
  khash_t(spatial_hash_cell_map_t) *grid;
  uint32_t cell_size;
  uint32_t grid_width;
  uint32_t grid_height;
  uint32_t grid_length;
  int32_t grid_x;
  int32_t grid_y;
  float inv_cell_size;
  uint64_t *temp_arr;
  uint32_t temp_arr_capacity;
  uint32_t temp_arr_size;
} binocle_spatial_hash;

binocle_collider_circle binocle_collider_circle_new(float radius, kmVec2 center);
kmVec2 binocle_collider_circle_get_absolute_position(binocle_collider_circle *circle);
float binocle_collider_circle_get_absolute_left(binocle_collider_circle *circle);
float binocle_collider_circle_get_absolute_top(binocle_collider_circle *circle);
float binocle_collider_circle_get_absolute_right(binocle_collider_circle *circle);
float binocle_collider_circle_get_absolute_bottom(binocle_collider_circle *circle);

binocle_collider_hitbox binocle_collider_hitbox_new(kmAABB2 aabb);
float binocle_collider_hitbox_get_absolute_left(binocle_collider_hitbox *hitbox);
float binocle_collider_hitbox_get_absolute_top(binocle_collider_hitbox *hitbox);
float binocle_collider_hitbox_get_absolute_right(binocle_collider_hitbox *hitbox);
float binocle_collider_hitbox_get_absolute_bottom(binocle_collider_hitbox *hitbox);
float binocle_collider_hitbox_get_width(binocle_collider_hitbox *hitbox);
float binocle_collider_hitbox_get_height(binocle_collider_hitbox *hitbox);
bool binocle_collider_hitbox_intersects_hitbox(binocle_collider_hitbox *hitbox_a, binocle_collider_hitbox *hitbox_b);

binocle_point_sectors binocle_collide_get_sector_rect_point(kmAABB2 rect, kmVec2 point);
binocle_point_sectors binocle_collide_get_sector_parts_point(float r_x, float r_y, float r_w, float r_h,
                                                             kmVec2 point);

kmVec2 binocle_collide_closest_point_on_line(kmVec2 lineA, kmVec2 lineB,
                                             kmVec2 closestTo);
bool binocle_collide_rect_to_circle(float rect_x, float rect_y, float rect_width, float rect_height,
                                    kmVec2 c_position, float c_radius);
bool binocle_collide_circle_to_point(kmVec2 circle_position, float circle_radius, kmVec2 point);
bool binocle_collide_circle_to_line(kmVec2 cPosiition, float cRadius, kmVec2 lineFrom,
                                    kmVec2 lineTo);
bool binocle_collide_circle_to_circle(binocle_collider_circle *circle_a, binocle_collider_circle *circle_b);
bool binocle_collide_circle_to_hitbox(binocle_collider_circle *circle, binocle_collider_hitbox *hitbox);
bool binocle_collide_hitbox_to_hitbox(binocle_collider_hitbox *hitbox_a, binocle_collider_hitbox *hitbox_b);

binocle_spatial_hash binocle_spatial_hash_new(float width, float height, uint32_t cell_size);
void binocle_spatial_hash_destroy(binocle_spatial_hash *spatial_hash);
binocle_spatial_hash_cell binocle_spatial_hash_cell_new();
kmVec2 binocle_spatial_hash_get_cell_coords(binocle_spatial_hash *spatial_hash, float x, float y);

#endif //BINOCLE_COLLISION_H
