//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_memory.h"

#include "binocle_log.h"
#include "binocle_math.h"
#include "binocle_sdl.h"
#include <assert.h>
// #include <sys/mman.h>

static binocle_memory_state g_memory_state;

void binocle_memory_init() {
  g_memory_state.memory_sentinel.prev = &g_memory_state.memory_sentinel;
  g_memory_state.memory_sentinel.next = &g_memory_state.memory_sentinel;
}

binocle_memory_arena_push_params binocle_memory_default_arena_params(void) {
  binocle_memory_arena_push_params params;
  params.flags = BINOCLE_MEMORY_ARENA_PUSH_FLAG_CLEAR_TO_ZERO;
  params.alignment = 4;
  return params;
}

binocle_memory_arena_bootstrap_params
binocle_memory_default_bootstrap_params(void) {
  binocle_memory_arena_bootstrap_params params = {0};
  return params;
}

binocle_memory_arena_bootstrap_params
binocle_memory_non_restored_arena_bootstrap_params(void) {
  binocle_memory_arena_bootstrap_params params =
    binocle_memory_default_bootstrap_params();
  params.allocation_flags = BINOCLE_MEMORY_FLAG_NOT_RESTORED;
  return params;
}

binocle_memory_index
binocle_memory_get_alignment_offset(binocle_memory_arena *Arena,
                                    binocle_memory_index alignment) {
  binocle_memory_index alignment_offset = 0;

  binocle_memory_index result_pointer =
    (binocle_memory_index)Arena->current_block->base +
    Arena->current_block->used;
  binocle_memory_index alignment_mask = alignment - 1;
  if (result_pointer & alignment_mask) {
    alignment_offset = alignment - (result_pointer & alignment_mask);
  }

  return (alignment_offset);
}

binocle_memory_index
binocle_memory_get_effective_size_for(binocle_memory_arena *arena,
                                      binocle_memory_index size_init,
                                      binocle_memory_arena_push_params params) {
  binocle_memory_index size = size_init;

  binocle_memory_index alignment_offset =
    binocle_memory_get_alignment_offset(arena, params.alignment);
  size += alignment_offset;

  return (size);
}

void binocle_memory_zero_size(binocle_memory_index size, void *ptr) {
  uint8_t *byte = (uint8_t *)ptr;
  while (size--) {
    *byte++ = 0;
  }
}

binocle_memory_block *binocle_memory_allocate(binocle_memory_index size,
                                              uint64_t flags) {
  // We require memory block headers not to change the cache
  // line alignment of an allocation
  binocle_log_debug("binocle_memory_allocate: size of binocle_memory_platform_block: %d", sizeof(binocle_memory_platform_block));
#if defined(__EMSCRIPTEN__)
  assert(sizeof(binocle_memory_platform_block) == 40);
#else
  assert(sizeof(binocle_memory_platform_block) == 56);
#endif
  uintptr_t page_size = 4096;
  uintptr_t total_size = size + sizeof(binocle_memory_platform_block);
  uintptr_t base_offset = sizeof(binocle_memory_platform_block);
  uintptr_t protect_offset = 0;
  if (flags & BINOCLE_MEMORY_FLAG_CHECK_UNDERFLOW) {
    total_size = size + 2 * page_size;
    base_offset = 2 * page_size;
    protect_offset = page_size;
  } else if (flags & BINOCLE_MEMORY_FLAG_CHECK_OVERFLOW) {
    uintptr_t size_rounded_up = binocle_math_align_pow_2(size, page_size);
    total_size = size_rounded_up + 2 * page_size;
    base_offset = page_size + size_rounded_up - size;
    protect_offset = page_size + size_rounded_up;
  }

  binocle_memory_platform_block *block =
    (binocle_memory_platform_block *)SDL_calloc(1, total_size);
  assert(block != 0);
  block->block.base = (uint8_t *)block + base_offset;
  assert(block->block.used == 0);
  assert(block->block.prev_arena == 0);

  //  if (flags & (BINOCLE_MEMORY_FLAG_CHECK_UNDERFLOW |
  //               BINOCLE_MEMORY_FLAG_CHECK_OVERFLOW)) {
  //    int Protected =
  //      mprotect((uint8_t *)block + protect_offset, page_size, PROT_NONE);
  //    assert(Protected != -1);
  //  }

  binocle_memory_platform_block *Sentinel = &g_memory_state.memory_sentinel;
  block->next = Sentinel;
  block->block.size = size;
  block->block.flags = flags;
  block->prev = Sentinel->prev;
  block->prev->next = block;
  block->next->prev = block;

  binocle_memory_block *plat_block = &block->block;
  return plat_block;
}

