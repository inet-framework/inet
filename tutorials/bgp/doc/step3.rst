Step 3. BGP Path Attributes
===========================

Goals
-----

This step explores a more complex multi-AS topology comprising four Autonomous
Systems (AS 64500 to AS 64800) with a total of 16 routers. The border routers
(RA4, RB1, RC2, and RD3) are interconnected in a full mesh of E-BGP sessions.

The primary objectives are:

- Observed path selection based on the ``AS_PATH`` attribute.
- Verify reachability across multiple ASes using OSPF-to-BGP redistribution.

In this scenario, every border router learns paths to every other AS through
multiple neighbors. BGP selects the best path primarily based on the shortest
``AS_PATH`` length.

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Topology_1a.png
   :width: 100%
   :align: center

.. literalinclude:: ../BGP_Topology_1a.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step3
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_FullAS.xml
   :language: xml

Results
~~~~~~~

The simulation begins with OSPF convergence within each AS. At 60 seconds, the
BGP sessions are initiated.

Because the border routers are fully meshed, each router has a direct path to
every other AS (distance 1) and several indirect paths (distance 2 through other
border routers).

Looking at the routing tables (``step3.rt``), we can observe:

- Border routers prefer the direct connection because it has the shortest ``AS_PATH``.
- For example, RA4 will install RD's networks (40.0.0.0/30 subnets) with a next
  hop of RD3 (via the RA4-RD3 link), ignoring the longer paths offered by RB1 or
  RC2.
- Due to redistribution, full reachability is achieved between all subnets in
  the simulation. You can verify this by checking that internal routers (e.g.,
  RA1) eventually populate their tables with routes to the other ASes via their
  respective border routers.

Sources: :download:`BGP_Topology_1a.ned <../BGP_Topology_1a.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig_FullAS.xml <../OSPFConfig_FullAS.xml>`,
:download:`BGPConfig_FullAS.xml <../BGPConfig_FullAS.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
