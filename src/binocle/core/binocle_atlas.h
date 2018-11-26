//
// Created by Valerio Santinelli on 26/11/2018.
//

#ifndef HELIRUN_BINOCLE_ATLAS_H
#define HELIRUN_BINOCLE_ATLAS_H

#define BINOCLE_ATLAS_MAX_SUBTEXTURES 256
#define BINOCLE_ATLAS_MAX_FILESIZE 65535

struct binocle_subtexture;
struct binocle_texture;

typedef enum binocle_atlas_file_format {
  BINOCLE_ATLAS_FILE_FORMAT_TEXTUREPACKER,
  BINOCLE_ATLAS_FILE_FORMAT_LIBGDX
} binocle_atlas_file_format;

void binocle_atlas_load_libgdx(struct binocle_subtexture *subtextures, char *filename);
void binocle_atlas_load_texturepacker(char *filename, struct binocle_texture *texture, struct binocle_subtexture *subtextures, int *num_subtextures);

#endif //HELIRUN_BINOCLE_ATLAS_H
