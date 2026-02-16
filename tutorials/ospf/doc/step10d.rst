Step 10d. Router R1 (DR) goes down
==================================

Goals
-----

The goal of this step is to demonstrate what happens when the Designated Router (DR) fails
on a multi-access network.

When the DR fails, the BDR immediately assumes the DR role, and a new BDR is elected from
the remaining DROthers. This ensures minimal disruption to the network.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 10. The simulation script shuts down router **R1**
(the DR) at t=60s.

.. figure:: media/TopologyChange.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10d
   :end-before: ------

Results
~~~~~~~

When R1 (the DR) is shutdown at t=60s:

1.  The BDR and other routers on Switch2 detect that R1 (the DR) has failed.

2.  **The BDR (R5) immediately becomes the new DR.** This is a critical OSPF feature - the
    BDR is pre-synchronized and ready to take over.

3.  **A new BDR is elected** from the remaining DROthers (R6). Since all remaining DROthers have priority 1, the new DBR is R4.

4.  The new DR (formerly the BDR) begins generating Network LSAs for the multi-access network.

5.  All routers update their adjacencies to reflect the new DR/BDR assignments.

6.  Routing is recomputed to account for the lost R1 and any topology changes.

The quick promotion of the BDR to DR ensures minimal disruption. The Network LSA generation
continues with minimal delay, and the multi-access network remains fully functional.

This demonstrates OSPF's high availability design for multi-access networks through the
DR/BDR redundancy mechanism.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`TopologyChange.ned <../TopologyChange.ned>`,
:download:`ASConfig_tp_priority.xml <../ASConfig_tp_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
