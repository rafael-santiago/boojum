#include <cutest.h>
#include "boojum_tests.h"
#include "btree_tests.h"

CUTE_TEST_CASE(boojum_tests)
    CUTE_RUN_TEST(boojum_init_tests);
    CUTE_RUN_TEST(boojum_deinit_tests);
    CUTE_RUN_TEST(boojum_add_addr_tests);
CUTE_TEST_CASE_END

CUTE_MAIN(boojum_tests)
