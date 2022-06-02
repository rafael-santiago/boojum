/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include "boojum_tests.h"
#include <boojum.h>
#include <boojum_proc.h>
#if defined(_WIN32)
# include <windows.h>
# include <process.h>
# include <sys/stat.h>
# if defined(_MSC_VER)
  typedef int pid_t;
# endif
#elif defined(__unix__)
# include <sys/wait.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

static void print_segment_data(const void *segment, const size_t segment_size);

static FILE *boojum_poker(const int with_boojum);

static int find_secret(const pid_t proc_pid);

static int has_needed_tools_to_poke(void);

// WARN(Rafael): libcutest's memory leak detector have been disabled for all main boojum's entry points
//               because it depends on pthread conveniences and it by default leaks some resources even
//               when taking care of requesting it for do not leak nothing... Anyway, all btree tests
//               are being executed with memory leak detector turned on. In Boojum this is the main
//               spot where memory leak could be harmful.
//
//               WINAPI also leaks some resources related to multi-threading stuff.

CUTE_TEST_CASE(boojum_init_tests)
    int status = g_cute_leak_check;
    CUTE_ASSERT(boojum_init(0) == EINVAL);
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_deinit_tests)
    int status = g_cute_leak_check;
    CUTE_ASSERT(boojum_deinit() == EINVAL);
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_alloc_free_tests)
    int status = g_cute_leak_check;
    char *segment = NULL;
    g_cute_leak_check = 0;
    segment = boojum_alloc(1024);
    CUTE_ASSERT(segment == NULL);
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(1024);
    CUTE_ASSERT(segment != NULL);
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_set_get_tests)
    int status = g_cute_leak_check;
    char *segment = NULL;
    char *plain = NULL;
    size_t plain_size = 0;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(6);
    CUTE_ASSERT(segment != NULL);
    plain_size = 6;
    plain = (char *)malloc(plain_size);
    CUTE_ASSERT(plain != NULL);
    memcpy(plain, "foobar", plain_size);
    CUTE_ASSERT(boojum_set(segment, plain, &plain_size) == EXIT_SUCCESS);
    CUTE_ASSERT(plain_size == 0);
    CUTE_ASSERT(memcmp(plain, "foobar", 6) != 0);
    free(plain);
    CUTE_ASSERT(memcmp(segment, "foobar", 6) != 0);
    plain = boojum_get(segment, &plain_size);
    CUTE_ASSERT(plain != NULL);
    CUTE_ASSERT(plain_size == 6);
    CUTE_ASSERT(memcmp(plain, "foobar", 6) == 0);
    free(plain);
    // INFO(Rafael): Since it is a C library it is up to users free everything they have allocated.
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_alloc_realloc_free_tests)
    int status = g_cute_leak_check;
    char *old = NULL, *new = NULL;
    char *data = NULL;
    size_t data_size = 0;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    old = boojum_alloc(3);
    CUTE_ASSERT(old != NULL);
    CUTE_ASSERT(boojum_realloc(NULL, 6) == NULL);
    CUTE_ASSERT(boojum_realloc(old, 0) == NULL);
    new = boojum_realloc(old, 6);
    CUTE_ASSERT(new != NULL && new != old);
    CUTE_ASSERT(boojum_free(new) == EXIT_SUCCESS);
    data = (char *)malloc(3);
    CUTE_ASSERT(data != NULL);
    memcpy(data, "foo", 3);
    data_size = 3;
    old = boojum_alloc(3);
    CUTE_ASSERT(old != NULL);
    CUTE_ASSERT(boojum_set(old, data, &data_size) == EXIT_SUCCESS);
    free(data);
    new = boojum_realloc(old, 6);
    CUTE_ASSERT(new != NULL && new != old);
    data = boojum_get(new, &data_size);
    CUTE_ASSERT(data != NULL);
    CUTE_ASSERT(data_size == 3);
    CUTE_ASSERT(memcmp(data, "foo", 3) == 0);
    free(data);
    data = (char *)malloc(6);
    CUTE_ASSERT(data != NULL);
    memcpy(data, "foobar", 6);
    data_size = 6;
    CUTE_ASSERT(boojum_set(new, data, &data_size) == EXIT_SUCCESS);
    free(data);
    data = boojum_get(new, &data_size);
    CUTE_ASSERT(data != NULL);
    CUTE_ASSERT(data_size == 6);
    CUTE_ASSERT(memcmp(data, "foobar", 6) == 0);
    free(data);
    CUTE_ASSERT(boojum_free(new) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_set_timed_get_tests)
    char *data = NULL;
    size_t data_size = 0;
    char *segment = NULL;
    int status = g_cute_leak_check;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    segment = boojum_alloc(6);
    CUTE_ASSERT(segment != NULL);
    data = (char *)malloc(6);
    CUTE_ASSERT(data != NULL);
    data_size = 6;
    memcpy(data, "secret", data_size);
    CUTE_ASSERT(boojum_set(segment, data, &data_size) == EXIT_SUCCESS);
    CUTE_ASSERT(data_size == 0);
    CUTE_ASSERT(memcmp(data, "\x0\x0\x0\x0\x0\x0", 6) == 0);
    free(data);
    data = boojum_timed_get(segment, &data_size, 1000);
    CUTE_ASSERT(data_size == 6);
    CUTE_ASSERT(data != NULL);
    CUTE_ASSERT(memcmp(data, "secret", data_size) == 0);
