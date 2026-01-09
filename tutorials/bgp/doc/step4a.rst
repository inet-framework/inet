Step 4a. Enable nextHopSelf on RB1 and RB2
==========================================

Goals
-----

Step 4a provides a solution to the unreachable next hop issue observed in Step
4. Within AS 64600, the border routers RB1 and RB2 are configured with the
``nextHopSelf`` attribute enabled for their I-BGP session.

The goal of this configuration is to ensure that when a border router advertises
a route learned from an external peer (E-BGP) to its internal peer (I-BGP), it
updates the ``NEXT_HOP`` attribute to its own address. This makes the route
reachable for the internal peer, provided there is internal connectivity (IGP or
direct link) between them.

Configuration
~~~~~~~~~~~~~

This step uses the same network as Step 4 (``BGP_Topology_2.ned``).

.. figure:: media/BGP_Topology_2.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables the feature:

.. code-block:: ini

   *.RA4.bgp.nextHopSelf = true
   *.RB1.bgp.nextHopSelf = true
   *.RB2.bgp.nextHopSelf = true
   *.RC1.bgp.nextHopSelf = true

The BGP configuration uses a specific XML file:

.. literalinclude:: ../BGPConfig_IBGP_NextHopSelf.xml
   :language: xml

Results
~~~~~~~

With ``nextHopSelf`` enabled, the BGP behavior changes as follows:

1. RB1 receives the RA network (10.0.0.0/30) route from RA4.
2. When RB1 advertises this route to RB2 via I-BGP, it now replaces RA4's
   interface address with its own address (the address of RB1's interface on the
   link to RB2).
3. RB2 receives the advertisement. Since RB1 is directly connected, RB2 can
   resolve the next hop and successfully installs the route into its IP routing
   table.

Checking ``step4a.rt``, we can see the successful installation:

- RB2 now has routes to RA's networks (10.0.0.x) with the next hop set to RB1 (``20.0.0.2``).
- Similarly, RB1 now has routes to RC's networks (30.0.0.x) with the next hop set to RB2 (``20.0.0.1``).

Full bi-directional reachability across the transit AS 64600 is now achieved.

Sources: :download:`BGP_Topology_2.ned <../BGP_Topology_2.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_IBGP_NextHopSelf.xml <../BGPConfig_IBGP_NextHopSelf.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
