# ND — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/nd/checks.md), [features.md](../../protocol/nd/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow for the ND level 2 pass. This is the first
document of the pass that may name the simulation model and reference code, and it does both:
the verdicts of the run, the class of every failure, and where the model implements, or fails
to implement, each checked behavior.

The level 2 pass ran on 2026-09-24 at `8f78f1a73c` (src `5c4f41c600`): 37 tests, 22 PASS, 15
FAIL, none declared expected, and eleven gaps. The repairs of 2026-09-25, on
`topic/standards-tests-nd-level2-fixes`, repaired all eleven; the run below is theirs. Each gap
keeps its description of the code of the level 2 run, and says how it was repaired.

## Run record

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-nd-level2-fixes`, commit `dfc67c34f4`, tree clean
- Trees: src `425b3706d3`, tests/protocol `4eb4ce50f5`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/nd$'`
- Suite: 37 tests, 37 PASS, so the suite reports PASS

The `src/` tree of the level 2 run, `5c4f41c600`, is the tree of `origin/master` at
`7772a7e4ef`: the pass changed no source file. The repairs change `Ipv6`, `Ipv6InterfaceData`,
`Ipv6RoutingTable`, `Ipv6NeighbourDiscovery`, the ICMPv6 serializer and message definitions, and
`Ipv6NetworkConfigurator`.

## Verdicts

One row per test. The check sections are in [`checks/`](../../protocol/nd/checks); the
statements are those of the `Checks:` line of each section. Two checks have more than one test,
so that one failing field does not hide the verdict of another: the Router Advertisement fields
(three tests) and the Redirect fields (two tests).

