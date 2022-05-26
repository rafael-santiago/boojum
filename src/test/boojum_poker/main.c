/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <boojum.h>
#if defined(__unix__)
# include <unistd.h>
#elif defined(_WIN32)
# include <windows.h>
# include <process.h>
# define getpid _getpid
# if defined(_MSC_VER)
  typedef int pid_t;
# endif
#else
# error Some code wanted.
#endif
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int should_quit = 0;

static void sigint_watchdog(int signo);

int main(int argc, char **argv) {
    char *segment = NULL;
    size_t secret_size = 0;
    char secret[1<<10];
    pid_t ppid;
    int err = EXIT_FAILURE;
    int with_boojum = 0;

    if (argc == 1
        || (strcmp(argv[1], "--with-boojum") != 0
            && strcmp(argv[1], "--without-boojum") != 0)) {
        fprintf(stderr, "use: %s --with-boojum|--without-boojum\n", argv[0]);
        return EXIT_FAILURE;
    }

    signal(SIGINT, sigint_watchdog);
    signal(SIGTERM, sigint_watchdog);

    ppid = getpid();

    memset(secret, 0, sizeof(secret));
#if !defined(_MSC_VER)
    strncpy(secret, "This is my secret: ", sizeof(secret) - 1);
    sprintf(secret + strlen(secret), "%d", ppid);
    strcat(secret, ". Do not tell anyone, please.\n");
#else
    strncpy_s(secret,  sizeof(secret) - 1, "This is my secret: ", 19);
    sprintf_s(secret + strlen(secret), sizeof(secret) - strlen(secret) - 1, "%d", ppid);
    strncat_s(secret, sizeof(secret) - strlen(secret) - 1, ". Do not tell anyone, please.\n", 31);
#endif
    with_boojum = (strcmp(argv[1], "--with-boojum") == 0);

    if (!with_boojum) {
        err = EXIT_SUCCESS;
    } else {
        if ((err = boojum_init(1000)) != EXIT_SUCCESS) {
            fprintf(stderr, "error: unable to initialize boojum.\n");
            goto epilogue;
        }

        secret_size = strlen(secret);
        segment = boojum_alloc(secret_size);
        if (segment == NULL) {
            fprintf(stderr, "error: unable to allocate masked segment.\n");
            err = ENOMEM;
            goto epilogue;
        }

        err = boojum_set(segment, secret, &secret_size);
        if (err != EXIT_SUCCESS) {
            fprintf(stderr, "error: unable to set masked segment.\n");
            goto epilogue;
        }
    }

    fprintf(stdout, "pid: %d\n", ppid);
    fflush(stdout);

    while (!should_quit) {
#if defined(__unix__)
        usleep(1);
#elif defined(_WIN32)
        Sleep(1);
#else
# error Some code wanted.
#endif
    }

epilogue:

    if (with_boojum) {
        if (segment != NULL) {
            boojum_free(segment);
        }
        boojum_deinit();
    }

    fprintf(stdout, "\nsee ya!\n");

    return err;
}

static void sigint_watchdog(int signo) {
    should_quit = 1;
}
