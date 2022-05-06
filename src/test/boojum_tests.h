#ifndef BOOJUM_TEST_BOOJUM_H
#define BOOJUM_TEST_BOOJUM_H 1

#include <cutest.h>

CUTE_DECLARE_TEST_CASE(boojum_init_tests);

CUTE_DECLARE_TEST_CASE(boojum_deinit_tests);

CUTE_DECLARE_TEST_CASE(boojum_alloc_free_tests);

CUTE_DECLARE_TEST_CASE(boojum_set_get_tests);

CUTE_DECLARE_TEST_CASE(boojum_alloc_realloc_free_tests);

CUTE_DECLARE_TEST_CASE(boojum_set_timed_get_tests);

CUTE_DECLARE_TEST_CASE(boojum_kupd_assurance_tests);

#endif
