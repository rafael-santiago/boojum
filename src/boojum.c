#include <boojum.h>
#include <boojum_types.h>
#include <boojum_proc.h>
#include <boojum_btree.h>
#include <kryptos.h>
#include <errno.h>

int boojum_init(const size_t kupd_timeout_in_msecs) {
    int err = EFAULT;

    if (kupd_timeout_in_msecs == 0) {
        return EINVAL;
    }

    if (gBoojumCtx != NULL) {
        err = boojum_deinit();
        if (err != EXIT_SUCCESS) {
            return err;
        }
    }

    gBoojumCtx = (struct boojum_ctx *) malloc(sizeof(struct boojum_ctx));
    if (gBoojumCtx == NULL) {
        return ENOMEM;
    }

    if ((err = boojum_init_mutex(&gBoojumCtx->giant_lock)) != EXIT_SUCCESS &&
        (err = boojum_init_thread(&gBoojumCtx->kupd)) != EXIT_SUCCESS) {
        free(gBoojumCtx);
        gBoojumCtx = NULL;
        return err;
    }

    gBoojumCtx->kupd_in_msecs = kupd_timeout_in_msecs;
    gBoojumCtx->alloc_tree = NULL;

    // TODO(Rafael): Start kupd routine.

    return EXIT_SUCCESS;
}

int boojum_deinit(void) {
    int err = EFAULT;

    if (gBoojumCtx == NULL) {
        return EINVAL;
    }

    if ((err = boojum_mutex_lock(&gBoojumCtx->giant_lock)) == EXIT_SUCCESS) {
        if ((err = boojum_deinit_thread(&gBoojumCtx->kupd)) == EXIT_SUCCESS) {
            boojum_mutex_unlock(&gBoojumCtx->giant_lock);
            boojum_deinit_mutex(&gBoojumCtx->giant_lock);
            free(gBoojumCtx);
            gBoojumCtx = NULL;
        }
    }

    return err;
}

void *boojum_alloc(const size_t ssize) {
    void *ptr = NULL;

    if (ssize == 0 || gBoojumCtx == NULL) {
        return NULL;
    }

    if (boojum_mutex_lock(&gBoojumCtx->giant_lock) != EXIT_SUCCESS) {
        return NULL;
    }

    if ((ptr = kryptos_newseg(ssize)) == NULL) {
        ptr = NULL;
        goto boojum_alloc_epilogue;
    }

    if (boojum_add_addr(&gBoojumCtx->alloc_tree, (uintptr_t)ptr) != EXIT_SUCCESS) {
        kryptos_freeseg(ptr, ssize);
        ptr = NULL;
    }

boojum_alloc_epilogue:

    boojum_mutex_unlock(&gBoojumCtx->giant_lock); // TODO(Rafael): Philosophical compsci question, what if unlock has failed?! :-\

    return ptr;
}

int boojum_free(void *ptr) {
    int err = EFAULT;

    if (ptr == NULL || gBoojumCtx == NULL) {
        return EINVAL;
    }

    if ((err = boojum_mutex_lock(&gBoojumCtx->giant_lock)) != EXIT_SUCCESS) {
        return err;
    }

    err = boojum_del_addr(&gBoojumCtx->alloc_tree, (uintptr_t)ptr);

    boojum_mutex_unlock(&gBoojumCtx->giant_lock); // :-\

    return err;
}

int boojum_set(void *ptr, void *data, size_t *data_size) {
    int err = EFAULT;

    if (ptr == NULL || data == NULL || data_size == NULL || gBoojumCtx == NULL) {
        return EINVAL;
    }

    if ((err = boojum_mutex_lock(&gBoojumCtx->giant_lock)) != EXIT_SUCCESS) {
        return err;
    }

    err = boojum_set_data(&gBoojumCtx->alloc_tree, (uintptr_t)ptr, data, data_size);

    boojum_mutex_unlock(&gBoojumCtx->giant_lock); // :-\

    return err;
}

void *boojum_get(const void *ptr, size_t *data_size) {
    void *data = NULL;

    if (ptr == NULL || data_size == NULL || gBoojumCtx == NULL) {
        return NULL;
    }

    *data_size = 0;

    if (boojum_mutex_lock(&gBoojumCtx->giant_lock) != EXIT_SUCCESS) {
        return NULL;
    }

    data = boojum_get_data(&gBoojumCtx->alloc_tree, (uintptr_t)ptr, data_size);

    boojum_mutex_unlock(&gBoojumCtx->giant_lock); // :-\

    return data;
}

void *boojum_timed_get(const void *ptr, size_t *data_size, const size_t ttv) {
    void *data = boojum_get(ptr, data_size);

    if (data != NULL && boojum_sched_data_wiping(data, *data_size, ttv) != EXIT_SUCCESS) {
        kryptos_freeseg(data, *data_size);
        *data_size = 0;
        data = NULL;
    }

    return data;
}
