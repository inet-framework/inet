Step 7. Influencing OSPF DR/BDR election
========================================

Goals
-----

The goal of this step is to demonstrate how to influence the DR/BDR election by configuring
router priorities.

The OSPF DR/BDR election is based primarily on the interface priority value (0-255). Network
administrators can use priorities to control which router becomes DR and BDR. This is useful
for ensuring that the most capable router (e.g., with better hardware or connectivity) serves
as the DR.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 6. The OSPF configuration assigns different priorities
to R3, making it more likely to become the DR.

.. figure:: media/Network2.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step7
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_priority.xml
   :language: xml

Results
~~~~~~~

With the modified priorities:

1.  **R3** has a higher priority than the other routers.

2.  During the DR/BDR election, R3 is elected as the DR (or BDR) based on its higher priority.

3.  Comparing with Step 6 (where all routers had default priorities), the DR/BDR assignment
    changes to reflect the priority configuration.

    In this simulation, R3 becomes the DR because it has the highest priority (10). Among the
    remaining routers (which all have priority 1), R6 has the highest Router ID, so it becomes the BDR.

This demonstrates that network administrators have control over the DR/BDR election and can
engineer the network to ensure specific routers take on these roles.

**Important**: The DR/BDR election is NOT preemptive. If a router with higher priority joins
the network after a DR/BDR have already been elected, it will not automatically replace them.
The election only occurs when there is no DR/BDR, or when the current DR/BDR fails.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network2.ned <../Network2.ned>`,
:download:`ASConfig_priority.xml <../ASConfig_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
