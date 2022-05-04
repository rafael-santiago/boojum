#include <boojum_btree.h>
#include <boojum_sxor.h>
#include <kryptos.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

// INFO(Rafael): This module implements all i/o stuff for Boojum's tree storage model.
//               The tree is a binary tree with height equals to the cpu wordsize, e.g.:
//
//                                   [ R O O T ]
//                            -           *
//                           |S|        /   \
//                           |E|       0     1      Most significant bit
//                           |G|     /   \ /   \
//                           |A|    0    1 0    1          (..)
//                           |D|
//                           |D|     (..)   (..)
//                           |R|
//                            -    0     1 0    1   Least significant bit
//
//                            [ A L L O C  L E A F S ]

#define BOOJUM_BITSIZE (sizeof(uintptr_t) << 3)

#define boojum_get_bitn(b, n)  ( ( ((b) >> (BOOJUM_BITSIZE - (n + 1))) & 0x1 ) )

static int new_alloc_branch(boojum_alloc_branch_ctx **n);

static int boojum_add_addr_iter(boojum_alloc_branch_ctx **node, const uintptr_t segment_addr, const size_t bn);

static int boojum_del_addr_iter(boojum_alloc_branch_ctx **node, const uintptr_t segment_addr, const size_t bn);

static boojum_alloc_branch_ctx *boojum_get_alloc_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr);

static void boojum_free_alloc_leaf_ctx(boojum_alloc_leaf_ctx *leaf);

static int new_alloc_leaf(boojum_alloc_leaf_ctx **n);

static int boojum_update_xor_maskings_iter(boojum_alloc_branch_ctx **alloc_tree);

int boojum_update_xor_maskings(boojum_alloc_branch_ctx **alloc_tree) {
    if (alloc_tree == NULL) {
        return EINVAL;
    }

    if (*alloc_tree == NULL) {
        return EXIT_SUCCESS;
    }

    return boojum_update_xor_maskings_iter(alloc_tree);
}

int boojum_add_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, const size_t ssize) {
    int err = EXIT_FAILURE;
    boojum_alloc_branch_ctx *alp = NULL;

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

    if (err == EXIT_SUCCESS) {
        if ((alp = boojum_get_alloc_addr(alloc_tree, segment_addr)) == NULL) {
            // INFO(Rafael): It should never happen in normal conditions.
            boojum_del_addr(alloc_tree, segment_addr);
            return EFAULT;
        }

        if ((err = new_alloc_leaf((boojum_alloc_leaf_ctx **)&alp->d)) != EXIT_SUCCESS) {
            boojum_del_addr(alloc_tree, segment_addr);
            return ENOMEM;
        }

        ((boojum_alloc_leaf_ctx *)alp->d)->m = (void *)segment_addr;
        ((boojum_alloc_leaf_ctx *)alp->d)->m_size = ssize;

        alp = NULL;
    }

    return err;
}

int boojum_del_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr) {
    int err = EXIT_FAILURE;

    if (alloc_tree == NULL || *alloc_tree == NULL) {
        return EINVAL;
    }

    if (boojum_get_alloc_addr(alloc_tree, segment_addr) == NULL) {
        return ENOENT;
    }

    if ((*alloc_tree)->refcount == 0) {
        // INFO(Rafael): It should never happen in normal conditions.
        return EINVAL;
    }

    (*alloc_tree)->refcount--;

    if (boojum_get_bitn(segment_addr, 0)) {
        err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->r, segment_addr, 0);
        if (((boojum_alloc_branch_ctx *)(*alloc_tree)->r)->refcount == 0) {
            kryptos_freeseg((*alloc_tree)->r, sizeof(boojum_alloc_branch_ctx));
            (*alloc_tree)->r = NULL;
        }
    } else {
        err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->l, segment_addr, 0);
        if (((boojum_alloc_branch_ctx *)(*alloc_tree)->l)->refcount == 0) {
            kryptos_freeseg((*alloc_tree)->l, sizeof(boojum_alloc_branch_ctx));
            (*alloc_tree)->l = NULL;
        }
    }

    if ((*alloc_tree)->refcount == 0) {
        kryptos_freeseg((*alloc_tree), sizeof(boojum_alloc_branch_ctx));
        (*alloc_tree) = NULL;
    }

    return err;
}

