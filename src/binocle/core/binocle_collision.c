//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_collision.h"
#include "binocle_log.h"
#include <stdlib.h>

#define DG_DYNARR_IMPLEMENTATION
#include <dg/DG_dynarr.h>

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

binocle_collider binocle_collider_new() {
  binocle_collider res = {0};
  res.hitbox = NULL;
  res.circle = NULL;
  da_init(res.grid_index);
  return res;
}

float binocle_collider_get_absolute_left(binocle_collider *collider) {
  if (collider->circle != NULL) {
    return binocle_collider_circle_get_absolute_left(collider->circle);
  } else {
    return binocle_collider_hitbox_get_absolute_left(collider->hitbox);
  }
}

float binocle_collider_get_absolute_bottom(binocle_collider *collider) {
  if (collider->circle != NULL) {
    return binocle_collider_circle_get_absolute_bottom(collider->circle);
  } else {
    return binocle_collider_hitbox_get_absolute_bottom(collider->hitbox);
  }
}

float binocle_collider_get_absolute_right(binocle_collider *collider) {
  if (collider->circle != NULL) {
    return binocle_collider_circle_get_absolute_right(collider->circle);
  } else {
    return binocle_collider_hitbox_get_absolute_right(collider->hitbox);
  }
}

float binocle_collider_get_absolute_top(binocle_collider *collider) {
  if (collider->circle != NULL) {
    return binocle_collider_circle_get_absolute_top(collider->circle);
  } else {
    return binocle_collider_hitbox_get_absolute_top(collider->hitbox);
  }
}

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
      binocle_spatial_hash_grid_key_t key = binocle_spatial_hash_get_key(x, y);
      int ret;
      khiter_t k = kh_put(spatial_hash_cell_map_t, res.grid, key, &ret);
      kh_value(res.grid, k) = binocle_spatial_hash_cell_new();
      //res.cells[GetKey(x, y)] = std::vector<Colliders::Collider *>();
    }
  }

  da_init(res.temp_arr);
  return res;
}

void binocle_spatial_hash_destroy(binocle_spatial_hash *spatial_hash) {
  da_free(spatial_hash->temp_arr);
  kh_destroy(spatial_hash_cell_map_t, spatial_hash->grid);
}

binocle_spatial_hash_cell binocle_spatial_hash_cell_new() {
  binocle_spatial_hash_cell res = {0};
  da_init(res.colliders);
  return res;
}

kmVec2 binocle_spatial_hash_get_cell_coords(binocle_spatial_hash *spatial_hash, float x, float y) {
  kmVec2 res;
  res.x = floorf(x * spatial_hash->inv_cell_size);
  res.y = floorf(y * spatial_hash->inv_cell_size);
  return res;
}

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
      binocle_spatial_hash_grid_key_t key = binocle_spatial_hash_get_key(x, y);
      binocle_spatial_hash_cell *c = binocle_spatial_hash_cell_at_position(spatial_hash, x, y, true);
      binocle_spatial_hash_add_index(spatial_hash, collider, key);
    }
  }
}

void binocle_spatial_hash_remove_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider) {
  binocle_spatial_hash_remove_indexes(spatial_hash, collider);
}

void binocle_spatial_hash_update_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider) {
  kmVec2 bottom_left;
  kmVec2 top_right;
  bottom_left.x = binocle_collider_get_absolute_left(collider);
  bottom_left.y = binocle_collider_get_absolute_bottom(collider);
  top_right.x = binocle_collider_get_absolute_right(collider);
  top_right.y = binocle_collider_get_absolute_top(collider);
  binocle_spatial_hash_update_indexes(spatial_hash, collider, binocle_spatial_hash_aabb_to_grid(spatial_hash, bottom_left, top_right));
}

binocle_spatial_hash_cell *binocle_spatial_hash_cell_at_position(binocle_spatial_hash *spatial_hash, int x, int y, bool createCellIfEmpty) {
  binocle_spatial_hash_cell *cell = NULL;
  kmVec2 v = binocle_spatial_hash_get_cell_coords(spatial_hash, x, y);
  binocle_spatial_hash_grid_key_t key = binocle_spatial_hash_get_key(v.x, v.y);
  khiter_t k = kh_get(spatial_hash_cell_map_t, spatial_hash->grid, key);
  if (k != kh_end(spatial_hash->grid)) {
    // found
    cell = &kh_val(spatial_hash->grid, k);
  }
  if (cell == NULL || da_count(cell->colliders) == 0) {
    if (createCellIfEmpty) {
      int ret;
      k = kh_put(spatial_hash_cell_map_t, spatial_hash->grid, key, &ret);
      kh_value(spatial_hash->grid, k) = binocle_spatial_hash_cell_new();
      cell = &kh_val(spatial_hash->grid, k);
    }
  }
  return cell;
}