| Test | Check | Verdict | Class |
| --- | --- | --- | --- |
| `Rfc4861RsHostComesUp` | [Router Solicitation of a host that comes up](../../protocol/nd/checks/router-discovery.md#router-solicitation-of-a-host-that-comes-up) | PASS | — |
| `Rfc4861RsNoRouter` | [Router Solicitations on a link without a router](../../protocol/nd/checks/router-discovery.md#router-solicitations-on-a-link-without-a-router) | PASS | — |
| `Rfc4861RaHeader` | [Router Advertisement header and addressing](../../protocol/nd/checks/router-discovery.md#router-advertisement-header-and-addressing) | PASS | — |
| `Rfc4861RaFields` | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | PASS | — |
| `Rfc4861RaMtuOption` | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | PASS | repaired, [gap 2](#gap-2-defect--two-defaults-of-the-router-are-wrong) |
| `Rfc4861RaCurHopLimit` | [Router Advertisement fields](../../protocol/nd/checks/router-discovery.md#router-advertisement-fields) | PASS | repaired, [gap 2](#gap-2-defect--two-defaults-of-the-router-are-wrong) |
| `Rfc4861RaPrefixInformation` | [Prefix Information option](../../protocol/nd/checks/router-discovery.md#prefix-information-option) | PASS | — |
| `Rfc4861RaSolicited` | [Router Advertisement in answer to a solicitation](../../protocol/nd/checks/router-discovery.md#router-advertisement-in-answer-to-a-solicitation) | PASS | repaired, [gap 5](#gap-5-defect--the-router-answers-only-its-first-solicitation) |
| `Rfc4861RaUnsolicited` | [Unsolicited Router Advertisements](../../protocol/nd/checks/router-discovery.md#unsolicited-router-advertisements) | PASS | repaired, [gap 6](#gap-6-defect--the-first-periodic-advertisement-comes-after-198-to-600-seconds) |
| `Rfc4861DefaultRouter` | [The default router](../../protocol/nd/checks/router-discovery.md#the-default-router) | PASS | — |
| `Rfc4861RouterLifetimeZero` | [A router with Router Lifetime zero](../../protocol/nd/checks/router-discovery.md#a-router-with-router-lifetime-zero) | PASS | repaired, [gap 7](#gap-7-defect--a-host-drops-every-advertisement-during-duplicate-address-detection) |
| `Rfc4861HopLimitFromRouter` | [Hop limit from the router](../../protocol/nd/checks/parameters.md#hop-limit-from-the-router) | PASS | repaired, [gap 1](#gap-1-defect--the-hop-limit-of-a-host-is-a-constant) |
| `Rfc4861HopLimitNoRouter` | [Hop limit without a router](../../protocol/nd/checks/parameters.md#hop-limit-without-a-router) | PASS | repaired, [gap 1](#gap-1-defect--the-hop-limit-of-a-host-is-a-constant) |
| `Rfc4861RetransTimerFromRouter` | [Retransmission timer from the router](../../protocol/nd/checks/parameters.md#retransmission-timer-from-the-router) | PASS | repaired, [gap 4](#gap-4-defect--address-resolution-waits-a-constant-not-retranstimer) |
| `Rfc4861MtuFromRouter` | [MTU from the router](../../protocol/nd/checks/parameters.md#mtu-from-the-router) | PASS | repaired, [gap 3](#gap-3-defect--the-host-fragments-to-the-mtu-of-the-interface-not-to-the-advertised-mtu) |
| `Rfc4861OnLinkNeighbor` | [On-link neighbor reached directly](../../protocol/nd/checks/on-link.md#on-link-neighbor-reached-directly) | PASS | — |
| `Rfc5942OnLinkFlagClear` | [Prefix with the on-link flag clear](../../protocol/nd/checks/on-link.md#prefix-with-the-on-link-flag-clear) | PASS | — |
| `Rfc5942NoRouterNoOnLink` | [No router and no on-link prefix](../../protocol/nd/checks/on-link.md#no-router-and-no-on-link-prefix) | PASS | — |
| `Rfc5942PrefixLifetime` | [Prefix after its valid lifetime](../../protocol/nd/checks/on-link.md#prefix-after-its-valid-lifetime) | PASS | — |
| `Rfc4861AddressResolutionHost` | [Address resolution of a host](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-host) | PASS | — |
| `Rfc4861NsNaFields` | [Neighbor Solicitation and Advertisement fields](../../protocol/nd/checks/address-resolution.md#neighbor-solicitation-and-advertisement-fields) | PASS | — |
| `Rfc4861AddressResolutionRouter` | [Address resolution of a router](../../protocol/nd/checks/address-resolution.md#address-resolution-of-a-router) | PASS | — |
| `Rfc4861AddressResolutionFailure` | [Address resolution failure](../../protocol/nd/checks/address-resolution.md#address-resolution-failure) | PASS | — |
| `Rfc4861RedirectFirstHop` | [Redirect from the first-hop router](../../protocol/nd/checks/redirect.md#redirect-from-the-first-hop-router) | PASS | — |
| `Rfc4861RedirectFields` | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | PASS | repaired, [gap 8](#gap-8-defect--the-redirect-carries-no-option) |
| `Rfc4861RedirectedHeader` | [Redirect fields](../../protocol/nd/checks/redirect.md#redirect-fields) | PASS | repaired, [gap 8](#gap-8-defect--the-redirect-carries-no-option) |
| `Rfc6980RedirectLargePacket` | [Redirect of a large packet](../../protocol/nd/checks/redirect.md#redirect-of-a-large-packet) | PASS | repaired, [gap 8](#gap-8-defect--the-redirect-carries-no-option) |
| `Rfc4861HostFollowsRedirect` | [Host follows a Redirect](../../protocol/nd/checks/redirect.md#host-follows-a-redirect) | PASS | — |
| `Rfc4861RedirectOnLink` | [Redirect to an on-link destination](../../protocol/nd/checks/redirect.md#redirect-to-an-on-link-destination) | PASS | — |
| `Rfc4862LinkLocalDad` | [Link-local address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#link-local-address-and-its-duplicate-address-detection) | PASS | — |
| `Rfc4862RouterDad` | [Duplicate Address Detection of a router](../../protocol/nd/checks/autoconfiguration.md#duplicate-address-detection-of-a-router) | PASS | repaired, [gap 10](#gap-10-defect--a-router-does-not-test-its-configured-address) |
| `Rfc4862GlobalAddressDad` | [Global address and its Duplicate Address Detection](../../protocol/nd/checks/autoconfiguration.md#global-address-and-its-duplicate-address-detection) | PASS | — |
| `Rfc4862DuplicateLinkLocal` | [Duplicate link-local address](../../protocol/nd/checks/autoconfiguration.md#duplicate-link-local-address) | PASS | — |
| `Rfc4862AutonomousFlagClear` | [Prefix with the Autonomous flag clear](../../protocol/nd/checks/autoconfiguration.md#prefix-with-the-autonomous-flag-clear) | PASS | — |
| `Rfc4862AddressLifetime` | [Address after its valid lifetime](../../protocol/nd/checks/autoconfiguration.md#address-after-its-valid-lifetime) | PASS | repaired, [gap 11](#gap-11-defect--an-expired-address-stays-in-use) |
| `Rfc4862GroupsBeforeDad` | [Groups joined before Duplicate Address Detection](../../protocol/nd/checks/multicast.md#groups-joined-before-duplicate-address-detection) | PASS | repaired, [gap 9](#gap-9-defect--a-node-does-not-join-its-solicited-node-groups) |
| `Rfc4861AllRoutersGroup` | [All-routers group of a router](../../protocol/nd/checks/multicast.md#all-routers-group-of-a-router) | PASS | — |

All 37 tests pass. In the level 2 run, router discovery passed 6 of its 11 tests, the
parameters from the router 0 of 4, on-link determination 4 of 4, address resolution 4 of 4,
Redirect 3 of 6, address autoconfiguration 4 of 6, and the multicast groups 1 of 2. Every
failure was a defect, and the eleven gaps below hold them all.

## The class of every failure

**After the repairs of 2026-09-25** no failure is left. Three test errors of this suite showed
while the gaps were repaired, and each has a commit of its own before the repair that made it
visible (see the gaps 6, 7 and 8 below, and the plan). The rest of this section classes the failures of the level 2
run.

All fifteen failures were of the class **defect** of
[the guide](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected):
the model claims the behavior, has code for it, and gets it wrong. None was declared expected,
because no limitation blocked a repair; each gap names the code that a repair would change. No
failure of that run was a test error or a misread of the specification. The earlier runs of the
pass found three errors of the mockups and fixed them before this run; the plan records them.

The shapes of the defects: a default of the wrong value (gaps 1 and 2), a value that the model
stores and then does not use (gaps 3 and 4), a timer with the wrong law (gaps 5 and 6), a message
that the model drops (gap 7), a message without its options (gap 8), and a mechanism that one
kind of address or group never reaches (gaps 9, 10 and 11).

## The model gaps

### Gap 1 (defect) — the hop limit of a host is a constant

- **Tests**: `Rfc4861HopLimitFromRouter` and `Rfc4861HopLimitNoRouter`, observation 2. Both
  echo requests leave with hop limit 30.
- **Statements**: RFC4861-HOST-16, RA-9 (the value of the router); RFC4861-HOST-1, HOST-4 (the
  default, 64).
- **The code**: `Ipv6::encapsulate` sets `ttl != -1 ? ttl : IPv6_DEFAULT_ADVCURHOPLIMIT`
  ([`Ipv6.cc:964`](../../../../../src/inet/networklayer/ipv6/Ipv6.cc)), and the constant is 30
  ([`Ipv6InterfaceData.h:31`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h)).
  The host copies Cur Hop Limit into its CurHopLimit variable
  ([`Ipv6NeighbourDiscovery.cc:1501`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)),
  but no code reads that variable when it sends.
- **Scope**: every packet of every node that the application does not give a hop limit.
- **Repaired** on 2026-09-25 (`922926f0b1`): a datagram without a hop limit from its transport
  takes the CurHopLimit of its outgoing interface, which `Ipv6::fragmentPostRouting` writes when
  routing has chosen the interface; the default of CurHopLimit is 64.

### Gap 2 (defect) — two defaults of the router are wrong

- **Tests**: `Rfc4861RaCurHopLimit`, observation 6 (Cur Hop Limit 30); `Rfc4861RaMtuOption`,
  observation 4 (an MTU option of 1280 on a link with MTU 1500).
- **Statements**: RFC4861-ADV-7, RCFG-12 (AdvCurHopLimit defaults to the value of the Assigned
  Numbers registry, 64); RFC4861-ADV-11, RCFG-9 (AdvLinkMTU defaults to zero, which sends no MTU
  option), and with them the MTU value of OPT-45 to OPT-47.
- **The code**: the parameters of the routing table, `advCurHopLimit = default(30)` and
  `advLinkMtu = default(1280)`
  ([`Ipv6RoutingTable.ned:101-102`](../../../../../src/inet/networklayer/ipv6/Ipv6RoutingTable.ned)).
- **Scope**: every router that keeps the defaults. With gap 3 the wrong MTU option has no effect
  on the hosts; after a repair of gap 3 alone, every host would send at most 1280 octets.
- **Repaired** on 2026-09-25 (`35e897ba39`): the defaults are 64 and 0, and an advertisement
  carries an MTU option only for a nonzero AdvLinkMTU.

### Gap 3 (defect) — the host fragments to the MTU of the interface, not to the advertised MTU

- **Test**: `Rfc4861MtuFromRouter`, observation 2: the echo request leaves whole, 1500 octets,
  after an advertisement with MTU 1400.
- **Statements**: RFC4861-HOST-24, HOST-3, OPT-49.
- **The code**: the host copies the MTU option into LinkMTU
  ([`Ipv6NeighbourDiscovery.cc:1536`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)).
  `Ipv6::fragmentPostRouting` fragments to `ie->getMtu()`, the MTU of the interface
  ([`Ipv6.cc:1059`](../../../../../src/inet/networklayer/ipv6/Ipv6.cc)), and no code reads
  LinkMTU.
- **Scope**: every link whose router advertises an MTU below the MTU of the interface.
- **Repaired** on 2026-09-25 (`4d7f3bc361`): `Ipv6::fragmentAndSend` uses the LinkMTU of the
  interface when it is smaller than the MTU of the interface. The default of LinkMTU was 1280;
  it is now the MTU of the interface, and a host takes an MTU option only up to that value.

### Gap 4 (defect) — address resolution waits a constant, not RetransTimer

- **Test**: `Rfc4861RetransTimerFromRouter`, observation 3: the solicitations come 1 s apart
  after an advertisement with Retrans Timer 2000.
- **Statement**: RFC4861-HOST-19.
- **The code**: address resolution schedules its retransmissions with `_getRetransTimer()`
  ([`Ipv6NeighbourDiscovery.cc:695, 713`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)),
  the node constant RETRANS_TIMER
  ([`Ipv6InterfaceData.cc:193`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc)).
  The host copies Retrans Timer into another variable
  ([`Ipv6NeighbourDiscovery.cc:1525`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)),
  which Duplicate Address Detection reads as seconds
  ([`Ipv6NeighbourDiscovery.cc:851, 872`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)).
- **Scope**: every retransmission of address resolution. The unit error of Duplicate Address
  Detection has no test: a router that advertised Retrans Timer 1000 milliseconds would make its
  hosts wait 1000 s. See [Other findings](#other-findings).
- **Repaired** on 2026-09-25 (`28dfed59ef`): the RetransTimer variable is a time, set from
  milliseconds by an advertisement; address resolution, the probes and Duplicate Address
  Detection read it. The default of AdvRetransTimer is 0, the default of RFC 4861.

### Gap 5 (defect) — the router answers only its first solicitation

- **Test**: `Rfc4861RaSolicited`, observation 2: host B solicits at 12.7, 16.7 and 20.7 s, and R
  answers none. Observation 3 has no verdict, because no answer comes.
- **Statements**: RFC4861-ADV-29, RS-1, ADV-32, ADV-34, ADV-35.
- **The code**: `processRsPacket` schedules an answer only when it comes before
  `nextScheduledRATime`
  ([`Ipv6NeighbourDiscovery.cc:1210`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)),
  and sets that time to the time of the answer. `sendSolicitedRa`
  ([`Ipv6NeighbourDiscovery.cc:1723`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc))
  neither moves the time forward nor restarts the timer of the interface. After the first
  answer the time stays in the past, and the router drops every later answer until its first
  periodic advertisement, 198 to 600 s after the start (gap 6).
- **Scope**: every host that comes up after the first solicitation of a link, for up to ten
  minutes. With gap 7, such a host has no default router in that time.
- **Repaired** on 2026-09-25 (`312e9e60c5`): after an answer, the time of the next advertisement
  is the time of the periodic timer again; the repair of gap 6 then restarts that timer.

### Gap 6 (defect) — the first periodic advertisement comes after 198 to 600 seconds

- **Test**: `Rfc4861RaUnsolicited`, observation 2: after the answer to A at 2.4 s, the next
  multicast advertisement does not come within 16 s. Observation 3 has no verdict.
- **Statements**: RFC4861-ADV-23; the intervals of ADV-22, ADV-31, ADV-36, RCFG-5 and RCFG-6
  have no verdict.
- **The code**: `createRaTimer` sets the first timer to a random value between
  MinRtrAdvInterval and MaxRtrAdvInterval
  ([`Ipv6NeighbourDiscovery.cc:1639`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)).
  `sendPeriodicRa` applies the cap of MAX_INITIAL_RTR_ADVERT_INTERVAL only to the timers after
  that one, and an answer to a solicitation does not restart the timer (RFC4861-ADV-31).
- **Scope**: every advertising interface. In the first minutes of a run, the only
  advertisements are the answers that gap 5 lets through.
- **Repaired** on 2026-09-25 (`3616c3db36`): the timers before the first three advertisements
  are at most 16 s, a multicast answer restarts the timer, and the timer starts when the router
  has booted, so that routers that start together do not transmit in step; the first version
  started it at t = 0, and twenty GPSR routers then sent at exactly 16 s, which the radio medium
  refuses. A test error showed at the bound: the deadline of `within(16.0)` fired before an
  advertisement at exactly 16 s (`ab54e3b914`).

### Gap 7 (defect) — a host drops every advertisement during Duplicate Address Detection

- **Test**: `Rfc4861RouterLifetimeZero`, observation 2: host A sends no echo request at all.
  The rule of the check, RFC4861-RA-17, has no verdict.
- **Statement**: RFC4861-HOST-12.
- **The code**: `processRaPacket` drops an advertisement while the interface runs Duplicate
  Address Detection
  ([`Ipv6NeighbourDiscovery.cc:1380`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)).
  The advertisement of R2 starts the detection of the global address of A, and the
  advertisement of R1 comes 45 ms later and is dropped. Gaps 5 and 6 keep R1 from sending
  another one in the window, so A has no default router. The host also stops its solicitations
  after any valid advertisement
  ([`Ipv6NeighbourDiscovery.cc:1388`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)),
  also one with Router Lifetime zero, which RFC4861-HOST-50 does not ask for; no check of this
  pass observes that on its own.
- **Scope**: every link with more than one router. The Redirect checks start R2 later, so that
  this gap does not hide the rules they check.
- **Repaired** on 2026-09-25 (`3690f26316`): a host updates its Default Router List and the
  parameters of the link from every valid advertisement; only the prefixes wait for an
  advertisement after the detection. The new timing of gap 6 had made the test pass with the
  defect in the code; a delay of 250 ms on the link of R1 now puts its answer into the detection
  of A (`c3d138a1bc`).

### Gap 8 (defect) — the Redirect carries no option

- **Tests**: `Rfc4861RedirectFields`, observation 2; `Rfc4861RedirectedHeader`, observation 3;
  `Rfc6980RedirectLargePacket`, observation 4. Every Redirect is 80 octets.
- **Statements**: RFC4861-RDM-15, RDR-7, OPT-12 (the Target Link-Layer Address option);
  OPT-38 to OPT-40, RDM-17, RDR-8 (the Redirected Header option).
- **The code**: `sendRedirect` builds the fixed part only, `redirect->setChunkLength(B(40))`,
  and adds no option
  ([`Ipv6NeighbourDiscovery.cc:2379`](../../../../../src/inet/networklayer/icmpv6/Ipv6NeighbourDiscovery.cc)).
  The model has no class for the Redirected Header option, and its serializer has no case for
  it ([`Icmpv6HeaderSerializer.cc`](../../../../../src/inet/networklayer/icmpv6/Icmpv6HeaderSerializer.cc)).
- **Scope**: every Redirect. A host of the model still follows it: `Rfc4861HostFollowsRedirect`
  passes.
- **Repaired** on 2026-09-25 (`5d7c5069ff`): the Redirect carries the Target Link-Layer Address
  option when the router knows the target, and the new `Ipv6NdRedirectedHeader` option with as
  much of the invoking packet as fits into 1280 octets. A test error showed: a router builds the
  Redirect before it resolves the next hop, so R1 did not know R2 for the first packet; R1 now
  pings R2 first (`99359634b6`).

### Gap 9 (defect) — a node does not join its solicited-node groups

- **Test**: `Rfc4862GroupsBeforeDad`, observation 2: host A sends no MLD Report for
  ff02::1:ff00:a, before or after its solicitation.
- **Statements**: RFC4862-DAD-11, RFC4861-AR-4, AR-6.
- **The code**: a node accepts a packet to a solicited-node address by matching the address
  against its own addresses
  ([`Ipv6InterfaceData.cc:375`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc)),
  and never joins the group with `joinMulticastGroup`, so MLD never reports it. The router
  joins ff02::2 with that function, and MLD reports that group
  (`Rfc4861AllRoutersGroup` passes).
- **Scope**: every solicited-node group of every node. A switch that snoops MLD would drop the
  solicitations of address resolution and of Duplicate Address Detection.
- **Repaired** on 2026-09-25 (`affa7e64fa`): an interface joins the solicited-node group of each
  unicast address when it gets the address, and leaves it with the last address of the group.

### Gap 10 (defect) — a router does not test its configured address

- **Test**: `Rfc4862RouterDad`, observation 2: R tests its link-local address, but never its
  global address 2001:db8:1::1.
- **Statement**: RFC4862-DAD-1.
- **The code**: the configurator assigns the address as not tentative,
  `assignAddress(interfaceInfo->globalAddress, false, ...)`
  ([`Ipv6NetworkConfigurator.cc:172`](../../../../../src/inet/networklayer/configurator/ipv6/Ipv6NetworkConfigurator.cc)).
- **Scope**: every address that the configurator gives, to a router or to a host.
- **Repaired** on 2026-09-25 (`91a91e1505`): the configurator assigns a tentative address when
  the interface does the detection, and Neighbor Discovery tests every tentative address when the
  node has booted.

### Gap 11 (defect) — an expired address stays in use

- **Test**: `Rfc4862AddressLifetime`, observation 2: from 44 s on, host A sends its solicitations
  for R from 2001:db8:1::800:ff:fe00:a, whose valid lifetime ended at 32.4 s.
- **Statements**: RFC4862-GLOB-22, GLOB-23.
- **The code**: `choosePreferredAddress` removes expired addresses
  ([`Ipv6InterfaceData.cc:446`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.cc)),
  but runs only when an address is added or removed; `getPreferredAddress` returns the cached
  address with the comment "FIXME TODO check expiry time!"
  ([`Ipv6InterfaceData.h:559`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h)).
- **Scope**: every address with a finite lifetime. The prefix itself expires correctly: the
  twin check of the prefix, `Rfc5942PrefixLifetime`, passes.
- **Repaired** on 2026-09-25 (`be7b698a38`): a timer of Neighbor Discovery removes an address
  when its valid lifetime ends.

## What the model does well

- **Since the repairs of 2026-09-25**, the parameters that a host takes from its router, the
  defaults of the router, the timing of the advertisements, the Redirect with its options, the
  solicited-node groups, the detection of configured addresses and the end of an address follow
  the documents too; the eleven gaps below describe the model before the repairs.
- **Router Solicitations** leave to ff02::2 from the link-local address with the link-layer
  address of the host, stop after the first advertisement, and on a link without a router stop
  after three, 4 s apart.
- **The Router Advertisement** leaves from the link-local address with hop limit 255, and its
  fields carry the router variables: the flags, Router Lifetime 1800, the two timers as
  configured, the Source Link-Layer Address option, and a Prefix Information option with the
  right flags, lifetimes and zero bits. No advertisement leaves a host.
- **On-link determination follows RFC 5942**: a prefix with the L flag clear leaves its
  addresses off-link, a host without a router and without an on-link prefix sends nothing to a
  global destination, and an expired prefix is off-link again.
- **Address resolution** follows §7.2 for hosts and routers: the solicitation to the
  solicited-node address with the Source Link-Layer Address option, the advertisement with the
  right target, Target Link-Layer Address option and R, S and O flags, the queued packet sent
  after the answer, three solicitations 1 s apart and then a Destination Unreachable with code
  3 for the queued packet.
- **The Redirect is sent and followed**: from the link-local address of the first-hop router,
  to the source of the packet, with the link-local address of the better router or the
  destination itself as its target; the host sends the next packets to the target.
- **Duplicate Address Detection** of a host sends one solicitation from the unspecified address
  without an option, keeps the address tentative for 1 s, finds a duplicate link-local address
  from the answer of the other node, and then stops IP on the interface. The address from a
  prefix with the A flag clear is not formed.

## Other findings

- **The advertising timers of routers that start together ran in step**, found by the repair
  of gap 6: a capped interval is exactly 16 s, and every router started its timer at t = 0.
  The timer now starts at the random boot time of the router.
- **A router builds the Redirect before it resolves the next hop** (`Ipv6::routePacket`, as
  `ip6_forward` of Linux), so the Redirect for the first packet to an unresolved next hop has no
  Target Link-Layer Address option. RFC 4861 section 4.5 includes the option "if known", so this
  follows the text.
- **The configurator gives the hosts routes.** With its defaults, `Ipv6NetworkConfigurator`
  adds a default route to every host with one interface
  ([`Ipv6NetworkConfigurator.cc:928-960`](../../../../../src/inet/networklayer/configurator/ipv6/Ipv6NetworkConfigurator.cc))
  and on-link routes towards the routers, also when `assignAddressesToHosts` is false. A host
  then reaches its router without Neighbor Discovery, and gaps 5, 6 and 7 do not show. The
  mockups of this pass set `addStaticRoutes = false` and give the routers their routes in
  `<route>` elements.
- **Two router defaults are values in seconds in fields of milliseconds.** The repair of gap 4
  set the default of AdvRetransTimer to 0; AdvReachableTime still defaults to 3600, and a host
  still reads the Reachable Time field as seconds. Before the repairs, AdvReachableTime and
  AdvRetransTimer defaulted to 3600 and 1, "seconds" in their comments
  ([`Ipv6InterfaceData.h:35-36`](../../../../../src/inet/networklayer/ipv6/Ipv6InterfaceData.h)),
  and the advertisement carries them in fields of milliseconds; RFC4861-RCFG-10 and RCFG-11 make
  zero the default. The two variables have no parameter, which RFC4861-RCFG-1 asks for, so the
  tests set them with a management module of their own. No test of this pass checks the two
  defaults.
- **MLD is off by default** in the node, `hasMld = false`
  ([`Ipv6NetworkLayer.ned:72`](../../../../../src/inet/networklayer/ipv6/Ipv6NetworkLayer.ned));
  the two multicast tests turn it on. With MLD on, the router sends its report from its global
  address, where MLD asks for a link-local source; that rule is outside the in-scope set.
- **A Router Solicitation of a host that comes up late carries no Source Link-Layer Address
  option** in a trace of the run of `Rfc4861RaSolicited` (host B, from its link-local address),
  which RFC4861-RS-12 asks for. The check observes host A only, whose solicitation carries it.

## The five facts of the level 1 look

The level 1 pass named five facts for this pass to check
([`conformance.md`](conformance.md#part-1--the-claims)):

1. **The two method stubs** of address resolution, `processArTimeout` and
   `dropQueuedPacketsAwaitingAr`, are not on the path of the failure check:
   `Rfc4861AddressResolutionFailure` passes, with the Destination Unreachable for the queued
   packet.
2. **Anycast and more than one prefix** have no check of this pass; the closing list of
   [`checks.md`](../../protocol/nd/checks.md#statements-this-pass-wrote-no-check-for) keeps them
   for a later one.
3. **RFC 7527** is out of the in-scope set, and no check reaches it.
4. **The four bare TODOs** of Duplicate Address Detection and the advertisement: gap 7 is on the
   path of the TODO at `Ipv6NeighbourDiscovery.cc:1382` of the level 2 run, "improve this
   procedure in order to allow reinitiating DAD". The TODO stays: the repair of gap 7 processes
   the router part of the advertisement, and the prefixes still wait.
5. **The on-link logic** follows RFC 5942 in all four checks of on-link determination, which
   pass.

## What the next pass owes

In the order I would do it:

1. **Level 3**, the crafted messages: the validation of every received message, the receiver
   halves of the reserved fields and the options, the fragmented ND messages of RFC 6980. The
   model has the validation code (`validateRaPacket` and the functions beside it), so the ledger
   records them as `owed`.
2. **The unit of AdvReachableTime and of the Reachable Time field**: the same repair as gap 4,
   with a check of the reachable time that a host takes from its router.
3. **The level 2 statements this pass left**: the closing list of
   [`checks.md`](../../protocol/nd/checks.md#statements-this-pass-wrote-no-check-for) names what
   each needs, from a router variable of zero to three routers on one link.
