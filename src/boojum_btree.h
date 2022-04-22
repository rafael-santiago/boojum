#ifndef BOOJUM_BOOJUM_BTREE_H
#define BOOJUM_BOOJUM_BTREE_H

#include <boojum_types.h>

int boojum_add_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr);

int boojum_del_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr);

int boojum_set_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, void *data, size_t *size);

void *boojum_get_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, size_t *size);


#endif
