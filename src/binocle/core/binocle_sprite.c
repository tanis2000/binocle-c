//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <string.h>
#include <limits.h>
#include "binocle_sprite.h"
#include "binocle_subtexture.h"
#include "backend/binocle_material.h"
#include "binocle_gd.h"
#include "backend/binocle_vpct.h"
#include "binocle_log.h"
#include "binocle_camera.h"
#include <ksort/ksort.h>

#define BINOCLE_SPRITE_VERTEX_COUNT 6

#define batch_item_lt(a, b) ((a).sort_key < (b).sort_key)
KSORT_INIT(sort_sprite_batch_item, binocle_sprite_batch_item, batch_item_lt)
KSORT_INIT_GENERIC(float)


binocle_sprite *binocle_sprite_from_material(binocle_material *material) {
  binocle_sprite *res = malloc(sizeof(binocle_sprite));
  memset(res, 0, sizeof(*res));
  res->animations =
      malloc(sizeof(binocle_sprite_animation) * BINOCLE_SPRITE_MAX_ANIMATIONS);
  memset(res->animations, 0, sizeof(*res->animations) * BINOCLE_SPRITE_MAX_ANIMATIONS);
  res->frames =
      malloc(sizeof(binocle_sprite_frame) * BINOCLE_SPRITE_MAX_FRAMES);
  memset(res->frames, 0, sizeof(*res->frames) * BINOCLE_SPRITE_MAX_FRAMES);
  // Default origin to bottom-left
  res->origin.x = 0;
  res->origin.y = 0;
  res->material = material;
  // Default to use the whole texture
  sg_image_info info = sg_query_image_info(material->albedo_texture);
  res->subtexture = binocle_subtexture_with_texture(&material->albedo_texture, 0, 0, info.width,
                                                   info.height);
  for (int i = 0; i < BINOCLE_SPRITE_MAX_ANIMATIONS; i++) {
    res->animations[i].enabled = false;
  }
  res->frames_number = 0;
  res->playing = false;
  res->finished = false;
  res->rate = 1;
  res->timer = 0;
  res->current_frame = 0;
  res->current_animation = NULL;
  res->current_animation_frame = 0;
  res->current_animation_id = 0;
  return res;
}

void binocle_sprite_destroy(struct binocle_sprite *sprite) {
  free(sprite->animations);
  for (int i = 0 ; i < sprite->frames_number ; i++) {
    free(sprite->frames[i].subtexture);
  }
  free(sprite->frames);
  free(sprite);
  sprite = NULL;
}

void binocle_sprite_draw(binocle_sprite *sprite, binocle_gd *gd, int64_t x, int64_t y, kmAABB2 *viewport, float rotation,
                         kmVec2 *scale, binocle_camera *camera) {
  binocle_vpct vertices[BINOCLE_SPRITE_VERTEX_COUNT];
  binocle_subtexture *s;
  float w, h;
  if (sprite->frames_number > 0) {
    s = sprite->frames[sprite->current_frame].subtexture;
    w = s->rect.max.x;
    h = s->rect.max.y;
//    binocle_log_info("current frame %d", sprite->current_frame);
//    binocle_log_info("subtexture %f %f %f %f %s", s->rect.min.x, s->rect.min.y, s->rect.max.x, s->rect.max.y, s->name);
  } else {
    s = &sprite->subtexture;
    w = sprite->subtexture.rect.max.x;
    h = sprite->subtexture.rect.max.y;
  }

  sg_image_info info = sg_query_image_info(sprite->material->albedo_texture);

  // TL
  vertices[0].pos.x =
      -sprite->origin.x * scale->x * cosf(rotation) - (-sprite->origin.y * scale->y + h * scale->y) * sinf(rotation) + x;
  vertices[0].pos.y =
      (-sprite->origin.y * scale->y + h * scale->y) * cosf(rotation) - sprite->origin.x * scale->x * sinf(rotation) + y;
  vertices[0].color = binocle_color_white();
  vertices[0].tex.x = s->rect.min.x / info.width;
  vertices[0].tex.y = (s->rect.min.y + s->rect.max.y) / info.height;
  // TR
  vertices[1].pos.x = (-sprite->origin.x * scale->x + w * scale->x) * cosf(rotation) -
                      (-sprite->origin.y * scale->y + h * scale->y) * sinf(rotation) + x;
  vertices[1].pos.y = (-sprite->origin.y * scale->y + h * scale->y) * cosf(rotation) +
                      (-sprite->origin.x * scale->x + w * scale->x) * sinf(rotation) + y;
  vertices[1].color = binocle_color_white();
  vertices[1].tex.x = (s->rect.min.x + s->rect.max.x) / info.width;
  vertices[1].tex.y = (s->rect.min.y + s->rect.max.y) / info.height;
  // BL
  vertices[2].pos.x = -sprite->origin.x * scale->x * cosf(rotation) + sprite->origin.y * scale->y * sinf(rotation) + x;
  vertices[2].pos.y = -sprite->origin.y * scale->y * cosf(rotation) - sprite->origin.x * scale->x * sinf(rotation) + y;
  vertices[2].color = binocle_color_white();
  vertices[2].tex.x = s->rect.min.x / info.width;
  vertices[2].tex.y = s->rect.min.y / info.height;
  // TR
  vertices[3].pos.x = (-sprite->origin.x * scale->x + w * scale->x) * cosf(rotation) -
                      (-sprite->origin.y * scale->y + h * scale->y) * sinf(rotation) + x;
  vertices[3].pos.y = (-sprite->origin.y * scale->y + h * scale->y) * cosf(rotation) +
                      (-sprite->origin.x * scale->x + w * scale->x) * sinf(rotation) + y;
  vertices[3].color = binocle_color_white();
  vertices[3].tex.x = (s->rect.min.x + s->rect.max.x) / info.width;
  vertices[3].tex.y = (s->rect.min.y + s->rect.max.y) / info.height;
  // BR
  vertices[4].pos.x =
      (-sprite->origin.x * scale->x + w * scale->x) * cosf(rotation) + sprite->origin.y * scale->y * sinf(rotation) + x;
  vertices[4].pos.y =
      -sprite->origin.y * scale->y * cosf(rotation) + (-sprite->origin.x * scale->x + w * scale->x) * sinf(rotation) + y;
  vertices[4].color = binocle_color_white();
  vertices[4].tex.x = (s->rect.min.x + s->rect.max.x) / info.width;
  vertices[4].tex.y = s->rect.min.y / info.height;
  // BL
  vertices[5].pos.x = -sprite->origin.x * scale->x * cosf(rotation) + sprite->origin.y * scale->y * sinf(rotation) + x;
  vertices[5].pos.y = -sprite->origin.y * scale->y * cosf(rotation) - sprite->origin.x * scale->x * sinf(rotation) + y;
  vertices[5].color = binocle_color_white();
  vertices[5].tex.x = s->rect.min.x / info.width;
  vertices[5].tex.y = s->rect.min.y / info.height;

  binocle_gd_draw(gd, vertices, BINOCLE_SPRITE_VERTEX_COUNT, *sprite->material, *viewport, camera);
}

