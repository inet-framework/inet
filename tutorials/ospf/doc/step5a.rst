Step 5a. Mismatched Parameters between two OSPF neighbor
========================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

R4 and R5 will not establish full adjacency because of mismatch OSPF network type.

This step uses the following network:

.. figure:: media/step5.png
   :width: 100%
   :align: center

.. literalinclude:: ../InterfaceNetworkType.ned
   :start-at: network InterfaceNetworkType
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_mismatch.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`InterfaceNetworkType.ned <../InterfaceNetworkType.ned>`,
:download:`ASConfig_mismatch.xml <../ASConfig_mismatch.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.

