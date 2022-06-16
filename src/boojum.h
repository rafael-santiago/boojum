/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef BOOJUM_BOOJUM_H
#define BOOJUM_BOOJUM_H 1

#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define BOOJUM_VERSION 0x20220001 // INFO(Rafael): 16-bits for the year | 16-bits for the release number

int boojum_init(const size_t kupd_timeout_in_msecs);

int boojum_deinit(void);

void *boojum_alloc(const size_t ssize);

int boojum_free(void *ptr);

void *boojum_realloc(void *ptr, const size_t ssize);

int boojum_set(void *ptr, void *data, size_t *data_size);

void *boojum_get(const void *ptr, size_t *data_size);

void *boojum_timed_get(const void *ptr, size_t *data_size, const size_t ttv);

#if defined(__cplusplus)
}
#endif

#endif
