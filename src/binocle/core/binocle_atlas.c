//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include <string.h>
#include "binocle_atlas.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "binocle_subtexture.h"
#include "binocle_sprite.h"
#include <parson/parson.h>
#include "sokol_gfx.h"
#include "binocle_array.h"

bool binocle_atlas_load_texturepacker(char *filename, binocle_atlas_texturepacker *atlas) {
  binocle_log_info("Loading TexturePacker file: %s", filename);
  SDL_strlcpy(atlas->asset_filename, filename, BINOCLE_MAX_ATLAS_FILENAME_LENGTH);

  SDL_RWops *file = SDL_RWFromFile(filename, "rb");
  if (file == NULL) {
    binocle_log_error("Cannot open JSON file");
    return false;
  }

  Sint64 res_size = SDL_RWsize(file);
  char *res = (char *) SDL_malloc(res_size + 1);

  Sint64 nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = SDL_RWread(file, buf, 1, (res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }
  SDL_RWclose(file);
  if (nb_read_total != res_size) {
    binocle_log_error("Size mismatch");
    SDL_free(res);
    return false;
  }

  res[nb_read_total] = '\0';

  JSON_Value *root_value;
  JSON_Object *root;
  JSON_Array *frames;
  JSON_Object *frame;
  JSON_Object *meta;
  JSON_Array *frame_tags;
  JSON_Object *frame_tag;
  JSON_Array *slices_array;
  JSON_Object *slice_object;

  root_value = json_parse_string(res);

  if (root_value == NULL) {
    binocle_log_error("Error parsing JSON, cannot parse the string");
    SDL_free(res);
    return false;
  }

  if (json_value_get_type(root_value) != JSONObject) {
    binocle_log_error("Error parsing JSON, root object isn't an Object");
    json_value_free(root_value);
    SDL_free(res);
    return false;
  }

  root = json_value_get_object(root_value);
  if (root == NULL) {
    binocle_log_error("Error parsing JSON, cannot read root object");
    json_value_free(root_value);
    SDL_free(res);
    return false;
  }

  meta = json_object_get_object(root, "meta");
  int atlas_w = (int) json_object_dotget_number(meta, "size.w");
  int atlas_h = (int) json_object_dotget_number(meta, "size.h");
  const char *app = json_object_get_string(meta, "app");
  const char *version = json_object_get_string(meta, "version");
  const char *image = json_object_get_string(meta, "image");
  const char *format = json_object_get_string(meta, "format");
  const char *scale = json_object_get_string(meta, "scale");
  const char *smartupdate = json_object_get_string(meta, "smartupdate");

  atlas->meta = (binocle_atlas_tp_meta){
    .size = (kmVec2){
      .x = atlas_w,
      .y = atlas_h,
    }
  };

  if (app != NULL) {
    atlas->meta.app = SDL_malloc(SDL_strlen(app) + 1);
    strcpy(atlas->meta.app, app);
  }

  if (version != NULL) {
    atlas->meta.version = SDL_malloc(SDL_strlen(version)+1);
    strcpy(atlas->meta.version, version);
  }

  if (image != NULL) {
    atlas->meta.image = SDL_malloc(SDL_strlen(image) + 1);
    strcpy(atlas->meta.image, image);
  }

  if (format != NULL) {
    atlas->meta.format = SDL_malloc(SDL_strlen(format) + 1);
    strcpy(atlas->meta.format, format);
  }

  if (scale != NULL) {
    atlas->meta.scale = SDL_malloc(SDL_strlen(scale) + 1);
    strcpy(atlas->meta.scale, scale);
  }

  if (smartupdate != NULL) {
    atlas->meta.smartupdate = SDL_malloc(SDL_strlen(smartupdate) + 1);
    strcpy(atlas->meta.smartupdate, smartupdate);
  }

  frame_tags = json_object_get_array(meta, "frameTags");
  if (frame_tags != NULL) {
    atlas->meta.num_frame_tags = json_array_get_count(frame_tags);
    binocle_atlas_animation *anims = SDL_malloc(sizeof(binocle_atlas_animation) * atlas->meta.num_frame_tags);
    for (int i = 0; i < json_array_get_count(frame_tags); i++) {
      frame_tag = json_array_get_object(frame_tags, i);

      int from, to = 0;
      const char *name;
      const char *direction;
      const char *repeat;

      name = json_object_get_string(frame_tag, "name");
      direction = json_object_get_string(frame_tag, "direction");
      from = (int) json_object_get_number(frame_tag, "from");
      to = (int) json_object_get_number(frame_tag, "to");
      repeat = json_object_get_string(frame_tag, "repeat");

      binocle_atlas_animation *anim = &anims[i];
      anim->name = SDL_malloc(SDL_strlen(name) + 1);
      strcpy(anim->name, name);
      anim->direction = SDL_malloc(SDL_strlen(direction) + 1);
      strcpy(anim->direction, direction);
      anim->from = from;
      anim->to = to;
      anim->repeat = repeat != NULL;
    }
    atlas->meta.frame_tags = anims;
  }

  slices_array = json_object_get_array(meta, "slices");
  if (slices_array != NULL) {
    atlas->meta.num_slices = json_array_get_count(slices_array);
    binocle_atlas_slice *slices = SDL_malloc(sizeof(binocle_atlas_slice) * atlas->meta.num_slices);
    for (int i = 0; i < json_array_get_count(slices_array); i++) {
      slice_object = json_array_get_object(slices_array, i);

      const char *name;
      const char *color;

      name = json_object_get_string(slice_object, "name");
      color = json_object_get_string(slice_object, "color");

      binocle_atlas_slice *slice = &slices[i];
      slice->name = SDL_malloc(SDL_strlen(name) + 1);
      strcpy(slice->name, name);
      slice->color = SDL_malloc(SDL_strlen(color) + 1);
      strcpy(slice->color, color);

      JSON_Array *slices_keys_array = json_object_get_array(slice_object, "keys");
      if (slices_keys_array != NULL) {
        slice->num_keys = json_array_get_count(slices_keys_array);
        binocle_atlas_slice_key *slice_keys = SDL_malloc(sizeof(binocle_atlas_slice_key) * slice->num_keys);
        for (int j = 0; j < json_array_get_count(slices_keys_array); j++) {
          JSON_Object *slice_key_object = json_array_get_object(slices_keys_array, j);

          int f = 0;
          int bx, by, bw, bh = 0;
          int px, py = 0;

          f = json_object_get_number(slice_key_object, "frame");
          bx = json_object_dotget_number(slice_key_object, "bounds.x");
          by = json_object_dotget_number(slice_key_object, "bounds.y");
          bw = json_object_dotget_number(slice_key_object, "bounds.w");
          bh = json_object_dotget_number(slice_key_object, "bounds.h");
          px = json_object_dotget_number(slice_key_object, "pivot.x");
          py = json_object_dotget_number(slice_key_object, "pivot.y");

          binocle_atlas_slice_key *slice_key = &slice_keys[j];
          slice_key->frame = f;
          slice_key->bounds = (kmAABB2){
            .min = {
              .x = bx,
              .y = by,
            },
            .max = {
              .x = bw,
              .y = bh,
            }
          };
          slice_key->pivot = (kmVec2){
            .x = px,
            .y = py,
          };
        }
        slices->keys = slice_keys;
      }

    }
    atlas->meta.slices = slices;
  }

  frames = json_object_get_array(root, "frames");
  atlas->num_frames = json_array_get_count(frames);
  binocle_atlas_tp_frame *atlas_frames = SDL_malloc(sizeof(binocle_atlas_tp_frame) * atlas->num_frames);
  for (int i = 0; i < atlas->num_frames; i++) {
    frame = json_array_get_object(frames, i);
    int x, y, w, h = 0;
    float pivot_x, pivot_y = 0;
    const char *frameFilename;
    JSON_Object *innerFrame;
    int duration = 0;

    frameFilename = json_object_get_string(frame, "filename");
    duration = (int)json_object_get_number(frame, "duration");
    innerFrame = json_object_get_object(frame, "frame");
    x = (int) json_object_get_number(innerFrame, "x");
    y = (int) json_object_get_number(innerFrame, "y");
    w = (int) json_object_get_number(innerFrame, "w");
    h = (int) json_object_get_number(innerFrame, "h");
    pivot_x = (float) json_object_dotget_number(frame, "pivot.x");
    pivot_y = (float) json_object_dotget_number(frame, "pivot.y");

    binocle_atlas_tp_frame *atlas_frame = &atlas_frames[i];
    atlas_frame->filename = SDL_malloc(SDL_strlen(frameFilename) + 1);
    strcpy(atlas_frame->filename, frameFilename);

    atlas_frame->frame = (kmAABB2){
      .min = {
        .x = x,
        .y = y,
      },
      .max = {
        .x = w,
        .y = h,
      }
    };

    atlas_frame->pivot = (kmVec2){
      .x = pivot_x,
      .y = pivot_y,
    };

    atlas_frame->duration = duration;

  }
  atlas->frames = atlas_frames;

  // Clean up after ourselves
  json_value_free(root_value);
  SDL_free(res);

  binocle_log_debug("Atlas loaded.");
  return true;
}

void binocle_atlas_destroy_texturepacker(binocle_atlas_texturepacker *atlas) {
  if (atlas->meta.app != NULL) {
    SDL_free(atlas->meta.app);
  }
  if (atlas->meta.version != NULL) {
    SDL_free(atlas->meta.version);
  }
  if (atlas->meta.image != NULL) {
    SDL_free(atlas->meta.image);
  }
  if (atlas->meta.format != NULL) {
    SDL_free(atlas->meta.format);
  }
  if (atlas->meta.scale != NULL) {
    SDL_free(atlas->meta.scale);
  }
  if (atlas->meta.smartupdate != NULL) {
    SDL_free(atlas->meta.smartupdate);
  }
  for (int i = 0 ; i < atlas->meta.num_frame_tags ; i++) {
    if (atlas->meta.frame_tags[i].name != NULL) {
      SDL_free(atlas->meta.frame_tags[i].name);
    }
    if (atlas->meta.frame_tags[i].direction != NULL)
    {
      SDL_free(atlas->meta.frame_tags[i].direction);
    }
  }
  SDL_free(atlas->meta.frame_tags);
  for (int i = 0 ; i < atlas->num_frames ; i++) {
    if (atlas->frames[i].filename != NULL) {
      SDL_free(atlas->frames[i].filename);
    }
  }
}

void binocle_atlas_texturepacker_create_subtextures(binocle_atlas_texturepacker *atlas, struct sg_image *texture,
                                        struct binocle_subtexture *subtextures, int *num_subtextures) {
  *num_subtextures = 0;

  for (int i = 0; i < atlas->num_frames; i++) {
    binocle_atlas_tp_frame *frame = &atlas->frames[i];

    // We need to invert the coordinates as our reference is with 0,0 in the bottom left corner, thus
    // the y becomes (atlas height - y - height of the frame)
    subtextures[*num_subtextures] = binocle_subtexture_with_texture(texture, frame->frame.min.x, atlas->meta.size.y - frame->frame.min.y - frame->frame.max.y, frame->frame.max.x, frame->frame.max.y);
    strcpy(subtextures[*num_subtextures].name, frame->filename);
    (*num_subtextures)++;
  }
}

void binocle_atlas_texturepacker_create_animations(
  binocle_atlas_texturepacker *atlas,
  sg_image texture,
  binocle_sprite *sprite
) {
  binocle_log_info("Creating %d frames of animation from atlas %s", atlas->num_frames, atlas->asset_filename);
  // Put all the frames in the sprite
  for (int i = 0 ; i < atlas->num_frames ; i++) {
    binocle_atlas_tp_frame *atlas_frame = &atlas->frames[i];
    binocle_subtexture st = binocle_subtexture_with_texture(&texture, atlas_frame->frame.min.x, atlas->meta.size.y - atlas_frame->frame.min.y - atlas_frame->frame.max.y, atlas_frame->frame.max.x, atlas_frame->frame.max.y);
    binocle_array_push(sprite->subtextures, st);
    binocle_sprite_frame frame = binocle_sprite_frame_from_subtexture(&st);
    binocle_sprite_add_frame(sprite, frame);
  }
  // Create all the animations
  for (int i = 0 ; i < atlas->meta.num_frame_tags ; i++) {
    binocle_log_info("Creating animation number %i", i);
    binocle_atlas_animation *atlas_animation = &atlas->meta.frame_tags[i];
    binocle_sprite_animation *anim = &sprite->animations[i];
    anim->looping = atlas_animation->repeat;
    for (int j = 0; j < BINOCLE_SPRITE_MAX_FRAMES; j++) {
      anim->frames[j] = -1;
    }
    anim->name = SDL_strdup(atlas_animation->name);
    int frames_count = atlas_animation->to - atlas_animation->from + 1;
    int animation_frame = 0;
    anim->frames_number = frames_count;
    for (int n = atlas_animation->from ; n <= atlas_animation->to ; n++) {
      binocle_log_info("Adding frame %d at position %d to %s", n, animation_frame, anim->name);
      binocle_atlas_tp_frame *atlas_frame = &atlas->frames[n];
      anim->delays[animation_frame] = (float)atlas_frame->duration / 1000.0f;
      anim->frames[animation_frame] = n;
      animation_frame++;
    }
    sprite->animations_number++;
    binocle_log_info("Created animation %s", anim->name);
  }
  binocle_log_info("Created %d animations", sprite->animations_number);
}

void binocle_atlas_animation_destroy(binocle_atlas_animation *animations, size_t num_animations) {
  for (int i = 0 ; i < num_animations ; i++) {
    if (animations[i].name != NULL) {
      SDL_free(animations[i].name);
    }
    if (animations[i].direction != NULL) {
      SDL_free(animations[i].direction);
    }
  }
  SDL_free(animations);
}

void binocle_atlas_load_libgdx(char *filename, struct sg_image *texture, struct binocle_subtexture *subtextures,
                               int *num_subtextures) {
  *num_subtextures = 0;
  binocle_log_info("Loading LibGDX file: %s", filename);
  SDL_RWops *file = SDL_RWFromFile(filename, "rb");
  if (file == NULL) {
    binocle_log_error("Cannot open text file");
    return;
  }

  Sint64 res_size = SDL_RWsize(file);
  char *res = (char *) SDL_malloc(res_size + 1);

  Sint64 nb_read_total = 0, nb_read = 1;
  char *buf = res;
  while (nb_read_total < res_size && nb_read != 0) {
    nb_read = SDL_RWread(file, buf, 1, (res_size - nb_read_total));
    nb_read_total += nb_read;
    buf += nb_read;
  }
  SDL_RWclose(file);
  if (nb_read_total != res_size) {
    binocle_log_error("Size mismatch");
    SDL_free(res);
    return;
  }

  res[nb_read_total] = '\0';

  // TODO: perform all the parsing as needed


  // Clean up after ourselves
  SDL_free(res);

}
