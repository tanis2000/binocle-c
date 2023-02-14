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
#include <parson/parson.h>
#include "sokol_gfx.h"

void binocle_atlas_load_texturepacker(char *filename, struct sg_image *texture,
                                      struct binocle_subtexture *subtextures, int *num_subtextures,
                                        bool include_animations,
                                        binocle_atlas_animation **animations, size_t *num_animations) {
  *num_subtextures = 0;
  binocle_log_info("Loading TexturePacker file: %s", filename);
  SDL_RWops *file = SDL_RWFromFile(filename, "rb");
  if (file == NULL) {
    binocle_log_error("Cannot open JSON file");
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

  JSON_Value *root_value;
  JSON_Object *root;
  JSON_Array *frames;
  JSON_Object *frame;
  JSON_Object *meta;
  JSON_Array *frame_tags;
  JSON_Object *frame_tag;

  root_value = json_parse_string(res);

  if (root_value == NULL) {
    binocle_log_error("Error parsing JSON, cannot parse the string");
    SDL_free(res);
    return;
  }

  if (json_value_get_type(root_value) != JSONObject) {
    binocle_log_error("Error parsing JSON, root object isn't an Object");
    json_value_free(root_value);
    SDL_free(res);
    return;
  }

  root = json_value_get_object(root_value);
  if (root == NULL) {
    binocle_log_error("Error parsing JSON, cannot read root object");
    json_value_free(root_value);
    SDL_free(res);
    return;
  }

  meta = json_object_get_object(root, "meta");
  int atlas_w = (int) json_object_dotget_number(meta, "size.w");
  int atlas_h = (int) json_object_dotget_number(meta, "size.h");

  frames = json_object_get_array(root, "frames");
  for (int i = 0; i < json_array_get_count(frames); i++) {
    frame = json_array_get_object(frames, i);
    int x, y, w, h = 0;
    const char *frameFilename;
    JSON_Object *innerFrame;

    frameFilename = json_object_get_string(frame, "filename");
    innerFrame = json_object_get_object(frame, "frame");
    x = (int) json_object_get_number(innerFrame, "x");
    y = (int) json_object_get_number(innerFrame, "y");
    w = (int) json_object_get_number(innerFrame, "w");
    h = (int) json_object_get_number(innerFrame, "h");

    // We need to invert the coordinates as our reference is with 0,0 in the bottom left corner, thus
    // the y becomes (atlas height - y - height of the frame)
    subtextures[*num_subtextures] = binocle_subtexture_with_texture(texture, x, atlas_h - y - h, w, h);
    strcpy(subtextures[*num_subtextures].name, frameFilename);
    (*num_subtextures)++;
  }

  if (include_animations != NULL) {
    frame_tags = json_object_get_array(meta, "frameTags");
    if (frame_tags != NULL) {
      *num_animations = json_array_get_count(frame_tags);
      binocle_atlas_animation *anims = SDL_malloc(sizeof(binocle_atlas_animation) * *num_animations);
      for (int i = 0; i < json_array_get_count(frame_tags); i++) {
        frame_tag = json_array_get_object(frame_tags, i);

        int from, to = 0;
        const char *name;
        const char *direction;

        name = json_object_get_string(frame_tag, "name");
        direction = json_object_get_string(frame_tag, "direction");
        from = (int) json_object_get_number(frame_tag, "from");
        to = (int) json_object_get_number(frame_tag, "to");

        binocle_atlas_animation *anim = &anims[i];
        anim->name = SDL_malloc(SDL_strlen(name) + 1);
        strcpy(anim->name, name);
        anim->direction = SDL_malloc(SDL_strlen(direction) + 1);
        strcpy(anim->direction, direction);
        anim->from = from;
        anim->to = to;
      }
      *animations = anims;
    }
  }


  // Clean up after ourselves
  json_value_free(root_value);
  SDL_free(res);

  binocle_log_debug("Atlas loaded.");
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
