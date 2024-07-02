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

The :ned:`Dsdv` module type implements DSDV. It is currently a partial
implementation.

:ned:`DsdvRouter` is a :ned:`ManetRouter` with the routing module type
set to :ned:`Dsdv`.

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