void binocle_sprite_draw_with_sprite_batch(binocle_sprite_batch *sprite_batch, binocle_sprite *sprite, binocle_gd *gd, int64_t x, int64_t y, kmAABB2 *viewport, float rotation,
                                           kmVec2 *scale, binocle_camera *camera, float depth) {
  kmVec2 pos;
  pos.x = x;
  pos.y = y;

  binocle_subtexture *s;
  float w, h;
  if (sprite->frames_number > 0) {
    s = sprite->frames[sprite->current_frame].subtexture;
    w = s->rect.max.x;
    h = s->rect.max.y;
    //binocle_log_info("subtexture %f %f %f %f %d %d", s.rect.min.x, s.rect.min.y, s.rect.max.x, s.rect.max.y, sprite.material->texture->width, sprite.material->texture->height);
  } else {
    s = &sprite->subtexture;
    w = sprite->subtexture.rect.max.x;
    h = sprite->subtexture.rect.max.y;
  }

  kmMat4 *m = binocle_camera_get_transform_matrix(camera);
  scale->x *= m->mat[4];
  scale->y *= m->mat[4];

  binocle_sprite_batch_draw(sprite_batch, &sprite->material->albedo_texture, &pos, NULL, &s->rect, NULL, 0.0f, scale, binocle_color_white(), depth);
}

void binocle_sprite_add_frame(binocle_sprite *sprite, binocle_sprite_frame frame) {
  memcpy(&sprite->frames[sprite->frames_number], &frame, sizeof(binocle_sprite_frame));
  sprite->frames_number++;
}


binocle_sprite_frame binocle_sprite_frame_from_subtexture(struct binocle_subtexture *subtexture) {
  binocle_sprite_frame res = {0};
  res.subtexture = SDL_malloc(sizeof(binocle_subtexture));
  res.subtexture->rect = (kmAABB2) {
    .min = {
      .x = subtexture->rect.min.x,
      .y = subtexture->rect.min.y,
    },
    .max = {
      .x = subtexture->rect.max.x,
      .y = subtexture->rect.max.y,
    }
  };
  res.subtexture->texture = subtexture->texture;
  strcpy(res.subtexture->name, subtexture->name);
  res.origin = binocle_subtexture_get_center(subtexture);
  return res;
}

binocle_sprite_frame
binocle_sprite_frame_from_subtexture_and_origin(struct binocle_subtexture *subtexture, kmVec2 origin) {
  binocle_sprite_frame res = {0};
  res.subtexture = subtexture;
  res.origin = origin;
  return res;
}

binocle_sprite_animation *binocle_sprite_create_animation_with_name(binocle_sprite *sprite, char *name) {
  binocle_sprite_animation *res = &sprite->animations[sprite->animations_number];
  res->name = SDL_malloc(SDL_strlen(name) + 1);
  strcpy(res->name, name);
  res->enabled = true;
  //res.delay = 0;
  res->looping = false;
  for (int i = 0; i < BINOCLE_SPRITE_MAX_FRAMES; i++) {
    res->frames[i] = -1;
  }
  res->frames_number = 0;
  sprite->animations_number++;
  return res;
}


void binocle_sprite_add_animation(binocle_sprite *sprite, int id, int frame) {
  binocle_sprite_animation *res = &sprite->animations[id];
  res->enabled = true;
  //res.delay = 0;
  res->looping = false;
  for (int i = 0; i < BINOCLE_SPRITE_MAX_FRAMES; i++) {
    res->frames[i] = -1;
  }
  res->frames[0] = frame;
  res->frames_number = 1;
}

void binocle_sprite_add_animation_with_frames(binocle_sprite *sprite, int id, bool looping, float delay, int frames[],
                                              int frames_count) {
  binocle_sprite_animation *res = &sprite->animations[id];
  res->enabled = true;
  //res.delay = delay;
  res->looping = looping;
  for (int i = 0; i < BINOCLE_SPRITE_MAX_FRAMES; i++) {
    res->frames[i] = -1;
  }
  for (int i = 0; i < frames_count; i++) {
    res->frames[i] = frames[i];
  }
  res->frames_number = frames_count;
}

bool binocle_sprite_has_animation(binocle_sprite *sprite, const char *animation_name) {
  for (int i = 0 ; i < sprite->animations_number ; i++) {
    if (SDL_strcmp(sprite->animations[i].name, animation_name) == 0) {
      return true;
    }
  }
  return false;
}

