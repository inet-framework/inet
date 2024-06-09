.. _ug:cha:ipv6:

IPv6 and Mobile IPv6
====================

.. _ug:sec:ipv6:overview:

Overview
--------

Similarly to IPv4, IPv6 support is implemented by several cooperating
modules. The base protocol is implemented in the :ned:`Ipv6` module, which relies on
the :ned:`Ipv6RoutingTable` to access the routes. Interface
configuration (address, state, timeouts, etc.) is held in the node’s
:ned:`InterfaceTable`.

The tasks associated with neighbor discovery and stateless address autoconfiguration are implemented
by the :ned:`Ipv6NeighbourDiscovery` module. The
data structures themselves (destination cache, neighbor cache, prefix
list) are stored in the :ned:`Ipv6RoutingTable`. The rest of ICMPv6’s
functionality, such as error messages, echo request/reply, etc., is
implemented in :ned:`Icmpv6`.

INET has received contributions from the xMIPv6 project for Mobile IPv6 support.
The main module, :ned:`xMIPv6`, implements Fast MIPv6,
Hierarchical MIPv6, and Fast Hierarchical MIPv6 (thus,
:math:`x \in \{F, H, FH\}`). The binding cache and related data structures
are kept in the :ned:`BindingCache` module.