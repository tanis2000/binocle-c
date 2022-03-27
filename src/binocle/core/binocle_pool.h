//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

/*
 * Resource pool implementation blatantly taken from sokol_gfx.
 * Thanks Andre! ;)
 */
#ifndef BINOCLE_POOL_H
#define BINOCLE_POOL_H

#include <stdint.h>

/* this must be 0 due to the way the pools work */
#define BINOCLE_POOL_INVALID_SLOT_INDEX (0)

#define BINOCLE_INVALID_ID (0)
#define BINOCLE_SLOT_SHIFT 16
#define BINOCLE_SLOT_MASK (1<<BINOCLE_SLOT_SHIFT)-1
#define BINOCLE_MAX_POOL_SIZE (1<<BINOCLE_SLOT_SHIFT)

/*
    binocle_resource_state

    The current state of a resource in its resource pool.
    Resources start in the INITIAL state, which means the
    pool slot is unoccupied and can be allocated. When a resource is
    created, first an id is allocated, and the resource pool slot
    is set to state ALLOC. After allocation, the resource is
    initialized, which may result in the VALID or FAILED state. The
    reason why allocation and initialization are separate is because
    some resource types (e.g. buffers and images) might be asynchronously
    initialized by the user application. If a resource which is not
    in the VALID state is attempted to be used for rendering, rendering
    operations will silently be dropped.

    The special INVALID state is returned in binocle_query_xxx_state() if no
    resource object exists for the provided resource id.
*/
typedef enum binocle_resource_state {
  BINOCLE_RESOURCESTATE_INITIAL,
  BINOCLE_RESOURCESTATE_ALLOC,
  BINOCLE_RESOURCESTATE_VALID,
  BINOCLE_RESOURCESTATE_FAILED,
  BINOCLE_RESOURCESTATE_INVALID,
  BINOCLE_RESOURCESTATE_FORCE_U32 = 0x7FFFFFFF
} binocle_resource_state;

typedef struct binocle_pool_t {
  int size;
  int queue_top;
  uint32_t* gen_ctrs;
  int* free_queue;
} binocle_pool_t;

/* resource pool slots */
typedef struct binocle_slot_t {
  uint32_t id;
  uint32_t ctx_id;
  binocle_resource_state state;
} binocle_slot_t;


void binocle_pool_init(struct binocle_pool_t* pool, uint32_t num);
void binocle_pool_discard(struct binocle_pool_t* pool);
int binocle_pool_alloc_index(struct binocle_pool_t* pool);
void binocle_pool_free_index(struct binocle_pool_t* pool, int slot_index);
uint32_t binocle_pool_slot_alloc(binocle_pool_t *pool, binocle_slot_t *slot,
                                 int slot_index);
int binocle_pool_slot_index(uint32_t id);

#endif // BINOCLE_POOL_H
