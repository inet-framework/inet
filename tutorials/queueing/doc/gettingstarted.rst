Getting Started
===============

INET contains a queueing library which provides various components
such as traffic generators, queues, and traffic conditioners.
Elements of the library can be used to assemble queueing functionality
that can be
used at layer 2 and layer 3 of the protocol stack. With extra elements, the queueing library can also be used
to define custom application behavior without C++ programming.

Each step in this tutorial demonstrates one of the available queueing elements,
with a few more complex examples at the end.
Note that most of the available elements are demonstrated here, but not all
(for example, elements specific to DiffServ are omitted from here).
See the INET Reference for the complete list of elements.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in OMNeT++ and INET. If you aren't, you can check out
the TicToc Tutorial to get started with using OMNeT++.


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/tutorials/queueing`` folder in the `Project Explorer`. There, you can view
and edit the tutorial files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.2 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.2.*/tutorials/queueing && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.2
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.


Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-tutorials/issues/1>`__ in
the GitHub issue tracker for commenting on this tutorial.
