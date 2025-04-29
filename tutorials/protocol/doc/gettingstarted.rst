Getting Started
===============

In this tutorial, we show you how to build protocol simulations in the INET
Framework. The tutorial contains a series of simulation models numbered from 1
through 7. The models are of increasing complexity -- they start from the
basics, and in each step, they introduce new INET features and concepts related
to network protocols.

In the tutorial, each step is a separate configuration in the same ``omnetpp.ini``
file. Steps build on each other; they extend the configuration of the previous
step by adding a few new lines. Consecutive steps mostly share the same network,
defined in NED.

This tutorial focuses on building a protocol stack from the ground up, starting
with simple application-to-application communication and gradually adding more
complex features such as packet transmission, queueing, error detection,
interpacket gaps, packet resending, and packet reordering.

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/tutorials/protocol`` folder in the `Project Explorer`. There, you can view
and edit the tutorial files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.5.*/tutorials/protocol && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
tutorial directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions>`__ in
the GitHub issue tracker for commenting on this tutorial.
