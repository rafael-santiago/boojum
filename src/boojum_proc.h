#ifndef BOOJUM_BOOJUM_PROC_H
#define BOOJUM_BOOJUM_PROC_H 1

#include <boojum_types.h>

int boojum_init_mutex(boojum_mutex *mtx);

int boojum_init_thread(boojum_thread *thread);

int boojum_deinit_mutex(boojum_mutex *mtx);

int boojum_deinit_thread(boojum_thread *thread);

int boojum_mutex_lock(boojum_mutex *mtx);

int boojum_mutex_unlock(boojum_mutex *mtx);

#endif