void binocle_spatial_hash_cell_add_collider(binocle_spatial_hash_cell *cell, binocle_collider *collider) {
  da_push(cell->colliders, collider);
}

void binocle_collider_add_grid_index(binocle_collider *collider, binocle_spatial_hash_grid_key_t cell_pos) {
  da_push(collider->grid_index, cell_pos);
}

void binocle_spatial_hash_add_index(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_spatial_hash_grid_key_t cell_pos) {
  khiter_t k = kh_get(spatial_hash_cell_map_t, spatial_hash->grid, cell_pos);
  binocle_spatial_hash_cell *cell = &kh_val(spatial_hash->grid, k);
  binocle_spatial_hash_cell_add_collider(cell, collider);
  binocle_collider_add_grid_index(collider, cell_pos);
}

void binocle_spatial_hash_remove_index(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_spatial_hash_grid_key_t cell_pos) {
  binocle_spatial_hash_cell *cell = NULL;
  khiter_t k = kh_get(spatial_hash_cell_map_t, spatial_hash->grid, cell_pos);
  if (k != kh_end(spatial_hash->grid)) {
    // found
    cell = &kh_val(spatial_hash->grid, k);
    for (int i = da_count(cell->colliders) - 1 ; i > 0 ; i--) {
      binocle_collider *coll = cell->colliders.p[i];
      if (coll == collider) {
        da_delete(cell->colliders, i);
      }
    }
  }
}

void binocle_spatial_hash_remove_indexes(binocle_spatial_hash *spatial_hash, binocle_collider *collider) {
  for (int i = 0 ; i < da_count(collider->grid_index) ; i++) {
    binocle_spatial_hash_grid_key_t key = collider->grid_index.p[i];
    binocle_spatial_hash_remove_index(spatial_hash, collider, key);
  }
  da_clear(collider->grid_index);
}

void binocle_spatial_hash_update_indexes(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_grid_key_array_t *ar) {
  for (int i = 0 ; i < da_count(collider->grid_index) ; i++) {
    binocle_spatial_hash_grid_key_t key = collider->grid_index.p[i];
    binocle_spatial_hash_remove_index(spatial_hash, collider, key);
  }
  da_clear(collider->grid_index);

  for (int i = 0 ; i < da_count(*ar) ; i++) {
    binocle_spatial_hash_grid_key_t key = ar->p[i];
    binocle_spatial_hash_add_index(spatial_hash, collider, key);
  }
}

binocle_grid_key_array_t *binocle_spatial_hash_aabb_to_grid(binocle_spatial_hash *spatial_hash, kmVec2 min, kmVec2 max) {
  da_clear(spatial_hash->temp_arr);

  kmVec2 p1 = binocle_spatial_hash_get_cell_coords(spatial_hash, min.x, min.y);
  kmVec2 p2 = binocle_spatial_hash_get_cell_coords(spatial_hash, max.x, max.y);
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
      binocle_spatial_hash_cell *c = binocle_spatial_hash_cell_at_position(spatial_hash, x, y, true);
      binocle_spatial_hash_grid_key_t key = binocle_spatial_hash_get_key(x, y);
      da_push(spatial_hash->temp_arr, key);
    }
  }

  return &spatial_hash->temp_arr;
}

binocle_spatial_hash_grid_key_t binocle_spatial_hash_get_key(int x, int y) {
  return (uint64_t) x << 32 | (uint64_t) (uint32_t) y;
}

void binocle_spatial_hash_get_all_bodies_sharing_cells_with_body(binocle_spatial_hash *spatial_hash, binocle_collider *collider, binocle_collider_ptr_array_t *colliding_colliders, int layer_mask) {
  da_clear(*colliding_colliders);

  for (int i = 0 ; i < da_count(collider->grid_index) ; i++) {
    binocle_spatial_hash_cell *cell = NULL;
    khiter_t k = kh_get(spatial_hash_cell_map_t, spatial_hash->grid, collider->grid_index.p[i]);
    if (k != kh_end(spatial_hash->grid)) {
      // found
      cell = &kh_val(spatial_hash->grid, k);
      if (da_count(cell->colliders) == 0) {
        continue;
      }
      for (int j = 0 ; j < da_count(cell->colliders) ; j++) {
        binocle_collider *coll = cell->colliders.p[j];
        if (coll == collider /* || !Flags::IsFlagSet(layerMask, cbd->layer)*/) {
          continue;
        }
        da_push(*colliding_colliders, coll);
      }
    } else {
      continue;
    }
  }
}

