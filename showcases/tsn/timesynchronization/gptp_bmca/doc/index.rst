gPTP: Best Master Clock Algorithm
=================================

Overview
~~~~~~~~

The Best Master Clock Algorithm (BMCA) is a fundamental component of gPTP protocol (IEEE 802.1AS).
Most of it's definition, however is defined in the Precision Time Protocol (PTP, see IEEE 1588).
It dynamically selects the most suitable master clock in a distributed network to ensure accurate synchronization 
without manual intervention.

gPTP nodes continously transmit Announce messages to their neighboring nodes,
transmitting information about it's current master clock (e.g. user-defined priorities, clock quality paramaters, ...).
BMCA continuously evaluates available clocks based on these Announce messages, ensuring that the highest-quality
clock is chosen as the Grandmaster Clock. If the current master clock becomes unavailable, BMCA re-evaluates
the network after a timeout and selects the next best candidate to maintain synchronization.
All these parmeters can be defined in the :ned:`Gptp` module.

The Model
~~~~~~~~~

In this showcase, we provide three BMCA network scenarios in the showcase: BmcaShowcaseSimple, BmcaPrioChange, BmcaDiamond and BmcaDiamondAsymmetric.

- **BMCA Simple Network**: A basic setup with a single switch connecting three devices. The best master clock is selected dynamically based on BMCA rules, 
  ensuring time synchronization across the network.
- **BMCA with Priority Change**: Similar to the simple setup but introduces dynamic priority changes. Devices with different priorities compete for the best master clock role, 
  showcasing BMCA’s adaptability.
- **BMCA Diamond Topology**: A redundant diamond-shaped network where devices are connected through two switches. The best master clock is determined dynamically, 
  ensuring synchronization even if a switch or link fails.
- **BMCA Asymmetric Diamond**: An asymmetric variation of the diamond topology, introducing different link configurations and priorities. 
  Demonstrates BMCA’s ability to handle uneven network structures while maintaining synchronization.

.. TODO: .. TODO: Consider updating the notation for `gptp_hotstandby` if needed.

In the ``General`` configuration, which has a similar setup to the :doc:`tsn/timesynchronization/gptp/doc/index` showcase,
we enable :ned:`Gptp` modules in all network nodes and configure a random clock drift rate for all clocks.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable time synchronization
   :end-at: driftRate

Each simulation is detailed in the following sections.

BMCA Simple Network
~~~~~~~~~~~~~~~~~~~

In the BMCA Simple Network, we have a basic setup with a single switch connecting three devices. The best master clock is selected dynamically based on 
BMCA rules, ensuring time synchronization across the network.

.. figure:: media/BmcaShowcaseSimple.png
   :align: center

Our goal is to demonstrate BMCA's ability to select the best master clock in a simple network.
We configure the devices with different priorities.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnDevice1.gptp.grandmasterPriority1 = 2
   :end-at: .tsnDevice3.gptp.grandmasterPriority1 = 3

In this setup:

- `TsnDevice1` has the highest priority with a value of 2
- `TsnDevice3` follows with a priority of 3
- `TsnDevice2` has the lowest priority, with the default value of 255

.. note:: Lower priority values indicate higher priority in BMCA.

For our scenario, we setup a link failure between the intended GM (`tsnDevice1`) and the `tsnSwitch`:

.. literalinclude:: ../simple-link-failure.xml
   :language: xml

The following configuration defines the grandmaster priority settings:

The BMCA algorithm will evaluate these priorities and select the best master clock based on the defined criteria.
The below chart shows the clock synchronization behavior throughout the simulation:

.. figure:: media/BMCA_zoomed.png
   :align: center

The chart shows the clock time differences between devices over the simulation period.
We can observe the following:

- All devices maintain good synchronization with a gradual upward drift until 7.5s
- At 7.5s ``tsnDevice1`` is disconnected.
- The :par:`announceTimeout` (typically 3 times the announce interval and thus 3s) is emitted on the other clocks due
  to missing Announce messages from the GM.
- At around 10s the new GM (``tsnDevice3``) is selected and after two Sync messages, the other devices start to follow
  the new GM.
- At 14.5s the link between ``tsnDevice1`` and ``tsnSwitch`` is restored.
- A new Announce message is sent by ``tsnDevice1`` at approx. 15s and the other devices start to follow it again after
  two Sync messages.

