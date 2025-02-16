Getting Started
===============

In INET simulations, configurator modules are commonly used for
assigning IP addresses to network nodes and for setting up their routing
tables. There are various configurators modules in INET; this tutorial
covers the most generic and most featureful one,
:ned:`Ipv4NetworkConfigurator`. :ned:`Ipv4NetworkConfigurator` supports
automatic and manual network configuration, and their combinations. By
default, the configuration is fully automatic. The user can also specify
parts (or all) of the configuration manually, and the rest will be
configured automatically by the configurator. The configurator's various
features can be turned on and off with parameters. The details of the
configuration, such as IP addresses and routes, can be specified in an
XML file.

This is an advanced tutorial, and it assumes that you are familiar with creating
and running simulations in OMNeT++ and INET. If you aren't, you can check out
the TicToc Tutorial to get started with using OMNeT++.


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/tutorials/configurator`` folder in the `Project Explorer`. There, you can view
and edit the tutorial files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.0 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.0.*/tutorials/configurator && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.0
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.


Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/999>`__ in
the GitHub issue tracker for commenting on this tutorial.
