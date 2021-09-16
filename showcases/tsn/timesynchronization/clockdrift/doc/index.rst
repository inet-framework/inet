Clock Drift
===========

Goals
-----

In this example we demonstrate the effects of clock drift and time synchronization
on network behavior. We show how to introduce local clocks in network nodes, how
to configure clock drift for them, and how to use time synchronization mechanisms
to reduce time differences.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/timesynchronization/clockdrift <https://github.com/inet-framework/inet-showcases/tree/master/tsn/timesynchronization/clockdrift>`__

The Model
---------

Here is the network:

.. .. image:: media/Network.png
   :align: center
   :width: 100%

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :align: center
   :language: ini

No Clock Drift
--------------

In this configuration network nodes don't have clocks. Applications and gate
schedules are synchronized by simulation time.

Constant Clock Drift
--------------------

In this configuration all network nodes have a clock with a random constant drift.
Clocks drift away from each other over time.

Out-of-Band Synchronization of Clocks
-------------------------------------

In this configuration the network node clocks are periodically synchronized by an
out-of-band mechanism that doesn't use the underlying network.

Synchronizing Clocks using gPTP
-------------------------------

In this configuration the clocks in network nodes are periodically synchronized
to a master clock using the Generic Precision Time Protocol (gPTP). The time
synchronization protocol measures the delay of individual links and disseminates
the clock time of the master clock on the network through a spanning tree.

Results
-------

The following video shows the behavior in Qtenv:

.. video:: media/behavior.mp4
   :align: center
   :width: 90%

Here are the simulation results:

.. .. image:: media/results.png
   :align: center
   :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ClockDriftShowcase.ned <../ClockDriftShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

