Exploring MANET Routing Protocols
=================================

Goals
-----

Routing in Mobile Ad Hoc Networks (MANETs) is a challenging problem due to the
dynamically changing network topology, which gave rise to the development of
many different routing protocols. MANET routing
protocols can be classified into reactive, proactive, and location-based, among
others. INET features various routing protocols for MANETs from different
categories.

In this showcase, we'll look at three representative MANET routing protocols: a
reactive protocol (AODV), a proactive protocol (DSDV), and a location-based
protocol (GPSR). We'll explore each of them through three example simulations.

| INET version: ``4.0``
| Source files location: `inet/showcases/routing/manet <https://github.com/inet-framework/inet/tree/master/showcases/routing/manet>`__

About MANETs
------------

MANETs are ad hoc networks comprised of mobile wireless nodes. Given the
mobile nature of the nodes, the network topology can change over time.
The nodes create their own network infrastructure: each node also acts
as a router, forwarding traffic in the network. MANET routing protocols
need to adapt to changes in the network topology and maintain routing
information, so that packets can be forwarded to their destinations. Although
MANET routing protocols are mainly for mobile networks, they can also be
useful for networks of stationary nodes that lack network
infrastructure.

.. todo::

   <!-- TODO: keywords: autonomous, wireless, self-configuring, continuously maintain information to properly route. each node is a router, forwarding traffic not mean for him. transport layer ? -->

There are two main types of MANET routing protocols, reactive and
proactive (although there are others which don't fit into either
category). ``Reactive`` or on-demand routing protocols update routing
information when there is an immediate demand for it, i.e. one of the
nodes wants to send a packet (and there is no working route to the
destination). Then, they exchange route discovery messages and forward
the packet. The routes stay the same until there is an error in a
packet's forwarding, i.e. the packet cannot be forwarded anymore due to
a change in the network topology. Examples of reactive MANET routing
protocols include AODV, DSR, ABR, etc.

``Proactive`` or table-driven routing protocols continuously maintain
routing information so the routes in the network are always up to date.
This update typically involves periodic routing maintenance messages exchanged
throughout the network. These types of protocols use more maintenance
transmissions than reactive protocols to make sure the routing
information is always up-to-date (they update it even when there is no
change in the network topology). Examples of reactive MANET routing
protocols include DSDV, OLSR, Babel, etc.