static int boojum_update_xor_maskings_iter(boojum_alloc_branch_ctx **alloc_tree) {
    int err = EXIT_SUCCESS;

    if (alloc_tree == NULL || *alloc_tree == NULL) {
        return EINVAL;
    }

    if ((*alloc_tree)->d != NULL) {
        // INFO(Rafael): We found a leaf, now it is only about updating the masked allocation and
        //               returning back because since it is an allocation leaf, there is nothing
        //               below it.
        return boojum_sync_sxor_upd((boojum_alloc_leaf_ctx *)(*alloc_tree)->d);
    }

    if ((*alloc_tree)->l != NULL) {
        err = boojum_update_xor_maskings_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->l);
    }

    if (err == EXIT_SUCCESS && (*alloc_tree)->r != NULL) {
        err = boojum_update_xor_maskings_iter((boojum_alloc_branch_ctx **)&(*alloc_tree)->r);
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
        return ENOENT;
    }


    (*node)->refcount--;

    if (bn < BOOJUM_BITSIZE) {
        if (bn == BOOJUM_BITSIZE - 1 && (*node)->d != NULL) {
            boojum_free_alloc_leaf_ctx((*node)->d);
            (*node)->d = NULL;
        }

        if (boojum_get_bitn(segment_addr, bn + 1)) {
            err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*node)->r, segment_addr, bn + 1);
            if (((boojum_alloc_branch_ctx *)(*node)->r)->refcount == 0) {
                kryptos_freeseg((*node)->r, sizeof(boojum_alloc_branch_ctx));
                (*node)->r = NULL;
            }
        } else {
            err = boojum_del_addr_iter((boojum_alloc_branch_ctx **)&(*node)->l, segment_addr, bn + 1);
            if (((boojum_alloc_branch_ctx *)(*node)->l)->refcount == 0) {
                kryptos_freeseg((*node)->l, sizeof(boojum_alloc_branch_ctx));
                (*node)->l = NULL;
            }
        }
    }

    return err;
}

int boojum_set_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, void *data, size_t *size) {
    boojum_alloc_branch_ctx *alp = NULL;
    int err = EFAULT;

    if (data == NULL || size == NULL) {
        return EINVAL;
    }

    if ((alp = boojum_get_alloc_addr(alloc_tree, segment_addr)) == NULL) {
        return ENOENT;
    }

    if (alp->d == NULL || ((boojum_alloc_leaf_ctx *)alp->d)->m == NULL) {
        // INFO(Rafael): It should never happen in normal conditions.
        err = EINVAL;
        goto boojum_set_data_epilogue;
    }

    if (*size > ((boojum_alloc_leaf_ctx *)alp->d)->m_size) {
        err = EOVERFLOW;
        goto boojum_set_data_epilogue;
    }

    if (*size == 0) {
        err = EXIT_SUCCESS;
    } else {
        ((boojum_alloc_leaf_ctx *)alp->d)->u_size = *size;
        memset(((boojum_alloc_leaf_ctx *)alp->d)->m, 0, *size);
        err = boojum_sync_sxor(((boojum_alloc_leaf_ctx *)alp->d), data, *size);
        if (err == EXIT_SUCCESS) {
            *size = 0;
        }
    }

boojum_set_data_epilogue:

    alp = NULL;

    return err;
}

