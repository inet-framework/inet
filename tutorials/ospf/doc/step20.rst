Step 20. Stub area
==================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step20.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Stub.ned
   :start-at: network OSPF_Stub
   :language: ned

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

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Stub.ned <../OSPF_Stub.ned>`,
:download:`ASConfig_Stub_Area.xml <../ASConfig_Stub_Area.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
