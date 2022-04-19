#include <boojum_proc.h>
#include <pthread.h>
#include <errno.h>

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
