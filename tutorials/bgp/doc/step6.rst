Step 6. BGP Scenario and using loopbacks
==========================================

Goals
-----

The goal of this step is to demonstrate the use of loopback interfaces for BGP sessions
and to show a more complex multi-router AS topology.

In production networks, BGP sessions are often established using **loopback interfaces**
rather than physical interface addresses. Loopback interfaces provide several advantages:

*   **Stability**: Loopback interfaces are always up (not dependent on physical link state)
*   **Redundancy**: BGP session remains up even if one physical path fails, as long as
    alternative paths exist via the IGP
*   **Simplified configuration**: A single loopback address can be used for all BGP
    peerings instead of tracking multiple interface addresses
*   **Load balancing**: With multiple physical paths, traffic can be load-balanced while
    the BGP session uses the loopback

When using loopback interfaces:

*   The IGP must advertise the loopback addresses
*   BGP uses the IGP to find paths to the peer's loopback
*   If the best IGP path changes, BGP traffic automatically follows without session reset

This step also demonstrates a larger AS (AS 64600) with four routers running both
OSPF and BGP in a full mesh.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The network has three autonomous systems:

*   **AS 64500** (router RA4): Border router with OSPF
*   **AS 64600** (routers RB1, RB2, RB3, RB4): Four routers with OSPF and full I-BGP mesh
*   **AS 64700** (router RC1): Border router with OSPF

AS 64600 has a more complex topology with redundant paths:

*   **RB1**: Border router with E-BGP to RA4
*   **RB4**: Border router with E-BGP to RC1
*   **RB2, RB3**: Internal routers providing redundancy

All four routers in AS 64600 run BGP and form a full I-BGP mesh (6 sessions).

.. literalinclude:: ../BGP_Topology_4.ned
   :language: ned
   :start-at: network BGP

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step6]
   :end-before: ------

Key configuration:

*   All routers run OSPF for internal routing
*   All six routers (RA4, RB1-RB4, RC1) have BGP enabled
*   OSPF advertises loopback and all internal addresses
*   BGP sessions can use either loopback or interface addresses

The BGP configuration:

.. literalinclude:: ../BGPConfig_Redist.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF establishes routing in all three ASes
    *   In AS 64600, OSPF creates a fully connected routing domain
    *   Multiple paths exist between routers (e.g., RB1 to RB4 via RB2 or RB3)
    *   OSPF calculates shortest paths and installs them in routing tables

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB4-RC1
    *   I-BGP full mesh in AS 64600: RB1-RB2, RB1-RB3, RB1-RB4, RB2-RB3, RB2-RB4, RB3-RB4
    *   Six I-BGP sessions for four routers (N×(N-1)/2 = 6)

3.  **Route Distribution**:

    *   RA4 advertises AS 64500's networks to RB1 via E-BGP
    *   RB1 redistributes these routes to all I-BGP peers (RB2, RB3, RB4)
    *   RC1 advertises AS 64700's networks to RB4 via E-BGP
    *   RB4 redistributes these to all I-BGP peers (RB1, RB2, RB3)

4.  **Redundancy Benefits**:

    *   If the direct link between RB1 and RB4 fails, their I-BGP session can remain
        up using alternative paths through RB2 or RB3
    *   OSPF automatically reroutes the BGP session traffic
    *   BGP learns about the network change without session disruption

5.  **Full Connectivity**:

    *   All routers in AS 64600 have routes to both external ASes
    *   Multiple paths provide redundancy
    *   Traffic can flow through alternative routers if primary paths fail

This topology demonstrates how BGP and OSPF work together in a redundant network.
The full I-BGP mesh ensures all routers have complete routing information, while
OSPF provides the underlying connectivity and path selection for BGP traffic.

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
