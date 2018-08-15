.. _dg:cha:authors-guide:

Appendix: Author’s Guide
========================

.. _dg:sec:authorsguide:overview:

Overview
--------

This chapter is intended for authors and contributors of this *INET
Developer’s Guide*, and covers the guidelines for deciding what type of
content is appropriate for this *Guide* and what is not.

The main guiding principle is to avoid redundancy and duplication of
information with other pieces of documentation, namely:

-  Standards documents (RFCs, IEEE specifications, etc.) that describe
   protocols that INET modules implement;

-  *INET User’s Guide*, which is intended for users who are interested
   in assembling simulations using the components provided by INET;

-  *INET Framework Reference*, directly generated from NED and MSG
   comments by OMNeT++ documentation generator;

-  Showcases, tutorials and simulation examples (``showcases/``,
   ``tutorials/`` and ``examples/`` folders in the INET project)

Why is duplication to be avoided? Multiple reasons:

-  It is a waste of our reader’s time they have to skip information they
   have already seen elsewhere

-  The text can easily get out of date as the INET Framework evolves

-  It is extra effort for maintainers to keep all copies up to date

.. _dg:sec:authorsguide:guidelines:

Guidelines
----------

.. _dg:sec:authorsguide:do-not-repeat-standard:

Do Not Repeat the Standard
~~~~~~~~~~~~~~~~~~~~~~~~~~

When describing a module that implements protocol X, do not go into
lengths explaining what protocol X does and how it works, because that
is appropriately (and usually, much better) explained in the
specification or books on protocol X. It is OK to summarize the
protocol’s goal and principles in a short paragraph though.

In particular, do not describe the *format of the protocol messages*. It
surely looks nice and takes up a lot of space, but the same information
can probably be found in a myriad places all over the internet.

.. _dg:sec:authorsguide:do-not-repeat-neddoc:

Do Not Repeat NED Documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Things like module parameters, gate names, emitted signals and collected
statistics are appropriately and formally part of the NED definitions,
and there is no need to duplicate that information in this *Guide*.

Detailed information on the module, such as *usage details* and the list
of *implemented standards* should be covered in the module’s NED
documentation, not in this *Guide*.

.. _dg:sec:authorsguide:do-not-repeat-cpp:

Do Not Repeat C++ Documentation
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Describing every minute detail of C++ classes, methods, arguments are
expected to be appropriately present in their *doxygen* documentation.

.. _dg:sec:authorsguide:what-then:

What then?
~~~~~~~~~~

Concentrate on giving a “big picture” of the implementation: what it is
generally capable of, how the parts fit together, how to use the
provided APIs, what were the main design decisions, etc. Give simple yet
meaningful examples and just enough information about the API that after
a quick read, users can “bootstrap” into implementing their own
protocols and applications. If they have questions afterwards, they
will/should refer to the C++ documentation.