void *boojum_get_data(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr, size_t *size) {
    kryptos_u8_t *data = NULL, *p = NULL, *p_end = NULL;
    kryptos_u8_t *mp = NULL;
    kryptos_u8_t *key = NULL, *kp = NULL;
    boojum_alloc_branch_ctx *alp = NULL;
    boojum_alloc_leaf_ctx *aleaf = NULL;

    if (alloc_tree == NULL || *alloc_tree == NULL || size == NULL) {
        return NULL;
    }

    if ((alp = boojum_get_alloc_addr(alloc_tree, segment_addr)) == NULL) {
        goto boojum_get_data_epilogue;
    }

    aleaf = (boojum_alloc_leaf_ctx *)alp->d;

    if (aleaf->u_size == 0) {
        *size = 0;
        goto boojum_get_data_epilogue;
    }

    if ((data = (kryptos_u8_t *)kryptos_newseg(aleaf->u_size)) == NULL) {
        goto boojum_get_data_epilogue;
    }

    *size = aleaf->u_size;

    // INFO(Rafael): We could create a function capable of unmask the original data,
    //               but since it is about computers it can fail when masking data
    //               again. It would increase the chance of exposition at the origin.
    //               Doing the KDF stuff here will only require one KDF call and after
    //               zeroing the produced sensitive memory segment a.k.a. the keystream.
    //
    //               Notice that at this point we already know that aleaf->r is 3 times
    //               greater than aleaf->m_size. If it has not, the program has a bug,
    //               and it must "explode" during tests warning up us.

    key = kryptos_hkdf(aleaf->r, aleaf->u_size,
                       sha3_512,
                       aleaf->r + aleaf->u_size, aleaf->u_size,
                       aleaf->r + (aleaf->u_size << 1), aleaf->u_size,
                       aleaf->u_size);

    if (key == NULL) {
        kryptos_freeseg(data, *size);
        data = NULL;
        *size = 0;
        goto boojum_get_data_epilogue;
    }

    kp = key;
    p = data;
    p_end = p + aleaf->u_size;
    mp = (kryptos_u8_t *)aleaf->m;

    // INFO(Rafael): Let's avoid spreading this sensitive information
    //               over libc's stack by calling memcpy.

    while (p != p_end) {
        *p = *mp ^ *kp;
        p++;
        kp++;
        mp++;
    }

boojum_get_data_epilogue:

    if (key != NULL) {
        kryptos_freeseg(key, aleaf->u_size);
    }

    alp = NULL;
    aleaf = NULL;

    p = p_end = mp = kp = key = NULL;

    return data; // INFO(Rafael): Now is up to the user ensure that
                 //               it will be cleaned as soon as possible
                 //               besides not spread.
}

static void boojum_free_alloc_leaf_ctx(boojum_alloc_leaf_ctx *leaf) {
    if (leaf == NULL) {
        return;
    }

    memset((unsigned char *)leaf->m, 0, leaf->m_size);
    memset((unsigned char *)leaf->r, 0, leaf->u_size);

    if (leaf->m != NULL) {
        kryptos_freeseg(leaf->m, leaf->m_size);
    }

    if (leaf->r != NULL) {
        kryptos_freeseg(leaf->r, leaf->u_size * 3);
    }

    leaf->m_size = 0;
    leaf->u_size = 0;
    kryptos_freeseg(leaf, sizeof(boojum_alloc_leaf_ctx));
}

static boojum_alloc_branch_ctx *boojum_get_alloc_addr(boojum_alloc_branch_ctx **alloc_tree, const uintptr_t segment_addr) {
    size_t b;
    boojum_alloc_branch_ctx *atp = NULL;

    if (alloc_tree == NULL || *alloc_tree == NULL) {
        return NULL;
    }

    atp = *alloc_tree;

    for (b = 0; b < BOOJUM_BITSIZE && atp != NULL; b++) {
        if (boojum_get_bitn(segment_addr, b)) {
            atp = atp->r;
        } else {
            atp = atp->l;
        }
    }

    return atp;
}

static int new_alloc_leaf(boojum_alloc_leaf_ctx **n) {
    (*n) = (boojum_alloc_leaf_ctx *)kryptos_newseg(sizeof(boojum_alloc_leaf_ctx));

    if ((*n) == NULL) {
        return ENOMEM;
    }

    (*n)->m_size = 0;
    (*n)->u_size = 0;
    (*n)->m = NULL;
    (*n)->r = NULL;

    return EXIT_SUCCESS;
}

static int new_alloc_branch(boojum_alloc_branch_ctx **n) {
    (*n) = (boojum_alloc_branch_ctx *)kryptos_newseg(sizeof(boojum_alloc_branch_ctx));

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
