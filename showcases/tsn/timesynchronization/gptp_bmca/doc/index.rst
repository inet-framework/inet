.. _ug:cha:bmca:

The Best Master Clock Algorithm (BMCA)
======================================

.. _ug:sec:bmca:overview:

Overview
--------

The Best Master Clock Algorithm (BMCA) is a fundamental part of the IEEE 1588 Precision Time Protocol (PTP).
It enables automatic selection of the best master clock in a distributed network to ensure accurate synchronization.
Imagine that a master clock in the network that provides time for the entire network.
When losing the time synchronization signal and the clock goes offline, with a single clock, the timing network may go into hold for a limited time, after which time synchronization is lost.
Then the BMCA comes into play, which selects the best master clock in the network to ensure accurate synchronization.
It decides which of the clocks should act as the master clock. Each clock sends a message to the network to detect other clocks, and then proceeds data set compare.
This compares the data strings from each device and determines which clock is best suited to maintain the timing network.
This algorithm runs independently on each device in a PTP domain and determines whether it should act as the master or follow another device as a slave.

PTP networks operate in a hierarchical structure with a single Grandmaster Clock at the top, distributing time to all slave clocks.
BMCA ensures that the best available clock is always selected dynamically without manual intervention.

A basic implementation of bmca is provided.

| Source files location: `inet/showcases/tsn/timesynchronization/gptp_bmca <https://github.com/inet-framework/inet/tree/master/showcases/tsn/timesynchronization/gptp_bmca>`__




.. _ug:sec:bmca:steps:

Basic Steps
-----------

- Initialize: Generate the local clock's priority vector.
- Compare: Compare the priority vectors of the local clock and the received Announce messages.
- Select: Select the best clock.
- Update: Update the port states based on the selected clock.



.. _ug:sec:bmca:initialization:

Initialization
--------------

Initialization so that the BMCA ports can subscribe to the transmission and reception signals.
Only BMCA nodes should have the BMCA ports.
They are able to receive multicast PTP messages.



.. _ug:sec:bmca:priority:

Set the Local Priority Vector
-----------------------------

Set up the local priority vector, which determines the node's eligibility to become the Grandmaster in a PTP network.
Lower values represent higher precision.

- Priority 1: ser-defined value that affects master clock selection.
- Clock Class (248): Indicates a generic clock with no special timing source.
- Clock Accuracy (0):Placeholder value; should be set dynamically based on actual clock performance.
- Offset Scaled Log Variance (0): Represents clock stability; ideally, should be dynamically calculated.
- Priority 2 (0): Another user-defined priority.
- GrandmasterIdentity: uniquely identifies this clock in the network.
- Steps Removed (0):This The number of hops from the Grandmaster. A device directly connected to the Grandmaster has stepsRemoved = 0.



.. _ug:sec:bmca:execution:

Execution
---------

Evaluates Announce messages, selects the best Grandmaster, and updates the port states accordingly.

- Initialize Local Announce Message: Creates an Announce message for the local nodes which contains the local clock’s priority vector.
- Select the Best Announce Message: Iterates through received Announce messages from other nodes. Calls "compareAnnounceMessages()" to determine the best Announce message, which is the one with the highest priority according to IEEE 1588-2019.
- Prints debugging information about the selected Grandmaster.
- Handle Grandmaster Selection Changes: If the best Announce remains the same, no topology changes occur.
                                        If a better Announce is received, the node updates its role:
                                        If the new Grandmaster is someone else, the node becomes a slave.
                                        If the node itself is the Grandmaster, it updates its ports.
                                        Announce messages are sent to notify others if a new Grandmaster is selected.
- Update Port States: Updates port roles based on the IEEE 1588-2019 standard.
                      Passive ports are identified (e.g., when a port connects to a better clock).
                      Master ports are determined by comparing the local clock with the best Announce.



.. _ug:sec:bmca:comparision:

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

This function follows IEEE 1588-2019 BMCA rules.
It prioritizes the best path, resolves tie-breakers, and detects invalid conditions.