The chart effectively demonstrates how the grandmaster clock role shifts between devices during link failures,
and how the synchronization is maintained among the devices in the operational part of the network.

BMCA with Priority Change
-------------------------

Using the same network, we now use a different scenario.
Instead of cutting the link to the GM, we will change the priority of ``tsnDevice3`` to become the intended GM at 10s:

.. literalinclude:: ../simple-priority-change.xml
   :language: xml

After this priority change and the consecutive Announce message, the other devices will select
``tsnDevice3`` as the new GM:

.. figure:: media/BMCA_PrioChange_zoomed.png
   :align: center


BMCA Diamond Topology
~~~~~~~~~~~~~~~~~~~~~

In the BMCA Diamond Topology, we create a redundant network with multiple paths between two devices. This configuration demonstrates BMCA's ability to 
handle network failures while maintaining synchronization.

The network consists of two TSN devices (``tsnDevice1`` and ``tsnDevice2``) connected through two switches (``tsnSwitchA1`` and ``tsnSwitchB1``). Each device
has a gPTP module for clock synchronization.

.. figure:: media/BmcaShowcaseDiamond.png
   :align: center

Both devices are connected to both switches, forming a diamond-shaped topology that provides path redundancy. In this setup, ``tsnDevice1`` is configured
as the grandmaster with priority 1. All other values use the default values.

The link failure settings are configured in the diamond-link-failure.xml file:

.. literalinclude:: ../diamond-link-failure.xml
   :language: xml

Between 1.5s and 6.5s, the link between ``tsnDevice1`` and ``tsnSwitchB1`` is cut.
Between 10.5s and 15.5s, the link between ``tsnDevice2`` and ``tsnSwitchB1`` is cut.

.. figure:: media/BMCA_Diamond_zoomed.png
   :align: center

As the figure shows, ``tsnDevice1`` remains the GM throughout the simulation.

Further, we can see, that in the case of the first link failure, ``tsnDevice2`` (orange) and ``tsnSwitchB1`` (red)
start to drift away for 3s (announceTimeout) before the BMCA algorithm starts to establish the new
synchronization tree via ``tsnSwitchA1``.

In the second link failure case, we can see that only ``tsnSwitchB1`` (red) starts to drift away.
This indicates that in the fully operational network
the spanning tree is established via ``tsnSwitchB1`` to ``tsnDevice2``.

Finally, the figure shows that upon re-establishing the link between ``tsnDevice1`` and ``tsnSwitchB1``
in the first failure case, there is no out-of-sync time and the network transitions smoothly
to the new topology.


BMCA Asymmetric Diamond
~~~~~~~~~~~~~~~~~~~~~~~

The BMCA Asymmetric Diamond topology extends the diamond configuration by introducing an asymmetric structure
with different path lengths.
This setup is used to test if the BMCA implementation does select the shorter synchronization path in case of
multiple redundant paths.

The network structure is provided in the following figure:

.. figure:: media/BmcaShowcaseDiamondAsymmetric.png
   :align: center

In this asymmetric topology:

- The left path through ``tsnSwitchA1`` and ``tsnSwitchA2`` is longer (two hops)
- The right path through ``tsnSwitchB1`` is shorter (one hop)

We again set up link failures in this network:

.. literalinclude:: ../asym-diamond-link-failure.xml
   :language: xml

The following failure case happen:

1. From 1.5s to 5.5s the link between ``tsnDevice1`` and ``tsnSwitchA1`` is cut.
2. From 7.5s to 11.5s the link between ``tsnSwitch1`` and ``tsnSwitchB1`` is cut.
3. From 13.5s to 17.5s the link between ``tsnSwitchA1`` and ``tsnSwitchA2`` is cut.

We assume that ``tsnDevice2`` is synchronized via ``tsnSwitchB1``, as it is the shorter path.
The following figure confirms this:

.. figure:: media/BMCAAsymmetricDiamond.png
   :align: center

1. When the link between ``tsnDevice1`` and ``tsnSwitchA1`` is cut, only ``tsnSwitchA2`` and ``tsnSwitchA2`` drift away.Between
2. When the link between ``tsnSwitch1`` and ``tsnSwitchB1`` is cut, ``tsnDevice2`` and ``tsnSwitchB1`` drift away.
3. When the link between ``tsnSwitchA2`` and ``tsnSwitchA1`` is cut, only ``tsnSwitchA2`` drifts away.

In all cases, the network is able to set up a new synchronization tree over the operational redundant path.


