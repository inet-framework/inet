The Best Master Clock Algorithm (BMCA)
======================================

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

In the BMCA Simple Network, we have a basic setup with a single switch connecting two devices. The best master clock is selected dynamically 
based on BMCA rules, ensuring time synchronization across the network.

The network consists of three devices (`TsnDevice1`, `TsnDevice2`, and `TsnDevice3`) connected through a switch. Both devices have a Gptp module, 
which synchronizes their clocks. The switch acts as a transparent bridge, forwarding gPTP messages between devices.

.. figure:: media/BmcaShowcaseSimple.png
   :align: center

Our goal is to demonstrate BMCA’s ability to select the best master clock in a simple network. We configure the devices with different priorities,
which compete for the grandmaster clock role. The network topology is straightforward, allowing us to observe BMCA’s behavior in a controlled environment.

The link failure settings can be adjusted in the XML file as follows:

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
- `TsnDevice3` follows with a priority of 3. 
- `TsnDevice2` has the lowest priority, as its value is not explicitly set.

According to the link failure settings:
- At 7.5 seconds, `TsnDevice1` will lose its connection to the switch, triggering a link failure. 
- The devices detect the failure, and BMCA initiates the grandmaster re-election process
- Since `TsnDevice1` is no longer reachable, the grandmaster clock role will be reassigned based on BMCA rules.
- Due to the priority settings, `TsnDevice3` becomes the new grandmaster clock.
- Time synchronization is maintained between `TsnDevice2` and `TsnDevice3` until the link is restored.
- At 14.5 seconds, the link between :ned:TsnDevice1 and the switch is restored, allowing BMCA to reevaluate and resume normal operation.

Here is the clock synchronization chart for the three devices and the switch:

.. figure:: media/BMCA.png
   :align: center

As shown in the chart, the grandmaster clock role is reassigned from `TsnDevice1` to `TsnDevice3` after the link failure at 7.5 seconds.
The drift rates of the devices are also visible, demonstrating the clock synchronization process. Once the link is restored at 14.5 seconds, 
the grandmaster clock role reverts to :ned:TsnDevice1, resuming normal synchronization.

BMCA with Priority Change
-------------------------

This scenario extends the BMCA Simple Network by introducing dynamic priority changes. Unlike the original setup, where priority remains static, 
this scenario demonstrates BMCA's adaptability when device priorities change over time.

To achieve this, we modify the grandmaster priority at a specific time using an XML configuration:

.. code-block:: xml
   
   <scenario>
      <set-param t="10s" module="tsnDevice3.gptp" par="grandmasterPriority1" value="1"/>
   </scenario>

- At 10 seconds, the grandmaster priority of `TsnDevice3` is changed to 1, making it the highest priority device in the network.
- The change triggers the BMCA selection process, resulting in the reassignment of the grandmaster clock role.
- The network automatically adjusts time synchronization based on the new grandmaster.

.. figure:: media/BMCAPrioChange.png
   :align: center

The grandmaster clock role transitions from `TsnDevice1` to `TsnDevice3` after the priority change at 10 seconds. The clock drift rates of 
the devices become visible, demonstrating BMCA’s dynamic synchronization mechanism. As a result, The network seamlessly adapts to the priority change, 
maintaining accurate time synchronization.

BMCA Diamond Topology
---------------------


Comparison
----------

This is the most important part of the BMCA algorithm.
It compares two gPTP Announce messages based on their Priority Vector and topology information, following the rules defined in IEEE 1588-2019 Figure 34 & 35.

- Extract Priority Vectors: Retrieves the Priority Vector from each GptpAnnounce message. The Priority Vector includes: grandmasterIdentity, stepsRemoved, grandmasterClockQuality, priority1 & priority2.
- Compare Grandmaster Identity: If both messages come from the same Grandmaster, we apply special rules (IEEE 1588-2019 Figure 35).
- Compare Steps Removed (Path Cost): Fewer stepsRemoved means a better path. A node closer to the Grandmaster is preferred. If the difference is 2 or more, the closer clock is always preferred.
- If stepsRemoved difference is 1: If one message has stepsRemoved + 1, topology rules apply. If the same port sent and received the message, it is invalid (Error-1). Otherwise, topology ordering determines priority.
- If stepsRemoved is equal: Compare Sender Identities. The sender with the lower SourcePortIdentity wins, which ensures a consistent tie-breaker in BMCA.
- If grandmasterIdentity is different: Compare entire Priority Vector. Uses lexicographical comparison on PriorityVector (grandmasterPriority1, grandmasterClockQuality, grandmasterPriority2, grandmasterIdentity, stepsRemoved).
- Error Handling for Unexpected Cases.

This function follows IEEE 1588-2019 BMCA rules. (Reference: `1588-2019 - IEEE Standard for a Precision Clock Synchronization Protocol for Networked Measurement and Control Systems <https://ieeexplore.ieee.org/document/9120376>`__)

It prioritizes the best path, resolves tie-breakers, and detects invalid conditions.

