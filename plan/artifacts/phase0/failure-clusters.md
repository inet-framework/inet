# Module Test Failure Clusters — topic/infrastructure branch

Analysis of 179 FAIL tests from `smoke2-module.log` and 4 queueing failures.

---

## Cluster Table (sorted by count desc)

| # | Count | Normalized Signature | Examples (up to 3) | Verbatim Error Line |
|---|-------|----------------------|--------------------|---------------------|
| 1 | **134** | `check_and_cast(): Cannot cast (inet::MessageDispatcher*)<modulepath> to type 'inet::Icmp *' -- in module (inet::Udp) <modulepath> (id=N), during network initialization` | AntennaOrientation_1, udpapp_lifecycle_1, lifecycle_1 | `<!> Error: check_and_cast(): Cannot cast (inet::MessageDispatcher*)Test.host1.tn to type 'inet::Icmp *' -- in module (inet::Udp) Test.host1.udp (id=22), during network initialization` |
| 2 | **30** | `Cannot find referenced module interface, module = (inet::ieee80211::Ieee80211LlcLpd)<modulepath> id=N, gate = (omnetpp::cGate)lowerLayerOut --> mac.upperLayerIn, type = inet::queueing::IPassivePacketSink, arguments = DispatchProtocolReq, protocol = ethernetmac(12), servicePrimitive = 1 (SP_REQUEST), direction = 0 -- in module (inet::ieee80211::Ieee80211LlcLpd) <modulepath> (id=N), during network initialization` | AODVSimpleTest, Ieee80211_1, lifecycle_WirelessHost_1 | `<!> Error: Cannot find referenced module interface, module = (inet::ieee80211::Ieee80211LlcLpd)llc id=35, gate = (omnetpp::cGate)lowerLayerOut --> mac.upperLayerIn, type = inet::queueing::IPassivePacketSink, arguments = DispatchProtocolReq, protocol = ethernetmac(12), servicePrimitive = 1 (SP_REQUEST), direction = 0 -- in module (inet::ieee80211::Ieee80211LlcLpd) AODVTest.sender.wlan[0].llc (id=35), during network initialization` |
| 3 | **7** | `%contains(1) FAIL: protocol IDs changed in golden output` — simulation ran OK but expected ppp(52)/ipv4(38)/tcp(60), actual output has ppp(54)/ipv4(39)/tcp(63) | ModuleInterfaceLookup_EthernetInterface, ModuleInterfaceLookup_Ipv4, ModuleInterfaceLookup_Tcp | `expected: DispatchProtocolReq, protocol = ppp(52), servicePrimitive = 1 (SP_REQUEST) --> not found \| actual: DispatchProtocolReq, protocol = ppp(54), servicePrimitive = 1 (SP_REQUEST) --> not found` |
| 4 | **6** | `Cannot find referenced module interface, module = (inet::Ipv6NeighbourDiscovery)<modulepath> id=N, gate = (omnetpp::cGate)ipv6Out --> ipv6.ndIn, type = inet::queueing::IPassivePacketSink, arguments = <nullptr>, direction = 0 -- in module (inet::Ipv6NeighbourDiscovery) <modulepath> (id=N), during network initialization` | ICMPv6_delivery, IPv6_fragmentation, lo0_IPv6 | `<!> Error: Cannot find referenced module interface, module = (inet::Ipv6NeighbourDiscovery)neighbourDiscovery id=63, gate = (omnetpp::cGate)ipv6Out --> ipv6.ndIn, type = inet::queueing::IPassivePacketSink, arguments = <nullptr>, direction = 0 -- in module (inet::Ipv6NeighbourDiscovery) NClientsEth.r1.ipv6.neighbourDiscovery (id=63), during network initialization` |
| 5 | **1** | `check_and_cast(): Cannot cast (inet::queueing::PassivePacketSink*)<modulepath> to type 'inet::Icmp *' -- in module (inet::Udp) <modulepath> (id=N), during network initialization` | ModuleInterfaceLookup_Udp | `<!> Error: check_and_cast(): Cannot cast (inet::queueing::PassivePacketSink*)TestNetwork.lowerSink to type 'inet::Icmp *' -- in module (inet::Udp) TestNetwork.udp (id=4), during network initialization` |
| 6 | **1** | `NO_TEST_ERR` — binary compiled but simulation produced no output (no test.err, no test.out) | tun-echo | _(no error output)_ |

