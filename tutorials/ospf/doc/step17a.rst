Step 17a. Make R3 an ABR - advertise its loopback to backbone
=============================================================

Goals
-----

The goal of this step is to demonstrate one solution to connectivity issues in disconnected
areas: making a router an ABR by assigning one of its interfaces (e.g., a loopback) to the
backbone area.

When an area is not directly connected to Area 0, connectivity problems arise. One solution
is to create an ABR by assigning one of the router's interfaces to Area 0, effectively
connecting the isolated area to the backbone.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 17. Router R3 is made an ABR by advertising its loopback
interface in Area 0.

.. figure:: media/OSPF_LoopAvoidance.png
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step17a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Loop_ABR.xml
   :language: xml

Results
~~~~~~~

By making R3 an ABR (with presence in both Area 0 and another area):

1.  R3 now has interfaces in the backbone area (its loopback) and in another area.

2.  R3 can generate Summary LSAs for both areas, providing inter-area connectivity.

3.  Routes that were previously unreachable or suboptimal are now properly computed.

4.  This solves some connectivity issues but may not be the optimal solution for all topologies.

This demonstrates one approach to connecting areas, though virtual links (shown in later steps)
are often more appropriate for complex topologies.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_LoopAvoidance.ned <../OSPF_LoopAvoidance.ned>`,
:download:`ASConfig_Area_Loop_ABR.xml <../ASConfig_Area_Loop_ABR.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
