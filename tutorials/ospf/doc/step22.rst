Step 22. Virtual link - connect a disconnected area to the backbone
===================================================================

Goals
-----

The goal of this step is to demonstrate using virtual links to connect an area that is not
directly attached to the backbone.

OSPF requires all areas to be connected to Area 0 (the backbone). If an area cannot be
physically connected to the backbone, a virtual link through a transit area can provide the
logical connection required for proper OSPF operation.

Configuration
~~~~~~~~~~~~~

This step demonstrates an area that is not direct ly connected to Area 0, and a virtual link
is used to connect it.

.. figure:: media/VirtualLink_2.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step22
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Virtual_Disconnected.xml
   :language: xml

Results
~~~~~~~

When an area is disconnected from Area 0:

1.  Without a connection to the backbone, the disconnected area cannot exchange Summary LSAs
    properly.
2.  Routers in the disconnected area may have incomplete routing information.
3.  A virtual link is configured between an ABR in the disconnected area and an ABR in the
    backbone, through a transit area.
4.  The virtual link provides the required logical connection to Area 0.
5.  Summary LSAs can now flow between the previously disconnected area and the backbone.
6.  Full inter-area routing is established.

This demonstrates how virtual links maintain OSPF's hierarchical design principles even when
physical topology constraints prevent direct backbone connectivity.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VirtualLink_2.ned <../VirtualLink_2.ned>`,
:download:`ASConfig_Virtual_Disconnected.xml <../ASConfig_Virtual_Disconnected.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
