Step 20. Stub area
==================

Goals
-----

The goal of this step is to demonstrate OSPF stub areas.

A stub area is an area that does not receive AS-External LSAs (Type-5). Instead, the ABR
injects a default route into the stub area, reducing the LSDB size and memory requirements
for routers in that area.

Stub areas are useful for:

*   Reducing memory and CPU usage in routers with limited resources
*   Simplifying routing in areas with limited external connectivity (single exit point)

All routers in a stub area must be configured as stub; otherwise, adjacencies will not form.

Configuration
~~~~~~~~~~~~~

This step configures an area as a stub area.

.. figure:: media/OSPF_Stub.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step20
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Stub_Area.xml
   :language: xml

Results
~~~~~~~

In a stub area configuration:

1.  The ABR does NOT flood AS-External LSAs (Type-5) into the stub area.

2.  Instead, the ABR generates a default route Summary LSA (0.0.0.0/0) into the stub area.

3.  Routers in the stub area have smaller LSDBs (no external LSAs).

4.  Traffic destined for external networks uses the default route to the ABR.

5.  Intra-area and inter-area routing works normally.

The stub area flag is negotiated in Hello packets. If there's a mismatch, routers will not
form adjacencies, as seen in the OSPF module logs.

Stub areas provide significant scalability benefits for areas that don't need detailed external
routing information.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Stub.ned <../OSPF_Stub.ned>`,
:download:`ASConfig_Stub_Area.xml <../ASConfig_Stub_Area.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
