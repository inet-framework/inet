Step 5b. Enabling BGP on RB3
=============================

Goals
-----

The goal of this step is to solve the routing problem from Step 5 by enabling BGP on
all routers in the AS, creating a full I-BGP mesh.

This is an alternative solution to Step 5a's redistribution approach. Instead of
redistributing BGP routes into OSPF, all routers in the AS run BGP and participate
in I-BGP sessions. This is the preferred approach in larger networks and is considered
a best practice.

**Full I-BGP Mesh** means:

*   Every BGP router establishes an I-BGP session with every other BGP router in the AS
*   All routers learn external routes directly via BGP
*   No need to redistribute BGP routes into the IGP
*   BGP policy information (AS_PATH, communities, etc.) is preserved throughout the AS

For an AS with N BGP routers, a full mesh requires N×(N-1)/2 I-BGP sessions. While
this scales poorly for very large ASes (which use route reflectors or confederations),
it works well for small to medium networks.

Configuration
~~~~~~~~~~~~~

This step uses the same network topology as Step 5:

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

The configuration extends Step 5:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step5b]
   :end-before: ------

Key changes:

*   ``*.RB3.hasBgp = true``: Enables BGP on the intermediate router RB3
*   Uses ``BGPConfig_Multi_RB3.xml`` which includes RB3 in the BGP configuration

The BGP configuration now includes RB3:

.. literalinclude:: ../BGPConfig_Multi_RB3.xml
   :language: xml

With three BGP routers in AS 64600, the full I-BGP mesh requires 3 I-BGP sessions:

*   RB1 ↔ RB2
*   RB1 ↔ RB3
*   RB2 ↔ RB3

Results
~~~~~~~

With BGP enabled on all routers:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF provides IP reachability between all routers in AS 64600
    *   This is necessary for the I-BGP sessions to establish

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB2-RC1
    *   I-BGP sessions: RB1-RB2, RB1-RB3, RB2-RB3 (full mesh)

3.  **Route Exchange** (t≈60-62s):

    *   RB1 learns AS 64500's networks via E-BGP from RA4
    *   RB1 advertises these routes to both RB2 and RB3 via I-BGP
    *   RB2 learns AS 64700's networks via E-BGP from RC1
    *   RB2 advertises these routes to both RB1 and RB3 via I-BGP

4.  **Complete BGP Routing**:

    *   RB3 now learns all external routes directly via I-BGP
    *   Routes from AS 64500 with NEXT_HOP pointing to RB1
    *   Routes from AS 64700 with NEXT_HOP pointing to RB2
    *   RB3 uses OSPF to resolve these NEXT_HOP addresses

5.  **I-BGP Route Propagation Rule**:

    *   Important: RB3 learns routes from both RB1 and RB2
    *   RB3 does NOT re-advertise routes learned from RB1 to RB2 (I-BGP rule)
    *   This is why the full mesh is necessary - without it, RB3 couldn't
        ensure RB1 and RB2 learn each other's routes
    *   With the full mesh, RB1 and RB2 learn each other's routes directly

6.  **End-to-End Connectivity**:

    *   Traffic from AS 64500 to AS 64700: RA4 → RB1 → RB3 → RB2 → RC1
    *   RB3 can forward correctly using its BGP-learned routes
    *   Full connectivity is restored without redistribution

**Advantages of this approach:**

*   Scalable - no IGP bloat from external routes
*   Preserves BGP attributes for policy decisions
*   Standard best practice approach
*   Clean separation between IGP (internal connectivity) and BGP (external routes)

**Disadvantages:**

*   Requires BGP configuration on all routers
*   Full mesh can become complex in large ASes (100+ routers)
*   More memory and CPU overhead on all routers

For very large ASes (hundreds of routers), BGP Route Reflectors or AS Confederations
are used to reduce the number of I-BGP sessions while maintaining full route
distribution.

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_Multi.xml <../OSPFConfig_Multi.xml>`,
:download:`BGPConfig_Multi_RB3.xml <../BGPConfig_Multi_RB3.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
