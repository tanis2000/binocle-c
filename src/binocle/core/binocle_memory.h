//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BINOCLE_MEMORY_H
#define BINOCLE_BINOCLE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

#if BINOCLE_LOG_MEMORY_ALLOCATIONS
#define BINOCLE_DEBUG_NAME__(A, B, C) A "|" #B "|" #C
#define BINOCLE_DEBUG_NAME_(A, B, C) BINOCLE_DEBUG_NAME__(A, B, C)
#define BINOCLE_DEBUG_NAME(Name) BINOCLE_DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__)
#define BINOCLE_DEBUG_MEMORY_NAME(Name) BINOCLE_DEBUG_NAME_(__FILE__, __LINE__, __COUNTER__),
#define BINOCLE_MEMORY_PARAM char *GUID,
#define BINOCLE_MEMORY_PASS GUID,
#else
#define BINOCLE_DEBUG_MEMORY_NAME(Name)
#define BINOCLE_MEMORY_PARAM
#define BINOCLE_MEMORY_PASS
#endif

#define BINOCLE_MEMORY_OFFSET_OF(type, Member) ((uintptr_t)&(((type *)0)->Member))

typedef enum binocle_memory_block_flags {
  BINOCLE_MEMORY_FLAG_NOT_RESTORED = 1,
  BINOCLE_MEMORY_FLAG_CHECK_OVERFLOW = 1 << 1,
  BINOCLE_MEMORY_FLAG_CHECK_UNDERFLOW = 1 << 2,
} binocle_memory_flags;

typedef struct binocle_memory_block {
  uint64_t flags;
  uint64_t size;
  uint8_t *base;
  uintptr_t used;
  struct binocle_memory_block *prev_arena;
} binocle_memory_block;

typedef struct binocle_memory_platform_block {
  binocle_memory_block block;
  struct binocle_memory_platform_block *prev;
  struct binocle_memory_platform_block *next;
} binocle_memory_platform_block;

typedef struct binocle_memory_arena {
  binocle_memory_block *current_block;
  uintptr_t minimum_block_size;
  uint64_t allocation_flags;
  int32_t temp_count;
} binocle_memory_arena;

typedef struct binocle_temporary_memory {
  binocle_memory_arena *arena;
  binocle_memory_block *block;
  uintptr_t used;
} binocle_temporary_memory;

typedef enum binocle_memory_arena_push_flag
{
  BINOCLE_MEMORY_ARENA_PUSH_FLAG_CLEAR_TO_ZERO = 1,
} binocle_memory_arena_push_flag;

typedef struct binocle_memory_arena_push_params
{
  uint32_t flags;
  uint32_t alignment;
} binocle_memory_arena_push_params;

typedef struct binocle_memory_arena_bootstrap_params
{
  uint64_t allocation_flags;
  uintptr_t minimum_block_size;
} binocle_memory_arena_bootstrap_params;

typedef size_t binocle_memory_index;

typedef struct binocle_buffer
{
  uintptr_t count;
  uint8_t *data;
} binocle_buffer;

typedef binocle_buffer binocle_string;

typedef struct binocle_memory_state {
  binocle_memory_platform_block memory_sentinel;
} binocle_memory_state;

void binocle_memory_init();

binocle_memory_arena_push_params binocle_memory_default_arena_params(void);

binocle_memory_arena_bootstrap_params
binocle_memory_default_bootstrap_params(void);

void *binocle_memory_bootstrap_push_size(
  BINOCLE_MEMORY_PARAM uintptr_t struct_size, uintptr_t offset_to_arena,
  binocle_memory_arena_bootstrap_params bootstrap_params,
  binocle_memory_arena_push_params push_params);

void *
binocle_memory_push_size_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                          binocle_memory_index size_init,
                          binocle_memory_arena_push_params params);

void *
binocle_memory_push_copy_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                          uintptr_t size, void *source,
                          binocle_memory_arena_push_params params);

binocle_buffer
binocle_memory_push_buffer_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                            uintptr_t size,
                            binocle_memory_arena_push_params params);

char *
binocle_memory_push_string_z_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                              char *source);

binocle_string
binocle_memory_push_cstring_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                             char *source);

binocle_string
binocle_memory_push_string_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                            binocle_string source);

char *binocle_memory_push_and_null_terminate_(
  BINOCLE_MEMORY_PARAM binocle_memory_arena *arena, uint32_t length,
  const char *source);

#define binocle_memory_copy_array(count, source, dest) binocle_memory_copy((count)*sizeof(*(source)), (source), (dest))

#define binocle_memory_zero_struct(instance) binocle_memory_zero_size(sizeof(instance), &(instance))
#define binocle_memory_zero_array(count, pointer) binocle_memory_zero_size((count)*sizeof((pointer)[0]), pointer)

#define binocle_memory_push_struct(arena, type, ...) (type *)binocle_memory_push_size_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_struct") arena, sizeof(type), ## __VA_ARGS__)
#define binocle_memory_push_array(arena, count, type, ...) (type *)binocle_memory_push_size_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_array") arena, (count)*sizeof(type), ## __VA_ARGS__)
#define binocle_memory_push_size(arena, size, ...) binocle_memory_push_size_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_size") arena, size, ## __VA_ARGS__)
#define binocle_memory_push_copy(...) binocle_memory_push_copy_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_copy") __VA_ARGS__)
#define binocle_memory_push_string_z(...) binocle_memory_push_string_z_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_string_z") __VA_ARGS__)
#define binocle_memory_push_string(...) binocle_memory_push_string_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_string") __VA_ARGS__)
#define binocle_memory_push_cstring(...) binocle_memory_push_cstring_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_cstring") __VA_ARGS__)
#define binocle_memory_push_buffer(...) binocle_memory_push_buffer_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_buffer") __VA_ARGS__)
#define binocle_memory_push_and_null_terminate(...) binocle_memory_push_and_null_terminate_(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_push_and_null_terminate_") __VA_ARGS__)
#define binocle_memory_bootstrap_push_struct(type, Member, ...) (type *)binocle_memory_bootstrap_push_size(BINOCLE_DEBUG_MEMORY_NAME("binocle_memory_bootstrap_push_size") sizeof(type), BINOCLE_MEMORY_OFFSET_OF(type, Member), ## __VA_ARGS__)

#endif // BINOCLE_BINOCLE_MEMORY_H