void binocle_sprite_play(binocle_sprite *sprite, int id, bool restart) {
  if (restart || (!sprite->playing && !sprite->finished) || sprite->current_animation_id != id) {
    sprite->current_animation_id = id;
    sprite->current_animation = &sprite->animations[id];

    sprite->current_animation_frame = 0;
    sprite->current_frame = sprite->current_animation->frames[0];
    sprite->timer = 0;

    sprite->finished = false;
    sprite->playing = true;
  }
}

void binocle_sprite_play_from_frame(binocle_sprite *sprite, int id, int start_frame, bool restart) {
  if (!sprite->playing || sprite->current_animation_id != id || restart) {
    binocle_sprite_play(sprite, id, true);
    sprite->current_animation_frame = start_frame;
    sprite->current_frame = sprite->current_animation->frames[sprite->current_animation_frame];
  }
}

void binocle_sprite_stop(binocle_sprite *sprite) {
  sprite->current_animation_frame = 0;
  sprite->finished = sprite->playing = false;
}

void binocle_sprite_update(binocle_sprite *sprite, float dt) {
  //binocle_log_info("dt %f, sprite->rate %f", dt, sprite->rate);
  if (sprite->playing && sprite->current_animation->delays[sprite->current_animation_frame] > 0) {
    sprite->timer += (dt * fabsf(sprite->rate));
    //binocle_log_info("timer %f fabsf(sprite->rate) %f", sprite->timer, fabsf(sprite->rate));
    while (sprite->timer >= sprite->current_animation->delays[sprite->current_animation_frame]) {
      //binocle_log_info("sprite->timer %f sprite->current_animation->delay %f", sprite->timer, sprite->current_animation->delay);
      int oldFrame = sprite->current_frame;
      sprite->timer -= sprite->current_animation->delays[sprite->current_animation_frame];
      sprite->current_animation_frame += SIGNOF(sprite->rate);

      if (sprite->current_animation_frame == sprite->current_animation->frames_number) {
        //Looping
        sprite->current_animation_frame = 0;
        if (sprite->current_animation->looping) {
          sprite->current_frame = sprite->current_animation->frames[0];
        } else {
          sprite->finished = true;
          sprite->playing = false;
        }
      } else if (sprite->current_animation_frame == -1) {
        //Reverse looping
        sprite->current_animation_frame = sprite->current_animation->frames_number - 1;
        if (sprite->current_animation->looping) {
          sprite->current_frame = sprite->current_animation->frames[0];
        } else {
          sprite->finished = true;
          sprite->playing = false;
        }
      } else {
        sprite->current_frame = sprite->current_animation->frames[sprite->current_animation_frame];
      }
    }
  }
}

int binocle_sprite_get_current_frame(binocle_sprite *sprite) {
  return sprite->current_frame;
}

void binocle_sprite_set_current_frame(binocle_sprite *sprite, int frame) {
  if (frame < -1 || frame >= sprite->frames_number) {
    //throw "Sprite frame index out of range! For null frame, set index to -1.";
    return;
  } else {
    if (sprite->playing) {
      binocle_sprite_stop(sprite);
    }
    if (frame != sprite->current_frame) {
      sprite->current_frame = frame;
    }
  }
}

void binocle_sprite_set_current_frame_by_name(binocle_sprite *sprite, const char *name) {
  if (name == NULL) {
    return;
  }
  if (sprite->playing) {
    binocle_sprite_stop(sprite);
  }
  for (int i = 0 ; i < sprite->frames_number ; i++) {
    binocle_sprite_frame *frame = &sprite->frames[i];
    if (frame->subtexture == NULL) {
      continue;
    }
    if (strcmp(frame->subtexture->name, name) == 0) {
      if (i != sprite->current_frame) {
        sprite->current_frame = i;
      }
    }
  }
}

int binocle_sprite_get_current_animation(binocle_sprite *sprite) {
  return sprite->current_animation_id;
}

void binocle_sprite_set_current_animation(binocle_sprite *sprite, int id) {
  binocle_sprite_play(sprite, id, false);
}

void binocle_sprite_clear_animations(binocle_sprite *sprite) {
  if (sprite->playing) {
    binocle_sprite_stop(sprite);
  }
  for (int i = 0; i < BINOCLE_SPRITE_MAX_ANIMATIONS; i++) {
    if (sprite->animations[i].name != NULL) {
      SDL_free(sprite->animations[i].name);
    }
    memset(&sprite->animations[i], 0, sizeof(binocle_sprite_animation));
  }
  sprite->current_animation = NULL;
  sprite->current_animation_frame = -1;
  sprite->animations_number = 0;
}

void binocle_sprite_clear_frames(binocle_sprite *sprite) {
  if (sprite->playing) {
    binocle_sprite_stop(sprite);
  }
  for (int i = 0; i < BINOCLE_SPRITE_MAX_ANIMATIONS; i++) {
    for (int j = 0; j < BINOCLE_SPRITE_MAX_FRAMES; j++) {
      sprite->animations[i].frames[j] = -1;
      sprite->animations[i].delays[j] = 0;
      memset(&sprite->animations[i].frame_mapping[j], 0, sizeof(binocle_sprite_animation_frame_mapping));
    }
    sprite->animations[i].frames_number = 0;
  }
}

void binocle_sprite_create_frames_with_string(binocle_sprite *sprite, binocle_sprite_animation *animation,
                                              char *subtextures_names, binocle_subtexture *subtextures,
                                              size_t subtextures_count) {
  int start_frame = 0;
  char *s = SDL_strdup(subtextures_names);
  char *token = strtok(s, ",");
  while (token) {
    bool added = false;
    binocle_subtexture *sub = subtextures;
    for (int i = 0; i < subtextures_count; i++) {
      if (strcmp(sub->name, token) == 0) {
        binocle_sprite_frame frame = binocle_sprite_frame_from_subtexture(sub);
        binocle_sprite_add_frame(sprite, frame);
        animation->frame_mapping[animation->frame_mapping_number].name = SDL_strdup(sub->name);
        animation->frame_mapping[animation->frame_mapping_number].original_frame = start_frame;
        animation->frame_mapping[animation->frame_mapping_number].real_frame = sprite->frames_number - 1;
        animation->frame_mapping_number++;
        /*
        animation->looping = loop;
        animation->delay = 1; // TODO: remove this
        animation->frames[animation->frames_number] = sprite->frames_number-1;
        animation->frames_number++;
         */
        added = true;
        start_frame++;
      }
      sub++;
    }
    if (added) {
      binocle_log_info("Frame %s added", token);
    } else {
      binocle_log_error("Cannot find frame with name %s", token);
    }
    token = strtok(NULL, ",");
  }
}