**Total: 134 + 30 + 7 + 6 + 1 + 1 = 179** ✓

---

## Cluster Details

### Cluster 1 — 134 tests: check_and_cast Udp→Icmp (MessageDispatcher source)

All 134 tests crash during network initialization with Udp attempting to cast a `MessageDispatcher*` (via `.tn` gate) to `inet::Icmp *`.

**Caster variants:** Only `inet::Udp` appears as the caster module in all 134 tests. No Tcp or Sctp casters.

**Target variants:** Only `inet::Icmp *` appears as the cast target in all 134 tests. No Icmpv6 target.

**Source-pointer variants:**
- `(inet::MessageDispatcher*)<modulepath>` — 134 tests
- _(Cluster 5 below is a related but distinct case where source is `inet::queueing::PassivePacketSink*`)_

Full test list:
AntennaOrientation_1–12, AODVLifecycleTest, AODVShortestPath, AODVSimpleTest_2,
DHCP_1, DHCP_2, DHCP_lifecycle_1–3,
IGMP_basic, IGMP_host_groupstates, IGMP_nonquerier_groupstates, IGMP_querier_groupstates, IGMP_router_ifstates,
Interference_APSKDimensionalRadio_{Collision,Reception}_{SS_1,SS_2,SW,WS,WW,SS,SW,WS},
Interference_APSKScalarRadio_{Collision,Reception}_{SS_1,SS_2,SW,WS,WW,SS,SW,WS},
Interference_IdealRadio_{Collision,Reception}_{SS_1,SS_2,SS_3,SW,WS,WW,SW,WS},
internetCloud_1–4,
IPv4_ICMPerror_NoProtocol, IPv4NetworkConfigurator_1/1a/1b/1c, IPv4_refragmentation,
lifecycle_1–3, lo0_IPv4,
NeighborCache_Grid/NeighborList/Off/QuadTree,
ospf_1_area, ospf_1_area_HostInterface, ospf_1_area_lifecycle,
ospf_backbone_and_2_areas, ospf_backbone_and_2_areas_HostInterface,
ospf_backbone_and_2_stub, ospf_backbone_and_3_areas_with_virtual_link,
pingapp_1, pingapp_lifecycle_1–8,
ReceptionState_{APSKDimensionalRadio,APSKScalarRadio,IdealRadio}_{Busy,Idle,Receiving},
rip_1–3, ScenarioManager_disconnect_1,
sctp_addip_addAddress, sctp_addip_setPrimary, sctp_auth, sctp_congestion, sctp_failover,
sctp_nat_peer_to_server, sctp_prsctp_rtx0/rtx1/ttl, sctp_streamReset, sctp_streams,
tcp_algorithm_{dumb,newreno,reno,tahoe,vegas,westwood},
tcpapp_lifecycle_1–8,
udpapp_lifecycle_1–8,
UDP_dscp_ipv4, UDPSocket_1, UDPSocket_2, UDP_tos_ipv4, UDP_ttl_ipv4

### Cluster 2 — 30 tests: Ieee80211LlcLpd cannot find IPassivePacketSink interface

Initialization fails because `Ieee80211LlcLpd.llc` cannot find a `IPassivePacketSink` module interface via its `lowerLayerOut` gate towards `mac.upperLayerIn`, looking for `DispatchProtocolReq / protocol=ethernetmac(12)`.

