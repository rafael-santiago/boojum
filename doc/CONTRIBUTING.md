# Boojum's contribution guide

First of all, thank you for being interested on contributing with this project and,
thank you again for taking some of your time by reading how to do it in the way we
like to do.

Anyway, there is no secret on contributing for this project, take a look:

1. Get a fork from the official [repo](https://github.com/rafael-santiago/boojum).
2. If you has not read the [coding style guides](https://github.com/rafael-santiago/boojum/blob/main/doc/Codingstyle.md),
   please, read it.
3. Implement your stuff or add your adjustment(s) or still fix(es).
4. Implement test(s) for your new stuff or regression test(s) for your adjustment(s) or fix(es).
5. If what you did requires new information, of course. Document what you did, please.
6. Make sure your tests are all passing and, the previous ones are still passing. The CI, too.
7. When commiting your changes, try to include those change in one single commit.
   Search on the internet about ``git rebasing/squashing``. Still, try to do it on your own,
   from your ``local git copy``, do not depend on ``git hosting services``, please.
8. Finally, open your pull request. Here a good tip is trying to explain, document, what your commit adds to the code base
   through the commit message, in this way, when opening the pull request you will only have to reformat a little the
   text field (related to the pull request's report) to make it more fancy in the web UI. It is a good practice
   because you will not tying the project lifecycle with a ``git hosting service``, that in the future can change.
   Remember, engineering. :wink: Moreover, commit things like "Oh! This is the pull request 12345. In order to know
   more about it just follow (the broken) link ![12345-pull-yyz-gama-alfa]" in the code base is sloppy and also kind
   of stupid. The project uses git to tidy up itself. When pushing people to have an internet link to get more specific
   information about the changing history of their copy, you just screwed up the main idea of git: provide a full
   functional and self-contained copy of the code base. At the end ``git`` becomes an old centralized version control
   system, in some sense... :anchor: :exploding_head: :flushed:
