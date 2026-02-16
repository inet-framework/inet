Step 5. Effect of network type of an interface on routing table routes
======================================================================

Goals
-----

The goal of this step is to demonstrate how the OSPF network type of an interface affects
the routing table entries.

OSPF supports different network types: Point-to-Point, Broadcast, NBMA (Non-Broadcast
Multi-Access), Point-to-Multipoint, and others. The network type determines how OSPF
behaves on that interface, including whether a Designated Router (DR) is elected and
how the network is represented in LSAs and routing tables.

*   **Broadcast networks** (e.g., Ethernet): Use DR/BDR election, described by Network LSAs.
*   **Point-to-Point networks** (e.g., PPP links): No DR election, direct adjacencies.

The network type affects both the OSPF protocol operation and how routes appear in the
routing table.

Configuration
~~~~~~~~~~~~~

This step uses the ``InterfaceNetworkType`` network to demonstrate different network types.

.. figure:: media/InterfaceNetworkType.png
   :width: 100%
   :align: center

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step5
   :end-before: ------

Results
~~~~~~~

The simulation shows how different OSPF network types result in different routing table
entries and OSPF behavior:

*   **Broadcast interfaces**: OSPF elects a DR and BDR. The network is represented by
    a Network LSA originated by the DR. Routes to the network show the DR's interface
    as the next hop.

*   **Point-to-Point interfaces**: No DR election occurs. The link is represented directly
    in Router LSAs from both ends. Routes show direct next-hop addresses.

The OSPF module logs and routing tables demonstrate how network type configuration affects:

1.  Neighbor discovery and adjacency formation
2.  LSA generation and flooding
3.  Routing table entries and next-hop selection

Understanding network types is crucial for correctly configuring OSPF on different media types
and troubleshooting adjacency issues.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`InterfaceNetworkType.ned <../InterfaceNetworkType.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1086>`__ in
the GitHub issue tracker for commenting on this tutorial.
