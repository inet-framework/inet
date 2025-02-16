BMCA
==========

Goals
-----

BMCA (Best Master Clock Algorithm) is a protocol that selects the best master clock in a network.
The showcase demonstrates the basic operation of BMCA in a simple network.

| Source files location: `inet/showcases/tsn/timesynchronization/gptp_bmca <https://github.com/inet-framework/inet/tree/master/showcases/tsn/timesynchronization/gptp_bmca>`__

About BMCA
----------

Overview
~~~~~~~~

In general, BMCA is an algorithm that helps choose which clock to use as a timing source on a network.
Imagine that a master clock in the network that provides time for the entire network.
When losing the time synchronization signal and the clock goes offline, with a single clock, the timing network may go into hold for a limited time, after which time synchronization is lost.
Then the BMCA comes into play, which selects the best master clock in the network to ensure accurate synchronization.
It decides which of the clocks should act as the master clock. Each clock sends a message to the network to detect other clocks, and then proceeds data set compare.
This compares the data strings from each device and determines which clock is best suited to maintain the timing network.

According to the IEEE 802.1 AS standard, the master clock can be automatically selected by the BMCA.
BMCA also determines the clock spanning tree, i.e., the routes on which sync messages are propagated to slave clocks in the network.

Basic Steps
~~~~~~~~~~~~

- Initialize: Generate the local clock's priority vector.
- Compare: Compare the priority vectors of the local clock and the received Announce messages.
- Select: Select the best clock.
- Update: Update the port states based on the selected clock.


Priority Vector Selection
~~~~~~~~~~~~

The priority vector is a set of parameters that determine the eligibility of a node to become the Grandmaster in a PTP network.
The comparison of priority vectors is based on the following parameters: grandmasterIdentity, stepsRemoved (path cost), grandmasterClockQuality, priority1 & priority2.
- Grandmaster Identity: If the Grandmaster is the same, using IEEE 1588-2019 Figure 35.
- Steps Removed: Fewer stepsRemoved is better, meaning closer to the Grandmaster. If the difference is 2 or more, select the closer clock.
- If stepsRemoved difference is 1: If one message has stepsRemoved + 1, topology rules apply. It's illegal if the same port sent and received the message.
- If stepsRemoved is equal: Select the sender with smaller SourcePortIdentity.
- If grandmasterIdentity is different: Uses lexicographical comparison on the whole PriorityVector (grandmasterPriority1, grandmasterClockQuality, grandmasterPriority2, grandmasterIdentity, stepsRemoved).



