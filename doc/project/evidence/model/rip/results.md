# RIP — run results and model analysis

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../../protocol/rip/checks.md), [features.md](../../protocol/rip/features.md), [coverage.md](coverage.md)

Step 7 artifact of the standards test workflow for the RIP level 2 pass. This is the first
document of the pass that may name the simulation model and reference code, and it does both:
the verdicts of the run, the class of every failure, and where the model implements, or fails
to implement, each checked behavior.

## Run record

- Date: 2026-09-29 17:29 +0200
- INET: branch `master`, commit `24675c3a37`, tree clean
- Trees: src `8b4f86968e`, tests/protocol `1f1d62beca`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/rip$'`
- Suite: 30 tests, 20 PASS, 10 FAIL (unexpected), 0 FAIL (expected), so the suite reports FAIL

The level 2 pass ran on 2026-09-24 at src `5c4f41c600`, the tree of `origin/master` at `7772a7e4ef`,
and changed no source file. The branch landed on `master` by a rebase onto `49e1fa0945`, whose
`src/` differs from that tree in one line of `src/inet/common/InitStages.cc`. So the suite ran again
on `master`, and the verdicts and the failure reasons are those of the level 2 run.

## Verdicts

One row per test. The check sections are in [`checks/`](../../protocol/rip/checks); the
statements are those of the `Checks:` line of each section.

