//
// Binocle
// Copyright (c) 2015-2019 Valerio Santinelli
// All rights reserved.
//

#include "binocle_http.h"
#include "binocle_sdl.h"
#include "binocle_log.h"
#include "curl/curl.h"
#include <stdbool.h>

#if defined(BINOCLE_HTTP)

#if defined(__EMSCRIPTEN__)
#else

static size_t
binocle_http_write_memory_cb(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  binocle_http_body_t *mem = (binocle_http_body_t *)userp;

  char *ptr = realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    binocle_log_error("OOM: not enough memory (realloc returned NULL)\n");
    return 0;
  }

  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

bool binocle_http_get(const char *url, binocle_http_body_t *body) {
  CURL *curl;
  CURLcode res;

  body->memory = malloc(1);  /* will be grown as needed by the realloc above */
  body->size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, binocle_http_write_memory_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)body);

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      binocle_log_error("Error: %s\n", curl_easy_strerror(res));
      return false;
    }

    binocle_log_debug("%lu bytes retrieved\n", (unsigned long)body->size);

    curl_easy_cleanup(curl);
  }
  return true;
}

bool binocle_http_post(const char *url, const char *post_body, binocle_http_body_t *response_body) {
  CURL *curl;
  CURLcode res;

  response_body->memory = malloc(1);  /* will be grown as needed by the realloc above */
  response_body->size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, binocle_http_write_memory_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(post_body));

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      binocle_log_error("Error: %s\n", curl_easy_strerror(res));
      return false;
    }

    binocle_log_debug("%lu bytes retrieved\n", (unsigned long)response_body->size);

    curl_easy_cleanup(curl);
  }
  return true;
}

bool binocle_http_put(const char *url, const char *put_body, binocle_http_body_t *response_body) {
  CURL *curl;
  CURLcode res;

  response_body->memory = malloc(1);  /* will be grown as needed by the realloc above */
  response_body->size = 0;    /* no data at this point */

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  if(curl) {
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, binocle_http_write_memory_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)response_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, put_body);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(put_body));
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

    res = curl_easy_perform(curl);
    if(res != CURLE_OK) {
      binocle_log_error("Error: %s\n", curl_easy_strerror(res));
      return false;
    }

    binocle_log_debug("%lu bytes retrieved\n", (unsigned long)response_body->size);

    curl_easy_cleanup(curl);
  }
  return true;
}
#endif

#endif