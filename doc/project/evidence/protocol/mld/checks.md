# MLD — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc9777/catalog.md](../../standard/rfc9777/catalog.md), [rfc2710/catalog.md](../../standard/rfc2710/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
group under [`checks/`](checks); the section names are the anchors that the coverage ledger
links to.

The checks are the twins of the IGMP checks of
[`igmp/checks.md`](../igmp/checks.md): MLDv2 is the IPv6 form of IGMPv3, and MLDv1 of IGMPv2.
Where the two protocols agree, the checks agree, so that a difference in their verdicts is a
difference of the models.

## Common mockups

Every link is an Ethernet link. The routers run MLDv2 unless a mockup says otherwise, with the
default of every timer and counter; the hosts run MLDv2 too, except where a mockup names an
older host. Every node forms its link-local addresses from its interface identifiers.

| Link | Prefix | Global addresses |
| --- | --- | --- |
| L1 | 2001:db8:1::/64 | A 2001:db8:1::10, B 2001:db8:1::11, C 2001:db8:1::12 |
| L2 | 2001:db8:2::/64 | S1 2001:db8:2::10, S2 2001:db8:2::11 |

The multicast addresses are G = ff0e::1:1, H = ff0e::1:2 and K = ff0e::1:3, of global scope and
outside the SSM range.

**The link.** Router R and hosts A and B on L1, through a switch. R is the only router, so it is
the querier.

```
        R
        |
  ------+------ L1
    |       |
    A       B
```

**The link with a source.** The link, and a second interface of R on L2 with the sources S1 and
S2. A source that the check names sends one UDP datagram to its multicast address every 0.5
seconds, from the start to the end of the run. R forwards multicast traffic from L2 to L1 as MLD
suggests; "forwarding" in the checks is the arrival of those datagrams on L1.

```
  S1   S2
   |    |
 --+----+-- L2
      |
      R
      |
 --+--+--+-- L1
   |     |
   A     B
```

**The two routers.** Routers R1 and R2 and host A on L1. The interface identifier of R1 on L1 is
lower than that of R2, so the link-local address of R1 is the lower one (RFC9777-RQRY-11).

**The link with an older querier.** The link with a source, but R runs MLDv1 (RFC 2710), so its
Queries are MLDv1 Queries.

**The link with an older host.** The link with a source, and a third host C on L1 that runs
MLDv1 only.

**The two routers of different versions.** The two routers, where R1 runs MLDv2 and R2 runs
MLDv1.

### The default values

The checks use the defaults of RFC 9777 §9:

| Value | Default |
| --- | --- |
| Robustness Variable | 2 |
| Query Interval | 125 seconds |
| Query Response Interval | 10 seconds (Maximum Response Code 10000) |
| Multicast Address Listening Interval | 2 × 125 + 2 × 10 = 270 seconds |
| Other Querier Present Interval | 2 × 125 + 0.5 × 10 = 255 seconds |
| Startup Query Interval | 125 / 4 = 31.25 seconds |
| Startup Query Count | 2 |
| Last Listener Query Interval | 1 second (Maximum Response Code 1000) |
| Last Listener Query Count | 2 |
| Last Listener Query Time | 2 × 1 = 2 seconds |
| Unsolicited Report Interval | 1 second |
| Older Version Querier Present Interval | 2 × 125 + 10 = 260 seconds |
| Older Version Host Present Interval | 2 × 125 + 10 = 260 seconds |

## Rules every check obeys

- **An observation names the link and the sender.** "On L1, from A" means the MLD messages that
  A sends onto L1, as they leave A. The message is read from the frame; the addresses, the Hop
  Limit and the options from the IPv6 header and its extension headers.
- **A start of listening, a stop and a source list are local requests of a host.** "A starts to
  listen to G" is a request to receive traffic of G from all sources, EXCLUDE({}); "A listens to
  G for S1" is INCLUDE({S1}); "A stops" ends a request. The listening state of an interface is
  the union of its requests, as RFC 9777 §4 defines it.
- **One observation confirms the stimulus.** Each procedure has an observation that shows the
  situation the rule needs — the start of listening, the Query, the stop — before the observation
  that checks the rule.
- **A random delay of the standard is a window, not a value.** Where the standard gives a random
  delay, the check tests its bounds and never a value drawn from it. An answer that leaves less
  than 1 millisecond after the Query that it answers leaves "at the instant of the Query": the
  margin only absorbs the transmission of the Query.
- **Start instants are fixed where a timer is under test**, so that the instant under test does
  not fall on another event of the same node by chance.
- **The fields of the header come last** in a check that also observes behavior, so that a wrong
  field does not keep the behavior from a verdict; a check never puts two rules into one
  observation when one could hide the other.
