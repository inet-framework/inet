Step 23. OSPF Path Selection
============================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step23.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Route_Selection.ned
   :start-at: network OSPF_Route_Selection
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step23
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Route_Selection.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Route_Selection.ned <../OSPF_Route_Selection.ned>`,
:download:`ASConfig_Route_Selection.xml <../ASConfig_Route_Selection.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
