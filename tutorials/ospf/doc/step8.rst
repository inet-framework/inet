Step 8. Setting all router priorities to zero
==============================================

Goals
-----

The goal of this step is to demonstrate what happens when all routers on a multi-access
network have priority 0.

A router with OSPF interface priority 0 cannot become DR or BDR. If all routers on a
multi-access network have priority 0, no DR or BDR will be elected, and:

*   No Network LSA will be generated for that network.
*   The network will not be properly recognized in the OSPF topology.

This effectively disables OSPF routing for that network segment.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 6. All routers have their priorities set to 0 for
the multi-access interface.

.. figure:: media/Network2.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step8
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_zero_priority.xml
   :language: xml

Results
~~~~~~~

When all routers have priority 0:

1.  The DR/BDR election completes, but no router is elected as DR or BDR.

2.  No Network LSA is generated for the Ethernet LAN between the routers.

3.  The OSPF topology is incomplete - the multi-access network is not visible to the routing
    protocol.

4.  Routing between networks may fail or use suboptimal paths since the multi-access segment
    is not properly integrated into the OSPF topology.

This demonstrates the critical role of the DR in multi-access networks and shows what happens
when the DR/BDR election mechanism is effectively disabled.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`Network2.ned <../Network2.ned>`,
:download:`ASConfig_zero_priority.xml <../ASConfig_zero_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
