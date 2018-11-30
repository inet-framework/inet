.. _ug:cha:ipv4:

The IPv4 Protocol Family
========================

.. _ug:sec:ipv4:overview:

Overview
--------

The IP protocol is the workhorse protocol of the TCP/IP protocol suite.
All UDP, TCP, ICMP packets are encapsulated into IP datagrams and
transported by the IP layer. While higher layer protocols transfer data
among two communication end-point, the IP layer provides an hop-by-hop,
unreliable and connectionless delivery service. IP does not maintain any
state information about the individual datagrams, each datagram handled
independently.

The nodes that are connected to the Internet can be either a host or a
router. The hosts can send and recieve IP datagrams, and their operating
system implements the full TCP/IP stack including the transport layer.
On the other hand, routers have more than one interface cards and
perform packet routing between the connected networks. Routers does not
need the transport layer, they work on the IP level only. The division
between routers and hosts is not strict, because if a host have several
interfaces, they can usually be configured to operate as a router too.

Each node on the Internet has a unique IP address. IP datagrams contain
the IP address of the destination. The task of the routers is to find
out the IP address of the next hop on the local network, and forward the
packet to it. Sometimes the datagram is larger, than the maximum
datagram that can be sent on the link (e.g. Ethernet has an 1500 bytes
limit.). In this case the datagram is split into fragments and each
fragment is transmitted independently. The destination host must collect
all fragments, and assemble the datagram, before sending up the data to
the transport layer.

The INET framework contains several modules to build the IPv4 network
layer of hosts and routers:

-  :ned:`Ipv4` is the main module that implements RFC791. This module
   performs IP encapsulation/decapsulation, fragmentation and assembly,
   and routing of IP datagrams.

-  The :ned:`Ipv4RoutingTable` is a helper module that manages the
   routing table of the node. It is queried by the :ned:`Ipv4` module
   for best routes, and updated by the routing daemons implementing RIP,
   OSPF, Manet, etc. protocols.

-  The :ned:`Icmp` module can be used to generate ICMP error packets. It
   also supports ICMP echo applications.

-  The :ned:`Arp` module performs the dynamic translation of IP
   addresses to MAC addresses.

-  The :ned:`Igmpv2` module to generate and process multicast group
   membership reports.

These modules are assembled into a complete network layer module called
:ned:`Ipv4NetworkLayer`. The :ned:`Ipv4NetworkLayer` module is present
e.g. in :ned:`StandardHost` and :ned:`Router`.

The subsequent sections describe the IPv4 modules in detail.

.. _ug:sec:ipv4:ipv4:

Ipv4
----

The :ned:`Ipv4` module implements the IPv4 protocol.

Its parameters include:

-  :par:`crcMode` TODO: @enum("declared", "computed") =
   default("declared");

-  :par:`procDelay` processing time of each incoming datagram.

-  :par:`timeToLive` default TTL of unicast datagrams.

-  :par:`multicastTimeToLive` default TTL of multicast datagrams.

-  :par:`fragmentTimeout` the maximum duration until fragments are kept
   in the fragment buffer.

-  :par:`limitedBroadcast` if ``true``, then link-local broadcast
   datagrams are sent out through each interface, if the higher layer
   did not specify the outgoing interface.

-  :par:`useProxyARP` TODO: default(true);

.. _ug:sec:ipv4:ipv4routingtable:

Ipv4RoutingTable
----------------

The :ned:`Ipv4RoutingTable` module represents the IPv4 route table.
Hosts and routers normally contain one instance of this module. The
:ned:`Ipv4RoutingTable` module does not send or receive messages.
Instead, C++ methods are for querying and updating the table, as well as
for unicast and multicast routing.

The :ned:`Ipv4RoutingTable` module has the following parameters:

-  :par:`routerId`: for routers, the router id using IPv4 address dotted
   notation; specify “auto” to select the highest interface address;
   should be left empty for hosts.

-  :par:`forwarding`: turns IP forwarding on/off. It is always
   ``true`` in a :ned:`Router` and is ``false`` by default in a
   :ned:`StandardHost`.

-  :par:`multicastForwarding`: turns multicast IP forwarding on/off.
   Default is ``false``, should be set to ``true`` in multicast
   routers.