#if defined(__unix__)
    sleep(30);
#elif defined(_WIN32)
    Sleep(30000);
#else
# error Some code wanted.
#endif
    CUTE_ASSERT(data_size == 0);
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_kupd_assurance_tests)
#define MAS_EH_CLARO "S.B.B.H.K.K! S.B.B.H.K.K! Aqui eh o Chapolin Colorado falando a Terra!"
    char *segment = NULL;
    char *data = NULL;
    size_t data_size;
    const size_t eavesdrop_attempts_nr = 50;
    int status = g_cute_leak_check;
    size_t e;
    g_cute_leak_check = 0;
    CUTE_ASSERT(boojum_init(1000) == EXIT_SUCCESS);
    data_size = strlen(MAS_EH_CLARO);
    data = (char *)malloc(data_size);
    CUTE_ASSERT(data != NULL);
    memcpy(data, MAS_EH_CLARO, data_size);
    segment = boojum_alloc(data_size);
    CUTE_ASSERT(segment != NULL);
    CUTE_ASSERT(boojum_set(segment, data, &data_size) == EXIT_SUCCESS);
    CUTE_ASSERT(data_size == 0);
    CUTE_ASSERT(data[0] == 0);
    data_size = strlen(MAS_EH_CLARO);
    fprintf(stdout, "Now eavesdropping masked segment... wait...");
    for (e = 0; e < eavesdrop_attempts_nr; e++) {
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // WARN(Rafael): In normal conditions Boojum users should never direct access protected data,       !!
        //               but here we need to do it. Since access data read/written by concurrent process    !!
        //               can cause undefined behaviors, race conditions etc, here we are going to hold the  !!
        //               mutex we internally use to synchronize the things out. In this way those local     !!
        //               readings cannot cause unstability inside the library.                              !!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        CUTE_ASSERT(boojum_mutex_lock(&gBoojumCtx->giant_lock) == EXIT_SUCCESS);
        CUTE_ASSERT(memcmp(segment, MAS_EH_CLARO, data_size) != 0);
        memcpy(data, segment, data_size);
        CUTE_ASSERT(boojum_mutex_unlock(&gBoojumCtx->giant_lock) == EXIT_SUCCESS);
#if defined(__unix__)
        sleep(2);
#elif defined(_WIN32)
        Sleep(2000);
#else
# error Some code wanted.
#endif
        CUTE_ASSERT(boojum_mutex_lock(&gBoojumCtx->giant_lock) == EXIT_SUCCESS);
        CUTE_ASSERT(memcmp(segment, data, data_size) != 0);
        CUTE_ASSERT(boojum_mutex_unlock(&gBoojumCtx->giant_lock) == EXIT_SUCCESS);
        fprintf(stdout, "\r                                                                                 "
                        "                            \r"
                        "[%2.f%% completed]", (((double)e + 1) / (double)eavesdrop_attempts_nr) * 100);
        fprintf(stdout, " Masked memory segment's last status: ");
        print_segment_data(data, 8);

    }
    fprintf(stdout, "\r                                                                                     "
                    "                               \r");
    free(data);
    data = NULL;
    CUTE_ASSERT(boojum_free(segment) == EXIT_SUCCESS);
    CUTE_ASSERT(boojum_deinit() == EXIT_SUCCESS);
    g_cute_leak_check = status;
