/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
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
#if !defined(BOOJUM_WITH_C11)
    CUTE_RUN_TEST(boojum_get_set_flag_tests);
#endif
    CUTE_RUN_TEST(boojum_init_tests);
    CUTE_RUN_TEST(boojum_deinit_tests);
    CUTE_RUN_TEST(boojum_alloc_free_tests);
    CUTE_RUN_TEST(boojum_set_get_tests);
    CUTE_RUN_TEST(boojum_alloc_realloc_free_tests);
    CUTE_RUN_TEST(boojum_set_timed_get_tests);
    CUTE_RUN_TEST(boojum_kupd_assurance_tests);
CUTE_TEST_CASE_END

CUTE_MAIN(boojum_tests)
