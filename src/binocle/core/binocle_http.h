//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#ifndef BINOCLE_BINOCLE_HTTP_H
#define BINOCLE_BINOCLE_HTTP_H

#if defined(BINOCLE_HTTP)

#include <stdbool.h>
#include <stddef.h>

typedef struct binocle_http_body_t {
  char *memory;
  size_t size;
} binocle_http_body_t;

bool binocle_http_get(const char *url, binocle_http_body_t *body);
bool binocle_http_post(const char *url, const char *post_body, binocle_http_body_t *response_body);
bool binocle_http_put(const char *url, const char *put_body, binocle_http_body_t *response_body);

#if defined(__EMSCRIPTEN__)
#else


#endif

#endif

#endif // BINOCLE_BINOCLE_HTTP_H
