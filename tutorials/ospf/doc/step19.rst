Step 19. Default-route distribution in OSPF
===========================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step19.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Default_Route_Distribution.ned
   :start-at: network OSPF_Default_Route_Distribution
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step19
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Default_Route.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Default_Route_Distribution.ned <../OSPF_Default_Route_Distribution.ned>`,
:download:`ASConfig_Area_Default_Route.xml <../ASConfig_Area_Default_Route.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