The preferred method for static initialization of routing tables is to
use :ned:`Ipv4NetworkConfigurator`. While :ned:`Ipv4RoutingTable` can
read the routes from a *routing file*, that is considered obsolete. Old
routing files should be replaced with the XML configuration of
:ned:`Ipv4NetworkConfigurator`. The :doc:`ch-network-autoconfig` chapter
describes the format of the new configuration files.

.. _ug:sec:ipv4:icmp:

Icmp
----

The :ned:`Icmp` module implements the Internet Control Message Protocol
(ICMP). ICMP is the error reporting and diagnostic mechanism of the
Internet. It uses the services of IP, so it is a transport layer
protocol, but unlike TCP or UDP it is not used to transfer user data. It
cannot be separated from IP, because the routing errors are reported by
ICMP.

The :ned:`Icmp` module can be used to send error messages and ping
request. It can also respond to incoming ICMP messages.

Each ICMP message is encapsulated within an IP datagram, so its delivery
is unreliable.

.. _ug:sec:ipv4:arp:

Arp
---

The :ned:`Arp` module implements the Address Resolution Protocol (ARP).
The ARP protocol is designed to translate a local protocol address to a
hardware address. Although the ARP protocol can be used with several
network protocol and hardware addressing schemes, in practice they are
almost always IPv4 and 802.3 addresses. The :ned:`Arp` module only
supports IPv4-to-MAC address translation, but not the opposite
direction, Reverse ARP (RARP).

The address to be resolved can be either an IPv4 broadcast/multicast or
a unicast address. The corresponding MAC addresses can be computed for
broadcast and multicast addresses (RFC 1122, 6.4); unicast addresses are
resolved using the ARP procotol.

If the MAC address is found in the ARP cache, then the packet is
transmitted to the addressed interface immediately. Otherwise the packet
is queued and an address resolution takes place.

For address resolution, ARP broadcasts a request frame on the network.
In the request it publishes its own IP and MAC addresses, so each node
in the local subnet can update their mapping. The node whose MAC address
was requested will respond with an ARP frame containing its own MAC
address directly to the node that sent the request. When the original
node receives the ARP response, it updates its ARP cache and sends the
delayed IP packet using the learned MAC address.

ARP resolution is initiated with a C++ call.

The module parameters of :ned:`Arp` are:

-  :par:`retryTimeout`: number of seconds ARP waits between retries to
   resolve an IPv4 address (default is 1s)

-  :par:`retryCount`: number of times ARP will attempt to resolve an
   IPv4 address (default is 3)

-  :par:`cacheTimeout`: number of seconds unused entries in the cache
   will time out (default is 120s)

-  :par:`proxyARP`: enables proxy ARP mode (default is ``true``)

-  :par:`globalARP`: use global ARP cache (default is ``false``)

.. _ug:sec:ipv4:igmp:

Igmp
----

The :ned:`Igmpv3` module implements the Internet Group Management Protocol
(IGMP). IGMP is a communications protocol used by hosts and adjacent
routers on IPv4 networks to establish multicast group memberships. IGMP
is an integral part of IP multicast.

IGMP is responsible for distributing the information of multicast group
memberships from hosts to routers. When an interface of a host joins to
a multicast group, it will send an IGMP report on that interface to
routers. It can also send reports when the interface leaves the
multicast group, so it does not want to receive those multicast
datagrams. The IGMP module of multicast routers processes these IGMP
reports: it updates the list of groups, that has members on the link of
the incoming message.

The :ned:`IIgmp` module interface defines the connections of IGMP
modules. IGMP reports are transmitted by IP, so the module contains
gates to be connected to the IP module (:gate:`ipIn/ipOut`). The IP
module delivers packets with protocol number 2 to the IGMP module.
However some multicast routing protocols (like DVMRP) also exchange
routing information by sending IGMP messages, so they should be
connected to the :gate:`routerIn/routerOut` gates of the IGMP module. The
IGMP module delivers the IGMP messages not processed by itself to the
connected routing module.

The :ned:`Igmpv2` module implements version 2 of the IGMP protocol (RFC
2236). Next we describe its behaviour in host and routers in details.
Note that multicast routers behaves as hosts too, i.e. they are sending
reports to other routers when joining or leaving a multicast group.

