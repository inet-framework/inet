Step 5. BGP Scenario with I-BGP over not directly-connected BGP speakers
=========================================================================

Goals
-----

The goal of this step is to demonstrate I-BGP when the BGP speakers are not directly
connected, requiring an Interior Gateway Protocol (IGP) for reachability.

In Step 4, the I-BGP peers (RB1 and RB2) were directly connected. In real networks,
I-BGP peers are often not directly connected, and rely on an IGP (like OSPF) to
provide IP reachability between them. This step explores this more realistic scenario.

Key concepts:

*   **IGP-BGP interaction**: BGP relies on the IGP to resolve NEXT_HOP addresses
    and provide paths between I-BGP peers
*   **BGP-IGP synchronization**: BGP routes should only be used if the IGP also
    has routes to reach the BGP NEXT_HOP
*   **Route redistribution**: External BGP routes learned by border routers may
    need to be redistributed into the IGP for internal routers to use them

This step introduces an intermediate router (RB3) between the two border routers,
making OSPF necessary for reachability within AS 64600.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

The network has three autonomous systems:

*   **AS 64500** (router RA4 with OSPF)
*   **AS 64600** (routers RB1, RB2, RB3 with OSPF): Three routers, with RB3 between the border routers
*   **AS 64700** (router RC1 with OSPF)

AS 64600's topology:

*   **RB1**: Border router with E-BGP to RA4, connected to RB3
*   **RB3**: Internal router (non-BGP), connects RB1 and RB2
*   **RB2**: Border router with E-BGP to RC1, connected to RB3

RB1 and RB2 are not directly connected, so they rely on OSPF to:

*   Provide IP reachability between them for the I-BGP session
*   Learn about each other's interfaces (for next-hop-self to work)
*   Potentially carry external routes (if redistributed)

.. literalinclude:: ../BGP_Topology_3.ned
   :language: ned
   :start-at: network BGP

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step5]
   :end-before: ------

Key configuration:

*   ``*.R*.hasOspf = true``: OSPF runs on all routers in AS 64600
*   Only RB1 and RB2 have BGP enabled (``*.RA4.hasBgp = true``, etc.)
*   RB3 is not a BGP speaker, only running OSPF

The BGP configuration:

.. literalinclude:: ../BGPConfig_Multi.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates the following:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF establishes routing in all three ASes
    *   In AS 64600, OSPF provides routes between RB1, RB2, and RB3
    *   RB1 and RB2 learn each other's addresses via OSPF

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB2-RC1 are established
    *   I-BGP session: RB1-RB2 is established over the OSPF-provided path through RB3

3.  **Route Exchange** (t≈60-62s):

    *   RA4 advertises AS 64500's networks to RB1
    *   RC1 advertises AS 64700's networks to RB2
    *   RB1 and RB2 exchange routes via I-BGP

4.  **Next-Hop Resolution**:

    *   With next-hop-self configured, RB1 sets itself as NEXT_HOP when advertising to RB2
    *   RB2 can reach RB1 via OSPF through RB3
    *   Similarly, RB1 can reach RB2 via OSPF through RB3

5.  **Traffic Flow Issue**:

    *   BGP establishes the routes correctly
    *   However, RB3 (the intermediate router) only has OSPF routes
    *   RB3 does not know about the external networks (AS 64500 and AS 64700)
    *   **Problem**: Traffic from AS 64500 reaches RB1, which forwards to RB2 via RB3,
        but RB3 may not have a route back to AS 64500, causing routing issues

This demonstrates that while I-BGP can operate over non-direct links using IGP for
reachability, internal routers that are not BGP speakers need some way to learn about
external routes. Solutions include:

*   Redistributing BGP routes into the IGP (Step 5a)
*   Enabling BGP on all routers (Step 5b)
*   Using MPLS to tunnel traffic across the AS

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_Multi.xml <../OSPFConfig_Multi.xml>`,
:download:`BGPConfig_Multi.xml <../BGPConfig_Multi.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
