#include <boojum_btree.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

static int new_alloc_branch(boojum_alloc_branch_ctx **n);

int boojum_add_addr(const uintptr_t segment_addr) {
    size_t b = 0;
    uintptr_t s = segment_addr;
    int err = EFAULT;
    boojum_alloc_branch_ctx *at = gBoojumCtx->alloc_tree, *atp, *new;

    if (at == NULL) {
        if ((err = new_alloc_branch(&at)) != 0) {
            goto boojum_add_addr_epilogue;
        }
    }

    atp = at;

    do {
        if ((s & 0x1) == 0) {
            if ((err = new_alloc_branch(&new)) != 0) {
                goto boojum_add_addr_epilogue;
            }
            atp->l = new;
            atp = atp->l;
        } else {
            if ((err = new_alloc_branch(&new)) != 0) {
                goto boojum_add_addr_epilogue;
            }
            atp->r = new;
            atp = atp->r;
        }
        b++;
        s >>= 1;
    } while (b < sizeof(uintptr_t) << 3);

    if (gBoojumCtx->alloc_tree == NULL) {
        gBoojumCtx->alloc_tree = at;
    }

    err = 0;

boojum_add_addr_epilogue:

    if (err != 0) {
        boojum_del_addr(segment_addr);
    }

    return err;
}

int boojum_del_addr(const uintptr_t segment_addr) {
    boojum_alloc_branch_ctx *atp = gBoojumCtx->alloc_tree, *mempath[sizeof(uintptr_t) << 3], *mp = NULL;
    boojum_alloc_leaf_ctx *lp = NULL;
    uintptr_t s = segment_addr;
    ssize_t b = 0;

    do {
        if (s & 1) {
            mempath[b] = atp->r;
            atp = atp->r;
        } else {
            mempath[b] = atp->l;
            atp = atp->l;
        }
        b++;
        s >>= 1;
    } while (b < sizeof(uintptr_t) << 3);

    if (atp->d != NULL) {
        lp = (boojum_alloc_leaf_ctx *)atp->d;
        if (lp->m != NULL) {
            memset((unsigned char *)lp->m, 0, lp->m_size);
            memset((unsigned char *)lp->r, 0, lp->m_size);
            free(lp->m);
            free(lp->r);
            lp->m_size = 0;
        }
        free(atp->d);
        atp->d = NULL;
    }

    b -= 1;
    s = segment_addr;

    do {
        mp = mempath[b];

        if (b > 0) {
            // INFO(Rafael): Unlink the current value in the upper branch.
            mp = mempath[b - 1];
            if ((s >> ((sizeof(uintptr_t) << 3) - b + 1)) & 0x1) {
                mp->r = NULL;
            } else {
                mp->l = NULL;
            }
        }

        mp->refcount--;
        if (mp->refcount == 0) {
            free(mp);
        }

        b--;
    } while (b > 0);

    return 0;
}

int boojum_set_data(const uintptr_t segment_addr, void *data, size_t *size) {
    return EFAULT;
}

void *boojum_get_data(const uintptr_t segment_addr, size_t *size) {
    return NULL;
}

static int new_alloc_branch(boojum_alloc_branch_ctx **n) {
    (*n) = (boojum_alloc_branch_ctx *)malloc(sizeof(boojum_alloc_branch_ctx));

    if ((*n) == NULL) {
        return ENOMEM;
    }

    (*n)->l = (*n)->r = (*n)->d = NULL;
    (*n)->refcount = 1;

    return 0;
}