typedef struct binocle_frames_codes {
  char *frames_sequence;
  float delay;
} binocle_frames_codes;

typedef struct binocle_animation_codes {
  char *animation_sequence;
} binocle_animation_codes;

int binocle_sprite_get_real_frame_for_animation_from_mapping(binocle_sprite_animation *animation, int original_frame) {
  for (int i = 0; i < animation->frame_mapping_number; i++) {
    if (animation->frame_mapping[i].original_frame == original_frame) {
      return animation->frame_mapping[i].real_frame;
    }
  }
  binocle_log_error("Something went wrong. Can't find a frame with original frame number %d", original_frame);
  return -1;
}

void binocle_sprite_create_animation_with_string(binocle_sprite *sprite, binocle_sprite_animation *animation,
                                                 char *sequence_code, bool loop) {
  char *ptr = NULL;
  struct binocle_animation_codes anim_codes[256]; // elements separated by ,
  struct binocle_frames_codes frames_codes[256]; // elements in form like 0-9

  animation->looping = loop;

  char *s = SDL_strdup(sequence_code);
  char *anim_code = strtok(s, ",");

  int anim_count = 0;
  int frames_count = 0;

  if (anim_code) {
    // 0-1:3(.75),0-2
    while (anim_code) {
      char *a = SDL_strdup(anim_code);
      anim_codes[anim_count].animation_sequence = a;
      anim_count++;
      anim_code = strtok(NULL, ",");
    }
  } else {
    // 0-1 or 0
    char *a = SDL_strdup(anim_code);
    anim_codes[anim_count].animation_sequence = a;
    anim_count++;
  }

  for (int i = 0; i < anim_count; i++) {
    anim_code = anim_codes[i].animation_sequence;
    char *frames_code = strtok(anim_code, ":");
    if (frames_code) {
      // 0-1:3(.75) or 0-1 or 0 or 0(.75)
      char *f = SDL_strdup(frames_code);
      frames_codes[frames_count].frames_sequence = f;

      frames_code = strtok(NULL, ":");

      float delay = 1.0f;
      if (frames_code) {
        delay = strtof(frames_code, &ptr);
      }
      frames_codes[frames_count].delay = delay;
      frames_count++;
    }
  }

  for (int i = 0; i < frames_count; i++) {
    char *frames_sequence = frames_codes[i].frames_sequence;
    char *frames_code = strtok(frames_sequence, "-");
    if (frames_code) {
      // 0-1 or 0
      int start_frame = (int) strtol(frames_code, &ptr, 10);
      frames_code = strtok(NULL, "-");

      if (frames_code) {
        int end_frame = (int) strtol(frames_code, &ptr, 10);
        for (int j = start_frame; j <= end_frame; j++) {
          animation->frames[animation->frames_number] = binocle_sprite_get_real_frame_for_animation_from_mapping(
              animation, j);
          animation->delays[animation->frames_number] = frames_codes[i].delay;
          animation->frames_number++;
        }
      } else {
        animation->frames[animation->frames_number] = binocle_sprite_get_real_frame_for_animation_from_mapping(
            animation, start_frame);
        animation->delays[animation->frames_number] = frames_codes[i].delay;
        animation->frames_number++;
      }
    }
  }
  /*
  while (anim_code) {
    char *a = SDL_strdup(anim_code);
    char *frames_code = strtok(a, ":");
    if (frames_code) {
      char *f = SDL_strdup(frames_code);
      char *frame_code = strtok(f, "-");
      if (frame_code) {
        // 0-3
        while (frame_code) {
          int frame_number = (int)strtol(frame_code, &ptr, 10);
          animation->frames[animation->frames_number] = frame_number;
          animation->delays[animation->frames_number] = delay;
          animation->frames_number++;
          frame_code = strtok(NULL, "-");
        }
      } else {
        // 0
        int frame_number = (int)strtol(frame_code, &ptr, 10);
        animation->frames[animation->frames_number] = frame_number;
        animation->delays[animation->frames_number] = delay;
        animation->frames_number++;
      }

      frames_code = strtok(NULL, ":");
    }

    anim_code = strtok(NULL, ",");
  }
  */
}

void binocle_sprite_create_animation(binocle_sprite *sprite, char *name, char *subtextures_names, char *sequence_code,
                                     bool loop, binocle_subtexture *subtextures, size_t subtextures_count) {
  // create the animation with name `name`
  binocle_sprite_animation *anim = binocle_sprite_create_animation_with_name(sprite, name);
  binocle_log_info("Created animation with name %s", name);
  // assign the subtextures to the frames of the sprite
  binocle_sprite_create_frames_with_string(sprite, anim, subtextures_names, subtextures, subtextures_count);
  // build the animation using the sequence code
  binocle_sprite_create_animation_with_string(sprite, anim, sequence_code, loop);
}

int binocle_sprite_get_animation_id(binocle_sprite *sprite, char *name) {
  binocle_sprite_animation *anim = sprite->animations;
  for (int i = 0; i < sprite->animations_number; i++) {
    if (anim->name != NULL && strcmp(anim->name, name) == 0) {
      return i;
    }
    anim++;
  }
  binocle_log_warning("Cannot find animation with name %s", name);
  return -1;
}

