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

Received IPv4 and IPv6 RREPs with a nonzero prefix install a normalized subnet route;
zero retains host-route semantics. Other eligible members can use the subnet.
Sequence numbers belong to exact prefixes, so a known inactive host record
blocks a covering subnet for that host until fresh host information arrives or
ordinary retention ends. Continuing demand can keep that host record indefinitely.
An inactive unknown placeholder does not impose that block. Active manual routes
retain their forwarding precedence.

The default subnet profile interprets a known RREQ sequence floor as host knowledge.
Retained aggregate history instead requests an authoritative answer with U and D
set, retaining its sequence locally. A cached aggregate answers only U=1,D=0
and retains its nonzero prefix. Authoritative subnet export requires the opt-in
profile described below. RERR identity requires a
stable unambiguous prefix per peer/interface/wire address; ambiguous entries are
skipped conservatively, including inactive candidates. A quoted ICMP Host
Unreachable for one member does not invalidate the aggregate.

``useLocalRepair=true`` enables bounded repair for eligible failed transit paths;
the default is false. ``maxRepairTTL`` defaults to ``floor(0.3*netDiameter)`` and
accepts zero through ``netDiameter``. Zero disables eligibility. Repair requires
the old path and an active reverse distance. Its initial TTL is the larger of
the old distance and half the source distance rounded up, plus ``localAddTTL``.
An unavailable distance or TTL above ``netDiameter``/255 declines repair.

Each exact route prefix gets one attempt with shared jitter/rate admission, a
reply wait from actual send, and a total creation-time bound of
``pathDiscoveryTime``. The MAC-owned failed packet is never retransmitted;
subsequent traffic can join repair. Existing originating searches keep their
purpose. A longer repaired path sends N and remains active. Failure disposes all
attached queues and notifies precursors without incrementing again. Independent
host recovery can release one aggregate member while its prefix repair and
notification deadline remain alive.

IPv6 uses the shared discovery and host-repair state
with its existing custom control encoding. IPv6 neighbor discovery passes
independently of AODV data discovery. Disable IPv6 ``sendRedirects`` for a MANET
whose next hop is not directly reachable by the original source. Explicit
interface selection and received subnet routes support both address families.
IPv6 identities must be non-link-local and non-tentative; this control encoding
is an INET IPv6 adaptation, rather than an interoperable standardized IPv6 AODV format.

Discovery uses increasing ring TTLs through the configured threshold, then
network-wide requests with exponential reply waits. ``rreqRetries`` counts
additional network-wide attempts. Each reply wait starts at actual send,
after jitter and rate admission. TTLs, rates, durations and sampled jitter are
validated before use. The destination reply lifetime defaults to at least
twice the path-discovery interval. The default blacklist duration bounds the
configured search without rate-limit queueing. Explicit positive blacklist
overrides remain supported.

Discoveries that exceed the request rate limit wait for the next one-second
window. Packets retain FIFO order for each destination. External destinations
may share one gateway discovery, whose success or failure completes all its
dependent queues. A queued packet with a caller-selected interface or next hop
incompatible with the discovered path is dropped individually.

Final originating-discovery exhaustion asks IP to report each queued packet
through native Destination Unreachable handling. Native suppression rules still
apply, and an application must support its transport's error indication to
observe the error. Stop, crash and failed transit repair retain silent queue
disposal. Ordinary transit data without a route follows the
route-error/drop path instead of starting an originating discovery.

Route selection uses 32-bit serial sequence ordering. An older control message
cannot replace newer route information. Values separated by exactly half the
sequence range are treated as ambiguous and do not establish freshness.
A rejected advertised path is neither forwarded nor used to complete a search.
An independent previous-hop update can still release that neighbor's queued data.

A-flagged replies receive a one-hop acknowledgment on their ingress interface,
including stale replies and replies reaching the original requester. Validation
deadlines belong to a neighbor/interface pair, and later replies do not postpone
an outstanding deadline. Unsolicited or late ACKs do not reactivate routes.

Failed transmissions are attributed from retained output interface/MAC tags
and stable IP/MAC bindings learned from one-hop control. A route that changed
while a packet was in flight remains intact if it no longer uses the failed
link. Missing, changed or ambiguous metadata is diagnosed and supplies no peer
to invalidate. Bindings remain usable after route expiry, and are cleared on
stop/crash. Custom interfaces must preserve the same immediate-peer contract.

