.. _ug:cha:ipv6:

The IPv6 Protocol Family
========================

.. _ug:sec:ipv6:overview:

Overview
--------

IPv6 is the successor of IPv4, designed primarily to overcome the
exhaustion of the IPv4 address space. It uses 128-bit addresses (written
as colon-separated groups of hexadecimal digits, e.g.
``2001:db8::1``), and refines the protocol in several ways: address
autoconfiguration, a simplified and fixed-size base header with optional
extension headers, routers that never fragment (the source performs Path
MTU Discovery instead), and the replacement of ARP, ICMP Router Discovery
and ICMP Redirect with a single Neighbor Discovery protocol built on
ICMPv6.

Like IPv4, the IPv6 network layer is built from several cooperating
modules:

-  :ned:`Ipv6` is the main module that implements RFC 8200. It performs
   encapsulation/decapsulation, routing and forwarding of datagrams,
   fragmentation and reassembly, and the processing of extension headers.

-  :ned:`Ipv6RoutingTable` is a helper module that holds the routing
   table together with the Neighbor Discovery data structures (the
   destination cache, neighbor cache and prefix list). It is queried and
   updated by the other modules through C++ method calls and does not
   send or receive messages.

-  :ned:`Icmpv6` implements ICMPv6 error messages and the Echo
   (ping) service.

-  :ned:`Ipv6NeighbourDiscovery` implements Neighbor Discovery (RFC 4861)
   and stateless address autoconfiguration (RFC 4862): Router and Neighbor
   Solicitation/Advertisement, Duplicate Address Detection, and Neighbor
   Unreachability Detection.

These modules are assembled into a complete network layer module,
:ned:`Ipv6NetworkLayer`, which is present in :ned:`StandardHost6` and
:ned:`Router6`.

The network layer can optionally include Mobile IPv6 (RFC 3775); see the
`Mobile IPv6`_ section. The :ned:`Ipv6NetworkConfigurator` module assigns
addresses and routes; see the :doc:`ch-network-autoconfig` chapter.

The subsequent sections describe the IPv6 modules in detail.

.. _ug:sec:ipv6:ipv6:

IPv6
----

The IPv6 protocol is implemented by the :ned:`Ipv6` module. It accepts
packets from higher-layer protocols (which attach an :msg:`L3AddressReq`
to specify the destination), encapsulates them, and routes them by
querying the :ned:`Ipv6RoutingTable` in C++. Received datagrams are
reassembled if fragmented, their extension headers are processed, and the
payload is delivered to the registered upper-layer protocol selected by
the header's protocol field (with an :msg:`L3AddressInd` attached).

Other modules may register *netfilter hooks* on :ned:`Ipv6` to inspect,
modify, drop or queue datagrams at well-defined points (pre-routing,
local-out, post-routing). Mobile IPv6 uses these hooks, so the IPv6 core
itself contains no mobility-specific code.

Its parameters include:

-  :par:`procDelay`: processing time of each incoming datagram (default 0s).

.. _ug:sec:ipv6:ipv6routingtable:

IPv6 Routing Table
------------------

The IPv6 routing table is represented by the :ned:`Ipv6RoutingTable`
module. Hosts and routers normally contain one instance. Like its IPv4
counterpart, it does not send or receive messages; C++ methods are used
for querying and updating the routes, the destination cache and the
neighbor/prefix caches.

Its parameters include:

-  :par:`forwarding`: turns IP forwarding on/off. It is ``true`` in a
   :ned:`Router6` and ``false`` by default in a :ned:`StandardHost6`.

-  :par:`multicastForwarding`: turns multicast forwarding on/off
   (default ``false``).

-  Router Advertisement defaults applied to all router interfaces, such
   as :par:`advManagedFlag`, :par:`advOtherConfigFlag`, :par:`advLinkMtu`,
   :par:`advCurHopLimit` and :par:`advDefaultLifetime`.

The preferred way to set up addresses and routes is the
:ned:`Ipv6NetworkConfigurator`, described in the
:doc:`ch-network-autoconfig` chapter. The module can also read manual
routes from its :par:`routes` XML parameter; a ``<route>`` element
accepts the destination prefix either as a separate ``prefixLength``
attribute or in CIDR ``address/prefixlen`` notation.

.. _ug:sec:ipv6:icmpv6:

ICMPv6
------

ICMPv6 (RFC 4443) is modeled by the :ned:`Icmpv6` module. It is the error
reporting and diagnostic mechanism of IPv6: it generates and processes
Destination Unreachable, Packet Too Big, Time Exceeded and Parameter
Problem messages on behalf of :ned:`Ipv6`, and answers Echo Requests
(ping) with Echo Replies. Each ICMPv6 message is carried in an IPv6
datagram, so its delivery is unreliable.

Unlike in IPv4, the Neighbor Discovery messages (Router/Neighbor
Solicitation and Advertisement, Redirect) are *not* handled here but by
the :ned:`Ipv6NeighbourDiscovery` module, even though they too are
ICMPv6 message types.

.. _ug:sec:ipv6:nd:

Neighbor Discovery and Autoconfiguration
----------------------------------------

The :ned:`Ipv6NeighbourDiscovery` module implements IPv6 Neighbor
Discovery (RFC 4861) and stateless address autoconfiguration (SLAAC,
RFC 4862). It subsumes several functions that are separate protocols in
IPv4 (ARP, ICMP Router Discovery, ICMP Redirect):

