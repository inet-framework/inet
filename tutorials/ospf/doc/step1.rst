Step 1. Pinging after OSPF convergence
======================================

Goals
-----

The goal of this step is to demonstrate basic OSPF operation in a single-area network
and verify that hosts can communicate after OSPF convergence.

OSPF (Open Shortest Path First) is a link-state routing protocol. Routers running OSPF
exchange Link State Advertisements (LSAs) to build a complete topology database (LSDB).
Each router then runs the SPF (Shortest Path First) algorithm on this database to
compute the shortest paths to all destinations and populate its routing table.

This step shows OSPF operating in a single area (Area 0.0.0.0), with routers establishing
adjacencies, exchanging LSAs, building their LSDBs, and computing routes. After convergence,
hosts can ping each other across the network using the OSPF-computed routes.

Configuration
~~~~~~~~~~~~~

This step uses the `OspfNetwork` topology with five routers (R1-R5) and nine
hosts (host0-host8) connected via switches and point-to-point links.

.. figure:: media/OspfNetwork.png
   :width: 100%
   :align: center

.. literalinclude:: ../OspfNetwork.ned
   :start-at: network OspfNetwork
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step1
   :end-before: ------

Results
~~~~~~~

When the simulation starts:

1.  All OSPF routers discover their neighbors and establish adjacencies.

2.  Routers exchange Hello packets, then Database Description packets, Link
    State Request/Update packets to synchronize their LSDBs.

3.  Each router runs the SPF algorithm to compute lowest cost paths (running Dijkstra's weighted shortest path algorithm) to all subnetworks
    in the area.

4.  The routing tables are populated with OSPF-learned routes. For example, R1
    learns routes to the 10.0.0.24/29 network (behind R3) via the path through
    R2 and R5->R4.

5.  The R5->R4 path turns out to be higher hop-count but lower cost than the R2 path due to cost associated with link datarates.

6.  At t=60s, **host0** begins pinging **host6**. The ping succeeds because OSPF
    has computed valid routes between all subnetworks.

.. The routing table visualizer shows that all routers have complete routing
.. information for reaching all networks in the OSPF area.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OspfNetwork.ned <../OspfNetwork.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