Reactive protocols require less overhead than proactive protocols (there
are no concerning routing when the routes don't change) but also might
react more slowly to changes in the network topology. In the case of
proactive protocols, due to the up-to-date nature of routing
information, latency is lower than in the case of reactive protocols.

There are other types of MANET routing protocols, such as Hybrid (both
reactive and proactive), Hierarchical, and Geographical routing. INET
features several routing protocols, for MANETs and other uses
(including wired and wireless cases). See
`/inet/src/inet/routing <https://github.com/inet-framework/inet/tree/master/src/inet/routing>`__
directory for the available routing protocols.

The example simulations in this showcase feature the reactive protocol
``Ad hoc On-Demand Distance Vector routing`` (AODV), the proactive
protocol ``Destination-Sequenced Distance Vector routing`` (DSDV), and
the geo routing protocol ``Greedy Perimeter Stateless Routing`` (GPSR).
The following section details these three protocols briefly.

About AODV
~~~~~~~~~~

AODV is a reactive (or on-demand) MANET routing protocol, and as such,
it maintains routes for which there is a demand in the network (i.e.
packets are frequently sent on the route). AODV maintains a routing
table with the next hop for reaching destinations. Routes time out after
a while if not used (i.e. no packets are sent on them). AODV features
the following routing message types:

-  ``RREQ``: Route request
-  ``RREP``: Route reply
-  ``RERR``: Route error

When a node wants to send a packet, and it doesn't know the route to the
destination, it initiates route discovery, by sending an ``RREQ``
multicast message. The neighboring nodes record where the message came
from and forward it to their neighbors until the message gets to the
destination node. The destination node replies with an ``RREP``, which
gets back to the source on the reverse path along which the ``RREQ``
came. Forward routes are set up in the intermediate nodes as the
``RREP`` travels back to the source. An intermediate node can also send
an ``RREP`` in reply to a received ``RREQ`` if it knows the route to
the destination, thus nodes can join an existing route. When the
``RREP`` arrives at the source, and the route is created, communication
can begin between the source and the destination. If a route no longer
works due to link break, i.e. messages cannot be forwarded on it, a
``RERR`` message is broadcast by the node which detects the link break.
Other nodes re-broadcast the message. The ``RERR`` message indicates the
destination which is unreachable. Nodes receiving the message make the
route inactive (and eventually the route is deleted). The next packet to
be sent triggers route discovery. As a reactive protocol, generally AODV
has less overhead (less route maintenance messages) than proactive ones,
but setting up new routes takes time while packets are waiting to be
delivered. (Note that the routing protocol overhead depends on the
mobility level in the network.)

Additionally, even though AODV is a reactive protocol, nodes can send
periodic hello messages to discover links to neighbors and update the
status of these links. This mechanism is local (hello messages are only
sent to neighbors, and not forwarded), and it can make the network more
responsive to local topology changes. By default, hello messages are
turned off in INET's AODV implementation.

About DSDV
~~~~~~~~~~

DSDV is a proactive (or table driven) MANET routing protocol, so it
makes sure routing information in the network is always up-to-date. Each
node maintains a routing table with the best route to each destination.
The routing table contains routing entries to all possible destinations
known either directly because it's a neighbor, or indirectly through
neighbors. A routing entry contains the destination's IP address, last
known sequence number, hop count required to reach the destination, and
the next hop. Routing information is frequently updated, so all nodes
have the best routes in the network. Routing information is updated in
two ways:

-  Nodes broadcast their entire routing tables periodically
   (infrequently)
-  Nodes broadcast small updates when a change in their routing table
   occurs

A node updates a routing table entry if it receives a better route. A
better route is one that has a higher sequence number, or a lower hop
count if the sequence number is the same.

In general, DSDV has more overhead than reactive routing protocols,
because route maintenance messages are sent all the time. Since the
routes are always up to date, DSDV has less delay in sending data.

About GPSR
~~~~~~~~~~

GPSR is stateless (regarding routes), geographic location based routing
protocol. Each node maintains the addresses and geographical
co-ordinates of its neighbors, i.e. other nodes in its communication
range. Nodes advertise their locations periodically by sending beacons.
When no beacons are received from a neighboring node for some time, the
node is assumed to be out of range, and its table entry is deleted. A
table entry for a node is also deleted after link failure. Nodes attach
their location data on all sent and forwarded packets as well. Each
packet transmission resets the beacon timer, reducing the required
protocol overhead in parts of the network with frequent packet traffic.
The protocol is stateless in the context of routes. Nodes only have
local information about their neighborhood, i.e. the positions of other
nodes in their communication range, but they don't have information
about node positions or routes in the network as a whole.

Destination is designated by an IP address, but the destination's
location is also appended to packets. Packets are routed towards the
destination's location specified with co-ordinates. IP addresses are
only used to determine whether a receiving node is the destination of a
packet. The protocol operates in one of two modes:

-  In greedy mode, a node forwards a packet to its neighbor which is
   geographically closest to the destination node. Thus the packet gets
   gradually closer to its destination with every hop. If a forwarding
   node is closer to the destination than any of its neighbors, the node
   switches the packet to perimeter mode. In this case, the packet must
   take a route that takes it farther from its destination temporarily -
   it routes around a void, a region without any nodes.
-  In perimeter routing mode, the packet can circumnavigate a void. When
   the packet is in this mode, nodes create a planar graph of their
   neighboring nodes based on their location, where vertices represent
   nodes and edges represent possible links between nodes. Nodes use the
   right-hand rule for forwarding packets, i.e. they forward the packet
   on the first edge to the right, compared to the edge the packet
   arrived from. Each node does this until the packet arrives at its
   destination, or at an intermediate node which is closer to the
   destination than the one where the packet was switched to perimeter
   mode. In the latter case, the packet is switched to greedy mode. If
   the packet is in perimeter mode and would be forwarded again on the
   first edge of the perimeter, it is discarded (there is no route to
   the destination).

Several parameters of the protocol can be set according to the mobility
rate and transmission ranges in the network, such as the interval of beacons
and timeout of neighbor location data. Also, there are multiple
planarization algorithms available, which can yield different planar
graphs and thus result in different behavior of the protocol in certain
situations.

Configuration and Results
-------------------------

This section contains the configuration and results for the three
simulations, which demonstrate the MANET routing protocols ``AODV``,
``DSDV`` and ``GPSR``. The AODV and DSDV simulations use the
``ManetRoutingProtocolsShowcaseA`` network, which features moving hosts.
The GPSR simulation uses the ``ManetRoutingProtocolsShowcaseB`` network,
featuring stationary hosts. The networks are defined in
:download:`ManetProtocolsShowcase.ned <../ManetProtocolsShowcase.ned>`.
Both networks contain hosts of the type :ned:`ManetRouter` (an extension of
:ned:`WirelessHost`), whose routing module type is configurable. Just as
:ned:`WirelessHost`, it uses :ned:`Ieee80211ScalarRadio` by default. It also
has IP forwarding enabled, and its management module is set to
:ned:`Ieee80211MgmtAdhoc`. In the network, there is a source host named
``source``, a destination host named ``destination``, and a number of
other hosts, which are named ``node1`` up to ``node10`` (their numbers
vary in the different networks). In addition to mobile nodes, both
networks contain an :ned:`Ieee80211ScalarRadioMedium`, an
:ned:`Ipv4NetworkConfigurator`, and an :ned:`IntegratedMultiVisualizer`
module. The nodes' default PHY model (IEEE 802.11) will suffice because
we're focusing on the routing protocols.

In all three simulations, the source node pings the destination node.
The two nodes are out of communication range of each other, and the
other nodes are responsible for forwarding packets between the two.
Since routes are managed dynamically by the MANET routing algorithms,
the :ned:`Ipv4NetworkConfigurator` module is instructed not to add any
routes (it will only assign IP addresses). The netmask routes added by
network interfaces are disabled as well. The following keys in the
``General`` configuration in :download:`omnetpp.ini <../omnetpp.ini>`
achieve this:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: configurator
   :end-at: netmaskRoutes

AODV
~~~~

The example simulation featuring AODV is defined in the :ned:`Aodv`
configuration in :download:`omnetpp.ini <../omnetpp.ini>`. This
configuration uses the ``ManetProtocolShowcaseA`` network. The network
looks like the following:

.. figure:: media/networkA.png
   :width: 60%
   :align: center

The nodes are scattered on the scene. The source and destination
nodes are stationary, and the other nodes are configured to move in random
directions. The communication ranges are set up so that ``source``
cannot reach ``destination`` directly but through the intermediate
nodes. The routing protocols will adapt the routes to the changing
network topology.

The mobility settings are defined in the ``MobileNodesBase``
configuration in :download:`omnetpp.ini <../omnetpp.ini>`. The
simulations for AODV and DSDV, which feature moving nodes, are based on
this configuration. The nodes will be moving on linear paths in random
directions with a speed of 25 meters per second, bouncing back from the
edge of the scene. The mobility settings are the following:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: LinearMobility
   :end-at: MinY

The ping app in ``source`` will send one ping request every second to
``destination``.

In INET, AODV is implemented by the :ned:`Aodv` module. This module is configured
in :download:`omnetpp.ini <../omnetpp.ini>` as the routing protocol
type in :ned:`ManetRouter`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: "Aodv"
   :end-at: "Aodv"

The :ned:`Aodv` module has many parameters for controlling the operation of
the protocol. The parameters should be set according to the number of
nodes in a network, the nodes' mobility levels, traffic, and radio
transmission power levels/communication ranges. All of the parameters
have default values, and :ned:`Aodv` should work out of the box, without
setting any of the parameters. We will fine-tune the protocol's behavior
to our scenario by setting two of the parameters:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: activeRouteTimeout
   :end-at: deletePeriod

The :par:`activeRouteTimeout` parameter sets the timeout for the active
routes. If the routes are not used for this period, they become
inactive. The :par:`deletePeriod` parameter sets the period after which the
inactive routes are deleted. The :par:`activeRouteTimeout` parameter is
lowered from the default 3s to 1s, and the :par:`deletePeriod` parameter is
lowered from the default 15s to 0.5s to make the protocol react
faster to the rapidly changing network topology. Higher mobility results
in routes becoming invalid faster. Thus the routing protocol can work
better - react to topology changes faster - with lower timeout values.
However, setting the timeout values too low results in increased routing
protocol overhead.

.. video:: media/Aodv5_s.mp4
   :width: 420
   :align: center

   <!--internal video recording, release mode (does it matter?), normal run, animation speed none, zoom 2 (or 1.54 if smaller), fadeOutMode = animationTime in datalink and networkroute visualizers-->
   <!--playback speed 0.38, normal run until event 1950-->

Successful data link layer transmissions are visualized by colored
arrows. Note that only the routing protocol and ping packets are
visualized, not the ACKs. Here is what happens in the video:

At the beginning of the simulation, ``source`` queues a ping request
packet for transmission. There are no routes for ``destination``, so it
broadcasts an ``AodvRreq`` message. The RREQ is re-broadcast by the
adjacent nodes until it gets to ``destination``. The destination node
sends a unicast ``AodvRrep``. It is forwarded on the reverse path the
RREQ message arrived on
(``destination``->``node6``->``node1``->``source``). As the intermediate
nodes receive the RREP message, the routes to ``destination`` are
created. The routes are visualized with black arrows, and the
:ned:`RoutingTableVisualizer` is configured to visualize only the routes
leading to ``destination``. When the route is established in ``source``,
it sends the ping request packet, which gets to the destination. The
ping reply packet gets back to ``source`` on the reverse path.

When source sends the next ping request packet, ``host6`` has already
moved out of range of ``destination``. The ping packet gets to
``host6``, but can't get to ``destination`` (``host6`` tries to transmit
the packet a few times, but it doesn't get an ACK). So ``host6``
broadcasts an ``AodvRerr`` message, indicating that the link no longer
works. When the RERR gets back to ``host1``, it initiates route discovery
by broadcasting an RREQ message. When a new route is discovered
(``source``->``node1``->``destination``), the ping traffic can continue.

The following log excerpt shows ``node6`` handling the first RREQ and
RREP messages:

.. figure:: media/aodvlog3.png
   :width: 100%

DSDV
~~~~

The example simulation featuring DSDV is defined in the :ned:`Dsdv`
configuration in :download:`omnetpp.ini <../omnetpp.ini>`. Just like
the AODV configuration, this one uses the
``ManetRoutingProtocolsShowcaseB`` network. The mobility settings are
also the same as in the AODV simulation. The ping app in ``source`` will
send a ping request every second.

The DSDV protocol is implemented in the :ned:`Dsdv` module. The routing
protocol type in all hosts is set to :ned:`Dsdv`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: "Dsdv"
   :end-at: "Dsdv"

Currently, complete routing table broadcasts are not implemented, only
the broadcasting of changes in the routing table using periodic hello
messages.

Like :ned:`Aodv` (and most routing protocol modules), :ned:`Dsdv` has many
parameters with default values that yield a working simulation without
any configuration. In this simulation, similarly to the previous one, we
set two parameters of the protocol:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: helloInterval
   :end-at: routeLifetime

The :par:`helloInterval` parameter controls the frequency of the periodic
updates, or hello messages. Setting this parameter to a higher value
decreases the protocol overhead, but the network will react more slowly
to changes in topology. We lower it from the default 5s to 1s
to make the network more adaptive to rapid changes. When a route is not
used or updated after a time, it gets deleted. The ``routeLifetime``
parameter sets after how long the routes are deleted after not being
used or updated anymore. We lower this from the default 5s to 2s.

The following video shows the nodes sending hello messages and routes
being created at the beginning of the simulation. Note that the black
arrows represent routes, and routes from all nodes to all destinations
are visualized here.

.. video:: media/Dsdv1.mp4
   :width: 420
   :align: center

   <!--internal video recording, animation speed none, data link visualizers fadeOutMode set to animation time, zoom 1.54-->

The following video shows ``source`` pinging ``destination``:

.. video:: media/DsdvPing1.mp4
   :width: 420
   :align: center

   <!--internal video recording, animation speed none, playback speed 0.38 (seems to have an effect), zoom 1.54, fadeOutMode animation time, run until event 3000, run until next sendPing-->

GPSR
~~~~

The example simulation featuring GPSR is defined in the :ned:`Gpsr`
configuration in :download:`omnetpp.ini <../omnetpp.ini>`. It uses the
``ManetRoutingProtocolsShowcaseB`` network. The network looks like the
following:

.. figure:: media/networkB.png
   :width: 100%

Just as with the previous two configurations, the nodes are
:ned:`ManetRouter`\ s. The nodes are laid out along a chain. The
transmitter power of the radios is configured so that nodes can only
reach their neighbors in the chain (except for node9, which can reach
nodes 11, 5, and 8). There is a forest, which represents a void that GPSR
can route around. In this example simulation, the nodes will be static
(though GPSR is suitable for scenarios with moving nodes). The source
node will ping the destination node, which is on the other side of the
void. (The ping app in ``source`` will send one ping request every
second.)

The hosts' routing protocol type is set to :ned:`Gpsr`:

.. literalinclude:: ../omnetpp.ini
   :language: ini
   :start-at: "Gpsr"
   :end-at: "Gpsr"

The following video shows running the simulation from the beginning:

.. video:: media/Gpsr1.mp4
   :width: 100%
   :align: center


   <!--simple screen recorder, 10 fps, normal run-->

The nodes start sending out GPSR beacons (and learning about the
positions of their neighbors). Then, ``source`` sends a ping request
packet. It gets forwarded along the chain to ``node9``, which sends it
to ``node5``, as it is the closest to destination among ``node9``'s
neighbors. However, ``node5`` doesn't have any neighbors closer to the
destination (and it is out of range of ``destination``), thus it
switches the packet to perimeter mode. It forwards the ping packet
according to the right-hand rule. The packet gets to ``node1`` and then
back up along the chain through ``node9`` again. Then ``node10``
switches it back to greedy routing mode because ``node10`` is closer to
the destination than ``node5``, where it was switched to perimeter mode.
Then the packet arrives at ``destination``.

The reply packet starts off in perimeter mode, as the destination is
closer to ``source`` than ``destination``'s only neighbor, ``node4``.
The packet is switched back to greedy mode at ``node10`` because it's
closer to ``source`` than ``destination``. From there, it gets to source
through ``node9`` and ``node11``.

Note that the reply packet didn't get back on the same route as the
request packet. Also, a packet is might not be routed to a
closer neighbor because the sender doesn't yet know about it (and its
position).

