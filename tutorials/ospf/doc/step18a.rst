Step 18a. AS-External LSAs of 'type 1 metric' with the same advertised destination
==================================================================================

Goals
-----

The goal of this step is to demonstrate how OSPF selects between multiple ASBRs advertising
the same external destination with Type-1 metrics.

When multiple ASBRs advertise the same external network using Type-1 metrics, each router
selects the best ASBR based on the total cost (external cost + internal cost to ASBR).

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 18. Multiple ASBRs advertise the same external destination.

.. figure:: media/OSPF_Area_External.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Type1_Dest.xml
   :language: xml

Results
~~~~~~~

With multiple ASBRs advertising the same destination:

1.  Each ASBR originates an AS-External LSA for the same network prefix.

2.  The LSAs may have different external costs.

3.  Each router calculates the total cost to reach the destination via each ASBR:
    **Total Cost via ASBR_X = External Cost (ASBR_X) + Internal Cost to ASBR_X**

4.  The router selects the ASBR with the lowest total cost.

5.  Routers in different parts of the topology may select different ASBRs based on their
    internal costs to each ASBR.

This demonstrates OSPF's ability to perform intelligent load balancing and path selection
for external routes when using Type-1 metrics.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External.ned <../OSPF_Area_External.ned>`,
:download:`ASConfig_Area_ExternalRoute_Type1_Dest.xml <../ASConfig_Area_ExternalRoute_Type1_Dest.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
