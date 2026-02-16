Step 18c. AS-External LSAs of 'type 2 metric' with the same advertised destination
==================================================================================

Goals
-----

The goal of this step is to demonstrate how OSPF selects between multiple ASBRs advertising
the same external destination with Type-2 metrics.

When multiple ASBRs advertise the same destination with Type-2 metrics, the selection process
is:

1.  **Primary**: Compare external costs - lowest wins
2.  **Tiebreaker** (if external costs are equal): Use internal cost to ASBR - lowest wins

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 18b. Multiple ASBRs advertise the same external destination
with Type-2 metrics.

.. figure:: media/OSPF_Area_External.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18c
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Type2_Dest.xml
   :language: xml

Results
~~~~~~~

With multiple ASBRs using Type-2 metrics for the same destination:

1.  Each router compares the external costs from all ASBRs.

2.  The ASBR with the lowest external cost is preferred.

3.  If external costs are equal, the ASBR with the lowest internal cost (closest to the router)
    is selected.

4.  All routers in the network see the same external cost, but may select different ASBRs based
    on internal costs if external costs are equal.

This demonstrates Type-2's preference for external cost over internal topology, while still
using internal cost for intelligent ASBR selection when needed.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External.ned <../OSPF_Area_External.ned>`,
:download:`ASConfig_Area_ExternalRoute_Type2_Dest.xml <../ASConfig_Area_ExternalRoute_Type2_Dest.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
