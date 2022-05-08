#include <boojum_proc.h>
#include <boojum_btree.h>
#include <kryptos.h>
#include <errno.h>

static DWORD WINAPI boojum_data_wiper(LPVOID arg);

static DWORD WINAPI boojum_kupd_job(LPVOID arg);

int boojum_init_mutex(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    *mtx = CreateMutex(NULL, FALSE, NULL);
    return (*mtx != NULL) ? EXIT_SUCCESS : EXIT_FAILURE;
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
    
    CloseHandle(*mtx);
    
    return EXIT_SUCCESS;
}

int boojum_deinit_thread(boojum_thread *thread) {
    if (thread == NULL) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

int boojum_mutex_lock(boojum_mutex *mtx) {
    DWORD wait_status = 0;
    
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    
    wait_status = WaitForSingleObject(*mtx, INFINITE);
    
    return (wait_status == WAIT_OBJECT_0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int boojum_mutex_unlock(boojum_mutex *mtx) {
    if (mtx == NULL) {
        return EXIT_FAILURE;
    }
    
    return ReleaseMutex(*mtx) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int boojum_thread_join(boojum_thread *thread) {
    if (thread == NULL) {
        return EINVAL;
    }
    
    if (WaitForSingleObject(*thread, INFINITE) == WAIT_FAILED) {
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
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

    dw->thread = CreateThread(NULL, 0,
                              (LPTHREAD_START_ROUTINE)boojum_data_wiper, dw,
                              0, NULL);
    if (dw->thread == NULL) {
        err = EFAULT;
        goto boojum_sched_data_wiping_epilogue;
    }

    while (!dw->enabled && ntry-- > 0) {
        Sleep(1);
    }

    err = (!dw->enabled) ? EXIT_FAILURE : EXIT_SUCCESS;

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

    kupd->thread = CreateThread(NULL, 0,
                                (LPTHREAD_START_ROUTINE)boojum_kupd_job, kupd,
                                0, NULL);
    if (kupd->thread == NULL) {
        err = EFAULT;
        goto boojum_run_kupd_job_epilogue;
    }

    while (ntry-- > 0 && *enabled_flag == 0) {
        Sleep(10);
    }

    err = (*enabled_flag == 0) ? EXIT_FAILURE : EXIT_SUCCESS;

boojum_run_kupd_job_epilogue:

    if (err != EXIT_SUCCESS) {
        *enabled_flag = 0;
        kryptos_freeseg(kupd, sizeof(struct boojum_kupd_ctx));
        kupd = NULL;
    }

    return err;
}

static DWORD WINAPI boojum_data_wiper(LPVOID arg) {
    struct boojum_data_wiper_ctx *dw = (struct boojum_data_wiper_ctx *)arg;

    if (dw != NULL && dw->data != NULL && dw->data_size != NULL && *dw->data_size > 0) {
        dw->enabled = 1;
        Sleep(dw->time_to_vanish);
        kryptos_freeseg(dw->data, *dw->data_size);
        *dw->data_size = 0;
        kryptos_freeseg(dw, sizeof(struct boojum_data_wiper_ctx));
        dw = NULL;
    }

    return 0;
}

static DWORD WINAPI boojum_kupd_job(LPVOID arg) {
    struct boojum_kupd_ctx *kupd = (struct boojum_kupd_ctx *)arg;
    if (kupd != NULL && kupd->giant_lock != NULL && kupd->enabled != NULL && kupd->alloc_tree != NULL) {
        *kupd->enabled = 1;
        while (*kupd->enabled) {
            if (boojum_mutex_lock(kupd->giant_lock) == EXIT_SUCCESS) {
                // TODO(Rafael): What if it has failed? What to effectively do?
                boojum_update_xor_maskings(kupd->alloc_tree);
                boojum_mutex_unlock(kupd->giant_lock);
            }
            Sleep(kupd->keys_expiration_time);
        }
        kryptos_freeseg(kupd, sizeof(struct boojum_kupd_ctx));
    }
    
    return 0;
}

