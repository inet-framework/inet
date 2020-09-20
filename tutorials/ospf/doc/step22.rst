Step 22. Virtual link - connect a disconnected area to the backbone
===================================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step22.png
   :width: 100%
   :align: center

.. literalinclude:: ../VirtualLink_2.ned
   :start-at: network VirtualLink_2
   :language: ned

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

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`VirtualLink_2.ned <../VirtualLink_2.ned>`,
:download:`ASConfig_Virtual_Disconnected.xml <../ASConfig_Virtual_Disconnected.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
