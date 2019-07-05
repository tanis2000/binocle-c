//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_CAMERA_H
#define BINOCLE_CAMERA_H

#include <stdint.h>
#include <stdbool.h>
#include <kazmath/kazmath.h>

struct binocle_viewport_adapter;

/**
 * 2D Camera
 */
typedef struct binocle_camera {
  kmMat4 transform_matrix; // the matrix representing the camera
  kmMat4 inverse_transform_matrix; // the inverse of the camera transform matrix
  kmVec2 position; // the position of the camera
  kmVec2 origin; // the origin of the camera
  float rotation; // the rotation
  float zoom; // the zoom level
  float min_zoom; // the minimum zoom level
  float max_zoom; // the maximum zoom level
  bool are_matrixes_dirty; // true if matrices need to be updates
  bool are_bounds_dirty; // true if bounds have changed and need to be adjusted
  kmAABB2 bounds; // the camera bounds
  struct binocle_viewport_adapter *viewport_adapter; // the viewport adapter
  float near_distance; // the near distance
  float far_distance; // the far distance
} binocle_camera;

/**
 * \brief Creates a new camera instance
 * @param adapter The viewport adapter to apply to this camera
 * @return The camera
 */
binocle_camera binocle_camera_new(struct binocle_viewport_adapter *adapter);

/**
 * \brief Converts a point from screen coordinates to world coordinates
 * @param camera The camera
 * @param screen_position The position in screen coordinates
 * @return The world point
 */
kmVec2 binocle_camera_screen_to_world_point(binocle_camera camera, kmVec2 screen_position);

/**
 * \brief Updates the camera matrixes
 * This does the actual calculations to update the internal matrixes
 * @param camera The camera
 */
void binocle_camera_update_matrixes(binocle_camera *camera);

/**
 * \brief Force updating the camera matrixes as soon as we access any of them
 * This sets the matrixes and bounds flags as dirty so that as soon as we need to access any of the matrixes we
 * recalculate them right away
 * @param camera The camera
 */
void binocle_camera_force_matrix_update(binocle_camera *camera);

/**
 * \brief Gets the transform matrix of the camera
 * @param camera The camera
 * @return The transform matrix
 */
kmMat4 *binocle_camera_get_transform_matrix(binocle_camera *camera);

/**
 * \brief Gets the viewport adapter associated to the camera
 * @param camera The camera
 * @return Teh viewport adapter associated to the camera
 */
struct binocle_viewport_adapter *binocle_camera_get_viewport_adapter(binocle_camera camera);

/**
 * \brief Helper to get the viewport of the camera
 * It actually gets the viewport of the viewport adapter associated to the camera
 * @param camera The camera
 * @return the viewport of the camera
 */
kmAABB2 binocle_camera_get_viewport(binocle_camera camera);

/**
 * \brief Sets the camera position in world coordinates
 * @param camera The camera
 * @param x X position
 * @param y Y position
 */
void binocle_camera_set_position(binocle_camera *camera, float x, float y);

/**
 * \brief Converts a point from world coordinates to screen coordinates
 * @param camera The camera
 * @param world_position The position in world coordinates
 * @return The screen point
 */
kmVec2 binocle_camera_world_to_screen_point(binocle_camera camera, kmVec2 world_position);

/**
 * \brief Gets the camera projection matrix
 * @param camera The camera
 * @return The projection matrix
 */
kmMat4 binocle_camera_get_projection_matrix(binocle_camera camera);

/**
 * \brief Get the view projection matrix of the given camera
 * Gets the view-projection matrix which is the transformMatrix * the projection matrix
 * @param camera The camera
 * @return The view projection matrix
 */
kmMat4 binocle_camera_get_view_projection_matrix(binocle_camera camera);

typedef struct binocle_camera_3d {
  kmMat4 transform_matrix;
  kmMat4 inverse_transform_matrix;
  kmVec3 position;
  kmVec3 rotation;
  float fov_y;
  float near;
  float far;
} binocle_camera_3d;

binocle_camera_3d binocle_camera_3d_new(kmVec3 position, kmVec3 rotation, float near, float far, float fov_y);
void binocle_camera_3d_update_matrixes(binocle_camera_3d *camera);
void binocle_camera_3d_set_position(binocle_camera_3d *camera, kmVec3 position);
void binocle_camera_3d_set_rotation(binocle_camera_3d *camera, kmVec3 rotation);
void binocle_camera_3d_set_near(binocle_camera_3d *camera, float near);
void binocle_camera_3d_set_far(binocle_camera_3d *camera, float far);
kmMat4 *binocle_camera_3d_get_transform_matrix(binocle_camera_3d *camera);
void binocle_camera_3d_translate(binocle_camera_3d *camera, float x, float y, float z);
void binocle_camera_3d_rotate(binocle_camera_3d *camera, float x, float y, float z);

#endif //BINOCLE_CAMERA_H
