Step 7. BGP with RIP redistribution
====================================

Goals
-----

The goal of this step is to demonstrate BGP redistribution from RIP (Routing Information
Protocol) instead of OSPF.

While OSPF is commonly used as an IGP in modern networks, RIP is still found in some
environments, particularly smaller or legacy networks. BGP can redistribute routes from
any IGP, not just OSPF. This step shows that the principles of BGP-IGP interaction
remain the same regardless of which IGP is used.

**RIP (Routing Information Protocol)** is a distance-vector routing protocol that:

*   Uses hop count as the metric (maximum 15 hops)
*   Sends periodic updates every 30 seconds
*   Is simpler to configure than OSPF
*   Has slower convergence than link-state protocols like OSPF
*   Is suitable for smaller networks

The redistribution process works the same way:

*   Border routers learn internal routes via RIP
*   Border routers redistribute these RIP routes into BGP for advertisement to peers
*   BGP routes received from peers can be redistributed back into RIP

Configuration
~~~~~~~~~~~~~

This step uses the same topology as Step 6:

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` replaces OSPF with RIP:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step7]
   :end-before: ------

Key changes from Step 6:

*   ``*.R*.hasRip = true``: Enables RIP instead of OSPF
*   ``*.R*.bgp.redistributeRip = true``: BGP redistributes RIP routes
*   Uses ``RIPConfig.xml`` for RIP configuration

The RIP configuration:

.. literalinclude:: ../RIPConfig.xml
   :language: xml

The BGP configuration remains the same:

.. literalinclude:: ../BGPConfig_Redist.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates:

1.  **RIP Convergence** (t≈0-40s):

    *   RIP establishes routing in all three ASes
    *   Routers exchange RIP updates every 30 seconds
    *   RIP propagates routes with hop count metrics
    *   Convergence is slower than OSPF but completes within the startup delay

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB4-RC1
    *   I-BGP full mesh in AS 64600: 6 sessions between the four routers
    *   BGP waits for RIP to converge before establishing sessions

3.  **RIP-to-BGP Redistribution**:

    *   Border routers (RA4, RB1-RB4, RC1) learn internal routes via RIP
    *   They redistribute these RIP routes into BGP
    *   External routes advertised via BGP include:
        
        - AS 64500's networks learned by RA4 via RIP
        - AS 64600's networks learned by RB1 and RB4 via RIP
        - AS 64700's networks learned by RC1 via RIP

4.  **BGP-to-RIP Redistribution**:

    *   Border routers learn external routes via BGP
    *   They redistribute these BGP routes into RIP
    *   RIP propagates these external routes throughout each AS
    *   All routers learn about external destinations via RIP

5.  **Full Connectivity**:

    *   Traffic flows between all three ASes
    *   RIP provides internal routing, BGP provides inter-AS routing
    *   The interaction between RIP and BGP is transparent to end users

The key takeaway is that BGP works with any IGP. The choice between OSPF, RIP, or
other IGPs (like EIGRP or IS-IS) depends on network requirements, not BGP capabilities.
BGP's role as an inter-domain protocol remains consistent regardless of the IGP used
within each domain.

**Comparison: RIP vs OSPF for BGP:**

*   **Convergence**: OSPF converges faster, better for larger networks
*   **Scalability**: OSPF scales better, RIP limited to 15 hops
*   **Complexity**: RIP is simpler to configure, OSPF offers more features
*   **Resources**: RIP uses less CPU/memory, OSPF requires more resources
*   **BGP interaction**: Both work equally well with BGP for route redistribution

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RIPConfig.xml <../RIPConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
