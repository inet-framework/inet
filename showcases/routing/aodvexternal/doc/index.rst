AODV External Gateway Routing
=============================

Goals
-----

In typical Mobile Ad Hoc Networks (MANETs), all nodes participate in the same
routing protocol. However, in real-world scenarios, it's often necessary to
route traffic between a MANET and an external network that uses different
routing mechanisms. This showcase demonstrates AODV routing with external
gateway support, where AODV-enabled wireless nodes can communicate with hosts
on a conventional wired network through a designated gateway router.

The goal of this showcase is to demonstrate how AODV's external gateway feature
enables seamless routing between an ad hoc wireless network and a traditional
Ethernet-based network infrastructure.

| INET version: ``4.6``
| Source files location: `inet/showcases/routing/aodvexternal <https://github.com/inet-framework/inet/tree/master/showcases/routing/aodvexternal>`__

The Model
---------

The Network
~~~~~~~~~~~

This showcase uses the ``AODVNetwork`` network, defined in
:download:`AODVNetwork.ned <../AODVNetwork.ned>`.

The network contains:

- **20 mobile wireless hosts** of type :ned:`AodvRouter`, equipped with AODV
  routing protocol and wireless interfaces
- **A gateway router** (also of type :ned:`AodvRouter`) that has both wireless
  and Ethernet interfaces, connecting the wireless and wired segments
- **An Ethernet switch** for the wired network
- **An Ethernet host** representing a destination on the wired network

All wireless hosts are mobile nodes that form an ad hoc network using AODV
routing. The gateway router participates in the AODV network via its wireless
interface and forwards traffic to the wired network via its Ethernet interface.

AODV External Gateway Configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The key feature demonstrated in this showcase is AODV's external gateway
support. In INET's AODV implementation, the :ned:`Aodv` module provides a
``gatewayAddress`` parameter that designates which node should act as the
gateway for routing traffic to external (non-AODV) destinations.

All AODV nodes are configured with the router as the gateway:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: gatewayAddress
   :end-at: gatewayAddress

This configuration tells all AODV nodes that when they need to send packets to
destinations outside the AODV network, they should route them towards the
``router``, which serves as the gateway.

The gateway router has both a wireless interface (for AODV communication) and
an Ethernet interface (for connection to the wired network). 

Network Addressing
~~~~~~~~~~~~~~~~~~

The network uses two separate address ranges:

- **Wireless AODV network**: ``10.0.x.x`` with netmask ``255.255.0.0``
- **Wired Ethernet network**: ``192.168.x.x`` with netmask ``255.255.255.0``

This separation clearly delineates the two network segments. The gateway router
has interfaces in both address ranges, enabling it to forward traffic between
them.

For all AODV hosts, the routing table configuration is disabled because AODV
manages routes dynamically:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: configurator.configureRoutingTable
   :end-at: configurator.configureRoutingTable

Configuration
-------------

The simulation is configured in :download:`omnetpp.ini <../omnetpp.ini>`. Key
configuration parameters include:

The Application
~~~~~~~~~~~~~~~

In the simulation, ``host[0]`` acts as the source and sends ping packets to
``ethernetHost``, which is on the wired network. The traffic must traverse the
wireless AODV network, pass through the gateway router, and then reach the
destination via the Ethernet segment.

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: numApps
   :end-at: destAddr

Gateway Configuration
~~~~~~~~~~~~~~~~~~~~~

As mentioned earlier, the ``gatewayAddress`` parameter on all AODV nodes points
to ``router``:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: gatewayAddress
   :end-at: gatewayAddress

Network Configuration
~~~~~~~~~~~~~~~~~~~~~

The :ned:`Ipv4NetworkConfigurator` is configured with XML to set up the address
ranges and routing:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: configurator.config
   :end-at: </config>

This configuration:

1. Assigns ``10.0.x.x`` addresses to all wireless interfaces on AODV hosts and the router
2. Creates a default route on the router directing traffic to its Ethernet interface
3. Assigns ``192.168.x.x`` addresses to all Ethernet interfaces

Results
-------

Simulation Behavior
~~~~~~~~~~~~~~~~~~~

When the simulation runs:

1. **Initial Route Discovery**: When ``host[0]`` wants to send a ping to
   ``ethernetHost``, AODV performs route discovery. Since ``ethernetHost`` is
   not part of the AODV network, all AODV nodes recognize it as an external
   destination and route towards the configured gateway (``router``).

2. **Route Establishment**: AODV establishes routes from ``host[0]`` to the
   router using the standard AODV route request/reply mechanism. The route
   dynamically adapts to the mobile topology as nodes move.

3. **Traffic Flow**: Once the route is established, ping packets flow from
   ``host[0]`` through intermediate AODV nodes to the router. The gateway
   forwards the packets via its Ethernet interface to the switch, which then
   delivers them to ``ethernetHost``.

4. **Topology Changes**: As the mobile nodes move, AODV adapts the routes in
   the wireless segment. If the path to the gateway breaks due to mobility,
   AODV discovers a new route.

.. video:: media/aodv-external.mp4
   :width: 60%
   :align: center

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`AODVNetwork.ned <../AODVNetwork.ned>`

Try It Yourself
---------------

If you already have INET and OMNeT++ installed, start the IDE by typing
``omnetpp``, import the INET project into the IDE, then navigate to the
``inet/showcases/routing/aodvexternal`` folder in the `Project Explorer`. There, you can view
and edit the showcase files, run simulations, and analyze results.

Otherwise, there is an easy way to install INET and OMNeT++ using `opp_env
<https://omnetpp.org/opp_env>`__, and run the simulation interactively.
Ensure that ``opp_env`` is installed on your system, then execute:

.. code-block:: bash

    $ opp_env run inet-4.6 --init -w inet-workspace --install --build-modes=release --chdir \
       -c 'cd inet-4.6.*/showcases/routing/aodvexternal && inet'

This command creates an ``inet-workspace`` directory, installs the appropriate
versions of INET and OMNeT++ within it, and launches the ``inet`` command in the
showcase directory for interactive simulation.

Alternatively, for a more hands-on experience, you can first set up the
workspace and then open an interactive shell:

.. code-block:: bash

    $ opp_env install --init -w inet-workspace --build-modes=release inet-4.6
    $ cd inet-workspace
    $ opp_env shell

Inside the shell, start the IDE by typing ``omnetpp``, import the INET project,
then start exploring.

Discussion
----------

Use `this page <https://github.com/inet-framework/inet/discussions/1058>`__ in
the GitHub issue tracker for commenting on this showcase.
