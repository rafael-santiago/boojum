#include <cutest.h>
#include "btree_tests.h"
#include "proc_tests.h"
#include "boojum_tests.h"

CUTE_TEST_CASE(boojum_tests)
    CUTE_RUN_TEST(boojum_btree_tests);
    CUTE_RUN_TEST(boojum_init_deinit_mutex_tests);
    CUTE_RUN_TEST(boojum_mutex_lock_unlock_tests);
    CUTE_RUN_TEST(boojum_init_deinit_thread_tests);
    CUTE_RUN_TEST(boojum_thread_join_tests);
    CUTE_RUN_TEST(boojum_get_set_flag_tests);
    CUTE_RUN_TEST(boojum_init_tests);
    CUTE_RUN_TEST(boojum_deinit_tests);
    CUTE_RUN_TEST(boojum_alloc_free_tests);
    CUTE_RUN_TEST(boojum_set_get_tests);
    CUTE_RUN_TEST(boojum_alloc_realloc_free_tests);
    CUTE_RUN_TEST(boojum_set_timed_get_tests);
    CUTE_RUN_TEST(boojum_kupd_assurance_tests);
CUTE_TEST_CASE_END

CUTE_MAIN(boojum_tests)