- **A node joins or leaves L1 through its link.** Where a check needs a node that comes up late
  or stops, the link between the node and L1 is broken until the node joins L1, or breaks when
  the node leaves L1. The node then hears nothing from L1, and L1 hears nothing from the node.
  The restart of a node is outside MLD, so no check stops a node.
- **One Report starts one query sequence.** Where a check reads the Queries that a
  State-Change Report of a host starts, that host sends each State-Change Report once: its
  Robustness Variable is 1. Each repetition of the Report would start the sequence again
  (RFC 9777 §7.6.3).

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Multicast Listener Report encapsulation](checks/message-format.md#multicast-listener-report-encapsulation) | `checks/message-format.md` | RFC9777-GEN-1, GEN-6, REP-2, REP-4, REP-7, REP-40, GEN-2, GEN-3, REP-15, REP-34, REP-19; covers RFC9777-REP-1, REP-5, REP-8, REP-11, REP-13, REP-14, REP-10) |
| [Multicast Listener Query encapsulation](checks/message-format.md#multicast-listener-query-encapsulation) | `checks/message-format.md` | RFC9777-GEN-5, QRY-2, QRY-4, QRY-7, QRY-8, QRY-10, QRY-13, QRY-15, QRY-17, QRY-23, QRY-28, TIMER-5, VER-1, QRY-22, QRY-26, RQRY-10; covers RFC9777-QRY-1, QRY-5, QRY-19 |
| [Router Alert on every message](checks/message-format.md#router-alert-on-every-message) | `checks/message-format.md` | RFC9777-GEN-4; covers RFC9777-GEN-2, GEN-3 |
| [Timer relations in the Query](checks/message-format.md#timer-relations-in-the-query) | `checks/message-format.md` | RFC9777-TIMER-3; should not), TIMER-6), TIMER-20; covers RFC9777-TIMER-2 |
| [Start of listening reported at once](checks/listener-reports.md#start-of-listening-reported-at-once) | `checks/listener-reports.md` | RFC9777-LSN-3, LSN-5, LSN-6, LSN-10, REP-9, REP-11, REP-12, REP-25, REP-27, LSN-23); covers RFC9777-LSN-1, LSN-2, LSN-7, RQ-10 |
| [Start of listening repeated](checks/listener-reports.md#start-of-listening-repeated) | `checks/listener-reports.md` | RFC9777-LSN-13, LSN-18, TIMER-15, LTIM-12), LTIM-16); covers RFC9777-LSN-19, LTIM-13, TIMER-2 |
| [Stop of listening reported](checks/listener-reports.md#stop-of-listening-reported) | `checks/listener-reports.md` | RFC9777-LSN-11, REP-26, LSN-22, LTIM-15) |
| [Source list change](checks/listener-reports.md#source-list-change) | `checks/listener-reports.md` | RFC9777-LSN-8, LSN-12, LTIM-19, REP-30, REP-31, LSN-25, LTIM-17, LTIM-18), LTIM-14); covers RFC9777-REP-29 |
| [Change inside EXCLUDE mode](checks/listener-reports.md#change-inside-exclude-mode) | `checks/listener-reports.md` | RFC9777-LSN-9 |
| [A change during the repetitions](checks/listener-reports.md#a-change-during-the-repetitions) | `checks/listener-reports.md` | RFC9777-LSN-14, LSN-15, LSN-16, LSN-17, LSN-24) |
| [Response to a General Query](checks/query-response.md#response-to-a-general-query) | `checks/query-response.md` | RFC9777-LQRY-2, LQRY-4, LQRY-6, LTIM-1, LTIM-2, REP-20, REP-21, REP-22, REP-23, LTIM-5; covers RFC9777-LQRY-3), LTIM-3, RQ-9, LTIM-4) |
| [Response to a Multicast Address Specific Query](checks/query-response.md#response-to-a-multicast-address-specific-query) | `checks/query-response.md` | RFC9777-LQRY-7, LTIM-6 |
| [Response to a Multicast Address and Source Specific Query](checks/query-response.md#response-to-a-multicast-address-and-source-specific-query) | `checks/query-response.md` | RFC9777-LTIM-7, LTIM-8, LTIM-10; covers RFC9777-LTIM-11 |
| [General Queries at startup and after it](checks/router-queries.md#general-queries-at-startup-and-after-it) | `checks/router-queries.md` | RFC9777-RQ-8, RQRY-7, TIMER-4, TIMER-9, TIMER-10, RFC2710-ROUTER-5, ROUTER-6, ROUTER-7; covers RFC9777-RQ-7, RFC2710-ROUTER-3 |
| [Querier election](checks/router-queries.md#querier-election) | `checks/router-queries.md` | RFC9777-RQRY-6, RQRY-9), RQRY-8, RQRY-11, RFC2710-ROUTER-8, ROUTER-9, ROUTER-10, RFC9777-TIMER-8; covers RFC2710-ROUTER-1, ROUTER-4, ROUTER-33, TIMER-10), TIMER-11 |
| [Forwarding after a start of listening](checks/router-state.md#forwarding-after-a-start-of-listening) | `checks/router-state.md` | RFC9777-FWD-8, RREP-22; covers RFC9777-FWD-1), RST-4, RST-8, RST-9, RST-11, RQ-2, RQ-3), RQ-4 |
| [Source-specific forwarding](checks/router-state.md#source-specific-forwarding) | `checks/router-state.md` | RFC9777-FWD-3, FWD-5, RREP-20, RST-7; covers RFC9777-RST-1, RST-2, RST-3, RST-5, RST-6, RST-23 |
| [Listening timeout without a stop](checks/router-state.md#listening-timeout-without-a-stop) | `checks/router-state.md` | RFC9777-RREP-19, RST-15, RST-18, RSW-1, RSW-3, TIMER-7; covers RFC9777-RST-13, RST-14, RST-17, RST-20 |
| [Stop of listening and the last listener query](checks/router-state.md#stop-of-listening-and-the-last-listener-query) | `checks/router-state.md` | RFC9777-RREP-27, RQ-12, RQ-13, RQRY-14, QRY-11, QRY-24, QRY-29, RREP-8, RREP-10, RREP-14, TIMER-11, TIMER-13, TIMER-14, RQ-11, RQRY-12, RQRY-13), RREP-12); covers RFC9777-RQ-14, RQRY-4, RREP-15, TIMER-12, RST-21, RST-22) |
| [Stop of listening with another listener](checks/router-state.md#stop-of-listening-with-another-listener) | `checks/router-state.md` | RFC9777-RREP-9; covers RFC9777-RREP-7 |
| [Source blocked while another listener wants it](checks/router-state.md#source-blocked-while-another-listener-wants-it) | `checks/router-state.md` | RFC9777-RREP-5, RQRY-15, RQRY-16), RREP-11, RREP-21, RQ-15, RQ-16, RQRY-17, RQRY-18, RQRY-19, QRY-25, RST-28; covers RFC9777-RQ-17, RQRY-20, RST-26, RST-27, RREP-6, RREP-7, RREP-13, RREP-16 |
| [Source blocked by its only listener](checks/router-state.md#source-blocked-by-its-only-listener) | `checks/router-state.md` | RFC9777-RQRY-2), RQRY-3, RST-25, RST-29, FWD-4 |
| [A router that comes up after a start of listening](checks/router-state.md#a-router-that-comes-up-after-a-start-of-listening) | `checks/router-state.md` | RFC9777-RREP-17 |
| [Listener in MLDv1 mode](checks/compatibility.md#listener-in-mldv1-mode) | `checks/compatibility.md` | RFC9777-COMPL-1, COMPL-4, COMPL-9, REP-41, RFC2710-NODE-10, NODE-13, NODE-14, NODE-17, NODE-20, NODE-22, RFC9777-GEN-7); covers RFC9777-VER-1, COMPL-3, COMPL-5, COMPL-6, COMPL-8, COMPL-10, TIMER-16, RFC2710-NODE-1, NODE-4, NODE-15, NODE-16, NODE-24, TIMER-5, TIMER-6, TIMER-12, TIMER-13, RFC9777-COMPL-2, RFC2710-NODE-3, TIMER-7) |
| [Report suppression in MLDv1 mode](checks/compatibility.md#report-suppression-in-mldv1-mode) | `checks/compatibility.md` | RFC2710-NODE-21; covers RFC2710-NODE-7, NODE-8 |
| [Done in MLDv1 mode](checks/compatibility.md#done-in-mldv1-mode) | `checks/compatibility.md` | RFC2710-NODE-12, NODE-19; covers RFC2710-NODE-11 |
| [Done at an MLDv1 router](checks/compatibility.md#done-at-an-mldv1-router) | `checks/compatibility.md` | RFC2710-ROUTER-21, ROUTER-26, ROUTER-28, ROUTER-29, ROUTER-31; covers RFC2710-ROUTER-11, ROUTER-12, ROUTER-15, ROUTER-18, ROUTER-19, ROUTER-20, ROUTER-22, ROUTER-23, ROUTER-24, TIMER-2, TIMER-3, TIMER-14, TIMER-15 |
| [Listener back to MLDv2](checks/compatibility.md#listener-back-to-mldv2) | `checks/compatibility.md` | RFC9777-COMPL-7, TIMER-17 |
| [Router with an MLDv1 listener](checks/compatibility.md#router-with-an-mldv1-listener) | `checks/compatibility.md` | RFC9777-COMPR-12, COMPR-14, COMPR-20, COMPR-21, COMPR-23; covers RFC9777-COMPR-13, COMPR-15, COMPR-16, COMPR-19, TIMER-18 |
| [Querier configured for an MLDv1 router](checks/compatibility.md#querier-configured-for-an-mldv1-router) | `checks/compatibility.md` | RFC9777-COMPR-1, COMPR-3, COMPR-5, COMPR-7, COMPR-4); covers RFC9777-COMPR-2) |
| [A mode change cancels the pending reports](checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | `checks/compatibility.md` | RFC9777-COMPL-11 |
| [A BLOCK record for an address in MLDv1 mode](checks/compatibility.md#a-block-record-for-an-address-in-mldv1-mode) | `checks/compatibility.md` | RFC9777-COMPR-22 |

## Statements this pass wrote no check for

Level 2 asks for a core check of every mandatory feature, and the checks above give one to every
mandatory feature except message validation, whose statements all need a crafted message. The
statements below have no check in this pass. This section says **what a check would need**. It
says nothing about the simulation model: whether the absence of a check is acceptable is a
judgment about what the model claims, and that judgment lives in the coverage ledger.

**A crafted message**, the toolset of level 3: every validity rule of a received message —
RFC9777-GEN-8, QRY-3, QRY-6, QRY-9, QRY-20, QRY-21, QRY-27, QRY-30, REP-3, REP-6, REP-16 to
REP-18, REP-33, REP-35, REP-36, REP-42, LSN-4, LQRY-1, RREP-1, RQRY-1, VER-2 and RFC2710-NODE-2,
NODE-6, NODE-9, ROUTER-2, ROUTER-13, ROUTER-14, ROUTER-16; a TO_IN record at a router in INCLUDE
mode, which normal listeners do not produce (RFC9777-RREP-23).

**SSM-aware systems**, level 5 by the standards map (RFC 4604): RFC9777-REP-24, REP-28, RREP-2,
RREP-3, COMPL-12 to COMPL-14, COMPR-10, COMPR-11.

**State inside a node**, the toolset of level 4: a variable or a notation that no message shows
(RFC9777-RQ-1, RQ-5, RQ-6, RREP-4, RST-10, RST-16, RST-19, RFC2710-TIMER-4), and a warning or a
log entry (RFC9777-COMPR-6, COMPR-8, COMPR-9).

**A permission that no observation can fail**: a router may forward excluded sources onto a
transit link (RFC9777-FWD-2); a host may let an MLDv1 Report suppress its own Version 2 Report
(RFC9777-COMPL-15).

**A larger mockup or a second message path**, work for a later level 2 pass:

- systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the
  querier (RFC9777-TIMER-1, QRY-14, QRY-16, RFC2710-TIMER-1);
- a second router that hears a Query with the S flag set (RFC9777-QRY-12, RQRY-5);
- many sources or many addresses, beyond the size of one message (RFC9777-QRY-18, REP-43 to
  REP-45);
- a node that reports before it has a link-local address (RFC9777-REP-37 to REP-39);
- a change that allows and blocks sources at once (RFC9777-REP-32), and a filter-mode change
  followed by a source change within the repetitions (LSN-20, LSN-21);
- two specific Queries for one address within one Maximum Response Delay, or a Query that meets a
  pending response (RFC9777-LQRY-5, LQRY-8, LQRY-9, RFC2710-NODE-23), and a stop of listening in
  the Delaying Listener state (RFC2710-NODE-18);
- a node that hears an MLDv1 Query for an address it does not listen to (RFC2710-NODE-5), and the
  scope rules of MLDv1 for reserved, node-local and link-local addresses (RFC2710-NODE-25,
  NODE-26);
- a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state
  of the router (RFC9777-LTIM-9, RST-12, RST-30 to RST-36, FWD-6, FWD-7, RREP-18, RREP-24 to
  RREP-26, RSW-2);
- a run longer than the Multicast Address Listening Interval with an INCLUDE-mode listener that
  answers the Queries (RFC9777-RST-24);
- the end of the MLDv1 mode of an address after the Older Version Host Present Interval
  (RFC9777-COMPR-17, COMPR-18, TIMER-19);
- an MLDv1 address that times out, and two MLDv1 routers, for the non-querier state machine
  (RFC2710-ROUTER-17, ROUTER-25, ROUTER-27, ROUTER-30, ROUTER-32, ROUTER-34 to ROUTER-39,
  TIMER-8, TIMER-9), and the Unsolicited Report Interval of an MLDv1-only node
  (RFC2710-TIMER-16).

The statements that no check targets because a check above shows them are in the ledger as
`covered`, with the check that shows them.
