//
// Created by Valerio Santinelli on 19/04/18.
//

#include <string.h>
#include "binocle_sprite.h"
#include "binocle_texture.h"
#include "binocle_subtexture.h"
#include "binocle_material.h"
#include "binocle_gd.h"
#include "binocle_vpct.h"
#include "binocle_log.h"

#define BINOCLE_SPRITE_VERTEX_COUNT 6

binocle_sprite binocle_sprite_from_material(binocle_material *material) {
  binocle_sprite res = {0};
  // Default origin to bottom-left
  res.origin.x = 0;
  res.origin.y = 0;
  res.material = material;
  // Default to use the whole texture
  res.subtexture = binocle_subtexture_with_texture(material->texture, 0, 0, material->texture->width, material->texture->height);
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_ANIMATIONS ; i++) {
    res.animations[i].enabled = false;
  }
  res.frames_number = 0;
  res.playing = false;
  res.finished = false;
  res.rate = 1;
  res.timer = 0;
  res.current_frame = 0;
  res.current_animation = NULL;
  res.current_animation_frame = 0;
  res.current_animation_id = 0;
  return res;
}

void binocle_sprite_draw(binocle_sprite sprite, binocle_gd *gd, int64_t x, int64_t y, kmAABB2 viewport, float rotation, kmVec2 scale) {
  binocle_vpct vertices[BINOCLE_SPRITE_VERTEX_COUNT];
  binocle_subtexture *s;
  float w, h;
  if (sprite.frames_number > 0) {
    s = sprite.frames[sprite.current_frame].subtexture;
    w = s->rect.max.x;
    h = s->rect.max.y;
    //binocle_log_info("subtexture %f %f %f %f %d %d", s.rect.min.x, s.rect.min.y, s.rect.max.x, s.rect.max.y, sprite.material->texture->width, sprite.material->texture->height);
  } else {
    s = &sprite.subtexture;
    w = sprite.subtexture.rect.max.x;
    h = sprite.subtexture.rect.max.y;
    //w = sprite.material->texture->width;
    //h = sprite.material->texture->height;
  }

  // TL
  vertices[0].pos.x = -sprite.origin.x * scale.x * cosf(rotation) - (-sprite.origin.y * scale.y + h * scale.y) * sinf(rotation) + x;
  vertices[0].pos.y = (-sprite.origin.y * scale.y + h * scale.y) * cosf(rotation) - sprite.origin.x * scale.x * sinf(rotation) + y;
  vertices[0].color = binocle_color_white();
  vertices[0].tex.x = s->rect.min.x / sprite.material->texture->width;
  vertices[0].tex.y = (s->rect.min.y + s->rect.max.y) / sprite.material->texture->height;
  // TR
  vertices[1].pos.x = (-sprite.origin.x * scale.x + w * scale.x) * cosf(rotation) - (-sprite.origin.y * scale.y + h * scale.y) * sinf(rotation) + x;
  vertices[1].pos.y = (-sprite.origin.y * scale.y + h * scale.y) * cosf(rotation) + (-sprite.origin.x * scale.x + w * scale.x) * sinf(rotation) + y;
  vertices[1].color = binocle_color_white();
  vertices[1].tex.x = (s->rect.min.x + s->rect.max.x) / sprite.material->texture->width;
  vertices[1].tex.y = (s->rect.min.y + s->rect.max.y) / sprite.material->texture->height;
  // BL
  vertices[2].pos.x = -sprite.origin.x * scale.x * cosf(rotation) + sprite.origin.y * scale.y * sinf(rotation) + x;
  vertices[2].pos.y = -sprite.origin.y * scale.y * cosf(rotation) - sprite.origin.x * scale.x * sinf(rotation) + y;
  vertices[2].color = binocle_color_white();
  vertices[2].tex.x = s->rect.min.x / sprite.material->texture->width;
  vertices[2].tex.y = s->rect.min.y / sprite.material->texture->height;
  // TR
  vertices[3].pos.x = (-sprite.origin.x * scale.x + w * scale.x) * cosf(rotation) - (-sprite.origin.y * scale.y + h * scale.y) * sinf(rotation) + x;
  vertices[3].pos.y = (-sprite.origin.y * scale.y + h * scale.y) * cosf(rotation) + (-sprite.origin.x * scale.x + w * scale.x) * sinf(rotation) + y;
  vertices[3].color = binocle_color_white();
  vertices[3].tex.x = (s->rect.min.x + s->rect.max.x) / sprite.material->texture->width;
  vertices[3].tex.y = (s->rect.min.y + s->rect.max.y) / sprite.material->texture->height;
  // BR
  vertices[4].pos.x = (-sprite.origin.x * scale.x + w * scale.x) * cosf(rotation) + sprite.origin.y * scale.y * sinf(rotation) + x;
  vertices[4].pos.y = -sprite.origin.y * scale.y * cosf(rotation) + (-sprite.origin.x * scale.x + w * scale.x) * sinf(rotation) + y;
  vertices[4].color = binocle_color_white();
  vertices[4].tex.x = (s->rect.min.x + s->rect.max.x) / sprite.material->texture->width;
  vertices[4].tex.y = s->rect.min.y / sprite.material->texture->height;
  // BL
  vertices[5].pos.x = -sprite.origin.x * scale.x * cosf(rotation) + sprite.origin.y * scale.y * sinf(rotation) + x;
  vertices[5].pos.y = -sprite.origin.y * scale.y * cosf(rotation) - sprite.origin.x * scale.x * sinf(rotation) + y;
  vertices[5].color = binocle_color_white();
  vertices[5].tex.x = s->rect.min.x / sprite.material->texture->width;
  vertices[5].tex.y = s->rect.min.y / sprite.material->texture->height;

  binocle_gd_draw(gd, vertices, BINOCLE_SPRITE_VERTEX_COUNT, *sprite.material, viewport);
}

