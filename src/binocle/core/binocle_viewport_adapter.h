//
// Created by Valerio Santinelli on 13/04/18.
//

#ifndef BINOCLE_BINOCLE_VIEWPORT_ADAPTER_H
#define BINOCLE_BINOCLE_VIEWPORT_ADAPTER_H

#include <stdint.h>
#include <stdbool.h>
#include <kazmath/kazmath.h>

struct binocle_window;

typedef enum binocle_viewport_adapter_kind {
  BINOCLE_VIEWPORT_ADAPTER_KIND_BASIC,
  BINOCLE_VIEWPORT_ADAPTER_KIND_SCALING,
} binocle_viewport_adapter_kind;

typedef enum binocle_viewport_adapter_scaling_type {
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_NONE,
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_FREE,
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_PIXEL_PERFECT,
  BINOCLE_VIEWPORT_ADAPTER_SCALING_TYPE_BOXING,
} binocle_viewport_adapter_scaling_type;

typedef struct binocle_viewport_adapter {
  kmMat4 scale_matrix;
  kmAABB2 viewport;
  kmAABB2 original_viewport;
  uint32_t virtual_width;
  uint32_t virtual_height;
  binocle_viewport_adapter_kind kind;
  binocle_viewport_adapter_scaling_type scaling_type;
} binocle_viewport_adapter;

binocle_viewport_adapter binocle_viewport_adapter_new(struct binocle_window window, binocle_viewport_adapter_kind kind, binocle_viewport_adapter_scaling_type scaling_type, uint32_t width, uint32_t height, uint32_t virtual_width, uint32_t virtual_height);
uint32_t binocle_viewport_adapter_get_virtual_width(binocle_viewport_adapter adapter);
uint32_t binocle_wiewport_adapter_get_virtual_height(binocle_viewport_adapter adapter);
uint32_t binocle_viewport_adapter_get_viewport_width(binocle_viewport_adapter adapter);
uint32_t binocle_viewport_adapter_get_viewport_height(binocle_viewport_adapter adapter);
kmVec2 binocle_viewport_adapter_point_to_virtual_viewport(binocle_viewport_adapter adapter, kmVec2 point);
kmVec2 binocle_viewport_adapter_screen_to_virtual_viewport(binocle_viewport_adapter adapter, kmVec2 point);
void binocle_viewport_adapter_reset(binocle_viewport_adapter *adapter, kmVec2 oldWindowSize, kmVec2 newWindowSize);
void binocle_viewport_adapter_set_viewport(binocle_viewport_adapter *adapter, kmAABB2 viewport);
kmAABB2 binocle_viewport_adapter_get_viewport(binocle_viewport_adapter adapter);
kmAABB2 binocle_viewport_adapter_get_original_viewport(binocle_viewport_adapter adapter);

#endif //BINOCLE_BINOCLE_VIEWPORT_ADAPTER_H
