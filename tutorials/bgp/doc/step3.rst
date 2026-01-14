Step 3. BGP Path Attributes
============================

Goals
-----

The goal of this step is to demonstrate BGP path attributes, particularly the AS_PATH
attribute, in a multi-AS scenario.

BGP uses path attributes to describe routes and make routing decisions. The most
important path attributes include:

*   **AS_PATH**: A list of AS numbers that a route advertisement has traversed.
    This attribute is used to detect and prevent routing loops, and as a tie-breaker
    in the route selection process (shorter AS paths are preferred).
*   **NEXT_HOP**: The IP address of the next hop router to reach a destination.
    In E-BGP, this is typically the IP address of the advertising router.
*   **ORIGIN**: Indicates how the route was originally injected into BGP
    (IGP, EGP, or incomplete).

When a BGP router advertises a route to an E-BGP peer, it prepends its own AS number
to the AS_PATH. This creates a record of the autonomous systems the route has traversed,
preventing routing loops (a router will reject a route if its own AS appears in the AS_PATH).

This step demonstrates:

*   Multiple E-BGP sessions in a fully-meshed topology of four autonomous systems
*   AS_PATH construction and propagation
*   Route selection based on AS_PATH length
*   Loop prevention via AS_PATH

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_1a.png
   :width: 100%
   :align: center

The network consists of four autonomous systems, each with internal OSPF routing:

*   **AS 64500** (router RA4): Connected to all other ASes
*   **AS 64600** (router RB1): Connected to RA4, RC2, and RD3
*   **AS 64700** (router RC2): Connected to RA4 and RB1
*   **AS 64800** (router RD3): Connected to RA4 and RB1

The topology creates multiple paths between ASes. For example, AS 64700 can reach
AS 64800 via two paths:

*   Direct: AS 64700 → AS 64500 → AS 64800 (AS_PATH: 64500 64800)
*   Indirect: AS 64700 → AS 64600 → AS 64800 (AS_PATH: 64600 64800)

.. literalinclude:: ../BGP_Topology_1a.ned
   :language: ned
   :start-at: network BGP

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step3
   :end-before: ------

The BGP configuration defines five E-BGP sessions forming a partial mesh:

.. literalinclude:: ../BGPConfig_FullAS.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF establishes routing within each autonomous system
    *   Each AS's internal networks are learned via OSPF

2.  **BGP Session Establishment** (t≈60s):

    *   Five E-BGP sessions are established between the border routers
    *   Sessions: RA4-RB1, RA4-RC2, RA4-RD3, RB1-RC2, RB1-RD3

3.  **Initial Route Advertisement** (t≈60-62s):

    *   Each AS advertises its internal networks to its BGP peers
    *   Routes propagate with AS_PATH containing only the origin AS

4.  **Route Propagation** (t≈62-65s):

    *   Routers re-advertise learned routes to other E-BGP peers
    *   AS_PATH is extended with each AS traversed
    *   For example, when RA4 advertises a route learned from RC2 to RB1:
        
        - Original AS_PATH from RC2: [64700]
        - RA4 prepends its AS: [64500, 64700]
        - RB1 receives the route with AS_PATH: [64500, 64700]

5.  **Route Selection**:

    *   When multiple paths exist to the same destination, BGP selects based on:
        
        a. Prefer the route with the shortest AS_PATH
        b. Among equal-length paths, prefer based on other attributes
    
    *   Example: RC2 learns about AS 64800's networks via:
        
        - RA4 with AS_PATH [64500, 64800] - length 2
        - RB1 with AS_PATH [64600, 64800] - length 2
        
        Both paths have equal AS_PATH length, so other tie-breakers apply

6.  **Loop Prevention**:

    *   When RA4 advertises a route from RB1 back to RB1, the route is rejected
    *   RB1 detects its own AS number (64600) in the AS_PATH and discards the route
    *   This prevents routing loops in the BGP domain

The AS_PATH attribute ensures loop-free routing between autonomous systems and
influences path selection, demonstrating BGP's policy-based routing capabilities.

Sources: :download:`BGP_Topology_1a.ned <../BGP_Topology_1a.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_FullAS.xml <../OSPFConfig_FullAS.xml>`,
:download:`BGPConfig_FullAS.xml <../BGPConfig_FullAS.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
