# MLD — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `MLD-F-*` · **Stands on:** [standards.md](standards.md), [rfc9777/catalog.md](../../standard/rfc9777/catalog.md), [rfc2710/catalog.md](../../standard/rfc2710/catalog.md)

Step 4 artifact of the standards test workflow. The catalogs are flat, fine-grained, and per
document. This document is the high-level view above them: the capabilities that the
in-scope set of [`standards.md`](standards.md#in-scope-set) defines, the requirement level of
each, and the cross reference into the catalogs.

The map reads in two directions:

- **Down** — which checks tell whether a feature works. A `core` check is one the feature
  cannot work without. A `supporting` check is a detail or an edge case.
- **Up** — which features the simulation model supports. Step 7 writes the answer into
  [`coverage.md`](../../model/mld/coverage.md), and step 8 compares it with the claims of the
  model in [`conformance.md`](../../model/mld/conformance.md).

The feature list comes from the standard texts only. The support of each feature — what the
run actually showed — is **not** in this document. It lives in the coverage ledger,
[`coverage.md`](../../model/mld/coverage.md).

## How this map decides the level

The rule of the guide: `mandatory` when any core statement says `must` or `shall`, or when the
mechanism is the only path the document gives to a state or an outcome; `optional` when every
core statement says `may` or `should`; `unstated` otherwise.

One refinement, which the DHCP, RIP, ND and IGMP maps state too: **a conditional keyword does not
raise the level of a feature.** RFC 9777 says that an SSM-aware router "SHOULD ignore" some
records; the rule holds only once a router is SSM-aware, which the in-scope set does not require.
The level comes from the statement that decides whether the mechanism has to exist at all, and
each feature names it.

A second refinement, which the IGMP map states too: **a keyword of the base document can make a
companion behavior mandatory.** RFC 9777 says that an MLDv2 host and router "MUST" operate in
version 1 compatibility mode (RFC9777-COMPL-1, COMPR-12), and RFC 2710 describes that mode with
state diagrams that have no keyword of their own. The two version 1 features take their level
from the keyword of RFC 9777, and say so.

## Index

| ID | Feature |
| --- | --- |
| [MLD-F-MESSAGE-FORMAT](#mld-f-message-format) | Every MLDv2 message is an ICMPv6 message from a link-local address, with Hop Limit 1 and a Router Alert option in a Hop-by-Hop Options header, and carries its type and a checksum. |
| [MLD-F-QUERY-FORMAT](#mld-f-query-format) | A Multicast Listener Query carries the Maximum Response Code, the multicast address, the S flag, QRV, QQIC and a source list, and its variant decides its destination. |
| [MLD-F-REPORT-FORMAT](#mld-f-report-format) | A Version 2 Multicast Listener Report carries Multicast Address Records of six types, each with a multicast address and a source list, and goes to ff02::16. |
| [MLD-F-MESSAGE-VALIDATION](#mld-f-message-validation) | A node verifies the checksum and the source, Hop Limit and Router Alert of a message, ignores reserved fields, extra octets, auxiliary data and unknown types, and accepts a message to any of its own addresses. |
| [MLD-F-STATE-CHANGE-REPORT](#mld-f-state-change-report) | A change of the listening state of an interface makes the node send a State-Change Report at once, with the records that the old and the new state give. |
| [MLD-F-REPORT-RETRANSMISSION](#mld-f-report-retransmission) | A node sends each State-Change Report [Robustness Variable] times in all, and merges a later change into the pending retransmissions. |
| [MLD-F-QUERY-RESPONSE](#mld-f-query-response) | A node answers a Query after a random delay within the Maximum Response Delay, with Current-State Records for all its addresses or for the queried address and sources. |
| [MLD-F-GENERAL-QUERY](#mld-f-general-query) | The querier sends General Queries to ff02::1 every Query Interval, and more often at startup. |
| [MLD-F-QUERIER-ELECTION](#mld-f-querier-election) | Among the routers of a link the one with the lowest IPv6 address is the querier; the others stay silent until the querier stops. |
| [MLD-F-LISTENING-STATE](#mld-f-listening-state) | A router keeps for each multicast address a filter mode, a Filter Timer and source records, sets them from Current-State Records, and times the listening state out after the Multicast Address Listening Interval. |
| [MLD-F-FORWARDING](#mld-f-forwarding) | A router suggests the forwarding of each source from its address state: INCLUDE forwards the listed sources, EXCLUDE all but the blocked ones. |
| [MLD-F-STATE-CHANGE-PROCESSING](#mld-f-state-change-processing) | A router applies Filter-Mode-Change and Source-List-Change Records to its state, and queries the sources and addresses that a node asked to stop. |
| [MLD-F-SPECIFIC-QUERIES](#mld-f-specific-queries) | Before it deletes an address or sources, the querier sends [Last Listener Query Count] Multicast Address Specific or Multicast Address and Source Specific Queries, [Last Listener Query Interval] apart, to the multicast address. |
| [MLD-F-QUERY-TIMER-UPDATES](#mld-f-query-timer-updates) | A router lowers its timers when it sends or hears a specific query without the S flag, and a non-querier adopts the QRV and QQIC of the querier. |
| [MLD-F-LISTENER-COMPATIBILITY](#mld-f-listener-compatibility) | A node falls back to MLDv1 when it hears an MLDv1 General Query, and returns to MLDv2 after the Older Version Querier Present Interval. |
| [MLD-F-VERSION-1-LISTENER](#mld-f-version-1-listener) | In MLDv1 mode a node reports a start of listening at once, answers a query after a random delay, suppresses its report when another node reports first, and sends a Done when it was the last to report. |
| [MLD-F-ROUTER-COMPATIBILITY](#mld-f-router-compatibility) | A router uses the lowest MLD version among the routers of a link, and keeps a compatibility mode for each multicast address from the versions of the reports it hears. |
| [MLD-F-VERSION-1-ROUTER](#mld-f-version-1-router) | In MLDv1 mode a router tracks each address in three states and answers a Done with Multicast Address Specific Queries before it removes the address. |
| [MLD-F-SSM-AWARE](#mld-f-ssm-aware) | An SSM-aware node and router ignore EXCLUDE-mode records and MLDv1 messages for an address of the SSM range. |
| [MLD-F-TIMER-CONFIGURATION](#mld-f-timer-configuration) | The timers and counters have their defaults and their relations, and every node of a link uses the same values. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [MLD-F-MESSAGE-FORMAT](#mld-f-message-format) | mandatory | RFC 9777 §5, §5.1.2, §5.2.2 | RFC9777-GEN-1, GEN-2, GEN-3, GEN-4, GEN-5, GEN-6, QRY-4, REP-4 |
| [MLD-F-QUERY-FORMAT](#mld-f-query-format) | mandatory | RFC 9777 §5.1, §7.6.2 | RFC9777-QRY-7, QRY-10, QRY-11, QRY-13, QRY-15, QRY-17, QRY-23, QRY-24, QRY-25, QRY-26, QRY-28, QRY-29, RQRY-10 |
| [MLD-F-REPORT-FORMAT](#mld-f-report-format) | mandatory | RFC 9777 §5.2 | RFC9777-REP-7, REP-9, REP-11, REP-12, REP-15, REP-21, REP-22, REP-23, REP-26, REP-27, REP-30, REP-31, REP-34, REP-40 |
| [MLD-F-MESSAGE-VALIDATION](#mld-f-message-validation) | mandatory | RFC 9777 §5, §5.1, §5.2, §6.2, §7.4, §7.6, §8.1; RFC 2710 §5, §6 | RFC9777-GEN-8, QRY-6, QRY-20, QRY-21, QRY-27, QRY-30, REP-6, REP-16, REP-17, REP-18, REP-33, REP-35, REP-42, VER-2, LQRY-1, RREP-1, RQRY-1, RFC2710-NODE-2, NODE-6, ROUTER-2, ROUTER-13, ROUTER-14, ROUTER-16 |
| [MLD-F-STATE-CHANGE-REPORT](#mld-f-state-change-report) | mandatory | RFC 9777 §6, §6.1 | RFC9777-LSN-3, LSN-5, LSN-6, LSN-8, LSN-9, LSN-10, LSN-11, LSN-12 |
| [MLD-F-REPORT-RETRANSMISSION](#mld-f-report-retransmission) | mandatory | RFC 9777 §6.1, §6.3, §9.11 | RFC9777-LSN-13, LSN-14, LSN-16, LSN-18, LSN-20, LSN-21, LSN-22, LSN-23, LSN-24, LSN-25, LTIM-12, LTIM-14, LTIM-15, LTIM-16, LTIM-17, LTIM-18, LTIM-19 |
| [MLD-F-QUERY-RESPONSE](#mld-f-query-response) | mandatory | RFC 9777 §6.2, §6.3 | RFC9777-LQRY-2, LQRY-4, LQRY-5, LQRY-6, LQRY-7, LTIM-1, LTIM-2, LTIM-5, LTIM-6, LTIM-7, LTIM-8, LTIM-9, LTIM-10 |
| [MLD-F-GENERAL-QUERY](#mld-f-general-query) | mandatory | RFC 9777 §7, §7.1, §7.6.2, §9.2, §9.3, §9.6, §9.7; RFC 2710 §6, §7.6, §7.7 | RFC9777-RQ-8, RQRY-7, TIMER-4, TIMER-5, TIMER-9, TIMER-10, RFC2710-ROUTER-5, ROUTER-6, ROUTER-7 |
| [MLD-F-QUERIER-ELECTION](#mld-f-querier-election) | mandatory | RFC 9777 §7.6.2, §9.5; RFC 2710 §6, §7.5 | RFC9777-RQRY-6, RQRY-8, RQRY-9, RQRY-11, TIMER-8, RFC2710-ROUTER-8, ROUTER-9, ROUTER-10 |
| [MLD-F-LISTENING-STATE](#mld-f-listening-state) | mandatory | RFC 9777 §7.2, §7.4.1, §7.5, §9.4 | RFC9777-RREP-16, RREP-17, RREP-18, RREP-19, RST-15, RST-25, RSW-1, RSW-2, RSW-3, TIMER-7 |
| [MLD-F-FORWARDING](#mld-f-forwarding) | mandatory | RFC 9777 §7.2.1, §7.3 | RFC9777-FWD-3, FWD-4, FWD-5, FWD-6, FWD-7, FWD-8, RST-7, RST-11 |
| [MLD-F-STATE-CHANGE-PROCESSING](#mld-f-state-change-processing) | mandatory | RFC 9777 §7.1, §7.2.3, §7.4.2 | RFC9777-RREP-5, RREP-20, RREP-21, RREP-22, RREP-23, RREP-24, RREP-25, RREP-26, RREP-27 |
| [MLD-F-SPECIFIC-QUERIES](#mld-f-specific-queries) | mandatory | RFC 9777 §7.2.3, §7.4.2, §7.6.3, §9.8 to §9.10 | RFC9777-RQRY-12, RQRY-13, RQRY-14, RQRY-15, RQRY-16, RQRY-17, RQRY-18, RQRY-19, RQRY-20, RREP-14, TIMER-11, TIMER-13, TIMER-14 |
| [MLD-F-QUERY-TIMER-UPDATES](#mld-f-query-timer-updates) | mandatory | RFC 9777 §5.1.7 to §5.1.9, §7.4.2, §7.6.1 | RFC9777-RQRY-2, RQRY-3, RQRY-4, RQRY-5, RREP-6, QRY-12 |
| [MLD-F-LISTENER-COMPATIBILITY](#mld-f-listener-compatibility) | mandatory | RFC 9777 §5, §5.2.15, §8.1, §8.2, §9.12 | RFC9777-VER-1, COMPL-1, COMPL-4, COMPL-5, COMPL-7, COMPL-9, COMPL-11, TIMER-17 |
| [MLD-F-VERSION-1-LISTENER](#mld-f-version-1-listener) | mandatory | RFC 2710 §5, §7.10 | RFC2710-NODE-10, NODE-12, NODE-13, NODE-14, NODE-17, NODE-18, NODE-19, NODE-20, NODE-21, NODE-22, NODE-23, NODE-24 |
| [MLD-F-ROUTER-COMPATIBILITY](#mld-f-router-compatibility) | mandatory | RFC 9777 §8.3, §9.13 | RFC9777-COMPR-1, COMPR-5, COMPR-7, COMPR-12, COMPR-14, COMPR-15, COMPR-17, COMPR-20, COMPR-21, COMPR-22, COMPR-23, TIMER-19 |
| [MLD-F-VERSION-1-ROUTER](#mld-f-version-1-router) | mandatory | RFC 2710 §6, §7.4, §7.8, §7.9 | RFC2710-ROUTER-26, ROUTER-27, ROUTER-28, ROUTER-29, ROUTER-30, ROUTER-31, ROUTER-32, ROUTER-34, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39 |
| [MLD-F-SSM-AWARE](#mld-f-ssm-aware) | optional | RFC 9777 §5.2.13, §7.4, §8.2.1, §8.3.1 | RFC9777-REP-24, REP-28, RREP-2, RREP-3 |
| [MLD-F-TIMER-CONFIGURATION](#mld-f-timer-configuration) | mandatory | RFC 9777 §9, §9.1, §9.3, §9.14.2; RFC 2710 §7, §7.1 to §7.3 | RFC9777-TIMER-1, TIMER-3, TIMER-6, TIMER-20, RFC2710-TIMER-1, TIMER-4, TIMER-7 |

## MLD-F-MESSAGE-FORMAT

**Every MLDv2 message is an ICMPv6 message from a link-local address, with Hop Limit 1 and a Router Alert option in a Hop-by-Hop Options header, and carries its type and a checksum.**

- **Sources** — RFC 9777 §5, §5.1.2, §5.2.2.
- **Level** — mandatory (reason: keyword). RFC9777-GEN-2 has the strength "must".
- **Description** — The Multicast Listener Query has type 130 and the Version 2 Multicast Listener Report type 143. The checksum is the ICMPv6 checksum over the message and a pseudo-header. A sender adds no octets after the defined fields and sets the Code and Reserved fields to zero.
- **Checks** — core: RFC9777-GEN-1, GEN-2, GEN-3, GEN-4, GEN-5, GEN-6, QRY-4, REP-4. Supporting: RFC9777-QRY-2, QRY-5, QRY-8, QRY-22, REP-2, REP-5, REP-19.

## MLD-F-QUERY-FORMAT

**A Multicast Listener Query carries the Maximum Response Code, the multicast address, the S flag, QRV, QQIC and a source list, and its variant decides its destination.**

- **Sources** — RFC 9777 §5.1, §7.6.2.
- **Level** — mandatory (reason: keyword). RFC9777-QRY-26 has the strength "must".
- **Description** — A General Query has the unspecified multicast address and no sources and goes to ff02::1. A Multicast Address Specific Query names the address and goes to it; a Multicast Address and Source Specific Query also lists sources. The Maximum Response Code uses a floating-point code from 32768 on, and the QQIC from 128 on. QRV carries the Robustness Variable of the querier, or zero above 7. Every Query leaves from a link-local address.
- **Checks** — core: RFC9777-QRY-7, QRY-10, QRY-11, QRY-13, QRY-15, QRY-17, QRY-23, QRY-24, QRY-25, QRY-26, QRY-28, QRY-29, RQRY-10. Supporting: RFC9777-QRY-1, QRY-18, QRY-19.

## MLD-F-REPORT-FORMAT

**A Version 2 Multicast Listener Report carries Multicast Address Records of six types, each with a multicast address and a source list, and goes to ff02::16.**

- **Sources** — RFC 9777 §5.2.
- **Level** — mandatory (reason: keyword). RFC9777-REP-15 has the strength "must".
- **Description** — The record types are MODE_IS_INCLUDE and MODE_IS_EXCLUDE (current state), CHANGE_TO_INCLUDE_MODE and CHANGE_TO_EXCLUDE_MODE (filter-mode change), ALLOW_NEW_SOURCES and BLOCK_OLD_SOURCES (source-list change). A Report leaves from a link-local address, or from the unspecified address before the interface has one. A node sends no auxiliary data, and splits records that do not fit into one message.
- **Checks** — core: RFC9777-REP-7, REP-9, REP-11, REP-12, REP-15, REP-21, REP-22, REP-23, REP-26, REP-27, REP-30, REP-31, REP-34, REP-40. Supporting: RFC9777-REP-1, REP-8, REP-10, REP-13, REP-14, REP-20, REP-25, REP-29, REP-32, REP-39, REP-43, REP-44, REP-45.

## MLD-F-MESSAGE-VALIDATION

**A node verifies the checksum and the source, Hop Limit and Router Alert of a message, ignores reserved fields, extra octets, auxiliary data and unknown types, and accepts a message to any of its own addresses.**

- **Sources** — RFC 9777 §5, §5.1, §5.2, §6.2, §7.4, §7.6, §8.1; RFC 2710 §5, §6.
- **Level** — mandatory (reason: keyword). RFC9777-GEN-8 has the strength "must".
- **Description** — An unrecognized message type, an unrecognized record type and a Query of an impossible length are silently ignored. A Query or a Report without a valid link-local source, Hop Limit 1 and the Router Alert option is dropped; a router drops a Report from the unspecified address. RFC 2710 requires a link-local source, at least 24 octets and a correct checksum for each message of the version 1 state machines.
- **Checks** — core: RFC9777-GEN-8, QRY-6, QRY-20, QRY-21, QRY-27, QRY-30, REP-6, REP-16, REP-17, REP-18, REP-33, REP-35, REP-42, VER-2, LQRY-1, RREP-1, RQRY-1, RFC2710-NODE-2, NODE-6, ROUTER-2, ROUTER-13, ROUTER-14, ROUTER-16. Supporting: RFC9777-QRY-3, QRY-9, REP-3, REP-36, REP-37, LSN-4, RFC2710-NODE-9.

## MLD-F-STATE-CHANGE-REPORT

**A change of the listening state of an interface makes the node send a State-Change Report at once, with the records that the old and the new state give.**

- **Sources** — RFC 9777 §6, §6.1.
- **Level** — mandatory (reason: keyword). RFC9777-LSN-3 has the strength "must".
- **Description** — From INCLUDE to INCLUDE and from EXCLUDE to EXCLUDE the report holds ALLOW and BLOCK records for the difference; a change of filter mode gives one TO_IN or TO_EX record with the new source list. A missing state counts as INCLUDE with no sources, so a join is a change to EXCLUDE({}) and a leave a change to INCLUDE({}). An empty ALLOW or BLOCK record is left out. No report ever names ff02::1 or an address of scope 0 or 1.
- **Checks** — core: RFC9777-LSN-3, LSN-5, LSN-6, LSN-8, LSN-9, LSN-10, LSN-11, LSN-12. Supporting: RFC9777-LSN-1, LSN-2, LSN-7, RQ-10, REP-38.

## MLD-F-REPORT-RETRANSMISSION

**A node sends each State-Change Report [Robustness Variable] times in all, and merges a later change into the pending retransmissions.**

- **Sources** — RFC 9777 §6.1, §6.3, §9.11.
- **Level** — mandatory (reason: only path). RFC 9777 §6.1 gives the repetition as the one way a State-Change Report survives the packet loss that the Robustness Variable allows; the lower-case musts of RFC9777-LSN-22 to LSN-25 fix the content of each repetition.
- **Description** — The repetitions come at random times within the Unsolicited Report Interval. A filter-mode change keeps its TO_IN or TO_EX record in the next [Robustness Variable] reports; source changes keep their ALLOW and BLOCK records the same number of times. A new change sends a merged report at once and restarts the count.
- **Checks** — core: RFC9777-LSN-13, LSN-14, LSN-16, LSN-18, LSN-20, LSN-21, LSN-22, LSN-23, LSN-24, LSN-25, LTIM-12, LTIM-14, LTIM-15, LTIM-16, LTIM-17, LTIM-18, LTIM-19. Supporting: RFC9777-LSN-15, LSN-17, LSN-19, LTIM-13, TIMER-15.

## MLD-F-QUERY-RESPONSE

**A node answers a Query after a random delay within the Maximum Response Delay, with Current-State Records for all its addresses or for the queried address and sources.**

- **Sources** — RFC 9777 §6.2, §6.3.
- **Level** — mandatory (reason: keyword). RFC9777-LTIM-5 has the strength "must, must not".
- **Description** — A General Query schedules one response for the interface; a specific query schedules one for the address, and a Multicast Address and Source Specific Query also records the queried sources. The response to a General Query never leaves at once. A source-specific response reports only the queried sources that the state wants, and an empty record is not sent.
- **Checks** — core: RFC9777-LQRY-2, LQRY-4, LQRY-5, LQRY-6, LQRY-7, LTIM-1, LTIM-2, LTIM-5, LTIM-6, LTIM-7, LTIM-8, LTIM-9, LTIM-10. Supporting: RFC9777-LQRY-3, LQRY-8, LQRY-9, LTIM-3, LTIM-4, LTIM-11, RQ-9.

## MLD-F-GENERAL-QUERY

**The querier sends General Queries to ff02::1 every Query Interval, and more often at startup.**

- **Sources** — RFC 9777 §7, §7.1, §7.6.2, §9.2, §9.3, §9.6, §9.7; RFC 2710 §6, §7.6, §7.7.
- **Level** — mandatory (reason: only path). The periodic General Query is the one way a router refreshes the listening state of a link, RFC 9777 §7.1.
- **Description** — A router starts as querier and sends [Startup Query Count] General Queries [Startup Query Interval] apart, then one every [Query Interval], with the Maximum Response Delay of the Query Response Interval. A router listens to ff02::16 on each interface and runs the listener part of MLDv2 too.
- **Checks** — core: RFC9777-RQ-8, RQRY-7, TIMER-4, TIMER-5, TIMER-9, TIMER-10, RFC2710-ROUTER-5, ROUTER-6, ROUTER-7. Supporting: RFC9777-RQ-1, RQ-2, RQ-3, RQ-4, RQ-6, RQ-7, RFC2710-ROUTER-3, TIMER-12, TIMER-13.

## MLD-F-QUERIER-ELECTION

**Among the routers of a link the one with the lowest IPv6 address is the querier; the others stay silent until the querier stops.**

- **Sources** — RFC 9777 §7.6.2, §9.5; RFC 2710 §6, §7.5.
- **Level** — mandatory (reason: keyword). RFC9777-TIMER-8 has the strength "must".
- **Description** — A router that hears a Query from a lower address becomes a non-querier and starts the Other-Querier-Present Timer; when the timer expires it becomes the querier again. The address comparison reads the last 64 bits, the interface identifier.
- **Checks** — core: RFC9777-RQRY-6, RQRY-8, RQRY-9, RQRY-11, TIMER-8, RFC2710-ROUTER-8, ROUTER-9, ROUTER-10. Supporting: RFC2710-ROUTER-1, ROUTER-4, ROUTER-33, TIMER-10, TIMER-11.

## MLD-F-LISTENING-STATE

**A router keeps for each multicast address a filter mode, a Filter Timer and source records, sets them from Current-State Records, and times the listening state out after the Multicast Address Listening Interval.**

- **Sources** — RFC 9777 §7.2, §7.4.1, §7.5, §9.4.
- **Level** — mandatory (reason: keyword). RFC9777-TIMER-7 has the strength "must".
- **Description** — IS_IN and IS_EX records move the state as Table 7 gives. When the Filter Timer of an EXCLUDE address expires, the address switches to INCLUDE with the sources whose timers still run, or goes away. The Multicast Address Listening Interval is [Robustness Variable] times [Query Interval] plus twice [Query Response Interval].
- **Checks** — core: RFC9777-RREP-16, RREP-17, RREP-18, RREP-19, RST-15, RST-25, RSW-1, RSW-2, RSW-3, TIMER-7. Supporting: RFC9777-RST-1, RST-2, RST-3, RST-4, RST-5, RST-6, RST-8, RST-9, RST-10, RST-13, RST-14, RST-16, RST-17, RST-18, RST-19, RST-20, RST-23, RST-24, RST-30, RST-31, RST-32, RST-36, RREP-4, RQ-5.

## MLD-F-FORWARDING

**A router suggests the forwarding of each source from its address state: INCLUDE forwards the listed sources, EXCLUDE all but the blocked ones.**

- **Sources** — RFC 9777 §7.2.1, §7.3.
- **Level** — mandatory (reason: only path). The forwarding suggestion is the one outcome that MLD gives to multicast routing, §7.3.
- **Description** — In INCLUDE mode a source with a running timer is forwarded and every other source is not. In EXCLUDE mode a source with a timer at zero is blocked and every other source is forwarded. The multicast routing protocol uses these suggestions.
- **Checks** — core: RFC9777-FWD-3, FWD-4, FWD-5, FWD-6, FWD-7, FWD-8, RST-7, RST-11. Supporting: RFC9777-FWD-1, FWD-2, RST-12.

## MLD-F-STATE-CHANGE-PROCESSING

**A router applies Filter-Mode-Change and Source-List-Change Records to its state, and queries the sources and addresses that a node asked to stop.**

- **Sources** — RFC 9777 §7.1, §7.2.3, §7.4.2.
- **Level** — mandatory (reason: keyword). RFC9777-RREP-5 has the strength "must (lower case)".
- **Description** — ALLOW, BLOCK, TO_IN and TO_EX records move the state as Table 8 gives. A record that stops sources or an address makes the router lower their timers to the Last Listener Query Time and query them; a record of interest that arrives during the query keeps them.
- **Checks** — core: RFC9777-RREP-5, RREP-20, RREP-21, RREP-22, RREP-23, RREP-24, RREP-25, RREP-26, RREP-27. Supporting: RFC9777-RREP-7, RREP-8, RREP-9, RREP-10, RREP-11, RREP-12, RREP-13, RREP-15, RQ-11, RQ-12, RQ-13, RQ-14, RQ-15, RQ-16, RQ-17, RST-26, RST-27, RST-28, RST-29, RST-33, RST-34, RST-35.

## MLD-F-SPECIFIC-QUERIES

**Before it deletes an address or sources, the querier sends [Last Listener Query Count] Multicast Address Specific or Multicast Address and Source Specific Queries, [Last Listener Query Interval] apart, to the multicast address.**

- **Sources** — RFC 9777 §7.2.3, §7.4.2, §7.6.3, §9.8 to §9.10.
- **Level** — mandatory (reason: keyword). RFC9777-RQRY-12 has the strength "must (lower case)".
- **Description** — A leave, a TO_IN or a BLOCK record triggers the queries. The first query leaves at once. A Multicast Address and Source Specific Query is split into a message with the S flag set and one with it clear, and a message with no sources is not sent. The S flag is set when the timer is above the Last Listener Query Time.
- **Checks** — core: RFC9777-RQRY-12, RQRY-13, RQRY-14, RQRY-15, RQRY-16, RQRY-17, RQRY-18, RQRY-19, RQRY-20, RREP-14, TIMER-11, TIMER-13, TIMER-14. Supporting: RFC9777-RST-21, RST-22, TIMER-12.

## MLD-F-QUERY-TIMER-UPDATES

**A router lowers its timers when it sends or hears a specific query without the S flag, and a non-querier adopts the QRV and QQIC of the querier.**

- **Sources** — RFC 9777 §5.1.7 to §5.1.9, §7.4.2, §7.6.1.
- **Level** — mandatory (reason: keyword). RFC9777-RQRY-2 has the strength "must (lower case)".
- **Description** — A Multicast Address Specific Query lowers the Filter Timer, and a Multicast Address and Source Specific Query the Source Timers of its sources, to the Last Listener Query Time. The S flag suppresses these updates, but not querier election.
- **Checks** — core: RFC9777-RQRY-2, RQRY-3, RQRY-4, RQRY-5, RREP-6, QRY-12. Supporting: RFC9777-QRY-14, QRY-16.

## MLD-F-LISTENER-COMPATIBILITY

**A node falls back to MLDv1 when it hears an MLDv1 General Query, and returns to MLDv2 after the Older Version Querier Present Interval.**

- **Sources** — RFC 9777 §5, §5.2.15, §8.1, §8.2, §9.12.
- **Level** — mandatory (reason: keyword). RFC9777-COMPL-1 has the strength "must".
- **Description** — The length of a Query tells its version. An MLDv1 General Query starts the Older-Version-Querier-Present Timer; while it runs the node uses only MLDv1, and a change of mode cancels the pending responses and retransmissions. The Maximum Response Code of an MLDv1 Query is linear.
- **Checks** — core: RFC9777-VER-1, COMPL-1, COMPL-4, COMPL-5, COMPL-7, COMPL-9, COMPL-11, TIMER-17. Supporting: RFC9777-GEN-7, COMPL-2, COMPL-3, COMPL-6, COMPL-8, COMPL-10, COMPL-15, REP-41, TIMER-16.

## MLD-F-VERSION-1-LISTENER

**In MLDv1 mode a node reports a start of listening at once, answers a query after a random delay, suppresses its report when another node reports first, and sends a Done when it was the last to report.**

- **Sources** — RFC 2710 §5, §7.10.
- **Level** — mandatory (reason: keyword elsewhere). RFC9777-COMPL-1: an MLDv2 host must operate in version 1 compatibility mode, and this is the behavior of that mode.
- **Description** — The node state machine of RFC 2710 has the states Non-Listener, Delaying Listener and Idle Listener for each address. A Done goes to the link-scope all-routers address ff02::2. A node never reports the all-nodes address or an address of reserved or node-local scope.
- **Checks** — core: RFC2710-NODE-10, NODE-12, NODE-13, NODE-14, NODE-17, NODE-18, NODE-19, NODE-20, NODE-21, NODE-22, NODE-23, NODE-24. Supporting: RFC2710-NODE-1, NODE-3, NODE-4, NODE-5, NODE-7, NODE-8, NODE-11, NODE-15, NODE-16, NODE-25, NODE-26, TIMER-16.

## MLD-F-ROUTER-COMPATIBILITY

**A router uses the lowest MLD version among the routers of a link, and keeps a compatibility mode for each multicast address from the versions of the reports it hears.**

- **Sources** — RFC 9777 §8.3, §9.13.
- **Level** — mandatory (reason: keyword). RFC9777-COMPR-1 has the strength "must".
- **Description** — An MLDv1 Report starts the Older-Version-Host-Present Timer of the address, and while it runs the router treats the address in MLDv1: a Report is IS_EX({}), a Done is TO_IN({}), and BLOCK records and the source lists of TO_EX are ignored. The querier keeps sending MLDv2 Queries. In MLDv1 mode, which an administrator configures, a router sends Queries of 24 octets with a linear Maximum Response Code.
- **Checks** — core: RFC9777-COMPR-1, COMPR-5, COMPR-7, COMPR-12, COMPR-14, COMPR-15, COMPR-17, COMPR-20, COMPR-21, COMPR-22, COMPR-23, TIMER-19. Supporting: RFC9777-COMPR-2, COMPR-3, COMPR-4, COMPR-6, COMPR-8, COMPR-9, COMPR-13, COMPR-16, COMPR-18, COMPR-19, TIMER-18.

## MLD-F-VERSION-1-ROUTER

**In MLDv1 mode a router tracks each address in three states and answers a Done with Multicast Address Specific Queries before it removes the address.**

- **Sources** — RFC 2710 §6, §7.4, §7.8, §7.9.
- **Level** — mandatory (reason: keyword elsewhere). RFC9777-COMPR-12: an MLDv2 router must operate in version 1 compatibility mode for older hosts, and this is the behavior of that mode.
- **Description** — The address states are No Listeners Present, Listeners Present and Checking Listeners. A Done in Listeners Present sends a Multicast Address Specific Query, lowers the timer to [Last Listener Query Interval] times [Last Listener Query Count], and repeats the query; a report ends the check. A non-querier follows the queries of the querier.
- **Checks** — core: RFC2710-ROUTER-26, ROUTER-27, ROUTER-28, ROUTER-29, ROUTER-30, ROUTER-31, ROUTER-32, ROUTER-34, ROUTER-35, ROUTER-36, ROUTER-37, ROUTER-38, ROUTER-39. Supporting: RFC2710-ROUTER-11, ROUTER-12, ROUTER-15, ROUTER-17, ROUTER-18, ROUTER-19, ROUTER-20, ROUTER-21, ROUTER-22, ROUTER-23, ROUTER-24, ROUTER-25, TIMER-8, TIMER-9, TIMER-14, TIMER-15.

## MLD-F-SSM-AWARE

**An SSM-aware node and router ignore EXCLUDE-mode records and MLDv1 messages for an address of the SSM range.**

- **Sources** — RFC 9777 §5.2.13, §7.4, §8.2.1, §8.3.1.
- **Level** — optional (reason: condition). Every rule holds only for a node that is SSM-aware, and the in-scope set does not require SSM-awareness; RFC 4604 is level 5.
- **Description** — An SSM-aware host sends no MODE_IS_EXCLUDE or CHANGE_TO_EXCLUDE_MODE record for an SSM address; an SSM-aware router ignores such records and MLDv1 messages, and may log them. A configuration option disables the compatibility modes for SSM-only operation.
- **Checks** — core: RFC9777-REP-24, REP-28, RREP-2, RREP-3. Supporting: RFC9777-COMPL-12, COMPL-13, COMPL-14, COMPR-10, COMPR-11.

## MLD-F-TIMER-CONFIGURATION

**The timers and counters have their defaults and their relations, and every node of a link uses the same values.**

- **Sources** — RFC 9777 §9, §9.1, §9.3, §9.14.2; RFC 2710 §7, §7.1 to §7.3.
- **Level** — mandatory (reason: keyword). RFC9777-TIMER-1 has the strength "must".
- **Description** — The Robustness Variable is not zero, and should not be one. The Query Response Interval is shorter than the Query Interval, and the Query Interval is at least the Maximum Response Delay of the General Queries.
- **Checks** — core: RFC9777-TIMER-1, TIMER-3, TIMER-6, TIMER-20, RFC2710-TIMER-1, TIMER-4, TIMER-7. Supporting: RFC9777-TIMER-2, RFC2710-TIMER-2, TIMER-3, TIMER-5, TIMER-6.

## Coverage of the catalog entries

Every one of the 388 entries of the two catalogs is in at least one feature above, as core
or as supporting: RFC 9777 with 307 and RFC 2710 with
81.