void binocle_sprite_play_animation(binocle_sprite *sprite, char *name, bool restart) {
  int id = binocle_sprite_get_animation_id(sprite, name);
  if (restart || (!sprite->playing && !sprite->finished) || sprite->current_animation_id != id) {
    sprite->current_animation_id = id;
    sprite->current_animation = &sprite->animations[id];

    sprite->current_animation_frame = 0;
    sprite->current_frame = sprite->current_animation->frames[0];
    sprite->timer = 0;

    sprite->finished = false;
    sprite->playing = true;
  }
}

//
// Sprite Batch Item
//

binocle_sprite_batch_item binocle_sprite_batch_item_new() {
  binocle_sprite_batch_item res = { 0 };
  res.texture = NULL;
  res.vertex_tl.color = binocle_color_white();
  res.vertex_tl.pos.x = 0;
  res.vertex_tl.pos.y = 0;
  res.vertex_tr.color = binocle_color_white();
  res.vertex_tr.pos.x = 0;
  res.vertex_tr.pos.y = 0;
  res.vertex_bl.color = binocle_color_white();
  res.vertex_bl.pos.x = 0;
  res.vertex_bl.pos.y = 0;
  res.vertex_br.color = binocle_color_white();
  res.vertex_br.pos.x = 0;
  res.vertex_br.pos.y = 0;
  res.sort_key = 0;
  return res;
}

void binocle_sprite_batch_item_set(binocle_sprite_batch_item *item, float x, float y, float dx, float dy, float w, float h, float sin, float cos, sg_color color, kmVec2 tex_coord_tl, kmVec2 tex_coord_br, float depth, sg_image *texture) {
  item->vertex_tl.pos.x = x + dx * cos - dy * sin;
  item->vertex_tl.pos.y = y + dx * sin + dy * cos;
  item->vertex_tl.color = color;
  item->vertex_tl.tex.x = tex_coord_tl.x;
  item->vertex_tl.tex.y = tex_coord_tl.y;

  item->vertex_tr.pos.x = x + (dx + w) * cos - dy * sin;
  item->vertex_tr.pos.y = y + (dx + w) * sin + dy * cos;
  item->vertex_tr.color = color;
  item->vertex_tr.tex.x = tex_coord_br.x;
  item->vertex_tr.tex.y = tex_coord_tl.y;

  item->vertex_bl.pos.x = x + dx * cos - (dy + h) * sin;
  item->vertex_bl.pos.y = y + dx * sin + (dy + h) * cos;
  item->vertex_bl.color = color;
  item->vertex_bl.tex.x = tex_coord_tl.x;
  item->vertex_bl.tex.y = tex_coord_br.y;

  item->vertex_br.pos.x = x + (dx + w) * cos - (dy + h) * sin;
  item->vertex_br.pos.y = y + (dx + w) * sin + (dy + h) * cos;
  item->vertex_br.color = color;
  item->vertex_br.tex.x = tex_coord_br.x;
  item->vertex_br.tex.y = tex_coord_br.y;

  item->sort_key = depth;
  item->texture = texture;
}

//
// Sprite Batcher
//

binocle_sprite_batcher binocle_sprite_batcher_new() {
  binocle_sprite_batcher res = { 0 };
  res.initial_batch_size = 256;
  res.max_batch_size = ULONG_MAX / 6; // 6 = 4 vertices unique and 2 shared, per quad
  res.batch_item_list = malloc(sizeof(binocle_sprite_batch_item) * res.initial_batch_size);
  res.batch_item_list_size = 0;
  res.batch_item_list_capacity = res.initial_batch_size;
  res.initial_vertex_array_size = 256 * 6;
  res.index = malloc(sizeof(uint64_t) * res.initial_vertex_array_size);
  res.index_size = 0;
  res.index_capacity = res.initial_vertex_array_size;
  res.vertex_array = malloc(sizeof(binocle_vpct) * res.initial_vertex_array_size);
  res.vertex_array_size = 0;
  res.vertex_array_capacity = res.initial_vertex_array_size;

  binocle_sprite_batcher_ensure_array_capacity(&res, 256);

  return res;
}

binocle_sprite_batch_item *binocle_sprite_batcher_create_batch_item(binocle_sprite_batcher *batcher) {
  if (batcher->batch_item_list_size >= batcher->batch_item_list_capacity) {
    uint64_t old_size = batcher->batch_item_list_capacity;
    uint64_t new_size = old_size + old_size / 2; // grow by x1.5
    new_size = (new_size + 63) & (~63); // grow in chunks of 64.
    batcher->batch_item_list = realloc(batcher->batch_item_list, sizeof(binocle_sprite_batch_item) * new_size);
    batcher->batch_item_list_capacity = new_size;
  }
  binocle_sprite_batch_item *item = &batcher->batch_item_list[batcher->batch_item_list_size];
  batcher->batch_item_list_size++;
  return item;
}

void binocle_sprite_batcher_ensure_array_capacity(binocle_sprite_batcher *batcher, uint64_t num_batch_items) {
  uint64_t needed_capacity = 6 * num_batch_items;
  if (needed_capacity <= batcher->index_capacity) {
    // Short circuit out of here because we have enough capacity.
    return;
  }

  batcher->index = realloc(batcher->index, sizeof(uint64_t) * needed_capacity);
  batcher->index_capacity = needed_capacity;

  uint64_t start = batcher->index_size / 6;
  for (uint64_t i = start ; i < num_batch_items ; i++) {
    /*
      *  TL    TR
      *   0----1 0,1,2,3 = index offsets for vertex indices
      *   |   /| TL,TR,BL,BR are vertex references in SpriteBatchItem.
      *   |  / |
      *   | /  |
      *   |/   |
      *   2----3
      *  BL    BR
      */
    // Triangle 1
    batcher->index[(i * 6)] = i * 4;
    batcher->index[(i * 6 + 1)] = i * 4 + 1;
    batcher->index[(i * 6 + 2)] = i * 4 + 2;
    // Triangle 2
    batcher->index[(i * 6 + 3)] = i * 4 + 1;
    batcher->index[(i * 6 + 4)] = i * 4 + 3;
    batcher->index[(i * 6 + 5)] = i * 4 + 2;
    batcher->index_size += 6;
  }

  batcher->vertex_array = realloc(batcher->vertex_array, sizeof(binocle_vpct) * needed_capacity);
  batcher->vertex_array_capacity = needed_capacity;
}

