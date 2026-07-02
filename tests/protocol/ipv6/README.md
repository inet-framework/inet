# IPv6 Neighbor Discovery + DAD/SLAAC Conformance Test Suite

A **spec-driven** conformance suite for INET's IPv6 Neighbor Discovery (RFC 4861) and
Stateless Address Autoconfiguration / Duplicate Address Detection (RFC 4862), built on the
protocol-test framework in [`../lib`](../lib) — a sibling of the
[`../wifi`](../wifi) suite.

Tests are written against the **standard**, not against INET's implementation. A test that
fails because INET is incomplete is a **finding**, not a suite defect — the suite doubles as
a conformance/gap report. Each test program asserts the spec-required behavior; the `.test`
wrapper records the *current* expected outcome so the suite is also a stable CI gate.

## Layout & how it works

```
ipv6/
  run-tests.sh        build + run everything via opp_test (-> one `ipv6tests` binary)
  Ipv6TestSupport.h   C++ predicates for parts of an ND message PacketFilter can't address
  ned/                Ipv6LanNetwork (advertising Router6 + 2 StandardHost6 on one switched LAN)
  ini/                _base.ini (SLAAC config shared by every test)
  nd/                 Neighbor Discovery tests (RFC 4861)
  dad/                DAD + SLAAC tests (RFC 4862)
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

### The one network

`Ipv6LanNetwork` is an advertising `Router6` and two `StandardHost6` hosts joined by an
`EthernetSwitch` into a single IPv6 link. From this one configuration the whole boot
sequence unfolds by itself and is what the suite observes: each node runs **link-local
DAD**, the hosts send **Router Solicitations** and the router replies with **Router
Advertisements**, the hosts form a **global address by SLAAC** and run **DAD** on it, and a
host→host flow (added per-test) triggers **address resolution** (NS/NA).

The router advertises the on-link prefix via its per-node `routes` XML (see the note under
*Findings*). Per-test knobs (DAD disabled, same-MAC collision, traffic) are layered in each
`.test`'s `%inifile` after `include ../../ini/_base.ini`.

### Observation model

Nodes are treated as **black boxes**: the suite observes the packets they actually put on and
take off the link — what the RFC specifies — not their internal module structure. Observation
is at the **network interface** (`X.eth[0].mac`):

- a node **sends** a packet on the wire → signal `packetSentToLower`
- a node **receives** a packet from the wire → signal `packetReceivedFromLower`

At the interface the full Ethernet+IPv6+ND frame is present on both sides, so ND *chunk* fields
(`targetAddress`, `solicitedFlag`, `managedAddrConfFlag`, …) **and** IPv6-header fields
(`ipv6.srcAddress` / `ipv6.destAddress` / `ipv6.hopLimit`) are all readable regardless of
direction.

The only exception is **DAD completion/failure**, which is an internal state transition with no
positive packet on the wire (success is the *absence* of a defence). Those few tests observe the
node's DAD state channel — the `startDad` / `dadCompleted` / `dadFailed` signals — subscribed at
the **host level** (`on("host1")`), still treating the host as a black box rather than reaching
into its ND submodule.

`Ipv6TestSupport.h` provides C++ predicates (`isDadProbe`, `nsTarget`, `isSolicitedNodeMulticast`,
`raHasAutonomousPrefix`, …) for what the PacketFilter string engine cannot express — notably the
`Ipv6Address`-typed fields and the TLV options nested inside a Router Advertisement.

## Outcome semantics

- **CONFORMS ✅** — INET produces the spec behavior; the program PASSes; `%contains` expects `PASS`.
- **NOT-MODELED ⛔** — INET does not implement the feature (or applies it inconsistently); the
  faithful spec assertion FAILs on its deadline; `%contains` expects `FAIL` (an *expected failure*).
- A red `opp_test` result therefore signals a **change**: a CONFORMS test regressed, or a
  NOT-MODELED feature started working (update the matrix).

**Today: 18 CONFORMS, 3 NOT-MODELED across 21 tests — aggregate PASS.**

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
| `Dad_Disabled` | DupAddrDetectTransmits=0 suppresses the link-local probe | O | ✅ |
| `Dad_DisabledGlobal` | …should suppress the global probe too (RFC 4862 5.1) | O | ⛔ still probes global |
| `Dad_GratuitousNa` | Unsolicited NA after DAD (RFC 4862 5.4.4) | O | ⛔ NAs only on request |

### nd — Neighbor Discovery core (RFC 4861)
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
| `Nd_AddressResolution` | NS → solicited NA resolves a neighbour | R | ✅ |
| `Nd_NaSolicitedFlag` | Solicited NA sets S=1, host clears R=0 | R | ✅ |
| `Nd_NsSolicitedNodeMulticast` | Resolution NS to the target's solicited-node mcast | R | ✅ |
| `Nd_RouterPreference` | RFC 4191 Default Router Preference | O | ⛔ not implemented |

## Findings & notes

- **RFC 4191 Default Router Preference not implemented** (`Nd_RouterPreference`): all routers
  are equivalent; the RA carries no preference (the reserved bits stay zero).
- **No gratuitous NA after DAD** (`Dad_GratuitousNa`): INET sends Neighbour Advertisements only
  in response to a Neighbour Solicitation; RFC 4862 5.4.4's optional unsolicited announcement is
  not sent (it would be a natural `Ipv6` module option, not a bug).
- **DAD-disable is applied inconsistently** (`Dad_DisabledGlobal`): `DupAddrDetectTransmits=0`
  suppresses DAD for the link-local address but the SLAAC global address is still probed once.
  RFC 4862 5.1 disables DAD for every configured address.
- **RA rate-limiting is not enforced** (bug): the `MIN_DELAY_BETWEEN_RAS` throttle between a
  solicited and the next unsolicited RA is commented out in `Ipv6NeighbourDiscovery.cc`. Not
  asserted here (it cannot be positively tested) — noted as a known issue.
- **Configurator does not advertise a prefix across a switch**: with a switched LAN,
  `Ipv6NetworkConfigurator` (even with `assignAddressesToHosts=false`) adds the on-link routes
  but does **not** install an `AdvPrefix` on the router interface, so its RAs carry no Prefix
  Information option and SLAAC never starts. (With *direct* host↔router links — as in
  `examples/ipv6/ipv6configurator` — it works.) The suite works around this by configuring the
  router's advertised prefix directly through its per-node `routes` XML in `ini/_base.ini`.
- **DAD for configurator/manually-assigned addresses is intentionally skipped**: the
  `Ipv6NetworkConfigurator` reproduces a converged, post-configuration snapshot, so it does not
  DAD statically-assigned addresses. This suite therefore exercises DAD on the *autoconfigured*
  (SLAAC) addresses, which is where INET performs it.

## Out of scope (this cut)

Neighbor Unreachability Detection (STALE→PROBE), ICMPv6 Redirect, MLD, and Mobile IPv6 (already
covered by the cookbook programs in [`../lib/ProtocolTests.cc`](../lib/ProtocolTests.cc)). The
suite is structured so these drop in later as new `.test` files without touching the shared
network/ini.

## Adding a test

Copy an existing `.test` (e.g. `nd/Nd_RouterSolicitation.test`). Put the program in
`%file: <Name>.cc` (unique basename) with a `Define_ProtocolTest(ipv6_<name>)`, select it via
`*.tester.testName`, `include ../../ini/_base.ini` (plus any inline overrides), and set
`%contains` to the expected verdict (`PASS` for CONFORMS, `FAIL` for a faithful NOT-MODELED
assertion). Observe sends at `X.eth[0].mac` `packetSentToLower`, receives at
`packetReceivedFromLower`, and DAD state via the host-level `startDad`/`dadCompleted`/`dadFailed`
signals (`on("X")`). To author against the real trace, add a tester with `logEvents=true` and
read `work/<Name>/test.out`.
