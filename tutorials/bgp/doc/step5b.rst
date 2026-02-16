Step 5b. Enabling BGP on RB3
============================

Goals
-----

Step 5b demonstrates the second (and preferred) solution to the transit
reachability problem: running BGP on all routers within the transit AS
(Full-mesh I-BGP).

In this scenario:

- **RB3** is now configured to run BGP.
- BGP sessions are established between all pairs of routers in AS 64600 (RB1-RB2, RB1-RB3, RB2-RB3).
- Every router learns external routes directly via BGP, eliminating the need to redistribute internal BGP routes into OSPF.

Configuration
~~~~~~~~~~~~~

This step extends Step 5.

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables BGP on RB3:

.. code-block:: ini

   *.RB3.hasBgp = true
   *.R*.bgp.bgpConfig = xmldoc("BGPConfig_Multi_RB3.xml")

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5b
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Multi_RB3.xml
   :language: xml

Results
~~~~~~~

In this simulation, the "BGP hole" is resolved through a full-mesh I-BGP
configuration:

1. **BGP Adjacencies**: RB3 establishes I-BGP sessions with both RB1 and RB2.
2. **Direct Route Learning**: Unlike Step 5, where RB3 was unaware of external
   routes, it now receives BGP UPDATE messages from RB1 (about RA's networks)
   and RB2 (about RC's networks).
3. **Successful Transit**: Because RB3 has these routes in its BGP table, it can
   successfully forward packets between RB1 and RB2.

This demonstrates the standard approach in large networks: every router in the
transit path between BGP border routers should also run BGP to ensure proper
packet forwarding without overwhelming the IGP with external routes.

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_Multi_RB3.xml <../BGPConfig_Multi_RB3.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1085>`__ in
the GitHub issue tracker for commenting on this tutorial.
