# IGMP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `IGMP-F-*` · **Stands on:** [standards.md](standards.md), [rfc9776/catalog.md](../../standard/rfc9776/catalog.md), [rfc2236/catalog.md](../../standard/rfc2236/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and per
document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of
each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/igmp/coverage.md), and step 8 compares it with the claims of the
  model in [`conformance.md`](../../model/igmp/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/igmp/coverage.md).

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when the
mechanism is the only path the document gives to a state or an outcome; `optional` when every
core statement says `may` or `should`; `unstated` otherwise.

One refinement, which the DHCP, RIP and ND maps state too: **a conditional keyword does not
raise the level of a feature.** RFC 9776 says that an SSM-aware router "SHOULD ignore" some
records; the rule holds only once a router is SSM-aware, which the in-scope set does not require.
The level comes from the statement that decides whether the mechanism has to exist at all, and
each feature names it.

A second refinement, new in this map: **a keyword of the base document can make a companion
behavior mandatory.** RFC 9776 says that an IGMPv3 host and router "MUST" be able to operate in
IGMPv2 compatibility mode (RFC9776-COMPH-1, COMPR-13), and RFC 2236 describes that mode with state
diagrams that have no keyword of their own. The two version 2 features take their level from
the keyword of RFC 9776, and say so.

## Index

