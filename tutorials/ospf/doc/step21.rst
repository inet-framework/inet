Step 21. Virtual link - connect two separate parts of a discontinuous backbone
==============================================================================

Goals
-----

The goal of this step is to demonstrate using virtual links to connect a partitioned backbone.

If the backbone area (Area 0) becomes partitioned (split into disconnected parts), OSPF
inter-area routing fails because Summary LSAs can only be exchanged through a contiguous
backbone. Virtual links can reconnect the backbone partitions through a transit area,
restoring full connectivity.

Configuration
~~~~~~~~~~~~~

This step simulates a partitioned backbone and uses a virtual link to reconnect it.

.. figure:: media/VirtualLink.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step21
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Virtual_Discontinuous.xml
   :language: xml

Results
~~~~~~~

With a partitioned backbone:

1.  Without virtual links, the two parts of Area 0 cannot exchange Summary LSAs.
2.  Inter-area routing between the partitioned sections fails.
3.  A virtual link is configured between two ABRs, one in each partition, through a transit area.
4.  The virtual link acts as a logical Area 0 connection.
5.  Summary LSAs can now be exchanged between the partitions.
6.  Full inter-area routing is restored.

The routing tables show that routes are successfully computed across the previously partitioned
backbone using the virtual link as a bridge.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VirtualLink.ned <../VirtualLink.ned>`,
:download:`ASConfig_Virtual_Discontinuous.xml <../ASConfig_Virtual_Discontinuous.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
