#ifndef BOOJUM_BOOJUM_TYPES_H
#define BOOJUM_BOOJUM_TYPES_H 1

#if defined(__cplusplus)
extern "C" {
#endif

# if defined(BOOJUM_WITH_C11)
#  include <threads.h>
  typedef mtx_t boojum_mutex;
  typedef thrd_t boojum_thread;
# elif defined(__unix__)
#  include <pthread.h>
  typedef pthread_mutex_t boojum_mutex;
  typedef pthread_t boojum_thread;
# elif defined(_WIN32)
#  include <windows.h>
  typedef HANDLE boojum_mutex;
  typedef HANDLE boojum_thread;
# else
#  error Some code wanted.
# endif

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

typedef struct boojum_alloc_branch {
    // INFO(Rafael): This structure is the basic building block of the Boojum's allocation tree.
    //
    //               - refcount expresses the total of references to this current tree branch.
    //
    //               - l is the "zero" side of our allocation tree.
    //
    //               - r is the "one" side of our allocation tree.
    //
    //               - d is the data info about the allocation, when set it makes the current branch
    //                 a leaf, actually.
    size_t refcount;
    void *l;
    void *r;
    void *d;
}boojum_alloc_branch_ctx;

typedef struct boojum_alloc_leaf {
    // INFO(Rafael): This structure expresses the masked user's allocation on Boojum.
    //
    //               - m_size is the total size in bytes of the allocated data.
    //
    //               - u_size is the total of bytes used since the last set operation.
    //
    //               - m is the masked data xored with r.
    //
    //               - r is the random seed that we derive to mask m, its size is given by 3 * m_size.
    size_t m_size, u_size;
    void *m;
    void *r;
}boojum_alloc_leaf_ctx;

struct boojum_ctx {
    // INFO(Rafael): You can understand it as a kind of singleton of the Boojum executing into your system.
    //
    //               - giant_lock as the name says stops all execution for doing a thing.
    //
    //               - kupd is the thread of the xor masking updates.
    //
    //               - kupd_in_msecs is the time interval (in milliseconds) that the xor maskings will be updated.
    //
    //               - alloc_tree is the Boojum's allocation tree, it gathers all protected allocations from the user.
    //
    //               - kupd_enabled holds the boolean status of the xor masking updater thread, I meant it is turned on
    //                 or turned off.
    boojum_mutex giant_lock;
    boojum_thread kupd;
    size_t kupd_in_msecs;
    boojum_alloc_branch_ctx *alloc_tree;
# if defined(BOOJUM_WITH_C11)
    _Atomic(int) kupd_enabled;
# else
    int kupd_enabled;
# endif
};

struct boojum_data_wiper_ctx {
    // INFO(Rafael): This context gathers information for the data wiper work on and also
    //               for its self destruction. If you like Rick & Morty, understand boojum_data_wiper_ctx
    //               as a Mr. Meeseeks that you asked to wipe some data after some specific time interval.
    //
    //               - thread holds the reference for the current execution line into the system.
    //
    //               - time_to_vanish is about how much time (in milliseconds) this data wiper
    //                 must sleep before wiping data and destruct yourself.
    //
    //               - data points to the plain sensitive data that must be wiped.
    //
    //               - data_size expresses the size (in bytes) of the data pointed by data pointer.
    boojum_thread thread;
    boojum_mutex lock;
    size_t time_to_vanish;
    void *data;
    size_t *data_size;
# if defined(BOOJUM_WITH_C11)
    _Atomic(int) enabled;
# else
    int enabled;
# endif
};

struct boojum_kupd_ctx {
    // INFO(Rafael): This context gathers all information relevant to make key xor maskings updating job work on.
    //
    //               - thread holds the reference for the current execution line into the system.
    //
    //               - giant_lock is the same lock used by main library entry points that edits user's allocations.
    //                 btw until now Boojum counts only with "giant" lock.
    //
    //               - alloc_tree is the binary tree that expresses all masked memory segments allocated by the user.
    //
    //               - keys expiration time is the "time to vanish", the time that a mask lasts over the memory segment,
    //                 the original paper and Cryptography Engineering book suggests 1 second.
    //
    //               - enabled it indicates that the current masking update thread is really running.
    boojum_thread *thread;
    boojum_mutex *giant_lock;
    boojum_alloc_branch_ctx **alloc_tree;
    size_t keys_expiration_time;
# if defined(BOOJUM_WITH_C11)
    _Atomic(int) *enabled;
# else
    int *enabled;
# endif
};

extern struct boojum_ctx *gBoojumCtx;

#if defined(__cplusplus)
}
#endif

#endif
