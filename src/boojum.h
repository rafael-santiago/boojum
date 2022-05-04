#ifndef BOOJUM_BOOJUM_H
#define BOOJUM_BOOJUM_H 1

#include <stdlib.h>

int boojum_init(const size_t kupd_timeout_in_msecs);

int boojum_deinit(void);

void *boojum_alloc(const size_t ssize);

int boojum_free(void *ptr);

void *boojum_realloc(void *ptr, const size_t ssize);

int boojum_set(void *ptr, void *data, size_t *data_size);

void *boojum_get(const void *ptr, size_t *data_size);

void *boojum_timed_get(const void *ptr, size_t *data_size, const size_t ttv);

#endif
