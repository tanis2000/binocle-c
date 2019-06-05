//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_ecs.h"
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define BINOCLE_FOREACH_SPARSEINTSET(I, N, S) for(N = 0, I = (S)->dense ? (S)->dense[N] : 0; N < (S)->size; N++, I = (S)->dense[N])
#define BINOCLE_FOREACH_DENSEINTSET(I, D) for(I = 0; I < (D)->capacity; I++) if(binocle_bits_is_set((D)->bytes, I))
#define BINOCLE_FOREACH_ARRAY(T, N, A, S) for(N = 0, T = A; N < S; N++, T++)

bool binocle_sparse_integer_set_insert(binocle_sparse_integer_set_t *set, uint64_t i) {
  if (i >= set->capacity) {
    uint64_t new_capacity = (uint64_t)((i + 1) * 1.5f);
    uint64_t *new_dense = realloc(set->dense, new_capacity * sizeof(uint64_t));
    if (new_dense == NULL) {
      return false;
    }
    uint64_t *new_sparse = realloc(set->sparse, new_capacity * sizeof(uint64_t));
    if (new_sparse == NULL) {
      return false;
    }
    set->dense = new_dense;
    set->sparse = new_sparse;
    set->capacity = new_capacity;
  }
  uint64_t a = set->sparse[i];
  uint64_t n = set->size;
  if (a >= n || set->dense[a] != i) {
    set->sparse[i] = n;
    set->dense[n] = i;
    set->size = n + 1;
  }
  return true;
}

bool binocle_sparse_integer_set_remove(binocle_sparse_integer_set_t *set, uint64_t i) {
  if (i >= set->capacity || set->size == 0) {
    return false;
  }
  uint64_t a = set->sparse[i];
  uint64_t n = set->size - 1;
  if (a <= n || set->dense[a] == i) {
    uint64_t e = set->dense[n];
    set->size = n;
    set->dense[n] = e;
    set->sparse[e] = a;
  }
  return true;
}

bool binocle_sparse_integer_set_contains(binocle_sparse_integer_set_t *set, uint64_t i) {
  if (i >= set->size) {
    return false;
  }
  uint64_t a = set->sparse[i];
  uint64_t n = set->size;
  return a < n && set->dense[a] == i;
}

void binocle_sparse_integer_set_clear(binocle_sparse_integer_set_t *set) {
  set->size = 0;
}

bool binocle_sparse_integer_set_is_empty(binocle_sparse_integer_set_t *set) {
  return set->size == 0;
}

uint64_t binocle_sparse_integer_set_pop(binocle_sparse_integer_set_t *set) {
  if(set->size >= 1) {
    return set->dense[--set->size];
  }
  return UINT64_MAX;
}

void binocle_sparse_integer_set_free(binocle_sparse_integer_set_t *set) {
  free(set->dense);
  free(set->sparse);
  memset(set, 0, sizeof(*set));
}




int binocle_bits_set(unsigned char *bytes, unsigned int bit) {
  int r = !!(bytes[bit >> 3] & (1 << (bit & 7)));
  bytes[bit >> 3] |= (1 << (bit & 7));
  return r;
}

int binocle_bits_is_set(unsigned char *bytes, unsigned int bit) {
  return !!(bytes[bit >> 3] & (1 << (bit & 7)));
}

int binocle_bits_clear(unsigned char *bytes, unsigned int bit) {
  int r = !!(bytes[bit >> 3] & (1 << (bit & 7)));
  bytes[bit >> 3] &= ~(1 << (bit & 7));
  return r;
}


int binocle_dense_integer_set_contains(binocle_dense_integer_set_t *is, uint64_t i) {
	return i < is->capacity && binocle_bits_is_set(is->bytes, i);
}

uint64_t binocle_dense_integer_set_insert(binocle_dense_integer_set_t *is, uint64_t i) {
  if (i >= is->capacity) {
    uint64_t new_capacity = (uint64_t)((i + 1) * 1.5f);
    is->bytes = realloc(is->bytes, (new_capacity + 7) >> 3);
    is->capacity = new_capacity;
  }
  return binocle_bits_set(is->bytes, i);
}

uint64_t binocle_dense_integer_set_remove(binocle_dense_integer_set_t *is, uint64_t i) {
  if (i < is->capacity) {
    return binocle_bits_clear(is->bytes, i);
  }
  return 0;
}

void binocle_dense_integer_set_clear(binocle_dense_integer_set_t *is) {
	memset(is->bytes, 0, (is->capacity + 7) >> 3);
}

