/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef BOOJUM_BOOJUM_BTREE_H
#define BOOJUM_BOOJUM_BTREE_H 1

#include <boojum_types.h>

#if defined(__cplusplus)
extern "C" {
#endif

// WARN(Rafael): All functions implemented in this module infers that alloc_tree is under
//               a well-synchronized status.

int boojum_add_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, const size_t ssize);

int boojum_del_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr);

int boojum_set_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, void *data, size_t *size);

void *boojum_get_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, size_t *size);

int boojum_update_xor_maskings(boojum_alloc_branch_ctx **alloc_tree);

#if defined(__cplusplus)
}
#endif

#endif
