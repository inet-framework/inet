.. _ug:cha:other-network-protocols:

Other Network Protocols
=======================

.. _ug:sec:networkprotocols:overview:

Overview
--------

Network layer protocols in INET are not restricted to IPv4 and IPv6.
INET nodes such as :ned:`Router` and :ned:`StandardHost` can be
configured to use an alternative network layer protocols instead of, or
in addition to, IPv4 and IPv6.

Node models contain three optional network layers that can be
individually turned on or off:



.. code-block:: ned

   ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer if hasIpv4;
   ipv6: <default("Ipv6NetworkLayer")> like INetworkLayer if hasIpv6;
   generic: <default("")> like INetworkLayer if hasGn;

In the default configuration, only IPv4 is turned on. If you want to use
an alternative network layer protocol instead of IPv4/IPv6, your
configuration will look something like this:



.. code-block:: ini

   **.hasIpv4 = false
   **.hasIpv6 = false
   **.hasGn = true
   **.generic.typename = "WiseRouteNetworkLayer"

The list of alternative network layers includes:

-  :ned:`SimpleNetworkLayer` is a generic network layer where the
   concrete protocol type is a parameter

-  :ned:`NextHopNetworkLayer` is a network layer specialized for the
   “Next-Hop Forwarding Protocol”, an abstract implementation of the
   next-hop routing concept

-  :ned:`WiseRouteNetworkLayer` is specialized for the Wise Route
   protocol

The list of network layer protocols that can be plugged into
:ned:`SimpleNetworkLayer` includes:

-  :ned:`Flooding` implements controlled flooding

-  :ned:`WiseRoute` implements Wise Route, a convergecasting protocol
   for wireless sensor networks

-  :ned:`ProbabilisticBroadcast` implements a multi-hop ad-hoc data
   dissemination protocol

-  :ned:`AdaptiveProbabilisticBroadcast` is a variant of the previous
   one

In addition to the network layer protocol, :ned:`SimpleNetworkLayer`
includes an instance of :ned:`GlobalArp` for address resolution, and an
instance of :ned:`EchoProtocol`, a module type that implements a simple
*ping*-like protocol.

All the above network protocols can work with IPv4 addresses, IPv6
addresses, use MAC address as network address (this is sometimes useful
in WSNs), or use sythetic addresses that only make sense within the
simulation.  [1]_

In apps, you need to specify which network layer protocol you want to
use. For example:



.. code-block:: ini

   **.app[*].networkProtocol = "flood"

.. _ug:sec:networkprotocols:protocols:

Protocols
---------

.. _ug:sec:networkprotocols:flooding:

Flooding
~~~~~~~~

:ned:`Flooding` is a simple flooding protocol for network-level
broadcast. It remembers already broadcast messages, and does not
rebroadcast them if it gets another copy of that message.

.. _ug:sec:networkprotocols:probabilisticbroadcast:

ProbabilisticBroadcast
~~~~~~~~~~~~~~~~~~~~~~

:ned:`ProbabilisticBroadcast` is a multi-hop ad-hoc data dissemination
protocol based on probabilistic broadcast.

This method reduces the number of packets sent on the channel (reducing
the broadcast storm problem) at the risk of some nodes not receiving the
data. It is particularly interesting for mobile networks.

The transmission probability for each attempt, the time between two
transmission attempts, the maximum number of broadcast transmissions of
a packet, and some other settings are parameters.

.. _ug:sec:networkprotocols:adaptiveprobabilisticbroadcast:

AdaptiveProbabilisticBroadcast
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

:ned:`AdaptiveProbabilisticBroadcast` is a variant of
:ned:`ProbabilisticBroadcast` that automatically adjusts transmission
probabilities depending on the estimated number of neighbours.

.. _ug:sec:networkprotocols:wiseroute:

WiseRoute
~~~~~~~~~

:ned:`WiseRoute` implements Wise Route, a simple loop-free routing
algorithm that builds a routing tree from a central network point,
designed for sensor networks and convergecast traffic (WIreless SEnsor
routing).

The sink (the device at the center of the network) broadcasts a route
building message. Each network node that receives it selects the sink as
parent in the routing tree, and rebroadcasts the route building message.
This procedure maximizes the probability that all network nodes can join
the network, and avoids loops.

The :par:`sinkAddress` parameter specifies the sink network address,
:par:`rssiThreshold` is a threshold to avoid using bad links (with too
low RSSI values) for routing, and :par:`routeFloodsInterval` should be
set to zero for all nodes except the sink. Each
:par:`routeFloodsInterval`, the sink restarts the tree building
procedure. Set it to a large value if you do not want the tree to be
rebuilt.

.. _ug:sec:networkprotocols:nexthopforwarding:

NextHopForwarding
~~~~~~~~~~~~~~~~~

The :ned:`NextHopForwarding` module is an implementation of the next-hop
forwarding concept. (It can be thought of as an abstracted version of
IP.)

The protocol needs an additional module, a :ned:`NextHopRoutingTable`
for its operation. The routing table module is included in the
:ned:`NextHopNetworkLayer` compound module.

.. _ug:sec:networkprotocols:address-types:

Address Types
-------------

The following address types are available:

-  IPv4 address

-  IPv6 address

-  MAC address

-  module ID

-  module path

Protocols described in this chapter work with “generic” L3 addresses,
they can use any address type.

When choosing IPv4 addresses, an :ned:`Ipv4NetworkConfigurator` global
instance can be used to assign addresses to network interfaces. (Note
that :ned:`Ipv4NetworkConfigurator` also needs a per-node instance of
:ned:`Ipv4NodeConfigurator` for it to work.)

.. _ug:sec:networkprotocols:address-resolution:

Address Resolution
------------------

Address resolution is done by :ned:`GlobalArp`. If the address type is
IPv4, :ned:`Arp` can be used instead of :ned:`GlobalArp`.

.. [1]
   This is possible because the implementation of these modules simply
   use the :cpp:`L3Address` C++ class, which is a variant type capable
   of holding several types of L3 addresses.
