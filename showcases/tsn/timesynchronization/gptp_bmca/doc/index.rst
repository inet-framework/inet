Using gPTP with the Best Master Clock Algorithm (BMCA)
======================================================

Overview
~~~~~~~~

The Best Master Clock Algorithm (BMCA) is a fundamental component of the IEEE 1588 Precision Time Protocol (PTP). 
It dynamically selects the most suitable master clock in a distributed network to ensure accurate synchronization 
without manual intervention.

BMCA continuously evaluates available clocks based on predefined criteria, ensuring that the highest-quality 
clock is chosen as the Grandmaster Clock. If the current master clock becomes unavailable, BMCA promptly re-evaluates 
the network and selects the next best candidate to maintain synchronization.

.. TODO: Create inet source file location

The Model
---------

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

In the ``General`` configuration, which has a similar setup to `gptp_hotstandby`, we enable :ned:`Gptp` modules in all network nodes.
We configure a random clock drift rate for the master clocks, while the clocks in slave and bridge nodes operate with a constant drift rate, each defined by a random distribution:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: enable time synchronization
   :end-at: driftRate

Each simulation is detailed in the following sections.

BMCA Simple Network
-------------------

In the BMCA Simple Network, we have a basic setup with a single switch connecting three devices. The best master clock is selected dynamically based on 
BMCA rules, ensuring time synchronization across the network.

The network consists of three devices (`TsnDevice1`, `TsnDevice2`, and `TsnDevice3`) connected through a switch. All devices have a gPTP module, 
which synchronizes their clocks. The switch acts as a transparent bridge, forwarding gPTP messages between devices.

.. figure:: media/BmcaShowcaseSimple.png
   :align: center

Our goal is to demonstrate BMCA's ability to select the best master clock in a simple network. We configure the devices with different priorities, 
which compete for the grandmaster clock role. The network topology is straightforward, allowing us to observe BMCA's behavior in a controlled environment.

The link failure settings are specified in the simple-link-failure.xml file:

.. code-block:: xml

   <scenario>
      <set-channel-param t="7.5s" src-module="tsnDevice1" dest-module="tsnSwitch" par="disabled" value="true"/>
      <set-channel-param t="14.5s" src-module="tsnDevice1" dest-module="tsnSwitch" par="disabled" value="false"/>
   </scenario>

The following configuration defines the grandmaster priority settings:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: *.tsnDevice1.gptp.grandmasterPriority1 = 2
   :end-at: .tsnDevice3.gptp.grandmasterPriority1 = 3

In this setup:

- `TsnDevice1` has the highest priority with a value of 2
- `TsnDevice3` follows with a priority of 3
- `TsnDevice2` has the lowest priority, as its value is not explicitly set

The BMCA algorithm will evaluate these priorities and select the best master clock based on the defined criteria.
The below chart shows the clock synchronization behavior throughout the simulation:

.. figure:: media/BMCA.png
   :align: center

The chart shows the clock time differences between devices over the simulation period. All devices start with synchronized clocks near 0 microseconds 
difference. As the simulation progresses, we can observe:

- All devices maintain good synchronization with a gradual upward drift until 7.5s
- At 7.5s, when `TsnDevice1` is disconnected, `TsnDevice3` becomes the grandmaster clock
- Between 7.5s and 10.0s, `TsnDevice2` and `TsnSwitch` synchronize to `TsnDevice3`, showing similar drift patterns
- Around 10.0s, there's a noticeable increase in drift, with `TsnSwitch` and `TsnDevice2` following `TsnDevice3`
- At 12.5s, `TsnDevice1` (blue line) shows a significant spike, likely due to its isolated state
- After 14.5s when `TsnDevice1` reconnects and becomes grandmaster again, all devices begin to realign
- By the end of the simulation, all devices show an upward drift as they follow the characteristic of the grandmaster clock

The chart effectively demonstrates how the grandmaster clock role shifts between devices during link failures, and how the synchronization is maintained 
throughout these topology changes.

BMCA with Priority Change
-------------------------

This scenario extends the BMCA Simple Network by introducing dynamic priority changes. Unlike the original setup, where priority remains static, 
this scenario demonstrates BMCA's adaptability when device priorities change over time.

We use the same network topology as in the BMCA Simple Network, but modify the grandmaster priority at a specific time using a simple-priority-change.xml configuration:

.. code-block:: xml
   
   <scenario>
      <set-param t="10s" module="tsnDevice3.gptp" par="grandmasterPriority1" value="1"/>
   </scenario>

At 10 seconds, the grandmaster priority of `TsnDevice3` is changed to 1, making it the highest priority device in the network. The change triggers the 
BMCA selection process, resulting in the reassignment of the grandmaster clock role and the network automatically adjusts time synchronization based on 
the new grandmaster.

.. figure:: media/BMCAPrioChange.png
   :align: center

The chart reveals the clock synchronization behavior during the priority change:

- Initially, all devices follow `TsnDevice1` as the grandmaster, showing similar drift patterns
- At 10.0s when `TsnDevice3`'s priority changes to 1, we can see a significant shift in the synchronization pattern
- `TsnDevice3` (green line) becomes the new reference, and other devices start to align with it
- Before 10.0s, all devices drift together with a positive slope
- After 10.0s, `TsnDevice2`, `TsnDevice3`, and `TsnSwitch` stabilize around a more negative value
- `TsnDevice1` (blue line) continues to drift upward as it's no longer the grandmaster and operates independently
- After 15.0s, there's a sharp correction where all devices realign to a common time base

