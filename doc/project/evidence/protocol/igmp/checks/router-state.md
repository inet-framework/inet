# IGMP — English check procedures: state and forwarding of a router

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9776/catalog.md](../../../standard/rfc9776/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Forwarding after a join

Checks: **RFC9776-FWD-8**, **RREP-22** (description); covers **RFC9776-FWD-1** (should (lower
case)), **RST-3**, **RST-4** (description), **RQ-2** (must).

### Requirement

RFC 9776 §6.4.2 and §6.3: a router in INCLUDE({}) that receives TO_EX({}) moves to EXCLUDE({},
{}); in EXCLUDE mode a source without a source record is forwarded, and with no group record at
all nothing is forwarded.

### Scenario constants

- The link with a source. S1 sends to G. At 10 seconds, host A joins G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. At 10 seconds, let A join G.
3. Observe the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, at 10 seconds, a Report with a CHANGE_TO_EXCLUDE_MODE record for G. This
   confirms the stimulus.
2. Before that Report, no datagram of S1 to G arrives on L1: without a group record, nothing
   is forwarded.
3. From 1 second after that Report on, every datagram of S1 to G arrives on L1 (RFC9776-FWD-8,
   RREP-22).

## Source-specific forwarding

Checks: **RFC9776-FWD-3**, **FWD-5**, **RREP-20** (description); covers **RFC9776-RST-6** (must
(lower case)), **RST-8**, **RST-13**, **RST-1**, **RST-2**, **RST-11**, **RST-12** (description).

### Requirement

RFC 9776 §6.4.2 and §6.3: a router in INCLUDE(A) that receives ALLOW(B) moves to INCLUDE(A + B)
with the Source Timers of B at the Group Membership Interval; in INCLUDE mode a source with a
running timer is forwarded, and a source without a record is not.

### Scenario constants

- The link with a source. S1 and S2 send to G. At 10 seconds, host A joins G for S1.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B, S1 and S2 come up.
2. At 10 seconds, let A join G for S1.
3. Observe the Reports of A and the datagrams of S1 and S2 on L1.

### Expected observations

1. On L1, from A, at 10 seconds, a Report with an ALLOW_NEW_SOURCES record for G with S1. This
   confirms the stimulus.
2. From 1 second after that Report on, every datagram of S1 to G arrives on L1 (RFC9776-FWD-3,
   RREP-20).
3. No datagram of S2 to G arrives on L1 in the window (RFC9776-FWD-5).

## Membership timeout without a leave

Checks: **RFC9776-RREP-11**, **RST-10** (description), **TIMER-8** (must); covers
**RFC9776-RREP-5**, **RREP-6**, **RST-7**, **RST-9** (description), **TIMER-7** (must (lower
case)).

### Requirement

RFC 9776 §6.4.1, §6.2.2 and §8.4: a Current-State Record IS_EX({}) sets the Group Timer to the
Group Membership Interval; when the Group Timer of an EXCLUDE group expires and no Source Timer
runs, the router deletes the group record and forwards the group no more. The Group Membership
Interval is [Robustness Variable] × [Query Interval] + 2 × [Query Response Interval].

### Scenario constants

- The link with a source. S1 sends to G. At 10 seconds, host A joins G; A answers the General
  Queries of R; at 45 seconds, A leaves L1 without a leave.
- Observation lasts 350 seconds from the start.

### Size or value arithmetic

The Group Membership Interval is 2 × 125 + 2 × 10 = 270 seconds. A answers the General Query of
31.25 seconds within 10 seconds, so its last Report comes before 45 seconds, and the forwarding
stops about 270 seconds after that Report, between 280 and 315 seconds.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. At 10 seconds, let A join G; at 45 seconds, break the link between A and L1.
3. Observe the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, before 45 seconds, a Report with a Current-State Record MODE_IS_EXCLUDE for G.
   Record the instant of the last Report of A. This confirms the stimulus.
2. Datagrams of S1 to G arrive on L1 until 270 seconds after that Report, with a margin of 1
   second (RFC9776-RREP-11, TIMER-8).
3. No datagram of S1 to G arrives on L1 later than 1 second after that instant (RFC9776-RST-10,
   TIMER-8).

### Notes

- RFC 2236 and RFC 3376 gave the interval as one Query Response Interval less, 260 seconds; RFC
  9776 governs, as the override table of [`standards.md`](../standards.md#override-table) says.

## Leave and the last member query

Checks: **RFC9776-RQ-7**, **RQRY-1**, **RQRY-10** (must (lower case)), **RQ-8**, **RREP-27**,
**RQRY-3**, **RQRY-11**, **RREP-18**, **QRY-8**, **QRY-26**, **QRY-29**, **TIMER-13**,
**TIMER-16** (description); covers **RFC9776-RSW-1**, **RSW-2**, **RREP-7**, **RREP-15**
(description), **RREP-17**, **TIMER-17** (may (lower case)), **RQRY-9** (must (lower case)).

### Requirement

RFC 9776 §6.4.2 and §6.6: a router in EXCLUDE mode that receives TO_IN({}) sends a Group-Specific
Query at once and [Last Member Query Count] − 1 more, [Last Member Query Interval] apart, to the
group address, and lowers the Group Timer to the Last Member Query Time; without an answer the
group times out, and the router forwards it no more. The Suppress Router-Side Processing flag
is set only when the Group Timer is above the Last Member Query Time.

### Scenario constants

- The link with a source. S1 sends to G. At 10 seconds, host A joins G; at 50 seconds, A leaves
  G. A is the only member of G. A sends each State-Change Report once (Robustness Variable 1).
- Observation lasts 60 seconds from the start.

### Size or value arithmetic

The Last Member Query Time is 2 × 1 = 2 seconds, so the forwarding stops 2 seconds after the
leave.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. At 10 seconds, let A join G; at 50 seconds, let A leave G.
3. Observe the Queries of R and the datagrams of S1 on L1 after 50 seconds.

### Expected observations

1. On L1, from A, at 50 seconds, a Report with a CHANGE_TO_INCLUDE_MODE record for G. This
   confirms the stimulus.
2. On L1, from R, within 0.1 seconds of that Report, a Group-Specific Query to G: Group Address
   G and no sources (RFC9776-RQ-7, RQ-8, RREP-27, RQRY-10, QRY-8, QRY-26, QRY-29).
3. A second Group-Specific Query for G 1 second after the first, and no third one
   (RFC9776-RREP-18, RQRY-10, TIMER-16).
4. Datagrams of S1 to G still arrive on L1 until 1.5 seconds after the leave, and none arrives
   later than 2.5 seconds after it (RFC9776-RQRY-1, RQRY-3, RREP-27).
5. Each of the two Queries has Max Resp Code 10, a Max Response Time of 1 second
   (RFC9776-TIMER-13).
6. Each of the two Queries has the S flag clear, because the Group Timer is not above the Last
   Member Query Time (RFC9776-RQRY-11).

## Leave with another member

Checks: **RFC9776-RREP-16** (description).

### Requirement

RFC 9776 §6.4.2: an EXCLUDE-mode record that arrives during the query of a group updates the
Group Timer, and the router forwards the group without a break.

### Scenario constants

- The link with a source. S1 sends to G. Hosts A and B join G at 10 seconds; B leaves G at 50
  seconds, and A answers the Group-Specific Query.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. Let A and B make the requests above.
3. Observe the Queries of R, the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, after the first Group-Specific Query for G, a Report with a MODE_IS_EXCLUDE
   record for G. This confirms the stimulus.
2. From 50 to 60 seconds, a datagram of S1 to G arrives on L1 every 0.5 seconds, with no gap
   longer than 1 second (RFC9776-RREP-16).

## Source blocked while another member wants it

Checks: **RFC9776-RQ-9**, **RREP-14**, **RREP-21**, **RQRY-14**, **QRY-27** (description),
**RQRY-13** (must (lower case)); covers **RFC9776-RREP-13**, **RQRY-15**, **RQ-10**
(description), **RQRY-12** (must (lower case)).

### Requirement

RFC 9776 §6.4.2 and §6.6.3.2: a router in INCLUDE(A) that receives BLOCK(B) keeps INCLUDE(A)
and sends Group-and-Source-Specific Queries for A ∩ B: one at once and [Last Member Query Count]
− 1 more, [Last Member Query Interval] apart; it lowers the Source Timers of those sources to the
Last Member Query Time, and sends a query with the S flag set only for sources whose timer is
above it.

### Scenario constants

- The link with a source. S1 sends to G. Host A joins G for S1 and S2 from 10 seconds on; host B
  joins G for S1 from 10 to 50 seconds, so that its leave at 50 seconds sends BLOCK({S1}). A
  answers the queries for S1. B sends each State-Change Report once (Robustness Variable 1).
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. Let A and B make the requests above.
3. Observe the Queries of R and the datagrams of S1 on L1 after 50 seconds.

### Expected observations

1. On L1, from B, at 50 seconds, a Report with a BLOCK_OLD_SOURCES record for G with S1. This
   confirms the stimulus.
2. On L1, from R, within 0.1 seconds of that Report, a Group-and-Source-Specific Query to G with
   Group Address G and the source S1 only (RFC9776-RQ-9, RREP-21, RQRY-13, QRY-27).
3. A second such Query 1 second after the first, and no third one (RFC9776-RQRY-13).
4. Datagrams of S1 to G arrive on L1 from 50 to 60 seconds with no gap longer than 1 second,
   because A answers (RFC9776-RREP-14).
5. The S flag is clear in the first such Query, because the router lowers the Source Timer of S1
   to the Last Member Query Time before it builds the Query. The S flag of the second Query is
   set if the answer of A came before it, because that answer raises the Source Timer of S1 to
   the Group Membership Interval, and clear if not (RFC9776-RQRY-14).

## Source blocked by its only member

Checks: **RFC9776-RQRY-2** (description), **RREP-12** (must (lower case)); covers
**RFC9776-FWD-4**, **RST-14** (description).

### Requirement

RFC 9776 §6.6.1: sending a Group-and-Source-Specific Query for sources A lowers their Source
Timers to the Last Member Query Time; when no record of interest arrives, the timers expire, and
in INCLUDE mode the router forwards those sources no more.

### Scenario constants

- The link with a source. S1 sends to G. Host A joins G for S1 from 10 to 50 seconds; its leave
  at 50 seconds sends BLOCK({S1}), and no other system wants S1. A sends each State-Change Report
  once (Robustness Variable 1).
- Observation lasts 60 seconds from the start.

### Size or value arithmetic

The Last Member Query Time is 2 seconds, so the forwarding of S1 stops 2 seconds after the
leave.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up.
2. Let A make the requests above.
3. Observe the Queries of R and the datagrams of S1 on L1 after 50 seconds.

### Expected observations

1. On L1, from R, after 50 seconds, a Group-and-Source-Specific Query for G with S1. This
   confirms the stimulus.
2. Datagrams of S1 to G still arrive on L1 until 1.5 seconds after the leave, and none arrives
   later than 2.5 seconds after it (RFC9776-RQRY-2).

## A router that comes up after a join

Checks: **RFC9776-RREP-9** (description).

### Requirement

RFC 9776 §6.4.1: a router in INCLUDE(A) that receives a Current-State Record IS_EX(B) moves to
EXCLUDE(A ∩ B, B − A) and sets the Group Timer to the Group Membership Interval; a router with no
state for the group is in INCLUDE({}), so it starts to forward the group.

### Scenario constants

- The link with a source. S1 sends to G. R leaves L1 at 1 second, after its first startup
  General Query, and joins L1 again at 20 seconds, so it misses the join of A at 10 seconds. Its
  next General Query, the second startup Query at 31.25 seconds, finds A a member already.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with a source, and let R, A, B and S1 come up; break the link between R and
   L1 from 1 to 20 seconds.
2. At 10 seconds, let A join G.
3. Observe the Queries of R, the Reports of A and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, after the General Query of R at 31.25 seconds, a Report with a
   MODE_IS_EXCLUDE record for G. This confirms the stimulus.
2. From 1 second after that Report on, every datagram of S1 to G arrives on L1
   (RFC9776-RREP-9).

