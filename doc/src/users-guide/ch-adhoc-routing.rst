.. _ug:cha:adhoc-routing:

Ad Hoc Routing
==============

.. _ug:sec:adhocrouting:overview:

Overview
--------

In ad hoc networks, the topology of the network is not known to the nodes.
Instead, the topology must be discovered. When a new node joins the network,
it announces its presence and listens for announcements from its neighbors.
Each node learns about nearby nodes and how to reach them, and it may also
announce that it can reach other nodes. The task of routing in ad hoc
networks is complicated by the fact that nodes may be mobile, resulting in a
changing topology.

There are two types of ad hoc routing protocols: proactive and reactive.
*Proactive* or *table-driven* protocols maintain up-to-date lists of
destinations and their routes by periodically distributing routing tables
throughout the network. *Reactive* or *on-demand* protocols find a route
on-demand by flooding the network with Route Request packets.

The INET Framework includes the implementation of several ad hoc routing
protocols, including :protocol:`AODV`, :protocol:`DSDV`, :protocol:`DYMO`,
and :protocol:`GPSR`.

The easiest way to add routing to an ad hoc network is to use the
:ned:`ManetRouter` NED type for nodes. :ned:`ManetRouter` contains a
submodule named ``routing`` which has parametric type, so it
can be configured to be an AODV router, a DYMO router, or a router for any
other supported routing protocol. For example, :ned:`ManetRouter` nodes in the
network can be configured to use AODV by adding the following line to the ini
file:

.. code-block:: ini

   **.routingApp.typename = "Aodv" # as an application
   **.routing.typename = "Gpsr" # as a routing protocol module

There are also NED types called :ned:`AodvRouter`, :ned:`DymoRouter`,
:ned:`DsdvRouter`, and :ned:`GpsrRouter`. These types are all based on
:ned:`ManetRouter` and have the routing protocol submodule type set
appropriately.

.. _ug:sec:adhocrouting:aodv:

AODV
----

AODV (Ad hoc On-Demand Distance Vector Routing) is a routing protocol for
mobile ad hoc networks and other wireless ad hoc networks. It provides
quick adaptation to dynamic link conditions, low processing and memory
overhead, low network utilization, and determines unicast routes to
destinations within the ad hoc network.

The :ned:`Aodv` module type implements AODV, based on RFC 3561.

:ned:`AodvRouter` is a :ned:`ManetRouter` with the routing module type
set to :ned:`Aodv`.

.. _ug:sec:adhocrouting:dsdv:

DSDV
----

DSDV (Destination-Sequenced Distance-Vector Routing) is a table-driven
routing scheme for ad hoc mobile networks based on the Bellman-Ford
algorithm.

The :ned:`Dsdv` module implements the IPv4 host-routing profile of
Perkins and Bhagwat (SIGCOMM 1994), with hop counts on one bidirectional ad hoc
interface. Forwarding changes take effect immediately; finite route
advertisements may wait for settling. Periodic full dumps repair lost updates.
New settling deadlines are at least ``minSettlingDelay`` and
at most ``settlingCap`` into the future. Later updates retain the existing
deadline. The minimum must be positive and no greater than the cap;
urgent advertisements bypass this delay.
The model uses its own versioned UDP format and retains withdrawn destination
sequence floors for the remainder of the run.

Incremental updates send required route changes first, then rotate through
sequence-only changes in the remaining space. Destinations with newer sequence
numbers therefore get a turn across successive updates instead of repeatedly
favoring the lowest addresses. This uses existing send opportunities and does
not create additional broadcasts just to send sequence-only changes.

Set ``useMacFailureFeedback = true`` to let MAC delivery-failure hints withdraw routes through the failed neighbor before its silence timeout. It uses the failed frame's receiver and a local ARP-cache lookup through the configured ``arpModule``; missing or unusable metadata leaves ordinary timeout processing active. Parameter declarations and defaults are in :ned:`Dsdv`. A failed receiver does not invalidate a route that has already switched to a different next hop. A MAC failure can also result from transient interference, so feedback is a routing hint rather than proof of physical disconnection.

:ned:`DsdvRouter` extends :ned:`AdhocHost` with the routing controller attached
through UDP. Custom :ned:`ManetRouter` nodes select it in ``routingApp``;
ordinary hosts may place it in ``app[]``. Use local ARP, enable IPv4 forwarding
and avoid configurator-supplied multihop routes when studying routing behavior.
The example under ``examples/dsdv`` demonstrates a deterministic mobile line.
IPv6, prefix/default advertisements and multiple active interfaces are outside
the supported profile. Restarting the same module preserves sequence floors;
destroying and recreating it requires persistent state outside this model.

.. _ug:sec:adhocrouting:dymo:

DYMO
----

The DYMO (Dynamic MANET On-demand) routing protocol is the successor to the
AODV routing protocol. DYMO can work as both a proactive and a
reactive routing protocol, i.e. routes can be discovered just when they
are needed.

The :ned:`Dymo` module type implements DYMO, based on the IETF draft
"draft-ietf-manet-dymo-24".

:ned:`DymoRouter` is a :ned:`ManetRouter` with the routing module type
set to :ned:`Dymo`.

.. _ug:sec:adhocrouting:gpsr:

GPSR
----

GPSR (Greedy Perimeter Stateless Routing) is a routing protocol for
mobile wireless networks that uses the geographic positions of nodes to
make packet forwarding decisions.

The :ned:`Gpsr` module type implements GPSR, based on the paper "GPSR:
Greedy Perimeter Stateless Routing for Wireless Networks" by Brad Karp
and H. T. Kung, 2000. The implementation supports both GG and RNG
planarization algorithms.

:ned:`GpsrRouter` is a :ned:`ManetRouter` with the routing module type
set to :ned:`Gpsr`.
