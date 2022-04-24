#include <boojum_sxor.h>
#include <kryptos.h>
#include <errno.h>

int boojum_sync_sxor(boojum_alloc_leaf_ctx *aleaf, unsigned char *data, const size_t data_size) {
    // INFO(Rafael): Only call this function under a well-synchronized execution status.
    unsigned char *rp = NULL;
    unsigned char *mp = NULL, *mp_end = NULL;
    unsigned char *p = NULL;

    if (aleaf == NULL) {
        return EINVAL;
    }

    if (data_size != aleaf->m_size || aleaf->m == NULL) {
        // INFO(Rafael): It should never happen in normal conditions.
        return EINVAL;
    }

    if (aleaf->r == NULL) {
        aleaf->r = kryptos_get_random_block(aleaf->m_size);
        if (aleaf->r == NULL) {
            return ENOMEM;
        }
    }

    rp = (unsigned char *)aleaf->r;
    mp = (unsigned char *)aleaf->m;
    mp_end = mp + aleaf->m_size;
    p = data;

    while (mp != mp_end) {
        *mp = *p ^ *rp;
        *p = 0;
        mp++;
        rp++;
        p++;
    }

    mp = mp_end = rp = p = NULL;

    return EXIT_SUCCESS;
}
