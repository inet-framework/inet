Step 17. Loop avoidance in multi-area OSPF topology
===================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step17.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_LoopAvoidance.ned
   :start-at: network OSPF_LoopAvoidance
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step17
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Loop.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_LoopAvoidance.ned <../OSPF_LoopAvoidance.ned>`,
:download:`ASConfig_Area_Loop.xml <../ASConfig_Area_Loop.xml>`


Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
