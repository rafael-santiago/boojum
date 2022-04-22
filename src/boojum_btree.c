#include <boojum_btree.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

#define BOOJUM_BITSIZE (sizeof(uintptr_t) << 3)

#define boojum_get_bitn(b, n)  ( ( ((b) >> (BOOJUM_BITSIZE - (n + 1))) & 0x1 ) )

static int new_alloc_branch(boojum_alloc_branch_ctx **n);

static int boojum_add_addr_iter(boojum_alloc_branch_ctx **node, const uintptr_t segment_addr, const size_t bn);

static int boojum_del_addr_iter(boojum_alloc_branch_ctx **node, const uintptr_t segment_addr, const size_t bn);

int boojum_add_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr) {
    int err = EXIT_FAILURE;

    if (alloc_tree == NULL) {
        return EINVAL;
    }

    if (*alloc_tree == NULL) {
        if ((err = new_alloc_branch(alloc_tree)) != EXIT_SUCCESS) {
            return err;
        }
    }

    (*alloc_tree)->refcount++;

    if (boojum_get_bitn(segment_addr, 0)) {
        err = boojum_add_addr_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->r, segment_addr, 0);
    } else {
        err = boojum_add_addr_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->l, segment_addr, 0);
    }

    return err;
}

int boojum_del_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr) {
    int err = EXIT_FAILURE;

    if (alloc_tree == NULL || *alloc_tree == NULL) {
        return EINVAL;
    }

    if ((*alloc_tree)->refcount == 0) {
        // INFO(Rafael): It should never happen in normal conditions.
        return EINVAL;
    }

    // TODO(Rafael): Ensure that the value exists in the tree before going ahead by removing it.

    (*alloc_tree)->refcount--;

    if (boojum_get_bitn(segment_addr, 0)) {
        err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->r, segment_addr, 0);
        if (((boojum_alloc_branch_ctx *)(*alloc_tree)->r)->refcount == 0) {
            free((*alloc_tree)->r);
            (*alloc_tree)->r = NULL;
        }
    } else {
        err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->l, segment_addr, 0);
        if (((boojum_alloc_branch_ctx *)(*alloc_tree)->l)->refcount == 0) {
            free((*alloc_tree)->l);
            (*alloc_tree)->l = NULL;
        }
    }

    if ((*alloc_tree)->refcount == 0) {
        free((*alloc_tree));
        (*alloc_tree) = NULL;
    }

    return err;
}

static int boojum_add_addr_iter(boojum_alloc_branch_ctx **node, const uintptr_t segment_addr, const size_t bn) {
    int err = EXIT_SUCCESS;

    if (*node == NULL) {
        if ((err = new_alloc_branch(node)) != EXIT_SUCCESS) {
            return err;
        }
    }

    (*node)->refcount++;

    if (bn < BOOJUM_BITSIZE) {
        if (boojum_get_bitn(segment_addr, bn + 1)) {
            err = boojum_add_addr_iter((boojum_alloc_branch_ctx **)&(*node)->r, segment_addr, bn + 1);
        } else {
            err = boojum_add_addr_iter((boojum_alloc_branch_ctx **)&(*node)->l, segment_addr, bn + 1);
        }

        if (err != EXIT_SUCCESS) {
            return err;
        }
    }

    return err;
}

static int boojum_del_addr_iter(boojum_alloc_branch_ctx **node, const uintptr_t segment_addr, const size_t bn) {
    int err = EXIT_SUCCESS;
    boojum_alloc_leaf_ctx *lp = NULL;

    if (*node == NULL) {
        // INFO(Rafael): It should never happen in normal conditions.
        return EINVAL;
    }

    (*node)->refcount--;

    if (bn < BOOJUM_BITSIZE) {
        if (bn == BOOJUM_BITSIZE - 1 && (*node)->d != NULL) {
            lp = (boojum_alloc_leaf_ctx *)(*node)->d;
            if (lp->m != NULL) {
                memset((unsigned char *)lp->m, 0, lp->m_size);
                memset((unsigned char *)lp->r, 0, lp->m_size);
                free(lp->m);
                free(lp->r);
                lp->m_size = 0;
            }
            free((*node)->d);
            (*node)->d = NULL;
        }

        if (boojum_get_bitn(segment_addr, bn + 1)) {
            err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*node)->r, segment_addr, bn + 1);
            if (((boojum_alloc_branch_ctx *)(*node)->r)->refcount == 0) {
                free((*node)->r);
                (*node)->r = NULL;
            }
        } else {
            err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*node)->l, segment_addr, bn + 1);
            if (((boojum_alloc_branch_ctx *)(*node)->l)->refcount == 0) {
                free((*node)->l);
                (*node)->l = NULL;
            }
        }
    }

    return err;
}

int boojum_set_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, void *data, size_t *size) {
    return EFAULT;
}

void *boojum_get_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, size_t *size) {
    return NULL;
}

static int new_alloc_branch(boojum_alloc_branch_ctx **n) {
    (*n) = (boojum_alloc_branch_ctx *)malloc(sizeof(boojum_alloc_branch_ctx));

    if ((*n) == NULL) {
        return ENOMEM;
    }

    (*n)->l = NULL;
    (*n)->r = NULL;
    (*n)->d = NULL;
    (*n)->refcount = 0;

    return EXIT_SUCCESS;
}

#undef BOOJUM_BITSIZE

#undef boojum_get_bitn
