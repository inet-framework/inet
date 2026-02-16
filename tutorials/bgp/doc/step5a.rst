Step 5a. BGP internal distribution
==================================

Goals
-----

Step 5a presents the first solution to the "BGP Hole" problem identified in Step
5. By enabling redistribution of BGP internal routes into the Interior Gateway
Protocol (OSPF), we ensure that non-BGP speakers within the transit AS learn how
to reach external destinations.

In this configuration, the ``redistributeInternal`` attribute is set to ``true``
on border routers RB1 and RB2. This instructs the BGP module to take routes
learned from I-BGP peers and inject them into the local OSPF instance.

Configuration
~~~~~~~~~~~~~

This step extends Step 5.

.. figure:: media/BGP_Topology_3.png
   :width: 100%
   :align: center

The key change in ``omnetpp.ini`` is:

.. code-block:: ini

   *.RB{1,2}.bgp.redistributeInternal = true

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5a
   :end-before: ------

Results
~~~~~~~

With internal redistribution enabled, we can observe the following in ``step5a.rt``:

- RB1 and RB2 continue to exchange routes via I-BGP.
- RB1 redistributes the routes it learns from RA into OSPF.
- **RB3 receives these routes via OSPF**. Check RB3's routing table (20.0.0.6)
  and you will see entries for the 10.0.0.x and 30.0.0.x networks.
- Reachability is now established: packets from AS 64500 to AS 64700 can
  successfully transit RB3 because RB3 now has OSPF routes to those external
  subnets.

While this approach works for small networks, it is generally discouraged in
large-scale internet routing because it can overwhelm the IGP with a massive
number of external routes.

Sources: :download:`BGP_Topology_3.ned <../BGP_Topology_3.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_Multi.xml <../OSPFConfig_Multi.xml>`,
:download:`BGPConfig_Multi.xml <../BGPConfig_Multi.xml>`


Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1085>`__ in
the GitHub issue tracker for commenting on this tutorial.