bool binocle_spatial_hash_is_body_sharing_any_cell(binocle_spatial_hash *spatial_hash, binocle_collider *collider) {
  for (int i = 0 ; i < da_count(collider->grid_index) ; i++) {
    binocle_spatial_hash_cell *cell = NULL;
    khiter_t k = kh_get(spatial_hash_cell_map_t, spatial_hash->grid, collider->grid_index.p[i]);
    if (k != kh_end(spatial_hash->grid)) {
      // found
      cell = &kh_val(spatial_hash->grid, k);
      if (da_count(cell->colliders) == 0) {
        continue;
      }
      for (int j = 0 ; j < da_count(cell->colliders) ; j++) {
        binocle_collider *coll = cell->colliders.p[j];
        if (coll == collider) {
          continue;
        }
        return true;
      }
    } else {
      continue;
    }
  }
  return false;
}

bool binocle_collision_ray_cast_obb(
  kmVec3 ray_origin,
  kmVec3 ray_direction,
  kmVec3 aabb_min,
  kmVec3 aabb_max,
  kmMat4 model_matrix,
  float *intersection_distance
){

// Intersection method from Real-Time Rendering and Essential Mathematics for Games

  float tMin = 0.0f;
  float tMax = 100000.0f;

  kmVec3 OBBposition_worldspace;
  kmMat4ExtractTranslationVec3(&model_matrix, &OBBposition_worldspace);

  kmVec3 delta;
  kmVec3Subtract(&delta, &OBBposition_worldspace, &ray_origin);

  // Test intersection with the 2 planes perpendicular to the OBB's X axis
  {
    kmVec3 xaxis;
    kmVec3Fill(&xaxis, model_matrix.mat[0 * 4], model_matrix.mat[0 * 4 + 1], model_matrix.mat[0 * 4 + 2]);
    float e = kmVec3Dot(&xaxis, &delta);
    float f = kmVec3Dot(&ray_direction, &xaxis);

    if ( fabsf(f) > 0.001f ){ // Standard case

      float t1 = (e+aabb_min.x)/f; // Intersection with the "left" plane
      float t2 = (e+aabb_max.x)/f; // Intersection with the "right" plane
      // t1 and t2 now contain distances betwen ray origin and ray-plane intersections

      // We want t1 to represent the nearest intersection,
      // so if it's not the case, invert t1 and t2
      if (t1>t2){
        float w=t1;t1=t2;t2=w; // swap t1 and t2
      }

      // tMax is the nearest "far" intersection (amongst the X,Y and Z planes pairs)
      if ( t2 < tMax )
        tMax = t2;
      // tMin is the farthest "near" intersection (amongst the X,Y and Z planes pairs)
      if ( t1 > tMin )
        tMin = t1;

      // And here's the trick :
      // If "far" is closer than "near", then there is NO intersection.
      // See the images in the tutorials for the visual explanation.
      if (tMax < tMin )
        return false;

    }else{ // Rare case : the ray is almost parallel to the planes, so they don't have any "intersection"
      if(-e+aabb_min.x > 0.0f || -e+aabb_max.x < 0.0f)
        return false;
    }
  }


  // Test intersection with the 2 planes perpendicular to the OBB's Y axis
  // Exactly the same thing than above.
  {
    kmVec3 yaxis;
    kmVec3Fill(&yaxis, model_matrix.mat[1 * 4], model_matrix.mat[1 * 4 + 1], model_matrix.mat[1 * 4 + 2]);
    float e = kmVec3Dot(&yaxis, &delta);
    float f = kmVec3Dot(&ray_direction, &yaxis);

    if ( fabsf(f) > 0.001f ){

      float t1 = (e+aabb_min.y)/f;
      float t2 = (e+aabb_max.y)/f;

      if (t1>t2){float w=t1;t1=t2;t2=w;}

      if ( t2 < tMax )
        tMax = t2;
      if ( t1 > tMin )
        tMin = t1;
      if (tMin > tMax)
        return false;

    }else{
      if(-e+aabb_min.y > 0.0f || -e+aabb_max.y < 0.0f)
        return false;
    }
  }


  // Test intersection with the 2 planes perpendicular to the OBB's Z axis
  // Exactly the same thing than above.
  {
    kmVec3 zaxis;
    kmVec3Fill(&zaxis, model_matrix.mat[2 * 4], model_matrix.mat[2 * 4 + 1], model_matrix.mat[2 * 4 + 2]);
    float e = kmVec3Dot(&zaxis, &delta);
    float f = kmVec3Dot(&ray_direction, &zaxis);

    if ( fabsf(f) > 0.001f ){

      float t1 = (e+aabb_min.z)/f;
      float t2 = (e+aabb_max.z)/f;

      if (t1>t2){float w=t1;t1=t2;t2=w;}

      if ( t2 < tMax )
        tMax = t2;
      if ( t1 > tMin )
        tMin = t1;
      if (tMin > tMax)
        return false;

    }else{
      if(-e+aabb_min.z > 0.0f || -e+aabb_max.z < 0.0f)
        return false;
    }
  }

  *intersection_distance = tMin;
  return true;

}