Full test list:
AODVSimpleTest, Ieee80211_1–4, Ieee80211Retransmission1–10, InterpolatingAntenna,
IPv4NetworkConfigurator_2/3, lifecycle_AdhocHost_1–3,
lifecycle_WirelessHost_1–5/switchingtime, ModuleInterfaceLookup_Ieee80211Interface,
Power_4, Power_5

### Cluster 3 — 7 tests: %contains(1) golden output has stale protocol IDs

Simulation completes successfully, but the `%contains(1)` rule fails because protocol numbers in the golden `.txt` files use old values (ppp=52, ipv4=38, tcp=60) while the current code emits new values (ppp=54, ipv4=39, tcp=63).

Tests: ModuleInterfaceLookup_EthernetInterface, ModuleInterfaceLookup_Ipv4,
ModuleInterfaceLookup_LayeredEthernetInterface, ModuleInterfaceLookup_LoopbackInterface,
ModuleInterfaceLookup_PassivePacketSink, ModuleInterfaceLookup_PppInterface,
ModuleInterfaceLookup_Tcp

### Cluster 4 — 6 tests: Ipv6NeighbourDiscovery cannot find IPassivePacketSink interface

Initialization fails because `Ipv6NeighbourDiscovery.neighbourDiscovery` cannot find a `IPassivePacketSink` module interface via its `ipv6Out` gate towards `ipv6.ndIn`.

Tests: ICMPv6_delivery, IPv6_fragmentation, lo0_IPv6, UDP_dscp_ipv6, UDP_tos_ipv6, UDP_ttl_ipv6

### Cluster 5 — 1 test: check_and_cast PassivePacketSink→Icmp

`ModuleInterfaceLookup_Udp`: same Udp→Icmp cast failure but the source pointer type is `inet::queueing::PassivePacketSink*` (not `MessageDispatcher*`) because this test uses a `PassivePacketSink` module as the stub downstream module, not a full `MessageDispatcher`.

### Cluster 6 — 1 test: tun-echo (no simulation output)

`tun-echo`: work directory contains only the compiled `tun-echo_dbg` binary and `Makefile`; no `test.err` or `test.out` were produced. The simulation did not execute (possible missing TUN device or immediate crash before output buffering). Discernible error: none.

---

## check_and_cast Variant Analysis (Task 3)

Across all 135 check_and_cast failures (clusters 1 and 5):

| Caster module type | Cast-from pointer type | Cast-to target | Count |
|---|---|---|---|
| `inet::Udp` | `inet::MessageDispatcher*` | `inet::Icmp *` | 134 |
| `inet::Udp` | `inet::queueing::PassivePacketSink*` | `inet::Icmp *` | 1 |

**No Tcp, no Sctp, no Icmpv6 variants.** All 135 failures go through `inet::Udp` casting to `inet::Icmp *`.

---

## Queueing Test Failures (Gate_1, Gate_2, Gate_3, PeriodicGate_1)

All 4 queueing tests ran to completion (empty `test.err`). The failure is in `%contains-regex(1)`: the actual stdout now contains **new "Method call X --> Y: methodName" log lines** interleaved between the expected packet-flow lines, breaking the contiguous multi-line pattern match.

### Gate_1 — failing rule: `%contains-regex(1)`

Expected block (abridged):
```
At 3s gate: Opening gate.
At 3s producer: Producing packet, .*?producer-0.*?
At 3s gate: Passing through packet, .*?producer-0.*?
At 3s consumer: Consuming packet, .*?producer-0.*?
...
```

Actual output (lines 78–86):
```
At 3s gate: Opening gate.
At 3s producer: Method call gate --> producer: handleCanPushPacketChanged   ← NEW
At 3s producer: Producing packet, packet = (Packet)producer-0 (1 B) ByteCountChunk, length = 1 B, data = 63.
At 3s gate: Method call producer --> gate: pushPacket                        ← NEW
At 3s gate: Passing through packet, packet = (Packet)producer-0 (1 B) ByteCountChunk, length = 1 B, data = 63.
At 3s consumer: Method call gate --> consumer: pushPacket                    ← NEW
At 3s consumer: Consuming packet, packet = (Packet)producer-0 (1 B) ByteCountChunk, length = 1 B, data = 63.
```

