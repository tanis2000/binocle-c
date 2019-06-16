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

/**
 * \brief Defines and creates a new component.
 * @param ecs the ECS instance
 * @param name the name of the component
 * @param component_size the size of the data structure of the component
 * @param component_ptr a pointer to the component's ID. This function will write the component's ID into that pointer
 * once it's been setup in the ECS
 * @return true if the component has been successfully added to the ECS
 */
bool binocle_ecs_create_component(binocle_ecs_t *ecs, const char *name, uint64_t component_size, binocle_component_id_t *component_ptr);

/**
 * \brief Removes all the components of a certain type from an entity
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param component the component ID
 * @return true if all the components with that component ID have been removed from the entity
 */
bool binocle_ecs_remove_components(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component);

/**
 * \brief Sets a component for an entity. Adds the component to the entity.
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param component the component ID
 * @param data a pointer to the actual component's data structure containing all the values
 * @return true if the component has been set on the entity
 */
bool binocle_ecs_set_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, const void * data);

/**
 * \brief Gets a component from an entity
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param component the component ID
 * @param ptr a pointer to the struct that will contain the data of the component requested
 * @return true if the component has been found
 */
bool binocle_ecs_get_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, void ** ptr);

/**
 * \brief Releases the resources allocated for a component
 * @param ecs the ECS instance
 * @param component the actual component structure
 */
void binocle_ecs_component_free(binocle_ecs_t *ecs, binocle_component_t *component);

/**
 * \brief Removes a component from an entity (Internal use only)
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param component the component ID
 * @param i the position of the component. Currently only 0 is accepted as we do not support multiple components of the
 * same type on the same entity
 * @return true if the component has been removed
 */
bool binocle_ecs_remove_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i);

/**
 * \brief Sets a component on an entity (Internal use only)
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param component the component ID
 * @param i the position of the component. Currently only 0 is accepted as we do not support multiple components of the
 * @param data the pointer to the data structure of the component
 * @return true if the component has been set
 */
bool binocle_ecs_set_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, const void * data);

/**
 * \brief Gets a component from an entity (Internal use only)
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param component the component ID
 * @param i the position of the component. Currently only 0 is accepted as we do not support multiple components of the
 * @param ptr the pointer that will be filled with the actual data structure of the component
 * @return true if the component has been found
 */
bool binocle_ecs_get_component_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, void ** ptr);

/**
 * \brief Creates a new system
 * @param ecs the ECS instance
 * @param name the name of the system
 * @param starting the callback that will be called once the system is starting up
 * @param process the callback that will be called at each update of the system
 * @param ending the callback that will be called when the system is terminated
 * @param subscribed the callback that will be called once an entity has been subscribed to this system
 * @param unsubscribed the callback that will be called once an entity has been unsubscribed from this system
 * @param user_data a pointer to user data that can be stored within the system
 * @param flags the flags that specify the kind of system. Available values:
 * BINOCLE_SYSTEM_FLAG_NORMAL for a normal system that will be called at each update
 * BINOCLE_SYSTEM_FLAG_PASSIVE for a system that should be manually called by the developer
 * @param system_ptr
 * @return the pointer to the system that has just been created
 */
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

/**
 * \brief Runs a system update
 * @param ecs the ECS instance
 * @param system the ID of the system
 * @param delta the delta time (in seconds) since the last update
 * @return true if the system has processed its entities successfully
 */
bool binocle_ecs_process_system(binocle_ecs_t *ecs, binocle_system_id_t system, float delta);

/**
 * \brief Sends a signal to an entity
 * Sending a signal sets up the values needed to let the systems process the entities
 * @param ecs the ECS instance
 * @param entity the entity ID
 * @param signal the signal to send to the entity
 * Available values:
 * BINOCLE_ENTITY_ADDED the entity has just been added to the ECS
 * BINOCLE_ENTITY_ENABLED the entity has been enabled
 * BINOCLE_ENTITY_DISABLED the entity has been disabled
 * BINOCLE_ENTITY_REMOVED the entity has been removed from the ECS
 * @return true if the signal has been sent successfully
 */
bool binocle_ecs_signal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_entity_signal_t signal);

/**
 * \brief Adds a component to the list of watched components of a system
 * @param ecs the ECS instance
 * @param system the system ID
 * @param component the component ID
 * @return true if the component has been added to the watch list of the system
 */
bool binocle_ecs_watch(binocle_ecs_t *ecs, binocle_system_id_t system, binocle_component_id_t component);

/**
 * \brief Excludes a component from the list of watched components of a system
 * @param ecs the ECS instance
 * @param system the system ID
 * @param component the component ID
 * @return true if the component has been excluded from the list of watched items of the system
 */
bool binocle_ecs_exclude(binocle_ecs_t *ecs, binocle_system_id_t system, binocle_component_id_t component);

/**
 * \brief Subscribes an entity to a system
 * @param ecs the ECS instance
 * @param system the system ID
 * @param entity the entity ID
 */
void binocle_ecs_subscribe(binocle_ecs_t *ecs, binocle_system_t *system, binocle_entity_id_t entity);

/**
 * \brief Unsubscribes an entity from a system
 * @param ecs the ECS instance
 * @param system the system ID
 * @param entity the entity ID
 */
void binocle_ecs_unsubscribe(binocle_ecs_t *ecs, binocle_system_t *system, binocle_entity_id_t entity);

/**
 * \brief Checks if an entity should be processed by a system (Internal use only)
 * @param ecs the ECS instance
 * @param system the system structure
 * @param entity the entity id
 */
void binocle_ecs_check(binocle_ecs_t *ecs, binocle_system_t *system, binocle_entity_id_t entity);

/**
 * \brief Updates the ECS and runs its associated systems
 * @param ecs the ECS instance
 * @param delta the delta time since the last update in seconds
 * @return true if the processing finished successfully
 */
bool binocle_ecs_process(binocle_ecs_t *ecs, float delta);

#endif //BINOCLE_ECS_H
