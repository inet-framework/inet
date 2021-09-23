Using gPTP
==========

Goals
-----

In this example we demonstrate how to configure gPTP (Generic Precision Time
Protocol, IEEE 802.1 AS) master clocks, bridges, and end stations to achieve
reliable time synchronization throughout the whole network.

| INET version: ``4.4``
| Source files location: `inet/showcases/tsn/timesynhronization/gptp <https://github.com/inet-framework/inet-showcases/tree/master/tsn/timesynchronization/gptp>`__

The Model
---------

Here is the ``General`` configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :end-before: OneMasterClock

One Master Clock
----------------

In this configuration the network topology is a simple tree. The network contains
one master clock, one bridge and two end stations:

.. figure:: media/OneMasterClockNetwork.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: OneMasterClock
   :end-before: PrimaryAndHotStandbyMasterClocks

Here are the results for one master clock:

.. figure:: media/OneMasterClock.png
   :align: center

Primary and Hot-Standby Master Clocks
-------------------------------------

In this configuration the tree network topology is further extended. The network
contains one primary master clock and one hot-standby master clock. Both master
clocks have their own time synchronization domain and they do their synchronization
separately. The only connection between the two is in the hot-standby master clock
which is also synchronized to the primary master clock. This connection effectively
causes the two time domains to be totally synchronized and allows seamless failover
in the case of the master clock failure.

Here is the network:

.. figure:: media/PrimaryAndHotStandbyNetwork.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: PrimaryAndHotStandbyMasterClocks
   :end-before: TwoMasterClocksExploitingNetworkRedundancy

Here are the results for the primary and hot standby clocks:

.. figure:: media/PrimaryAndHotStandby2.svg
   :align: center

And here are the time domains of the primary and hot standby clocks:

.. figure:: media/TimeDomainsPrimaryAndHotStandby2.svg
   :align: center

Two Master Clocks Exploiting Network Redundancy
-----------------------------------------------

In this configuration the network topology is a ring. Each of the primary master
clock and the hot-standby master clock has two separate time domains. One time
domain uses the clockwise and another one uses the counter-clockwise direction
in the ring topology to disseminate the clock time in the network. This approach
provides protection against a single link failure in the ring topology because
all bridges can be reached in both directions by one of the time synchronization
domains of both master clocks.

Here is the network:

.. figure:: media/TwoMasterClocksNetwork.png
   :align: center

Here is the configuration:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: TwoMasterClocksExploitingNetworkRedundancy

Here are the results for two master clocks:

.. figure:: media/TwoMasterClocks.svg
   :align: center
   :width: 100%

And here are the time domains of the two master clocks:

.. figure:: media/TimeDomainsTwoMasterClocks.svg
   :align: center
   :width: 100%

.. Results
   -------

   The following video shows the behavior in Qtenv:

   .. video:: media/behavior.mp4
      :align: center
      :width: 90%

   Here are the simulation results:

   .. .. image:: media/results.png
      :align: center
      :width: 100%


Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`GptpShowcase.ned <../GptpShowcase.ned>`

Discussion
----------

Use `this <https://github.com/inet-framework/inet/discussions/TODO>`__ page in the GitHub issue tracker for commenting on this showcase.

