Step 17b. Make R3 an ABR - create a virtual link between R1 and R3
==================================================================

Goals
-----

The goal of this step is to demonstrate the use of OSPF virtual links to logically connect
an area to the backbone.

A virtual link is a logical connection through a transit area that allows two ABRs to appear
directly connected in Area 0. Virtual links are used when:

*   An area cannot be physically connected to the backbone
*   The backbone itself is partitioned and needs to be reconnected

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 17. A virtual Link is configured between R1 and R3
through a transit area.

.. figure:: media/OSPF_LoopAvoidance.png
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step17b
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Loop_ABR_Virtual.xml
   :language: xml

Results
~~~~~~~

With the virtual link configured:

1.  R1 and R3 establish a virtual adjacency through the transit area.

2.  The virtual link is treated as part of Area 0 (the backbone).

3.  Inter-area routing works correctly as if the areas were physically connected to the backbone.

4.  The routing tables show improved connectivity compared to Step 17.

Virtual links provide flexibility in network design but should be used sparingly as they add
complexity and can make troubleshooting more difficult.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_LoopAvoidance.ned <../OSPF_LoopAvoidance.ned>`,
:download:`ASConfig_Area_Loop_ABR_Virtual.xml <../ASConfig_Area_Loop_ABR_Virtual.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
