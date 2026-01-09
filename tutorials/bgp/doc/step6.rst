Step 6. BGP Scenario and using loopbacks
========================================

Goals
-----

Step 6 introduces a more complex, three-AS hierarchical topology
(``BGP_Topology_4.ned``) and demonstrates a complete BGP-based transit scenario.
This step focuses on establishing a full-mesh I-BGP configuration within the
transit Autonomous System (AS 64600) and using BGP to provide end-to-end
connectivity between external ASes.

The network consists of:

- **AS 64500 (RA)**: A multi-router AS running OSPF internally. RA4 is the border router.
- **AS 64600 (RB)**: The transit AS. It contains four routers (RB1, RB2, RB3,
  RB4) running OSPF. RB1 and RB4 are border routers.
- **AS 64700 (RC)**: Another multi-router AS running OSPF. RC1 is the border router.

Key features demonstrated:

- **E-BGP Peering**: RA4 peers with RB1, and RB4 peers with RC1.
- **Full-Mesh I-BGP**: Within AS 64600, all routers (RB1, RB2, RB3, RB4) run BGP and are configured as I-BGP peers.
- **Multi-hop I-BGP**: Note that some I-BGP sessions (e.g., RB1 to RB4) are not
  directly connected. They rely on the IGP (OSPF) to provide reachability to the
  peering addresses.
- **Reachability**: The network is configured to allow hosts in AS 64500 (host0) to reach hosts in AS 64700 (host1).

Configuration
~~~~~~~~~~~~~

This step uses the ``BGP_Topology_4.ned`` network, which includes three distinct
ASes with several routers and hosts.

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step6
   :end-before: ------

The BGP configuration (``BGPConfig_Redist.xml``) defines the ASes and the
full-mesh sessions within AS 64600. It also uses ``nextHopSelf="true"`` to
ensure I-BGP peers can reach external next hops.

Results
~~~~~~~

In the simulation, we observe the convergence of the entire network:

1. **IGP Convergence**: OSPF converges within each AS, providing internal
   reachability. In AS 64600, this allows multi-hop BGP sessions (like RB1-RB4)
   to establish.
   
2. **BGP Session Establishment**:

   - EBGP sessions are established: RA4 <-> RB1 and RB4 <-> RC1.
   - A full mesh of IBGP sessions is established among RB1, RB2, RB3, and RB4.
3. **Route Propagation**:

   - RB1 learns RA's networks (10.x.x.x) and redistributes them into the RB mesh.
   - RB4 learns RC's networks (30.x.x.x) and redistributes them into the RB mesh.
   - Due to the full mesh, every router in AS 64600 learns the path to both external ASes.

4. **End-to-End Reachability**: ``step6.rt`` shows that all routers eventually
   possess routes to all subnets across all three ASes. Host0 can successfully
   communicate with Host1 through the transit AS 64600.

This step validates the use of a BGP full mesh within a transit AS as a robust solution for internet-scale routing.

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
