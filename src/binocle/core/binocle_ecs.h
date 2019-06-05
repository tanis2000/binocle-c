//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_ECS_H
#define BINOCLE_ECS_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

typedef uint64_t binocle_entity_id_t;
typedef uint64_t binocle_component_id_t;

// see https://www.computist.xyz/2018/06/sparse-sets.html
typedef struct binocle_sparse_integer_set_t {
  uint64_t *dense;
  uint64_t *sparse;
  uint64_t size;
  uint64_t capacity;
} binocle_sparse_integer_set_t;

typedef struct binocle_dense_integer_set_t {
  unsigned char *bytes;
  unsigned int capacity;
} binocle_dense_integer_set_t;

typedef struct binocle_component_t {
  const char *name;
  size_t size;
  size_t offset;
  unsigned int flags;

  void **data;
  binocle_sparse_integer_set_t free_data_indexes;
  uint64_t next_data_index;
} binocle_component_t;

typedef struct binocle_ecs_t {
  bool initialized;
  bool processing;

  // Manage entity IDs, we actually reuse them when possible
  binocle_sparse_integer_set_t free_entity_ids;
  binocle_entity_id_t next_entity_id;

  // The actual entity data. The first column is bits of components defined, the rest are components
  uint64_t data_width;
  uint64_t data_height;
  uint64_t data_height_capacity;
  void *data;

  // Data of entities being processed
  uint64_t processing_data_height;
  void **processing_data;

  // Buffers for entity status changes and notifications
  struct binocle_sparse_integer_set_t added;
  struct binocle_sparse_integer_set_t enabled;
  struct binocle_sparse_integer_set_t disabled;
  struct binocle_sparse_integer_set_t removed;

  // All the active entities (enabled and added)
  struct binocle_dense_integer_set_t active;

  uint64_t num_components;
  binocle_component_t *components;
} binocle_ecs_t;

bool binocle_sparse_integer_set_insert(binocle_sparse_integer_set_t *set, uint64_t i);
bool binocle_sparse_integer_set_remove(binocle_sparse_integer_set_t *set, uint64_t i);
bool binocle_sparse_integer_set_contains(binocle_sparse_integer_set_t *set, uint64_t i);
void binocle_sparse_integer_set_clear(binocle_sparse_integer_set_t *set);
bool binocle_sparse_integer_set_is_empty(binocle_sparse_integer_set_t *set);
uint64_t binocle_sparse_integer_set_pop(binocle_sparse_integer_set_t *set);
void binocle_sparse_integer_set_free(binocle_sparse_integer_set_t *set);

int binocle_bits_set(unsigned char *bytes, unsigned int bit);
int binocle_bits_is_set(unsigned char *bytes, unsigned int bit);
int binocle_bits_clear(unsigned char *bytes, unsigned int bit);

int binocle_dense_integer_set_contains(binocle_dense_integer_set_t *is, uint64_t i);
uint64_t binocle_dense_integer_set_insert(binocle_dense_integer_set_t *is, uint64_t i);
uint64_t binocle_dense_integer_set_remove(binocle_dense_integer_set_t *is, uint64_t i);
void binocle_dense_integer_set_clear(binocle_dense_integer_set_t *is);
int binocle_dense_integer_set_is_empty(binocle_dense_integer_set_t *is);
void binocle_dense_integer_set_free(binocle_dense_integer_set_t *is);

binocle_ecs_t binocle_ecs_new();
bool binocle_ecs_initialize(binocle_ecs_t *ecs);
bool binocle_ecs_free(binocle_ecs_t *ecs);
bool binocle_ecs_fix_data(binocle_ecs_t *ecs);

bool binocle_ecs_create_entity(binocle_ecs_t *ecs, binocle_entity_id_t *entity_ptr);
unsigned char *binocle_ecs_get_entity_data(binocle_ecs_t *ecs, binocle_entity_id_t entity);

bool binocle_ecs_create_component(binocle_ecs_t *ecs, const char *name, uint64_t component_size, binocle_component_id_t *component_ptr);
bool binocle_ecs_remove_components(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component);
bool binocle_ecs_set_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, const void * data);
bool binocle_ecs_get_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, void ** ptr);
void binocle_ecs_component_free(binocle_ecs_t *ecs, binocle_component_t *component);
bool binocle_ecs_remove_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i);
bool binocle_ecs_set_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, const void * data);
bool binocle_ecs_get_component_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, void ** ptr);

#endif //BINOCLE_ECS_H
