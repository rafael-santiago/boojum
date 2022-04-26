#include <boojum_proc.h>
#include <kryptos.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>

static void *boojum_data_wiper(void *arg);

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

int boojum_sched_data_wiping(void *data, const size_t data_size, const size_t ttv) {
    int err = EINVAL;
    struct boojum_data_wiper_ctx *dw = NULL;

    dw = (struct boojum_data_wiper_ctx *)kryptos_newseg(sizeof(struct boojum_data_wiper_ctx));

    if (dw == NULL || (err = boojum_init_thread(&dw->thread)) != EXIT_SUCCESS) {
        goto boojum_sched_data_wiping_epilogue;
    }

    dw->data = data;
    dw->data_size = data_size;
    dw->time_to_vanish = ttv;

    err = pthread_create(&dw->thread, NULL, boojum_data_wiper, &dw);

boojum_sched_data_wiping_epilogue:

    if (err != EXIT_SUCCESS) {
        kryptos_freeseg(dw, sizeof(struct boojum_data_wiper_ctx));
        dw = NULL;
    }

    return err;
}

static void *boojum_data_wiper(void *arg) {
    struct boojum_data_wiper_ctx *dw = (struct boojum_data_wiper_ctx *)arg;

    if (dw != NULL && dw->data != NULL && dw->data_size > 0) {
        usleep(dw->time_to_vanish * 1000);
        kryptos_freeseg(dw->data, dw->data_size);
        kryptos_freeseg(dw, sizeof(struct boojum_data_wiper_ctx));
        dw = NULL;
    }

    return NULL;
}
