/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef BOOJUM_BOOJUM_SXOR_H
#define BOOJUM_BOOJUM_SXOR_H 1

#include <boojum_types.h>

#if defined(__cplusplus)
extern "C" {
#endif

int boojum_sync_sxor(boojum_alloc_leaf_ctx *aleaf, unsigned char *data, const size_t data_size);

int boojum_sync_sxor_upd(boojum_alloc_leaf_ctx *aleaf);

#if defined(__cplusplus)
}
#endif

#endif
