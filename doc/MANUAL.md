# Boojum user's guide

**Abstract**: This document aims to provide the main guidance of how to integrate ``Boojum`` memory masking protection
into your previous stuff. Any unclear or even untreated topic you can report [here](https://github.com/rafael-santiago/boojum/issues),
please and thank you!

## Topics

- [Has ``libboojum`` really been implementing the original ``Boojum`` idea?](#has-libboojum-really-been-implementing-the-original-boojum-idea)
- [How to integrate boojum into your stuff](#how-to-integrate-boojum-into-your-stuff)
- [Protecting your sensitive data with Boojum](#protecting-your-sensitive-data-with-boojum)
- [How slow by accessing masked memory by ``Boojum`` could be?](#how-slow-by-accessing-masked-memory-by-boojum-could-be)

## Has ``libboojum`` really been implementing the original ``Boojum`` idea?

In general it has. The only difference is that here it is being used ``HKDF``
to derive the masking key instead of using directly a general ``Hash`` function.

By doing it we are able to protect keys greater than the ``Hash`` function output
more easily by using a standard/well-known way. It becomes a more general solution.
Anyway, it is not a good idea to store pretty long sensitive data because it can
consume a bunch of memory.

For ``HKDF``'s hash function ``SHA-3/512`` has been taken.

[Back](#topics)

## How to integrate boojum into your stuff

``Boojum`` is about a ``C library``, so the workflow is not new and also pretty
strainghtforward. Let's see...

It must be linked against your application by using ``-lboojum`` or ``libboojum.a``
if you are using ``GCC`` or ``Clang``. On ``VisualStudio`` you must indicate
``libboojummt.lib`` or ``libboojummtd.lib`` depending on your chose build
configuration.

As a matter of best practice you should pass the ``Boojum``'s headers directory
to your compiler.

On ``Unix`` ``libboojum`` depends on ``pthreads`` and on ``Windows`` depends on
``bcrypt.lib``.

``Boojum`` also depends on another library of mine: [``kryptos``](https://github.com/rafael-santiago/kryptos).
On ``GCC`` or ``Clang`` you need to link with ``-lkryptos`` on ``MSVC`` with
``libkryptosmt.lib`` or ``libkryptosmtd.lib``.

Check on **Table 1** to get a summary.

**Table 1**: Required compiler flags to link your ``Boojum`` client application.

|    **Compiler**        |  **Platform**   |              **Linker flags**                               |
|:----------------------:|:---------------:|------------------------------------------------------------:|
|   ``GCC``              |   ``Unix-like`` | ``-lboojum``, ``-lkryptos``, ``-lpthread``                  |
|  ``Clang``             |   ``Unix-like`` | ``-lboojum``, ``-lkryptos``, ``-lpthread``                  |
|   ``MinGW``            |   ``Windows``   | ``-lboojum``, ``-lkryptos``, ``-lbcrypt``                   |
|  ``Clang``             |   ``Windows``   | ``-lboojum``, ``-lkryptos``, ``-lbcrypt``                   |
|  ``MSVC``  *(Release)* |   ``Windows``   | ``libboojummt.lib``, ``libkryptosmt.lib``, ``bcrypt.lib``   |
|  ``MSVC``  *(Debug)*   |   ``Windows``   | ``libboojummtd.lib``, ``libkryptosmtd.lib``, ``bcrypt.lib`` |

``Boojum`` supports ``C11`` threading conveniences. If your compiler and ``libc``
ships all those ``C11`` stuff correctly you can build the library by using those
conveniences. Once built when compiling your code, you must define ``BOOJUM_WITH_C11``
macro. Besides, of course, enable ``C11`` capabilities of your compiler.

[Back](#topics)

## Protecting your sensitive data with Boojum

``Boojum`` exposes some memory functions that you need to replace when dealing
with areas that must be protected against data leaking.

The first is ``boojum_alloc()``, it will allocate a memory segment that will be
watched by ``Boojum``. The ``boojum_alloc()`` function works as a commom call
to ``malloc()``:

```c
    void *boojum_alloc(const size_t ssize);
```

If for some reason you need to reallocate the sensitive buffer
you must use ``boojum_realloc()``. It works as the same way
of ``realloc()``:

```c
    void *boojum_realloc(void *ptr, const size_t ssize);
```

Any segment allocated by ``boojum_alloc()`` or ``boojum_realloc()`` must be
freed by ``boojum_free()``:

```c
    int boojum_free(void *ptr);
```

Once allocated, the memory segment **must not** be set directly, you should use
the function ``boojum_set()``. This function takes three paremeters:

- The first one is about the protect memory segment address that will
  receive the sensitive data.

- The second is a buffer to the plain buffer that will be wiped once
  its content loaded into the protected memory segment.

- The third is about a pointer indicating the size in bytes of the
  plain buffer. On a successful return the value pointed is zeroed.

Take a look at its prototype:

```c
    int boojum_set(void *ptr, void *data, size_t *data_size);
```

Now that you know how to allocate, set and free protected segments "maybe"
you need to know how to retrieve the plain data to do your stuff. It
is done by calling the function ``boojum_get()``. This function returns
a void pointer and it takes two arguments:

- The first one is about the protected memory segment previously
  loaded with sensitive data.

- The second is a ``size_t`` pointer that on a successful return
  indicates the size in bytes of the returned void pointer.

**Remark**: The returned **buffer is loaded with plain data** and it should
be freed **as soon as possible**. It is up to you ensure this data wiping.

Take a look at its prototype:

```c
    void *boojum_get(const void *ptr, size_t *data_size);
```

If you are good at forgetting doing stuff, maybe ``boojum_timed_get()``
is the best way of retrieving plain stuff from ``Boojum``. It takes the
same parameters of ``boojum_get()`` and one more. This additional parameter
is the ``ttv`` (``time to vanish``, in milliseconds). After ``ttv`` secs
the returned buffer and the variable indicating its size will be zeroed.
The buffer will also be freed. You can understand it as a kind of paranoid
``gc``. Notice that after ``ttv`` milliseconds if you access the returned
pointer you will cause invalid pointer access exceptions on your program.

Take a look at its prototype:

```c
    void *boojum_timed_get(const void *ptr, size_t *data_size, const size_t ttv);
```

The ``Boojum`` library only can work after a successful ``boojum_init()`` call.
This call only takes one parameter that indicates the time interval in milliseconds
that the masking keys will be updated. The time interval should not be so
large since the update is applied in order to mitigate cold boot attacks.

Follows its prototype:

```c
    int boojum_init(const size_t kupd_timeout_in_msecs);
```

Once initialized before exiting the program as best practice ``Boojum`` should
be deinitialized by calling ``boojum_deinit()``.

This is the prototype:

```c
    int boojum_deinit(void);
```

**Rule of thumb**: All functions that returns ``int`` on success returns ``EXIT_SUCCESS``.
On failure it will return ``EXIT_FAILURE`` or the exact error from ``errno.h``.

Now, follows everything we have talking about as code:

```c
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
```

You can find the sample presented above into  ``src/sample``.

[Back](#topics)

## How slow by accessing masked memory by ``Boojum`` could be?

The access to masked memory is constant time, being its Big-O about *O(``w``)* [sic... :P].
Here ``w`` denotes the cpu word size. Thus, if you are on a ``32-bit`` cpu it will be
about *O(32)*, on a ``64-bit`` cpu it will be *O(64)*. Why?

It is because ``Boojum`` "unwraps" the bits from the value representing
the memory segment in a binary tree layout. This care ensures an constant
time access to all protected segments. This tree is internally called
``allocation tree``.

You can also draw a conclusion that the maximum allocation tree height is equals to
the cpu word size. Enough! This is the last nerdy ``compsci`` remark that you will
see here....

Depending on your resources (memory and cpu) it can consume a bunch of memory (and
cpu, too). Due to it you should only protect small pieces of information and also for
short periods of time. The cpu consume is about the ``Boojum``'s thread responsible
for updating masking keys of the protected segments from time to time. More you
protect, more cpu time you spend by updating the protection layers over the
protected data.

[Back](#topics)
