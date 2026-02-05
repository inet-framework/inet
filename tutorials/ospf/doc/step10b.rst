Step 10b. Router R2 (DROTHER) goes down
=======================================

Goals
-----

The goal of this step is to demonstrate what happens when a DROther router fails on a
multi-access network.

When a DROther (a router that is neither DR nor BDR) fails, the impact is limited since
DROthers do not have special responsibilities in the OSPF protocol operation. The DR and
BDR are unaffected, and the multi-access network continues to operate normally.

Configuration
~~~~~~~~~~~~~

This configuration is based on Step 10. The simulation script shuts down router **R2**
(a DROther) at t=60s.

.. figure:: media/TopologyChange.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10b
   :end-before: ------

Results
~~~~~~~

When R2 (a DROther) is shutdown at t=60s:

1.  The DR and BDR on Switch2 detect that R2 has failed when Hello packets stop arriving.

2.  The DR generates a new Network LSA for Switch2 that no longer lists R2 as an attached router.

3.  Other routers adjacent to R2 on other interfaces also detect the failure and update their
    LSAs accordingly.

4.  The topology change is flooded throughout the area.

5.  Importantly, **no DR/BDR re-election occurs** on Switch2. The existing DR and BDR continue
    their roles.

In general, Routing around the failed router is recomputed using alternative paths, if available.

This demonstrates that DROther failures have minimal impact on the OSPF protocol operation
on multi-access networks.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`TopologyChange.ned <../TopologyChange.ned>`,
:download:`ASConfig_tp_priority.xml <../ASConfig_tp_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
