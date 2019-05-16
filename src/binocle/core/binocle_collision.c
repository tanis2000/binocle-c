//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_collision.h"
#include "binocle_log.h"
#include <stdlib.h>

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

float binocle_collider_circle_get_absolute_left(binocle_collider_circle *circle) {
  return circle->center.x - circle->radius;
}

float binocle_collider_circle_get_absolute_right(binocle_collider_circle *circle) {
  return circle->center.x + circle->radius;
}

float binocle_collider_circle_get_absolute_top(binocle_collider_circle *circle) {
  return circle->center.y + circle->radius;
}

float binocle_collider_circle_get_absolute_bottom(binocle_collider_circle *circle) {
  return circle->center.y - circle->radius;
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

//
// Spatial hash
//

binocle_spatial_hash binocle_spatial_hash_new(float width, float height, uint32_t cell_size) {
  binocle_spatial_hash res = {0};
  res.cell_size = cell_size;
  res.inv_cell_size = 1.0f / cell_size;
  res.grid_width = (uint32_t)ceilf(width * res.inv_cell_size);
  res.grid_height = (uint32_t)ceilf(height * res.inv_cell_size);
  res.grid_length = res.grid_width * res.grid_height;

  // Preallocate the starting grid
  res.grid = kh_init(spatial_hash_cell_map_t);
  for (int y = 0; y < res.grid_height; y++) {
    for (int x = 0; x < res.grid_width; x++) {
      kmVec2 key;
      key.x = x;
      key.y = y;
      int ret;
      khiter_t k = kh_put(spatial_hash_cell_map_t, res.grid, key, &ret);
      kh_value(res.grid, k) = binocle_spatial_hash_cell_new();
      //res.cells[GetKey(x, y)] = std::vector<Colliders::Collider *>();
    }
  }

  res.temp_arr_capacity = 16;
  res.temp_arr_size = 0;
  res.temp_arr = malloc(sizeof(uint64_t) * res.temp_arr_capacity);
  return res;
}

void binocle_spatial_hash_destroy(binocle_spatial_hash *spatial_hash) {
  free(spatial_hash->temp_arr);
  kh_destroy(spatial_hash_cell_map_t, spatial_hash->grid);
}

binocle_spatial_hash_cell binocle_spatial_hash_cell_new() {
  binocle_spatial_hash_cell res = {0};
  res.capacity = 16;
  res.size = 0;
  res.colliders = malloc(sizeof(binocle_collider) * res.capacity);
  return res;
}

kmVec2 binocle_spatial_hash_get_cell_coords(binocle_spatial_hash *spatial_hash, float x, float y) {
  kmVec2 res;
  res.x = floorf(x * spatial_hash->inv_cell_size);
  res.y = floorf(y * spatial_hash->inv_cell_size);
  return res;
}

/*
void binocle_spatial_hash_add_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider) {
  kmVec2 p1;
  kmVec2 p2;

  if (collider->circle != NULL) {
    kmVec2 min;
    kmVec2 max;
    min.x = binocle_collider_circle_get_absolute_left(collider->circle);
    min.y = binocle_collider_circle_get_absolute_bottom(collider->circle);
    max.x = binocle_collider_circle_get_absolute_right(collider->circle);
    max.y = binocle_collider_circle_get_absolute_top(collider->circle);
    p1 = binocle_spatial_hash_get_cell_coords(spatial_hash, min.x, min.y);
    p2 = binocle_spatial_hash_get_cell_coords(spatial_hash, max.x, max.y);
  } else if (collider->hitbox != NULL) {
    kmVec2 min;
    kmVec2 max;
    min.x = binocle_collider_hitbox_get_absolute_left(collider->hitbox);
    min.y = binocle_collider_hitbox_get_absolute_bottom(collider->hitbox);
    max.x = binocle_collider_hitbox_get_absolute_right(collider->hitbox);
    max.y = binocle_collider_hitbox_get_absolute_top(collider->hitbox);
    p1 = binocle_spatial_hash_get_cell_coords(spatial_hash, min.x, min.y);
    p2 = binocle_spatial_hash_get_cell_coords(spatial_hash, max.x, max.y);
  } else {
    binocle_log_error("binocle_spatial_hash_add_body(): this collider has no shape");
    return;
  }

  if (p2.x > spatial_hash->grid_width) {
    spatial_hash->grid_width = p2.x;
  }
  if (p1.x < spatial_hash->grid_x) {
    spatial_hash->grid_x = p1.x;
  }
  if (p2.y > spatial_hash->grid_height) {
    spatial_hash->grid_height = p2.y;
  }
  if (p1.y < spatial_hash->grid_y) {
    spatial_hash->grid_y = p1.y;
  }
  for (int x = p1.x; x <= p2.x; x++) {
    for (int y = p1.y; y <= p2.y; y++) {
      // make sure we have allocated a cell
      kmVec2 key;
      key.x = x;
      key.y = y;
      auto c = binocle_spatial_hash_cell_at_position(x, y, true);
      binocle_spatial_hash_add_index(b, binocle_spatial_hash_get_key(key));
    }
  }
}

std::vector<Colliders::Collider *> *SpatialHash::CellAtPosition(int x, int y, bool createCellIfEmpty = false) {
  std::vector<Colliders::Collider *> *cell = nullptr;
  glm::vec2 v = GetCellCoords(x, y);
  if (grid.find(GetKey(v.x, v.y)) != grid.end()) {
    // found
    cell = &grid.at(GetKey(v.x, v.y));
  }
  if (cell == nullptr || cell->empty()) {
    if (createCellIfEmpty) {
      cell = new std::vector<Colliders::Collider *>();
      grid[GetKey(v.x, v.y)] = *cell;
    }
  }
  return cell;
}
*/