Getting Started
===============

In this tutorial, we show you how to build wireless simulations in the INET
Framework. The tutorial contains a series of simulation models numbered from 1
through 14. The models are of increasing complexity -- they start from the
basics, and in each step, they introduce new INET features and concepts related
to wireless communication networks.

In the tutorial, each step is a separate configuration in the same ``omnetpp.ini``
file. Steps build on each other; they extend the configuration of the previous
step by adding a few new lines. Consecutive steps mostly share the same network,
defined in NED.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in OMNeT++ and INET. If you aren't, you can check out
the TicToc Tutorial to get started with using OMNeT++.


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/tutorials/wireless`` folder in the `Project Explorer`. There, you can view
and edit the tutorial files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.5 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.5.*/tutorials/wireless && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.5
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.


Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/998>`__ in
the GitHub issue tracker for commenting on this tutorial.
