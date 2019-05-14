//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_collision.h"

kmVec2 binocle_collide_closest_point_on_line(kmVec2 lineA, kmVec2 lineB,
                                             kmVec2 closestTo) {
  kmVec2 v;
  kmVec2Subtract(&v, &lineB, &lineA);
  kmVec2 w;
  kmVec2Subtract(&w, &closestTo, &lineA);
  float t = kmVec2Dot(&w, &v) / kmVec2Dot(&v, &v);
  t = kmClamp(t, 0, 1);
  kmVec2 res;
  res.x = lineA.x + v.x * t;
  res.y = lineA.y + v.y * t;
  return res;
}


binocle_collider_circle binocle_collider_circle_new(float radius, kmVec2 center) {
  binocle_collider_circle res = {0};
  res.center.x = center.x;
  res.center.y = center.y;
  res.radius = radius;
  return res;
}

kmVec2 binocle_collider_circle_get_absolute_position(binocle_collider_circle *circle) {
  return circle->center;
}

binocle_collider_hitbox binocle_collider_hitbox_new(kmAABB2 aabb) {
  binocle_collider_hitbox res = {0};
  res.aabb.min.x = aabb.min.x;
  res.aabb.min.y = aabb.min.y;
  res.aabb.max.x = aabb.max.x;
  res.aabb.max.y = aabb.max.y;
  return res;
}

bool binocle_collide_circle_to_circle(binocle_collider_circle *circle_a, binocle_collider_circle *circle_b) {
  kmVec2 pos_a = binocle_collider_circle_get_absolute_position(circle_a);
  kmVec2 pos_b = binocle_collider_circle_get_absolute_position(circle_b);
  return fabsf(kmVec2DistanceBetween(&pos_a, &pos_b)) < (circle_a->radius + circle_b->radius);
}

bool binocle_collide_circle_to_point(kmVec2 circle_position, float circle_radius, kmVec2 point) {
  return fabsf(kmVec2DistanceBetween(&circle_position, &point)) < circle_radius;
}

bool binocle_collide_circle_to_line(kmVec2 cPosiition, float cRadius, kmVec2 lineFrom,
                           kmVec2 lineTo) {
  kmVec2 closest = binocle_collide_closest_point_on_line(lineFrom, lineTo, cPosiition);
  return fabsf(kmVec2DistanceBetween(&cPosiition, &closest)) < cRadius;
}

bool binocle_collide_rect_to_circle(float rect_x, float rect_y, float rect_width, float rect_height,
                                    kmVec2 c_position, float c_radius) {
  // Check if the circle contains the rectangle's center-point
  kmVec2 point;
  point.x = rect_x + rect_width / 2;
  point.y = rect_y + rect_height / 2;
  if (binocle_collide_circle_to_point(c_position, c_radius, point)) {
    return true;
  }

  // Check the circle against the relevant edges
  kmVec2 edgeFrom;
  kmVec2 edgeTo;
  binocle_point_sectors sector = binocle_collide_get_sector_parts_point(rect_x, rect_y, rect_width, rect_height, c_position);

  if ((sector & BINOCLE_POINT_SECTOR_TOP) != 0) {
    edgeFrom.x = rect_x;
    edgeFrom.y = rect_y + rect_height;
    edgeTo.x = rect_x + rect_width;
    edgeTo.y = rect_y + rect_height;
    if (binocle_collide_circle_to_line(c_position, c_radius, edgeFrom, edgeTo)) {
      return true;
    }
  }

  if ((sector & BINOCLE_POINT_SECTOR_BOTTOM) != 0) {
    edgeFrom.x = rect_x;
    edgeFrom.y = rect_y;
    edgeTo.x = rect_x + rect_width;
    edgeTo.y = rect_y;
    if (binocle_collide_circle_to_line(c_position, c_radius, edgeFrom, edgeTo)) {
      return true;
    }
  }

  if ((sector & BINOCLE_POINT_SECTOR_LEFT) != 0) {
    edgeFrom.x = rect_x;
    edgeFrom.y = rect_y;
    edgeTo.x = rect_x;
    edgeTo.y = rect_y + rect_height;
    if (binocle_collide_circle_to_line(c_position, c_radius, edgeFrom, edgeTo)) {
      return true;
    }
  }

  if ((sector & BINOCLE_POINT_SECTOR_RIGHT) != 0) {
    edgeFrom.x = rect_x + rect_width;
    edgeFrom.y = rect_y;
    edgeTo.x = rect_x + rect_width;
    edgeTo.y = rect_y + rect_height;
    if (binocle_collide_circle_to_line(c_position, c_radius, edgeFrom, edgeTo)) {
      return true;
    }
  }

  return false;
}

