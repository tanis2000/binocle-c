//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_COLLISION_H
#define BINOCLE_COLLISION_H

#include <stdbool.h>
#include <kazmath/kazmath.h>

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


binocle_collider_circle binocle_collider_circle_new(float radius, kmVec2 center);
kmVec2 binocle_collider_circle_get_absolute_position(binocle_collider_circle *circle);

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

#endif //BINOCLE_COLLISION_H
