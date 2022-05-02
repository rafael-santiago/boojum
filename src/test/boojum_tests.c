#include "boojum_tests.h"
#include <boojum.h>
#include <errno.h>

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
