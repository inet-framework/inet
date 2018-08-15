.. _ug:cha:authors-guide:

Appendix: Author’s Guide
========================

.. _ug:sec:authorsguide:overview:

Overview
--------

This chapter is intended for authors and contributors of this *INET
User’s Guide*, and covers the guidelines for deciding what type of
content is appropriate for this *Guide* and what is not.

The main guiding principle is to avoid redundancy and duplication of
information with other pieces of documentation, namely:

-  Standards documents (RFCs, IEEE specifications, etc.) that describe
   protocols that INET modules implement;

-  *INET Developer’s Guide*, which is intended as a guide for anyone
   wishing to understand or modify the operation of INET’s components at
   C++ level;

-  *INET Framework Reference*, directly generated from NED and MSG
   comments by OMNeT++ documentation generator;

-  Showcases, tutorials and simulation examples (``showcases/``,
   ``tutorials/`` and ``examples/`` folders in the INET project)

Why is duplication to be avoided? Multiple reasons:

-  It is a waste of our reader’s time they have to skip information they
   have already seen elsewhere

-  The text can easily get out of date as the INET Framework evolves

-  It is extra effort for maintainers to keep all copies up to date

.. _ug:sec:authorsguide:guidelines:

Guidelines
----------

.. _ug:sec:authorsguide:do-not-repeat-the-standard:

Do Not Repeat the Standard
~~~~~~~~~~~~~~~~~~~~~~~~~~

When describing a module that implements protocol X, do not go into
lengths explaining what protocol X does and how it works, because that
is appropriately (and usually, much better) explained in the
specification or books on protocol X. It is OK to summarize the
protocol’s goal and principles in short paragraph though.

In particular, do not describe the *format of the protocol messages*. It
surely looks nice and takes up a lot of space, but the same information
can probably be found in a myriad places all over the internet.

.. _ug:sec:authorsguide:do-not-repeat-ned:

Do Not Repeat NED
~~~~~~~~~~~~~~~~~

Things like module parameters, gate names, emitted signals and collected
statistics are appropriately and formally part of the NED definitions,
and there is no need to duplicate that information in this *Guide*.

Detailed information on the module, such as *usage details* and the list
of *implemented standards* should be covered in the module’s NED
documentation, not in this *Guide*.

.. _ug:sec:authorsguide:no-cplusplus:

No C++
~~~~~~

Any content which only makes sense on C++ level should go to the
*Developer’s Guide*, and has no place in this *Guide*.

.. _ug:sec:authorsguide:keep-examples-short:

Keep Examples Short
~~~~~~~~~~~~~~~~~~~

When giving examples about usage, keep them concise and to the point.
Giving ini or NED file fragments of a few lines length is preferable to
complete working examples.

Complete examples should be written up as showcases. A working
simulation without much commentary should go under ``examples``. A
practical, potentially multi-step guide to using a nontrivial feature
should be written up as a tutorial.

.. _ug:sec:authorsguide:no-reference-to-simulation-examples:

No Reference to Simulation Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Do not refer to concrete example simulations, showcases or tutorials in
the text, because they might get renamed, moved, merged or deleted, and
when they do, no one will think about updating the reference in the
*Users Guide*.

.. _ug:sec:authorsguide:what-then:

What then?
~~~~~~~~~~

Concentrate on giving a “big picture” of the models: what it is
generally capable of, how the parts fit together, etc. Give just enough
information that after a quick read, users can “bootstrap” into putting
together their own simulations with the model. If they have questions
afterwards, they will/should refer to the NED documentation (INET
Reference), or if that’s not enough, delve into the C++ code to find the
answers.
