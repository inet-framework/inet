Step 4a. Enable nextHopSelf on RB1 and RB2
===========================================

Goals
-----

The goal of this step is to solve the NEXT_HOP reachability problem from Step 4
by using the **next-hop-self** feature.

In Step 4, we saw that I-BGP preserves the NEXT_HOP attribute from E-BGP routes,
which can cause problems when I-BGP peers cannot reach the external NEXT_HOP address.
The solution is to configure I-BGP routers to change the NEXT_HOP to themselves when
advertising routes to I-BGP peers.

**Next-Hop-Self** is a BGP feature where a router sets itself as the NEXT_HOP when
advertising routes to I-BGP peers. This ensures that:

*   I-BGP peers can reach the NEXT_HOP (it's the advertising router itself)
*   Traffic flows through the border router that learned the route
*   The IGP (or direct connectivity) within the AS can be used to reach the border router

This is a common configuration in enterprise and service provider networks where
border routers advertise external routes into the AS via I-BGP.

Configuration
~~~~~~~~~~~~~

This step uses the same network topology as Step 4:

.. figure:: media/BGP_Topology_2.png
   :width: 100%
   :align: center

The only difference is in the BGP configuration. This configuration extends Step 4:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: [Config Step4a]
   :end-before: ------

The key change is using a different BGP configuration file:

.. literalinclude:: ../BGPConfig_IBGP_NextHopSelf.xml
   :language: xml

The new configuration includes ``nextHopSelf='true'`` for the I-BGP neighbors:

*   RB1 sets next-hop-self when advertising to RB2
*   RB2 sets next-hop-self when advertising to RB1

Results
~~~~~~~

With next-hop-self enabled, the simulation works correctly:

1.  **OSPF Convergence** (t≈0-40s):

    *   OSPF runs in AS 64500 (RA4) and AS 64700 (RC1)
    *   Default routes are distributed

2.  **BGP Session Establishment** (t≈60s):

    *   E-BGP sessions: RA4-RB1 and RB2-RC1
    *   I-BGP session: RB1-RB2

3.  **Route Exchange with Modified NEXT_HOP** (t≈60-62s):

    *   RA4 advertises AS 64500's networks to RB1
        
        - RB1 receives with NEXT_HOP = 192.168.0.2 (RA4)
        - RB1 advertises to RB2 with NEXT_HOP = 20.0.0.2 (RB1 itself)
        - RB2 can now reach the NEXT_HOP (directly connected to RB1)
    
    *   RC1 advertises AS 64700's networks to RB2
        
        - RB2 receives with NEXT_HOP = 192.168.0.2 (RC1)
        - RB2 advertises to RB1 with NEXT_HOP = 20.0.0.1 (RB2 itself)
        - RB1 can now reach the NEXT_HOP (directly connected to RB2)

4.  **Successful Routing**:

    *   RB2 installs routes to AS 64500 with next hop = RB1 (20.0.0.2)
    *   RB1 installs routes to AS 64700 with next hop = RB2 (20.0.0.1)
    *   Both border routers redistribute BGP routes into OSPF for their respective ASes

5.  **End-to-End Connectivity**:

    *   Traffic from AS 64500 to AS 64700 flows: RA4 → RB1 → RB2 → RC1
    *   Traffic from AS 64700 to AS 64500 flows: RC1 → RB2 → RB1 → RA4
    *   The next-hop-self configuration ensures packets are routed to the correct
        border router within AS 64600

By setting next-hop-self, the I-BGP routers ensure that their peers can reach the
NEXT_HOP, solving the reachability problem and enabling proper inter-AS routing.

This is a best practice for I-BGP configurations, especially when:

*   Border routers are not running a full-mesh IGP
*   E-BGP peer addresses are not advertised in the IGP
*   You want to centralize routing decisions at the border routers

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_IBGP.xml <../OSPFConfig_IBGP.xml>`,
:download:`BGPConfig_IBGP_NextHopSelf.xml <../BGPConfig_IBGP_NextHopSelf.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
