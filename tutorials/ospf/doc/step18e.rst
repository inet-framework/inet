Step 18e. Address Forwarding
============================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step18e.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Area_External_Forwarding.ned
   :start-at: network OSPF_Area_External_Forwarding
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step18e
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_ExternalRoute_Forwarding.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External_Forwarding.ned <../OSPF_Area_External_Forwarding.ned>`,
:download:`ASConfig_Area_ExternalRoute_Forwarding.xml <../ASConfig_Area_ExternalRoute_Forwarding.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