void *
binocle_memory_push_size_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                          binocle_memory_index size_init,
                          binocle_memory_arena_push_params params) {
  void *result;

  assert(params.alignment <= 128);
  assert(binocle_math_is_pow_2(params.alignment));

  binocle_memory_index size = 0;
  if (arena->current_block) {
    size = binocle_memory_get_effective_size_for(arena, size_init, params);
  }

  if (!arena->current_block ||
      ((arena->current_block->used + size) > arena->current_block->size)) {
    size = size_init;
    if (arena->allocation_flags & (BINOCLE_MEMORY_FLAG_CHECK_OVERFLOW |
                                   BINOCLE_MEMORY_FLAG_CHECK_UNDERFLOW)) {
      arena->minimum_block_size = 0;
      size = binocle_math_align_pow_2(size, params.alignment);
    } else if (!arena->minimum_block_size) {
      arena->minimum_block_size = 1024 * 1024;
    }

    binocle_memory_index block_size = MAX(size, arena->minimum_block_size);

    binocle_memory_block *new_block =
      binocle_memory_allocate(block_size, arena->allocation_flags);
    new_block->prev_arena = arena->current_block;
    arena->current_block = new_block;
  }

  assert((arena->current_block->used + size) <= arena->current_block->size);

  binocle_memory_index alignment_offset =
    binocle_memory_get_alignment_offset(arena, params.alignment);
  uintptr_t OffsetInBlock = arena->current_block->used + alignment_offset;
  result = arena->current_block->base + OffsetInBlock;
  arena->current_block->used += size;

  assert(size >= size_init);
  assert(arena->current_block->used <= arena->current_block->size);

  if (params.flags & BINOCLE_MEMORY_ARENA_PUSH_FLAG_CLEAR_TO_ZERO) {
    binocle_memory_zero_size(size_init, result);
  }

  return result;
}

void *binocle_memory_bootstrap_push_size(
  BINOCLE_MEMORY_PARAM uintptr_t struct_size, uintptr_t offset_to_arena,
  binocle_memory_arena_bootstrap_params bootstrap_params,
  binocle_memory_arena_push_params push_params) {
  binocle_memory_arena bootstrap = {0};
  bootstrap.allocation_flags = bootstrap_params.allocation_flags;
  bootstrap.minimum_block_size = bootstrap_params.minimum_block_size;
  void *res_struct = binocle_memory_push_size_(BINOCLE_MEMORY_PASS & bootstrap,
                                               struct_size, push_params);
  *(binocle_memory_arena *)((uint8_t *)res_struct + offset_to_arena) =
    bootstrap;

  return (res_struct);
}

uint32_t binocle_string_length(char *string) {
  uint32_t count = 0;
  if (string) {
    while (*string++) {
      ++count;
    }
  }

  return (count);
}

void *binocle_memory_copy(binocle_memory_index size, void *source_init,
                          void *dest_init) {
  uint8_t *Source = (uint8_t *)source_init;
  uint8_t *Dest = (uint8_t *)dest_init;
  while (size--) {
    *Dest++ = *Source++;
  }

  return dest_init;
}

binocle_memory_arena_push_params binocle_memory_no_clear(void) {
  binocle_memory_arena_push_params params =
    binocle_memory_default_arena_params();
  params.flags &= ~BINOCLE_MEMORY_ARENA_PUSH_FLAG_CLEAR_TO_ZERO;
  return params;
}

void *
binocle_memory_push_copy_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                          uintptr_t size, void *source,
                          binocle_memory_arena_push_params params) {
  void *result =
    binocle_memory_push_size_(BINOCLE_MEMORY_PASS arena, size, params);
  binocle_memory_copy(size, source, result);
  return (result);
}

char *
binocle_memory_push_string_z_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                              char *source) {
  uint32_t size = 1;
  for (char *At = source; *At; ++At) {
    ++size;
  }

  char *dest = (char *)binocle_memory_push_size_(
    BINOCLE_MEMORY_PASS arena, size, binocle_memory_no_clear());
  for (uint32_t char_index = 0; char_index < size; ++char_index) {
    dest[char_index] = source[char_index];
  }

  return (dest);
}

binocle_buffer
binocle_memory_push_buffer_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                            uintptr_t size,
                            binocle_memory_arena_push_params params) {
  binocle_buffer result;
  result.count = size;
  result.data = (uint8_t *)binocle_memory_push_size_(BINOCLE_MEMORY_PASS arena,
                                                     result.count, params);

  return (result);
}

binocle_string
binocle_memory_push_cstring_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                             char *source) {
  binocle_string result;
  result.count = binocle_string_length(source);
  result.data = (uint8_t *)binocle_memory_push_copy_(
    BINOCLE_MEMORY_PASS arena, result.count, source,
    binocle_memory_default_arena_params());

  return (result);
}

binocle_string
binocle_memory_push_string_(BINOCLE_MEMORY_PARAM binocle_memory_arena *arena,
                            binocle_string source) {
  binocle_string result;
  result.count = source.count;
  result.data = (uint8_t *)binocle_memory_push_copy_(
    BINOCLE_MEMORY_PASS arena, result.count, source.data,
    binocle_memory_default_arena_params());

  return (result);
}

char *binocle_memory_push_and_null_terminate_(
  BINOCLE_MEMORY_PARAM binocle_memory_arena *arena, uint32_t length,
  const char *source) {
  char *dest = (char *)binocle_memory_push_size_(
    BINOCLE_MEMORY_PASS arena, length + 1, binocle_memory_no_clear());
  for (uint32_t char_index = 0; char_index < length; ++char_index) {
    dest[char_index] = source[char_index];
  }
  dest[length] = 0;

  return (dest);
}