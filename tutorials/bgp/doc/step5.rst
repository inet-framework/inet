Step 5. BGP Scenario with I-BGP over not directly-connected BGP speakers
========================================================================

Goals
-----

This step explores Internal BGP (I-BGP) in a more realistic scenario where I-BGP
peers are not directly connected. The topology (``BGP_Topology_3.ned``) features
a transit Autonomous System, AS 64600 (RB), consisting of three routers: RB1,
RB3, and RB2.

The setup is as follows:

- **RB1 and RB2** are border routers running BGP. They are I-BGP peers.
- **RB3** is an interior router that **does not run BGP**.
- The routers are connected in a chain: RB1 <-> RB3 <-> RB2.
- OSPF is running on all routers within AS 64600 to provide internal reachability.

The primary goal is to demonstrate the "BGP Hole" or "BGP Synchronization"
problem. While RB1 and RB2 can establish an I-BGP session over the multi-hop
OSPF path, the interior router RB3 remains unaware of the external routes
learned via BGP.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_3.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Multi.xml
   :language: xml

Results
~~~~~~~

In this simulation, the following happens:

1. OSPF converges, allowing RB1 and RB2 to ping each other's interface addresses via RB3.
2. RB1 and RB2 establish an I-BGP session.
3. RB1 learns RA's networks (10.0.0.x) from RA4 and advertises them to RB2.
4. RB2 learns RC's networks (30.0.0.x) from RC1 and advertises them to RB1.
5. **The Problem**: While RB1 and RB2 have these routes in their BGP tables, the
   transit router **RB3 has no knowledge of these external networks**.

If you were to trace a packet from AS 64500 to AS 64700:

- RA4 sends it to RB1.
- RB1 knows the destination is reachable via RB2 and sends it towards RB3.
- RB3 receives the packet, looks up the destination (e.g., 30.0.0.26), finds no match in its OSPF-only routing table, and drops the packet.

This demonstrates that simply having I-BGP peering between border routers is
insufficient for a transit AS if the intermediate routers are not running BGP or
receiving redistributed routes.

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_Multi.xml <../OSPFConfig_Multi.xml>`
:download:`BGPConfig_Multi.xml <../BGPConfig_Multi.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1085>`__ in
the GitHub issue tracker for commenting on this tutorial.
