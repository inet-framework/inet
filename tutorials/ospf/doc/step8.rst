Step 8. Setting all router ids to zero
======================================

Goals
-----

[explanation]

Configuration
~~~~~~~~~~~~~

This configuration is based on step 6.

# this also means that the Ethernet network will not be recognized by any routers.
The OSPF configuration:

.. literalinclude:: ../ASConfig_zero_priority.xml
   :language: xml

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step8
   :end-before: ------

Results
~~~~~~~

[explanation]

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network2.ned <../Network2.ned>`,
:download:`ASConfig_zero_priority.xml <../ASConfig_zero_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.

