#include "boojum_tests.h"
#include <boojum.h>
#include <errno.h>

CUTE_TEST_CASE(boojum_init_tests)
    CUTE_ASSERT(boojum_init(0) == EINVAL);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_deinit_tests)
    CUTE_ASSERT(boojum_deinit() == EINVAL);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
CUTE_TEST_CASE_END
