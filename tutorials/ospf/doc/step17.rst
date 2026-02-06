Step 17. Loop avoidance in multi-area OSPF topology
===================================================

Goals
-----

The goal of this step is to demonstrate how OSPF prevents routing loops in
multi-area topologies.

In multi-area OSPF, there is a risk of routing loops if areas are not properly
connected to the backbone (Area 0). OSPF has a rule that inter-area routes
(learned via Summary LSAs) are only accepted from the backbone area. This
ensures a hierarchical topology and prevents loops.

If an area is not directly connected to the backbone, it must use a virtual link
to logically connect to Area 0.

Configuration
~~~~~~~~~~~~~

This step uses the ``OSPF_LoopAvoidance`` network with a topology that could
create loops if not properly configured.

.. figure:: media/OSPF_LoopAvoidance.png
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step17
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Loop.xml
   :language: xml

Results
~~~~~~~

The configuration demonstrates potential loop scenarios:

1.  Areas are configured in a way that could create routing loops.

2.  OSPF's loop prevention mechanisms ensure that:

    *   Summary LSAs from non-backbone areas are ignored by ABRs
    *   Only Summary LSAs received via the backbone are accepted for inter-area routing

3.  If the topology violates OSPF's hierarchical requirements (non-backbone area
    not connected to Area 0), routing may be suboptimal or certain networks may
    be unreachable.

4.  The routing tables show how OSPF enforces the hierarchical structure to
    prevent loops.

This demonstrates the importance of proper area design and the backbone's
central role in OSPF multi-area networks.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_LoopAvoidance.ned <../OSPF_LoopAvoidance.ned>`,
:download:`ASConfig_Area_Loop.xml <../ASConfig_Area_Loop.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
