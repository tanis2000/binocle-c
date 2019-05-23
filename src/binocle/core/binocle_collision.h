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
#include <dg/DG_dynarr.h>

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

typedef uint64_t binocle_spatial_hash_grid_key_t;

DA_TYPEDEF(binocle_spatial_hash_grid_key_t, binocle_grid_key_array_t)

typedef struct binocle_collider {
  binocle_collider_circle *circle;
  binocle_collider_hitbox *hitbox;
  binocle_grid_key_array_t grid_index;
} binocle_collider;

DA_TYPEDEF(binocle_collider *, binocle_collider_ptr_array_t)

typedef struct binocle_spatial_hash_cell {
  binocle_collider_ptr_array_t colliders;
} binocle_spatial_hash_cell;


#define binocle_spatial_hash_func(key) (binocle_spatial_hash_grid_key_t)(key)
#define binocle_spatial_hash_equal(a, b) ((a) == (b))
KHASH_INIT(spatial_hash_cell_map_t, binocle_spatial_hash_grid_key_t, binocle_spatial_hash_cell, 1, binocle_spatial_hash_func, binocle_spatial_hash_equal)

typedef struct binocle_spatial_hash {
  khash_t(spatial_hash_cell_map_t) *grid;
  uint32_t cell_size;
  uint32_t grid_width;
  uint32_t grid_height;
  uint32_t grid_length;
  int32_t grid_x;
  int32_t grid_y;
  float inv_cell_size;
  binocle_grid_key_array_t temp_arr;
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

binocle_collider binocle_collider_new();
binocle_spatial_hash binocle_spatial_hash_new(float width, float height, uint32_t cell_size);
void binocle_spatial_hash_destroy(binocle_spatial_hash *spatial_hash);
binocle_spatial_hash_cell binocle_spatial_hash_cell_new();
kmVec2 binocle_spatial_hash_get_cell_coords(binocle_spatial_hash *spatial_hash, float x, float y);
binocle_spatial_hash_cell *binocle_spatial_hash_cell_at_position(binocle_spatial_hash *spatial_hash, int x, int y, bool createCellIfEmpty);
void binocle_spatial_hash_add_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider);
void binocle_spatial_hash_remove_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider);
void binocle_spatial_hash_update_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider);
void binocle_spatial_hash_add_index(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_spatial_hash_grid_key_t cell_pos);
void binocle_collider_add_grid_index(binocle_collider *collider, binocle_spatial_hash_grid_key_t cell_pos);
void binocle_spatial_hash_cell_add_collider(binocle_spatial_hash_cell *cell, binocle_collider *collider);
void binocle_spatial_hash_remove_indexes(binocle_spatial_hash *spatial_hash, binocle_collider *collider);
void binocle_spatial_hash_update_indexes(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_grid_key_array_t *ar);
binocle_grid_key_array_t *binocle_spatial_hash_aabb_to_grid(binocle_spatial_hash *spatial_hash, kmVec2 min, kmVec2 max);
uint64_t binocle_spatial_hash_get_key(int x, int y);
void binocle_spatial_hash_get_all_bodies_sharing_cells_with_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_collider_ptr_array_t *colliding_colliders, int layer_mask);
bool binocle_spatial_hash_is_body_sharing_any_cell(binocle_spatial_hash *spatial_hash, binocle_collider *collider);

#endif //BINOCLE_COLLISION_H
