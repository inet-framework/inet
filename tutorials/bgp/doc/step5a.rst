Step 5a. BGP internal distribution
===================================

Goals
-----

The goal of this step is to solve the routing problem from Step 5 by redistributing
BGP routes into the IGP (OSPF).

In Step 5, we saw that intermediate routers (like RB3) that don't run BGP have no
knowledge of external routes learned via BGP. One solution is to redistribute BGP
routes into the IGP, making them available to all routers in the AS.

**BGP-to-IGP Redistribution** involves:

*   Border routers inject BGP-learned routes into the IGP
*   The IGP propagates these routes throughout the AS
*   All routers (BGP and non-BGP) learn about external destinations

This approach has trade-offs:

**Advantages:**
*   Simple - all routers automatically learn external routes
*   No need to run BGP on every router

**Disadvantages:**
*   Can overwhelm the IGP with many external routes (Internet full table has 900k+ routes)
*   IGP convergence may be slow with many routes
*   Loss of BGP policy information (AS_PATH, communities, etc.)
*   Not scalable for large networks or Internet service providers

This is typically used in smaller enterprise networks with a limited number of external
routes, not in service provider networks carrying full Internet routing tables.

Configuration
~~~~~~~~~~~~~

This step uses the same network topology as Step 5:

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

The configuration extends Step 5:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step5a]
   :end-before: ------

The key addition is:

*   ``*.RB{1,2}.bgp.redistributeInternal = true``: Border routers RB1 and RB2
    redistribute BGP-learned routes into OSPF

This setting causes BGP routes (both E-BGP and I-BGP learned routes) to be injected
into the OSPF domain as external routes.

Results
~~~~~~~

With BGP-to-OSPF redistribution enabled:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF establishes routing within all autonomous systems
    *   RB1, RB2, and RB3 all participate in OSPF

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB2-RC1
    *   I-BGP session: RB1-RB2

3.  **Route Exchange and Redistribution** (t≈60-65s):

    *   RB1 learns AS 64500's networks via E-BGP from RA4
    *   RB1 redistributes these BGP routes into OSPF as external routes
    *   OSPF floods these external routes throughout AS 64600
    *   RB2 and RB3 now learn about AS 64500's networks via OSPF
    
    *   Similarly, RB2 learns AS 64700's networks via E-BGP from RC1
    *   RB2 redistributes these into OSPF
    *   RB1 and RB3 learn about AS 64700's networks via OSPF

4.  **Complete Routing Table**:

    *   RB3 (non-BGP router) now has routes to both AS 64500 and AS 64700
    *   These routes point to RB1 and RB2 respectively (via OSPF)
    *   All routers in AS 64600 can forward traffic to external destinations

5.  **End-to-End Connectivity**:

    *   Traffic from AS 64500 to AS 64700: RA4 → RB1 → RB3 → RB2 → RC1
    *   RB3 can now correctly forward the traffic because it learned the routes via OSPF
    *   Traffic from AS 64700 to AS 64500: RC1 → RB2 → RB3 → RB1 → RA4
    *   Full connectivity is restored

The redistribution of BGP routes into OSPF solves the routing problem for intermediate
routers. However, this approach should be used cautiously in production networks due
to scalability concerns.

**Best Practice**: Use BGP redistribution into IGP only when:

*   The number of external routes is small (typically < 100)
*   The AS is relatively small
*   Simplicity is preferred over scalability
*   You're not receiving full Internet routes

For larger deployments, consider:

*   Running BGP on all routers (full mesh I-BGP or route reflectors)
*   Using MPLS to tunnel traffic through the core
*   Using default routes in the IGP

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_Multi.xml <../OSPFConfig_Multi.xml>`,
:download:`BGPConfig_Multi.xml <../BGPConfig_Multi.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
