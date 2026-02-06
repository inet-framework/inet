Step 18b. AS-External LSAs of 'type 2 metric' with different advertised destination
===================================================================================

Goals
-----

The goal of this step is to demonstrate AS-External LSAs with Type-2 metrics.

Type-2 (E2) is the default external metric type in OSPF. With Type-2 metrics:

*   The cost to the external destination is ONLY the external cost advertised by the ASBR
*   Internal OSPF cost to reach the ASBR is NOT added to the metric
*   If multiple ASBRs advertise the same destination with the same external cost, the ASBR
    with the lowest internal cost is preferred (used as a tiebreaker)

Type-2 is appropriate when external costs are much larger than internal OSPF costs.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 18, using Type-2 metrics instead of Type-1.

.. figure:: media/OSPF_Area_External.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18b
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Type2.xml
   :language: xml

Results
~~~~~~~

With Type-2 external routes:

1.  ASBRs generate AS-External LSAs with Type-2 metric (E2).

2.  Routers calculate routes using ONLY the external cost.

3.  The route cost displayed in routing tables is the external cost, regardless of how far
    the ASBR is from the router.

4.  All routers see the same cost for a given external route.

5.  Internal topology has minimal impact on external route selection (only used as tiebreaker).

Type-2 is useful when external metrics represent costs in a different domain (e.g., BGP AS-path
length) that shouldn't be mixed with internal OSPF costs.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External.ned <../OSPF_Area_External.ned>`,
:download:`ASConfig_Area_ExternalRoute_Type2.xml <../ASConfig_Area_ExternalRoute_Type2.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