### Gate_2 — failing rule: `%contains-regex(1)`

Same pattern: "Method call" lines inserted between packet flow events (pull-side: `handleCanPullPacketChanged`, `canPullPacket`, `pullPacket`).

Actual lines 79–86:
```
At 3s gate: Opening gate.
At 3s collector: Method call gate --> collector: handleCanPullPacketChanged   ← NEW
At 3s provider: Method call collector --> provider: canPullPacket             ← NEW
At 3s gate: Method call collector --> gate: pullPacket                        ← NEW
At 3s provider: Method call gate --> provider: pullPacket                     ← NEW
At 3s provider: Providing packet, packet = (Packet)provider-0 (1 B) ByteCountChunk, length = 1 B, data = 63.
At 3s gate: Passing through packet, packet = (Packet)provider-0 (1 B) ByteCountChunk, length = 1 B, data = 63.
At 3s collector: Collecting packet, packet = (Packet)provider-0 (1 B) ByteCountChunk, length = 1 B, data = 63.
```

### Gate_3 — failing rule: `%contains-regex(1)`

Same "Method call" lines PLUS new `Starting/Ending packet streaming` events from the gate itself.

Expected block includes:
```
At 3s gate: Passing through packet, .*?producer-0.*?
At 3s destreamer: Starting destreaming packet, .*?producer-0.*?
```

Actual lines 121–135 (abridged):
```
At 3s gate: Opening gate.
At 3s streamer: Method call gate --> streamer: handleCanPushPacketChanged     ← NEW
At 3s producer: Method call streamer --> producer: handleCanPushPacketChanged ← NEW
At 3s producer: Producing packet, packet = (Packet)producer-0 ...
At 3s streamer: Method call producer --> streamer: pushPacket                 ← NEW
At 3s streamer: Starting streaming packet, ...
At 3s gate: Method call streamer --> gate: pushPacketStart                    ← NEW
At 3s gate: Starting packet streaming, ...                                    ← NEW
At 3s gate: Passing through packet, ...
At 3s destreamer: Method call gate --> destreamer: pushPacketStart            ← NEW
At 3s destreamer: Starting destreaming packet, ...
```

### PeriodicGate_1 — failing rule: `%contains-regex(1)`

Same "Method call" lines at t=0s before the first `Providing packet` event:

Expected first line: `At 0s provider: Providing packet, .*?provider-0.*?`

Actual lines 78–82:
```
At 0s gate: Method call provider --> gate: handleCanPullPacketChanged         ← NEW
At 0s provider: Method call collector --> provider: canPullPacket             ← NEW
At 0s gate: Method call collector --> gate: pullPacket                        ← NEW
At 0s provider: Method call gate --> provider: pullPacket                     ← NEW
At 0s provider: Providing packet, packet = (Packet)provider-0 ...
```

**Root cause common to all 4:** New verbose "Method call" logging (and for Gate_3 also "Starting/Ending packet streaming" events) has been added to queueing module infrastructure. These extra log lines break the `%contains-regex` contiguous-block match.

---

## Coverage Check

| Category | Count |
|---|---|
| Cluster 1: Udp→Icmp check_and_cast (MessageDispatcher) | 134 |
| Cluster 2: Ieee80211LlcLpd IPassivePacketSink not found | 30 |
| Cluster 3: %contains(1) stale protocol IDs | 7 |
| Cluster 4: Ipv6NeighbourDiscovery IPassivePacketSink not found | 6 |
| Cluster 5: Udp→Icmp check_and_cast (PassivePacketSink) | 1 |
| Cluster 6: tun-echo — no simulation output | 1 |
| **Total** | **179** |

**179 = 179** ✓

Tests with missing work dir: **0**
Tests with no discernible error (cluster 6, tun-echo): **1** — work dir exists, binary compiled, but no test output produced.
