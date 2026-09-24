# MLD — English check procedures: state and forwarding of a router

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9777/catalog.md](../../../standard/rfc9777/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Forwarding after a start of listening

Checks: **RFC9777-FWD-8**, **RREP-22** (description); covers **RFC9777-FWD-1** (should (lower
case)), **RST-4**, **RST-8**, **RST-9**, **RST-11** (description), **RQ-2**, **RQ-3** (must
(lower case)), **RQ-4** (must).

### Requirement

RFC 9777 §7.4.2 and §7.3: a router in INCLUDE({}) that receives TO_EX({}) moves to EXCLUDE({},
{}); in EXCLUDE mode a source without a source record is forwarded, and with no multicast
address record at all nothing is forwarded.

### Scenario constants

- The link with a source. S1 sends to G. At 10 seconds, host A starts to listen to G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. At 10 seconds, let A start to listen to G.
3. Observe the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, at 10 seconds, a Report with a CHANGE_TO_EXCLUDE_MODE record for G. This
   confirms the stimulus.
2. Before that Report, no datagram of S1 to G arrives on L1: without a multicast address record,
   nothing is forwarded.
3. From 1 second after that Report on, every datagram of S1 to G arrives on L1 (RFC9777-FWD-8,
   RREP-22).

## Source-specific forwarding

Checks: **RFC9777-FWD-3**, **FWD-5**, **RREP-20**, **RST-7** (description); covers
**RFC9777-RST-1**, **RST-2**, **RST-3**, **RST-5**, **RST-6**, **RST-23** (description).

### Requirement

RFC 9777 §7.4.2, §7.2.1 and §7.3: a router in INCLUDE(A) that receives ALLOW(B) moves to
INCLUDE(A + B) with the Source Timers of B at the Multicast Address Listening Interval; in
INCLUDE mode a source of the Include List is forwarded, and every other source is not.

### Scenario constants

- The link with a source. S1 and S2 send to G. At 10 seconds, host A starts to listen to G for
  S1.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B, S1 and S2 come up.
2. At 10 seconds, let A start to listen to G for S1.
3. Observe the Reports of A and the datagrams of S1 and S2 on L1.

### Expected observations

1. On L1, from A, at 10 seconds, a Report with an ALLOW_NEW_SOURCES record for G with S1. This
   confirms the stimulus.
2. From 1 second after that Report on, every datagram of S1 to G arrives on L1 (RFC9777-FWD-3,
   RREP-20, RST-7).
3. No datagram of S2 to G arrives on L1 in the window (RFC9777-FWD-5, RST-7).

## Listening timeout without a stop

Checks: **RFC9777-RREP-19**, **RST-15**, **RST-18**, **RSW-1**, **RSW-3** (description),
**TIMER-7** (must); covers **RFC9777-RST-13**, **RST-14**, **RST-17**, **RST-20** (description).

### Requirement

RFC 9777 §7.4.1, §7.2.2, §7.5 and §9.4: a Current-State Record IS_EX({}) sets the Filter Timer
to the Multicast Address Listening Interval; when the Filter Timer of an EXCLUDE address expires
and the Requested List is empty, the router deletes the multicast address record and forwards
the address no more. The Multicast Address Listening Interval is [Robustness Variable] ×
[Query Interval] + 2 × [Query Response Interval].

### Scenario constants

- The link with a source. S1 sends to G. At 10 seconds, host A starts to listen to G; A answers
  the General Queries of R; at 45 seconds, A leaves L1 without a stop.
- Observation lasts 350 seconds from the start.

### Size or value arithmetic

The Multicast Address Listening Interval is 2 × 125 + 2 × 10 = 270 seconds. A answers the
General Query of 31.25 seconds within 10 seconds, so its last Report comes before 45 seconds,
and the forwarding stops about 270 seconds after that Report, between 280 and 315 seconds.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. At 10 seconds, let A start to listen to G; at 45 seconds, break the link between A and L1.
3. Observe the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, before 45 seconds, a Report with a Current-State Record MODE_IS_EXCLUDE for G.
   Record the instant of the last Report of A. This confirms the stimulus.
2. Datagrams of S1 to G arrive on L1 until 270 seconds after that Report, with a margin of 1
   second (RFC9777-RREP-19, TIMER-7).
3. No datagram of S1 to G arrives on L1 later than 1 second after that instant (RFC9777-RST-15,
   RST-18, RSW-1, RSW-3, TIMER-7).

### Notes

- RFC 2710 and RFC 3810 gave the interval as one Query Response Interval less, 260 seconds; RFC
  9777 governs, as the override table of [`standards.md`](../standards.md#override-table) says.

## Stop of listening and the last listener query

Checks: **RFC9777-RREP-27**, **RQ-12**, **RQ-13**, **RQRY-14**, **QRY-11**, **QRY-24**,
**QRY-29**, **RREP-8**, **RREP-10**, **RREP-14**, **TIMER-11**, **TIMER-13**, **TIMER-14**
(description), **RQ-11**, **RQRY-12**, **RQRY-13** (must (lower case)), **RREP-12** (may (lower
case)); covers **RFC9777-RQ-14**, **RQRY-4**, **RREP-15**, **TIMER-12** (description),
**RST-21**, **RST-22** (should (lower case)).

### Requirement

RFC 9777 §7.4.2 and §7.6.3.1: a router in EXCLUDE mode that receives TO_IN({}) lowers the
Filter Timer to the Last Listener Query Time, sends a Multicast Address Specific Query at once
and [Last Listener Query Count] − 1 more, [Last Listener Query Interval] apart, to the multicast
address; without an answer the address switches to INCLUDE mode with no sources, and the router
forwards it no more. The Suppress Router-Side Processing flag is set only when the Filter Timer
is above the Last Listener Query Time.

### Scenario constants

- The link with a source. S1 sends to G. At 10 seconds, host A starts to listen to G; at 50
  seconds, A stops. A is the only listener of G. A sends each State-Change Report once
  (Robustness Variable 1).
- Observation lasts 60 seconds from the start.

### Size or value arithmetic

The Last Listener Query Time is 2 × 1 = 2 seconds, so the forwarding stops 2 seconds after the
stop.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. At 10 seconds, let A start to listen to G; at 50 seconds, let A stop.
3. Observe the Queries of R and the datagrams of S1 on L1 after 50 seconds.

### Expected observations

1. On L1, from A, at 50 seconds, a Report with a CHANGE_TO_INCLUDE_MODE record for G. This
   confirms the stimulus.
2. On L1, from R, within 0.1 seconds of that Report, a Multicast Address Specific Query to G:
   Multicast Address G and no sources (RFC9777-RREP-27, RQ-11, RQ-12, RQ-13, RQRY-13, QRY-11,
   QRY-24, QRY-29).
3. A second Multicast Address Specific Query for G 1 second after the first, and no third one
   (RFC9777-RQRY-13, RREP-14, TIMER-13, TIMER-14).
4. Datagrams of S1 to G still arrive on L1 until 1.5 seconds after the stop, and none arrives
   later than 2.5 seconds after it (RFC9777-RQRY-12, RREP-8, RREP-10, RREP-12).
5. Each of the two Queries has a Maximum Response Code of 1000, a Maximum Response Delay of 1
   second (RFC9777-TIMER-11).
6. Each of the two Queries has the S flag clear, because the Filter Timer is not above the Last
   Listener Query Time (RFC9777-RQRY-14).

## Stop of listening with another listener

Checks: **RFC9777-RREP-9** (description); covers **RFC9777-RREP-7** (description).

### Requirement

RFC 9777 §7.4.2: an EXCLUDE-mode record that arrives during the query of an address updates the
Filter Timer, and the router forwards the address without a break.

### Scenario constants

- The link with a source. S1 sends to G. Hosts A and B start to listen to G at 10 seconds; B
  stops at 50 seconds, and A answers the Multicast Address Specific Query.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. Let A and B make the requests above.
3. Observe the Queries of R, the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, after the first Multicast Address Specific Query for G, a Report with a
   MODE_IS_EXCLUDE record for G. This confirms the stimulus.
2. From 50 to 60 seconds, a datagram of S1 to G arrives on L1 every 0.5 seconds, with no gap
   longer than 1 second (RFC9777-RREP-9).

## Source blocked while another listener wants it

Checks: **RFC9777-RREP-5**, **RQRY-15**, **RQRY-16** (must (lower case)), **RREP-11**,
**RREP-21**, **RQ-15**, **RQ-16**, **RQRY-17**, **RQRY-18**, **RQRY-19**, **QRY-25**, **RST-28**
(description); covers **RFC9777-RQ-17**, **RQRY-20**, **RST-26**, **RST-27**, **RREP-6**,
**RREP-7**, **RREP-13**, **RREP-16** (description).

### Requirement

RFC 9777 §7.4.2 and §7.6.3.2: a router in INCLUDE(A) that receives BLOCK(B) keeps INCLUDE(A)
and sends Multicast Address and Source Specific Queries for A ∩ B: one at once and [Last
Listener Query Count] − 1 more, [Last Listener Query Interval] apart; it lowers the Source Timers
of those sources to the Last Listener Query Time. Each query goes as two messages: one with the
S flag set for the sources whose timer is above that time, one with the flag clear for the
others; a message without sources is not sent.

### Scenario constants

- The link with a source. S1 sends to G. Host A listens to G for S1 and S2 from 10 seconds on;
  host B listens to G for S1 from 10 to 50 seconds, so that its stop at 50 seconds sends
  BLOCK({S1}). A answers the queries for S1. B sends each State-Change Report once (Robustness
  Variable 1).
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. Let A and B make the requests above.
3. Observe the Queries of R and the datagrams of S1 on L1 after 50 seconds.

### Expected observations

1. On L1, from B, at 50 seconds, a Report with a BLOCK_OLD_SOURCES record for G with S1. This
   confirms the stimulus.
2. On L1, from R, within 0.1 seconds of that Report, a Multicast Address and Source Specific
   Query to G with Multicast Address G and the source S1 only (RFC9777-RREP-5, RREP-21, RQ-15,
   RQ-16, RQRY-16, QRY-25).
3. A second such Query 1 second after the first, and no third one (RFC9777-RQRY-16).
4. Datagrams of S1 to G arrive on L1 from 50 to 60 seconds with no gap longer than 1 second,
   because A answers (RFC9777-RREP-11, RST-28).
5. The S flag is clear in the first such Query, because the router lowers the Source Timer of S1
   to the Last Listener Query Time before it builds the Query (RFC9777-RQRY-15, RQRY-19). The S
   flag of the second Query is set if the answer of A came before it, because that answer raises
   the Source Timer of S1 to the Multicast Address Listening Interval, and clear if not
   (RFC9777-RQRY-17, RQRY-18).

## Source blocked by its only listener

Checks: **RFC9777-RQRY-2** (must (lower case)), **RQRY-3**, **RST-25**, **RST-29**, **FWD-4**
(description).

### Requirement

RFC 9777 §7.6.1 and §7.2.3: sending a Multicast Address and Source Specific Query for sources A
lowers their Source Timers to the Last Listener Query Time; when no record of interest arrives,
the timers expire, the sources leave the Include List, and in INCLUDE mode the router forwards
them no more.

### Scenario constants

- The link with a source. S1 sends to G. Host A listens to G for S1 from 10 to 50 seconds; its
  stop at 50 seconds sends BLOCK({S1}), and no other node wants S1. A sends each State-Change
  Report once (Robustness Variable 1).
- Observation lasts 60 seconds from the start.

### Size or value arithmetic

The Last Listener Query Time is 2 seconds, so the forwarding of S1 stops 2 seconds after the
stop.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. Let A make the requests above.
3. Observe the Queries of R and the datagrams of S1 on L1 after 50 seconds.

### Expected observations

1. On L1, from R, after 50 seconds, a Multicast Address and Source Specific Query for G with S1.
   This confirms the stimulus.
2. Datagrams of S1 to G still arrive on L1 until 1.5 seconds after the stop, and none arrives
   later than 2.5 seconds after it (RFC9777-RQRY-2, RQRY-3, RST-25, RST-29, FWD-4).

## A router that comes up after a start of listening

Checks: **RFC9777-RREP-17** (description).

### Requirement

RFC 9777 §7.4.1: a router in INCLUDE(A) that receives a Current-State Record IS_EX(B) moves to
EXCLUDE(A ∩ B, B − A) and sets the Filter Timer to the Multicast Address Listening Interval; a
router with no state for the address is in INCLUDE({}), so it starts to forward the address.

### Scenario constants

- The link with a source. S1 sends to G. R leaves L1 at 1 second, after its first startup
  General Query, and joins L1 again at 20 seconds, so it misses the start of listening of A at
  10 seconds. Its next General Query, the second startup Query at 31.25 seconds, finds A a
  listener already.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up; break the link between R and
   L1 from 1 to 20 seconds.
2. At 10 seconds, let A start to listen to G.
3. Observe the Queries of R, the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, after the General Query of R at 31.25 seconds, a Report with a
   MODE_IS_EXCLUDE record for G. This confirms the stimulus.
2. From 1 second after that Report on, every datagram of S1 to G arrives on L1
   (RFC9777-RREP-17).
