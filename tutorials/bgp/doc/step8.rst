Step 8. BGP with OSPF and RIP redistribution
============================================

Goals
-----

Step 8 demonstrates BGP's ability to act as a unified routing plane over a
heterogeneous internal landscape. In this scenario (``BGP_Topology_4.ned``),
different Autonomous Systems use different IGPs, and BGP is responsible for
providing seamless connectivity between them.

The setup is as follows:

- **AS 64500 (RA)**: Uses **RIP** as its internal routing protocol.
- **AS 64700 (RC)**: Also uses **RIP** internally.
- **AS 64600 (RB)**: Uses **OSPF** as its internal routing protocol.

Key features demonstrated:

- **Hybrid Redistribution**: Border routers are configured to redistribute routes from both RIP and OSPF into BGP.
- **Protocol Interworking**: BGP carries routes from a RIP-based domain, across
  an OSPF-based transit domain, to another RIP-based domain.
- **Unified Reachability**: Demonstrating that end-to-end connectivity (e.g.,
  host0 to host1) is independent of the specific IGPs used within each AS,
  provided BGP is correctly configured.

Configuration
~~~~~~~~~~~~~

The topology is the same as in Step 6 (``BGP_Topology_4.ned``).

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` defines the protocol mix:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step8
   :end-before: ------

The BGP configuration enables redistribution for both protocols:

.. code-block:: ini

   *.R*.bgp.redistributeRip = true
   *.R*.bgp.redistributeOspf = "O IA"

The BGP configuration:

.. literalinclude:: ../BGPConfig_Redist.xml
   :language: xml

Results
~~~~~~~

The simulation results in ``step8.rt`` show that BGP successfully bridges the different protocols:

1. **IGP Convergence**:

   - Routers in RA and RC reach internal convergence using RIP.
   - Routers in RB reach internal convergence using OSPF.

2. **Redistribution into BGP**:

   - RA4 and RC1 redistribute their RIP-learned routes into BGP.
   - RB1 and RB4 redistribute their OSPF-learned routes (including paths to IBGP peers) into BGP.

3. **BGP Propagation**:

   - Routes from RA (10.x.x.x) are carried across RB via OSPF-enabled IBGP sessions and delivered to RC.
   - Routes from RC (30.x.x.x) are carried across RB and delivered to RA.

4. **End-to-End Success**: Host0 in the RIP-based AS 64500 can successfully ping
   Host1 in the RIP-based AS 64700, transiting through the OSPF-based AS 64600.

This step highlights BGP's role as the "glue" of the Internet, enabling
communication between networks with entirely different internal architectures.

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RIPConfig.xml <../RIPConfig.xml>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1085>`__ in
the GitHub issue tracker for commenting on this tutorial.
