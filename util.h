#pragma once

#include <stdio.h>
#include <stdlib.h>

static inline void* xmalloc(size_t size)
{
  void* ptr = malloc(size);
  if(ptr == NULL) {
    perror("xmalloc");
    exit(1);
  }
  return ptr;
}

static inline void* xcalloc(size_t size)
{
  void* ptr = calloc(size, 1);
  if(ptr == NULL) {
    perror("xcalloc");
    exit(1);
  }
  return ptr;
}

static inline void xfree(void* ptr)
{
  if(ptr != NULL) {
    free(ptr);
  }
}