int binocle_dense_integer_set_is_empty(binocle_dense_integer_set_t *is) {
	uint64_t n = (is->capacity + 7) >> 3, i = 0;
	for (; i < n; i++) {
		if (is->bytes[i]) {
			return 0;
		}
	}
	return 1;
}

void binocle_dense_integer_set_free(binocle_dense_integer_set_t *is) {
  free(is->bytes);
  memset(is, 0, sizeof(*is));
}

binocle_ecs_t binocle_ecs_new() {
  binocle_ecs_t res = { 0 };
  return res;
}

bool binocle_ecs_free(binocle_ecs_t *ecs) {
  uint64_t i;
  uint64_t j;
  binocle_component_t *component;

  bool res = binocle_ecs_fix_data(ecs);
  if (!res) {
    return false;
  }

  for (i = 0 ; i < ecs->next_entity_id ; i++) {
    for (j = 0 ; j < ecs->num_components ; j++) {
      binocle_ecs_remove_components(ecs, i, j);
    }
  }

  free(ecs->data);
  binocle_sparse_integer_set_free(&ecs->free_entity_ids);
  binocle_sparse_integer_set_free(&ecs->added);
  binocle_sparse_integer_set_free(&ecs->enabled);
  binocle_sparse_integer_set_free(&ecs->disabled);
  binocle_sparse_integer_set_free(&ecs->removed);
  binocle_dense_integer_set_free(&ecs->active);

  BINOCLE_FOREACH_ARRAY(component, i, ecs->components, ecs->num_components) {
    binocle_ecs_component_free(ecs, component);
  }
  free(ecs->components);

  /*
  BINOCLE_FOREACH_ARRAY(system, i, diana->systems, diana->num_systems) {
    _system_free(diana, system);
  }
  _free(diana, diana->systems);

  BINOCLE_FOREACH_ARRAY(manager, i, diana->managers, diana->num_managers) {
    _manager_free(diana, manager);
  }
  _free(diana, diana->managers);
*/
  return true;
}

bool binocle_ecs_initialize(binocle_ecs_t *ecs) {
  uint64_t extra_bytes = (ecs->num_components + 7) >> 3;
  uint64_t n;
  binocle_component_t *c;

  if (ecs->initialized) {
    return false;
  }

  BINOCLE_FOREACH_ARRAY(c, n, ecs->components, ecs->num_components) {
    c->offset += extra_bytes;
  }

  ecs->data_width += extra_bytes;

  ecs->initialized = true;

  return true;
}

bool binocle_ecs_fix_data(binocle_ecs_t *ecs) {
  // Takes care of spawns happening while processing
  if (ecs->processing_data != NULL) {
    uint64_t new_data_height = ecs->data_height + ecs->processing_data_height;
    uint64_t i;

    if (new_data_height >= ecs->data_height_capacity) {
      uint64_t new_data_height_capacity = (uint64_t)((new_data_height + 1) * 1.5f);
      void * new_data = realloc(ecs->data, ecs->data_width * new_data_height_capacity);
      if (new_data == NULL) {
        return false;
      }
      ecs->data = new_data;
      ecs->data_height_capacity = new_data_height_capacity;
    }

    for (uint64_t i = 0; i < ecs->processing_data_height; i++) {
      memcpy((unsigned char *)ecs->data + (ecs->data_width * (ecs->data_height + i)), ecs->processing_data[i], ecs->data_width);
      free(ecs->processing_data[i]);
    }
    free(ecs->processing_data);

    ecs->data_height = new_data_height;

    ecs->processing_data = NULL;
    ecs->processing_data_height = 0;
  }
  return true;
}

bool binocle_ecs_create_component(binocle_ecs_t *ecs, const char *name, uint64_t component_size, binocle_component_id_t *component_ptr) {
  binocle_component_t c;

  if (ecs->initialized) {
    return false;
  }

  memset(&c, 0, sizeof(c));
  c.name = strdup(name);
  c.size = component_size;
  c.offset = ecs->data_width;
  ecs->data_width += component_size;
  ecs->components = realloc(ecs->components, sizeof(*ecs->components) * (ecs->num_components + 1));
  ecs->components[ecs->num_components++] = c;
  *component_ptr = ecs->num_components - 1;
  return true;
}

bool binocle_ecs_remove_components(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component) {
  unsigned char *entity_data;
  binocle_component_t *c;
  uint64_t i;

  if(!ecs->initialized) {
    return false;
  }

  if((!ecs->processing && entity >= ecs->data_height) || (ecs->processing && entity >= ecs->data_height_capacity + ecs->processing_data_height)) {
    return false;
  }

  if(component >= ecs->num_components) {
    return false;
  }

  entity_data = binocle_ecs_get_entity_data(ecs, entity);
  c = ecs->components + component;

  return binocle_ecs_remove_component_i_internal(ecs, entity, component, 0);
}