void binocle_sprite_batcher_draw_batch(binocle_sprite_batcher *batcher, binocle_sprite_sort_mode sort_mode, binocle_render_state *render_state, binocle_gd *gd) {
  if (batcher->batch_item_list_size == 0) {
    // Nothing to do
    return;
  }

  // sort the batch items
  ks_mergesort(sort_sprite_batch_item, batcher->batch_item_list_size, batcher->batch_item_list, 0);

  // Determine how many iterations through the drawing code we need to make
  uint64_t batch_index = 0;
  uint64_t batch_count = batcher->batch_item_list_size;

  // Iterate through the batches, doing short.MaxValue sets of vertices only.
  while (batch_count > 0) {
    // setup the vertexArray array
    uint64_t start_index = 0;
    uint64_t index = 0;
    sg_image *tex = NULL;

    uint64_t num_batches_to_process = batch_count;
    if (num_batches_to_process > batcher->max_batch_size) {
      num_batches_to_process = batcher->max_batch_size;
    }

    binocle_sprite_batcher_ensure_array_capacity(batcher, num_batches_to_process);

    // Draw the batches
    for (int i = 0 ; i < num_batches_to_process ; i++) {
      // if the texture changed, we need to flush and bind the new texture
      bool should_flush = false;

      if (batcher->batch_item_list[batch_index].texture != NULL && tex == NULL) {
        should_flush = true;
      } else if (batcher->batch_item_list[batch_index].texture == NULL && tex != NULL) {
        should_flush = true;
      } else if (batcher->batch_item_list[batch_index].texture == NULL && tex == NULL) {
        should_flush = false;
      } else {
        should_flush = batcher->batch_item_list[batch_index].texture->id != tex->id;
      }
      if (should_flush) {
        binocle_sprite_batcher_flush_vertex_array(batcher, start_index, index, tex, render_state, gd);

        tex = batcher->batch_item_list[batch_index].texture;
        start_index = 0;
        index = 0;
      }

      binocle_sprite_batch_item *item = &batcher->batch_item_list[batch_index];
      // store the SpriteBatchItem data in our vertexArray
      batcher->vertex_array[index] = item->vertex_tl;
      index = index + 1;
      batcher->vertex_array[index] = item->vertex_tr;
      index = index + 1;
      batcher->vertex_array[index] = item->vertex_bl;
      index = index + 1;
      batcher->vertex_array[index] = item->vertex_tr;
      index = index + 1;
      batcher->vertex_array[index] = item->vertex_br;
      index = index + 1;
      batcher->vertex_array[index] = item->vertex_bl;
      index = index + 1;

      // Release the texture.
      item->texture = NULL;
      batch_index += 1;
    }
    // flush the remaining vertexArray data
    binocle_sprite_batcher_flush_vertex_array(batcher, start_index, index, tex, render_state, gd);
    // Update our batch count to continue the process of culling down
    // large batches
    batch_count -= num_batches_to_process;
  }
  // return items to the pool.
  batcher->batch_item_list_size = 0;
}

void binocle_sprite_batcher_flush_vertex_array(binocle_sprite_batcher *batcher, uint64_t start, uint64_t end, sg_image *texture, binocle_render_state *render_state, binocle_gd *gd) {
  if (start == end) {
    return;
  }

  uint64_t vertex_count = end - start;
  render_state->texture = texture;

  binocle_gd_draw_with_state(gd, batcher->vertex_array, vertex_count, render_state);
}

//
// Sprite Batch
//

binocle_sprite_batch binocle_sprite_batch_new() {
  binocle_sprite_batch res = {0};
  res.batcher = binocle_sprite_batcher_new();
  res.render_state = binocle_render_state_new(NULL, NULL);
  res.gd = NULL;
  res.begin_called = false;
  kmMat4Identity(&res.matrix);
  res.temp_rect.min.x = 0;
  res.temp_rect.min.y = 0;
  res.temp_rect.max.x = 0;
  res.temp_rect.max.y = 0;
  res.tex_coord_tl.x = 0;
  res.tex_coord_tl.y = 0;
  res.tex_coord_br.x = 0;
  res.tex_coord_br.y = 0;
  res.scaled_origin.x = 0;
  res.scaled_origin.y = 0;
  res.origin_rect.min.x = 0;
  res.origin_rect.min.y = 0;
  res.origin_rect.max.x = 0;
  res.origin_rect.max.y = 0;
  res.sort_mode = BINOCLE_SPRITE_SORT_MODE_IMMEDIATE;
  res.cull_rect.min.x = 0;
  res.cull_rect.min.y = 0;
  res.cull_rect.max.x = 0;
  res.cull_rect.max.y = 0;
  res.vertex_to_cull_tl.x = 0;
  res.vertex_to_cull_tl.y = 0;
  res.vertex_to_cull_tr.x = 0;
  res.vertex_to_cull_tr.y = 0;
  res.vertex_to_cull_bl.x = 0;
  res.vertex_to_cull_bl.y = 0;
  res.vertex_to_cull_br.x = 0;
  res.vertex_to_cull_br.y = 0;
  return res;
}

void binocle_sprite_batch_compute_cull_rectangle(binocle_sprite_batch *batch, kmAABB2 viewport) {
  batch->cull_rect.min.x = viewport.min.x;
  batch->cull_rect.min.y = viewport.min.y;
  batch->cull_rect.max.x = viewport.max.x;
  batch->cull_rect.max.y = viewport.max.y;
}