With Hello messages enabled, a positive attributable Hello arms a separate
neighbor-loss deadline. Hellos carry the RFC 3561 Hello Interval extension,
rounded upward to whole milliseconds. Each neighbor's advertised interval
determines its deadline; a legacy Hello uses the receiver's local interval.
Unknown optional extensions are preserved, while malformed or unsupported
mandatory extensions are discarded before affecting liveness. Incoming IP traffic refreshes the learned immediate
sender identified by interface and source MAC, regardless of the original IP
source. Outgoing traffic extends route leases without postponing neighbor loss.
Data alone does not arm a new deadline. Missing sender metadata, rebinding or
an ambiguous MAC cancels this detector on the affected interface until restart.
Hello exchange and attributable MAC failure feedback remain available. Direct
AckingMac, independent nonaggregating 802.11 and Ethernet provide tested metadata
profiles; bridged and aggregated paths require their own attribution evidence.

Restart begins a recovery interval of ``deletePeriod``, including when the
restart occurs at simulation time zero. Initial startup, including a node
initialized down, has no reboot hold. Local data waits while incoming controls
can restore route information. Transit data is dropped with the applicable
route error and moves the recovery deadline to its arrival plus ``deletePeriod``.
Route replies, discovery and Hello advertisements wait; one-hop ACKs and route
errors remain permitted. On expiry, a usable learned route releases held data
without an unnecessary request. Stop/crash removes this instance's routes and
cancels pending work before a later restart.

``interface`` remains the primary address and legacy link selection. The optional
``interfaces`` parameter selects multiple same-family links with a whitespace-separated
list of exact names, for example ``"wlan0 wlan1"``. The list must include the
primary; an empty value selects only that primary interface. Selected interfaces
must be up, nonloopback and addressed at actual startup, with distinct local
addresses. Each selected local address answers requests using the shared
node sequence. Discovery copies use each link's IP source while preserving the
primary originator identity and one logical request/rate charge.

Unicast replies, data and errors follow the learned next-hop interface. Hello
source and advertised address belong to their outgoing interface, with separate
broadcast suppression and neighbor observations per link. Control received on an
unselected interface is ignored; legitimate data can still traverse an external
gateway interface. Every selected IPv4 subnet contributes to internal address
scope, and a gateway's default adapter follows its learned outgoing interface.

The configured interface names apply at the next start. While running, AODV
tracks disappearance, reappearance and address changes of those names. Losing a
secondary link invalidates its paths while preserving retained freshness and
unaffected links. Losing the primary identity suspends protocol output; its
return begins the same recovery hold used by restart. Native manual routes keep
their precedence. For AckingWirelessInterface
profiles that must receive immediately after restart, set the radio's
``initialRadioMode`` to ``receiver`` or ``transceiver``; its default ``off`` can
otherwise wait for a local transmission to activate reception.

To export an attached subnet, set ``subnetExtensionType`` consistently to a
scenario-local value in ``128..255`` on participating peers and configure, for
example, ``advertisedSubnets = xml("<subnets><subnet prefix='192.0.2.0/24'
interface='eth0'/></subnets>")``. The default extension value, zero, disables
this profile; no extension number is claimed as an IANA assignment. An attached
export must be contained in the interface's connected IPv4 prefix or its own
advertised on-link IPv6 prefix. Host prefixes and default routes are not exports.
``externalSubnets`` uses the same XML form for a network reached through an
independent native route covering the whole prefix on the named interface.
These exports cannot be combined with the legacy ``gatewayAddress`` adapter.

An authority replies on demand with the requested member address and aggregate
prefix. Prefix-qualified request extensions carry retained aggregate freshness
without comparing it with an independent host counter. Members use native
forwarding and address resolution; an unreachable member produces the native
ICMP error without invalidating the aggregate. Mixed peers that cannot process
the mandatory profile extension cannot provide its recovery guarantees.

Multiple authorities can supply a common ``candidates`` address list and a
``coordinationInterface`` attribute on each export. They exchange announcements
on ``subnetCoordinationPort`` and select the lowest eligible address after a
qualification interval. This model assumes trusted, finite candidates on a
common connected network, reliable delivery within ``subnetAnnouncementDelay``
and fail-stop failures. It does not guarantee a single authority across arbitrary
partitions. ``subnetAnnouncementInterval`` controls the announcement period;
the coordination port must differ from ``udpPort``.

RFC 3561 sections 6.1 and 6.6.1 give conflicting descriptions of destination
sequence updates. This model follows the freshness rule of section 6.1: the
destination adopts a known fresher requested sequence before replying, without
an additional unconditional increment. An unknown requested sequence does not
advance the destination's sequence.

Reply and Hello lifetimes are relative durations encoded in whole milliseconds,
truncated downward. Configured advertisement durations must yield a count in
``1..4294967295``. A cached route whose remaining duration rounds to zero or
exceeds that range does not generate a reply. The request continues forwarding
when its TTL permits. Gratuitous replies likewise advertise the remaining
reverse-route duration, so their lease remains meaningful late in a simulation.

A nonduplicate request refreshes its reverse-route lifetime independently of
whether its sequence or hop count improves the selected path. Refresh and path
replacement preserve an already later expiry.

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
