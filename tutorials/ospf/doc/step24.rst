Step 24. OSPF Path Selection - Suboptimal routes
================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step24.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Suboptimal.ned
   :start-at: network OSPF_Suboptimal
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step24
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Suboptimal.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Suboptimal.ned <../OSPF_Suboptimal.ned>`,
:download:`ASConfig_Suboptimal.xml <../ASConfig_Suboptimal.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
