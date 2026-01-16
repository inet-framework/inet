Step 6. OSPF DR/BDR election in a multi-access network (Ethernet)
=================================================================

Goals
-----

The goal of this step is to demonstrate Designated Router (DR) and Backup Designated Router
(BDR) election on a multi-access network.

On multi-access networks like Ethernet, OSPF elects a DR and BDR to reduce the number of
adjacencies and LSA flooding overhead. Instead of every router forming adjacencies with
every other router (N×(N-1)/2 adjacencies), all routers form adjacencies only with the DR
and BDR, reducing complexity to 2N adjacencies.

The DR is responsible for:

*   Generating Network LSAs (Type-2) for the multi-access network
*   Coordinating LSA flooding on the segment

The election is based on router priority (higher wins) and Router ID (as a tiebreaker).

Configuration
~~~~~~~~~~~~~

This step uses the ``Network2`` topology with multiple routers connected to an Ethernet switch.

.. figure:: media/Network2.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6
   :end-before: ------

Results
~~~~~~~

When the simulation starts:

1.  All routers on the Ethernet segment exchange Hello packets.

2.  Each router sees which other routers are present and their priorities.

3.  The router with the highest priority becomes the DR. The router with the second-highest
    priority becomes the BDR. (If priorities are equal, the highest Router ID wins.)

    In this simulation, all routers have the default priority of 1. R6 has the highest Router ID,
    so it becomes the DR. R5 has the second-highest Router ID, so it becomes the BDR.

4.  Routers with priority 0 cannot become DR or BDR.

5.  Other routers (called DROthers) form Full adjacencies only with the DR and BDR, remaining
    in 2-Way state with each other.

6.  The DR generates a Network LSA describing all routers attached to the multi-access network.

The OSPF module logs show the election process and the final DR/BDR assignments. The routing
tables and LSDBs reflect the hierarchical adjacency structure enabled by the DR/BDR mechanism.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network2.ned <../Network2.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
