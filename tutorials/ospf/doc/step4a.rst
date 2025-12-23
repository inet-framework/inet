Step 4a. Advertising loopback interface
=======================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/step4.png
   :width: 100%
   :align: center

.. literalinclude:: ../RouterLSA.ned
   :start-at: network RouterLSA
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step4a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Loopback.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RouterLSA.ned <../RouterLSA.ned>`,
:download:`ASConfig_Loopback.xml <../ASConfig_Loopback.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.

