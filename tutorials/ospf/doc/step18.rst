Step 18. AS-External LSAs of 'type 1 metric' with different advertised destination
==================================================================================

Goals
-----

The goal of this step is to demonstrate AS-External LSAs (Type-5 LSAs) with Type-1 metrics.

OSPF routers can advertise routes to destinations outside the OSPF domain using AS-External
LSAs. These external routes can use two metric types:

*   **Type-1 (E1)**: The metric is the sum of the external cost plus the internal OSPF cost
    to reach the ASBR (Autonomous System Boundary Router). This allows OSPF to select the
    closest exit point.
*   **Type-2 (E2)**: The metric is only the external cost; internal cost is ignored. Type-2
    is the default.

This step demonstrates Type-1 metrics with different external destinations.

Configuration
~~~~~~~~~~~~~

This step uses the ``OSPF_Area_External`` network with ASBRs advertising external routes.

.. figure:: media/OSPF_Area_External.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Type1.xml
   :language: xml

Results
~~~~~~~

With Type-1 external routes:

1.  ASBRs (e.g., R3, R5) generate AS-External LSAs for external networks.

2.  The LSAs specify Type-1 metric (E1).

3.  When routers calculate routes to these external destinations, they use:
    **Total Cost = External Cost + Internal Cost to ASBR**

4.  Routers select the ASBR with the lowest total cost. This may result in
    choosing a closer ASBR even if it advertises a higher external cost, as long
    as the total cost remains lower.

5.  Different routers may choose different ASBRs for the same destination based on their
    location in the topology.

Type-1 metrics are useful when the external cost is comparable to internal OSPF costs and
you want traffic to exit the AS at the nearest point.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External.ned <../OSPF_Area_External.ned>`,
:download:`ASConfig_Area_ExternalRoute_Type1.xml <../ASConfig_Area_ExternalRoute_Type1.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