Also, note that there are no IP routes; the ``ipv4`` module routing
tables are empty. Instead, :ned:`Gpsr` maintains the positions of the nodes
in communication range (those that a beacon was received from) and uses
that for routing decisions. Here is ``node12``'s neighbor position
table:

.. figure:: media/positions.png
   :width: 100%

The table links node positions with IP addresses (it also contains the
beacon arrival time).

Further information
-------------------

The following papers describe the three MANET routing protocols featured
in this showcase:

-  `Ad hoc On-Demand Distance Vector (AODV)
   Routing <https://www.rfc-editor.org/info/rfc3561>`__
-  `Destination-Sequenced Distance Vector (DSDV)
   Protocol <http://www.netlab.tkk.fi/opetus/s38030/k02/Papers/03-Guoyou.pdf>`__
-  `GPSR: Greedy Perimeter Stateless Routing for Wireless
   Networks <http://www.icir.org/bkarp/jobs/gpsr-mobicom2000.pdf>`__

Sources: :download:`omnetpp.ini <../omnetpp.ini>`, :download:`ManetProtocolsShowcase.ned <../ManetProtocolsShowcase.ned>`

Discussion
----------

Use `this
page <https://github.com/inet-framework/inet-showcases/issues/21>`__ in
the GitHub issue tracker for commenting on this showcase.