Host behaviour
~~~~~~~~~~~~~~

When an interface joins to a multicast group, the host will send a
Membership Report immediately to the group address. This report is
repeated after :par:`unsolicitedReportInterval` to cover the possibility
of the first report being lost.

When a host’s interface leaves a multicast group, and it was the last
host that sent a Membership Report for that group, it will send a Leave
Group message to the all-routers multicast group (224.0.0.2).

This module also responds to IGMP Queries. When the host receives a
Group-Specific Query on an interface that belongs to that group, then it
will set a timer to a random value between 0 and Max Response Time of
the Query. If the timer expires before the host observe a Membership
Report sent by other hosts, then the host sends an IGMPv2 Membership
Report. When the host receives a General Query on an interface, a timer
is initialized and a report is sent for each group membership of the
interface.

Router behaviour
~~~~~~~~~~~~~~~~

Multicast routers maintains a list for each interface containing the
multicast groups that have listeners on that interface. This list is
updated when IGMP Membership Reports and Leave Group messages arrive, or
when a timer expires since the last Query.

When multiple routers are connected to the same link, the one with the
smallest IP address will be the Querier. When other routers observe that
they are Non-Queriers (by receiving an IGMP Query with a lower source
address), they stop sending IGMP Queries until
:par:`otherQuerierPresentInterval` elapsed since the last received
query.

Routers periodically (:par:`queryInterval`) send a General Query on each
attached network for which this router is a Querier. On startup the
router sends :par:`startupQueryCount` queries separated by
:par:`startupQueryInterval`. A General Query has unspecified Group
Address field, a Max Response Time field set to
:par:`queryResponseInterval`, and is sent to the all-systems multicast
address (224.0.0.1).

When a router receives a Membership Report, it will add the reported
group to the list of multicast group memberships. At the same time it
will set a timer for the membership to :par:`groupMembershipInterval`.
Repeated reports restart the timer. If the timer expires, the router
assumes that the group has no local members, and multicast traffic is no
more forwarded to that interface.

When a Querier receives a Leave Group message for a group, it sends a
Group-Specific Query to the group being left. It repeats the Query
:par:`lastMemberQueryCount` times in separated by
:par:`lastMemberQueryInterval` until a Membership Report is received. If
no Report received, then the router assumes that the group has no local
members.

Parameters
~~~~~~~~~~

The following parameters have effects in both hosts and routers:

-  :par:`enabled` if ``false`` then the IGMP module never sends any
   message and discards incoming messages. Default is ``true``.

The following parameters are only used in hosts:

-  :par:`unsolicitedReportInterval` the time between repetitions of a
   host’s initial report of membership in a group. Default is 10s.

Router timeouts are configured by these parameters:

-  :par:`robustnessVariable` the IGMP is robust to
   :par:`robustnessVariable`-1 packet losses. Default is 2.

-  :par:`queryInterval` the interval between General Queries sent by a
   Querier. Default is 125s.

-  :par:`queryResponseInterval` the Max Response Time inserted into
   General Queries

-  :par:`groupMembershipInterval` the amount of time that must pass
   before a multicast router decides there are no more members of a
   group on a network. Fixed to :par:`robustnessVariable` \*
   :par:`queryInterval` + :par:`queryResponseInterval`.

-  :par:`otherQuerierPresentInterval` the length of time that must pass
   before a multicast router decides that there is no longer another
   multicast router which should be the querier. Fixed to
   :par:`robustnessVariable` \* :par:`queryInterval` +
   :par:`queryResponseInterval` / 2.

-  :par:`startupQueryInterval` the interval between General Queries sent
   by a Querier on startup. Default is :par:`queryInterval` / 4.

-  :par:`startupQueryCount` the number of Queries sent out on startup,
   separated by the :par:`startupQueryInterval`. Default is
   :par:`robustnessVariable`.

-  :par:`lastMemberQueryInterval` the Max Response Time inserted into
   Group-Specific Queries sent in response to Leave Group messages, and
   is also the amount of time between Group-Specific Query messages.
   Default is 1s.

-  :par:`lastMemberQueryCount` the number of Group-Specific Queries sent
   before the router assumes there are no local members. Default is
   :par:`robustnessVariable`.
