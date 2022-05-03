#include "boojum_tests.h"
#include <boojum.h>
#include <errno.h>
#include <string.h>

CUTE_TEST_CASE(boojum_init_tests)
    int yes = g_cute_leak_check;
    CUTE_ASSERT(boojum_init(0) == EINVAL);
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = yes;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_deinit_tests)
    int yes = g_cute_leak_check;
    CUTE_ASSERT(boojum_deinit() == EINVAL);
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = yes;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_alloc_free_tests)
    char *segment = NULL;
    segment = boojum_alloc(1024);
    CUTE_ASSERT(segment == NULL);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(1024);
    CUTE_ASSERT(segment != NULL);
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_set_get_tests)
    int yes = g_cute_leak_check;
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
    g_cute_leak_check = yes;
CUTE_TEST_CASE_END
