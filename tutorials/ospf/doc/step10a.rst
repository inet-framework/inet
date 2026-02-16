Step 10a. Router R4 goes down
=============================

Goals
-----

The goal of this step is to demonstrate OSPF's reaction when an entire router fails.

When a router completely fails (as opposed to just a single link), all of its adjacencies
are lost simultaneously. Neighboring routers detect the failure and update the topology
accordingly.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 10. The simulation script shuts down router **R4** at t=60s.

.. figure:: media/TopologyChange.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10a
   :end-before: ------

Results
~~~~~~~

When R4 is shutdown at t=60s:

1.  All routers adjacent to **R4** (R3, and any others) detect the adjacency loss when they stop
    receiving Hello packets.

2.  After the Dead Interval expires, these neighbors declare R4 dead and remove it from their
    neighbor lists.

3.  Routers that had adjacencies with R4 generate new LSAs reflecting the topology change
    (removing links to R4).

4.  These LSAs are flooded throughout the area.

5.  All routers update their LSDBs, removing R4's Router LSA and any links to R4.

In General, routes that used a failed router as a next-hop or transit router are recomputed, using alternative paths
if available.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`TopologyChange.ned <../TopologyChange.ned>`,
:download:`ASConfig_tp_priority.xml <../ASConfig_tp_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
