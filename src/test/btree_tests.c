#include "btree_tests.h"
#include <boojum_btree.h>
#include <boojum.h>

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

    while (test != test_end) {
        CUTE_ASSERT(boojum_add_addr(&root, test->addr) == EXIT_SUCCESS);
        test++;
    }

    CUTE_ASSERT(root != NULL);

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

    test = &test_vector[0];

    while (test != test_end) {
        CUTE_ASSERT(boojum_del_addr(&root, test->addr) == EXIT_SUCCESS);
        for (tp = test + 1; tp != test_end; tp++) {
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