bool binocle_collide_circle_to_hitbox(binocle_collider_circle *circle, binocle_collider_hitbox *hitbox) {
  return binocle_collide_rect_to_circle(binocle_collider_hitbox_get_absolute_left(hitbox), binocle_collider_hitbox_get_absolute_bottom(hitbox),
                                     binocle_collider_hitbox_get_width(hitbox), binocle_collider_hitbox_get_height(hitbox),
                                     binocle_collider_circle_get_absolute_position(circle), circle->radius);
}

bool binocle_collide_hitbox_to_hitbox(binocle_collider_hitbox *hitbox_a, binocle_collider_hitbox *hitbox_b) {
  return binocle_collider_hitbox_intersects_hitbox(hitbox_a, hitbox_b);
}

float binocle_collider_hitbox_get_absolute_left(binocle_collider_hitbox *hitbox) {
  return hitbox->aabb.min.x;
}

float binocle_collider_hitbox_get_absolute_right(binocle_collider_hitbox *hitbox) {
  return hitbox->aabb.max.x;
}

float binocle_collider_hitbox_get_absolute_top(binocle_collider_hitbox *hitbox) {
  return hitbox->aabb.max.y;
}

float binocle_collider_hitbox_get_absolute_bottom(binocle_collider_hitbox *hitbox) {
  return hitbox->aabb.min.y;
}

float binocle_collider_hitbox_get_width(binocle_collider_hitbox *hitbox) {
  return hitbox->aabb.max.x - hitbox->aabb.min.x;
}

float binocle_collider_hitbox_get_height(binocle_collider_hitbox *hitbox) {
  return hitbox->aabb.max.y - hitbox->aabb.min.y;
}

bool binocle_collider_hitbox_intersects_hitbox(binocle_collider_hitbox *hitbox_a, binocle_collider_hitbox *hitbox_b) {
  return binocle_collider_hitbox_get_absolute_left(hitbox_a) < binocle_collider_hitbox_get_absolute_right(hitbox_b) &&
    binocle_collider_hitbox_get_absolute_right(hitbox_a) > binocle_collider_hitbox_get_absolute_left(hitbox_b) &&
    binocle_collider_hitbox_get_absolute_bottom(hitbox_a) < binocle_collider_hitbox_get_absolute_top(hitbox_b) &&
    binocle_collider_hitbox_get_absolute_top(hitbox_a) > binocle_collider_hitbox_get_absolute_bottom(hitbox_b);
}

/*
 *  Bitflags and helpers for using the Cohen–Sutherland algorithm
 *  http://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
 *
 *  Sector bitflags:
 *      1001  1000  1010
 *      0001  0000  0010
 *      0101  0100  0110
 */

binocle_point_sectors binocle_collide_get_sector_rect_point(kmAABB2 rect, kmVec2 point) {
  binocle_point_sectors sector = BINOCLE_POINT_SECTOR_CENTER;

  if (point.x < rect.min.x)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_LEFT);
  else if (point.x >= rect.min.x + rect.max.x)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_RIGHT);

  if (point.y > rect.min.y + rect.max.y)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_TOP);
  else if (point.y <= rect.min.y)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_BOTTOM);

  return sector;
}

binocle_point_sectors binocle_collide_get_sector_parts_point(float r_x, float r_y, float r_w, float r_h,
                                                    kmVec2 point) {
  binocle_point_sectors sector = BINOCLE_POINT_SECTOR_CENTER;

  if (point.x < r_x)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_LEFT);
  else if (point.x >= r_x + r_w)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_RIGHT);

  if (point.y > r_y + r_h)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_TOP);
  else if (point.y <= r_y)
    sector = (binocle_point_sectors) (sector | BINOCLE_POINT_SECTOR_BOTTOM);

  return sector;
}
