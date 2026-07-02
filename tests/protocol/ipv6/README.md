# IPv6 Conformance Test Suite

A **spec-driven** conformance suite for INET's IPv6 stack, built on the protocol-test framework
in [`../lib`](../lib) — a sibling of the [`../wifi`](../wifi) suite. It covers:

- **Neighbor Discovery** (RFC 4861): Router Solicitation/Advertisement, address resolution (NS/NA),
  Neighbor Unreachability Detection, and Redirect;
- **SLAAC + Duplicate Address Detection** (RFC 4862);
- **ICMPv6** (RFC 4443): Echo, and the error messages (Destination Unreachable, Time Exceeded,
  Packet Too Big);
- **Multicast Listener Discovery** (RFC 2710 / 3810, MLDv1 + MLDv2);
- **Forwarding, fragmentation & Path MTU Discovery** (RFC 8200 / 8201);
- **Mobile IPv6** (RFC 6275): home registration, return-routability, route optimization, return-home;
- **Proxy Mobile IPv6** (RFC 5213): network-based mobility (PBU/PBA, tunnelling, handover);
- **DHCPv6** (RFC 8415): stateful configuration — a gap (not modeled in INET).

Tests are written against the **standard**, not against INET's implementation. A test that fails
because INET is incomplete is a **finding**, not a suite defect — the suite doubles as a
conformance/gap report. Each test program asserts the spec-required behavior; the `.test` wrapper
records the *current* expected outcome so the suite is also a stable CI gate.

## Layout & how it works

```
ipv6/
  run-tests.sh        build + run everything via opp_test (-> one `ipv6tests` binary)
  Ipv6TestSupport.h   C++ predicates for parts of a message PacketFilter can't address
  ned/                Ipv6LanNetwork (one LAN) + Ipv6RoutedNetwork (multi-hop)
  ini/                _base.ini (on-link SLAAC config) + _routed.ini (routed config)
  dad/                DAD + SLAAC (RFC 4862)
  nd/                 Neighbor Discovery: RS/RA, NS/NA, Redirect (RFC 4861)
  nud/                Neighbor Unreachability Detection (RFC 4861 7.3)
  icmpv6/             ICMPv6 Echo + errors (RFC 4443)
  mld/                Multicast Listener Discovery (RFC 2710 / 3810)
  forwarding/         Hop Limit decrement + fragmentation (RFC 8200)
  mipv6/              Mobile IPv6 registration + route optimization (RFC 6275)
  pmipv6/             Proxy Mobile IPv6 network-based mobility (RFC 5213)
  dhcpv6/             Stateful address configuration (RFC 8415, NOT-MODELED)
```

Each test is one **`.test`** file: its program lives in a `%file: <Name>.cc`
(a named `Define_ProtocolTest(...)`); `opp_test gen` extracts all of them and a single
`--deep` build links them into **one `ipv6tests` binary**, selected per test by the
`ProtocolTester.testName` parameter. `%contains` asserts the verdict.

```sh
# omnetpp tools on PATH (or: source <inet>/setenv); ../lib/libprotocoltest.so built first
./run-tests.sh                       # all tests
./run-tests.sh nd/Nd_RouterSolicitation.test   # a subset
```

### The networks

- **`Ipv6LanNetwork`** — an advertising `Router6` and two `StandardHost6` hosts joined by an
  `EthernetSwitch` into a single IPv6 link. From this one configuration the whole boot sequence
  unfolds by itself: link-local DAD, Router Solicitation/Advertisement, SLAAC global-address
  formation + DAD, and (with a host→host flow added per-test) address resolution, NUD, on-link
  ICMPv6 (Echo, Port Unreachable) and MLD. The router advertises the on-link prefix via its
  per-node `routes` XML (see *Findings*).
- **`Ipv6RoutedNetwork`** — host1, routerA and routerB share one link (switch); host2 is on a
  second link behind routerB. Because both routers are on host1's link, the flat configurator
  produces routes where a router forwards back out the arrival interface, which triggers ICMPv6
  **Redirect** (RFC 4861 8). host1→host2 traffic (off-link) also exercises Hop Limit decrement,
  the ICMPv6 errors (Time Exceeded, Packet Too Big, No-Route), source fragmentation, and NUD of
  the next-hop router. Modelled on INET's own `tests/module/IPv6_redirect`.