void binocle_sprite_add_frame(binocle_sprite *sprite, binocle_sprite_frame frame) {
  sprite->frames[sprite->frames_number] = frame;
  sprite->frames_number++;
}


binocle_sprite_frame binocle_sprite_frame_from_subtexture(struct binocle_subtexture *subtexture) {
  binocle_sprite_frame res = {0};
  res.subtexture = subtexture;
  res.origin = binocle_subtexture_get_center(subtexture);
  return res;
}

binocle_sprite_frame binocle_sprite_frame_from_subtexture_and_origin(struct binocle_subtexture *subtexture, kmVec2 origin) {
  binocle_sprite_frame res = {0};
  res.subtexture = subtexture;
  res.origin = origin;
  return res;
}

binocle_sprite_animation *binocle_sprite_create_animation_with_name(binocle_sprite *sprite, char *name) {
  binocle_sprite_animation res = {0};
  res.name = name;
  res.enabled = true;
  res.delay = 0;
  res.looping = false;
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_FRAMES ; i++) {
    res.frames[i] = -1;
  }
  res.frames_number = 0;
  sprite->animations[sprite->animations_number] = res;
  sprite->animations_number++;
  return &sprite->animations[sprite->animations_number-1];
}


void binocle_sprite_add_animation(binocle_sprite *sprite, int id, int frame) {
  binocle_sprite_animation res = {0};
  res.enabled = true;
  res.delay = 0;
  res.looping = false;
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_FRAMES ; i++) {
    res.frames[i] = -1;
  }
  res.frames[0] = frame;
  res.frames_number = 1;
  sprite->animations[id] = res;
}

void binocle_sprite_add_animation_with_frames(binocle_sprite *sprite, int id, bool looping, float delay, int frames[], int frames_count) {
  binocle_sprite_animation res = {0};
  res.enabled = true;
  res.delay = delay;
  res.looping = looping;
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_FRAMES ; i++) {
    res.frames[i] = -1;
  }
  for (int i = 0 ; i < frames_count ; i++) {
    res.frames[i] = frames[i];
  }
  res.frames_number = frames_count;
  sprite->animations[id] = res;
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
    if (sprite->playing && sprite->current_animation->delay > 0) {
      sprite->timer += (dt * fabsf(sprite->rate));
      //binocle_log_info("timer %f fabsf(sprite->rate) %f", sprite->timer, fabsf(sprite->rate));
      while (sprite->timer >= sprite->current_animation->delay) {
        //binocle_log_info("sprite->timer %f sprite->current_animation->delay %f", sprite->timer, sprite->current_animation->delay);
        int oldFrame = sprite->current_frame;
        sprite->timer -= sprite->current_animation->delay;
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
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_ANIMATIONS ; i++) {
    sprite->animations[i].enabled = false;
  }
}

void binocle_sprite_clear_frames(binocle_sprite *sprite) {
  if (sprite->playing) {
    binocle_sprite_stop(sprite);
  }
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_FRAMES ; i++) {
    // TODO
  }
}

void binocle_sprite_create_frames_with_string(binocle_sprite *sprite, binocle_sprite_animation *animation, char *subtextures_names, binocle_subtexture *subtextures, size_t subtextures_count) {
  char *s = SDL_strdup(subtextures_names);
  char *token = strtok(s, ",");
  while (token) {
    bool added = false;
    binocle_subtexture *sub = subtextures;
    for (int i = 0 ; i < subtextures_count ; i++) {
      if (strcmp(sub->name, token) == 0) {
        binocle_sprite_frame frame = binocle_sprite_frame_from_subtexture(sub);
        binocle_sprite_add_frame(sprite, frame);
        animation->looping = true; // TODO: remove this
        animation->delay = 1; // TODO: remove this
        animation->frames[animation->frames_number] = sprite->frames_number-1;
        animation->frames_number++;
        added = true;
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

void binocle_sprite_create_animation(binocle_sprite *sprite, char *name, char *subtextures_names, char *sequence_code, binocle_subtexture *subtextures, size_t subtextures_count) {
  // create the animation with name `name`
  binocle_sprite_animation *anim = binocle_sprite_create_animation_with_name(sprite, name);
  binocle_log_info("Created animation with name %s", name);
  // assign the subtextures to the frames of the sprite
  binocle_sprite_create_frames_with_string(sprite, anim, subtextures_names, subtextures, subtextures_count);
  // build the animation using the sequence code
}

int binocle_sprite_get_animation_id(binocle_sprite *sprite, char *name) {
  binocle_sprite_animation *anim = sprite->animations;
  for (int i = 0 ; i < BINOCLE_SPRITE_MAX_ANIMATIONS ; i++) {
    if (strcmp(anim->name, name) == 0) {
      return i;
    }
    anim++;
  }
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