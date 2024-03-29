/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <boojum_proc.h>
#include <boojum_btree.h>
#include <kryptos.h>
#include <pthread.h>
#if defined(__FreeBSD__)
# include <pthread_np.h>
#endif
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

static void *boojum_data_wiper(void *arg);

static void *boojum_kupd_job(void *arg);

int boojum_init_mutex(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    return pthread_mutex_init(mtx, NULL);
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
    return pthread_mutex_destroy(mtx);
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
    return pthread_mutex_lock(mtx);
}

int boojum_mutex_unlock(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    return pthread_mutex_unlock(mtx);
}

int boojum_thread_join(boojum_thread *thread) {
    if (thread == NULL) {
        return EINVAL;
    }
    return pthread_join(*thread, NULL);
}

int boojum_sched_data_wiping(void *data, size_t *data_size, const size_t ttv) {
    int err = EINVAL;
    struct boojum_data_wiper_ctx *dw = NULL;
    pthread_attr_t attr;
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

    // INFO(Rafael): Just in case, but it does not seem to work.
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    err = pthread_create(&dw->thread, &attr, boojum_data_wiper, dw);
    pthread_attr_destroy(&attr);

    if (err != EXIT_SUCCESS) {
        goto boojum_sched_data_wiping_epilogue;
    }

    while (!boojum_get_flag(&dw->enabled, &dw->lock) && ntry-- > 0) {
        usleep(1);
    }

    if (!boojum_get_flag(&dw->enabled, &dw->lock)) {
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
    pthread_attr_t attr;

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

    // INFO(Rafael): Just in case, but it does not seem to work.
    pthread_attr_init(&attr);
#if !defined(__OpenBSD__)
    // INFO(Rafael): On OpenBSD running this thread in detached state
    //               will drop parent process with SIGBUS. This signal
    //               will raise at the moment the program tries to
    //               acquire the giant_lock mutex from kupd_job routine.
    pthread_attr_setdetachstate(&attr, 1);
#endif
    err = pthread_create(kupd->thread, &attr, boojum_kupd_job, kupd);
    pthread_attr_destroy(&attr);

    if (err != EXIT_SUCCESS) {
        goto boojum_run_kupd_job_epilogue;
    }

    while (ntry-- > 0 && boojum_get_flag(enabled_flag, giant_lock) == 0) {
        usleep(10);
    }

    if (boojum_get_flag(enabled_flag, giant_lock) == 0) {
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

static void *boojum_data_wiper(void *arg) {
    struct boojum_data_wiper_ctx *dw = (struct boojum_data_wiper_ctx *)arg;

    if (dw != NULL && dw->data != NULL && dw->data_size != NULL && *dw->data_size > 0) {
        if (boojum_set_flag(&dw->enabled, 1, &dw->lock) != EXIT_SUCCESS) {
            fprintf(stderr, "Boojum error: Unable to set data wiper thread enabled.\n");
        }
        usleep(10); // INFO(Rafael): On NetBSD I have noticed a monitor's thread starvation.
                    //               If we do not do this short sleep the monitor's thread
                    //               is not able to get the dw->enabled update. Since by
                    //               sleeping on other Unix-like will no hurt, I decided
                    //               make it avaiable for all.
        usleep(dw->time_to_vanish * 1000);
        kryptos_freeseg(dw->data, *dw->data_size);
        *dw->data_size = 0;
        boojum_deinit_mutex(&dw->lock);
        kryptos_freeseg(dw, sizeof(struct boojum_data_wiper_ctx));
        dw = NULL;
    }

    //pthread_exit(NULL);

    return NULL;
}

static void *boojum_kupd_job(void *arg) {
    struct boojum_kupd_ctx *kupd = (struct boojum_kupd_ctx *)arg;
    if (kupd != NULL && kupd->giant_lock != NULL && kupd->enabled != NULL && kupd->alloc_tree != NULL) {
        if (boojum_set_flag(kupd->enabled, 1, kupd->giant_lock) != EXIT_SUCCESS) {
            fprintf(stderr, "Boojum error: Unable to set KUPD thread enabled.\n");
            //pthread_exit(NULL);
            return NULL;
        }
        while (boojum_get_flag(kupd->enabled, kupd->giant_lock)) {
            if (boojum_mutex_lock(kupd->giant_lock) == EXIT_SUCCESS) {
                // TODO(Rafael): What if it has failed? What to effectively do?
                boojum_update_xor_maskings(kupd->alloc_tree);
                boojum_mutex_unlock(kupd->giant_lock);
            }
            usleep(kupd->keys_expiration_time * 1000);
        }
        kryptos_freeseg(kupd, sizeof(struct boojum_kupd_ctx));
    }

    //pthread_exit(NULL);

    return NULL;
}
