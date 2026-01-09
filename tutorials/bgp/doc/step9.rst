Step 9. Using Network attribute to advertise specific networks
==============================================================

Goals
-----

Step 9 focuses on the ``Network`` attribute in BGP, which provides a manual and
selective way to advertise network prefixes. In previous steps (6, 7, and 8), we
relied on automatic redistribution from IGPs (OSPF/RIP), which often advertises
all known internal routes to BGP peers.

In a real-world scenario, a network administrator might want to advertise only
specific prefixes to the public Internet or to a specific partner AS, while
keeping other internal subnets private. The ``Network`` attribute allows for
this fine-grained control.

Key features demonstrated:

- **Selective Advertisement**: Using the ``Network`` element in the BGP
  configuration XML to explicitly list the subnets to be advertised.
- **Independence from IGP Redistribution**: In this step, OSPF is running for
  internal connectivity, but the automatic redistribution into BGP is disabled.
  Only the prefixes listed in the XML are propagated via BGP.
- **Complex Topology Verification**: Verifying that even in a multi-AS transit
  topology (``BGP_Topology_4.ned``), manually advertised routes can still
  provide end-to-end reachability for the selected networks.

Configuration
~~~~~~~~~~~~~

The topology is the same as in Step 6 (``BGP_Topology_4.ned``).

.. figure:: media/BGP_Topology_4.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` enables OSPF but does not set ``redistributeOspf``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step9
   :end-before: ------

The BGP configuration (``BGPConfig.xml``) uses ``Network`` elements to define the advertised prefixes for each AS:

.. literalinclude:: ../BGPConfig.xml
   :language: xml
   :start-at: <AS id="64500">
   :end-before: <!--bi-directional

Results
~~~~~~~

The simulation results in ``step9.rt`` confirm that prefix propagation is limited to the defined subnets:

1. **Internal Convergence**: OSPF ensures that routers within each AS can reach all internal subnets.
2. **Selective BGP Injection**: Border routers (like RA4 and RC1) only inject
   the prefixes specifically mentioned in ``BGPConfig.xml`` (e.g., 10.0.0.0/30,
   30.0.0.0/30) into their BGP sessions.
3. **Restricted Routing Tables**: Unlike Step 6, where all OSPF-learned routes
   were visible across the entire network, we now observe that routers only
   learn BGP routes for the manually advertised prefixes.
4. **Targeted Reachability**: Connectivity is verified only for the subnets
   included in BGP. For example, if a subnet in RA was omitted from the
   ``Network`` list, it would remain unreachable forRC, even if RA's internal
   routers knew about it via OSPF.

This step demonstrates how BGP's ``Network`` attribute is fundamental for
implementing routing policies and controlling the information shared with the
outside world.

Sources: :download:`BGP_Topology_4.ned <../BGP_Topology_4.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPFConfig.xml <../OSPFConfig.xml>`,
:download:`BGPConfig.xml <../BGPConfig.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/??>`__ in
the GitHub issue tracker for commenting on this tutorial.
