Step 1. Static Routing
======================

Introduction
------------

Welcome to the RIP (Routing Information Protocol) tutorial for the INET
Framework. This tutorial series will guide you through understanding and
experimenting with the RIP routing protocol implementation in OMNeT++. RIP is
one of the oldest distance-vector routing protocols, and despite its
limitations, it remains useful for understanding fundamental routing concepts.

Before diving into the dynamic routing capabilities of RIP, we'll start with
static routing to establish a baseline understanding of our network topology and
routing concepts. This approach allows us to compare static and dynamic routing
behaviors in later steps.

Goals
-----

In this first step, our goals are to:

1. Familiarize ourselves with the experimental network topology that we'll use throughout the tutorial
2. Understand how static routing works using the :ned:`Ipv4NetworkConfigurator` module
3. Observe how packets travel through a network with pre-configured routing tables
4. Establish a baseline for comparison with dynamic RIP routing in subsequent steps

The Network Model
-----------------

Our network consists of three switched LANs connected by routers to form an
internetwork. Each LAN contains multiple hosts connected to an Ethernet switch.
The LANs are interconnected through a network of routers, creating multiple
possible paths between endpoints.

.. figure:: media/step1network.png
   :width: 80%
   :align: center

Key components of the network:

* **Three LANs**: Each with a switch and multiple hosts
* **Five routers**: Connecting the LANs and providing multiple routing paths
* **Connection types**: A mix of 10Mbps and 100Mbps Ethernet links
* **Host0 and Host6**: Special hosts (highlighted in red and blue) that we'll use for testing connectivity

The complete network topology is defined in the ``RipNetworkA.ned`` file. The
network is designed to demonstrate various routing scenarios in the following
steps, including route discovery, convergence, and handling of link failures.

Static Routing Configuration
----------------------------

In this step, we use static routing rather than RIP. Static routing means that
the routing tables are calculated and configured at the beginning of the
simulation and remain unchanged throughout. This is handled by the
:ned:`Ipv4NetworkConfigurator` module, which performs two essential tasks:

1. **Address Assignment**: Automatically assigns IP and MAC addresses to all network interfaces, saving us from the tedious process of manual configuration
2. **Route Calculation**: Computes optimal routes between all network nodes and populates the routing tables accordingly

The configurator uses graph algorithms to determine the shortest paths between
nodes, ensuring efficient routing. It's important to note that in this step,
once the routing tables are set up, they remain static - there's no dynamic
adaptation to network changes as would happen with RIP.

The configuration in ``omnetpp.ini`` for Step 1 includes:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: Step1
   :end-before: ------

Note the ``addDirectRoutes = false`` setting, which instructs the configurator
not to add routes to directly connected networks automatically. This makes the
routing configuration more explicit and easier to understand for our learning
purposes.

Experiment: Testing Connectivity
--------------------------------

To test the routing configuration, we set up a simple experiment where ``host0``
sends ping packets to ``host6``. This allows us to observe how packets traverse
the network using the pre-configured static routes.

When the simulation starts:

1. The configurator module assigns addresses to all interfaces
2. The configurator calculates optimal routes and populates routing tables
3. ``host0`` begins sending ICMP echo (ping) requests to ``host6``
4. The packets travel through the network following the routes in the routing tables
5. ``host6`` responds with ICMP echo replies that travel back to ``host0``

The visualization settings in the general section of ``omnetpp.ini`` help us observe the routing paths:

.. literalinclude:: ../omnetpp.ini
   :start-at: *.visualizer.routingTableVisualizer[0].destinationFilter
   :end-at: *.visualizer.routingTableVisualizer[1].lineColor = "blue"
   :language: ini

These settings display:

* Red arrows: Routes towards ``host0``
* Blue arrows: Routes towards ``host6``

The following video demonstrates the simulation:

.. video:: media/step1_2.mp4
   :width: 100%

..   <!--internal video recording, animation speed none, playback speed 2.138, zoom 0.77-->

Observing the Routing Paths
---------------------------

In the simulation, you can observe:

1. **Complete routing paths**: The network has full connectivity with pre-configured optimal routes
2. **Bidirectional communication**: Packets can flow in both directions between ``host0`` and ``host6``
3. **Multiple routing paths**: The network has redundant paths, though only the optimal ones are used with static routing

This screenshot shows the routing paths (red towards ``host0``, blue towards ``host6``):

.. figure:: media/step1routes.png
   :width: 80%
   :align: center

Notice how the routing tables are complete from the start of the simulation.
This is the key difference from dynamic routing protocols like RIP, where routes
are discovered gradually through router advertisements.

Conclusion and Next Steps
-------------------------

In this step, we've established our network topology and observed how static
routing works with pre-configured routing tables. The
:ned:`Ipv4NetworkConfigurator` has done all the hard work of calculating optimal
routes and setting up the routing tables.

While static routing works well in this stable network, it has limitations:

* It doesn't adapt to network changes (link failures, new routers, etc.)
* It requires manual reconfiguration when the network topology changes
* It doesn't scale well to large, dynamic networks

In the next step, we'll introduce RIP and observe how routers dynamically build
their routing tables through the exchange of routing information. This will
demonstrate how RIP discovers routes and converges to a stable routing state
without pre-configuration.

Sources:
:download:`omnetpp.ini <../omnetpp.ini>`,
:download:`RipNetworkA.ned <../RipNetworkA.ned>`

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1084>`__ in
the GitHub issue tracker for commenting on this tutorial.
