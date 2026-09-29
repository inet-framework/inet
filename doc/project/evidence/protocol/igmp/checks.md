# IGMP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc9776/catalog.md](../../standard/rfc9776/catalog.md), [rfc2236/catalog.md](../../standard/rfc2236/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow. The procedures come from the specification
only. They name no simulation model and no code. This file holds the common mockups, the
rules every check obeys, and the index. The checks themselves live in one file per feature
group under [`checks/`](checks); the section names are the anchors that the coverage ledger
links to.

## Common mockups

Every link is an Ethernet link. The routers run IGMPv3 unless a mockup says otherwise, with the
default of every timer and counter; the hosts run IGMPv3 too, except where a mockup names an
older host.

| Link | Prefix | Addresses |
| --- | --- | --- |
| L1 | 10.0.1.0/24 | R or R1 10.0.1.1, R2 10.0.1.2, A 10.0.1.10, B 10.0.1.11, C 10.0.1.12 |
| L2 | 10.0.2.0/24 | R 10.0.2.1, S1 10.0.2.10, S2 10.0.2.11 |

The groups are G = 225.0.1.1, H = 225.0.1.2 and K = 225.0.1.3.

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
S2. A source that the check names sends one UDP datagram to its group every 0.5 seconds, from
the start to the end of the run. R forwards multicast traffic from L2 to L1 as IGMP suggests;
"forwarding" in the checks is the arrival of those datagrams on L1.

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

**The two routers.** Routers R1 (10.0.1.1) and R2 (10.0.1.2) and host A on L1.

**The link with an older querier.** The link with a source, but R runs IGMPv2 (RFC 2236), so its
Queries are IGMPv2 Queries.

**The link with an older host.** The link with a source, and a third host C on L1 that runs
IGMPv2 only.

**The two routers of different versions.** The two routers, where R1 runs IGMPv3 and R2 runs
IGMPv2.

### The default values

The checks use the defaults of RFC 9776 §8:

| Value | Default |
| --- | --- |
| Robustness Variable | 2 |
| Query Interval | 125 seconds |
| Query Response Interval | 10 seconds (Max Resp Code 100) |
| Group Membership Interval | 2 × 125 + 2 × 10 = 270 seconds |
| Other Querier Present Interval | 2 × 125 + 0.5 × 10 = 255 seconds |
| Startup Query Interval | 125 / 4 = 31.25 seconds |
| Startup Query Count | 2 |
| Last Member Query Interval | 1 second (Max Resp Code 10) |
| Last Member Query Count | 2 |
| Last Member Query Time | 2 × 1 = 2 seconds |
| Unsolicited Report Interval | 1 second |
| Older Version Querier Present Interval | 2 × 125 + 10 × the Max Response Time of the last older Query |
| Older Host Present Interval | 2 × 125 + 10 = 260 seconds |

## Rules every check obeys

- **An observation names the link and the sender.** "On L1, from A" means the IGMP messages that
  A sends onto L1, as they leave A. The message is read from the frame; the addresses, the TTL
  and the options from the IPv4 header.
- **A join, a leave and a source list are local requests of a host.** "A joins G" is a request
  to receive traffic of G from all sources, EXCLUDE({}); "A joins G for S1" is INCLUDE({S1});
  "A leaves G" ends a request. The interface state of a host is the union of its requests, as
  RFC 9776 §3 defines it.
- **One observation confirms the stimulus.** Each procedure has an observation that shows the
  situation the rule needs — the join, the Query, the leave — before the observation that
  checks the rule.
- **A random delay of the standard is a window, not a value.** Where the standard gives a random
  delay, the check tests its bounds and never a value drawn from it. An answer that leaves less
  than 1 millisecond after the Query that it answers leaves "at the instant of the Query": the
  margin only absorbs the transmission of the Query.
- **Start instants are fixed where a timer is under test**, so that the instant under test does
  not fall on another event of the same node by chance.
