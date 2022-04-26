#include <boojum_sxor.h>
#include <kryptos.h>
#include <errno.h>

int boojum_sync_sxor(boojum_alloc_leaf_ctx *aleaf, unsigned char *data, const size_t data_size) {
    // INFO(Rafael): Only call this function under a well-synchronized execution status.
    kryptos_u8_t *kp = NULL, *key = NULL;
    kryptos_u8_t *mp = NULL, *mp_end = NULL;
    kryptos_u8_t *p = NULL;
    size_t r_size = 0;

    if (aleaf == NULL) {
        return EINVAL;
    }

    if (data_size != aleaf->m_size || aleaf->m == NULL) {
        // INFO(Rafael): It should never happen in normal conditions.
        return EINVAL;
    }

    if (aleaf->r == NULL) {
        aleaf->r = kryptos_get_random_block(aleaf->m_size * 3);
        if (aleaf->r == NULL) {
            return ENOMEM;
        }
    }

    key = kryptos_hkdf(aleaf->r, aleaf->m_size,
                       sha3_512,
                       aleaf->r + aleaf->m_size, aleaf->m_size,
                       aleaf->r + (aleaf->m_size << 1), aleaf->m_size,
                       aleaf->m_size);

    if (key == NULL) {
        return ENOMEM;
    }

    kp = key;
    mp = (unsigned char *)aleaf->m;
    mp_end = mp + aleaf->m_size;
    p = data;

    while (mp != mp_end) {
        *mp = *p ^ *kp;
        *p = 0;
        mp++;
        kp++;
        p++;
    }

    if (key != NULL) {
        kryptos_freeseg(key, aleaf->m_size);
    }

    mp = mp_end = kp = p = key = NULL;

    return EXIT_SUCCESS;
}
