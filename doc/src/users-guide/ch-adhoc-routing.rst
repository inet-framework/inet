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

Native IEEE 802.11 mesh forwarding is also available. It discovers paths at
Layer 2 and keeps mesh endpoints on one IP subnet. The mesh interface does
not require an IP routing application on each relay.

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

.. _ug:sec:adhocrouting:mesh:

IEEE 802.11 Mesh and HWMP
------------------------

Select :ned:`Ieee80211MeshInterface` on ordinary :ned:`StandardHost` nodes
to establish mesh peer links and relay frames through intermediate stations.
The interface combines open mesh peering, native mesh forwarding and HWMP
path selection. A relay changes the wireless next hop and mesh TTL while
preserving the end-to-end mesh addresses and IP TTL.

Keep the endpoints on one subnet, use dynamic ARP, and install direct
interface routes without a gateway. Disable IP forwarding on every node:

.. code-block:: ini

   *.station[*].wlan[0].typename = "Ieee80211MeshInterface"
   *.station[*].wlan[0].meshId = "example-mesh"
   *.station[*].forwarding = false
   *.station[*].ipv4.arp.typename = "Arp"
   *.configurator.addStaticRoutes = false
   *.configurator.addManualRoutes = true

The ``examples/wireless/mesh`` directory supplies complete configurations,
address/route XML, ScenarioManager scripts and passive wireless captures.
``LineOfdm`` connects three stations through a relay. ``DiamondOfdm`` offers
two paths with different airtime costs; ``FailureAlternate`` moves a relay
to break a link. ``RootPreq`` and ``RootRann`` show the two root mechanisms.
Run a named configuration from that directory, for example:

.. code-block:: shell

   inet --debug -u Cmdenv -f omnetpp.ini -c LineOfdm -r 0

HWMP normally discovers a path when an individual destination is needed.
Set ``wlan[0].pathSelection.rootMode`` to 3 for a proactive PREQ root or 4
for a RANN root. A RANN identifies a root; confirmation PREQ/PREP exchanges
establish its usable path. Configure discovery and packet-wait budgets to
cover the rendezvous interval when sleeping peers are involved.

PHY profiles, aggregation and metrics
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``phyProfile="ofdm"`` uses the scalar radio and metric ID 1 by default.
``"ht"`` and ``"vht"`` select the complete HT/VHT catalog and metric ID 2.
Use a dimensional radio and matching medium for HT/VHT. Fix the transmitted
MCS, bandwidth, spatial streams and guard interval through
``mac.hcf.rateSelection``; adaptive rate control is outside this profile.
The nested examples use 20 MHz, one spatial stream, MCS 7 and long GI.
Wider operation is limited to the model's contiguous 5 GHz allocations and
the committed directional peer capabilities. Optional 160 MHz capability
must be explicitly enabled with ``mib.meshVht160MhzCapable`` before use.
Regulatory permission, channel availability and DFS decisions are external.

Enable ``BasicMsduAggregationPolicy`` to combine compatible source MSDUs
and ``BasicMpduAggregationPolicy`` to place MPDUs in a native A-MPDU. The
latter requires ``hcf.isBlockAckSupported=true`` and compressed Block Ack.
Forwarding preserves an existing A-MSDU's constituents; selective Block Ack
recovery retries missing MPDUs without delivering received constituents twice.
The ``HtNested`` and ``VhtNested`` examples combine both aggregation levels.

Use one ``pathSelectionMetricId`` throughout a mesh. ID 1 encodes airtime
in units of 10.24 microseconds and fixes the aggregation factor ``n`` to 1.
ID 2 uses units of 0.01 microseconds and divides the nominal overhead by
the estimated number ``n`` of MSDUs per payload PPDU. Both use a nominal
8192-bit payload and divide by ``1-ef``, where ``ef`` is the directional
error estimate. Values are rounded up and saturated; metric values from
different IDs cannot be compared directly. The ``*Metric1`` examples select
ID 1 explicitly on all HT/VHT stations.

