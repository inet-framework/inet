In-vehicle Network
==================

Goals
-----

In this example, we demonstrate the combined features of Time-Sensitive Networking
in a complex in-vehicle network. The network utilizes time-aware shaping, automatic
gate scheduling, clock drift, time synchronization, credit-based shaping, per-stream
filtering and policing, stream redundancy, unicast and multicast streams, link
failure protection, frame preemption, and cut-through switching.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/combiningfeatures/invehicle <https://github.com/inet-framework/inet/tree/master/showcases/tsn/combiningfeatures/invehicle>`__

The Model
---------

In this showcase, we model the communication network inside a vehicle. The network
consists of several Ethernet switches connected in a redundant way and multiple
end devices. There are several data flows between the end device applications.

Here is the network:

.. .. figure:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini

Standard Ethernet
-----------------

In this configuration, we use only standard Ethernet features to have a baseline
of statistical results.

Time-Sensitive Networking
-------------------------

In this configuration, we use advanced Time-Sensitive Networking features to
evaluate their performance.

Results
-------

.. The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

Here are the simulation results:

.. .. figure:: media/results.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`InVehicleNetworkShowcase.ned <../InVehicleNetworkShowcase.ned>`


Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/tsn/combiningfeatures/invehicle`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.4 --init -w inet-workspace --install --chdir \
       -c 'cd inet-4.4.*/showcases/tsn/combiningfeatures/invehicle && inet'

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

Use `this <https://github.com/inet-framework/inet/discussions/785>`__ page in the GitHub issue tracker for commenting on this showcase.

