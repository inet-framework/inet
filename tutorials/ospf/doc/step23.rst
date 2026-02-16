Step 23. OSPF Path Selection
=============================

Goals
-----

The goal of this step is to demonstrate OSPF's route selection hierarchy when multiple paths
to the same destination exist with different route types.

OSPF prioritizes routes in the following order:

1.  **Intra-area routes** (routes within the same area)
2.  **Inter-area routes** (routes learned via Summary LSAs from other areas)
3.  **External Type-1 routes** (E1)
4.  **External Type-2 routes** (E2)

This hierarchy ensures that OSPF prefers routes with the most accurate cost information and
maintains the hierarchical area structure.

Configuration
~~~~~~~~~~~~~

This step uses a topology where the same destination can be reached via different route types.

.. figure:: media/OSPF_Route_Selection.png
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step23
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Route_Selection.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates route selection hierarchy:

1.  Multiple routers advertise the same destination (1.1.1.0/24) using different methods:

    - R2: Intra-area (RouterLsa), cost 1
    - R3: Inter-area (SummaryLsa), cost 2
    - R4: External Type-1 (NetworkLsa), cost 10
    - R5: External Type-2 (NetworkLsa), cost 1

2.  At t=60s, the link between R1 and R2 breaks, changing available paths.

3.  At t=70s, another link breaks, further changing available paths.

4.  Throughout these changes, OSPF consistently applies its route preference hierarchy.

5.  Intra-area routes are always preferred over inter-area routes, even if the inter-area
    route has lower cost.

6.  Similarly, inter-area routes are preferred over external routes.

The routing tables clearly show OSPF selecting routes based on type hierarchy, not just
cost, demonstrating the protocol's design for stability and hierarchical routing.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Route_Selection.ned <../OSPF_Route_Selection.ned>`,
:download:`ASConfig_Route_Selection.xml <../ASConfig_Route_Selection.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