- **Every node computes the IGMP checksum** of the messages it sends, because one observation
  reads the value.
- **The fields of the header come last** in a check that also observes behavior, so that a wrong
  field does not keep the behavior from a verdict; a check never puts two rules into one
  observation when one could hide the other.
- **A node joins or leaves L1 through its link.** Where a check needs a node that comes up late
  or stops, the link between the node and L1 is broken until the node joins L1, or breaks when
  the node leaves L1. The node then hears nothing from L1, and L1 hears nothing from the node.
  The restart of a node is outside IGMP, so no check stops a node.
- **One Report starts one query sequence.** Where a check reads the Queries that a
  State-Change Report of a host starts, that host sends each State-Change Report once: its
  Robustness Variable is 1. Each repetition of the Report would start the sequence again
  (RFC 9776 §6.6.3.1).

## Index

| Check | File | Statements |
| --- | --- | --- |
| [Membership Report encapsulation](checks/message-format.md#membership-report-encapsulation) | `checks/message-format.md` | RFC9776-GEN-1, GEN-2, GEN-6, REP-2, REP-4, REP-7, REP-31, REP-34, REP-14, REP-17; covers RFC9776-REP-6, REP-1, REP-13, REP-9) |
| [Membership Query encapsulation](checks/message-format.md#membership-query-encapsulation) | `checks/message-format.md` | RFC9776-GEN-5, QRY-3, QRY-5, QRY-7, QRY-12, QRY-15, QRY-16, QRY-20, QRY-25, QRY-28, VER-3, TIMER-5, QRY-24; covers RFC9776-QRY-1, QRY-2, QRY-9, QRY-19, QRY-22, GEN-1, GEN-2 |
| [Router Alert and precedence on every message](checks/message-format.md#router-alert-and-precedence-on-every-message) | `checks/message-format.md` | RFC9776-GEN-4, GEN-3 |
| [Timer relations in the Query](checks/message-format.md#timer-relations-in-the-query) | `checks/message-format.md` | RFC9776-TIMER-3, should not), TIMER-6), TIMER-26; covers RFC9776-TIMER-2 |
| [Join reported at once](checks/host-reports.md#join-reported-at-once) | `checks/host-reports.md` | RFC9776-HOST-4, HOST-6, HOST-10, REP-8, REP-10, REP-11, REP-24; covers RFC9776-HOST-7, REP-22, HOST-1, HOST-3 |
| [Join repeated](checks/host-reports.md#join-repeated) | `checks/host-reports.md` | RFC9776-HOST-13, HOST-18, TIMER-18, HOST-21), HOST-24); covers RFC9776-TIMER-2 |
| [Leave reported](checks/host-reports.md#leave-reported) | `checks/host-reports.md` | RFC9776-HOST-11, REP-23, HOST-23) |
| [Source list change](checks/host-reports.md#source-list-change) | `checks/host-reports.md` | RFC9776-HOST-8, HOST-12, HOST-27, REP-27, REP-28, HOST-22), HOST-26); covers RFC9776-REP-26, REP-12 |
| [Change inside EXCLUDE mode](checks/host-reports.md#change-inside-exclude-mode) | `checks/host-reports.md` | RFC9776-HOST-9 |
| [A change during the repetitions](checks/host-reports.md#a-change-during-the-repetitions) | `checks/host-reports.md` | RFC9776-HOST-14, HOST-15, HOST-16, HOST-17, HOST-25) |
| [Response to a General Query](checks/query-response.md#response-to-a-general-query) | `checks/query-response.md` | RFC9776-HQRY-1, HQRY-3, HQRY-5, HQRY-10, REP-18, REP-19, REP-20, HQRY-12; covers RFC9776-HQRY-2), HQRY-11 |
| [Response to a Group-Specific Query](checks/query-response.md#response-to-a-group-specific-query) | `checks/query-response.md` | RFC9776-HQRY-6, HQRY-13; covers RFC9776-HQRY-4 |
| [Response to a Group-and-Source-Specific Query](checks/query-response.md#response-to-a-group-and-source-specific-query) | `checks/query-response.md` | RFC9776-HQRY-7, HQRY-14, HQRY-15, HQRY-17; covers RFC9776-HQRY-9, HQRY-18 |
| [General Queries at startup and after it](checks/router-queries.md#general-queries-at-startup-and-after-it) | `checks/router-queries.md` | RFC9776-RQ-6, TIMER-4, TIMER-11, TIMER-12, RFC2236-ROUTER-8, ROUTER-9, ROUTER-10; covers RFC9776-RQ-1, RFC2236-ROUTER-3, ROUTER-6, ROUTER-14) |
| [Querier election](checks/router-queries.md#querier-election) | `checks/router-queries.md` | RFC9776-RQRY-5, RQRY-6, RFC2236-ROUTER-11, ROUTER-12, ROUTER-13, RFC9776-RQRY-7), TIMER-10; covers RFC9776-TIMER-9), RFC2236-ROUTER-1, ROUTER-2, ROUTER-4, ROUTER-5, ROUTER-7 |
| [Forwarding after a join](checks/router-state.md#forwarding-after-a-join) | `checks/router-state.md` | RFC9776-FWD-8, RREP-22; covers RFC9776-FWD-1), RST-3, RST-4, RQ-2 |
| [Source-specific forwarding](checks/router-state.md#source-specific-forwarding) | `checks/router-state.md` | RFC9776-FWD-3, FWD-5, RREP-20; covers RFC9776-RST-6), RST-8, RST-13, RST-1, RST-2, RST-11, RST-12 |
| [Membership timeout without a leave](checks/router-state.md#membership-timeout-without-a-leave) | `checks/router-state.md` | RFC9776-RREP-11, RST-10, TIMER-8; covers RFC9776-RREP-5, RREP-6, RST-7, RST-9, TIMER-7) |
| [Leave and the last member query](checks/router-state.md#leave-and-the-last-member-query) | `checks/router-state.md` | RFC9776-RQ-7, RQRY-1, RQRY-10), RQ-8, RREP-27, RQRY-3, RQRY-11, RREP-18, QRY-8, QRY-26, QRY-29, TIMER-13, TIMER-16; covers RFC9776-RSW-1, RSW-2, RREP-7, RREP-15, RREP-17, TIMER-17), RQRY-9) |
| [Leave with another member](checks/router-state.md#leave-with-another-member) | `checks/router-state.md` | RFC9776-RREP-16 |
| [Source blocked while another member wants it](checks/router-state.md#source-blocked-while-another-member-wants-it) | `checks/router-state.md` | RFC9776-RQ-9, RREP-14, RREP-21, RQRY-14, QRY-27, RQRY-13); covers RFC9776-RREP-13, RQRY-15, RQ-10, RQRY-12) |
| [Source blocked by its only member](checks/router-state.md#source-blocked-by-its-only-member) | `checks/router-state.md` | RFC9776-RQRY-2, RREP-12); covers RFC9776-FWD-4, RST-14 |
| [A router that comes up after a join](checks/router-state.md#a-router-that-comes-up-after-a-join) | `checks/router-state.md` | RFC9776-RREP-9 |
| [Host in IGMPv2 mode](checks/compatibility.md#host-in-igmpv2-mode) | `checks/compatibility.md` | RFC9776-COMPH-1, GEN-8, VER-2, COMPH-6, COMPH-17, REP-35, RFC2236-HOST-15, HOST-21, HOST-22, HOST-25, HOST-28, HOST-31; covers RFC9776-COMPH-2, COMPH-3, COMPH-4, COMPH-7, COMPH-13, COMPH-16, COMPH-21, TIMER-20, RFC2236-HOST-1, HOST-2, HOST-3, HOST-6, HOST-9, HOST-32, RFC9776-COMPH-15), RFC2236-HOST-4, HOST-13) |
| [Report suppression in IGMPv2 mode](checks/compatibility.md#report-suppression-in-igmpv2-mode) | `checks/compatibility.md` | RFC2236-HOST-29; covers RFC2236-HOST-10, HOST-12, HOST-19, HOST-20, HOST-24 |
| [Leave in IGMPv2 mode](checks/compatibility.md#leave-in-igmpv2-mode) | `checks/compatibility.md` | RFC9776-GEN-9, RFC2236-HOST-18, HOST-27; covers RFC2236-HOST-5), HOST-26 |
| [Leave at an IGMPv2 router](checks/compatibility.md#leave-at-an-igmpv2-router) | `checks/compatibility.md` | RFC2236-ROUTER-27, ROUTER-28, ROUTER-32, ROUTER-35, ROUTER-39, ROUTER-43, ROUTER-44; covers RFC2236-ROUTER-15, ROUTER-16, ROUTER-18, ROUTER-19, ROUTER-22, ROUTER-24, ROUTER-25, ROUTER-30, ROUTER-33, ROUTER-34, ROUTER-38 |
| [Host back to IGMPv3](checks/compatibility.md#host-back-to-igmpv3) | `checks/compatibility.md` | RFC9776-COMPH-9, TIMER-22; covers RFC9776-COMPH-10, COMPH-12, TIMER-19, TIMER-21 |
| [Router with an IGMPv2 member](checks/compatibility.md#router-with-an-igmpv2-member) | `checks/compatibility.md` | RFC9776-COMPR-13, COMPR-17, COMPR-27, COMPR-28; covers RFC9776-RQ-5, GEN-8, GEN-9, COMPR-14, COMPR-15, COMPR-18, COMPR-22, COMPR-23, COMPR-26, TIMER-23, TIMER-24, COMPR-25 |
| [Querier with an IGMPv2 router](checks/compatibility.md#querier-with-an-igmpv2-router) | `checks/compatibility.md` | RFC9776-RQRY-8, COMPR-1, COMPR-6, COMPR-8; covers RFC9776-COMPR-2 |
| [Querier configured in IGMPv1 mode](checks/compatibility.md#querier-configured-in-igmpv1-mode) | `checks/compatibility.md` | RFC9776-COMPR-2, COMPR-3, COMPH-18; covers RFC9776-COMPH-14, COMPH-19 |
| [A mode change cancels the pending reports](checks/compatibility.md#a-mode-change-cancels-the-pending-reports) | `checks/compatibility.md` | RFC9776-COMPH-22 |
| [A BLOCK record for a group in IGMPv2 mode](checks/compatibility.md#a-block-record-for-a-group-in-igmpv2-mode) | `checks/compatibility.md` | RFC9776-COMPR-29 |

## Statements this pass wrote no check for

Level 2 asks for a core check of every mandatory feature, and the checks above give one to every
mandatory feature except message validation, whose statements all need a crafted message. The
statements below have no check in this pass. This section says **what a check would need**. It
says nothing about the simulation model: whether the absence of a check is acceptable is a
judgment about what the model claims, and that judgment lives in the coverage ledger.

**A crafted message**, the toolset of level 3: every validity rule of a received message —
RFC9776-GEN-10, QRY-6, QRY-23, QRY-30, REP-3, REP-5, REP-15, REP-16, REP-30, REP-33, REP-36,
VER-4, HOST-5 and RFC2236-HOST-7, HOST-8, HOST-11, HOST-14, ROUTER-20, ROUTER-23; an IGMPv2
Group-Specific Query that reaches a host in IGMPv3 mode (RFC9776-COMPH-11); a Report of IGMPv3 in
an order that normal members do not produce, a TO_IN record at a router in INCLUDE mode
(RFC9776-RREP-23); a Leave Group message at a router in IGMPv1 mode, which no host in IGMPv1 mode
sends (RFC9776-COMPR-4).

**A version 1 system**, level 5 by the standards map: a host or a router of RFC 1112 and every
rule that only an IGMPv1 message reaches — RFC9776-GEN-7, VER-1, COMPH-5, COMPH-8, COMPH-20,
COMPR-16, COMPR-19, COMPR-24, COMPR-31 to COMPR-33, and RFC2236-HOST-16, HOST-33 to HOST-40,
ROUTER-17, ROUTER-21, ROUTER-26, ROUTER-31, ROUTER-36, ROUTER-40, ROUTER-42, ROUTER-45 to
ROUTER-47. An IGMPv3 router configured in IGMPv1 mode (RFC9776-COMPR-2) sends the
IGMPv1 Query on the ordinary link, so the check
[Querier configured in IGMPv1 mode](checks/compatibility.md#querier-configured-in-igmpv1-mode)
reads RFC9776-COMPR-3, COMPH-14, COMPH-18 and COMPH-19 without a version 1 system.

**SSM-aware systems**, level 5 by the standards map (RFC 4604): RFC9776-REP-21, REP-25, RREP-1 to
RREP-4, COMPH-23 to COMPH-25, COMPH-27, COMPR-11, COMPR-12.

**State inside a node**, the toolset of level 4: a variable that no message shows
(RFC9776-HOST-2, RQ-3, RQ-4) and a warning or a log entry (RFC9776-COMPR-5, COMPR-7, COMPR-9,
COMPR-10).

**A permission that no observation can fail**: a router may forward excluded sources onto a
transit network (RFC9776-FWD-2); a host may let an older Report suppress its own record
(RFC9776-COMPH-26); a host may skip the Leave when another host reported last
(RFC2236-HOST-17).

**A larger mockup or a second message path**, work for a later level 2 pass:

- a value above the plain range of a code field: a Max Response Time above 12.7 seconds or a
  Query Interval above 127 seconds, for the floating-point codes (RFC9776-QRY-4, QRY-17,
  TIMER-14, TIMER-15), and a Robustness Variable above 7 (QRY-13);
- systems with non-default values, and a non-querier that adopts the QRV and the QQIC of the
  querier (RFC9776-TIMER-1, QRY-14, QRY-18);
- a second router that hears a Query with the S flag set (RFC9776-QRY-10, QRY-11, RQRY-4);
- many sources or many groups, beyond the size of one message (RFC9776-QRY-21, REP-37 to
  REP-39);
- a system that reports before it has an address (RFC9776-REP-32);
- a change that allows and blocks sources at once (RFC9776-REP-29), and a filter-mode change
  followed by a source change within the repetitions (HOST-19, HOST-20);
- two specific Queries for one group within one Max Response Time, or two changes within one
  query period (RFC9776-HQRY-8, RREP-19), and a second IGMPv2 member that answers the Query after
  a Leave (RFC2236-ROUTER-41), or a second Query within the delay of a host (HOST-23, HOST-30);
- a system in EXCLUDE mode with blocked sources, which puts source records into the EXCLUDE state
  of the router (RFC9776-HQRY-16, RST-5, RST-15 to RST-17, FWD-6, FWD-7, RREP-10, RREP-24 to
  RREP-26, COMPR-30);
- a run longer than the Group Membership Interval with an INCLUDE-mode member that answers the
  Queries (RFC9776-RREP-8);
- the end of the IGMPv2 mode of a group after the Older Host Present Interval (RFC9776-COMPR-20,
  COMPR-21, TIMER-25);
- an IGMPv2 group that times out, and two IGMPv2 routers, for the non-querier state machine
  (RFC2236-ROUTER-29, ROUTER-37, ROUTER-48 to ROUTER-54).

The statements that no check targets because a check above shows them are in the ledger as
`covered`, with the check that shows them.