| ID | Feature |
| --- | --- |
| [IGMP-F-MESSAGE-FORMAT](#igmp-f-message-format) | Every IGMP message is an IPv4 datagram of protocol 2, with TTL 1 and a Router Alert option, and carries its type and a checksum. |
| [IGMP-F-QUERY-FORMAT](#igmp-f-query-format) | A Membership Query carries the Max Resp Code, the group address, the S flag, QRV, QQIC and a source list, and its variant decides its destination. |
| [IGMP-F-REPORT-FORMAT](#igmp-f-report-format) | A Version 3 Membership Report carries Group Records of six types, each with a multicast address and a source list, and goes to 224.0.0.22. |
| [IGMP-F-MESSAGE-VALIDATION](#igmp-f-message-validation) | A system verifies the checksum, ignores reserved fields, extra octets, auxiliary data and unknown types, and accepts a message to any of its own addresses. |
| [IGMP-F-STATE-CHANGE-REPORT](#igmp-f-state-change-report) | A change of the reception state of an interface makes the system send a State-Change Report at once, with the records that the old and the new state give. |
| [IGMP-F-REPORT-RETRANSMISSION](#igmp-f-report-retransmission) | A system sends each State-Change Report [Robustness Variable] times in all, and merges a later change into the pending retransmissions. |
| [IGMP-F-QUERY-RESPONSE](#igmp-f-query-response) | A system answers a Query after a random delay within the Max Response Time, with Current-State Records for all its groups or for the queried group and sources. |
| [IGMP-F-GENERAL-QUERY](#igmp-f-general-query) | The querier sends General Queries to 224.0.0.1 every Query Interval, and more often at startup. |
| [IGMP-F-QUERIER-ELECTION](#igmp-f-querier-election) | Among the routers of a network the one with the lowest IP address is the querier; the others stay silent until the querier stops. |
| [IGMP-F-GROUP-MEMBERSHIP](#igmp-f-group-membership) | A router keeps for each group a filter mode, a Group Timer and source records, sets them from Current-State Records, and times the membership out after the Group Membership Interval. |
| [IGMP-F-FORWARDING](#igmp-f-forwarding) | A router suggests the forwarding of each source from its group state: INCLUDE forwards the listed sources, EXCLUDE all but the blocked ones. |
| [IGMP-F-STATE-CHANGE-PROCESSING](#igmp-f-state-change-processing) | A router applies Filter-Mode-Change and Source-List-Change Records to its state, and queries the sources and groups that a system asked to stop. |
| [IGMP-F-SPECIFIC-QUERIES](#igmp-f-specific-queries) | Before it deletes a group or sources, the querier sends [Last Member Query Count] Group-Specific or Group-and-Source-Specific Queries, [Last Member Query Interval] apart, to the group address. |
| [IGMP-F-QUERY-TIMER-UPDATES](#igmp-f-query-timer-updates) | A router lowers its timers when it sends or hears a specific query without the S flag, and a non-querier adopts the QRV and QQIC of the querier. |
| [IGMP-F-HOST-COMPATIBILITY](#igmp-f-host-compatibility) | A system falls back to IGMPv1 or IGMPv2 when it hears an older General Query, and returns to IGMPv3 after the Older Version Querier Present Interval. |
| [IGMP-F-VERSION-2-HOST](#igmp-f-version-2-host) | In IGMPv2 mode a system reports a join at once, answers a query after a random delay, suppresses its report when another member reports first, and sends a Leave when it was the last to report. |
| [IGMP-F-ROUTER-COMPATIBILITY](#igmp-f-router-compatibility) | A router uses the lowest IGMP version among the routers of a network, and keeps a compatibility mode for each group from the versions of the reports it hears. |
| [IGMP-F-VERSION-2-ROUTER](#igmp-f-version-2-router) | In IGMPv2 mode a router tracks each group in four states and answers a Leave with Group-Specific Queries before it removes the group. |
| [IGMP-F-SSM-AWARE](#igmp-f-ssm-aware) | An SSM-aware system and router ignore EXCLUDE-mode records and older-version messages for an address of the SSM range. |
| [IGMP-F-TIMER-CONFIGURATION](#igmp-f-timer-configuration) | The timers and counters have their defaults and their relations, and every system of a link uses the same values. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [IGMP-F-MESSAGE-FORMAT](#igmp-f-message-format) | mandatory | RFC 9776 §4, §4.1.2, §4.2.2 | RFC9776-GEN-1, GEN-2, GEN-4, GEN-5, GEN-6, QRY-5, REP-4 |
| [IGMP-F-QUERY-FORMAT](#igmp-f-query-format) | mandatory | RFC 9776 §4.1 | RFC9776-QRY-3, QRY-4, QRY-7, QRY-8, QRY-12, QRY-13, QRY-15, QRY-16, QRY-17, QRY-20, QRY-25, QRY-26, QRY-27, QRY-28, QRY-29 |
| [IGMP-F-REPORT-FORMAT](#igmp-f-report-format) | mandatory | RFC 9776 §4.2 | RFC9776-REP-7, REP-8, REP-10, REP-11, REP-12, REP-14, REP-19, REP-20, REP-23, REP-24, REP-27, REP-28, REP-31, REP-34 |
| [IGMP-F-MESSAGE-VALIDATION](#igmp-f-message-validation) | mandatory | RFC 9776 §4, §4.1, §4.2, §7.1; RFC 2236 §6, §7 | RFC9776-GEN-10, QRY-6, QRY-23, QRY-30, REP-3, REP-5, REP-15, REP-16, REP-30, REP-33, REP-36, VER-4, RFC2236-HOST-7, HOST-11, ROUTER-20, ROUTER-23 |
| [IGMP-F-STATE-CHANGE-REPORT](#igmp-f-state-change-report) | mandatory | RFC 9776 §5, §5.1 | RFC9776-HOST-4, HOST-6, HOST-8, HOST-9, HOST-10, HOST-11, HOST-12 |
| [IGMP-F-REPORT-RETRANSMISSION](#igmp-f-report-retransmission) | mandatory | RFC 9776 §5.1, §8.11 | RFC9776-HOST-13, HOST-14, HOST-16, HOST-18, HOST-21, HOST-22, HOST-23, HOST-24, HOST-25, HOST-26, HOST-27 |
| [IGMP-F-QUERY-RESPONSE](#igmp-f-query-response) | mandatory | RFC 9776 §5.2 | RFC9776-HQRY-1, HQRY-3, HQRY-5, HQRY-6, HQRY-10, HQRY-12, HQRY-13, HQRY-14, HQRY-15, HQRY-16, HQRY-17 |
| [IGMP-F-GENERAL-QUERY](#igmp-f-general-query) | mandatory | RFC 9776 §6, §6.1, §8.2, §8.3, §8.6, §8.7; RFC 2236 §7 | RFC9776-RQ-6, TIMER-4, TIMER-5, TIMER-11, TIMER-12, RFC2236-ROUTER-8, ROUTER-9, ROUTER-10 |
| [IGMP-F-QUERIER-ELECTION](#igmp-f-querier-election) | mandatory | RFC 9776 §6.6.2, §8.5; RFC 2236 §7 | RFC9776-RQRY-5, RQRY-6, RQRY-7, TIMER-10, RFC2236-ROUTER-11, ROUTER-12, ROUTER-13 |
| [IGMP-F-GROUP-MEMBERSHIP](#igmp-f-group-membership) | mandatory | RFC 9776 §6.2, §6.4.1, §6.5, §8.4 | RFC9776-RREP-8, RREP-9, RREP-10, RREP-11, RST-10, RST-14, RSW-1, RSW-2, TIMER-8 |
| [IGMP-F-FORWARDING](#igmp-f-forwarding) | mandatory | RFC 9776 §6.3 | RFC9776-FWD-3, FWD-4, FWD-5, FWD-6, FWD-7, FWD-8 |
| [IGMP-F-STATE-CHANGE-PROCESSING](#igmp-f-state-change-processing) | mandatory | RFC 9776 §6.4.2 | RFC9776-RREP-12, RREP-20, RREP-21, RREP-22, RREP-23, RREP-24, RREP-25, RREP-26, RREP-27 |
| [IGMP-F-SPECIFIC-QUERIES](#igmp-f-specific-queries) | mandatory | RFC 9776 §6.1, §6.6.3, §8.8 to §8.10 | RFC9776-RQ-7, RQ-8, RQ-9, RQ-10, RQRY-10, RQRY-11, RQRY-13, RQRY-14, RREP-18, TIMER-13, TIMER-16 |
| [IGMP-F-QUERY-TIMER-UPDATES](#igmp-f-query-timer-updates) | mandatory | RFC 9776 §4.1.5 to §4.1.7, §6.6.1 | RFC9776-RQRY-1, RQRY-2, RQRY-3, RQRY-4, QRY-10 |
| [IGMP-F-HOST-COMPATIBILITY](#igmp-f-host-compatibility) | mandatory | RFC 9776 §4, §7.1, §7.2, §8.12 | RFC9776-VER-1, VER-2, VER-3, COMPH-1, COMPH-5, COMPH-6, COMPH-8, COMPH-9, COMPH-11, COMPH-17, COMPH-18, COMPH-20, COMPH-22, TIMER-22 |
| [IGMP-F-VERSION-2-HOST](#igmp-f-version-2-host) | mandatory | RFC 2236 §6 | RFC2236-HOST-15, HOST-18, HOST-21, HOST-22, HOST-25, HOST-26, HOST-27, HOST-28, HOST-29, HOST-30, HOST-31, HOST-32 |
| [IGMP-F-ROUTER-COMPATIBILITY](#igmp-f-router-compatibility) | mandatory | RFC 9776 §4, §6, §6.6.2, §7.3, §8.13 | RFC9776-GEN-7, GEN-8, GEN-9, RQ-5, RQRY-8, COMPR-1, COMPR-13, COMPR-16, COMPR-17, COMPR-19, COMPR-20, COMPR-27, COMPR-28, COMPR-29, COMPR-30, COMPR-31, COMPR-32, COMPR-33, TIMER-25 |
| [IGMP-F-VERSION-2-ROUTER](#igmp-f-version-2-router) | mandatory | RFC 2236 §7 | RFC2236-ROUTER-27, ROUTER-28, ROUTER-32, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39, ROUTER-40, ROUTER-41, ROUTER-42, ROUTER-43, ROUTER-44, ROUTER-45, ROUTER-46, ROUTER-47 |
| [IGMP-F-SSM-AWARE](#igmp-f-ssm-aware) | optional | RFC 9776 §4.2.13, §6.4, §7.2, §7.3 | RFC9776-REP-21, REP-25, RREP-1, RREP-2, COMPH-27 |
| [IGMP-F-TIMER-CONFIGURATION](#igmp-f-timer-configuration) | mandatory | RFC 9776 §8, §8.1, §8.3, §8.14.2 | RFC9776-TIMER-1, TIMER-3, TIMER-6, TIMER-26 |

## IGMP-F-MESSAGE-FORMAT

**Every IGMP message is an IPv4 datagram of protocol 2, with TTL 1 and a Router Alert option, and carries its type and a checksum.**

- **Sources** — RFC 9776 §4, §4.1.2, §4.2.2.
- **Level** — mandatory (reason: only path). Each message has one encapsulation and no other; RFC 9776 §4 states it for every message it describes.
- **Description** — The Membership Query has type 0x11 and the Version 3 Membership Report type 0x22. The checksum covers the whole IGMP message. A sender adds no octets after the defined fields and sets the Reserved fields to zero.
- **Checks** — core: RFC9776-GEN-1, GEN-2, GEN-4, GEN-5, GEN-6, QRY-5, REP-4. Supporting: RFC9776-GEN-3, QRY-9, QRY-24, REP-2, REP-6, REP-17.

## IGMP-F-QUERY-FORMAT

**A Membership Query carries the Max Resp Code, the group address, the S flag, QRV, QQIC and a source list, and its variant decides its destination.**

- **Sources** — RFC 9776 §4.1.
- **Level** — mandatory (reason: only path). The Query is the one way a router asks for membership, and §4.1 gives its only format.
- **Description** — A General Query has group address zero and no sources and goes to 224.0.0.1. A Group-Specific Query names the group and goes to it; a Group-and-Source-Specific Query also lists sources. Max Resp Code and QQIC use a floating-point code from 128 on. QRV carries the Robustness Variable of the querier, or zero above 7.
- **Checks** — core: RFC9776-QRY-3, QRY-4, QRY-7, QRY-8, QRY-12, QRY-13, QRY-15, QRY-16, QRY-17, QRY-20, QRY-25, QRY-26, QRY-27, QRY-28, QRY-29. Supporting: RFC9776-QRY-1, QRY-2, QRY-19, QRY-21, QRY-22.

## IGMP-F-REPORT-FORMAT

**A Version 3 Membership Report carries Group Records of six types, each with a multicast address and a source list, and goes to 224.0.0.22.**

- **Sources** — RFC 9776 §4.2.
- **Level** — mandatory (reason: keyword). RFC9776-REP-14 has the strength "must".
- **Description** — The record types are MODE_IS_INCLUDE and MODE_IS_EXCLUDE (current state), CHANGE_TO_INCLUDE_MODE and CHANGE_TO_EXCLUDE_MODE (filter-mode change), ALLOW_NEW_SOURCES and BLOCK_OLD_SOURCES (source-list change). A Report leaves from a unicast address of the subnet, or 0.0.0.0 before the system has one. A system sends no auxiliary data, and splits records that do not fit into one message.
- **Checks** — core: RFC9776-REP-7, REP-8, REP-10, REP-11, REP-12, REP-14, REP-19, REP-20, REP-23, REP-24, REP-27, REP-28, REP-31, REP-34. Supporting: RFC9776-REP-1, REP-9, REP-13, REP-18, REP-22, REP-26, REP-29, REP-32, REP-35, REP-37, REP-38, REP-39.

## IGMP-F-MESSAGE-VALIDATION

**A system verifies the checksum, ignores reserved fields, extra octets, auxiliary data and unknown types, and accepts a message to any of its own addresses.**

- **Sources** — RFC 9776 §4, §4.1, §4.2, §7.1; RFC 2236 §6, §7.
- **Level** — mandatory (reason: keyword). RFC9776-GEN-10 has the strength "must".
- **Description** — An unrecognized message type, an unrecognized Group Record type and a Query of an impossible length are silently ignored. Extra octets count in the checksum and are otherwise ignored. A router accepts a Report from 0.0.0.0. RFC 2236 requires at least 8 octets and a correct checksum for each message of the version 2 state machines.
- **Checks** — core: RFC9776-GEN-10, QRY-6, QRY-23, QRY-30, REP-3, REP-5, REP-15, REP-16, REP-30, REP-33, REP-36, VER-4, RFC2236-HOST-7, HOST-11, ROUTER-20, ROUTER-23. Supporting: RFC9776-HOST-5, RFC2236-HOST-14.

## IGMP-F-STATE-CHANGE-REPORT

**A change of the reception state of an interface makes the system send a State-Change Report at once, with the records that the old and the new state give.**

- **Sources** — RFC 9776 §5, §5.1.
- **Level** — mandatory (reason: only path). The State-Change Report is the one way a system tells the routers about a new join or leave.
- **Description** — From INCLUDE to INCLUDE and from EXCLUDE to EXCLUDE the report holds ALLOW and BLOCK records for the difference; a change of filter mode gives one TO_IN or TO_EX record with the new source list. A missing state counts as INCLUDE with no sources, so a join is a change to EXCLUDE({}) and a leave a change to INCLUDE({}). An empty ALLOW or BLOCK record is left out. No report ever names 224.0.0.1.
- **Checks** — core: RFC9776-HOST-4, HOST-6, HOST-8, HOST-9, HOST-10, HOST-11, HOST-12. Supporting: RFC9776-HOST-1, HOST-2, HOST-3, HOST-7.

## IGMP-F-REPORT-RETRANSMISSION

**A system sends each State-Change Report [Robustness Variable] times in all, and merges a later change into the pending retransmissions.**

- **Sources** — RFC 9776 §5.1, §8.11.
- **Level** — mandatory (reason: only path). RFC 9776 §5.1 gives the repetition as the one way a State-Change Report survives the packet loss that the Robustness Variable allows; the lower-case musts of RFC9776-HOST-23 to HOST-26 fix the content of each repetition.
- **Description** — The repetitions come at random times within the Unsolicited Report Interval. A filter-mode change keeps its TO_IN or TO_EX record in the next [Robustness Variable] reports; source changes keep their ALLOW and BLOCK records the same number of times. A new change sends a merged report at once and restarts the count.
- **Checks** — core: RFC9776-HOST-13, HOST-14, HOST-16, HOST-18, HOST-21, HOST-22, HOST-23, HOST-24, HOST-25, HOST-26, HOST-27. Supporting: RFC9776-HOST-15, HOST-17, HOST-19, HOST-20, TIMER-2, TIMER-18.

## IGMP-F-QUERY-RESPONSE

**A system answers a Query after a random delay within the Max Response Time, with Current-State Records for all its groups or for the queried group and sources.**

- **Sources** — RFC 9776 §5.2.
- **Level** — mandatory (reason: keyword). RFC9776-HQRY-12 has the strength "must not".
- **Description** — A General Query schedules one response for the interface; a specific query schedules one for the group, and a Group-and-Source-Specific Query also records the queried sources. The response to a General Query never leaves at once. A Group-and-Source-Specific response reports only the queried sources that the state wants, and an empty record is not sent.
- **Checks** — core: RFC9776-HQRY-1, HQRY-3, HQRY-5, HQRY-6, HQRY-10, HQRY-12, HQRY-13, HQRY-14, HQRY-15, HQRY-16, HQRY-17. Supporting: RFC9776-HQRY-2, HQRY-4, HQRY-7, HQRY-8, HQRY-9, HQRY-11, HQRY-18.

## IGMP-F-GENERAL-QUERY

**The querier sends General Queries to 224.0.0.1 every Query Interval, and more often at startup.**

- **Sources** — RFC 9776 §6, §6.1, §8.2, §8.3, §8.6, §8.7; RFC 2236 §7.
- **Level** — mandatory (reason: only path). The periodic General Query is the one way a router refreshes the membership of a network, RFC 9776 §6.1.
- **Description** — A router starts as querier and sends [Startup Query Count] General Queries [Startup Query Interval] apart, then one every [Query Interval], with the Max Response Time of the Query Response Interval. A router listens to 224.0.0.22 on each interface.
- **Checks** — core: RFC9776-RQ-6, TIMER-4, TIMER-5, TIMER-11, TIMER-12, RFC2236-ROUTER-8, ROUTER-9, ROUTER-10. Supporting: RFC9776-RQ-1, RQ-2, RQ-3, RQ-4, RFC2236-ROUTER-3, ROUTER-6, ROUTER-14.

## IGMP-F-QUERIER-ELECTION

**Among the routers of a network the one with the lowest IP address is the querier; the others stay silent until the querier stops.**

- **Sources** — RFC 9776 §6.6.2, §8.5; RFC 2236 §7.
- **Level** — mandatory (reason: only path). RFC 9776 §6.6.2 gives the one way to have one querier on a network; the must of RFC9776-TIMER-10 fixes its timer.
- **Description** — A router that hears a General Query from a lower address becomes a non-querier and starts the Other-Querier-Present Timer; when the timer expires it becomes the querier again.
- **Checks** — core: RFC9776-RQRY-5, RQRY-6, RQRY-7, TIMER-10, RFC2236-ROUTER-11, ROUTER-12, ROUTER-13. Supporting: RFC9776-TIMER-9, RFC2236-ROUTER-1, ROUTER-2, ROUTER-4, ROUTER-5, ROUTER-7.

## IGMP-F-GROUP-MEMBERSHIP

**A router keeps for each group a filter mode, a Group Timer and source records, sets them from Current-State Records, and times the membership out after the Group Membership Interval.**

- **Sources** — RFC 9776 §6.2, §6.4.1, §6.5, §8.4.
- **Level** — mandatory (reason: only path). The router state is the one place where IGMP keeps what the members want; the must of RFC9776-TIMER-8 fixes its timeout.
- **Description** — IS_IN and IS_EX records move the state as the table of §6.4.1 gives. When the Group Timer of an EXCLUDE group expires, the group switches to INCLUDE with the sources whose timers still run, or goes away. The Group Membership Interval is [Robustness Variable] times [Query Interval] plus twice [Query Response Interval].
- **Checks** — core: RFC9776-RREP-8, RREP-9, RREP-10, RREP-11, RST-10, RST-14, RSW-1, RSW-2, TIMER-8. Supporting: RFC9776-RST-1, RST-2, RST-3, RST-4, RST-5, RST-6, RST-7, RST-8, RST-9, RST-11, RST-12, RST-13, RST-15, RST-16, RST-17, RREP-5, RREP-6, RREP-7, TIMER-7.

## IGMP-F-FORWARDING

**A router suggests the forwarding of each source from its group state: INCLUDE forwards the listed sources, EXCLUDE all but the blocked ones.**

- **Sources** — RFC 9776 §6.3.
- **Level** — mandatory (reason: only path). The forwarding suggestion is the one outcome that IGMP gives to multicast routing, §6.3.
- **Description** — In INCLUDE mode a source with a running timer is forwarded and every other source is not. In EXCLUDE mode a source with a timer at zero is blocked and every other source is forwarded. The multicast routing protocol uses these suggestions.
- **Checks** — core: RFC9776-FWD-3, FWD-4, FWD-5, FWD-6, FWD-7, FWD-8. Supporting: RFC9776-FWD-1, FWD-2.

## IGMP-F-STATE-CHANGE-PROCESSING

**A router applies Filter-Mode-Change and Source-List-Change Records to its state, and queries the sources and groups that a system asked to stop.**

- **Sources** — RFC 9776 §6.4.2.
- **Level** — mandatory (reason: keyword). RFC9776-RREP-12 has the strength "must (lower case)".
- **Description** — ALLOW, BLOCK, TO_IN and TO_EX records move the state as the table of §6.4.2 gives. A record that stops sources or a group makes the router lower their timers to the Last Member Query Time and query them; a record of interest that arrives during the query keeps them.
- **Checks** — core: RFC9776-RREP-12, RREP-20, RREP-21, RREP-22, RREP-23, RREP-24, RREP-25, RREP-26, RREP-27. Supporting: RFC9776-RREP-13, RREP-14, RREP-15, RREP-16, RREP-17, RREP-19.

## IGMP-F-SPECIFIC-QUERIES

**Before it deletes a group or sources, the querier sends [Last Member Query Count] Group-Specific or Group-and-Source-Specific Queries, [Last Member Query Interval] apart, to the group address.**

- **Sources** — RFC 9776 §6.1, §6.6.3, §8.8 to §8.10.
- **Level** — mandatory (reason: keyword). RFC9776-RQ-7 has the strength "must (lower case)".
- **Description** — A leave, a TO_IN or a BLOCK record triggers the queries. The first query leaves at once. A Group-and-Source-Specific Query is split into a message with the S flag set and one with it clear, and a message with no sources is not sent. The S flag is set when the timer is above the Last Member Query Time.
- **Checks** — core: RFC9776-RQ-7, RQ-8, RQ-9, RQ-10, RQRY-10, RQRY-11, RQRY-13, RQRY-14, RREP-18, TIMER-13, TIMER-16. Supporting: RFC9776-RQRY-9, RQRY-12, RQRY-15, TIMER-14, TIMER-15, TIMER-17.

## IGMP-F-QUERY-TIMER-UPDATES

**A router lowers its timers when it sends or hears a specific query without the S flag, and a non-querier adopts the QRV and QQIC of the querier.**

- **Sources** — RFC 9776 §4.1.5 to §4.1.7, §6.6.1.
- **Level** — mandatory (reason: keyword). RFC9776-RQRY-1 has the strength "must (lower case)".
- **Description** — A Group-Specific Query lowers the Group Timer, and a Group-and-Source-Specific Query the Source Timers of its sources, to the Last Member Query Time. The S flag suppresses these updates, but not querier election.
- **Checks** — core: RFC9776-RQRY-1, RQRY-2, RQRY-3, RQRY-4, QRY-10. Supporting: RFC9776-QRY-11, QRY-14, QRY-18.

## IGMP-F-HOST-COMPATIBILITY

**A system falls back to IGMPv1 or IGMPv2 when it hears an older General Query, and returns to IGMPv3 after the Older Version Querier Present Interval.**

- **Sources** — RFC 9776 §4, §7.1, §7.2, §8.12.
- **Level** — mandatory (reason: keyword). RFC9776-COMPH-1 has the strength "must".
- **Description** — The length and the Max Resp Code of a Query tell its version. An older General Query starts the IGMPv1- or IGMPv2-Querier-Present Timer; while a timer runs the system uses only that version, and a change of mode cancels the pending responses. A Max Resp Code of zero means 10 seconds. An older Group-Specific Query does not change the mode.
- **Checks** — core: RFC9776-VER-1, VER-2, VER-3, COMPH-1, COMPH-5, COMPH-6, COMPH-8, COMPH-9, COMPH-11, COMPH-17, COMPH-18, COMPH-20, COMPH-22, TIMER-22. Supporting: RFC9776-GEN-7, GEN-8, GEN-9, COMPH-2, COMPH-3, COMPH-4, COMPH-7, COMPH-10, COMPH-12, COMPH-13, COMPH-14, COMPH-15, COMPH-16, COMPH-19, COMPH-21, COMPH-26, TIMER-19, TIMER-20, TIMER-21.

## IGMP-F-VERSION-2-HOST

**In IGMPv2 mode a system reports a join at once, answers a query after a random delay, suppresses its report when another member reports first, and sends a Leave when it was the last to report.**

- **Sources** — RFC 2236 §6.
- **Level** — mandatory (reason: keyword elsewhere). RFC9776-COMPH-1: an IGMPv3 host must be able to operate in IGMPv2 compatibility mode, and this is the behavior of that mode.
- **Description** — The host state machine of RFC 2236 has the states Non-Member, Delaying Member and Idle Member for each group. A Leave goes to 224.0.0.2, and a host that knows of an IGMPv1 querier sends none. A host never reports 224.0.0.1.
- **Checks** — core: RFC2236-HOST-15, HOST-18, HOST-21, HOST-22, HOST-25, HOST-26, HOST-27, HOST-28, HOST-29, HOST-30, HOST-31, HOST-32. Supporting: RFC2236-HOST-1, HOST-2, HOST-3, HOST-4, HOST-5, HOST-6, HOST-8, HOST-9, HOST-10, HOST-12, HOST-13, HOST-16, HOST-17, HOST-19, HOST-20, HOST-23, HOST-24, HOST-33, HOST-34, HOST-35, HOST-36, HOST-37, HOST-38, HOST-39, HOST-40.

## IGMP-F-ROUTER-COMPATIBILITY

**A router uses the lowest IGMP version among the routers of a network, and keeps a compatibility mode for each group from the versions of the reports it hears.**

- **Sources** — RFC 9776 §4, §6, §6.6.2, §7.3, §8.13.
- **Level** — mandatory (reason: keyword). RFC9776-GEN-7 has the strength "must".
- **Description** — An IGMPv1 or IGMPv2 Report starts the Host-Present Timer of its version for the group, and while it runs the router treats the group in that version: a version 2 Report is IS_EX({}), a Leave is TO_IN({}), and BLOCK records and the source lists of TO_EX are ignored. In IGMPv1 or IGMPv2 mode a router sends queries of that version.
- **Checks** — core: RFC9776-GEN-7, GEN-8, GEN-9, RQ-5, RQRY-8, COMPR-1, COMPR-13, COMPR-16, COMPR-17, COMPR-19, COMPR-20, COMPR-27, COMPR-28, COMPR-29, COMPR-30, COMPR-31, COMPR-32, COMPR-33, TIMER-25. Supporting: RFC9776-COMPR-2, COMPR-3, COMPR-4, COMPR-5, COMPR-6, COMPR-7, COMPR-8, COMPR-9, COMPR-10, COMPR-14, COMPR-15, COMPR-18, COMPR-21, COMPR-22, COMPR-23, COMPR-24, COMPR-25, COMPR-26, TIMER-23, TIMER-24.

## IGMP-F-VERSION-2-ROUTER

**In IGMPv2 mode a router tracks each group in four states and answers a Leave with Group-Specific Queries before it removes the group.**

- **Sources** — RFC 2236 §7.
- **Level** — mandatory (reason: keyword elsewhere). RFC9776-COMPR-13: an IGMPv3 router must be able to operate in IGMPv2 compatibility mode for older hosts, and this is the behavior of that mode.
- **Description** — The group states are No Members Present, Members Present, Version 1 Members Present and Checking Membership. A Leave in Members Present sends a Group-Specific Query, lowers the timer to [Last Member Query Interval] times [Last Member Query Count], and repeats the query; a report ends the check. In Version 1 Members Present a Leave is ignored.
- **Checks** — core: RFC2236-ROUTER-27, ROUTER-28, ROUTER-32, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39, ROUTER-40, ROUTER-41, ROUTER-42, ROUTER-43, ROUTER-44, ROUTER-45, ROUTER-46, ROUTER-47. Supporting: RFC2236-ROUTER-15, ROUTER-16, ROUTER-17, ROUTER-18, ROUTER-19, ROUTER-21, ROUTER-22, ROUTER-24, ROUTER-25, ROUTER-26, ROUTER-29, ROUTER-30, ROUTER-31, ROUTER-33, ROUTER-34, ROUTER-48, ROUTER-49, ROUTER-50, ROUTER-51, ROUTER-52, ROUTER-53, ROUTER-54.

## IGMP-F-SSM-AWARE

**An SSM-aware system and router ignore EXCLUDE-mode records and older-version messages for an address of the SSM range.**

- **Sources** — RFC 9776 §4.2.13, §6.4, §7.2, §7.3.
- **Level** — optional (reason: condition). Every rule holds only for a system that is SSM-aware, and the in-scope set does not require SSM-awareness; RFC 4604 is level 5.
- **Description** — An SSM-aware host sends no MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE record for an SSM address and lets no older report suppress its record; an SSM-aware router ignores such records and older messages, and may log them.
- **Checks** — core: RFC9776-REP-21, REP-25, RREP-1, RREP-2, COMPH-27. Supporting: RFC9776-RREP-3, RREP-4, COMPH-23, COMPH-24, COMPH-25, COMPR-11, COMPR-12.

## IGMP-F-TIMER-CONFIGURATION

**The timers and counters have their defaults and their relations, and every system of a link uses the same values.**

- **Sources** — RFC 9776 §8, §8.1, §8.3, §8.14.2.
- **Level** — mandatory (reason: keyword). RFC9776-TIMER-1 has the strength "must".
- **Description** — The Robustness Variable is not zero, and should not be one. The Query Response Interval is shorter than the Query Interval, and the Query Interval is at least the Max Response Time of the General Queries.
- **Checks** — core: RFC9776-TIMER-1, TIMER-3, TIMER-6, TIMER-26. Supporting: RFC9776-TIMER-2.

## Coverage of the catalog entries

Every one of the 387 entries of the two catalogs is in at least one feature above, as core
or as supporting: RFC 9776 with 293 and RFC 2236 with
94.
