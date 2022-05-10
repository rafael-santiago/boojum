#include <boojum_proc.h>
#include <boojum_btree.h>
#include <kryptos.h>
#include <threads.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

static int boojum_data_wiper(void *arg);

static int boojum_kupd_job(void *arg);

int boojum_init_mutex(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    return (mtx_init(mtx, mtx_plain) == thrd_success) ? EXIT_SUCCES : EXIT_FAILURE;
}

int boojum_init_thread(boojum_thread *thread) {
    if (thread == NULL) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int boojum_deinit_mutex(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    mtx_destroy(mtx);
    return EXIT_SUCCESS;
}

int boojum_deinit_thread(boojum_thread *thread) {
    if (thread == NULL) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int boojum_mutex_lock(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    return (mtx_lock(mtx) == thrd_success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int boojum_mutex_unlock(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    return (mtx_unlock(mtx) == thrd_success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int boojum_thread_join(boojum_thread *thread) {
    if (thread == NULL) {
        return EINVAL;
    }
    return (thrd_join(*thread, NULL) == thrd_success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int boojum_sched_data_wiping(void *data, size_t *data_size, const size_t ttv) {
    int err = EINVAL;
    struct boojum_data_wiper_ctx *dw = NULL;
    int ntry = 10;

    dw = (struct boojum_data_wiper_ctx *)kryptos_newseg(sizeof(struct boojum_data_wiper_ctx));

    if (dw == NULL || (err = boojum_init_thread(&dw->thread)) != EXIT_SUCCESS) {
        goto boojum_sched_data_wiping_epilogue;
    }

    dw->data = data;
    dw->data_size = data_size;
    dw->time_to_vanish = ttv;
    dw->enabled = 0;
    if ((err = boojum_init_mutex(&dw->lock)) != EXIT_SUCCESS) {
        goto boojum_sched_data_wiping_epilogue;
    }

    err = (thrd_create(&dw->thread,
                       boojum_data_wiper, dw) == thrd_success) ? EXIT_SUCCESS :
                                                                 EXIT_FAILURE;

    if (err != EXIT_SUCCESS) {
        goto boojum_sched_data_wiping_epilogue;
    }

    while (!dw->enabled && ntry-- > 0) {
        thrd_sleep(&(struct timespec){.tv_nsec = 1}, NULL);
    }

    if (!dw->enabled) {
        err = EFAULT;
    }

boojum_sched_data_wiping_epilogue:

    if (err != EXIT_SUCCESS) {
        kryptos_freeseg(dw, sizeof(struct boojum_data_wiper_ctx));
        dw = NULL;
    }

    return err;
}

int boojum_run_kupd_job(boojum_thread *thread,
                        boojum_mutex *giant_lock,
                        boojum_alloc_branch_ctx **alloc_tree,
                        const size_t key_expiration_time,
                        int *enabled_flag) {
    struct boojum_kupd_ctx *kupd = NULL;
    int err = EFAULT;
    int ntry = 10;

    if (alloc_tree == NULL       ||
        key_expiration_time == 0 ||
        enabled_flag == NULL) {
        return EINVAL;
    }

    kupd = (struct boojum_kupd_ctx *)kryptos_newseg(sizeof(struct boojum_kupd_ctx));
    if (kupd == NULL) {
        return ENOMEM;
    }

    kupd->thread = thread;
    kupd->giant_lock = giant_lock;
    kupd->alloc_tree = alloc_tree;
    kupd->keys_expiration_time = key_expiration_time;
    kupd->enabled = enabled_flag;

    err = (thrd_create(kupd->thread,
                      &attr, boojum_kupd_job, kupd) == thrd_success) ? EXIT_SUCCESS
                                                                     : EXIT_FAILURE;

    if (err != EXIT_SUCCESS) {
        goto boojum_run_kupd_job_epilogue;
    }

    while (ntry-- > 0 && boojum_get_flag(enabled_flag, giant_lock) == 0) {
        thrd_sleep(&(struct timespec){.tv_nsec = 10}, NULL);
    }

    if (*enabled_flag == 0) {
        err = EFAULT;
    }

boojum_run_kupd_job_epilogue:

    if (err != EXIT_SUCCESS) {
        *enabled_flag = 0;
        kryptos_freeseg(kupd, sizeof(struct boojum_kupd_ctx));
        kupd = NULL;
    }

    return err;
}

static int boojum_data_wiper(void *arg) {
    struct boojum_data_wiper_ctx *dw = (struct boojum_data_wiper_ctx *)arg;

    if (dw != NULL && dw->data != NULL && dw->data_size != NULL && *dw->data_size > 0) {
        dw->enabled = 1;
        thrd_sleep(&(struct timespec){.tv_nsec = dw->time_to_vanish * 1000}, NULL);
        kryptos_freeseg(dw->data, *dw->data_size);
        *dw->data_size = 0;
        boojum_deinit_mutex(&dw->lock);
        kryptos_freeseg(dw, sizeof(struct boojum_data_wiper_ctx));
        dw = NULL;
    }

    return EXIT_SUCCESS;
}

static int boojum_kupd_job(void *arg) {
    struct boojum_kupd_ctx *kupd = (struct boojum_kupd_ctx *)arg;
    if (kupd != NULL && kupd->giant_lock != NULL && kupd->enabled != NULL && kupd->alloc_tree != NULL) {
        *kupd->enabled = 1;
        while (*kupd->enabled) {
            if (boojum_mutex_lock(kupd->giant_lock) == EXIT_SUCCESS) {
                // TODO(Rafael): What if it has failed? What to effectively do?
                boojum_update_xor_maskings(kupd->alloc_tree);
                boojum_mutex_unlock(kupd->giant_lock);
            }
            thrd_sleep(&(struct timespec *)&{.tv_nsec = kupd->keys_expiration_time * 1000}, NULL);
        }
        kryptos_freeseg(kupd, sizeof(struct boojum_kupd_ctx));
    }

    return EXIT_SUCCESS;
}
