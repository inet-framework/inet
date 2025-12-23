Step 18. AS-External LSAs of 'type 1 metric' with different advertised destination
==================================================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step18.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Area_External.ned
   :start-at: network OSPF_Area_External
   :language: ned

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

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Area_External.ned <../OSPF_Area_External.ned>`,
:download:`ASConfig_Area_ExternalRoute_Type1.xml <../ASConfig_Area_ExternalRoute_Type1.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
