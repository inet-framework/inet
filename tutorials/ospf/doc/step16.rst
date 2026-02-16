Step 16. OSPF topology change in multi-area OSPF
================================================

Goals
-----

The goal of this step is to examine how OSPF reacts to network topology changes
in a multi-area environment and how LSA updates propagate across area boundaries.

When a link fails in a multi-area OSPF network, the affected router generates new
LSAs to reflect the topology change. These LSAs are flooded within the area, and
ABRs generate updated Summary LSAs to propagate the change to other areas. This
step demonstrates the cascading effect of topology changes through the hierarchical
OSPF structure.

Configuration
~~~~~~~~~~~~~

This configuration is based on step 14. The simulation script disconnects the link
between **R1** and **switch1** at t=60s, which isolates **host1** from the rest of
the network.

.. figure:: media/OSPF_AreaTest.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step16
   :end-before: ------

Results
~~~~~~~

When the link between **R1** and **switch1** breaks at t=60s:

1.  **R1** detects the link down event on its eth0 interface.

2.  **R1** generates a new Router LSA that no longer includes the link to the
    192.168.11.0/30 network (the network containing **host1**).

3.  This Router LSA is flooded within Area 0.0.0.1 to **R2** and **R3**.

4.  **R2** and **R3** update their LSDBs and run the SPF algorithm. They remove
    the route to 192.168.11.0/30 from their routing tables because it was reachable
    only via **R1**.

5.  **R3** (as an ABR) regenerates Summary LSAs for Area 0.0.0.1 that it advertises
    into Area 0.0.0.0. The Summary LSA for 192.168.11.0/30 is withdrawn
    because no other path exists.

6.  **R4** (the other ABR) receives the updated Summary LSAs from **R3** via the
    backbone and updates its LSDB and routing table accordingly.

7.  **R4** then generates new Summary LSAs to advertise into Area 0.0.0.2, informing
    **R5** about the topology change.

8.  **R5** updates its routing table to remove the route to 192.168.11.0/30.

The routing table visualizer output shows that at t=60s, the 192.168.11.0/30 network
becomes unreachable from all other routers in the network. The routing table changes
propagate through the multi-area hierarchy: first within Area 0.0.0.1 (intra-area),
then across the backbone (inter-area), and finally to Area 0.0.0.2 (inter-area).

This demonstrates how OSPF's hierarchical design efficiently propagates topology
changes across area boundaries using Summary LSAs, while containing the detailed
topology changes (Router LSAs) within their originating area.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`OSPF_AreaTest.ned <../OSPF_AreaTest.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
