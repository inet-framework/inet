.. _ug:cha:ipv6:

IPv6 and Mobile IPv6
====================

.. _ug:sec:ipv6:overview:

Overview
--------

Similarly to IPv4, IPv6 support is implemented by several cooperating
modules. The base protocol is in the :ned:`Ipv6` module, which relies on
the :ned:`Ipv6RoutingTable` to get access to the routes. Interface
configuration (address, state, timeouts, etc.) is held in the node’s
:ned:`InterfaceTable`.

The :ned:`Ipv6NeighbourDiscovery` module implements all tasks associated
with neighbour discovery and stateless address autoconfiguration. The
data structures themselves (destination cache, neighbour cache, prefix
list) are kept in :ned:`Ipv6RoutingTable`. The rest of ICMPv6’s
functionality, such as error messages, echo request/reply, etc.) is
implemented in :ned:`Icmpv6`.

Mobile IPv6 support has been contributed to INET by the xMIPv6 project.
The main module is :ned:`xMIPv6`, which implements Fast MIPv6,
Hierarchical MIPv6 and Fast Hierarchical MIPv6 (thus,
:math:`x \in {F, H, FH}`). The binding cache and related data structures
are kept in the :ned:`BindingCache` module.
