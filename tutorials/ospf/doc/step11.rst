Step 11. Configure an interface as NoOSPF
=========================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This configuration is based on step 3.

.. figure:: media/step1-2-11-12.png
   :width: 100%
   :align: center

The OSPF configuration:

.. literalinclude:: ../ASConfig_NoOspf.xml
   :language: xml

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step11
   :end-before: ------

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network.ned <../Network.ned>`,
:download:`ASConfig_NoOspf.xml <../ASConfig_NoOspf.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
