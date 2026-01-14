Step 2. BGP Scenario with E-BGP session only
============================================

Goals
-----

The goal of this step is to demonstrate BGP in a more realistic scenario where:

*   Each AS has an internal network with multiple routers
*   OSPF is used as the Interior Gateway Protocol (IGP) within each AS
*   BGP border routers redistribute OSPF routes into BGP
*   BGP routes from external ASes are redistributed into OSPF for internal distribution

This illustrates a common deployment pattern where BGP handles inter-AS routing
while an IGP (OSPF) handles intra-AS routing. The interaction between BGP and OSPF
through redistribution is a key concept in enterprise and service provider networks.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_1.png
   :width: 100%
   :align: center

The network consists of two autonomous systems:

*   **AS 64500** (routers RA1-RA4): Uses OSPF for internal routing
*   **AS 64600** (routers RB1-RB4): Uses OSPF for internal routing

An E-BGP session is established between the border routers:

*   **RA4** (10.0.0.5) in AS 64500
*   **RB1** (20.0.0.18) in AS 64600

Both routers run OSPF internally and BGP externally. They redistribute:

*   OSPF routes (learned from their AS) into BGP for advertisement to the peer AS
*   BGP routes (learned from the peer AS) into OSPF for distribution within their AS

.. literalinclude:: ../BGP_Topology_1.ned
   :language: ned
   :start-at: network BGP

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step2
   :end-before: ------

Key configuration parameters:

*   ``*.R*.hasOspf = true``: Enables OSPF on all routers
*   ``*.RA4.hasBgp = true`` and ``*.RB1.hasBgp = true``: Enables BGP on border routers
*   ``*.RA4.bgp.redistributeOspf = "O IA"``: RA4 redistributes OSPF inter-area routes into BGP
*   ``*.RB1.bgp.redistributeOspf = "O IA"``: RB1 redistributes OSPF inter-area routes into BGP

The BGP configuration:

.. literalinclude:: ../BGPConfig_EBGP.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates the following sequence:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF runs within each AS, distributing internal routes
    *   Each router learns routes to all networks within its AS via OSPF
    *   Default routes propagate through each AS from the border routers

2.  **BGP Session Establishment** (t≈60s):

    *   After OSPF converges, the BGP session between RA4 and RB1 is established
    *   The startDelay is set to 60s to ensure OSPF has converged first

3.  **Route Redistribution** (t≈60-62s):

    *   RA4 redistributes AS 64500's internal networks into BGP and advertises them to RB1
    *   RB1 redistributes AS 64600's internal networks into BGP and advertises them to RA4
    *   Each border router learns the other AS's networks via BGP

4.  **BGP-to-OSPF Redistribution**:

    *   RA4 redistributes the BGP-learned routes (AS 64600's networks) into OSPF
    *   RB1 redistributes the BGP-learned routes (AS 64500's networks) into OSPF
    *   OSPF propagates these external routes throughout each AS

5.  **Full Connectivity**:

    *   All routers in AS 64500 learn routes to AS 64600's networks via OSPF (installed by RA4)
    *   All routers in AS 64600 learn routes to AS 64500's networks via OSPF (installed by RB1)
    *   End-to-end connectivity is established between the two ASes

The routing table changes show that initially only OSPF routes exist within each AS.
After the BGP session is established and routes are redistributed, routers throughout
each AS learn about the remote AS's networks, enabling inter-AS communication.

Sources: :download:`BGP_Topology_1.ned <../BGP_Topology_1.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_EBGP.xml <../OSPFConfig_EBGP.xml>`,
:download:`BGPConfig_EBGP.xml <../BGPConfig_EBGP.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
