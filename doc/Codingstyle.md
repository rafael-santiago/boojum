# Quick coding style guide

**Excuses..err, Abstract**: Coding is a kind of craft that involves many cultural and idiomatic stuff. Due to it,
well as in many other segments, ``the Truth`` is such a ``big, biiiig winged unicorn``... Anyway, the following text
seeks to describe ``objectively`` the main features of ``my current Unicorn``.


# Topics

- [Basic formatting](#basic-formatting)
- [Header files](#header-files)
- [Implementation files](#implementation-files)
- [Naming things](#naming-things)
- [If..Else blocks](#if-else-blocks)
- [While blocks](#while-blocks)
- [Do..While blocks](#do-while-blocks)
- [Functions](#functions)

## Basic formatting

1. Here we are replacing tabs with spaces and a tab is equals to 4 spaces (some text editors understands 4
as 8 spaces, maybe due to a different base, the Truth...)

2. Control blocks have to be always embraced with ``{ ... }``.

3. Yes, ``80`` columns is a really short limit, try not to exceed ``120``!

## Header files

1. Headers files have to start with a copyright disclaimer (that you can get from ``boojum.h``, for example).

2. Avoid ``#pragma once``, please. Use standard include guards. We do not want to force users to update
their tool chain just for building our stuff, this is tech facism... Really! Good software projects must
be ready to compile anywhere it should. Try to use the less possible because less is quicker and less
headache prone. New features are awesome but it is a cutting point in practice: ``Boojum`` is ready to
use ``C11 threading``, it is compiling in many environments but it is not linking! Due to ``libc``
inconsistences. Some OSes are using newest compiler versions but their library are still old.
I am writing it in ``2022``!

3. Avoid implementing things directly into headers.

[``Back``](#topics)

## Implementation files

1. Implementation files have to start with a copyright disclaimer (that you can get from ``boojum.h``, for example).

2. Functions unused outside from the implementation file must be ``static``.

3. Even being ``static`` those functions must be prototyped at the beginning of the implementation file.

[``Back``](#topics)

## Naming things

1. Avoid prepend names with underscores (``________this_is_anti_pattern``).

2. Try to find the best balance between information and objectivity.

3. Be idiomatic to the knowledge field you are coding from not to design pattern book that you love.
   Here, the knowledge field is ``information security``, ``cryptography``, ``memory management`` and so on.

4. ``snake_case`` is the generic naming convention of choice here.

[``Back``](#topics)

## If..Else blocks

This is the adopted style:

```c
    if (abc == def) {
        do_this(def, ghi);
    } else {
        do_that(ghi);
    }
```

[``Back``](#topics)

## While blocks

This is the adopted style:

```c
    while (!yours_not_equals) {
        reformat_this();
    }
```

[``Back``](#topics)

## Do..While blocks

This is the adopted style:

```c
    do {
        reformat_this();
    } while (!yours_not_equals);
```

[``Back``](#topics)

## Functions

This is the adopted style:

```c
int do_something(const int i_know_but_better_to_const, const char *buf, const size_t buf_size) {
    // All variables declared here, so you can plan before going.
    return whatever;
}
```

[``Back``](#topics)
