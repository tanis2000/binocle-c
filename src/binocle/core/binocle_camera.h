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
 * \brief Gets the inverse transform matrix of the camera
 * @param camera The camera
 * @return The inverse transform matrix
 */
kmMat4 *binocle_camera_get_inverse_transform_matrix(binocle_camera *camera);

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
  kmMat4 transform_matrix; // cached view matrix
  kmMat4 inverse_transform_matrix; // cached inverse view matrix
  kmVec3 position;
  kmVec3 front;
  kmVec3 up;
  kmVec3 right;
  kmVec3 world_up;
  float yaw;
  float pitch;
  float fov_y;
  float near_distance;
  float far_distance;
} binocle_camera_3d;

/**
 * \brief Creates a new 3D camera instance
 * @param position the position of the camera in world coordinates
 * @param near the near plane distance
 * @param far the far plane distance
 * @param fov_y the vertical field of view
 * @return an instance of binocle_camera_3d
 */
binocle_camera_3d binocle_camera_3d_new(kmVec3 position, float near, float far, float fov_y);

/**
 * \brief Updates the transform and inverse_transform matrices
 * @param camera the instance of binocle_camera_3d to update
 */
void binocle_camera_3d_update_matrixes(binocle_camera_3d *camera);

/**
 * \brief Sets the camera position in world coordinates and updates the underlying matrices
 * @param camera the instance of binocle_camera_3d
 * @param position the position in world coordinates
 */
void binocle_camera_3d_set_position(binocle_camera_3d *camera, kmVec3 position);

/**
 * \brief Sets the orientation of the camera and updates the underlying matrices
 * @param camera the instance of the binocle_camera_3d
 * @param pitch the pitch in degrees
 * @param yaw the yaw in degrees
 * @param roll the roll in degrees
 */
void binocle_camera_3d_set_rotation(binocle_camera_3d *camera, float pitch, float yaw, float roll);

/**
 * \brief Sets the camera near plane distance
 * @param camera the instance of the binocle_camera_3d
 * @param near the distance of the near plane
 */
void binocle_camera_3d_set_near(binocle_camera_3d *camera, float near);

/**
 * \brief Sets the camera far plane distance
 * @param camera the instance of the binocle_camera_3d
 * @param far the distance of the far plane
 */
void binocle_camera_3d_set_far(binocle_camera_3d *camera, float far);

/**
 * \brief Returns the transform matrix of the camera
 * @param camera the instance of the binocle_camera_3d
 * @return the transform matrix of the camera
 */
kmMat4 *binocle_camera_3d_get_transform_matrix(binocle_camera_3d *camera);

/**
 * \brief Translates the camera in the world
 * @param camera the instance of the binocle_camera_3d
 * @param x the amount to move on the X direction (positive is right)
 * @param y the amount to move on the Y direction (positive is up)
 * @param z the amount to move on the Z direction (positive is into the scene)
 */
void binocle_camera_3d_translate(binocle_camera_3d *camera, float x, float y, float z);

/**
 * \brief Rotates the camera by the given amount on the different axes
 * @param camera the instance of binocle_camera_3d
 * @param pitch the pitch in degrees
 * @param yaw the yaw in degrees
 * @param roll the roll in degrees
 */
void binocle_camera_3d_rotate(binocle_camera_3d *camera, float pitch, float yaw, float roll);

/**
 * \brief Returns the inverse transform matrix
 * @param camera the instance of binocle_camera_3d
 * @return the inverse transform matrix
 */
kmMat4 *binocle_camera_3d_get_inverse_transform_matrix(binocle_camera_3d *camera);

/**
 * \brief Internal function to transform an X,Y screen space coordinate to world space and get the
 * direction ray from it
 * @param mouse_x the X coordinate in screen space
 * @param mouse_y the Y coordinate in screen space
 * @param screen_width the window width in pixels
 * @param screen_height the window height in pixels
 * @param view_matrix camera position and orientation
 * @param projection_matrix camera parameters (ratio, field of view, near and far plane)
 * @param out_direction the direction, in world space, of the ray that goes "through" the mouse position
 */
void binocle_camera_3d_screen_to_world_ray_internal(
  int mouse_x,
  int mouse_y,
  int screen_width,
  int screen_height,
  kmMat4 view_matrix,
  kmMat4 projection_matrix,
  kmVec3 *out_direction
);

/**
 * \brief Transforms an X,Y screen space coordinate into a direction ray in world space
 * @param camera an instance of binocle_camera_3d
 * @param mouse_x the X coordinate in screen space
 * @param mouse_y the Y coordinate in screen space
 * @param viewport the camera/window viewport
 * @param out_direction the direction, in world space, of the ray that goes "through" the mouse position
 */
void binocle_camera_3d_screen_to_world_ray(
  binocle_camera_3d *camera,
  int mouse_x,
  int mouse_y,
  kmAABB2 viewport,
  kmVec3 *out_direction
);

#endif //BINOCLE_CAMERA_H
