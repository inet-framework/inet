Step 17c. Summary LSA
=====================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step17c.png
   :width: 100%
   :align: center

.. literalinclude:: ../OSPF_Summary_LSA.ned
   :start-at: network OSPF_Summary_LSA
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step17c
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Summary.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_Summary_LSA.ned <../OSPF_Summary_LSA.ned>`,
:download:`ASConfig_Summary.xml <../ASConfig_Summary.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