The local error EWMA starts at zero and has gain 1/8 per actual payload
attempt. The payload-count EWMA starts at one and has the same gain, once
per actual payload PPDU. QoS Null and sleep/eligibility holds provide neither
sample. Peer retirement and isolated metric stop/crash reset both estimates;
an aggregation reset clears ``n`` alone. These are estimator policies of
this model, not measurements of energy consumption.

Peer power modes
~~~~~~~~~~~~~~~~

Set ``wlan[0].powerSave=true`` on every participating peer, including a
peer that remains ACTIVE. After automatic peering, ScenarioManager can
request an individual peer's ``active``, ``light-sleep`` or ``deep-sleep``
mode using ``set-mesh-power-mode`` on ``wlan[0].powerSavePolicy``. Use fixed
MAC addresses in those scripts. A lower-activity request commits only after
its real acknowledgment; higher activity takes effect immediately. The
script timestamp is a request time, and a command is rejected when the
owner is unavailable or the peer is not established. A request is not
automatically reapplied after re-peering. See the scenario-scripting chapter
for the command syntax.

LIGHT stations rendezvous for the required beacons and awake windows.
DEEP stations use the selected lowest-address established peer as a timing
reference and the model's ten-local-beacon-period maintenance schedule.
Loss of a usable reference keeps the station conservatively awake until
timing is reacquired. Group traffic follows DTIM delivery. Individual buffered
traffic uses directional mesh peer service periods; a final EOSP is confirmed
by its ACK or Block Ack. Several exchanges may belong to the same period.
The radio may sleep only after all remaining obligations permit it.

A DEEP station may miss corresponding peer beacons and their unacknowledged
group frames, including HWMP management broadcasts. The DEEP example keeps
its endpoints ACTIVE and places the destination's beacon within the relay's
own awake window so reverse discovery has a shared rendezvous. Its traffic
starts before the selected reference rendezvous. Different beacon phases
and workloads may require different discovery/ARP budgets or accept ordinary
broadcast loss; selecting DEEP does not guarantee reception of every broadcast.

The default beacon interval is 102.4 ms, advertised awake window 10.24 ms
and DTIM period one beacon. ``powerSavePolicy.wakeMargin`` defaults to zero;
the wake lead also includes the public radio switching delay and one
simulation tick. Radio-awake operation means TRANSCEIVER. The shared
ending-MPDU recovery limit ``missingAckRetryLimit`` defaults to 7 actual
retransmissions and is a model policy independent of ordinary MAC retries.

Unresolved forwarding defaults to 64 packets, 256 KiB and a five-second
exclusive per-packet wait deadline, dropping new arrivals at capacity.
With power saving enabled, each EDCAF pending queue defaults to 64 packets
and 256 KiB. Ordinary QoS DATA lifetime defaults to 512 ms, including queue
residence; increase it explicitly when demonstrating longer rendezvous.
A sleep hold never renews that lifetime or turns an expiry into a successful
transmission. The examples declare their longer finite sleep budgets.

``LightRelay``, ``DeepRelay`` and ``DtimGroup`` use ordinary scenario scripts.
``SleepingHtNested`` and ``SleepingVhtNested`` combine service periods with
nested aggregation. Mesh power save is separate from HT SMPS and
infrastructure APSD; enabling it does not enable either mechanism.

Supported scope and inspection
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The profile uses open peer links, one interface and one fixed channel,
native mesh endpoints and fixed PHY selection. It excludes portals, proxy
addressing, security, adaptive rate control, RTS and fragmentation. Isolated
mesh-owner stop/start/crash cleanup is supported; whole-NIC or whole-node
restart is outside the validated profile. The examples demonstrate IPv4
with dynamic ARP; they make no IPv6 or external interoperability claim.

Inspect management peer states/AIDs, MIB peer PHY/power views, HWMP paths
and power awake requirements in the runtime inspector. Directional period
views are available through ``getServicePeriods()`` in a source debugger;
the outcome signal reports their completion or withdrawal. Radio mode
vectors identify actual sleep intervals. Each example records separate
transmit and good-receive radiotap PCAPs, with native aggregate members
split by the production recorder. A transmit capture alone does not prove
reception or application delivery. Protocol operation is validated separately
from any claim about battery-life accuracy.
