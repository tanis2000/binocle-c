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

/// System bits positions
#define BINOCLE_SYSTEM_PASSIVE_BIT 1

/// System flags
#define BINOCLE_SYSTEM_FLAG_NORMAL  0
#define BINOCLE_SYSTEM_FLAG_PASSIVE BINOCLE_SYSTEM_PASSIVE_BIT

struct binocle_ecs_t;

/**
 * \brief A signal that can be sent to an entity
 */
typedef enum binocle_entity_signal_t {
  BINOCLE_ENTITY_ADDED,
  BINOCLE_ENTITY_ENABLED,
  BINOCLE_ENTITY_DISABLED,
  BINOCLE_ENTITY_REMOVED
} binocle_entity_signal_t;

/// The entity itself, actually just an ID
typedef uint64_t binocle_entity_id_t;
/// The component itself, actually just an ID
typedef uint64_t binocle_component_id_t;
/// The system itself, actually just an ID
typedef uint64_t binocle_system_id_t;

/**
 * \brief A sparse integer set, used to quickly lookup components
 * \see https://www.computist.xyz/2018/06/sparse-sets.html
 */
typedef struct binocle_sparse_integer_set_t {
  uint64_t *dense;
  uint64_t *sparse;
  uint64_t size;
  uint64_t capacity;
} binocle_sparse_integer_set_t;

/**
 * \brief A dense integer set, used to save some memory space to represent the entities in use by a system
 */
typedef struct binocle_dense_integer_set_t {
  unsigned char *bytes;
  unsigned int capacity;
} binocle_dense_integer_set_t;

/**
 * \brief A component
 */
typedef struct binocle_component_t {
  const char *name;
  size_t size;
  size_t offset;
  unsigned int flags;

  void **data;
  binocle_sparse_integer_set_t free_data_indexes;
  uint64_t next_data_index;
} binocle_component_t;

/**
 * \brief A system
 */
typedef struct binocle_system_t {
  const char *name;
  uint64_t flags;
  void *user_data;
  void (*starting)(struct binocle_ecs_t* ecs, void *user_data);
  void (*process)(struct binocle_ecs_t* ecs, void *user_data, binocle_entity_id_t entity, float delta);
  void (*ending)(struct binocle_ecs_t* ecs, void *user_data);
  void (*subscribed)(struct binocle_ecs_t *ecs, void *user_data, binocle_entity_id_t entity);
  void (*unsubscribed)(struct binocle_ecs_t *ecs, void *user_data, binocle_entity_id_t entity);
  binocle_sparse_integer_set_t watch;
  binocle_sparse_integer_set_t exclude;
  binocle_dense_integer_set_t entities;
} binocle_system_t;

/**
 * \brief The ECS container
 */
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

  uint64_t num_systems;
  binocle_system_t *systems;
} binocle_ecs_t;

/**
 * \brief Insert an integer in a sparse integer set
 * @param set the sparse integer set
 * @param i the integer to insert
 * @return true if the integer has been inserted
 */
bool binocle_sparse_integer_set_insert(binocle_sparse_integer_set_t *set, uint64_t i);

/**
 * \brief Remove an integer from a sparse integer set
 * @param set the sparse integer set
 * @param i the integer to remove
 * @return true if the integer has been removed
 */
bool binocle_sparse_integer_set_remove(binocle_sparse_integer_set_t *set, uint64_t i);

/**
 * \brief Checks if a sparse integer set contains an integer
 * @param set the sparse integer set
 * @param i the integer to check against
 * @return true if the sparse integer set contains the integer
 */
bool binocle_sparse_integer_set_contains(binocle_sparse_integer_set_t *set, uint64_t i);

/**
 * \brief Clears a sparse integer set
 * @param set the sparse integer set
 */
void binocle_sparse_integer_set_clear(binocle_sparse_integer_set_t *set);

/**
 * \brief Checks if the sparse integer set is empty
 * @param set the sparse integer set
 * @return true if the sparse integer set is empty
 */
bool binocle_sparse_integer_set_is_empty(binocle_sparse_integer_set_t *set);

/**
 * \brief Pops the last element of the sparse integer set
 * @param set the sparse integer set
 * @return the last element of the sparse integer set
 */
uint64_t binocle_sparse_integer_set_pop(binocle_sparse_integer_set_t *set);

/**
 * \brief Releases the resources allocated by the sparse integer set
 * @param set the sparse integer set
 */
void binocle_sparse_integer_set_free(binocle_sparse_integer_set_t *set);

/**
 * \brief Sets a bit in an array of bytes given the bit position
 * @param bytes the array used as a bit container
 * @param bit the bit to set
 * @return the bit set
 */
int binocle_bits_set(unsigned char *bytes, unsigned int bit);

/**
 * \brief Checks whether a given bit is set
 * @param bytes the array used as a bit container
 * @param bit the bit to set
 * @return true if the bit is set
 */
int binocle_bits_is_set(unsigned char *bytes, unsigned int bit);

/**
 * \brief Clears a given bit
 * @param bytes the array used as a bit cvontainer
 * @param bit the bit to clear
 * @return the bit that has been cleared
 */
int binocle_bits_clear(unsigned char *bytes, unsigned int bit);