void binocle_sprite_batch_begin(binocle_sprite_batch *batch, kmAABB2 viewport, binocle_sprite_sort_mode sort_mode, struct sg_shader *shader, kmMat4 *transform_matrix) {
  batch->render_state.shader = shader;
  if (transform_matrix != NULL) {
    kmMat4Assign(&batch->matrix, transform_matrix);
  } else {
    kmMat4Identity(&batch->matrix);
  }
  batch->render_state.transform = batch->matrix;
  batch->sort_mode = sort_mode;
  binocle_sprite_batch_compute_cull_rectangle(batch, viewport);
  if (batch->sort_mode == BINOCLE_SPRITE_SORT_MODE_IMMEDIATE) {
    binocle_sprite_batch_setup(batch, viewport);
  }
  batch->begin_called = true;
}

void binocle_sprite_batch_end(binocle_sprite_batch *batch, kmAABB2 viewport) {
  batch->begin_called = false;
  if (batch->sort_mode != BINOCLE_SPRITE_SORT_MODE_IMMEDIATE) {
    binocle_sprite_batch_setup(batch, viewport);
  }
  binocle_sprite_batcher_draw_batch(&batch->batcher, batch->sort_mode, &batch->render_state, batch->gd);
}

void binocle_sprite_batch_setup(binocle_sprite_batch *batch, kmAABB2 viewport) {
  batch->render_state.viewport = viewport;
}

void binocle_sprite_batch_draw_internal(binocle_sprite_batch *batch, sg_image *texture, kmAABB2 *source_rectangle, sg_color color, float rotation, float depth, bool auto_flush) {
  sg_image_info info = sg_query_image_info(*texture);

  // Cull geometry outside the viewport
  batch->vertex_to_cull_tl.x = batch->origin_rect.min.x + -batch->scaled_origin.x * cosf(rotation) - -batch->scaled_origin.y * sinf(rotation);
  batch->vertex_to_cull_tl.y = batch->origin_rect.min.y + -batch->scaled_origin.x * sinf(rotation) + -batch->scaled_origin.y * cosf(rotation);

  batch->vertex_to_cull_tr.x = batch->origin_rect.min.x + (-batch->scaled_origin.x + batch->origin_rect.max.x) * cosf(rotation) - -batch->scaled_origin.y * sinf(rotation);
  batch->vertex_to_cull_tr.y = batch->origin_rect.min.y + (-batch->scaled_origin.x + batch->origin_rect.max.x) * sinf(rotation) + -batch->scaled_origin.y * cosf(rotation);

  batch->vertex_to_cull_bl.x = batch->origin_rect.min.x + -batch->scaled_origin.x * cosf(rotation) - (-batch->scaled_origin.y + batch->origin_rect.max.y) * sinf(rotation);
  batch->vertex_to_cull_bl.y = batch->origin_rect.min.y + -batch->scaled_origin.x * sinf(rotation) + (-batch->scaled_origin.y + batch->origin_rect.max.y) * cosf(rotation);

  batch->vertex_to_cull_br.x = batch->origin_rect.min.x + (-batch->scaled_origin.x + batch->origin_rect.max.x) * cosf(rotation) - (-batch->scaled_origin.y + batch->origin_rect.max.y) * sinf(rotation);
  batch->vertex_to_cull_br.y = batch->origin_rect.min.y + (-batch->scaled_origin.x + batch->origin_rect.max.x) * sinf(rotation) + (-batch->scaled_origin.y + batch->origin_rect.max.y) * cosf(rotation);

  if (source_rectangle != NULL) {
    batch->temp_rect.min.x = source_rectangle->min.x;
    batch->temp_rect.min.y = source_rectangle->min.y;
    batch->temp_rect.max.x = source_rectangle->max.x;
    batch->temp_rect.max.y = source_rectangle->max.y;
  } else {
    batch->temp_rect.min.x = 0.0f;
    batch->temp_rect.min.y = 0.0f;
    batch->temp_rect.max.x = info.width;
    batch->temp_rect.max.y = info.height;
  }

  batch->tex_coord_tl.x = batch->temp_rect.min.x / info.width;
  batch->tex_coord_tl.y = batch->temp_rect.min.y / info.height;
  batch->tex_coord_br.x = (batch->temp_rect.min.x + batch->temp_rect.max.x) / info.width;
  batch->tex_coord_br.y = (batch->temp_rect.min.y + batch->temp_rect.max.y) / info.height;

  binocle_sprite_batch_item *item = binocle_sprite_batcher_create_batch_item(&batch->batcher);
  binocle_sprite_batch_item_set(item, batch->origin_rect.min.x, batch->origin_rect.min.y,
                                -batch->scaled_origin.x, -batch->scaled_origin.y, batch->origin_rect.max.x, batch->origin_rect.max.y,
  sinf(rotation), cosf(rotation), color, batch->tex_coord_tl,
    batch->tex_coord_br, depth, texture);

  // set SortKey based on SpriteSortMode.
  switch (batch->sort_mode) {
    // Comparison of Texture objects.
    case BINOCLE_SPRITE_SORT_MODE_TEXTURE: {
      //item->sortKey = texture->sortingKey;
      break;
    }
    // Comparison of Depth
    case BINOCLE_SPRITE_SORT_MODE_FRONT_TO_BACK: {
      item->sort_key = depth;
      break;
    }
    // Comparison of Depth in reverse
    case BINOCLE_SPRITE_SORT_MODE_BACK_TO_FRONT: {
      item->sort_key = -depth;
      break;
    }
    case BINOCLE_SPRITE_SORT_MODE_DEFERRED:
      // TODO: implement deferred sorting
      break;
    case BINOCLE_SPRITE_SORT_MODE_IMMEDIATE:
      // TODO: implement immediate sorting
      break;
    default:
      break;
  }

  if (auto_flush) {
    binocle_sprite_batch_flush_if_needed(batch);
  }

}