bool binocle_ecs_create_entity(binocle_ecs_t *ecs, binocle_entity_id_t *entity_ptr) {
  binocle_entity_id_t r;
  if (!ecs->initialized) {
    return false;
  }

  if (binocle_sparse_integer_set_is_empty(&ecs->free_entity_ids)) {
    r = ecs->next_entity_id++;
  } else {
    r = binocle_sparse_integer_set_pop(&ecs->free_entity_ids);
  }

  ecs->data_height = ecs->data_height > (r + 1) ? ecs->data_height : (r + 1);

  if (ecs->data_height > ecs->data_height_capacity) {
    if (ecs->processing) {
      void *entity_data = malloc(ecs->data_width);
      if (entity_data == NULL) {
        return false;
      }
      void *new_processing_data = realloc(ecs->processing_data, sizeof(ecs->processing_data) * (ecs->processing_data_height + 1));
      if (new_processing_data == NULL) {
        return false;
      }
      ecs->processing_data = new_processing_data;
      ecs->processing_data[ecs->processing_data_height++] = entity_data;
    } else {
      uint64_t new_data_height_capacity = (uint64_t)(ecs->data_height * 1.5f);
      void *new_data = realloc(ecs->data, ecs->data_width * new_data_height_capacity);
      if (new_data == NULL) {
        return false;
      }
      ecs->data = new_data;
      ecs->data_height_capacity = new_data_height_capacity;
    }
  }

  *entity_ptr = r;
  return true;
}

unsigned char *binocle_ecs_get_entity_data(binocle_ecs_t *ecs, binocle_entity_id_t entity) {
  if(entity >= ecs->data_height_capacity) {
    return ecs->processing_data[entity - ecs->data_height_capacity];
  }
  return (void *)((unsigned char *)ecs->data + (ecs->data_width * entity));
}

bool binocle_ecs_remove_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i) {
  unsigned char *entity_data = binocle_ecs_get_entity_data(ecs, entity);
  binocle_component_t *c = ecs->components + component;

  if(!binocle_bits_clear(entity_data, component)) {
    return true;
  }

  return true;
}

void binocle_ecs_component_free(binocle_ecs_t *ecs, binocle_component_t *component) {
  uint64_t i = 0;
  free((void *)component->name);
  for(i = 0; i < component->next_data_index; i++) {
    free(component->data[i]);
  }
  free(component->data);
  binocle_sparse_integer_set_free(&component->free_data_indexes);
  memset(component, 0, sizeof(*component));
}

bool binocle_ecs_set_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, const void * data) {
	if(!ecs->initialized) {
		return false;
	}

	if((!ecs->processing && entity >= ecs->data_height) || (ecs->processing && entity >= ecs->data_height_capacity + ecs->processing_data_height)) {
		return false;
	}

	if(component >= ecs->num_components) {
		return false;
	}

	return binocle_ecs_set_component_i_internal(ecs, entity, component, 0, data);
}

bool binocle_ecs_set_component_i_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, const void * data) {
	unsigned char *entity_data = binocle_ecs_get_entity_data(ecs, entity);
	binocle_component_t *c = ecs->components + component;
	int defined = binocle_bits_set(entity_data, component);
	void *component_data = NULL;

  component_data = (void *)(entity_data + c->offset);

	if(data != NULL) {
		memcpy(component_data, data, c->size);
	}

	return true;
}

bool binocle_ecs_get_component(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, void ** ptr) {
	if(!ecs->initialized) {
		return false;
	}

	if((!ecs->processing && entity >= ecs->data_height) || (ecs->processing && entity >= ecs->data_height_capacity + ecs->processing_data_height)) {
		return false;
	}

	if(component >= ecs->num_components) {
		return false;
	}

	return binocle_ecs_get_component_internal(ecs, entity, component, 0, ptr);
}

bool binocle_ecs_get_component_internal(binocle_ecs_t *ecs, binocle_entity_id_t entity, binocle_component_id_t component, uint64_t i, void ** ptr) {
	unsigned char *entity_data = binocle_ecs_get_entity_data(ecs, entity);
	binocle_component_t *c = ecs->components + component;
	void *component_data = NULL;

	if(!binocle_bits_is_set(entity_data, component)) {
		return false;
	}

  component_data = (void *)(entity_data + c->offset);

	*ptr = component_data;

	return true;
}