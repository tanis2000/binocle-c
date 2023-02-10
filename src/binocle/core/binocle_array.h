//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_ARRAY_H
#define BINOCLE_ARRAY_H

#include <inttypes.h>
#include <stdbool.h>

typedef struct binocle_array_header_t
{
  uint64_t capacity;
  uint64_t size;
} binocle_array_header_t;

/**
 * Returns the header given an array
 */
#define binocle_array_header(a) ((binocle_array_header_t *)((uint8_t *)(a) - sizeof(binocle_array_header_t)))

/**
 * Returns the number of elements in the given array
 */
#define binocle_array_size(a) ((a) ? binocle_array_header(a)->size : 0)

/**
 * Returns the number of bytes used by element data given the array
 */
#define binocle_array_bytes(a) (binocle_array_size(a) * sizeof(*(a)))

/**
 * Returns a pointer past the end of the given array. Returns NULL if the array has not been allocated
 */
#define binocle_array_end(a) ((a) ? (a) + binocle_array_size(a) : 0)

/**
 * Returns the number of elements allocated for the given array
 */
#define binocle_array_capacity(a) ((a) ? binocle_array_header(a)->capacity : 0)

/**
 * The maximum number of items of size `item_size` an array allocated with `bytes` bytes can hold
 */
#define binocle_array_max_capacity(bytes, item_size) (((bytes) - sizeof(binocle_array_header_t)) / (item_size))

/**
 * Returns *true* if the array needs to grow to hold `n` elements.
 */
#define binocle_array_needs_to_grow(a, n) ((n) > binocle_array_capacity(a))

/**
 * Pops the last item from the array and returns it.
 */
#define binocle_array_pop(a) ((a)[--binocle_array_header(a)->size])

/**
 * As `binocle_array_resize()` but can only shrink the array. This won't reallocate memory.
 */
#define binocle_array_shrink(a, n) ((a) ? binocle_array_header(a)->size = n : 0)

/**
 * Grows the capacity of the given array geometrically to hold at least `n` elements.
 * The array is updated in-place.
 */
#define binocle_array_grow(a, n) ((*(void **)&(a)) = binocle_array_grow_internal((void *)a, n, sizeof(*(a))))

/**
 * Ensures that the given array has capacity to to hold at least `n` elements.
 * Grows `a` as needed (geometrically) to hold the specified number of elements.
 * The array is updated in-place.
 */
#define binocle_array_ensure(a, n) (binocle_array_needs_to_grow(a, n) ? binocle_array_grow(a, n) : 0)

/**
 * Sets the capacity of the array to exactly `n`.
 * The array is updated in-place.
 */
#define binocle_array_set_capacity(a, n) ((*(void **)&(a)) = binocle_array_set_capacity_internal((void *)a, n, sizeof(*(a))))

/**
 * Pushes `item` to the end of `a`, growing it geometrically if needed. `a` is updated in-place.
 * Returns a pointer to the pushed item in `a`.
 */
#define binocle_array_push(a, item) (binocle_array_ensure(a, binocle_array_size(a) + 1), (a)[binocle_array_header(a)->size++] = (item), (a) + binocle_array_header(a)->size - 1)
#define binocle_array_push_array(a, items, n) ((n) ? ((binocle_array_ensure(a, binocle_array_size(a) + n), memcpy(a + binocle_array_size(a), items, n * sizeof(*items)), binocle_array_header(a)->size += n), 0) : 0)

/**
 * Resizes `a` to `n` elements. If `a` needs to grow, it will grow to exactly `n` elements. Note
 * that this growth is not geometric. Use `tm_carray_resize_geom()` instead if you want geometric
 * growth. `a` is updated in-place.
 */
#define binocle_array_resize(a, n) ((binocle_array_needs_to_grow(a, n) ? binocle_array_set_capacity(a, n) : 0), (a) ? binocle_array_header(a)->size = n : 0)

/**
 * As `binocle_array_resize()` but uses geometric growth.
 */
#define binocle_array_resize_geom(a, n) (tm_carray_ensure_at(a, n), (a) ? binocle_array_header(a)->size = n : 0)

/**
 * Frees the memory used by the array.
 */
#define binocle_array_free(a) ((*(void **)&(a)) = binocle_array_set_capacity_internal((void *)a, 0, sizeof(*(a))))

void *binocle_array_set_capacity_internal(void *arr, uint64_t new_capacity, uint64_t item_size);
void *binocle_array_grow_internal(void *arr, uint64_t to_at_least, uint64_t item_size);

#endif //BINOCLE_ARRAY_H