- **`Mipv6Demo`** (from [`../lib`](../lib)) — a wireless network (Home Agent, Correspondent Node,
  two routers + access points, and a roaming mobile node) for the Mobile IPv6 tests. The MN roams
  from its home to a foreign AP over tens of seconds; the config (`ini/_mipv6.ini`) mirrors the
  `[Config Mipv6Scenario]` in `../lib/omnetpp.ini` (and `ini/_mipv6_returnhome.ini` a faster roam so
  the MN returns home).
- **`Pmipv6Demo`** (`ned/`) — a Proxy Mobile IPv6 domain: a Local Mobility Anchor, two Mobile Access
  Gateways, a correspondent node, and a plain wireless mobile node (no mobility software) that roams
  mag1→mag2 keeping its address. Modelled on `examples/ipv6/pmipv6`; config in `ini/_pmipv6.ini`.

Per-test knobs (traffic, DAD disabled, same-MAC collision, MTU, MLD on) are layered in each
`.test`'s `%inifile` after `include ../../ini/_base.ini` (or `_routed.ini` / `_mipv6.ini`).

### Observation model

Nodes are treated as **black boxes**: the suite observes the packets they actually put on and take
off the link — what the RFC specifies — at the **network interface** (`X.eth[0].mac`):

- a node **sends** a packet on the wire → signal `packetSentToLower`
- a node **receives** a packet from the wire → signal `packetReceivedFromLower`

At the interface the full Ethernet+IPv6 frame is present on both sides, so message fields
(`Ipv6NeighbourSolicitation.targetAddress`, `Icmpv6DestUnreachableMsg.code`, …) **and** IPv6-header
fields (`ipv6.srcAddress` / `ipv6.destAddress` / `ipv6.hopLimit`) are readable regardless of
direction. The only exception is **DAD completion/failure**, an internal transition with no
positive packet on the wire (success is the *absence* of a defence): those tests observe the node's
`startDad`/`dadCompleted`/`dadFailed` signals at the **host level** (`on("host1")`).
`Ipv6TestSupport.h` provides C++ predicates (`isDadProbe`, `nsTarget`, `raHasAutonomousPrefix`,
`mldGroup`, `chunkOfType<T>`, …) for what the PacketFilter string engine cannot express — notably
`Ipv6Address`-typed fields and nested TLV options.

## Outcome semantics

Every test asserts the RFC-required behavior, so `%contains` always expects the program to
`PASS`. The two outcomes differ only in whether INET actually does it:

- **CONFORMS ✅** — INET produces the spec behavior; the program PASSes → opp_test **PASS**.
- **NOT-MODELED ⛔** — INET does not implement the feature (or applies it inconsistently), so the
  faithful assertion misses its deadline and the program FAILs. The `.test` carries an
  `%expected-failure:` directive, so opp_test reports it as **EXPECTEDFAIL** (yellow) — an honest,
  first-class "expected failure", *not* a disguised PASS, and it does not fail the run. If INET
  later implements the feature the program PASSes and the test flips EXPECTEDFAIL → PASS — a
  visible signal to move the row from ⛔ to ✅. A real regression turns a ✅ test **FAIL** (red).

**Today: 49 CONFORMS (PASS), 5 NOT-MODELED (EXPECTEDFAIL) across 54 tests — aggregate PASS.**

## Conformance matrix

`[R]` required, `[O]` optional. ✅ CONFORMS · ⛔ NOT-MODELED.

### dad — Duplicate Address Detection + SLAAC (RFC 4862)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Dad_LinkLocalNs` | Link-local DAD probe (NS from :: to solicited-node mcast) | R | ✅ |
| `Dad_StartThenComplete` | DAD runs to completion (startDad → dadCompleted) | R | ✅ |
| `Dad_GlobalAddress` | DAD is also run for the SLAAC global address | R | ✅ |
| `Slaac_PrefixToAddress` | SLAAC address formed within the advertised prefix | R | ✅ |
| `Dad_Collision` | Duplicate detected (dadFailed) on same-MAC collision | R | ✅ |
| `Dad_Defends` | Owner defends an assigned address with an unsolicited NA | R | ✅ |
| `Dad_Disabled` | DupAddrDetectTransmits=0 suppresses the link-local probe | O | ✅ |
| `Dad_DisabledGlobal` | …should suppress the global probe too (RFC 4862 5.1) | O | ⛔ still probes global |
| `Dad_GratuitousNa` | Unsolicited NA after DAD (RFC 4862 5.4.4) | O | ⛔ NAs only on request |

