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

- Standards documents (RFCs, IEEE specifications, etc.) that describe
  protocols that INET modules implement;

- *INET Developer’s Guide*, which is intended as a guide for anyone
  wishing to understand or modify the operation of INET’s components at
  the C++ level;

- *INET Framework Reference*, directly generated from NED and MSG
  comments by OMNeT++ documentation generator;

- Showcases, tutorials, and simulation examples (``showcases/``,
  ``tutorials/``, and ``examples/`` folders in the INET project)

The reasons to avoid duplication are as follows:

- It wastes readers' time if they have to skip information they
  have seen elsewhere.

- The text can easily become outdated as the INET Framework evolves.

- It requires extra effort for maintainers to keep all copies up to date.

.. _ug:sec:authorsguide:guidelines:

Guidelines
----------

.. _ug:sec:authorsguide:do-not-repeat-the-standard:

Do Not Repeat the Standard
~~~~~~~~~~~~~~~~~~~~~~~~~~

When describing a module that implements protocol X, avoid going into
lengthy explanations of how protocol X works or what it does since
this is usually better explained in the
specification or books on protocol X. It is permissible to summarize the
protocol’s goal and principles in a short paragraph instead.

In particular, avoid describing the *protocol message format.* Although it
may look nice and take up a lot of space, the same information
can probably be found in various places on the internet.

.. _ug:sec:authorsguide:do-not-repeat-ned:

Do Not Repeat NED
~~~~~~~~~~~~~~~~~

Module parameters, gate names, emitted signals, and collected
statistics are formally part of the NED definitions,
and there is no need to duplicate that information in this *Guide*.

Detailed information about the module, such as *usage details* and the list
of *implemented standards,* should be covered in the module’s NED
documentation, not in this *Guide*.

.. _ug:sec:authorsguide:no-cplusplus:

No C++
~~~~~~

Any content that only makes sense at the C++ level should be included in the
*Developer’s Guide* and should not be included in this *Guide*.

.. _ug:sec:authorsguide:keep-examples-short:

Keep Examples Short
~~~~~~~~~~~~~~~~~~~

When providing usage examples, keep them concise and to the point.
Providing ini or NED file fragments a few lines in length is preferable to
providing complete working examples.

Complete examples should be written up as showcases. A working
simulation without much commentary should be placed under ``examples``. A
practical, potentially multi-step guide to using a nontrivial feature
should be written up as a tutorial.

.. _ug:sec:authorsguide:no-reference-to-simulation-examples:

No Reference to Simulation Examples
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Avoid referring to specific example simulations, showcases, or tutorials in
the text because they might be renamed, moved, merged, or deleted. When this occurs, no one will consider updating the reference in the
*Users Guide*.

.. _ug:sec:authorsguide:what-then:

What then?
~~~~~~~~~~

Focus on providing a "big picture" of the models: what they are
generally capable of, how the parts fit together, etc. Provide enough
information that after a quick read, users can "bootstrap" themselves
into creating their own simulations with the model. If users have questions
afterwards, they can refer to the NED documentation (INET
Reference) or, if necessary, delve into the C++ code to find the
answers.