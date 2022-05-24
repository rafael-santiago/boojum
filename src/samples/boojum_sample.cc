/*
 * Copyright (c) 2022, Rafael Santiago
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */
#include <iostream>
#include <boojum.h>
#include <errno.h>
#include <string.h>

int main(void) {
    int err = EXIT_FAILURE;
    char *secret = nullptr;
    char *data = nullptr;
    size_t data_size = 0;
    constexpr size_t kMaskingUpdaterTimeout = 1000;

    if ((err = boojum_init(kMaskingUpdaterTimeout)) != EXIT_SUCCESS) {
        std::cerr << "error: unable to initialize Boojum." << std::endl;
        goto epilogue;
    }

    secret = reinterpret_cast<char *>(boojum_alloc(10));
    if (secret == nullptr) {
        std::cerr << "error: unable to get a masked memory segment." << std::endl;
        err = ENOMEM;
        goto epilogue;
    }

    data_size = 10;
    data = new char[data_size];
    memcpy(data, "123mudar*\0", data_size);

    if ((err = boojum_set(secret, data, &data_size)) != EXIT_SUCCESS) {
        std::cerr << "error: unable to load secret data into masked memory segment." << std::endl;
        goto epilogue;
    }

    std::cout << "info: sensitive data is stored in masked segment." << std::endl;

    data = reinterpret_cast<char *>(boojum_get(secret, &data_size));

    if (data == nullptr) {
        std::cerr << "error: unable to get the plain sensitive data." << std::endl;
        err = EFAULT;
        goto epilogue;
    }

    std::cout << "info: this is the plain sensitive data `" << data << "` shhhhhh! ;)" << std::endl;

    err = EXIT_SUCCESS;

epilogue:

    if (data != nullptr) {
        memset(data, 0, data_size);
        delete [] data;
    }

    if (secret != nullptr) {
        boojum_free(secret);
    }

    boojum_deinit();

    return err;
}
