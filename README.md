![Ilustration by Henry Holiday (1839-1927) / Public Domain](https://github.com/rafael-santiago/boojum/blob/main/etc/boojum_by_henry_holiday.png "Ilustration by Henry Holiday (1839-1927) / Public Domain")

## What does Boojum is? [![CircleCI](https://circleci.com/gh/rafael-santiago/boojum/tree/main.svg?style=shield)](https://circleci.com/gh/rafael-santiago/boojum/tree/main)

Good question but here ``"Boojum"`` is my implementation of an ideia for data oblivion
discussed by *Ferguson, Schneier and Kohno* in their book *"Cryptography Engineering"*.

There they called *"Boojum"* their proposal for sensitive data retention issues mitigation.
By referring to *Lewis Carroll*'s character. In short, you know that maybe something is
there but you cannot see it (sorry for the spoiler, kids!)

In the book, the base of the presented discussion is given by the paper from *2002*
["How to Forget a Secret"](https://link.springer.com/chapter/10.1007/3-540-49116-3_47)
by *Crescenzo, Ferguson, Impagliazzo and Jakobsson*.

It also mitigates cold boot attacks, by the way, if you are worried about
this kind of data leaking, cold boot attacks would be the most harmful problem
that sensitive data retention in ``RAM`` can bring up you. If you are still skeptic
about you can get some tips of this kind of issue in the paper
["Lest We Remember: Cold Boot Attacks on Encryption Keys"](http://citpsite.s3.amazonaws.com/wp-content/uploads/2019/01/23195456/halderman.pdf)
from 2008 by *Halderman, Schoen, Heninger, Clarkson, Paul, Calandrino, Feldman, Appelbaum and Felen*.

Here as a ``C library``, ``Boojum`` can be used when you need to retain sensitive
data in memory. Yes, even for short periods of time, if it is sensitive and you
are seeking to deliver best security practices from your stuff, you should care
about attacks during this short retation time, too.

In general, I have been implemented it as it was originally described, having
only few differences that you can know more about by reading the tech docs.

## Supported platforms

I have been using it on ``FreeBSD``, ``NetBSD``, ``OpenBSD``, ``Windows`` and also ``Linux``.

## How can I clone this repo?

The easiest way is:

```
BichoPapaum@R00f# git clone https://github.com/rafael-santiago/boojum --recursive
BichoPapaum@R00f# cd boojum/src
BichoPapaum@Roof:~/boojum/src# _
```

Congrats! Now you should read:

- [doc/BUILD.md](https://github.com/rafael-santiago/boojum/blob/main/doc/BUILD.md)
- [doc/MANUAL.md](https://github.com/rafael-santiago/boojum/blob/main/doc/MANUAL.md)

*Enjoy!
Rafael, May 2022.*