-  *Address autoconfiguration*: on startup, an interface forms a
   link-local address from its interface identifier, and — on hosts —
   derives global addresses from the prefixes advertised by routers.

-  *Router discovery*: routers periodically multicast Router
   Advertisements (and answer Router Solicitations) carrying on-link
   prefixes, the default-router lifetime, MTU and other parameters.
   Hosts use them to configure addresses and a default route.

-  *Address resolution*: a node resolves a neighbor's link-layer address
   by multicasting a Neighbor Solicitation and receiving a Neighbor
   Advertisement (the IPv6 equivalent of ARP).

-  *Duplicate Address Detection (DAD)*: before an autoconfigured address
   is used, the node verifies it is unique on the link by soliciting it.

-  *Neighbor Unreachability Detection (NUD)*: a node tracks whether a
   neighbor is still reachable and re-resolves it when needed.

The Neighbor Discovery data structures (neighbor cache, destination
cache, prefix list, default router list) live in the
:ned:`Ipv6RoutingTable`.

Its parameters include the Router Advertisement timing
(:par:`minIntervalBetweenRAs`, :par:`maxIntervalBetweenRAs`,
:par:`advReachableTime`), the number of DAD solicitations
(:par:`dupAddrDetectTransmits`, ``0`` disables DAD), optimistic DAD
(:par:`optimisticDad`, RFC 4429), and retransmission timers
(:par:`retransTimer`, :par:`baseReachableTime`).

.. _ug:sec:ipv6:tunneling:

IPv6 Tunneling
--------------

IPv6-in-IPv6 tunneling (RFC 2473) is modeled as an ordinary virtual
network interface, :ned:`Ipv6TunnelInterface`. Traffic routed to such an
interface is encapsulated in an outer IPv6 header (from the tunnel entry
point toward the tunnel exit point) and handed back to :ned:`Ipv6`, which
forwards the encapsulated datagram like any other packet; at the exit
point it is decapsulated and the inner datagram re-enters the IPv6 layer
as ordinary ingress. A tunnel is therefore just *an interface plus a
route*, and the IPv6 core needs no tunneling-specific code. Mobile IPv6
uses these interfaces for its home-agent and reverse tunnels.

.. _ug:sec:ipv6:mipv6:

Mobile IPv6
-----------

Mobile IPv6 (MIPv6, RFC 3775) lets a node remain reachable under a stable
address while it moves around the IPv6 Internet. A *mobile node* (MN)
owns a permanent **home address** on its home link, and acquires a
**care-of address** at each new point of attachment. Three roles
participate:

-  the **mobile node** (MN), which moves and registers its current
   care-of address;

-  the **home agent** (HA), a router on the home link that intercepts
   traffic addressed to the MN's home address and tunnels it to the MN's
   care-of address;

-  the **correspondent node** (CN), any peer the MN communicates with,
   which may additionally support *route optimization*.

The protocol logic is implemented by the :ned:`Mipv6` module, a submodule
of :ned:`Ipv6NetworkLayer` that is present when its :par:`hasMipv6`
parameter is set. The role is selected by the :par:`isMobileNode` and
:par:`isHomeAgent` parameters of :ned:`Mipv6`. Two passive data-store
modules accompany it: the :ned:`BindingUpdateList` (on a mobile node,
recording the bindings it has registered with its home agent and
correspondent nodes) and the :ned:`BindingCache` (on a home agent or
correspondent node, recording the bindings registered by mobile nodes).

These pieces are pre-wired in the ready-made node types:
:ned:`MobileHost6` and the wireless :ned:`WirelessHost6` act as mobile
nodes, :ned:`HomeAgent6` as a home agent, and :ned:`CorrespondentNode6`
as a route-optimization-capable correspondent.

The operation proceeds as follows:

-  *Home registration and bidirectional tunneling*: when the MN detects
   (via Neighbor Discovery) that it has moved to a foreign link, it forms
   a care-of address and sends a Binding Update to its home agent. The HA
   records the binding in its :ned:`BindingCache` and, from then on,
   intercepts packets addressed to the MN's home address and tunnels them
   to its care-of address; the MN reverse-tunnels its replies through the
   HA. The tunnels are :ned:`Ipv6TunnelInterface` interfaces.

-  *Route optimization*: to avoid the triangular path through the home
   agent, the MN can register its care-of address directly with a
   correspondent node. First the *return-routability* procedure (Home
   Test Init/Care-of Test Init and the corresponding Home Test/Care-of
   Test messages) proves the MN owns both addresses; then the MN sends a
   Binding Update to the CN. Afterwards the CN sends directly to the
   care-of address using a Type 2 Routing Header, and the MN sends to the
   CN using a Home Address Option — both inserted by :ned:`Mipv6` through
   netfilter hooks, again without special-casing the IPv6 core.

-  *Handover and return home*: on each move the MN re-registers its new
   care-of address (and refreshes the route-optimized bindings); when it
   returns to its home link it de-registers, tearing down the tunnels.

Binding lifetimes are bounded by the :par:`maxHaBindingLifeTime` (home
registration) and :par:`maxRrBindingLifeTime` (route optimization)
parameters of :ned:`Mipv6`; bindings are refreshed before they expire.

A complete, runnable example is provided under ``examples/ipv6/mipv6``.
Note that Mobile IPv6 (network-layer mobility) is distinct from the
physical mobility models described in the :doc:`ch-mobility` chapter,
although a wireless mobile node typically uses both.
