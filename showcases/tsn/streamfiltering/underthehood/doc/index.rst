Peeking Under the Hood
======================

Goals
-----

This showcase demonstrates that the filtering and policing modules can work
outside the context of a network node. Doing so may facilitate assembling and
validating specific complex filtering and policing behaviors which can be difficult
to replicate in a complete network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/streamfiltering/underthehood <https://github.com/inet-framework/inet/tree/master/showcases/tsn/streamfiltering/underthehood>`__

The Model
---------

In this configuration we directly connect a per-stream filtering module to multiple
packet sources.

Here is the network:

.. figure:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Results
-------

Here are the results:

.. figure:: media/TrafficClass1.png
   :align: center

.. figure:: media/TrafficClass2.png
   :align: center

.. figure:: media/TrafficClass3.png
   :align: center

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. figure:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`PeekingUnderTheHoodShowcase.ned <../PeekingUnderTheHoodShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/streamfiltering/underthehood`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir
       -c 'cd inet-4.4/showcases/tsn/streamfiltering/underthehood && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace inet-4.4
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/796>`__ page in the GitHub issue tracker for commenting on this showcase.

