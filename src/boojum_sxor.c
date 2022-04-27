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

int boojum_sync_sxor_upd(boojum_alloc_leaf_ctx *aleaf) {
    // INFO(Rafael): This function is supposed to work on only with a previous
    //               well-synchronized execution status.
    kryptos_u8_t *key[2], *kp = NULL, *kpp = NULL;
    kryptos_u8_t *r, *rp = NULL;
    kryptos_u8_t *mp = NULL, *mp_end = NULL;
    int err = EFAULT;

    key[0] = key[1] = NULL;

    if (aleaf == NULL && aleaf->r == NULL) {
        return EINVAL;
    }

    if (aleaf->m == NULL || aleaf->m_size == 0) {
        return EXIT_SUCCESS;
    }

    if ((r = kryptos_get_random_block(aleaf->m_size * 3)) == NULL) {
        return ENOMEM;
    }

    rp = r;
    mp = aleaf->r;
    mp_end = mp + aleaf->m_size * 3;

    while (mp != mp_end) {
        *rp ^= *mp;
        rp++;
        mp++;
    }

    // INFO(Rafael): key[0] is equivalent to H(r) and key[1] is H(r ^ r').

    key[0] = kryptos_hkdf(aleaf->r, aleaf->m_size,
                          sha3_512,
                          aleaf->r + aleaf->m_size, aleaf->m_size,
                          aleaf->r + (aleaf->m_size << 1), aleaf->m_size,
                          aleaf->m_size);

    key[1] = kryptos_hkdf(r, aleaf->m_size,
                          sha3_512,
                          aleaf->r + aleaf->m_size, aleaf->m_size,
                          aleaf->r + (aleaf->m_size << 1),  aleaf->m_size,
                          aleaf->m_size);

    if (key[0] == NULL || key[1] == NULL) {
        err = ENOMEM;
        goto boojum_sync_sxor_upd_epilogue;
    }

    kp = key[0];
    kpp = key[1];
    mp = aleaf->m;
    mp_end = mp + aleaf->m_size;

    while (mp != mp_end) {
        *mp = *mp ^ *kp ^ *kpp;
        kp++;
        kpp++;
        mp++;
    }

    kryptos_freeseg(aleaf->r, aleaf->m_size * 3);
    aleaf->r = r;
    r = NULL;

    err = EXIT_SUCCESS;

boojum_sync_sxor_upd_epilogue:

    if (r == NULL) {
        kryptos_freeseg(r, aleaf->m_size * 3);
    }

    if (key[0] == NULL) {
        kryptos_freeseg(key[0], aleaf->m_size);
    }

    if (key[1] == NULL) {
        kryptos_freeseg(key[1], aleaf->m_size);
    }

    key[0] = key[1] =
    kp = kpp =
    r = rp =
    mp = mp_end = NULL;

    return err;
}
