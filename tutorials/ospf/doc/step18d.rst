Step 18d. AS-External LSAs of mixed 'type 1/type 2 metric' with the same advertised destination
===============================================================================================

Goals
-----

The goal of this step is to demonstrate OSPF's behavior when multiple ASBRs advertise the
same external destination using different metric types (Type-1 and Type-2).

OSPF always prefers Type-1 (E1) routes over Type-2 (E2) routes for the same destination,
regardless of the metric values. This is because E1 routes include internal topology in
the cost calculation, making them more accurate.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 18. Some ASBRs use Type-1 metrics while others use
Type-2 metrics for the same destination.

.. figure:: media/OSPF_Area_External.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18d
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Mixed_Dest.xml
   :language: xml

Results
~~~~~~~

When ASBRs advertise the same destination with mixed metric types:

1.  OSPF **always prefers E1 (Type-1) routes** over E2 (Type-2) routes.

2.  Even if an E2 route has a lower advertised cost, the E1 route will be selected.

3.  This preference reflects OSPF's design: E1 routes provide more accurate end-to-end
    cost calculation by including internal topology.

4.  All routers will select an ASBR advertising E1, even if they are closer to an ASBR
    advertising E2.

The routing tables clearly show the preference for E1 routes, demonstrating OSPF's route
selection hierarchy: Intra-area > Inter-area > E1 > E2.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External.ned <../OSPF_Area_External.ned>`,
:download:`ASConfig_Area_ExternalRoute_Mixed_Dest.xml <../ASConfig_Area_ExternalRoute_Mixed_Dest.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
