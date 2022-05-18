/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef BOOJUM_TEST_PROC_TESTS_H
#define BOOJUM_TEST_PROC_TESTS_H 1

#include <cutest.h>

CUTE_DECLARE_TEST_CASE(boojum_init_deinit_mutex_tests);

CUTE_DECLARE_TEST_CASE(boojum_mutex_lock_unlock_tests);

CUTE_DECLARE_TEST_CASE(boojum_init_deinit_thread_tests);

CUTE_DECLARE_TEST_CASE(boojum_thread_join_tests);

# if !defined(BOOJUM_WITH_C11)
 CUTE_DECLARE_TEST_CASE(boojum_get_set_flag_tests);
# endif // !defiend(BOOJUM_WITH_C11)

#endif