void binocle_sprite_batch_flush_if_needed(binocle_sprite_batch *batch) {
  // Mark the end of a draw operation for Immediate SpriteSortMode.
  if (batch->sort_mode == BINOCLE_SPRITE_SORT_MODE_IMMEDIATE) {
    binocle_sprite_batcher_draw_batch(&batch->batcher, batch->sort_mode, &batch->render_state, batch->gd);
  }
}

void binocle_sprite_batch_draw(binocle_sprite_batch *batch, sg_image *texture, kmVec2 *position,
                               kmAABB2 *destination_rectangle,
                               kmAABB2 *source_rectangle, kmVec2 *origin,
                               float rotation, kmVec2 *scale, sg_color color,
                               float layer_depth) {
  kmVec2 base_origin;
  base_origin.x = 0;
  base_origin.y = 0;
  kmVec2 base_scale;
  base_scale.x = 1.0f;
  base_scale.y = 1.0f;
  // Assign default values to null parameters here, as they are not compile-time
  // constants
  if (origin != NULL) {
    base_origin.x = origin->x;
    base_origin.y = origin->y;
  }
  if (scale != NULL) {
    base_scale.x = scale->x;
    base_scale.y = scale->y;
  }

  // If both drawRectangle and position are null, or if both have been assigned
  // a value, raise an error
  if ((destination_rectangle != NULL && position != NULL) ||
      (destination_rectangle == NULL && position == NULL)) {
    binocle_log_error("Expected drawRectangle or position, but received neither or both.");
  } else if (position != NULL) {
    // Call Draw() using position
    binocle_sprite_batch_draw_vector_scale(batch, texture, position, source_rectangle, color, rotation, base_origin,
                                           base_scale, layer_depth);
  } else {
    // Call Draw() using drawRectangle
    binocle_log_error("This should call with drawRectangle but we're not yet supporting it");
    // Draw(texture, (Rectangle)destination_rectangle, source_rectangle,
    // (Color)color, rotation, (Vector2)origin, effects, layerDepth);
  }
}

void binocle_sprite_batch_draw_vector_scale(binocle_sprite_batch *batch, sg_image *texture, kmVec2 *position,
                                            kmAABB2 *source_rectangle, sg_color color,
                                            float rotation, kmVec2 origin, kmVec2 scale,
                                            float layer_depth) {
  sg_image_info info = sg_query_image_info(*texture);

  float w = info.width * scale.x;
  float h = info.height * scale.y;
  if (source_rectangle != NULL) {
    w = source_rectangle->max.x * scale.x;
    h = source_rectangle->max.y * scale.y;
  }

  batch->scaled_origin.x = origin.x * scale.x;
  batch->scaled_origin.y = origin.y * scale.y;
  batch->origin_rect.min.x = 0;
  batch->origin_rect.min.y = 0;
  if (position != NULL) {
    batch->origin_rect.min.x = position->x;
    batch->origin_rect.min.y = position->y;
  }
  batch->origin_rect.max.x = (int64_t) w;
  batch->origin_rect.max.y = (int64_t) h;
  binocle_sprite_batch_draw_internal(batch, texture, source_rectangle, color, rotation,
                                            layer_depth, true);
}

void binocle_sprite_batch_draw_float_scale(binocle_sprite_batch *batch, sg_image *texture, kmVec2 position,
                                           kmAABB2 source_rectangle, sg_color color,
                                           float rotation, kmVec2 origin, float scale,
                                           float layer_depth) {
  // CheckValid(texture);
  kmVec2 s;
  s.x = scale;
  s.y = scale;
  binocle_sprite_batch_draw_vector_scale(batch, texture, &position, &source_rectangle, color, rotation, origin, s,
                                         layer_depth);
}

void binocle_sprite_batch_draw_position(binocle_sprite_batch *batch, sg_image *texture, kmVec2 position) {
  binocle_sprite_batch_draw(batch, texture, &position, NULL, NULL, NULL, 0.0f, NULL, binocle_color_white(), 0.0f);
}

void
binocle_sprite_batch_draw_noscale(binocle_sprite_batch *batch, sg_image *texture, kmAABB2 destination_rectangle,
                                  kmAABB2 *source_rectangle, sg_color color,
                                  float rotation, kmVec2 origin,
                                  float layer_depth) {
  sg_image_info info = sg_query_image_info(*texture);

  batch->origin_rect.min.x = destination_rectangle.min.x;
  batch->origin_rect.min.y = destination_rectangle.min.y;
  batch->origin_rect.max.x = destination_rectangle.max.x;
  batch->origin_rect.max.y = destination_rectangle.max.y;

  if (source_rectangle != NULL && source_rectangle->max.x != 0) {
    batch->scaled_origin.x =
      origin.x * (destination_rectangle.max.x /
                  source_rectangle->max.x);
  } else {
    batch->scaled_origin.x =
      origin.x * (destination_rectangle.max.x /
                  info.width);
  }

  if (source_rectangle != NULL && source_rectangle->max.y != 0) {
    batch->scaled_origin.y =
      origin.y * (destination_rectangle.max.y /
                  source_rectangle->max.y);
  } else {
    batch->scaled_origin.y =
      origin.y * (destination_rectangle.max.y /
                  info.width);
  }

  binocle_sprite_batch_draw_internal(batch, texture, source_rectangle, color, rotation,
                                     layer_depth, true);
}

void
binocle_sprite_draw_dst_src_color(binocle_sprite_batch *batch, sg_image *texture, kmAABB2 destination_rectangle,
                                  kmAABB2 source_rectangle, sg_color color) {
  kmVec2 origin;
  binocle_sprite_batch_draw_noscale(batch, texture, destination_rectangle, &source_rectangle, color, 0.0f, origin,
                                    0.0f);
}
