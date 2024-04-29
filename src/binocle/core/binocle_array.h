//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_ARRAY_H
#define BINOCLE_ARRAY_H

#include <inttypes.h>
#include <stdbool.h>

typedef struct binocle_array_header_t {
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

/*
 * Returns a pointer to the last element of `a` or `NULL` if `a` is empty.
 */
#define binocle_array_last(a) ((a && binocle_array_header(a)->size) ? binocle_array_end(a) - 1 : 0)

/**
 * Returns the number of elements allocated for the given array
 */
#define binocle_array_capacity(a) ((a) ? binocle_array_header(a)->capacity : 0)

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

/*
 * Erases `n` elements from `a` starting at `i`.
 */
#define binocle_array_erase(a, i, n) (((a && n) ? memmove((a) + (i), (a) + (i) + (n), (binocle_array_size(a) - (i) - (n)) * sizeof(*(a))) : 0), (a && n) ? binocle_array_header(a)->size -= (n) : 0)

/**
 * Grows the capacity of the given array geometrically to hold at least `n` elements.
 * The array is updated in-place.
 */
#define binocle_array_grow(a, n) binocle_array_grow_at(a, n, __FILE__, __LINE__)

/**
 * Ensures that the given array has capacity to to hold at least `n` elements.
 * Grows `a` as needed (geometrically) to hold the specified number of elements.
 * The array is updated in-place.
 */
#define binocle_array_ensure(a, n) binocle_array_ensure_at(a, n, __FILE__, __LINE__)

/**
 * Sets the capacity of the array to exactly `n`.
 * The array is updated in-place.
 */
#define binocle_array_set_capacity(a, n) binocle_array_set_capacity_at(a, n, __FILE__, __LINE__)

/**
 * Pushes `item` to the end of `a`, growing it geometrically if needed. `a` is updated in-place.
 * Returns a pointer to the pushed item in `a`.
 */
#define binocle_array_push(a, item) binocle_array_push_at(a, item, __FILE__, __LINE__)

/**
 * Inserts `item` at the specified `idx` in `a`, growing it geometrically if needed. `a` is updated
 * in-place by the macro. Returns a pointer to the inserted item in `a`.
 */
#define binocle_array_insert(a, idx, item) binocle_array_insert_at(a, idx, item, __FILE__, __LINE__)

/**
 * Pushes an array of `n` items to the end of `a`, growing it geometrically if needed. `a` is
 * updated in-place by the macro.
 */
#define binocle_array_push_array(a, items, n) binocle_array_push_array_at(a, items, n, __FILE__, __LINE__)

/**
 * Resizes `a` to `n` elements. If `a` needs to grow, it will grow to exactly `n` elements. Note
 * that this growth is not geometric. Use `binocle_array_resize_geom()` instead if you want geometric
 * growth. `a` is updated in-place.
 */
#define binocle_array_resize(a, n) binocle_array_resize_at(a, n, __FILE__, __LINE__)

/**
 * As `binocle_array_resize()` but uses geometric growth.
 */
#define binocle_array_resize_geom(a, n) binocle_array_resize_geom_at(a, n, __FILE__, __LINE__)

/**
 * Creates a array from the array `ptr` of `n` elements.
 */
#define binocle_array_from(ptr, n) (binocle_array_from_at(ptr, n, __FILE__, __LINE__))

/**
 * Creates an empty array of type T with `n` zero-initialized elements.
 */
#define binocle_array_create(T, n) (binocle_array_create_at(T, n, __FILE__, __LINE__))

/**
 * Frees the memory used by the array.
 */
#define binocle_array_free(a) binocle_array_free_at(a, __FILE__, __LINE__)

/*
 * Explicit file and line interface
 *
 * These macros correspond to the regular macros, but take explicit file and line arguments instead
 * of using the current `__FILE__` and `__LINE__`. This can be useful when you want to report memory
 * leaks in a higher level system.
 */

#define binocle_array_grow_at(a, n, file, line) ((*(void **)&(a)) = binocle_array_grow_internal((void *)a, n, sizeof(*(a)), file, line))

#define binocle_array_ensure_at(a, n, file, line) (binocle_array_needs_to_grow(a, n) ? binocle_array_grow_at(a, n, file, line) : 0)

#define binocle_array_set_capacity_at(a, n, file, line) ((*(void **)&(a)) = binocle_array_set_capacity_internal((void *)a, n, sizeof(*(a)), file, line))

#define binocle_array_push_at(a, item, file, line) (binocle_array_ensure_at(a, binocle_array_size(a) + 1, file, line), (a)[binocle_array_header(a)->size++] = (item), (a) + binocle_array_header(a)->size - 1)

#define binocle_array_insert_at(a, idx, item, file, line) (binocle_array_push_at(a, item, file, line), memmove(a + idx + 1, a + idx, (binocle_array_size(a) - idx - 1) * sizeof(*(a))), a[idx] = (item), a + idx)

#define binocle_array_push_array_at(a, items, n, file, line) ((n) ? ((binocle_array_ensure_at(a, binocle_array_size(a) + n, file, line), memcpy(a + binocle_array_size(a), items, n * sizeof(*items)), binocle_array_header(a)->size += n), 0) : 0)

#define binocle_array_resize_at(a, n, file, line) ((binocle_array_needs_to_grow(a, n) ? binocle_array_set_capacity_at(a, n, file, line) : 0), (a) ? binocle_array_header(a)->size = n : 0)

#define binocle_array_resize_geom_at(a, n, file, line) (binocle_array_ensure_at(a, n, file, line), (a) ? binocle_array_header(a)->size = n : 0)

#define binocle_array_from_at(ptr, n, file, line) \
    (false ? ptr : binocle_array_create_internal(ptr, n, sizeof(*ptr), file, line))

#define binocle_array_create_at(T, n, file, line) \
    ((T *)binocle_array_create_internal(0, n, sizeof(T), file, line))

#define binocle_array_free_at(a, file, line) ((*(void **)&(a)) = binocle_array_set_capacity_internal((void *)a, 0, sizeof(*(a)), file, line))


void *binocle_array_set_capacity_internal(void *arr, uint64_t new_capacity, uint64_t item_size, const char *file,
                                          uint32_t line);

void *binocle_array_grow_internal(void *arr, uint64_t to_at_least, uint64_t item_size, const char *file, uint32_t line);

void *binocle_array_create_internal(const void *ptr, uint64_t size, uint64_t item_size,
                                    const char *file, uint32_t line);

#endif //BINOCLE_ARRAY_H
