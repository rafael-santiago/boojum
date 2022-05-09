#ifndef BOOJUM_TEST_PROC_TESTS_H
#define BOOJUM_TEST_PROC_TESTS_H 1

#include <cutest.h>

CUTE_DECLARE_TEST_CASE(boojum_init_deinit_mutex_tests);

CUTE_DECLARE_TEST_CASE(boojum_mutex_lock_unlock_tests);

CUTE_DECLARE_TEST_CASE(boojum_init_deinit_thread_tests);

CUTE_DECLARE_TEST_CASE(boojum_thread_join_tests);

CUTE_DECLARE_TEST_CASE(boojum_get_set_flag_tests);

#endif
