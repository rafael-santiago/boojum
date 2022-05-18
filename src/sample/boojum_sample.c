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
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define USE_TIMED_GET 1

int main(void) {
    int err = EXIT_FAILURE;
    char *secret = NULL;
    char *plain = NULL;
    size_t plain_size = 0;

    if ((err = boojum_init(1000)) != EXIT_SUCCESS) {
        fprintf(stderr, "error: unable to initialize boojum library.\n");
        return err;
    }

    if (secret != NULL) {
        if ((err = boojum_free(secret)) != EXIT_SUCCESS) {
            fprintf(stderr, "panic: unable to free secret segment!\n");
        }
        secret = NULL;
    }

    secret = boojum_alloc(4);
    if (secret == NULL) {
        err = ENOMEM;
        goto epilogue;
    }

    if ((plain = (char *)malloc(4)) == NULL) {
        err = ENOMEM;
        goto epilogue;
    }

    memcpy(plain, "1234", 4);
    plain_size = 4;

    if ((err = boojum_set(secret, plain, &plain_size)) != EXIT_SUCCESS) {
        fprintf(stderr, "error: unable to set boojum protected segment.\n");
        memset(plain, 0, plain_size);
        free(plain);
        plain_size = 0;
        goto epilogue;
    }

    // INFO(Rafael): Now plain is zeroed and plain_size is equals to zero.
    //               Let's only free plain buffer.
    free(plain);

#if defined(USE_TIMED_GET)
    plain = boojum_timed_get(secret, &plain_size, 1000);
#else
    plain = boojum_get(secret, &plain_size);
#endif
    if (plain == NULL) {
        fprintf(stderr, "error: unable to get plain data from protected segment.\n");
        err = ENOMEM;
        goto epilogue;
    }

    fprintf(stdout, "plain data: ");
    fwrite(plain, 1, plain_size, stdout);
    fprintf(stdout, "\n");

epilogue:

#if !defined(USE_TIMED_GET)
    if (plain != NULL) {
        memset(plain, 0, plain_size);
        free(plain);
        plain = NULL;
        plain_size = 0;
    }
#elif defined(__unix__)
    sleep(1);
#elif defined(_WIN32)
    Sleep(1000);
#else
# error Some code wanted.
#endif

    if (boojum_deinit() != EXIT_SUCCESS) {
        fprintf(stderr, "warnin: unabled to deinitialize boojum library.\n");
    }

    return err;
}
