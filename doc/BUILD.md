# Boojum's BUILD

``Boojum`` can be built by using [``Hefesto``](https://github.com/rafael-santiago/hefesto)
> "- Welcome to The Twilight Zone!" :)

But it simply works without calling you a bunch of dru... I meant dependencies!

After following all steps to be able to "invoke" ``Hefesto`` from your system, being into
``src`` sub-directories of your ``Boojum``'s repo copy, you just:

```
BichoPapaum@R00f:~/boojum/src# hefesto
```

The library will be built inside ``../lib`` and some tests will run.
After a successful build if you want to install it:

```
BichoPapaum@R00f:~/boojum/src# hefesto --install [--boojum-home=<path>]
```

For uninstalling:

```
BichoPapaum@R00f:~/boojum/src# hefesto --uninstall [--boojum-home=<path>]
```

Still, if your ``C`` compiler has support for ``C11`` you can build ``Boojum``
by using ``C11 threading`` conveniences as follows:

```
BichoPapaum@R00f:~/boojum/src# hefesto --boojum-with-c11
```

By default ``Boojum``'s build gives preference for ``GCC`` and when not found
``Clang`` will be tried. If you want to force the usage of a specific toolset:

```
BichoPapaum@R00f:~/boojum/src# hefesto --toolset=<gcc|clang>
```

On ``Windows`` you can use ``MSVC`` (2019), too:

```
BichoPapaum@R00f:~/boojum/src# hefesto --toolset=msvc
```

On ``MSVC`` if you want to a debug library:

```
BichoPapaum@R00f:~/boojum/src# hefesto --toolset=msvc --compile-model=debug
```

If you want to build the well-simple provided samples into ``src/samples``:

```
BichoPapaum@R00f:~/boojum/src# hefesto --mk-samples
```

Once built, you will get those samples in the ``bin`` top-level sub-directory.