| Test | Check | Verdict | Class |
| --- | --- | --- | --- |
| `Rfc2453UpdateTransport` | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | PASS | — |
| `Rfc2453UpdateFields` | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | PASS | — |
| `Rfc2080UpdateTransport` | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | PASS | — |
| `Rfc2080UpdateFields` | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | FAIL at observation 6 | defect, [gap 1](#gap-1-defect--ripng-messages-carry-version-2) |
| `Rfc2453PeriodicUpdate` | [Periodic update interval (RIP version 2)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-rip-version-2) | PASS | — |
| `Rfc2080PeriodicUpdate` | [Periodic update interval (RIPng)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-ripng) | PASS | — |
| `Rfc2453MetricChain` | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | PASS | — |
| `Rfc2080MetricChain` | [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | PASS | — |
| `Rfc2453ShorterPath` | [Shorter path kept (RIP version 2)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-rip-version-2) | PASS | — |
| `Rfc2080ShorterPath` | [Shorter path kept (RIPng)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-ripng) | PASS | — |
| `Rfc2453SplitHorizon` | [Split horizon on the link of a learned route (RIP version 2)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | PASS | — |
| `Rfc2080SplitHorizon` | [Split horizon on the link of a learned route (RIPng)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-ripng) | PASS | — |
| `Rfc2453TableRequest` | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | PASS | — |
| `Rfc2080TableRequest` | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | FAIL at observation 4 | defect, [gap 2](#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface) |
| `Rfc2453LostNetwork` | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | PASS | — |
| `Rfc2080LostNetwork` | [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | FAIL at observation 5 | defect, [gap 1](#gap-1-defect--ripng-messages-carry-version-2) |
| `Rfc2453TriggeredRate` | [Rate of triggered updates (RIP version 2)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-rip-version-2) | PASS | — |
| `Rfc2080TriggeredRate` | [Rate of triggered updates (RIPng)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-ripng) | PASS | — |
| `Rfc2453RouteExpiry` | [Route expiry after a silent neighbor (RIP version 2)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-rip-version-2) | FAIL at observation 4 | defect, [gap 4](#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent) |
| `Rfc2080RouteExpiry` | [Route expiry after a silent neighbor (RIPng)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-ripng) | FAIL at observation 4 | defect, [gap 4](#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent) |
| `Rfc2453ExpiryGarbageCollection` | [Garbage collection after an expiry (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-rip-version-2) | PASS | — |
| `Rfc2080ExpiryGarbageCollection` | [Garbage collection after an expiry (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-ripng) | PASS | — |
| `Rfc2453LostNetworkGarbageCollection` | [Garbage collection of a lost network (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-rip-version-2) | FAIL at observation 4 | defect, [gap 5](#gap-5-defect--a-router-never-removes-a-network-it-lost) |
| `Rfc2080LostNetworkGarbageCollection` | [Garbage collection of a lost network (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-ripng) | FAIL at observation 4 | defect, [gap 5](#gap-5-defect--a-router-never-removes-a-network-it-lost) |
| `Rfc2453WithdrawnRouteGarbageCollection` | [Garbage collection while the next hop still withdraws (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-rip-version-2) | FAIL at observation 4 | defect, [gap 6](#gap-6-defect--a-withdrawn-route-stays-while-its-next-hop-repeats-the-withdrawal) |
| `Rfc2080WithdrawnRouteGarbageCollection` | [Garbage collection while the next hop still withdraws (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-ripng) | FAIL at observation 4 | defect, [gap 6](#gap-6-defect--a-withdrawn-route-stays-while-its-next-hop-repeats-the-withdrawal) |
| `Rfc2453GarbageCollection` | [Garbage collection ended by a new route (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-ended-by-a-new-route-rip-version-2) | PASS | — |
| `Rfc2080GarbageCollection` | [Garbage collection ended by a new route (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-ended-by-a-new-route-ripng) | FAIL at observation 3 | defect, [gap 3](#gap-3-defect--an-ipv6-router-does-not-advertise-its-network-again-after-the-link-returns) |
| `Rfc2453ManyRoutes` | [More than 25 routes (RIP version 2)](../../protocol/rip/checks/response-contents.md#more-than-25-routes-rip-version-2) | PASS | — |
| `Rfc2080MtuMessages` | [Messages limited by the MTU (RIPng)](../../protocol/rip/checks/response-contents.md#messages-limited-by-the-mtu-ripng) | PASS | — |

RIP version 2 passes 12 of its 15 tests. RIPng passes 8 of 15. Every failure is a defect, and
the six gaps below hold them all. Three of the gaps belong to RIPng alone, three to both.

## The class of every failure

All ten failures are of the class **defect** of
[the guide](../../../guide/derive-tests-from-a-standard.md#the-class-of-a-failure-and-when-to-declare-it-expected):
the model claims the behavior, has the code for it, and gets it wrong. None is declared
expected, because no limitation blocks a repair; each gap names the code that a repair would
change. No failure is a test error or a misread of the specification. The earlier runs of the
pass found such errors and fixed them before this run; the plan records them.

The shapes of the defects are the four that the guide lists: a field that is set to a wrong
value (gap 1), a complete function that sends its result the wrong way (gap 2), a mechanism of
another module that the protocol depends on and that does not do its half (gap 3), and a
mechanism present with the wrong timing or the wrong law (gaps 4, 5, 6).

## The model gaps

### Gap 1 (defect) — RIPng messages carry version 2

- **Tests**: `Rfc2080UpdateFields`, observation 6; `Rfc2080LostNetwork`, observation 5.
- **Statements**: RFC2080-GEN-3 ("The version described in this document is version 1"),
  RFC2080-TRIG-8.
- **The code**: `RipPacket` declares `short version @bit(8) = 2`
  ([`RipPacket.msg:60`](../../../../../src/inet/routing/rip/RipPacket.msg)), and no code in
  [`Rip.cc`](../../../../../src/inet/routing/rip/Rip.cc) sets the version. The default of RIP
  version 2 goes out in RIPng mode, in every request and every response.
- **Scope**: every RIPng message. The observations before the version in both tests hold,
  which is why each test puts the version last.

### Gap 2 (defect) — the RIPng answer to a request leaves on the wrong interface

- **Test**: `Rfc2080TableRequest`, observation 4, and with it observations 5 to 7, which read
  the answer.
- **Statements**: RFC2080-OUT-1, MSG-4, REQ-4 (the answer half), REQ-6; covers GEN-1 and
  GEN-3.
- **The code**: `Rip::processRequest` answers a whole-table request with `sendRoutes` to the
  address and port of the requester
  ([`Rip.cc:498`](../../../../../src/inet/routing/rip/Rip.cc)). `Rip::sendPacket` attaches the
  outgoing interface, and for RIPng the link-local source address, only when the destination
  is a multicast group ([`Rip.cc:982-992`](../../../../../src/inet/routing/rip/Rip.cc)). The
  requester of RIPng is a link-local address, which is ambiguous without its interface. The
  IPv6 layer sends the answer out of the first interface, `eth0` on netA; neighbour discovery
  for R2 fails there after three solicitations, and the answer is dropped with an ICMPv6
  destination unreachable. The run shows it at 127.8 s to 130.8 s.
- **Scope**: every RIPng answer to a request, whole-table or specific. The RIP version 2 twin
  passes, because the IPv4 address of R2 routes through L1.

### Gap 3 (defect) — an IPv6 router does not advertise its network again after the link returns

- **Test**: `Rfc2080GarbageCollection`, observation 3. The check never reaches observation 4,
  the rule RFC2080-TIMER-7 that it exists for.
- **Statement reached**: none of its own; RFC2080-TIMER-7 is not reached. The gap breaks the
  premise of RFC 2080 §2 that the router sets up the entries of its directly-connected
  networks.
- **The code**: the IPv6 routing table deletes the routes of an interface when its carrier
  goes down, and adds nothing back when the carrier returns
  ([`Ipv6RoutingTable.cc:182-190`](../../../../../src/inet/networklayer/ipv6/Ipv6RoutingTable.cc)).
  RIP adds an interface route again only on the `routeAdded` signal
  ([`Rip.cc:357-371`](../../../../../src/inet/routing/rip/Rip.cc)), which never comes. The
  IPv4 routing table restores the routes in `Ipv4RoutingTable::updateNetmaskRoutes`, which is
  why the IPv4 twin passes.
- **Scope**: every IPv6 network of a RIPng router whose link goes down and comes back. The
  repair belongs to the IPv6 routing table, not to RIP, but the defect is a defect of the
  router that the check models.

### Gap 4 (defect) — the timeout is looked at only when an update is sent

- **Tests**: `Rfc2453RouteExpiry` and `Rfc2080RouteExpiry`, observation 4. In both runs R2
  withdraws netA 195 s after the last update of R1, 15 s late.
- **Statements**: RFC2453-TIMER-4, TIMER-5; RFC2080-TIMER-4, TIMER-5.
- **The code**: `Rip::checkExpiredRoutes`, which compares the age of each route with
  `routeExpiryTime`, runs only from `Rip::sendRoutes`
  ([`Rip.cc:537`, `Rip.cc:551`](../../../../../src/inet/routing/rip/Rip.cc)); no timer of its
  own fires at the expiry. The router notices the expiry at its next update, up to one update
  interval late.
- **Scope**: every expiry. An earlier run with the default random start passed by chance: R2
  started 1.3 s after R1, so its next periodic update came 1.3 s after the expiry. The check
  now starts R2 15 s after R1, and the note of the check says why.

### Gap 5 (defect) — a router never removes a network it lost

- **Tests**: `Rfc2453LostNetworkGarbageCollection` and `Rfc2080LostNetworkGarbageCollection`,
  observation 4.
- **Statements**: RFC2453-TIMER-5 (the 120 seconds), TIMER-6; RFC2080-TIMER-5, TIMER-6.
- **The code**: `Rip::checkExpiredRoutes` purges only routes of type `RIP_ROUTE_RTE`, that is
  learned routes ([`Rip.cc:909-918`](../../../../../src/inet/routing/rip/Rip.cc)). The route to
  a network of the router itself has the type `RIP_ROUTE_INTERFACE`; after
  `Rip::invalidateRoute` ([`Rip.cc:936-948`](../../../../../src/inet/routing/rip/Rip.cc)) it
  stays in the table at metric 16, and every update holds it without end.
- **Scope**: every network that a router loses, so every withdrawal lives on in the whole RIP
  domain, because gap 6 keeps the neighbors from removing it either.

### Gap 6 (defect) — a withdrawn route stays while its next hop repeats the withdrawal

- **Tests**: `Rfc2453WithdrawnRouteGarbageCollection` and
  `Rfc2080WithdrawnRouteGarbageCollection`, observation 4.
- **Statements**: RFC2453-RESP-10, TIMER-6; RFC2080-RESP-12, TIMER-6.
- **The code**: `Rip::processResponse` refreshes the time of the last update of a route with
  every entry from its next hop, metric 16 included
  ([`Rip.cc:688-689`](../../../../../src/inet/routing/rip/Rip.cc)), and `Rip::checkExpiredRoutes`
  purges a route at that time plus the timeout plus the garbage-collection time
  ([`Rip.cc:909-918`](../../../../../src/inet/routing/rip/Rip.cc)). Two errors combine: each
  repeated withdrawal restarts the garbage collection, which RFC2453-RESP-10 forbids, and even
  a single withdrawal keeps the route for 300 s instead of 120 s.
- **Scope**: every route that its next hop withdraws. After a timeout the same arithmetic is
  right by accident — the last update plus 180 s plus 120 s is the expiry plus 120 s — which
  is why `Rfc2453ExpiryGarbageCollection` and its RIPng twin pass.

## What the model does well

- **The update message and its transport** follow both documents: ports 520 and 521, the
  multicast groups, command 2, zero octets, address family 2, the mask or prefix length of
  each entry, and next hop 0.0.0.0 (the model always sends an unspecified next hop,
  [`Rip.cc:586`](../../../../../src/inet/routing/rip/Rip.cc)). A RIPng update leaves from the
  link-local address of its interface with hop limit 255
  ([`Rip.cc:985-989`](../../../../../src/inet/routing/rip/Rip.cc)).
- **The periodic update** comes every 30 s exactly
  ([`Rip.cc:136`](../../../../../src/inet/routing/rip/Rip.cc)): the first precaution of §3.8,
  a clock that load does not affect. The run measured 30.000 s between five consecutive
  updates.
- **Route learning** follows §3.9.2: the metric grows by the cost at each hop, the shorter of
  two paths wins and stays, and host A reaches host C through the chain on both address
  families.
- **Split horizon** is split horizon with poisoned reverse by default (the `mode` attribute of
  `ripConfig`, [`Rip.ned`](../../../../../src/inet/routing/rip/Rip.ned)), the form that RFC 2453
  recommends and RFC 2080 prefers. The model applies it to the network of the outgoing link
  too, which it advertises with metric 16 on that link; the checks do not ask about that
  network.
- **Triggered updates** leave 1 to 5 s after the change (the parameter
  `triggeredUpdateDelay`, drawn in `Rip::triggerUpdate`,
  [`Rip.cc:889-899`](../../../../../src/inet/routing/rip/Rip.cc)), hold only the changed
  routes, and collect several changes into one message: the three withdrawals of the rate
  check left R1 in one update at 104.4 s. The model also suppresses a triggered update when a
  periodic one is due, RFC2453-TRIG-4, which no check of this pass reaches.
- **The table request of RIP version 2** is answered 10 µs after it arrives, to the address and
  port of the requester, after split horizon.
- **The entry limit** is 25 for RIP version 2 and follows the MTU for RIPng
  ([`Rip.cc:543`](../../../../../src/inet/routing/rip/Rip.cc)): 35 routes went out in two
  messages, and 83 prefixes of RIPng in two messages of at most 72 entries.
- **The garbage collection after a timeout** and **the end of a garbage collection by a new
  route** work, for the reason that gap 6 names.

## Other findings

- **The dissector of a frame never reaches the RIP header.** The UDP port table has no entry
  for port 520 or 521 ([`ProtocolGroup.cc:157-164`](../../../../../src/inet/common/ProtocolGroup.cc)),
  so the `RipProtocolDissector` that is registered for the protocol is never called on a
  frame. Filter expressions over `rip.*` cannot match, and a packet printer or a capture shows
  the RIP message as an unknown payload. The tests read the message with
  [`RipChecks.h`](../../../../../tests/protocol/rip/RipChecks.h) instead. This is not a finding
  against a statement of the in-scope set, but it is a gap of the model's tooling.
- **The RIPng whole-table request holds an unspecified address** where RFC 2080 has the prefix
  `::`. The model has no byte form for RIPng, so both mean the same 16 zero octets, and the
  test accepts both.
- **The RIPng entries carry address family 2** in the model, a field that RIPng does not have.
  Nothing reads it on the wire, because RIPng has no serializer.

## What the next pass owes

In the order I would do it:

1. **Level 3**, the crafted messages: the validation of a response (the model has the code,
   [`Rip.cc:724-790`](../../../../../src/inet/routing/rip/Rip.cc)), the specific query, the
   route tag, the next hop of a received entry. All of them are claimed, so the ledger records
   them as `owed`.
2. **The level 2 statements this pass left**: the suppression of a triggered update before a
   periodic one, the clearing of the change flags, the switch at half the timeout, three
   routers on one LAN, a configured cost, a default route, a host route, two link-local
   addresses on one interface. The closing list of
   [`checks.md`](../../protocol/rip/checks.md#statements-this-pass-wrote-no-check-for) names
   what each needs.
3. **After a repair of gap 3**, run `Rfc2080GarbageCollection` again: its observation 4, the
   rule RFC2080-TIMER-7, has no verdict until the new route can arrive.
