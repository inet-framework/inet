Step 10. Network Topology Changes
=================================

Goals
-----

The goal of this step is to examine how OSPF reacts to network topology changes.

OSPF routers monitor the state of their links. When a link state changes (e.g., up/down),
the router generates a new LSA and floods it throughout the area. Other routers receive
the update, update their LSDBs, and run the SPF algorithm to calculate new routes.

Configuration
~~~~~~~~~~~~~

This step uses the ``TopologyChange`` network. The OSPF configuration in
``ASConfig_tp_priority.xml`` assigns different priorities to the routers connected to
the central switch (Switch2) to deterministically select the DR and BDR.

*   **R1**: Priority 10 (DR)
*   **R5**: Priority 9 (BDR)
*   **R2, R6**: Priority 1 (DROthers)

The simulation script disconnects the link between **R4** and **Switch3** at t=60s.

.. figure:: media/TopologyChange.png
   :width: 100%
   :align: center

.. literalinclude:: ../TopologyChange.ned
   :start-at: network TopologyChange
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step10
   :end-before: ------

The OSPF configuration:

.. literalinclude:: ../ASConfig_tp_priority.xml
   :language: xml

Results
~~~~~~~

When the link between R4 and Switch3 breaks at t=60s:

1.  **R4** detects the link down event.
2.  R4 generates a new Router LSA that *excludes* the link to Switch3's network.
3.  R4 floods this new LSA to its neighbors (R3).
4.  The LSA propagates through the network (R3 → R2 → Switch2 → others).
5.  All routers update their LSDB and remove the route to the network that was behind Switch3
    (if it's no longer reachable).

This demonstrates the basic mechanism of LSA flooding and database synchronization upon
topology change.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`TopologyChange.ned <../TopologyChange.ned>`,
:download:`ASConfig_tp_priority.xml <../ASConfig_tp_priority.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
