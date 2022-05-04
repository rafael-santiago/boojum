#include <boojum.h>
#include <boojum_types.h>
#include <boojum_proc.h>
#include <boojum_btree.h>
#include <kryptos.h>
#include <errno.h>
#if defined(__unix__)
# include <unistd.h>
#endif
#include <stdio.h>

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

    gBoojumCtx = (struct boojum_ctx *) kryptos_newseg(sizeof(struct boojum_ctx));
    if (gBoojumCtx == NULL) {
        return ENOMEM;
    }

    if ((err = boojum_init_mutex(&gBoojumCtx->giant_lock)) != EXIT_SUCCESS &&
        (err = boojum_init_thread(&gBoojumCtx->kupd)) != EXIT_SUCCESS) {
        free(gBoojumCtx);
        gBoojumCtx = NULL;
        return err;
    }

    gBoojumCtx->kupd = 0;
    gBoojumCtx->kupd_in_msecs = kupd_timeout_in_msecs;
    gBoojumCtx->alloc_tree = NULL;
    gBoojumCtx->kupd_in_msecs = 1000; // INFO(Rafael): I have been using the default suggested in the original paper
                                      //               for the xor maskings update, 1 second.
    gBoojumCtx->kupd_enabled = 0;

    if ((err = boojum_run_kupd_job(&gBoojumCtx->kupd,
                                   &gBoojumCtx->alloc_tree,
                                   gBoojumCtx->kupd_in_msecs,
                                   &gBoojumCtx->kupd_enabled)) != EXIT_SUCCESS) {
        boojum_deinit();
    }

    return err;
}

int boojum_deinit(void) {
    int err = EFAULT;

    if (gBoojumCtx == NULL) {
        return EINVAL;
    }

    gBoojumCtx->kupd_enabled = 0;
    boojum_thread_join(&gBoojumCtx->kupd);

    if ((err = boojum_mutex_lock(&gBoojumCtx->giant_lock)) == EXIT_SUCCESS) {
        if ((err = boojum_deinit_thread(&gBoojumCtx->kupd)) == EXIT_SUCCESS) {
            boojum_mutex_unlock(&gBoojumCtx->giant_lock);
            boojum_deinit_mutex(&gBoojumCtx->giant_lock);
            kryptos_freeseg(gBoojumCtx, sizeof(struct boojum_ctx));
            gBoojumCtx = NULL;
        }
    }

#if defined(__unix__)
    usleep(1000);
#endif

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

    if (boojum_add_addr(&gBoojumCtx->alloc_tree, (uintptr_t)ptr, ssize) != EXIT_SUCCESS) {
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

void *boojum_realloc(void *ptr, const size_t ssize) {
    // INFO(Rafael): Different from the original realloc, this function
    //               will always return a new segment address. This make
    //               the operation more easy besides make more random
    //               the data storage. Enhancing the data protection idea.
    void *data = NULL;
    size_t data_size = 0;
    void *n_ptr = NULL;

    if (ssize == 0 || ptr == NULL || gBoojumCtx == NULL) {
        return NULL;
    }

    data = boojum_get(ptr, &data_size);

    if ((n_ptr = boojum_alloc(ssize)) == NULL) {
        goto boojum_realloc_epilogue;
    }

    if (data != NULL &&
        boojum_set(n_ptr, data, &data_size) != EXIT_SUCCESS) {
        boojum_free(n_ptr);
        n_ptr = NULL;
        goto boojum_realloc_epilogue;
    }

    if (boojum_free(ptr) != EXIT_SUCCESS) {
        boojum_free(n_ptr);
        n_ptr = NULL;
        goto boojum_realloc_epilogue;
    }

    ptr = NULL;

boojum_realloc_epilogue:

    if (data != NULL) {
        kryptos_freeseg(data, data_size);
    }

    data = NULL;
    data_size = 0;

    return n_ptr;
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
