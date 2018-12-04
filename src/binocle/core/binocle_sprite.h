//
// Created by Valerio Santinelli on 19/04/18.
//

#ifndef BINOCLE_BINOCLE_SPRITE_H
#define BINOCLE_BINOCLE_SPRITE_H

#include <stdbool.h>
#include <stdlib.h>
#include <kazmath/kazmath.h>
#include "binocle_subtexture.h"

#define BINOCLE_SPRITE_MAX_FRAMES 256
#define BINOCLE_SPRITE_MAX_ANIMATIONS 16

struct binocle_material;
struct binocle_gd;

typedef struct binocle_sprite_frame {
  binocle_subtexture *subtexture;
  kmVec2 origin;
} binocle_sprite_frame;

typedef struct binocle_sprite_animation_frame_mapping {
  char *name;
  int original_frame;
  int real_frame;
} binocle_sprite_animation_frame_mapping;

typedef struct binocle_sprite_animation {
  bool enabled;
  int frames[BINOCLE_SPRITE_MAX_FRAMES];
  float delays[BINOCLE_SPRITE_MAX_FRAMES];
  bool looping;
  int frames_number;
  char *name;
  binocle_sprite_animation_frame_mapping frame_mapping[BINOCLE_SPRITE_MAX_FRAMES];
  int frame_mapping_number;
} binocle_sprite_animation;

typedef struct binocle_sprite {
  binocle_subtexture subtexture;
  struct binocle_material *material;
  kmVec2 origin;
  binocle_sprite_animation animations[BINOCLE_SPRITE_MAX_ANIMATIONS];
  binocle_sprite_frame frames[BINOCLE_SPRITE_MAX_FRAMES];
  int frames_number;
  bool playing;
  bool finished;
  float rate;
  int current_frame;
  binocle_sprite_animation *current_animation;
  int animations_number;
  int current_animation_id;
  int current_animation_frame;
  float timer;
} binocle_sprite;

binocle_sprite binocle_sprite_from_material(struct binocle_material *material);
void binocle_sprite_draw(binocle_sprite sprite, struct binocle_gd *gd, int64_t x, int64_t y, kmAABB2 viewport, float rotation, kmVec2 scale);
void binocle_sprite_add_frame(binocle_sprite *sprite, binocle_sprite_frame frame);
void binocle_sprite_add_animation(binocle_sprite *sprite, int id, int frame);
void binocle_sprite_add_animation_with_frames(binocle_sprite *sprite, int id, bool looping, float delay, int frames[], int frames_count);
void binocle_sprite_play(binocle_sprite *sprite, int id, bool restart);
void binocle_sprite_play_from_frame(binocle_sprite *sprite, int id, int start_frame, bool restart);
void binocle_sprite_stop(binocle_sprite *sprite);
void binocle_sprite_update(binocle_sprite *sprite, float dt);
int binocle_sprite_get_current_frame(binocle_sprite *sprite);
void binocle_sprite_set_current_frame(binocle_sprite *sprite, int frame);
int binocle_sprite_get_current_animation(binocle_sprite *sprite);
void binocle_sprite_set_current_animation(binocle_sprite *sprite, int id);
void binocle_sprite_clear_animations(binocle_sprite *sprite);
void binocle_sprite_clear_frames(binocle_sprite *sprite);

// sequence code format:
// startFrame-endFrame:time(chance)
// time: also be set to "forever" - this will loop the sequence indefinitely
// chance: float value from 0-1, chance that the sequence will play (if not played, it will be skipped)
// time and chance can both be ignored, this will mean the sequence plays through once
//
// sequence code examples:
// TV: 0-1:3, 2-3:3, 4-5:4, 6-7:4, 8:3, 9:3
// Idle animation with random fidgets: 0-59, 60-69, 10-59, 0-59(.25), 70-129(.75)
// Jump animation with looping finish: 0-33, 20-33:forever
void binocle_sprite_create_animation(binocle_sprite *sprite, char *name, char *subtextures_names, char *sequence_code, bool loop, binocle_subtexture *subtextures, size_t subtextures_count);
void binocle_sprite_play_animation(binocle_sprite *sprite, char *name, bool restart);

// binocle_sprite_frame stuff
binocle_sprite_frame binocle_sprite_frame_from_subtexture(struct binocle_subtexture *subtexture);
binocle_sprite_frame binocle_sprite_frame_from_subtexture_and_origin(struct binocle_subtexture *subtexture, kmVec2 origin);



#endif //BINOCLE_BINOCLE_SPRITE_H
