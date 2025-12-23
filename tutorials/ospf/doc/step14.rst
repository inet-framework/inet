Step 14. Hierarchical OSPF topology and summary LSA
===================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step141516.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_AreaTest.ned
   :start-at: network OSPF_AreaTest
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step14
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_AreaTest.ned <../OSPF_AreaTest.ned>`,
:download:`ASConfig_Area.xml <../ASConfig_Area.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.

