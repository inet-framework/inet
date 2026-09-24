# RIP checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
category the check belongs to, confirmed after the run, and the reason. The categories and
what each can establish are in [test-anatomy.md](../../../design/test-anatomy.md#the-categories);
the rule that the category must match the claim is
[TR-CAT-MATCH](../../../rule/testing.md#tr-cat-match).

## The decision, per check

All 30 checks of this pass are **protocol tests**, and all 30 are in `tests/protocol/rip/`.
The prediction that step 3 made from the observation class held for every one of them: RIP is
a protocol of messages between neighbors, so `wire` dominates both catalogs, and a `wire`
statement maps to a protocol test.

| Check | Observation class of its statements | Category | Reason |
| --- | --- | --- | --- |
| [Update transport and addressing (RIP version 2)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-rip-version-2) | wire | protocol test | Ports and addresses of one message on a link. |
| [Update message fields (RIP version 2)](../../protocol/rip/checks/update-message.md#update-message-fields-rip-version-2) | wire | protocol test | Header and entry fields of one message on a link, as the message holds them; the bit layout is a serializer test. |
| [Update transport and addressing (RIPng)](../../protocol/rip/checks/update-message.md#update-transport-and-addressing-ripng) | wire | protocol test | Ports and addresses of one message on a link. |
| [Update message fields (RIPng)](../../protocol/rip/checks/update-message.md#update-message-fields-ripng) | wire | protocol test | Header and entry fields of one message on a link, as the message holds them; the bit layout is a serializer test. |
| [Periodic update interval (RIP version 2)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-rip-version-2) | wire | protocol test | Instants of consecutive messages on one link, against the bounds that the standard gives. |
| [Periodic update interval (RIPng)](../../protocol/rip/checks/periodic-update.md#periodic-update-interval-ripng) | wire | protocol test | Instants of consecutive messages on one link, against the bounds that the standard gives. |
| [Metric through a chain (RIP version 2)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-rip-version-2) | wire, end-to-end | protocol test | Entries on three links, then one echo exchange across the chain. |
| [Metric through a chain (RIPng)](../../protocol/rip/checks/route-learning.md#metric-through-a-chain-ripng) | wire, end-to-end | protocol test | Entries on three links, then one echo exchange across the chain. |
| [Shorter path kept (RIP version 2)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-rip-version-2) | wire | protocol test | Entries of one router over a window, after a stimulus on another link. |
| [Shorter path kept (RIPng)](../../protocol/rip/checks/route-learning.md#shorter-path-kept-ripng) | wire | protocol test | Entries of one router over a window, after a stimulus on another link. |
| [Split horizon on the link of a learned route (RIP version 2)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-rip-version-2) | wire | protocol test | Entries on the link a route was learned from, against the entries on the other link. |
| [Split horizon on the link of a learned route (RIPng)](../../protocol/rip/checks/split-horizon.md#split-horizon-on-the-link-of-a-learned-route-ripng) | wire | protocol test | Entries on the link a route was learned from, against the entries on the other link. |
| [Table request of a restarted router (RIP version 2)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-rip-version-2) | wire | protocol test | A request and its answer on one link, after a restart that the scenario sets up. |
| [Table request of a restarted router (RIPng)](../../protocol/rip/checks/table-request.md#table-request-of-a-restarted-router-ripng) | wire | protocol test | A request and its answer on one link, after a restart that the scenario sets up. |
| [Triggered update for a lost network (RIP version 2)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-rip-version-2) | wire | protocol test | Messages on two links after a link failure that the scenario sets up. |
| [Triggered update for a lost network (RIPng)](../../protocol/rip/checks/triggered-update.md#triggered-update-for-a-lost-network-ripng) | wire | protocol test | Messages on two links after a link failure that the scenario sets up. |
| [Rate of triggered updates (RIP version 2)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-rip-version-2) | wire | protocol test | Count and spacing of messages in a window of 10 s, against the bounds of the standard. |
| [Rate of triggered updates (RIPng)](../../protocol/rip/checks/triggered-update.md#rate-of-triggered-updates-ripng) | wire | protocol test | Count and spacing of messages in a window of 10 s, against the bounds of the standard. |
| [Route expiry after a silent neighbor (RIP version 2)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-rip-version-2) | wire | protocol test | The instant of one message against the instant of another, 180 s apart, with a window of 6 s. |
| [Route expiry after a silent neighbor (RIPng)](../../protocol/rip/checks/route-expiry.md#route-expiry-after-a-silent-neighbor-ripng) | wire | protocol test | The instant of one message against the instant of another, 180 s apart, with a window of 6 s. |
| [Garbage collection after an expiry (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-rip-version-2) | wire | protocol test | Presence and absence of an entry over 120 s after a withdrawal, with 1 s of margin. |
| [Garbage collection after an expiry (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-after-an-expiry-ripng) | wire | protocol test | Presence and absence of an entry over 120 s after a withdrawal, with 1 s of margin. |
| [Garbage collection of a lost network (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-rip-version-2) | wire | protocol test | The same, at the router that lost the network. |
| [Garbage collection of a lost network (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-of-a-lost-network-ripng) | wire | protocol test | The same, at the router that lost the network. |
| [Garbage collection while the next hop still withdraws (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-rip-version-2) | wire | protocol test | The same, at the router whose next hop withdrew the route. |
| [Garbage collection while the next hop still withdraws (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-while-the-next-hop-still-withdraws-ripng) | wire | protocol test | The same, at the router whose next hop withdrew the route. |
| [Garbage collection ended by a new route (RIP version 2)](../../protocol/rip/checks/route-expiry.md#garbage-collection-ended-by-a-new-route-rip-version-2) | wire | protocol test | Presence of an entry over a window after a route comes back. |
| [Garbage collection ended by a new route (RIPng)](../../protocol/rip/checks/route-expiry.md#garbage-collection-ended-by-a-new-route-ripng) | wire | protocol test | Presence of an entry over a window after a route comes back. |
| [More than 25 routes (RIP version 2)](../../protocol/rip/checks/response-contents.md#more-than-25-routes-rip-version-2) | wire | protocol test | Entry counts of the messages of one update, and their union. |
| [Messages limited by the MTU (RIPng)](../../protocol/rip/checks/response-contents.md#messages-limited-by-the-mtu-ripng) | wire | protocol test | The same, against the MTU of the link. |

### The checks with a time window, and why they are not statistical tests

Seven kinds of check compare instants: the periodic interval, the triggered update within
6 s, the rate of triggered updates, the expiry at 180 s, and the three garbage collections of
120 s. Each one tests a bound that the standard gives as a fixed figure — 30 s with an offset
of at most 5 or 15 s, 180 s, 120 s, 1 to 5 s — and one run can break a bound. None of them
tests the shape of a distribution. That half of RFC2453-TIMER-2, RFC2080-TIMER-2,
RFC2453-TRIG-3 and RFC2080-TRIG-2, the randomness of the offset and of the timer, is a
statistical test, and the ledger records it as `later`.

The expiry check needed one decision that a statistical test would have made for it: the start
of R2 is fixed 15 s after R1, so that the expiry falls between two periodic updates of R2.
With the default random start, the first run of the check passed by chance; see
[gap 4](results.md#gap-4-defect--the-timeout-is-looked-at-only-when-an-update-is-sent).

### The checks that read the header, not the octets

The field checks read the RIP header and the entries as the message holds them, not as bytes.
The model has a serializer for RIP version 2 only, so a byte-level check of RIPng is not
possible at all; for RIP version 2 the bit layout is a serializer test (below). A field read
from the message — a command, a version, a metric, a prefix length — is still a `wire`
observation: it is what the receiver of the message acts on.

## The statements that belong in another suite

| Statements | Category | Why |
| --- | --- | --- |
| RFC2453-MSG-6, MSG-8, MSG-11; RFC2080-MSG-6, MSG-7, MSG-9, NH-1, NH-2, NH-3 | serializer unit test | the bit layout of the header, of an entry and of a RIPng next hop entry; the model has no RIPng serializer, so the RIPng rows wait for one |
| the random halves of RFC2453-TIMER-2, TRIG-3; RFC2080-TIMER-2, TRIG-2 | statistical test | the distribution of the offset and of the triggered-update timer |
| RFC2453-ADDR-8 | a test of the forwarding of IPv4 | the most specific route is chosen by the forwarding, not by the exchange of routes |

## A category this pass did not need

No check needs a module test: every checked behavior shows in a message on a link or in a
forwarded datagram, and no statement of the in-scope set is `internal` without a message that
shows it. No check needs a fingerprint test: the pass locks behaviors to the standard, not
trajectories to a reference run.
