Step 1. BGP Basic Topology
===========================

Goals
-----

The goal of this step is to introduce the Border Gateway Protocol (BGP) and establish
a basic External BGP (E-BGP) session between two autonomous systems (ASes).

BGP is the de facto inter-domain routing protocol used on the Internet. It enables
routers in different autonomous systems to exchange routing information. An autonomous
system (AS) is a collection of IP networks and routers under the control of a single
administrative entity.

BGP sessions are established over TCP (port 179), providing reliable delivery of routing
updates. In E-BGP, routers in different ASes peer with each other to exchange routes.
The BGP routers learn network prefixes from their peers and use BGP path attributes
to make routing decisions.

This step demonstrates:

*   Configuration of two autonomous systems (AS 64520 and AS 64530)
*   Establishment of an E-BGP session between routers in different ASes
*   Advertisement of network prefixes via BGP
*   Formation of BGP routing tables

Configuration
~~~~~~~~~~~~~

This step uses the following network:

.. figure:: media/BGP_Basic_Topology.png
   :width: 100%
   :align: center

The network consists of four routers organized into two autonomous systems:

*   **AS 64520**: Contains routers RA1 and RA2
*   **AS 64530**: Contains routers RB1 and RB2

An E-BGP session is established between **RA1** (10.0.0.9) and **RB1** (10.0.0.5),
which are directly connected. Each AS advertises a network prefix:

*   RA1 advertises 10.0.0.8/30 (the network between RA1 and RA2)
*   RB1 advertises 10.0.0.0/30 (the network between RB1 and RB2)

.. literalinclude:: ../BGP_Basic_Topology.ned
   :start-at: network BGP
   :language: ned

The configuration in ``omnetpp.ini`` is the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step1
   :end-before: ------

The BGP configuration:

.. literalinclude:: ../BGPConfig_Basic.xml
   :language: xml

The XML configuration defines:

*   **TimerParams**: BGP timers including keepAlive (60s) and holdTime (180s)
*   **AS definitions**: Each AS lists its BGP-speaking routers and the networks they advertise
*   **Session**: Defines the E-BGP peering between RA1 and RB1 using their interface addresses

Results
~~~~~~~

When the simulation starts:

1.  **TCP Connection Establishment** (t≈5-6s): RA1 and RB1 establish a TCP connection on port 179
2.  **BGP Session Negotiation**: The routers exchange BGP OPEN messages to establish the BGP session
3.  **Route Exchange**: After the session is established:

    *   RA1 sends an UPDATE message advertising 10.0.0.8/30 to RB1
    *   RB1 sends an UPDATE message advertising 10.0.0.0/30 to RA1

4.  **Routing Table Updates** (t≈6.5-9s): The learned BGP routes are installed:

    *   RB1 learns route to 10.0.0.8/30 via RA1 (next hop 10.0.0.6)
    *   RA1 learns route to 10.0.0.0/30 via RB1 (next hop 10.0.0.5)

5.  **Keepalive Messages**: Periodic keepalive messages maintain the BGP session

The routing table visualizer shows the BGP routes as they are learned and installed.
The routers can now forward traffic between the two autonomous systems using the
BGP-learned routes.

Sources: :download:`BGP_Basic_Topology.ned <../BGP_Basic_Topology.ned>`,
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`BGPConfig_Basic.xml <../BGPConfig_Basic.xml>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet-tutorials/issues/TODO>`__ in
the GitHub issue tracker for commenting on this tutorial.
