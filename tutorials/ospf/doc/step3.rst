Step 3. OSPF full adjacency establishment and LSDB sync
=======================================================

Goals
-----

The goal of this step is to examine the process of OSPF adjacency establishment and
Link State Database (LSDB) synchronization between two routers.

OSPF routers go through several states when forming an adjacency: Down → Init → 2-Way →
ExStart → Exchange → Loading → Full. The Exchange and Loading states involve exchanging
Database Description (DBD) packets, Link State Request (LSR) packets, and Link State
Update (LSU) packets to synchronize the LSDBs. Only after reaching the Full state do
routers have identical LSDBs and can use each other for SPF calculations.

Configuration
~~~~~~~~~~~~~

This step uses a simple two-router topology to clearly observe the adjacency process.

.. figure:: media/Network.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step3
   :end-before: ------

Results
~~~~~~~

When the simulation starts, the two OSPF routers (R1 and R2) establish an adjacency
on their point-to-point link:

1.  **Init state**: Each router receives a Hello packet from the other and transitions to Init.

2.  **2-Way state**: When each router sees its own Router ID in the neighbor's Hello packet,
    they transition to 2-Way.

3.  **ExStart state**: The routers determine which one will be the master (higher Router ID)
    for the DBD exchange.

4.  **Exchange state**: The routers exchange DBD packets describing the LSAs in their LSDBs.

5.  **Loading state**: If a router finds that its neighbor has LSAs it doesn't have (or newer
    versions), it sends LSR packets to request them. The neighbor responds with LSU packets
    containing the requested LS As.

6.  **Full state**: Once the LSDBs are synchronized, the adjacency reaches the Full state.

The PCAP and OSPF module logs show the detailed packet exchange during these state transitions,
demonstrating the LSDB synchronization mechanism that ensures all OSPF routers in an area have
consistent topology information.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network.ned <../Network.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