/**
 * \brief Insert an integer in a dense integer set
 * @param is the dense integer set
 * @param i the integer to insert
 * @return the inserted integer bit position
 */
uint64_t binocle_dense_integer_set_insert(binocle_dense_integer_set_t *is, uint64_t i);

/**
 * \brief Remove an integer from a dense integer set
 * @param is the dense integer set
 * @param i the integer to remove
 * @return the position of the bit that has been cleared
 */
uint64_t binocle_dense_integer_set_remove(binocle_dense_integer_set_t *is, uint64_t i);

/**
 * \brief Check if a dense integer set contins an integer
 * @param is the dense integer set
 * @param i the integer to check
 * @return true if the dense integer set contains the integer
 */
bool binocle_dense_integer_set_contains(binocle_dense_integer_set_t *is, uint64_t i);

/**
 * Clear a dense integer set
 * @param is the dense integer set
 */
void binocle_dense_integer_set_clear(binocle_dense_integer_set_t *is);

/**
 * Check if a dense integer set is empty
 * @param is the dense integer set
 * @return true if the dense integer set is empty
 */
int binocle_dense_integer_set_is_empty(binocle_dense_integer_set_t *is);

/**
 * Release all the resources allocated by a dense integer set
 * @param is the dense integer set
 */
void binocle_dense_integer_set_free(binocle_dense_integer_set_t *is);

/**
 * \brief Creates a new instance of the ECS, leaving it in an uninitialized state
 * While the ECS isn't initialized you can call \ref binocle_ecs_create_component and \ref binocle_ecs_create_system to
 * create the components and systems that you will need during program execution.
 * Once done you have to call \ref binocle_ecs_initialize to actually initialize all the data structures.
 * \note Once the ECS has been initialized you can no longer define new components and systems.
 * @return a new ECS instance
 */
binocle_ecs_t binocle_ecs_new();

/**
 * \brief Initializes the data structures of the ECS
 * \note Once this function has been called you can no longer define new components or systems
 * @param ecs the ECS instance
 * @return true if the ECS has been initialized
 */
bool binocle_ecs_initialize(binocle_ecs_t *ecs);

/**
 * \brief Releases the resources allocated by the ECS instance
 * @param ecs the ECS instance
 * @return true if the resources have been fred
 */
bool binocle_ecs_free(binocle_ecs_t *ecs);

/**
 * \brief Moves all the entities that have been spawned during the processing of systems to the correct entity
 * data array. This function is for internal use only and should never be called outside of the ECS itself.
 * @param ecs the ECS instance
 * @return true if everything went ok
 */
bool binocle_ecs_fix_data(binocle_ecs_t *ecs);

/**
 * \brief Creates a new entity
 * @param ecs the ECS instance
 * @param entity_ptr a pointer to the entity id that will be assigned by this function
 * @return true if the entity has been spawned
 */
bool binocle_ecs_create_entity(binocle_ecs_t *ecs, binocle_entity_id_t *entity_ptr);

/**
 * \brief Gets the entity data for a given entity
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @return a pointer to the entity's data
 */
unsigned char *binocle_ecs_get_entity_data(binocle_ecs_t *ecs, binocle_entity_id_t entity);

bool binocle_ecs_create_component(binocle_ecs_t *ecs, const char *name, uint64_t component_size, binocle_component_id_t *component_ptr);
bool binocle_ecs_remove_components(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component);
bool binocle_ecs_set_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, const void * data);
bool binocle_ecs_get_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, void ** ptr);
void binocle_ecs_component_free(binocle_ecs_t *ecs, binocle_component_t *component);
bool binocle_ecs_remove_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i);
bool binocle_ecs_set_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, const void * data);
bool binocle_ecs_get_component_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, void ** ptr);

bool binocle_ecs_create_system(binocle_ecs_t *ecs, const char *name,
                               void (*starting)(struct binocle_ecs_t*, void *),
                               void (*process)(struct binocle_ecs_t*, void *, binocle_entity_id_t, float),
                               void (*ending)(struct binocle_ecs_t*, void *),
                               void (*subscribed)(struct binocle_ecs_t*, void *, binocle_entity_id_t),
                               void (*unsubscribed)(struct binocle_ecs_t*, void *, binocle_entity_id_t),
                               void *user_data,
                               uint64_t flags,
                               uint64_t *system_ptr
);
bool binocle_ecs_watch(binocle_ecs_t *ecs, binocle_system_id_t system, binocle_component_id_t component);
bool binocle_ecs_exclude(binocle_ecs_t *ecs, binocle_system_id_t system, binocle_component_id_t component);
bool binocle_ecs_signal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_entity_signal_t signal);
void binocle_ecs_subscribe(binocle_ecs_t *ecs, binocle_system_t *system, binocle_entity_id_t entity);
void binocle_ecs_unsubscribe(binocle_ecs_t *ecs, binocle_system_t *system, binocle_entity_id_t entity);
void binocle_ecs_check(binocle_ecs_t *ecs, binocle_system_t *system, binocle_entity_id_t entity);
bool binocle_ecs_process(binocle_ecs_t *ecs, float delta);
bool binocle_ecs_process_system(binocle_ecs_t *ecs, binocle_system_id_t system, float delta);

#endif //BINOCLE_ECS_H
