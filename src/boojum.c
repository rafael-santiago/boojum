#include <boojum.h>
#include <boojum_types.h>
#include <boojum_proc.h>
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
    return NULL;
}

void boojum_free(void *ptr) {
}

int boojum_set(void *ptr, void *data, size_t *data_size) {
    return ENOTSUP;
}

void *boojum_get(const void *ptr, size_t *data_size) {
    return NULL;
}

void *boojum_timed_get(const void *ptr, const size_t ttv) {
    return NULL;
}