### nd — Neighbor Discovery: RS/RA, NS/NA, Redirect (RFC 4861)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Nd_DadNsBeforeRs` | Address config (DAD) precedes router discovery (RS) | R | ✅ |
| `Nd_RouterSolicitation` | RS to all-routers (ff02::2), Hop Limit 255 | R | ✅ |
| `Nd_RouterAdvertisement` | RA to all-nodes (ff02::1) | R | ✅ |
| `Nd_RaSolicited` | RA sent in response to an RS | R | ✅ |
| `Nd_RaPeriodic` | Unsolicited RAs re-sent periodically | R | ✅ |
| `Nd_RaManagedFlags` | M/O flags clear under stateless config | R | ✅ |
| `Nd_RaRouterLifetime` | Non-zero Router Lifetime (a default router) | R | ✅ |
| `Nd_RaPrefixOption` | Prefix Information option (autonomous) drives SLAAC | R | ✅ |
| `Nd_HopLimit255` | ND messages sent with IPv6 Hop Limit 255 | R | ✅ |
| `Nd_RaMtuOption` | RA carries an MTU option (RFC 4861 4.6.4) | O | ✅ |
| `Nd_AddressResolution` | NS → solicited NA resolves a neighbour | R | ✅ |
| `Nd_NaSolicitedFlag` | Solicited NA sets S=1, host clears R=0 | R | ✅ |
| `Nd_NsSolicitedNodeMulticast` | Resolution NS to the target's solicited-node mcast | R | ✅ |
| `Nd_NsHasSlla` | Resolution NS carries the Source Link-Layer Address option | R | ✅ |
| `Nd_NaHasTlla` | Solicited NA carries the Target Link-Layer Address option | R | ✅ |
| `Nd_Redirect` | Router sends an ICMPv6 Redirect (RFC 4861 8) | R | ✅ |
| `Nd_RedirectTarget` | Redirect names an on-link (link-local) better next hop | R | ✅ |
| `Nd_RouterPreference` | RFC 4191 Default Router Preference | O | ⛔ not implemented |

### nud — Neighbor Unreachability Detection (RFC 4861 7.3)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Nud_RouterProbe` | STALE next hop re-verified with a unicast NS probe | R | ✅ |

### icmpv6 — ICMPv6 Echo & errors (RFC 4443)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Icmpv6_EchoRequestReply` | Echo Request answered by Echo Reply (ping6) | R | ✅ |
| `Icmpv6_EchoIdentifier` | Reply echoes the Request's identifier + seq number | R | ✅ |
| `Icmpv6_PortUnreachable` | Dest Unreachable (Port) for a closed UDP port | R | ✅ |
| `Icmpv6_DestUnreachableNoRoute` | Dest Unreachable (No Route) when no route exists | R | ✅ |
| `Icmpv6_TimeExceeded` | Time Exceeded when Hop Limit reaches zero | R | ✅ |
| `Icmpv6_PacketTooBig` | Packet Too Big for an oversized forward | R | ✅ |
| `Icmpv6_PacketTooBigMtu` | …carries the next-hop MTU (RFC 4443 3.2 / PMTUD) | R | ⛔ MTU field left 0 |

### mld — Multicast Listener Discovery (RFC 2710 / 3810)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Mld_Report` | Host joining a group sends a Multicast Listener Report | R | ✅ |
| `Mld_Query` | Router querier sends Multicast Listener Queries | R | ✅ |
| `Mld_Done` | Last listener leaving sends a Multicast Listener Done | R | ✅ |
| `Mldv2_Report` | MLDv2 Version 2 Report (type 143) with group records | R | ✅ |
| `Mldv2_SourceInclude` | MLDv2 source-specific membership (INCLUDE filter) | R | ✅ |

### forwarding — Forwarding + Fragmentation (RFC 8200)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Fwd_HopLimitDecrement` | Router decrements Hop Limit of forwarded packets | R | ✅ |
| `Frag_Fragmentation` | Source fragments a datagram larger than the link MTU | R | ✅ |
| `Frag_Reassembly` | Fragments delivered end-to-end to the destination | R | ✅ |

### mipv6 — Mobile IPv6 (RFC 6275)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Mipv6_HomeRegistration` | Binding Update (H+A, lifetime) → Binding Ack (status 0, matching seq) | R | ✅ |
| `Mipv6_ReturnRoutability` | HoTI/CoTI → HoT/CoT with cookies echoed + keygen tokens | R | ✅ |
| `Mipv6_RouteOptimization` | Correspondent-registration Binding Update (H flag clear) | R | ✅ |
| `Mipv6_Tunneling` | Correspondent traffic to the home address tunnelled to the care-of address | R | ✅ |
| `Mipv6_ReturnHome` | De-registration Binding Update (lifetime 0) on returning home | R | ✅ |

