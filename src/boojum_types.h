#ifndef BOOJUM_BOOJUM_TYPES_H
#define BOOJUM_BOOJUM_TYPES_H 1

#if defined(__unix__)
# include <pthread.h>
 typedef pthread_mutex_t boojum_mutex;
 typedef pthread_t boojum_thread;
#else
# error Some code wanted.
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

typedef struct boojum_alloc_branch {
    size_t refcount;
    void *l;
    void *r;
    void *d;
}boojum_alloc_branch_ctx;

typedef struct boojum_alloc_leaf {
    size_t m_size;
    void *m;
    void *r;
}boojum_alloc_leaf_ctx;

struct boojum_ctx {
    boojum_mutex giant_lock;
    boojum_thread kupd;
    size_t kupd_in_msecs;
    boojum_alloc_branch_ctx *alloc_tree;
    int kupd_enabled; // TODO(Rafael): If detected C11 use atomic.
};

struct boojum_data_wiper_ctx {
    boojum_thread thread;
    size_t time_to_vanish;
    void *data;
    size_t data_size;
};

extern struct boojum_ctx *gBoojumCtx;

#endif
