#ifndef BOOJUM_BOOJUM_PROC_H
#define BOOJUM_BOOJUM_PROC_H 1

#include <boojum_types.h>

#if defined(__cplusplus)
extern "C" {
#endif

// TODO(Rafael): When using C11 use standard abstractions instead of platform dependent codes.

int boojum_init_mutex(boojum_mutex *mtx);

int boojum_init_thread(boojum_thread *thread);

int boojum_deinit_mutex(boojum_mutex *mtx);

int boojum_deinit_thread(boojum_thread *thread);

int boojum_mutex_lock(boojum_mutex *mtx);

int boojum_mutex_unlock(boojum_mutex *mtx);

int boojum_thread_join(boojum_thread *thread);

int boojum_sched_data_wiping(void *data, size_t *data_size, const size_t ttv);

# if !defined(BOOJUM_WITH_C11)
  int boojum_run_kupd_job(boojum_thread *thread,
                          boojum_mutex *giant_lock,
                          boojum_alloc_branch_ctx **alloc_tree,
                          const size_t key_expiration_time,
                          int *enabled_flag);

  // INFO(Rafael): With C11 we have _Atomic() facilities.

  int boojum_get_flag(const int *flag, boojum_mutex *mtx);

  int boojum_set_flag(int *flag, const int value, boojum_mutex *mtx);
# else
  int boojum_run_kupd_job(boojum_thread *thread,
                          boojum_mutex *giant_lock,
                          boojum_alloc_branch_ctx **alloc_tree,
                          const size_t key_expiration_time,
                          _Atomic(int) *enabled_flag);
# endif // !defined(BOOJUM_WITH_C11)

#if defined(__cplusplus)
}
#endif

#endif