#undef MAS_EH_CLARO
CUTE_TEST_CASE_END

CUTE_TEST_CASE(boojum_poke_tests)
#if defined(_WIN32)
    HANDLE proc_handle = NULL;
#endif
    FILE *proc = NULL;
    pid_t proc_pid = 0;
    char proc_out[1<<10], *p = NULL;
    if (!has_needed_tools_to_poke()) {
        fprintf(stdout, "WARN: Test skipped because your system does not have"
                        " the necessary tools to run this test.\n");
        return NULL;
    }
    proc = boojum_poker(0);
    CUTE_ASSERT(proc != NULL);
#if !defined(_WIN32)
    usleep(10);
#else
    Sleep(10);
#endif
    memset(proc_out, 0, sizeof(proc_out));
    fgets(proc_out, sizeof(proc_out), proc);
    p = strstr(proc_out, "pid: ");
    CUTE_ASSERT(p != NULL);
    proc_pid = atoi(p + 5);
    CUTE_ASSERT(find_secret(proc_pid) == 1);
    fclose(proc);
#if defined(_WIN32)
    proc_handle = OpenProcess(PROCESS_TERMINATE, FALSE, proc_pid);
    CUTE_ASSERT(proc_handle != NULL);
    CUTE_ASSERT(TerminateProcess(proc_handle, 0) != FALSE);
    CloseHandle(proc_handle);
#endif
    proc = boojum_poker(1);
    CUTE_ASSERT(proc != NULL);
#if !defined(_WIN32)
    usleep(10);
#else
    Sleep(10);
#endif
    memset(proc_out, 0, sizeof(proc_out));
    fgets(proc_out, sizeof(proc_out), proc);
    p = strstr(proc_out, "pid: ");
    CUTE_ASSERT(p != NULL);
    proc_pid = atoi(p + 5);
    CUTE_ASSERT(find_secret(proc_pid) == 0);
    fclose(proc);
#if defined(_WIN32)
    proc_handle = OpenProcess(PROCESS_TERMINATE, FALSE, proc_pid);
    CUTE_ASSERT(proc_handle != NULL);
    CUTE_ASSERT(TerminateProcess(proc_handle, 0) != FALSE);
    CloseHandle(proc_handle);
#endif
CUTE_TEST_CASE_END

static void print_segment_data(const void *segment, const size_t segment_size) {
    const unsigned char *sp = (const unsigned char *)segment;
    const unsigned char *sp_end = sp + segment_size;
    const char token[2] = { 0, ',' };
#if defined(_WIN32)
    fprintf(stdout, " 0x%p..%.2X = { ", segment, ((uintptr_t)segment & 0xFF) + segment_size);
#else
    fprintf(stdout, " %p..%.2lx = { ", segment, ((uintptr_t)segment & 0xFF) + segment_size);
#endif
    while (sp != sp_end) {
        fprintf(stdout, "%.2x%c ", *sp, token[(sp + 1) != sp_end]);
        sp++;
    }
    fprintf(stdout, "};");
}

