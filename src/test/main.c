#include <cutest.h>
#include "boojum_tests.h"
#include "btree_tests.h"

CUTE_TEST_CASE(boojum_tests)
    CUTE_RUN_TEST(boojum_btree_tests);
    CUTE_RUN_TEST(boojum_init_tests);
    CUTE_RUN_TEST(boojum_deinit_tests);
    CUTE_RUN_TEST(boojum_alloc_free_tests);
    CUTE_RUN_TEST(boojum_set_get_tests);
CUTE_TEST_CASE_END

CUTE_MAIN(boojum_tests)