### pmipv6 — Proxy Mobile IPv6 (RFC 5213)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Pmipv6_ProxyBindingUpdate` | MAG registers the MN with a Proxy Binding Update (P flag) | R | ✅ |
| `Pmipv6_ProxyBindingAck` | LMA replies with a Proxy Binding Acknowledgement (P flag) | R | ✅ |
| `Pmipv6_Handover` | The new MAG re-registers the MN on handover | R | ✅ |
| `Pmipv6_Tunnel` | MN traffic tunnelled (IPv6-in-IPv6) between LMA and MAG | R | ✅ |
| `Pmipv6_AddressConstancy` | The MN keeps its address (and reachability) across the handover | R | ✅ |

### dhcpv6 — Stateful address configuration (RFC 8415)
| Test | Feature | R/O | |
|------|---------|-----|--|
| `Dhcpv6_Solicit` | DHCPv6 SOLICIT (stateful config) | O | ⛔ no DHCPv6 client |

## Findings & notes

- **RFC 4191 Default Router Preference not implemented** (`Nd_RouterPreference`): all routers are
  equivalent; the RA carries no preference (reserved bits stay zero).
- **No gratuitous NA after DAD** (`Dad_GratuitousNa`): INET sends NAs only in response to an NS;
  RFC 4862 5.4.4's optional unsolicited announcement is not sent (a natural `Ipv6` module option).
- **DHCPv6 is not modeled** (`Dhcpv6_Solicit`): INET has no DHCPv6 client (no message set or module),
  so stateful address configuration cannot occur; hosts only ever use SLAAC.
- **DAD-disable is applied inconsistently** (`Dad_DisabledGlobal`): `DupAddrDetectTransmits=0`
  suppresses DAD for the link-local address but the SLAAC global address is still probed once.
- **Packet Too Big carries no MTU** (`Icmpv6_PacketTooBigMtu`): INET generates the message but leaves
  the MTU field zero (a `// TODO set MTU` in `Ipv6.cc`), so Path MTU Discovery cannot converge on the
  correct size. Relatedly, INET does not truncate the error to 1280 bytes (RFC 4443 2.4) — it embeds
  the full original packet — so the suite reduces only routerB's *far* interface, letting the error
  return unfragmented.
- **RA rate-limiting is not enforced** (bug): the `MIN_DELAY_BETWEEN_RAS` throttle is commented out in
  `Ipv6NeighbourDiscovery.cc`. Not asserted here.
- **Configurator does not advertise a prefix across a switch**: with a switched LAN,
  `Ipv6NetworkConfigurator` adds on-link routes but installs no `AdvPrefix` on the router interface, so
  its RAs carry no Prefix Information option and SLAAC never starts (it works with *direct*
  host↔router links). The suite works around this via the router's per-node `routes` XML in `_base.ini`.
- **DAD for configurator/manually-assigned addresses is intentionally skipped** (converged-snapshot
  model), so the suite exercises DAD on the autoconfigured (SLAAC) addresses.
- **Interface-MTU config gotcha**: reducing an Ethernet interface's effective MTU needs
  `**.<node>.eth[N].**.mtu` (or `**.<node>.**.mtu`); the plain `**.<node>.eth[N].mtu` form does not
  take effect.

## Out of scope (this cut)

MLDv2 EXCLUDE-mode / state-change record specifics, and IPsec-over-IPv6 (covered elsewhere). The
suite is structured so these drop in later as new `.test` files.

## Adding a test

Copy an existing `.test` (e.g. `nd/Nd_RouterSolicitation.test`). Put the program in
`%file: <Name>.cc` (unique basename) with a `Define_ProtocolTest(ipv6_<name>)`, select it via
`*.tester.testName`, `include ../../ini/_base.ini` (on-link) or `../../ini/_routed.ini` (routed) plus
any inline overrides. Assert the RFC behavior and set `%contains` to `PROTOCOLTEST <name>: PASS`; for
a NOT-MODELED point INET can't satisfy, also add `%expected-failure: <reason>` so it is reported as
EXPECTEDFAIL rather than a red FAIL. Observe sends at `X.eth[0].mac` `packetSentToLower`, receives at
`packetReceivedFromLower`, and DAD state via the host-level `startDad`/`dadCompleted`/`dadFailed`
signals. To author against the real trace, add a tester with `logEvents=true` and read
`work/<Name>/test.out`.