static FILE *boojum_poker(const int with_boojum) {
    FILE *proc = NULL;
    char cmdline[1<<10];
#if defined(__unix__)
    snprintf(cmdline, sizeof(cmdline) - 1, "bin/boojum-poker %s &", (with_boojum) ? "--with-boojum" : "--without-boojum");
    proc = popen(cmdline, "r");
#elif defined(_WIN32)
# if !defined(_MSC_VER)
    snprintf(cmdline, sizeof(cmdline) - 1, "start /b bin\\boojum-poker.exe %s", (with_boojum) ? "--with-boojum" : "--without-boojum");
# else
    sprintf_s(cmdline, sizeof(cmdline) - 1, "start /b bin\\boojum-poker.exe %s", (with_boojum) ? "--with-boojum" : "--without-boojum");
# endif
# if !defined(_MSC_VER)
    proc = popen(cmdline, "r");
# else
    proc = _popen(cmdline, "r");
# endif
#else
# error Some code wanted.
#endif
    return proc;
}

static int find_secret(const pid_t proc_pid) {
    char cmdline[4<<10];
    char wanted_data[1<<10];
    int retval = -1;
#if defined(__unix__)
    snprintf(cmdline, sizeof(cmdline) - 1, "gcore %d >/dev/null 2>&1", proc_pid);
    if (system(cmdline) != 0) {
        return -1;
    }
    snprintf(wanted_data, sizeof(wanted_data) - 1, "This is my secret: %d. Do not tell anyone, please.", proc_pid);
    snprintf(cmdline, sizeof(cmdline) - 1, "grep \"%s\" core.%d >/dev/null 2>&1", wanted_data, proc_pid);
    retval = (system(cmdline) == 0);
    snprintf(cmdline, sizeof(cmdline) - 1, "core.%d", proc_pid);
    remove(cmdline);
#elif defined(_WIN32)
    struct stat st;
    remove(".boojum-poker.dmp");
# if !defined(_MSC_VER)
    snprintf(cmdline, sizeof(cmdline) - 1, "procdump -ma %d .boojum-poker.dmp >nul 2>&1", proc_pid);
# else
    sprintf_s(cmdline, sizeof(cmdline) - 1, "procdump -ma %d .boojum-poker.dmp >nul 2>&1", proc_pid);
# endif
    system(cmdline);
    if (stat(".boojum-poker.dmp", &st) != 0) {
        return -1;
    }
# if !defined(_MSC_VER)
    snprintf(wanted_data, sizeof(wanted_data) - 1, "This is my secret: %d. Do not tell anyone, please.", proc_pid);
    snprintf(cmdline, sizeof(cmdline) - 1, "findstr /c:\"%s\" .boojum-poker.dmp >nul 2>&1", wanted_data);
# else
    sprintf_s(wanted_data, sizeof(wanted_data) - 1, "This is my secret: %d. Do not tell anyone, please.", proc_pid);
    sprintf_s(cmdline, sizeof(cmdline) - 1, "findstr /c:\"%s\" .boojum-poker.dmp >nul 2>&1", wanted_data);
#endif
    retval = (system(cmdline) == 0);
    remove(".boojum-poker.dmp");
#else
# error Some code wanted.
#endif
    return retval;
}

static int has_needed_tools_to_poke(void) {
    int has = 0;
#if defined(__unix__)
    has = (WEXITSTATUS(system("grep --version >/dev/null 2>&1")) == 0) &&
          (WEXITSTATUS(system("man -f gcore >/dev/null 2>&1")) == 0);
#elif defined(_WIN32)
    has = (system("findstr /? >nul 2>&1") == 0) &&
          (system("procdump -? -e >nul 2>&1") == -1);
#else
# error Some code wanted.
#endif
    return has;
}
