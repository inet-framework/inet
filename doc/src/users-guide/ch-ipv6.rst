.. _usr:cha:ipv6:

IPv6 and Mobile IPv6
====================

.. _usr:sec:ipv6:overview:

Overview
--------

IPv6 support is implemented by several cooperating modules. The IPv6
module implements IPv6 datagram handling (sending, forwarding etc). It
relies on :ned:`Ipv6RoutingTable` to get access to the routes.
:ned:`Ipv6RoutingTable` also contains the neighbour discovery data
structures (destination cache, neighbour cache, prefix list – the latter
effectively merged into the route table). Interface configuration
(address, state, timeouts etc) is held in the :ned:`InterfaceTable`, in
:cpp:`Ipv6InterfaceData` objects attached to :cpp:`InterfaceEntry` as
its ``ipv6()`` member.

The module :ned:`Ipv6NeighbourDiscovery` implements all tasks associated
with neighbour discovery and stateless address autoconfiguration. The
data structures themselves (destination cache, neighbour cache, prefix
list) are kept in :ned:`Ipv6RoutingTable`, and are accessed via public
C++ methods. Neighbour discovery packets are only sent and processed by
this module – when IPv6 receives one, it forwards the packet to
:ned:`Ipv6NeighbourDiscovery`.

The rest of ICMPv6 (ICMP errors, echo request/reply etc) is implemented
in the module :ned:`Icmpv6`, just like with IPv4. ICMP errors are sent
into :ned:`Ipv6ErrorHandling`, which the user can extend or replace to
get errors handled in any way they like.
