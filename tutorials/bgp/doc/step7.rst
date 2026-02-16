Step 7. BGP with RIP redistribution
===================================

Goals
-----

Step 7 demonstrates BGP route redistribution from the Routing Information
Protocol (RIP). While OSPF is a common IGP in modern networks, BGP is flexible
enough to interact with several other routing protocols, including RIP.

In this scenario (``BGP_Topology_4.ned``), each Autonomous System uses RIP as
its internal routing protocol instead of OSPF. The border routers (RA4, RB1,
RB4, RC1) are configured to take the routes they learn from RIP and advertise
them to their BGP peers.

Key features demonstrated:

- **RIP as an IGP**: All routers within each AS run RIP to discover internal subnets.
- **RIP-to-BGP Redistribution**: The ``redistributeRip`` parameter is set to ``true`` in the BGP configuration, allowing border routers to inject RIP-learned routes into the BGP table for advertisement to external ASes.
- **Multiprotocol Interworking**: Verifying that end-to-end connectivity is maintained when BGP is used to bridge multiple RIP-based domains.

Configuration
~~~~~~~~~~~~~

The topology remains the same as in Step 6 (``BGP_Topology_4.ned``).

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables RIP and disables OSPF:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7
   :end-before: ------

The BGP configuration uses ``redistributeRip = true``:

.. code-block:: ini

   *.R*.bgp.redistributeRip = true

Results
~~~~~~~

The simulation results in ``step7.rt`` confirm that RIP-learned routes successfully propagate via BGP:

1. **RIP Convergence**: Routers within each AS exchange RIP updates. For
   example, in AS 64700, RC1 learns about the networks connected to RC2, RC3,
   and RC4 via RIP.
2. **Redistribution**: RC1 takes these RIP routes (30.0.0.8/30, 30.0.0.12/30,
   etc.) and advertises them to its E-BGP peer, RB4.
3. **BGP Propagation**: RB4 receives these routes and advertises them via I-BGP
   to RB1, who then advertises them via E-BGP to RA4.
4. **End-to-End Connectivity**: Like in previous steps, all routers eventually
   learn routes to all possible destinations in the network. Routing table
   entries for 30.x.x.x networks appear in AS 64500 routers, and vice-versa,
   with BGP serving as the conduit.

This step shows that BGP's redistribution mechanism is protocol-agnostic, making it a powerful tool for connecting diverse network islands.

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RIPConfig.xml <../RIPConfig.xml>`,
:download:`BGPConfig_Redist.xml <../BGPConfig_Redist.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1085>`__ in
the GitHub issue tracker for commenting on this tutorial.
