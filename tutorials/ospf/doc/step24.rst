Step 24. OSPF Path Selection - Suboptimal routes
=================================================

Goals
-----

The goal of this step is to demonstrate scenarios where OSPF's route selection hierarchy can
lead to suboptimal routing.

Because OSPF always prefers intra-area routes over inter-area routes (regardless of cost), there
are situations where the protocol selects a longer path within an area instead of a shorter
path that crosses area boundaries.

This is an intentional design trade-off: OSPF prioritizes routing stability and hierarchical
structure over always finding the absolute shortest path.

Configuration
~~~~~~~~~~~~~

This step uses a topology where an intra-area path is longer than an available inter-area path.

.. figure:: media/OSPF_Suboptimal.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step24
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Suboptimal.xml
   :language: xml

Results
~~~~~~~

The simulation demonstrates suboptimal routing:

1.  host0 pings host6.

2.  An inter-area path R1 → R7 exists with low cost.

3.  However, an intra-area path R1 → R2 → R4 → R5 → R3 → R7 also exists.

4.  OSPF selects the longer intra-area path because intra-area routes are always preferred
    over inter-area routes.

5.  The traffic follows the suboptimal path: R1 → R2 → R4 → R5 → R3 → R7 → 10.0.0.52

This demonstrates that OSPF's route preference hierarchy can lead to suboptimal paths in
certain topologies, which is an acceptable trade-off for the benefits of hierarchical routing
(scalability, stability, summarization).

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Suboptimal.ned <../OSPF_Suboptimal.ned>`,
:download:`ASConfig_Suboptimal.xml <../ASConfig_Suboptimal.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
