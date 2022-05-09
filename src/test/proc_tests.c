#include "proc_tests.h"
#include <boojum_proc.h>

struct th_get_set_tests_ctx {
    boojum_mutex *mtx;
    int *enabled;
    int done;
};

#if defined(__unix__)
static void *th_test_routine(void *arg);
static void *th_test_get_set_routine(void *args);
#elif defined(_WIN32)
static DWORD WINAPI th_test_routine(PVOID arg);
static DWORD WINAPI th_test_get_set_routine(void *args);
#else
# error Some code wanted.
#endif

CUTE_TEST_CASE(boojum_init_deinit_mutex_tests)
    boojum_mutex mtx;
    CUTE_ASSERT(boojum_init_mutex(NULL) == EXIT_FAILURE);
    CUTE_ASSERT(boojum_init_mutex(&mtx) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit_mutex(NULL) == EXIT_FAILURE);
    CUTE_ASSERT(boojum_deinit_mutex(&mtx) == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_mutex_lock_unlock_tests)
    boojum_mutex mtx;
    CUTE_ASSERT(boojum_init_mutex(&mtx) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_mutex_lock(NULL) == EXIT_FAILURE);
    CUTE_ASSERT(boojum_mutex_lock(&mtx) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_mutex_unlock(NULL) == EXIT_FAILURE);
    CUTE_ASSERT(boojum_mutex_unlock(&mtx) == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_init_deinit_thread_tests)
    boojum_thread th;
    CUTE_ASSERT(boojum_init_thread(NULL) == EXIT_FAILURE);
    CUTE_ASSERT(boojum_init_thread(&th) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit_thread(NULL) == EXIT_FAILURE);
    CUTE_ASSERT(boojum_deinit_thread(&th) == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_thread_join_tests)
    int status = g_cute_leak_check;
    boojum_thread th;
    int retval = 0;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init_thread(&th) == EXIT_SUCCESS);
#if defined(__unix__)
    CUTE_ASSERT(pthread_create(&th, NULL, th_test_routine, &retval) == EXIT_SUCCESS);
#elif defined(_WIN32)
    th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)th_test_routine, &retval, 0, NULL);
    CUTE_ASSERT(th != NULL);
#else
# error Some code wanted.
#endif
    CUTE_ASSERT(retval == 0);
    CUTE_ASSERT(boojum_thread_join(&th) == EXIT_SUCCESS);
    CUTE_ASSERT(retval == 1);
    CUTE_ASSERT(boojum_deinit_thread(&th) == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_get_set_flag_tests)
    int status = g_cute_leak_check;
    struct th_get_set_tests_ctx th_args;
    boojum_thread th;
    boojum_mutex mtx;
    int enabled = 0;
    size_t ntry = 10;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init_thread(&th) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_init_mutex(&mtx) == EXIT_SUCCESS);
    th_args.mtx = &mtx;
    th_args.enabled = &enabled;
    th_args.done = 0;
#if defined(__unix__)
    CUTE_ASSERT(pthread_create(&th, NULL, th_test_get_set_routine, &th_args) == EXIT_SUCCESS);
    while (ntry-- > 0 && boojum_get_flag(&enabled, &mtx) == 0) {
        usleep(1);
    }
#elif defined(_WIN32)
    th = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)th_test_routine, &retval, 0, NULL);
    CUTE_ASSERT(th != NULL);
    while (ntry-- > 0 && boojum_get_flag(&enabled, &mtx) == 0) {
        Sleep(1);
    }
#else
# error Some code wanted.
#endif
    CUTE_ASSERT(boojum_set_flag(&enabled, 0, &mtx) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_thread_join(&th) == EXIT_SUCCESS);
    CUTE_ASSERT(th_args.done == 1);
    CUTE_ASSERT(boojum_deinit_thread(&th) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit_mutex(&mtx) == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

#if defined(__unix__)
static void *th_test_routine(void *arg) {
    int *retval = NULL;
    sleep(5);
    retval = (int *)arg;
    *retval = 1;
    return NULL;
}

static void *th_test_get_set_routine(void *args) {
    struct th_get_set_tests_ctx *th_args = (struct th_get_set_tests_ctx *)args;
    if (th_args->done == 0 &&
        boojum_set_flag(th_args->enabled, 1, th_args->mtx) == EXIT_SUCCESS) {
        usleep(1);
        while (boojum_get_flag(th_args->enabled, th_args->mtx) == 1) {
            usleep(1);
        }
        th_args->done = 1;
    }
    return NULL;
}
#elif defined(_WIN32)
static DWORD WINAPI th_test_routine(PVOID arg) {
    int *retval = NULL;
    sleep(5);
    retval = (int *)arg;
    *retval = 1;
    return 0;
}

static DWORD WINAPI th_test_get_set_routine(PVOID args) {
    struct th_get_set_tests_ctx *th_args = (struct th_get_set_tests_ctx *)args;
    if (th_args->done == 0 &&
        boojum_set_flag(th_args->enabled, 1, th_args->mtx) == EXIT_SUCCESS) {
        usleep(1);
        while (boojum_get_flag(th_args->enabled, th_args->mtx) == 1) {
            usleep(1);
        }
        th_args->done = 1;
    }
    return 0;
}
#else
# error Some code wanted.
#endif
