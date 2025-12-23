Step 17a. Make R3 an ABR - advertise its loopback to backbone
=============================================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This configuration is based on step 17.

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step17a
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_Area_Loop_ABR.xml
   :language: xml

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_LoopAvoidance.ned <../OSPF_LoopAvoidance.ned>`,
:download:`ASConfig_Area_Loop_ABR.xml <../ASConfig_Area_Loop_ABR.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.

