Step 8. BGP with OSPF and RIP redistribution
============================================

Goals
-----

The goal of this step is to demonstrate a mixed IGP environment where different
autonomous systems use different interior gateway protocols (OSPF vs RIP), and how
BGP handles redistribution from multiple IGP sources.

In real-world scenarios, especially during network migrations or in multi-vendor
environments, different parts of the network may run different IGPs. BGP must be able
to:

*   Redistribute routes from multiple IGP sources simultaneously
*   Maintain separation between different IGP domains
*   Provide seamless inter-AS connectivity despite heterogeneous IGPs

This step shows:

*   AS 64500 and AS 64700 running RIP internally
*   AS 64600 running OSPF internally
*   BGP redistributing from both OSPF and RIP
*   Full connectivity despite the mixed IGP environment

Configuration
~~~~~~~~~~~~~

This step uses the same topology as Steps 6 and 7:

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables different IGPs in different ASes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step8]
   :end-before: ------

Key configuration:

*   ``*.RA*.hasRip = true``: AS 64500 uses RIP
*   ``*.RB*.hasOspf = true``: AS 64600 uses OSPF
*   ``*.RC*.hasRip = true``: AS 64700 uses RIP
*   ``*.R*.bgp.redistributeRip = true``: BGP redistributes RIP routes where applicable
*   ``*.R*.bgp.redistributeOspf = "O IA"``: BGP redistributes OSPF routes where applicable

Each border router is configured to redistribute routes from its local IGP:

*   **RA4**: Runs RIP, redistributes RIP→BGP
*   **RB1**: Runs OSPF, redistributes OSPF→BGP
*   **RB4**: Runs OSPF, redistributes OSPF→BGP
*   **RC1**: Runs RIP, redistributes RIP→BGP

Results
~~~~~~~

The simulation demonstrates:

1.  **IGP Convergence** (t≈0-40s):

    *   RIP converges in AS 64500 (RA4 and internal routers)
    *   OSPF converges in AS 64600 (RB1, RB2, RB3, RB4)
    *   RIP converges in AS 64700 (RC1 and internal routers)
    *   Each IGP operates independently within its AS

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 (connecting RIP and OSPF domains)
    *   E-BGP sessions: RB4-RC1 (connecting OSPF and RIP domains)
    *   I-BGP full mesh in AS 64600: All OSPF routers peer via BGP

3.  **Multi-Source Redistribution**:

    *   **RA4** (in AS 64500):
        
        - Learns AS 64500's networks via RIP
        - Redistributes RIP→BGP, advertises to RB1
        - Learns AS 64600 and AS 64700 routes via BGP
        - Redistributes BGP→RIP for local distribution
    
    *   **RB1 and RB4** (in AS 64600):
        
        - Learn AS 64600's networks via OSPF
        - Redistribute OSPF→BGP, advertise to E-BGP and I-BGP peers
        - Learn AS 64500 and AS 64700 routes via BGP
        - Redistribute BGP→OSPF for local distribution
    
    *   **RC1** (in AS 64700):
        
        - Learns AS 64700's networks via RIP
        - Redistributes RIP→BGP, advertises to RB4
        - Learns AS 64500 and AS 64600 routes via BGP
        - Redistributes BGP→RIP for local distribution

4.  **Protocol Translation**:

    *   BGP acts as a "translator" between different IGP domains
    *   Routes learned via RIP in AS 64500 are advertised via BGP to AS 64600
    *   In AS 64600, these BGP routes are redistributed into OSPF
    *   From AS 64600, routes are advertised via BGP to AS 64700
    *   In AS 64700, these BGP routes are redistributed into RIP
    *   The chain: RIP (AS 64500) → BGP → OSPF (AS 64600) → BGP → RIP (AS 64700)

5.  **Seamless Connectivity**:

    *   Despite using different IGPs, all ASes achieve full connectivity
    *   BGP abstracts away the internal routing details of each AS
    *   Traffic flows smoothly across IGP boundaries
    *   Each AS maintains autonomy over its internal routing protocol choice

This demonstrates BGP's fundamental role as an inter-domain protocol: it provides a
common routing language between autonomous systems, regardless of their internal
routing protocols. This is crucial for the Internet, where thousands of organizations
use different IGPs internally but all interconnect via BGP.

**Key Insights:**

*   BGP is IGP-agnostic - it works with any IGP
*   Each AS can choose its IGP independently
*   Route metrics and attributes don't translate between IGPs
    (RIP hop count ≠ OSPF cost)
*   BGP policy attributes (AS_PATH, LOCAL_PREF) are preserved across IGP boundaries
*   This flexibility is essential for the scalability and heterogeneity of the Internet

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RIPConfig.xml <../RIPConfig.xml>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
