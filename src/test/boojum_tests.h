/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#ifndef BOOJUM_TEST_BOOJUM_TESTS_H
#define BOOJUM_TEST_BOOJUM_TESTS_H 1

#include <cutest.h>

CUTE_DECLARE_TEST_CASE(boojum_init_tests);

CUTE_DECLARE_TEST_CASE(boojum_deinit_tests);

CUTE_DECLARE_TEST_CASE(boojum_alloc_free_tests);

CUTE_DECLARE_TEST_CASE(boojum_set_get_tests);

CUTE_DECLARE_TEST_CASE(boojum_alloc_realloc_free_tests);

CUTE_DECLARE_TEST_CASE(boojum_set_timed_get_tests);

CUTE_DECLARE_TEST_CASE(boojum_kupd_assurance_tests);

CUTE_DECLARE_TEST_CASE(boojum_poke_tests);

#endif
