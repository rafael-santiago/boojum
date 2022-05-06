#include "boojum_tests.h"
#include <boojum.h>
#include <errno.h>
#include <string.h>

// WARN(Rafael): libcutest's memory leak detector have been disabled for all main boojum's entry points
//               because it depends on pthread conveniences and it by default leaks some resources even
//               when taking care of requesting it for do not leak nothing... Anyway, all btree tests
//               are being executed with memory leak detector tuned on. In Boojum this is the main
//               spot where memory leak could be harmful.

CUTE_TEST_CASE(boojum_init_tests)
    int status = g_cute_leak_check;
    CUTE_ASSERT(boojum_init(0) == EINVAL);
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_deinit_tests)
    int status = g_cute_leak_check;
    CUTE_ASSERT(boojum_deinit() == EINVAL);
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_alloc_free_tests)
    int status = g_cute_leak_check;
    char *segment = NULL;
    g_cute_leak_check = 0;
    segment = boojum_alloc(1024);
    CUTE_ASSERT(segment == NULL);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(1024);
    CUTE_ASSERT(segment != NULL);
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_set_get_tests)
    int status = g_cute_leak_check;
    char *segment = NULL;
    char *plain = NULL;
    size_t plain_size = 0;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(6);
    CUTE_ASSERT(segment != NULL);
    plain_size = 6;
    plain = (char *)malloc(plain_size);
    CUTE_ASSERT(plain != NULL);
    memcpy(plain, "foobar", plain_size);
    CUTE_ASSERT(boojum_set(segment, plain, &plain_size) == EXIT_SUCCESS);
    CUTE_ASSERT(plain_size == 0);
    CUTE_ASSERT(memcmp(plain, "foobar", 6) != 0);
    free(plain);
    CUTE_ASSERT(memcmp(segment, "foobar", 6) != 0);
    plain = boojum_get(segment, &plain_size);
    CUTE_ASSERT(plain != NULL);
    CUTE_ASSERT(plain_size == 6);
    CUTE_ASSERT(memcmp(plain, "foobar", 6) == 0);
    free(plain);
    // INFO(Rafael): Since it is a C library it is up to users free everything they have allocated.
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_alloc_realloc_free_tests)
    int status = g_cute_leak_check;
    char *old = NULL, *new = NULL;
    char *data = NULL;
    size_t data_size = 0;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    old = boojum_alloc(3);
    CUTE_ASSERT(old != NULL);
    CUTE_ASSERT(boojum_realloc(NULL, 6) == NULL);
    CUTE_ASSERT(boojum_realloc(old, 0) == NULL);
    new = boojum_realloc(old, 6);
    CUTE_ASSERT(new != NULL && new != old);
    CUTE_ASSERT(boojum_free(new) == EXIT_SUCCESS);
    data = (char *)malloc(3);
    CUTE_ASSERT(data != NULL);
    memcpy(data, "foo", 3);
    data_size = 3;
    old = boojum_alloc(3);
    CUTE_ASSERT(old != NULL);
    CUTE_ASSERT(boojum_set(old, data, &data_size) == EXIT_SUCCESS);
    free(data);
    new = boojum_realloc(old, 6);
    CUTE_ASSERT(new != NULL && new != old);
    data = boojum_get(new, &data_size);
    CUTE_ASSERT(data != NULL);
    CUTE_ASSERT(data_size == 3);
    CUTE_ASSERT(memcmp(data, "foo", 3) == 0);
    free(data);
    data = (char *)malloc(6);
    CUTE_ASSERT(data != NULL);
    memcpy(data, "foobar", 6);
    data_size = 6;
    CUTE_ASSERT(boojum_set(new, data, &data_size) == EXIT_SUCCESS);
    free(data);
    data = boojum_get(new, &data_size);
    CUTE_ASSERT(data != NULL);
    CUTE_ASSERT(data_size == 6);
    CUTE_ASSERT(memcmp(data, "foobar", 6) == 0);
    free(data);
    CUTE_ASSERT(boojum_free(new) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_set_timed_get_tests)
    char *data = NULL;
    size_t data_size = 0;
    char *segment = NULL;
    int status = g_cute_leak_check;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(6);
    CUTE_ASSERT(segment != NULL);
    data = (char *)malloc(6);
    CUTE_ASSERT(data != NULL);
    data_size = 6;
    memcpy(data, "secret", data_size);
    CUTE_ASSERT(boojum_set(segment, data, &data_size) == EXIT_SUCCESS);
    CUTE_ASSERT(data_size == 0);
    CUTE_ASSERT(memcmp(data, "\x0\x0\x0\x0\x0\x0", 6) == 0);
    free(data);
    data = boojum_timed_get(segment, &data_size, 1000);
    CUTE_ASSERT(data_size == 6);
    CUTE_ASSERT(data != NULL);
    CUTE_ASSERT(memcmp(data, "secret", data_size) == 0);
#if defined(__unix__)
    sleep(3);
#else
# error Some code wanted.
#endif
    CUTE_ASSERT(data_size == 0);
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END
