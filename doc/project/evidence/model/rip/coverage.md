# RIP — coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc2453/catalog.md](../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../standard/rfc2080/catalog.md), [features.md](../../protocol/rip/features.md), [results.md](results.md)

The single place that holds the changing state of the RIP workflow. Every other artifact of
this protocol states what a standard says and never changes again unless the standard
changes. This one changes on every pass.

**No step edits an artifact of an earlier step. Steps 5, 6 and 7 record their outcome here.**

State of the ledger, from the run after the repairs of 2026-09-25 (the level 2 run of
2026-09-24 was at `7969452e2d`, src `5c4f41c600`, with 20 PASS and 10 FAIL):

- Date: 2026-09-29 18:33 +0200
- INET: branch `topic/standards-tests-rip-level2-fixes`, commit `06af064900`, tree clean
- Trees: src `533b256702`, tests/protocol `32bfe2dfb9`
- OMNeT++: 6.4.0, commit `cf58891643`
- Build: debug, built from this commit
- Compiler: Ubuntu clang version 23.0.0
- Platform: Ubuntu 26.04.1 LTS, Linux 7.0.0-34-generic x86_64
- Command: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/rip$'`
- Suite: 30 tests, 30 PASS, so the suite reports PASS
- Target level: 2

The analysis of every failure of the level 2 run, and the repair of each gap, is in
[`results.md`](results.md#the-model-gaps).

## Statement coverage

`Status` is the state of the workflow, not of the standard:

| Status | Means |
| --- | --- |
| `selected` | a check targets it |
| `covered` | a selected check establishes it as a side effect, or it is a permission that a check accepts |
| `owed` | the model claims the behavior and **no check exists yet**; the note names what a check would need |
| `later` | it needs a toolset of a higher level — a crafted message at level 3 — or its category is another suite: a serializer test, the forwarding of IPv4 |
| `no check` | the model does not claim the behavior, so [the third principle](../../../guide/derive-tests-from-a-standard.md#principle-a-claimed-feature-gets-a-test) asks for no test |

A verdict belongs to the test. Where a test fails at a later observation and the observation
of a statement held, the row says PASS and names the observation in the note; where a failure
kept a test from the observation a statement needs, the row says FAIL and says so.

| Status | RFC 2453 | RFC 2080 | Together |
| --- | --- | --- | --- |
| `selected` | 45 | 42 | 87 |
| `covered` | 10 | 7 | 17 |
| `owed` | 11 | 10 | 21 |
| `later` | 19 | 21 | 40 |
| `no check` | 3 | 0 | 3 |
| all | 88 | 80 | 168 |

### RFC 2453

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC2453-MSG-1](../../standard/rfc2453/catalog.md#rfc2453-msg-1) | selected | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | `Rfc2453UpdateTransport` | PASS | — |
| [RFC2453-MSG-2](../../standard/rfc2453/catalog.md#rfc2453-msg-2) | selected | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | `Rfc2453UpdateTransport` | PASS | — |
| [RFC2453-MSG-3](../../standard/rfc2453/catalog.md#rfc2453-msg-3) | selected | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | `Rfc2453UpdateTransport` | PASS | — |
| [RFC2453-MSG-4](../../standard/rfc2453/catalog.md#rfc2453-msg-4) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-MSG-5](../../standard/rfc2453/catalog.md#rfc2453-msg-5) | later | — | — | — | level 3: needs a crafted message: a querier from another port |
| [RFC2453-MSG-6](../../standard/rfc2453/catalog.md#rfc2453-msg-6) | later | — | — | — | serializer test: the bit layout |
| [RFC2453-MSG-7](../../standard/rfc2453/catalog.md#rfc2453-msg-7) | selected | [More than 25 routes (RIP version 2)](../../protocol/rip/checks/response-contents.md#more-than-25-routes-rip-version-2) | `Rfc2453ManyRoutes` | PASS | — |
| [RFC2453-MSG-8](../../standard/rfc2453/catalog.md#rfc2453-msg-8) | later | — | — | — | serializer test: the bit layout |
| [RFC2453-MSG-9](../../standard/rfc2453/catalog.md#rfc2453-msg-9) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MSG-10](../../standard/rfc2453/catalog.md#rfc2453-msg-10) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MSG-11](../../standard/rfc2453/catalog.md#rfc2453-msg-11) | later | — | — | — | serializer test: the bit layout |
| [RFC2453-MSG-12](../../standard/rfc2453/catalog.md#rfc2453-msg-12) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MET-1](../../standard/rfc2453/catalog.md#rfc2453-met-1) | covered | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MET-2](../../standard/rfc2453/catalog.md#rfc2453-met-2) | owed | — | — | — | a configured cost other than 1; the model claims it, the `metric` attribute of `ripConfig` |
| [RFC2453-MET-3](../../standard/rfc2453/catalog.md#rfc2453-met-3) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MET-4](../../standard/rfc2453/catalog.md#rfc2453-met-4) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MET-5](../../standard/rfc2453/catalog.md#rfc2453-met-5) | selected | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | `Rfc2453MetricChain` | PASS | — |
| [RFC2453-TABLE-1](../../standard/rfc2453/catalog.md#rfc2453-table-1) | covered | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | `Rfc2453MetricChain` | PASS | — |
| [RFC2453-TABLE-2](../../standard/rfc2453/catalog.md#rfc2453-table-2) | selected | [More than 25 routes (RIP version 2)](../../protocol/rip/checks/response-contents.md#more-than-25-routes-rip-version-2) | `Rfc2453ManyRoutes` | PASS | — |
| [RFC2453-TABLE-3](../../standard/rfc2453/catalog.md#rfc2453-table-3) | selected | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | `Rfc2453MetricChain` | PASS | — |
| [RFC2453-ADDR-1](../../standard/rfc2453/catalog.md#rfc2453-addr-1) | owed | — | — | — | a static route to one address; routes of every prefix length go through the same code |
| [RFC2453-ADDR-2](../../standard/rfc2453/catalog.md#rfc2453-addr-2) | later | — | — | — | level 5: needs a version 1 router in the network; the model claims the subnet rules of §3.7 with a bare TODO, `Rip.h:77-78` |
| [RFC2453-ADDR-3](../../standard/rfc2453/catalog.md#rfc2453-addr-3) | later | — | — | — | level 5: needs a version 1 router in the network; the model claims the subnet rules of §3.7 with a bare TODO, `Rip.h:77-78` |
| [RFC2453-ADDR-4](../../standard/rfc2453/catalog.md#rfc2453-addr-4) | later | — | — | — | level 5: needs a version 1 router in the network; the model claims the subnet rules of §3.7 with a bare TODO, `Rip.h:77-78` |
| [RFC2453-ADDR-5](../../standard/rfc2453/catalog.md#rfc2453-addr-5) | owed | — | — | — | a static route to one address |
| [RFC2453-ADDR-6](../../standard/rfc2453/catalog.md#rfc2453-addr-6) | owed | — | — | — | a router that originates a default route; the model imports one, `Rip.cc:200-201` |
| [RFC2453-ADDR-7](../../standard/rfc2453/catalog.md#rfc2453-addr-7) | owed | — | — | — | a router with a default route in its configuration |
| [RFC2453-ADDR-8](../../standard/rfc2453/catalog.md#rfc2453-addr-8) | later | — | — | — | another suite: the forwarding of IPv4 |
| [RFC2453-SH-1](../../standard/rfc2453/catalog.md#rfc2453-sh-1) | selected | [Split horizon on the link of a learned route (RIP version 2)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | `Rfc2453SplitHorizon` | PASS | — |
| [RFC2453-SH-2](../../standard/rfc2453/catalog.md#rfc2453-sh-2) | selected | [Split horizon on the link of a learned route (RIP version 2)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | `Rfc2453SplitHorizon` | PASS | the model uses poisoned reverse |
| [RFC2453-SH-3](../../standard/rfc2453/catalog.md#rfc2453-sh-3) | covered | [Split horizon on the link of a learned route (RIP version 2)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | `Rfc2453SplitHorizon` | PASS | a permission; the check accepts both forms |
| [RFC2453-SH-4](../../standard/rfc2453/catalog.md#rfc2453-sh-4) | owed | — | — | — | three routers on one LAN |
| [RFC2453-TIMER-1](../../standard/rfc2453/catalog.md#rfc2453-timer-1) | selected | [Periodic update interval (RIP version 2)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-rip-version-2) | `Rfc2453PeriodicUpdate` | PASS | — |
| [RFC2453-TIMER-2](../../standard/rfc2453/catalog.md#rfc2453-timer-2) | selected | [Periodic update interval (RIP version 2)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-rip-version-2) | `Rfc2453PeriodicUpdate` | PASS | the bounds; the random offset is `later`, a statistical test |
| [RFC2453-TIMER-3](../../standard/rfc2453/catalog.md#rfc2453-timer-3) | covered | [Garbage collection after an expiry (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-rip-version-2) | `Rfc2453ExpiryGarbageCollection` | PASS | — |
| [RFC2453-TIMER-4](../../standard/rfc2453/catalog.md#rfc2453-timer-4) | selected | [Route expiry after a silent neighbor (RIP version 2)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-rip-version-2) | `Rfc2453RouteExpiry` | PASS | repaired, [gap 4](results.md#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent) |
| [RFC2453-TIMER-5](../../standard/rfc2453/catalog.md#rfc2453-timer-5) | selected | [Route expiry after a silent neighbor (RIP version 2)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-rip-version-2); [Garbage collection of a lost network (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-rip-version-2) | `Rfc2453RouteExpiry`, `Rfc2453LostNetworkGarbageCollection` | PASS | repaired, [gap 4](results.md#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent) and [gap 5](results.md#gap-5-defect--a-router-never-removes-a-network-it-lost) |
| [RFC2453-TIMER-6](../../standard/rfc2453/catalog.md#rfc2453-timer-6) | selected | [Garbage collection after an expiry (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-rip-version-2); [Garbage collection of a lost network (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-rip-version-2); [Garbage collection while the next hop still withdraws (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-rip-version-2) | `Rfc2453ExpiryGarbageCollection`, `Rfc2453LostNetworkGarbageCollection`, `Rfc2453WithdrawnRouteGarbageCollection` | PASS | repaired for a lost network, [gap 5](results.md#gap-5-defect--a-router-never-removes-a-network-it-lost), and for a route the next hop withdraws, [gap 6](results.md#gap-6-defect--a-withdrawn-route-stays-while-its-next-hop-repeats-the-withdrawal) |
| [RFC2453-TIMER-7](../../standard/rfc2453/catalog.md#rfc2453-timer-7) | selected | [Garbage collection ended by a new route (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-ended-by-a-new-route-rip-version-2) | `Rfc2453GarbageCollection` | PASS | — |
| [RFC2453-REQ-1](../../standard/rfc2453/catalog.md#rfc2453-req-1) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-REQ-2](../../standard/rfc2453/catalog.md#rfc2453-req-2) | later | — | — | — | level 3: needs a crafted message: a querier |
| [RFC2453-REQ-3](../../standard/rfc2453/catalog.md#rfc2453-req-3) | later | — | — | — | level 3: needs a crafted message: an empty request |
| [RFC2453-REQ-4](../../standard/rfc2453/catalog.md#rfc2453-req-4) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-REQ-5](../../standard/rfc2453/catalog.md#rfc2453-req-5) | later | — | — | — | level 3: needs a crafted message: a specific request |
| [RFC2453-REQ-6](../../standard/rfc2453/catalog.md#rfc2453-req-6) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-REQ-7](../../standard/rfc2453/catalog.md#rfc2453-req-7) | later | — | — | — | level 3: needs a crafted message: a specific request |
| [RFC2453-REQ-8](../../standard/rfc2453/catalog.md#rfc2453-req-8) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-RESP-1](../../standard/rfc2453/catalog.md#rfc2453-resp-1) | covered | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | `Rfc2453MetricChain` | PASS | — |
| [RFC2453-RESP-2](../../standard/rfc2453/catalog.md#rfc2453-resp-2) | later | — | — | — | level 3: needs a crafted message; the model has the check, `Rip.cc:727` |
| [RFC2453-RESP-3](../../standard/rfc2453/catalog.md#rfc2453-resp-3) | later | — | — | — | level 3: needs a crafted message; the model has the check, `Rip.cc:752` |
| [RFC2453-RESP-4](../../standard/rfc2453/catalog.md#rfc2453-resp-4) | later | — | — | — | level 3: needs a crafted message; the model has the check, `Rip.cc:735` |
| [RFC2453-RESP-5](../../standard/rfc2453/catalog.md#rfc2453-resp-5) | later | — | — | — | level 3: needs a crafted message; the model has the checks, `Rip.cc:759-790` |
| [RFC2453-RESP-6](../../standard/rfc2453/catalog.md#rfc2453-resp-6) | selected | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | `Rfc2453MetricChain` | PASS | — |
| [RFC2453-RESP-7](../../standard/rfc2453/catalog.md#rfc2453-resp-7) | selected | [Garbage collection after an expiry (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-rip-version-2) | `Rfc2453ExpiryGarbageCollection` | PASS | — |
| [RFC2453-RESP-8](../../standard/rfc2453/catalog.md#rfc2453-resp-8) | covered | [Route expiry after a silent neighbor (RIP version 2)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-rip-version-2) | `Rfc2453RouteExpiry` | PASS | observation 3 of `Rfc2453RouteExpiry` holds; the test fails at observation 4 |
| [RFC2453-RESP-9](../../standard/rfc2453/catalog.md#rfc2453-resp-9) | selected | [Shorter path kept (RIP version 2)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-rip-version-2); [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `Rfc2453ShorterPath`, `Rfc2453LostNetwork` | PASS | — |
| [RFC2453-RESP-10](../../standard/rfc2453/catalog.md#rfc2453-resp-10) | selected | [Garbage collection while the next hop still withdraws (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-rip-version-2) | `Rfc2453WithdrawnRouteGarbageCollection` | PASS | repaired, [gap 6](results.md#gap-6-defect--a-withdrawn-route-stays-while-its-next-hop-repeats-the-withdrawal) |
| [RFC2453-RESP-11](../../standard/rfc2453/catalog.md#rfc2453-resp-11) | owed | — | — | — | two equal paths and a next hop that goes silent; the model claims it with a bare TODO, `Rip.cc:712` |
| [RFC2453-RESP-12](../../standard/rfc2453/catalog.md#rfc2453-resp-12) | selected | [Shorter path kept (RIP version 2)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-rip-version-2) | `Rfc2453ShorterPath` | PASS | — |
| [RFC2453-OUT-1](../../standard/rfc2453/catalog.md#rfc2453-out-1) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-OUT-2](../../standard/rfc2453/catalog.md#rfc2453-out-2) | selected | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | `Rfc2453UpdateTransport` | PASS | — |
| [RFC2453-TRIG-1](../../standard/rfc2453/catalog.md#rfc2453-trig-1) | selected | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `Rfc2453LostNetwork` | PASS | — |
| [RFC2453-TRIG-2](../../standard/rfc2453/catalog.md#rfc2453-trig-2) | selected | [Rate of triggered updates (RIP version 2)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-rip-version-2) | `Rfc2453TriggeredRate` | PASS | — |
| [RFC2453-TRIG-3](../../standard/rfc2453/catalog.md#rfc2453-trig-3) | selected | [Rate of triggered updates (RIP version 2)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-rip-version-2) | `Rfc2453TriggeredRate` | PASS | the lower bound; the random timer is `later`, a statistical test |
| [RFC2453-TRIG-4](../../standard/rfc2453/catalog.md#rfc2453-trig-4) | owed | — | — | — | a change timed a few seconds before a periodic update; the model has the code, `Rip.cc:895` |
| [RFC2453-TRIG-5](../../standard/rfc2453/catalog.md#rfc2453-trig-5) | selected | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `Rfc2453LostNetwork` | PASS | — |
| [RFC2453-TRIG-6](../../standard/rfc2453/catalog.md#rfc2453-trig-6) | selected | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `Rfc2453LostNetwork` | PASS | — |
| [RFC2453-TRIG-7](../../standard/rfc2453/catalog.md#rfc2453-trig-7) | covered | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `Rfc2453LostNetwork` | PASS | a permission |
| [RFC2453-TRIG-8](../../standard/rfc2453/catalog.md#rfc2453-trig-8) | owed | — | — | — | two changes some seconds apart; the model has the code, `Rip.cc:452` |
| [RFC2453-TRIG-9](../../standard/rfc2453/catalog.md#rfc2453-trig-9) | selected | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | `Rfc2453LostNetwork` | PASS | — |
| [RFC2453-GEN-1](../../standard/rfc2453/catalog.md#rfc2453-gen-1) | selected | [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | `Rfc2453TableRequest` | PASS | — |
| [RFC2453-GEN-2](../../standard/rfc2453/catalog.md#rfc2453-gen-2) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-GEN-3](../../standard/rfc2453/catalog.md#rfc2453-gen-3) | selected | [More than 25 routes (RIP version 2)](../../protocol/rip/checks/response-contents.md#more-than-25-routes-rip-version-2) | `Rfc2453ManyRoutes` | PASS | — |
| [RFC2453-GEN-4](../../standard/rfc2453/catalog.md#rfc2453-gen-4) | covered | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-GEN-5](../../standard/rfc2453/catalog.md#rfc2453-gen-5) | selected | [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2); [Split horizon on the link of a learned route (RIP version 2)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | `Rfc2453LostNetwork`, `Rfc2453SplitHorizon` | PASS | — |
| [RFC2453-TAG-1](../../standard/rfc2453/catalog.md#rfc2453-tag-1) | later | — | — | — | level 3: needs a crafted message: a route with a tag; the model keeps the tag, `Rip.cc:850` |
| [RFC2453-TAG-2](../../standard/rfc2453/catalog.md#rfc2453-tag-2) | owed | — | — | — | an import with a configured tag; `importRoute` has the parameter, `Rip.cc:266`, and every caller passes the default |
| [RFC2453-MASK-1](../../standard/rfc2453/catalog.md#rfc2453-mask-1) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | — |
| [RFC2453-MASK-2](../../standard/rfc2453/catalog.md#rfc2453-mask-2) | no check | — | — | — | the model does not claim version 1: it sends version 2 only and has no compatibility switch |
| [RFC2453-NH-1](../../standard/rfc2453/catalog.md#rfc2453-nh-1) | covered | [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | `Rfc2453MetricChain` | PASS | — |
| [RFC2453-NH-2](../../standard/rfc2453/catalog.md#rfc2453-nh-2) | selected | [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | `Rfc2453UpdateFields` | PASS | the model always sends 0.0.0.0 |
| [RFC2453-NH-3](../../standard/rfc2453/catalog.md#rfc2453-nh-3) | later | — | — | — | level 3: needs a crafted message: a response that names another next hop |
| [RFC2453-NH-4](../../standard/rfc2453/catalog.md#rfc2453-nh-4) | later | — | — | — | level 3: needs a crafted message: a response that names an unreachable next hop |
| [RFC2453-MCAST-1](../../standard/rfc2453/catalog.md#rfc2453-mcast-1) | selected | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | `Rfc2453UpdateTransport` | PASS | — |
| [RFC2453-MCAST-2](../../standard/rfc2453/catalog.md#rfc2453-mcast-2) | owed | — | — | — | a second network behind the receiver of a multicast |
| [RFC2453-MCAST-3](../../standard/rfc2453/catalog.md#rfc2453-mcast-3) | no check | — | — | — | the model does not claim networks without broadcast: it has no list of neighbors |
| [RFC2453-MCAST-4](../../standard/rfc2453/catalog.md#rfc2453-mcast-4) | covered | [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | `Rfc2453UpdateTransport` | PASS | — |
| [RFC2453-QRY-1](../../standard/rfc2453/catalog.md#rfc2453-qry-1) | no check | — | — | — | the model does not claim version 1: it sends version 2 only and has no compatibility switch; it never reads the version of a request |

### RFC 2080

| Statement | Status | Check | Test | Verdict | Note |
| --- | --- | --- | --- | --- | --- |
| [RFC2080-MSG-1](../../standard/rfc2080/catalog.md#rfc2080-msg-1) | selected | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | `Rfc2080UpdateTransport` | PASS | — |
| [RFC2080-MSG-2](../../standard/rfc2080/catalog.md#rfc2080-msg-2) | selected | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | `Rfc2080UpdateTransport` | PASS | — |
| [RFC2080-MSG-3](../../standard/rfc2080/catalog.md#rfc2080-msg-3) | selected | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | `Rfc2080UpdateTransport` | PASS | — |
| [RFC2080-MSG-4](../../standard/rfc2080/catalog.md#rfc2080-msg-4) | selected | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | `Rfc2080TableRequest` | PASS | repaired, [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface) |
| [RFC2080-MSG-5](../../standard/rfc2080/catalog.md#rfc2080-msg-5) | later | — | — | — | level 3: needs a crafted message: a querier from another port |
| [RFC2080-MSG-6](../../standard/rfc2080/catalog.md#rfc2080-msg-6) | later | — | — | — | serializer test: the bit layout; the model has no RIPng serializer |
| [RFC2080-MSG-7](../../standard/rfc2080/catalog.md#rfc2080-msg-7) | later | — | — | — | serializer test: the bit layout |
| [RFC2080-MSG-8](../../standard/rfc2080/catalog.md#rfc2080-msg-8) | selected | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | observation 2 of `Rfc2080UpdateFields` holds; the test fails at observation 6 |
| [RFC2080-MSG-9](../../standard/rfc2080/catalog.md#rfc2080-msg-9) | later | — | — | — | serializer test: the bit layout |
| [RFC2080-MSG-10](../../standard/rfc2080/catalog.md#rfc2080-msg-10) | selected | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | observation 4 holds; the test fails at observation 6 |
| [RFC2080-MSG-11](../../standard/rfc2080/catalog.md#rfc2080-msg-11) | selected | [Messages limited by the MTU (RIPng)](../../protocol/rip/checks/response-contents.md#messages-limited-by-the-mtu-ripng) | `Rfc2080MtuMessages` | PASS | — |
| [RFC2080-MET-1](../../standard/rfc2080/catalog.md#rfc2080-met-1) | covered | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | observation 3 holds |
| [RFC2080-MET-2](../../standard/rfc2080/catalog.md#rfc2080-met-2) | owed | — | — | — | a configured cost other than 1 |
| [RFC2080-MET-3](../../standard/rfc2080/catalog.md#rfc2080-met-3) | selected | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | observation 4 holds |
| [RFC2080-MET-4](../../standard/rfc2080/catalog.md#rfc2080-met-4) | selected | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | observation 3 holds |
| [RFC2080-MET-5](../../standard/rfc2080/catalog.md#rfc2080-met-5) | selected | [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | `Rfc2080MetricChain` | PASS | — |
| [RFC2080-TABLE-1](../../standard/rfc2080/catalog.md#rfc2080-table-1) | covered | [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | `Rfc2080MetricChain` | PASS | — |
| [RFC2080-TABLE-2](../../standard/rfc2080/catalog.md#rfc2080-table-2) | selected | [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | `Rfc2080MetricChain` | PASS | — |
| [RFC2080-TAG-1](../../standard/rfc2080/catalog.md#rfc2080-tag-1) | later | — | — | — | level 3: needs a crafted message: a route with a tag |
| [RFC2080-TAG-2](../../standard/rfc2080/catalog.md#rfc2080-tag-2) | owed | — | — | — | an import with a configured tag |
| [RFC2080-NH-1](../../standard/rfc2080/catalog.md#rfc2080-nh-1) | later | — | — | — | serializer test: the bit layout: the model holds the next hop in each entry and has no next hop entry |
| [RFC2080-NH-2](../../standard/rfc2080/catalog.md#rfc2080-nh-2) | later | — | — | — | serializer test: the bit layout |
| [RFC2080-NH-3](../../standard/rfc2080/catalog.md#rfc2080-nh-3) | later | — | — | — | serializer test: the bit layout |
| [RFC2080-NH-4](../../standard/rfc2080/catalog.md#rfc2080-nh-4) | later | — | — | — | level 3: needs a crafted message: a response with a next hop entry of zeros |
| [RFC2080-NH-5](../../standard/rfc2080/catalog.md#rfc2080-nh-5) | later | — | — | — | level 3: needs a crafted message: the model never names a next hop |
| [RFC2080-NH-6](../../standard/rfc2080/catalog.md#rfc2080-nh-6) | later | — | — | — | level 3: needs a crafted message: a response that names another next hop |
| [RFC2080-NH-7](../../standard/rfc2080/catalog.md#rfc2080-nh-7) | later | — | — | — | level 3: needs a crafted message: a response that names a global next hop |
| [RFC2080-ADDR-1](../../standard/rfc2080/catalog.md#rfc2080-addr-1) | owed | — | — | — | a router that originates a default route |
| [RFC2080-ADDR-2](../../standard/rfc2080/catalog.md#rfc2080-addr-2) | owed | — | — | — | a router that originates a default route |
| [RFC2080-ADDR-3](../../standard/rfc2080/catalog.md#rfc2080-addr-3) | owed | — | — | — | a configured metric for the default route; `importRoute` has the parameter, `Rip.cc:266` |
| [RFC2080-SH-1](../../standard/rfc2080/catalog.md#rfc2080-sh-1) | selected | [Split horizon on the link of a learned route (RIPng)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-ripng) | `Rfc2080SplitHorizon` | PASS | — |
| [RFC2080-SH-2](../../standard/rfc2080/catalog.md#rfc2080-sh-2) | selected | [Split horizon on the link of a learned route (RIPng)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-ripng) | `Rfc2080SplitHorizon` | PASS | the model uses poisoned reverse |
| [RFC2080-SH-3](../../standard/rfc2080/catalog.md#rfc2080-sh-3) | owed | — | — | — | the three settings of one interface; the model claims them, the `mode` attribute of `ripConfig` |
| [RFC2080-TIMER-1](../../standard/rfc2080/catalog.md#rfc2080-timer-1) | selected | [Periodic update interval (RIPng)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-ripng) | `Rfc2080PeriodicUpdate` | PASS | — |
| [RFC2080-TIMER-2](../../standard/rfc2080/catalog.md#rfc2080-timer-2) | selected | [Periodic update interval (RIPng)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-ripng) | `Rfc2080PeriodicUpdate` | PASS | the bounds; the random offset is `later`, a statistical test |
| [RFC2080-TIMER-3](../../standard/rfc2080/catalog.md#rfc2080-timer-3) | covered | [Garbage collection after an expiry (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-ripng) | `Rfc2080ExpiryGarbageCollection` | PASS | — |
| [RFC2080-TIMER-4](../../standard/rfc2080/catalog.md#rfc2080-timer-4) | selected | [Route expiry after a silent neighbor (RIPng)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-ripng) | `Rfc2080RouteExpiry` | PASS | repaired, [gap 4](results.md#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent) |
| [RFC2080-TIMER-5](../../standard/rfc2080/catalog.md#rfc2080-timer-5) | selected | [Route expiry after a silent neighbor (RIPng)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-ripng); [Garbage collection of a lost network (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-ripng); [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080RouteExpiry`, `Rfc2080LostNetworkGarbageCollection`, `Rfc2080LostNetwork` | PASS | repaired, [gap 4](results.md#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent) and [gap 5](results.md#gap-5-defect--a-router-never-removes-a-network-it-lost) |
| [RFC2080-TIMER-6](../../standard/rfc2080/catalog.md#rfc2080-timer-6) | selected | [Garbage collection after an expiry (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-ripng); [Garbage collection of a lost network (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-ripng); [Garbage collection while the next hop still withdraws (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-ripng) | `Rfc2080ExpiryGarbageCollection`, `Rfc2080LostNetworkGarbageCollection`, `Rfc2080WithdrawnRouteGarbageCollection` | PASS | repaired for a lost network, [gap 5](results.md#gap-5-defect--a-router-never-removes-a-network-it-lost), and for a route the next hop withdraws, [gap 6](results.md#gap-6-defect--a-withdrawn-route-stays-while-its-next-hop-repeats-the-withdrawal) |
| [RFC2080-TIMER-7](../../standard/rfc2080/catalog.md#rfc2080-timer-7) | selected | [Garbage collection ended by a new route (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-ended-by-a-new-route-ripng) | `Rfc2080GarbageCollection` | PASS | reached since the repair of [gap 3](results.md#gap-3-defect--an-ipv6-router-does-not-advertise-its-network-again-after-the-link-returns) |
| [RFC2080-REQ-1](../../standard/rfc2080/catalog.md#rfc2080-req-1) | selected | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | `Rfc2080TableRequest` | PASS | observation 2 of `Rfc2080TableRequest` holds; the test fails at observation 4 |
| [RFC2080-REQ-2](../../standard/rfc2080/catalog.md#rfc2080-req-2) | later | — | — | — | level 3: needs a crafted message: a querier |
| [RFC2080-REQ-3](../../standard/rfc2080/catalog.md#rfc2080-req-3) | later | — | — | — | level 3: needs a crafted message: an empty request |
| [RFC2080-REQ-4](../../standard/rfc2080/catalog.md#rfc2080-req-4) | selected | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | `Rfc2080TableRequest` | PASS | the answer repaired, [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface) |
| [RFC2080-REQ-5](../../standard/rfc2080/catalog.md#rfc2080-req-5) | later | — | — | — | level 3: needs a crafted message: a specific request |
| [RFC2080-REQ-6](../../standard/rfc2080/catalog.md#rfc2080-req-6) | selected | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | `Rfc2080TableRequest` | PASS | reached since the repair of [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface) |
| [RFC2080-REQ-7](../../standard/rfc2080/catalog.md#rfc2080-req-7) | later | — | — | — | level 3: needs a crafted message: a specific request |
| [RFC2080-REQ-8](../../standard/rfc2080/catalog.md#rfc2080-req-8) | selected | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | `Rfc2080TableRequest` | PASS | observation 3 holds |
| [RFC2080-RESP-1](../../standard/rfc2080/catalog.md#rfc2080-resp-1) | covered | [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | `Rfc2080MetricChain` | PASS | — |
| [RFC2080-RESP-2](../../standard/rfc2080/catalog.md#rfc2080-resp-2) | later | — | — | — | level 3: needs a crafted message |
| [RFC2080-RESP-3](../../standard/rfc2080/catalog.md#rfc2080-resp-3) | later | — | — | — | level 3: needs a crafted message |
| [RFC2080-RESP-4](../../standard/rfc2080/catalog.md#rfc2080-resp-4) | later | — | — | — | level 3: needs a crafted message |
| [RFC2080-RESP-5](../../standard/rfc2080/catalog.md#rfc2080-resp-5) | selected | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | `Rfc2080UpdateTransport` | PASS | — |
| [RFC2080-RESP-6](../../standard/rfc2080/catalog.md#rfc2080-resp-6) | later | — | — | — | level 3: needs a crafted message: a multicast with another hop limit; the model has the check, `Rip.cc:745` |
| [RFC2080-RESP-7](../../standard/rfc2080/catalog.md#rfc2080-resp-7) | later | — | — | — | level 3: needs a crafted message |
| [RFC2080-RESP-8](../../standard/rfc2080/catalog.md#rfc2080-resp-8) | selected | [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | `Rfc2080MetricChain` | PASS | — |
| [RFC2080-RESP-9](../../standard/rfc2080/catalog.md#rfc2080-resp-9) | selected | [Garbage collection after an expiry (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-ripng) | `Rfc2080ExpiryGarbageCollection` | PASS | — |
| [RFC2080-RESP-10](../../standard/rfc2080/catalog.md#rfc2080-resp-10) | covered | [Route expiry after a silent neighbor (RIPng)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-ripng) | `Rfc2080RouteExpiry` | PASS | observation 3 of `Rfc2080RouteExpiry` holds |
| [RFC2080-RESP-11](../../standard/rfc2080/catalog.md#rfc2080-resp-11) | selected | [Shorter path kept (RIPng)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-ripng); [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080ShorterPath`, `Rfc2080LostNetwork` | PASS | observation 4 of `Rfc2080LostNetwork` holds |
| [RFC2080-RESP-12](../../standard/rfc2080/catalog.md#rfc2080-resp-12) | selected | [Garbage collection while the next hop still withdraws (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-ripng) | `Rfc2080WithdrawnRouteGarbageCollection` | PASS | repaired, [gap 6](results.md#gap-6-defect--a-withdrawn-route-stays-while-its-next-hop-repeats-the-withdrawal) |
| [RFC2080-RESP-13](../../standard/rfc2080/catalog.md#rfc2080-resp-13) | owed | — | — | — | two equal paths and a next hop that goes silent; the bare TODO at `Rip.cc:712` names RIPng |
| [RFC2080-RESP-14](../../standard/rfc2080/catalog.md#rfc2080-resp-14) | selected | [Shorter path kept (RIPng)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-ripng) | `Rfc2080ShorterPath` | PASS | — |
| [RFC2080-OUT-1](../../standard/rfc2080/catalog.md#rfc2080-out-1) | selected | [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | `Rfc2080TableRequest` | PASS | repaired, [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface) |
| [RFC2080-OUT-2](../../standard/rfc2080/catalog.md#rfc2080-out-2) | selected | [Periodic update interval (RIPng)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-ripng); [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080PeriodicUpdate`, `Rfc2080LostNetwork` | PASS | — |
| [RFC2080-OUT-3](../../standard/rfc2080/catalog.md#rfc2080-out-3) | selected | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | `Rfc2080UpdateTransport` | PASS | — |
| [RFC2080-TRIG-1](../../standard/rfc2080/catalog.md#rfc2080-trig-1) | selected | [Rate of triggered updates (RIPng)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-ripng) | `Rfc2080TriggeredRate` | PASS | — |
| [RFC2080-TRIG-2](../../standard/rfc2080/catalog.md#rfc2080-trig-2) | selected | [Rate of triggered updates (RIPng)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-ripng) | `Rfc2080TriggeredRate` | PASS | the lower bound; the random timer is `later`, a statistical test |
| [RFC2080-TRIG-3](../../standard/rfc2080/catalog.md#rfc2080-trig-3) | owed | — | — | — | a change timed a few seconds before a periodic update; the model has the code, `Rip.cc:895` |
| [RFC2080-TRIG-4](../../standard/rfc2080/catalog.md#rfc2080-trig-4) | selected | [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080LostNetwork` | PASS | observation 2 holds |
| [RFC2080-TRIG-5](../../standard/rfc2080/catalog.md#rfc2080-trig-5) | selected | [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080LostNetwork` | PASS | observation 4 holds |
| [RFC2080-TRIG-6](../../standard/rfc2080/catalog.md#rfc2080-trig-6) | covered | [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080LostNetwork` | PASS | a permission |
| [RFC2080-TRIG-7](../../standard/rfc2080/catalog.md#rfc2080-trig-7) | owed | — | — | — | two changes some seconds apart; the model has the code, `Rip.cc:452` |
| [RFC2080-TRIG-8](../../standard/rfc2080/catalog.md#rfc2080-trig-8) | selected | [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | `Rfc2080LostNetwork` | PASS | the version repaired, [gap 1](results.md#gap-1-defect--ripng-messages-carry-version-2) |
| [RFC2080-GEN-1](../../standard/rfc2080/catalog.md#rfc2080-gen-1) | selected | [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | `Rfc2080UpdateTransport` | PASS | for the updates, and for the answer to a request since the repair of [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface) |
| [RFC2080-GEN-2](../../standard/rfc2080/catalog.md#rfc2080-gen-2) | owed | — | — | — | an interface with two link-local addresses |
| [RFC2080-GEN-3](../../standard/rfc2080/catalog.md#rfc2080-gen-3) | selected | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | repaired, [gap 1](results.md#gap-1-defect--ripng-messages-carry-version-2) |
| [RFC2080-GEN-4](../../standard/rfc2080/catalog.md#rfc2080-gen-4) | selected | [Messages limited by the MTU (RIPng)](../../protocol/rip/checks/response-contents.md#messages-limited-by-the-mtu-ripng) | `Rfc2080MtuMessages` | PASS | — |
| [RFC2080-GEN-5](../../standard/rfc2080/catalog.md#rfc2080-gen-5) | selected | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | observation 5 holds |
| [RFC2080-GEN-6](../../standard/rfc2080/catalog.md#rfc2080-gen-6) | covered | [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | `Rfc2080UpdateFields` | PASS | — |
| [RFC2080-GEN-7](../../standard/rfc2080/catalog.md#rfc2080-gen-7) | selected | [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng); [Split horizon on the link of a learned route (RIPng)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-ripng) | `Rfc2080LostNetwork`, `Rfc2080SplitHorizon` | PASS | — |
## The coverage debt: the checks this pass owes

Twenty-one statements that the model claims have no check yet. Each needs a normal exchange
only, so each is level 2 work for a later pass:

| What the check needs | Statements |
| --- | --- |
| a configured cost other than 1 | RFC2453-MET-2, RFC2080-MET-2 |
| the three settings of split horizon on one interface | RFC2080-SH-3 |
| three routers on one LAN | RFC2453-SH-4 |
| a change timed a few seconds before a periodic update | RFC2453-TRIG-4, RFC2080-TRIG-3 |
| two changes some seconds apart | RFC2453-TRIG-8, RFC2080-TRIG-7 |
| two equal paths and a next hop that goes silent | RFC2453-RESP-11, RFC2080-RESP-13 |
| a router that originates a default route, with a configured metric | RFC2453-ADDR-6, ADDR-7, RFC2080-ADDR-1, ADDR-2, ADDR-3 |
| a static route to one address | RFC2453-ADDR-1, ADDR-5 |
| an import with a configured route tag | RFC2453-TAG-2, RFC2080-TAG-2 |
| a second network behind the receiver of a multicast | RFC2453-MCAST-2 |
| an interface with two link-local addresses | RFC2080-GEN-2 |

Two of them will fail when their check exists, from what the code shows: RFC2453-RESP-11 and
RFC2080-RESP-13 are a bare TODO (`Rip.cc:748`), and the route tag and the metric of an import
are parameters that every caller leaves at the default (`Rip.cc:272`).

## Feature support

The rule of the guide, per feature: `supported` when every core check ran and passed,
`partial` when at least one passed and at least one failed or has no check, `not supported`
when every core check that ran failed, `untested` when no core check exists.

| Feature | Level | Support | Core statements that fail or have no check |
| --- | --- | --- | --- |
| [RIP-F-MESSAGE-FORMAT](../../protocol/rip/features.md#rip-f-message-format) | mandatory | supported | — (RFC2080-GEN-3 repaired, [gap 1](results.md#gap-1-defect--ripng-messages-carry-version-2)) |
| [RIP-F-TRANSPORT](../../protocol/rip/features.md#rip-f-transport) | mandatory | supported | — (RFC2080-MSG-4 repaired, [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface)) |
| [RIP-F-UPDATE-ADDRESSING](../../protocol/rip/features.md#rip-f-update-addressing) | mandatory | supported | — |
| [RIP-F-METRIC](../../protocol/rip/features.md#rip-f-metric) | mandatory | supported | — |
| [RIP-F-DESTINATION-PREFIX](../../protocol/rip/features.md#rip-f-destination-prefix) | mandatory | supported | — |
| [RIP-F-PERIODIC-UPDATE](../../protocol/rip/features.md#rip-f-periodic-update) | mandatory | supported | — |
| [RIP-F-ROUTE-LEARNING](../../protocol/rip/features.md#rip-f-route-learning) | mandatory | supported | — |
| [RIP-F-SPLIT-HORIZON](../../protocol/rip/features.md#rip-f-split-horizon) | mandatory | supported | — |
| [RIP-F-TRIGGERED-UPDATE](../../protocol/rip/features.md#rip-f-triggered-update) | mandatory | supported | — (RFC2080-TRIG-8 repaired, [gap 1](results.md#gap-1-defect--ripng-messages-carry-version-2)) |
| [RIP-F-ROUTE-EXPIRY](../../protocol/rip/features.md#rip-f-route-expiry) | mandatory | supported | — (TIMER-4, TIMER-5 and TIMER-6 of both documents repaired, [gaps 4, 5, 6](results.md#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent); RFC2080-TIMER-7 reached since the repair of [gap 3](results.md#gap-3-defect--an-ipv6-router-does-not-advertise-its-network-again-after-the-link-returns)) |
| [RIP-F-RESPONSE-CONTENTS](../../protocol/rip/features.md#rip-f-response-contents) | mandatory | supported | — |
| [RIP-F-RESPONSE-VALIDATION](../../protocol/rip/features.md#rip-f-response-validation) | mandatory | untested | every core statement is `later`, level 3 |
| [RIP-F-TABLE-REQUEST](../../protocol/rip/features.md#rip-f-table-request) | unstated | supported | — (RFC2080-REQ-4, REQ-6, OUT-1 repaired, [gap 2](results.md#gap-2-defect--the-ripng-answer-to-a-request-leaves-on-the-wrong-interface)) |
| [RIP-F-SPECIFIC-QUERY](../../protocol/rip/features.md#rip-f-specific-query) | unstated | untested | every core statement is `later`, level 3 |
| [RIP-F-NEXT-HOP](../../protocol/rip/features.md#rip-f-next-hop) | optional | partial | RFC2080-NH-4, NH-5 are `later`, level 3 |
| [RIP-F-ROUTE-TAG](../../protocol/rip/features.md#rip-f-route-tag) | mandatory | untested | RFC2453-TAG-1, RFC2080-TAG-1 are `later`, level 3 |
| [RIP-F-DEFAULT-ROUTE](../../protocol/rip/features.md#rip-f-default-route) | unstated | untested | every core statement is `owed` |
| [RIP-F-HOST-ROUTES](../../protocol/rip/features.md#rip-f-host-routes) | optional | untested | every core statement is `owed` |
| [RIP-F-VERSION-1-INTERWORKING](../../protocol/rip/features.md#rip-f-version-1-interworking) | mandatory | untested | RFC2453-ADDR-2, ADDR-4 are `later`, level 5; MASK-2, QRY-1 are `no check`: the model has no version 1 |

Twelve features are supported, one partial, six untested, and none is not supported. In the
level 2 run of 2026-09-24, before the repairs, seven were supported and six partial.

## Achieved level

**Level 2, reached.** Target: level 2, from
[`standards.md`](../../protocol/rip/standards.md#target-level).

The exit criterion of level 2 has two halves, and both hold:

| Half of the criterion | State | Evidence |
| --- | --- | --- |
| Every normal-path mandatory mechanism of the base documents appears as a feature | holds | the catalogs hold every normative statement of RFC 2453 §3 and §4.2 to §4.6 and of RFC 2080 §2, 168 entries; the feature map has 19 features and places every entry but the forwarding rule RFC2453-ADDR-8 |
| Every mandatory feature has a core check that ran and has a verdict | holds for the normal path | 11 of the 14 mandatory features have core checks that ran: 30 tests, 30 PASS since the repairs (20 PASS, 10 FAIL in the level 2 run) |

The three mandatory features without a core check have no normal path, so the criterion does
not reach them at this level, and the ledger says so rather than count them:
RIP-F-RESPONSE-VALIDATION and RIP-F-ROUTE-TAG need a crafted response (a tag other than zero
reaches a router only in a crafted response, because no import in the model sets one), which
is level 3; RIP-F-VERSION-1-INTERWORKING needs a version 1 router, which the standards map
puts at level 5.

A level is a measure of how deeply the pass looked, not of how well the model did. In the level
2 run ten tests failed, and six gaps of the model stood behind them; the level held because each
of those checks ran. The repairs of 2026-09-25 closed the six gaps, and all 30 tests pass.

| Level | State | What it needs |
| --- | --- | --- |
| 1, Survey | **reached** | — |
| 2, Core | **reached** | the 21 `owed` statements are the debt of the level; see [the coverage debt](#the-coverage-debt-the-checks-this-pass-owes) |
| 3, Edge | not started | the catalog half holds; the checks need crafted responses and requests: the validation of a response, the specific query, the route tag, the next hop of a received entry |
| 4, Dynamics | partial | every timer of the in-scope set has a check with a stated window — 30 s, 180 s, 120 s, 1 to 5 s — but the random offset of the periodic update and the random timer of the triggered update have no statistical test |
| 5, Complete | not started | authentication, the interworking with version 1, demand circuits |

Per feature, the level reached is the level of the rows above: level 2 for the eleven
mandatory features with a normal path and for RIP-F-TABLE-REQUEST and RIP-F-NEXT-HOP; level 1
for RIP-F-RESPONSE-VALIDATION, RIP-F-SPECIFIC-QUERY and RIP-F-ROUTE-TAG, whose checks are
level 3; level 1 for RIP-F-DEFAULT-ROUTE and RIP-F-HOST-ROUTES, whose checks are owed; and
level 1 for RIP-F-VERSION-1-INTERWORKING.

## Pass log

| Pass | Date | Level | Scope | Result |
| --- | --- | --- | --- | --- |
| 1 | 2026-09-23 | **1, reached** | RFC 2453 and RFC 2080 downloaded; the standards map; the claims of the model | no run; one obsolete claim (RFC 1058), one declined area (authentication) |
| 2 | 2026-09-24 | **2, reached** | catalogs of RFC 2453 (88) and RFC 2080 (80); 19 features; 30 checks; 30 tests; the conformance matrix | 20 PASS, 10 FAIL, none declared; six gaps of the model, three of them in RIPng alone; 21 statements owed |
| repairs | 2026-09-25 | 2, held | the six gaps, on `topic/standards-tests-rip-level2-fixes`; one test error of the split-horizon checks | 30 PASS; 21 statements still owed |

The read record of pass 1 named commit `97f4559eee` of the wave 0 branch; its trees, src
`16dc528e10` and tests/protocol `6f0a6bdb05`, identify the code it read.

## Out of scope

The areas that the catalogs leave out, and why, are at the end of each catalog:
[RFC 2453](../../standard/rfc2453/catalog.md#out-of-scope-in-this-catalog),
[RFC 2080](../../standard/rfc2080/catalog.md#out-of-scope-in-this-catalog). The documents
outside the in-scope set are in [`standards.md`](../../protocol/rip/standards.md#in-scope-set).
