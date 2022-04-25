#include "btree_tests.h"
#include <boojum_btree.h>
#include <boojum.h>
#include <errno.h>
#include <string.h>

#define BOOJUM_BITSIZE (sizeof(uintptr_t) << 3)

#define boojum_get_bitn(b, n)  ( ( ((b) >> (BOOJUM_BITSIZE - (n + 1))) & 0x1 ) )

CUTE_TEST_CASE(boojum_btree_tests)
    struct test_ctx {
        const uintptr_t addr;
        const char *data;
        const size_t data_size;
    } test_vector[] = {
        { 0xDEADBEEF, "DEADBEEF", 8 },
        { 0x0000BEEF, "BEEF", 4 },
        { 0xAABBCCDD, "AABBCCDD", 8 },
    }, *test = &test_vector[0], *test_end = test + sizeof(test_vector) / sizeof(test_vector[0]), *tp;
    boojum_alloc_branch_ctx *root = NULL, *rp = NULL;
    size_t b;
    uintptr_t a;
    char *temp_data = NULL;
    size_t temp_data_size;

    // INFO(Rafael): Testing addition.

    while (test != test_end) {
        CUTE_ASSERT(boojum_add_addr(&root, test->addr) == EXIT_SUCCESS);
        test++;
    }

    CUTE_ASSERT(root != NULL);

    // INFO(Rafael): Ensuring that all bits from the added segment address
    //               created the right branches in the tree.

    test = &test_vector[0];

    while (test != test_end) {
        rp = root;
        for (b = 0; b < BOOJUM_BITSIZE; b++) {
            if (boojum_get_bitn(test->addr, b)) {
                rp = rp->r;
            } else {
                rp = rp->l;
            }
            CUTE_ASSERT(rp != NULL);
        }
        test++;
    }

    // INFO(Rafael): Ensuring that all set data will be erased at the source (data and size).

    test = &test_vector[0];

    CUTE_ASSERT(boojum_set_data(&root, 0x404, (void *)test_vector[0].data, &temp_data_size) == ENOENT);

    while (test != test_end) {
        temp_data_size = test->data_size;
        temp_data = (char *)malloc(temp_data_size);
        CUTE_ASSERT(temp_data != NULL);
        memcpy(temp_data, test->data, temp_data_size);
        CUTE_ASSERT(boojum_set_data(&root, test->addr, temp_data, &temp_data_size) == EXIT_SUCCESS);
        CUTE_ASSERT(temp_data_size == 0);
        for (b = 0; b < test->data_size; b++) {
            CUTE_ASSERT(temp_data[b] == 0);
        }
        free(temp_data);
        temp_data = NULL;
        test++;
    }

    CUTE_ASSERT(boojum_set_data(&root, 0x404, (void *)test_vector[0].data, &temp_data_size) == ENOENT);

    // INFO(Rafael): Ensuring that all data will be right got.

    test = &test_vector[0];

    CUTE_ASSERT(boojum_get_data(&root, 0x404, &temp_data_size) == NULL);

    while (test != test_end) {
        temp_data = boojum_get_data(&root, test->addr, &temp_data_size);
        CUTE_ASSERT(temp_data != NULL);
        CUTE_ASSERT(temp_data_size == test->data_size);
        CUTE_ASSERT(memcmp(temp_data, test->data, temp_data_size) == 0);
        memset(temp_data, 0, temp_data_size);
        free(temp_data);
        temp_data_size = 0;
        test++;
    }

    CUTE_ASSERT(boojum_get_data(&root, 0x404, &temp_data_size) == NULL);

    // INFO(Rafael): Testing deletion.

    test = &test_vector[0];

    CUTE_ASSERT(boojum_del_addr(&root, 0x404) == ENOENT);

    while (test != test_end) {
        CUTE_ASSERT(boojum_del_addr(&root, test->addr) == EXIT_SUCCESS);
        for (tp = test + 1; tp != test_end; tp++) {
            // INFO(Rafael): Ensuring that the last deletion did not screw up
            //               the remaining additions.
            rp  = root;
            for (b = 0; b < BOOJUM_BITSIZE; b++) {
                if (boojum_get_bitn(tp->addr, b)) {
                    rp = rp->r;
                } else {
                    rp = rp->l;
                }
                CUTE_ASSERT(rp != NULL);
            }
        }
        test++;
    }

    CUTE_ASSERT(root == NULL);

CUTE_TEST_CASE_END

#undef BOOJUM_BITSIZE

#undef boojum_get_bitn
