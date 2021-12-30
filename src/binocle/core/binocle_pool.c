//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_pool.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void binocle_pool_init(struct binocle_pool_t* pool, uint32_t num) {
  assert(pool && (num >= 1));
  /* slot 0 is reserved for the 'invalid id', so bump the pool size by 1 */
  pool->size = num + 1;
  pool->queue_top = 0;
  /* generation counters indexable by pool slot index, slot 0 is reserved */
  size_t gen_ctrs_size = sizeof(uint32_t) * pool->size;
  pool->gen_ctrs = (uint32_t*) malloc(gen_ctrs_size);
  assert(pool->gen_ctrs);
  memset(pool->gen_ctrs, 0, gen_ctrs_size);
  /* it's not a bug to only reserve 'num' here */
  pool->free_queue = (int*) malloc(sizeof(int)*num);
  assert(pool->free_queue);
  /* never allocate the zero-th pool item since the invalid id is 0 */
  for (int i = pool->size-1; i >= 1; i--) {
    pool->free_queue[pool->queue_top++] = i;
  }
}

void binocle_pool_discard(struct binocle_pool_t* pool) {
  assert(pool);
  assert(pool->free_queue);
  free(pool->free_queue);
  pool->free_queue = 0;
  assert(pool->gen_ctrs);
  free(pool->gen_ctrs);
  pool->gen_ctrs = 0;
  pool->size = 0;
  pool->queue_top = 0;
}

int binocle_pool_alloc_index(struct binocle_pool_t* pool) {
  assert(pool);
  assert(pool->free_queue);
  if (pool->queue_top > 0) {
    int slot_index = pool->free_queue[--pool->queue_top];
    assert((slot_index > 0) && (slot_index < pool->size));
    return slot_index;
  }
  else {
    /* pool exhausted */
    return BINOCLE_POOL_INVALID_SLOT_INDEX;
  }
}

void binocle_pool_free_index(struct binocle_pool_t* pool, int slot_index) {
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) && (slot_index < pool->size));
  assert(pool);
  assert(pool->free_queue);
  assert(pool->queue_top < pool->size);
#ifdef DEBUG
  /* debug check against double-free */
    for (int i = 0; i < pool->queue_top; i++) {
        assert(pool->free_queue[i] != slot_index);
    }
#endif
  pool->free_queue[pool->queue_top++] = slot_index;
  assert(pool->queue_top <= (pool->size-1));
}


/* allocate the slot at slot_index:
    - bump the slot's generation counter
    - create a resource id from the generation counter and slot index
    - set the slot's id to this id
    - set the slot's state to ALLOC
    - return the resource id
*/
uint32_t binocle_pool_slot_alloc(binocle_pool_t *pool, binocle_slot_t *slot,
                                 int slot_index) {
  /* FIXME: add handling for an overflowing generation counter,
     for now, just overflow (another option is to disable
     the slot)
  */
  assert(pool && pool->gen_ctrs);
  assert((slot_index > BINOCLE_POOL_INVALID_SLOT_INDEX) &&
         (slot_index < pool->size));
  assert((slot->state == BINOCLE_RESOURCESTATE_INITIAL) &&
         (slot->id == BINOCLE_INVALID_ID));
  uint32_t ctr = ++pool->gen_ctrs[slot_index];
  slot->id = (ctr << BINOCLE_SLOT_SHIFT) | (slot_index & BINOCLE_SLOT_MASK);
  slot->state = BINOCLE_RESOURCESTATE_ALLOC;
  return slot->id;
}

/* extract slot index from id */
int binocle_pool_slot_index(uint32_t id) {
  int slot_index = (int) (id & BINOCLE_SLOT_MASK);
  assert(BINOCLE_POOL_INVALID_SLOT_INDEX != slot_index);
  return slot_index;
}