This scenario effectively demonstrates BMCA's ability to respond to priority changes dynamically, ensuring that the device with the highest priority 
serves as the grandmaster clock, regardless of initial configurations. The time synchronization across the network is maintained throughout the priority 
change, showcasing BMCA's adaptability in real-time situations.

BMCA Diamond Topology
---------------------

In the BMCA Diamond Topology, we create a redundant network with multiple paths between two devices. This configuration demonstrates BMCA's ability to 
handle network failures while maintaining synchronization.

The network consists of two TSN devices (`tsnDevice1` and `tsnDevice2`) connected through two switches (`tsnSwitchA1` and `tsnSwitchB1`). Each device 
has a gPTP module for clock synchronization.

.. figure:: media/BmcaShowcaseDiamond.png
   :align: center

Both devices are connected to both switches, forming a diamond-shaped topology that provides path redundancy. In this setup, `TsnDevice1` is configured 
as the grandmaster with priority1 = 1, while `TsnDevice2` acts as a slave. The diamond topology ensures that synchronization can continue even if one 
connection fails.

The link failure settings are configured in the diamond-link-failure.xml file:

.. code-block:: xml

   <scenario>
       <set-channel-param t="1.5s" src-module="tsnDevice1" dest-module="tsnSwitchB1" par="disabled" value="true"/>
       <set-channel-param t="6.5s" src-module="tsnDevice1" dest-module="tsnSwitchB1" par="disabled" value="false"/>
       <set-channel-param t="10.5s" src-module="tsnSwitchB1" dest-module="tsnDevice2" par="disabled" value="true"/>
       <set-channel-param t="15.5s" src-module="tsnSwitchB1" dest-module="tsnDevice2" par="disabled" value="false"/>
   </scenario>

.. figure:: media/BMCADiamond.png
   :align: center

The chart shows the clock time differences between devices over the simulation period. All devices start with synchronized clocks at around 0 microseconds 
difference. As the simulation progresses, we can observe:

- `tsnDevice1` remains the grandmaster throughout (blue line)
- After the link failures at 1.5s and restored at 6.5s, all devices remain well synchronized
- After the link between `tsnSwitchB1` and `tsnDevice2` fails at 10.5s, we see `tsnSwitchB1` (red line) beginning to drift
- When all connections are restored at 15.5s, the drift of `tsnSwitchB1` continues to increase due to its clock characteristics
- `tsnDevice2` and `tsnSwitchA1` maintain good synchronization with the grandmaster throughout the simulation

The increasing drift in `tsnSwitchB1` demonstrates how clock characteristics impact synchronization quality even after connectivity is restored. 
Despite this drift, BMCA successfully maintains synchronization for the critical devices in the network.

BMCA Asymmetric Diamond
-----------------------

The BMCA Asymmetric Diamond topology extends the diamond configuration by introducing an asymmetric structure with different path lengths. This setup 
tests BMCA's ability to handle uneven network structures while maintaining synchronization.

The network consists of two TSN devices (`tsnDevice1` and `tsnDevice2`) connected through three switches, forming an asymmetric diamond pattern:

.. figure:: media/BmcaShowcaseDiamondAsymmetric.png
   :align: center

In this asymmetric topology:
- The left path through `tsnSwitchA1` and `tsnSwitchA2` is longer (two hops)
- The right path through `tsnSwitchB1` is shorter (one hop)

This configuration is designed to demonstrate how BMCA handles networks with paths of different lengths and how it responds to failures in such an 
asymmetric environment.

The link failure settings are configured in the asym-diamond-link-failure.xml file and these link failures create a series of challenges for the BMCA algorithm:

.. code-block:: xml

   <scenario>
       <set-channel-param t="1.5s" src-module="tsnDevice1" dest-module="tsnSwitchA1" par="disabled" value="true"/>
       <set-channel-param t="5.5s" src-module="tsnDevice1" dest-module="tsnSwitchA1" par="disabled" value="false"/>
       <set-channel-param t="7.5s" src-module="tsnDevice1" dest-module="tsnSwitchB1" par="disabled" value="true"/>
       <set-channel-param t="11.5s" src-module="tsnDevice1" dest-module="tsnSwitchB1" par="disabled" value="false"/>
       <set-channel-param t="13.5s" src-module="tsnSwitchA1" dest-module="tsnSwitchA2" par="disabled" value="true"/>
       <set-channel-param t="17.5s" src-module="tsnSwitchA1" dest-module="tsnSwitchA2" par="disabled" value="false"/>
   </scenario>

.. figure:: media/BMCAAsymmetricDiamond.png
   :align: center

The chart illustrates the clock time differences between devices over the simulation period. All devices start synchronized near 0 microseconds difference. 
As the simulation progresses, we can observe:

- All devices maintain good synchronization until approximately 7.5s
- After 7.5s (when the link between `tsnDevice1` and `tsnSwitchB1` fails), the clocks begin to drift negatively
- At around 10s, `tsnDevice2` (orange line) shows a significant negative spike, indicating a temporary synchronization issue
- At 13.5s (when the link between `tsnSwitchA1` and `tsnSwitchA2` fails), `tsnSwitchA2` (red line) stabilizes while other devices continue to drift
- By the end of the simulation, we see significant negative drift in most network devices, with `tsnSwitchB1` (purple line) showing the largest deviation

This test case demonstrates that despite multiple link failures in an asymmetric topology, BMCA continues to maintain the time synchronization hierarchy. 
The increasing negative drift across most devices shows the accumulated effect of path changes and clock characteristics over time. The algorithm successfully 
handles the asymmetric paths and link failures, though with varying levels of synchronization quality depending on the network conditions.


