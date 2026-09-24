# MLD — English check procedures: interoperation with MLDv1

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9777/catalog.md](../../../standard/rfc9777/catalog.md), [rfc2710/catalog.md](../../../standard/rfc2710/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Listener in MLDv1 mode

Checks: **RFC9777-COMPL-1** (must), **COMPL-4**, **COMPL-9**, **REP-41**, **RFC2710-NODE-10**,
**NODE-13**, **NODE-14**, **NODE-17**, **NODE-20**, **NODE-22** (description), **RFC9777-GEN-7**
(must (lower case)); covers **RFC9777-VER-1**, **COMPL-3**, **COMPL-5**, **COMPL-6**,
**COMPL-8**, **COMPL-10**, **TIMER-16**, **RFC2710-NODE-1**, **NODE-4**, **NODE-15**,
**NODE-16**, **NODE-24**, **TIMER-5**, **TIMER-6**, **TIMER-12**, **TIMER-13** (description),
**RFC9777-COMPL-2** (must), **RFC2710-NODE-3**, **TIMER-7** (must (lower case)).

### Requirement

RFC 9777 §8.1 and §8.2.1: a Query of 24 octets is an MLDv1 Query; an MLDv1 General Query sets the
Host Compatibility Mode of the interface to MLDv1 and starts the Older-Version-Querier-Present
Timer, and while that timer runs the node uses only MLDv1 on the interface. RFC 2710 §5: on a
start of listening, an MLDv1 node sends a Report to the address at once and again when a timer
of up to the Unsolicited Report Interval expires; on a Query it answers after a random delay of
up to the Maximum Response Delay.

### Scenario constants

- The link with an older querier. R sends MLDv1 General Queries at 0 and 31.25 seconds and every
  125 seconds after. At 40 seconds, host A starts to listen to G. The Unsolicited Report
  Interval of A is 1 second, the default of RFC 9777 §9.11.
- Observation lasts 200 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up.
2. At 40 seconds, let A start to listen to G.
3. Observe the Queries of R and the MLD messages of A on L1.

### Expected observations

1. On L1, from R, a Multicast Listener Query of 24 octets. This confirms the stimulus.
2. On L1, from A, within 10 milliseconds of the start of listening, an MLDv1 Multicast Listener
   Report for G: ICMPv6 type 131, 24 octets, Multicast Address G, IPv6 destination G
   (RFC9777-VER-1, COMPL-1, COMPL-4, COMPL-9, GEN-7, REP-41, RFC2710-NODE-10, NODE-17).
3. A second MLDv1 Report for G from A, at most 1 second after the first (RFC2710-NODE-14,
   NODE-22).
4. After the General Query of 156.25 seconds, and within its Maximum Response Delay, an MLDv1
   Report for G from A that does not leave at the instant of the Query (RFC2710-NODE-13,
   NODE-20, NODE-22).
5. No Version 2 Report leaves A in the window (RFC9777-COMPL-9).

## Report suppression in MLDv1 mode

Checks: **RFC2710-NODE-21** (description); covers **RFC2710-NODE-7**, **NODE-8** (description).

### Requirement

RFC 2710 §5: a node in the Delaying Listener state that hears a Report for its address stops its
timer, clears its flag, and sends no Report of its own.

### Scenario constants

- The link with an older querier. Hosts A and B start to listen to G at 40 seconds.
- The General Query under test is the one of 156.25 seconds.
- Observation lasts 170 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up.
2. At 40 seconds, let A and B start to listen to G.
3. Observe the Reports of A and B on L1 after 156.25 seconds.

### Expected observations

1. On L1, after the General Query of 156.25 seconds, an MLDv1 Report for G from A or B. This
   confirms the stimulus.
2. Within the Maximum Response Delay of that Query, no second Report for G leaves A or B
   (RFC2710-NODE-21).

## Done in MLDv1 mode

Checks: **RFC2710-NODE-12**, **NODE-19** (description); covers **RFC2710-NODE-11** (may).

### Requirement

RFC 2710 §5: a node that stops listening to an address, and that was the last node to report it,
sends a Done message to the link-scope all-routers address ff02::2.

### Scenario constants

- The link with an older querier. Host A starts to listen to G at 40 seconds and stops at 60
  seconds; A is the only listener of G.
- Observation lasts 70 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the MLD messages of A on L1 after 60 seconds.

### Expected observations

1. On L1, from A, before 60 seconds, an MLDv1 Report for G. This confirms the stimulus.
2. On L1, from A, within 10 milliseconds of the stop, an MLDv1 Done message: ICMPv6 type 132, 24
   octets, Multicast Address G (RFC2710-NODE-19).
3. Its IPv6 destination is ff02::2 (RFC2710-NODE-12).

## Done at an MLDv1 router

Checks: **RFC2710-ROUTER-21**, **ROUTER-26**, **ROUTER-28**, **ROUTER-29**, **ROUTER-31**
(description); covers **RFC2710-ROUTER-11**, **ROUTER-12**, **ROUTER-15**, **ROUTER-18**,
**ROUTER-19**, **ROUTER-20**, **ROUTER-22**, **ROUTER-23**, **ROUTER-24**, **TIMER-2**,
**TIMER-3**, **TIMER-14**, **TIMER-15** (description).

### Requirement

RFC 2710 §6: a querier that receives a Report for an address starts its timer and tells the
routing protocol that the link has listeners; a Done in Listeners Present sends a Multicast
Address Specific Query with the Maximum Response Delay of the Last Listener Query Interval, sets
the timer to [Last Listener Query Interval] × [Last Listener Query Count], and repeats the Query
when the retransmit timer expires; when the timer expires, the router tells the routing protocol
that no listeners remain.

### Scenario constants

- The link with an older querier. S1 sends to G. Host A starts to listen to G at 40 seconds and
  stops at 60 seconds.
- Observation lasts 70 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A, B and S1 come up.
2. Let A make the requests above.
3. Observe the Queries of R and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, at 60 seconds, an MLDv1 Done message for G. This confirms the stimulus.
2. From 1 second after the first Report of A to the Done, every datagram of S1 to G arrives on
   L1 (RFC2710-ROUTER-26).
3. On L1, from R, within 0.1 seconds of the Done, an MLDv1 Multicast Address Specific Query: 24
   octets, Multicast Address G, IPv6 destination G, Maximum Response Code 1000
   (RFC2710-ROUTER-21, ROUTER-31).
4. A second such Query 1 second after the first, and no third one (RFC2710-ROUTER-29).
5. No datagram of S1 to G arrives on L1 later than 2.5 seconds after the Done, and datagrams
   still arrive until 1.5 seconds after it (RFC2710-ROUTER-28, ROUTER-31).

## Listener back to MLDv2

Checks: **RFC9777-COMPL-7** (description), **TIMER-17** (must).

### Requirement

RFC 9777 §8.2.1 and §9.12: when the Older-Version-Querier-Present Timer expires, the node goes
back to MLDv2. The Older Version Querier Present Interval is [Robustness Variable] × [Query
Interval] of the last Query + [Query Response Interval].

### Scenario constants

- The link with an older querier. R leaves L1 at 100 seconds, after its MLDv1 General Queries of
  0 and 31.25 seconds. Host A starts to listen to K at 40 seconds, to G at 280 seconds and to H
  at 300 seconds.
- Observation lasts 320 seconds from the start.

### Size or value arithmetic

The interval is 2 × 125 + 10 = 260 seconds after the Query of 31.25 seconds, so A is in MLDv1
mode until 291.25 seconds and in MLDv2 mode after it.

### Procedure

1. Build the link with an older querier, and let R, A and B come up; at 100 seconds, break the
   link between R and L1.
2. Let A make the requests above.
3. Observe the Queries of R and the Reports of A on L1.

### Expected observations

1. On L1, from A, at 40 seconds, an MLDv1 Report for K. This confirms the stimulus: A is in
   MLDv1 mode.
2. On L1, from A, at 280 seconds, an MLDv1 Report for G: A is still in MLDv1 mode, 248.75 seconds
   after the last MLDv1 Query (RFC9777-TIMER-17).
3. On L1, from A, at 300 seconds, a Version 2 Report with a record for H, and no MLDv1 Report for
   H (RFC9777-COMPL-7, TIMER-17).

## Router with an MLDv1 listener

Checks: **RFC9777-COMPR-12** (must), **COMPR-14**, **COMPR-20**, **COMPR-21**, **COMPR-23**
(description); covers **RFC9777-COMPR-13**, **COMPR-15**, **COMPR-16**, **COMPR-19**,
**TIMER-18** (description).

### Requirement

RFC 9777 §8.3.2: an MLDv1 Report sets the Older-Version-Host-Present Timer of the address and
puts the address in MLDv1 compatibility mode; in that mode the router treats an MLDv1 Report as
IS_EX({}) and a Done as TO_IN({}), and the querier goes on with MLDv2 Queries.

### Scenario constants

- The link with an older host. S1 sends to G. Host C starts to listen to G at 10 seconds and
  stops at 50 seconds; C is the only listener of G.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with an older host, and let R, A, B, C and S1 come up.
2. Let C make the requests above.
3. Observe the MLD messages and the datagrams of S1 on L1.

### Expected observations

1. On L1, from C, at 10 seconds, an MLDv1 Report for G. This confirms the stimulus.
2. From 1 second after that Report to the stop, every datagram of S1 to G arrives on L1
   (RFC9777-COMPR-12, COMPR-14, COMPR-20).
3. On L1, from R, within 0.1 seconds of the Done of C, a Multicast Address Specific Query for G,
   and no datagram of S1 to G later than 2.5 seconds after the Done (RFC9777-COMPR-21).
4. Every Query of R in the window is an MLDv2 Query of at least 28 octets (RFC9777-COMPR-23).

## Querier configured for an MLDv1 router

Checks: **RFC9777-COMPR-1**, **COMPR-3**, **COMPR-5**, **COMPR-7** (must), **COMPR-4** (must
(lower case)); covers **RFC9777-COMPR-2** (must (lower case)).

### Requirement

RFC 9777 §8.3.1: when an MLDv1 router is on the link, the querier uses the lowest MLD version.
This is administratively assured: an MLDv2 router that wants MLDv1 compatibility has a
configuration option to act in MLDv1 mode, and the administrator configures every MLDv2 router
of the link into it. In MLDv1 mode the querier sends General Queries of 24 octets, with the
Maximum Response Delay in the Maximum Response Code directly.

### Scenario constants

- The two routers of different versions, up from the start. R1 (MLDv2, the lower interface
  identifier) is configured to act in MLDv1 mode; R2 runs MLDv1 and sends its startup General
  Query at the start.
- Observation lasts 200 seconds from the start.

### Procedure

1. Build the two routers of different versions, with R1 configured in MLDv1 mode.
2. Observe the General Queries of R1 and R2 on L1.

### Expected observations

1. On L1, from R2, at the start, a General Query of 24 octets. This confirms the stimulus.
2. Every General Query of R1 is 24 octets long (RFC9777-COMPR-1, COMPR-3, COMPR-4, COMPR-5).
3. Its Maximum Response Code is 10000, the Maximum Response Delay in milliseconds without the
   exponential code (RFC9777-COMPR-7).

### Notes

- With the default of 10 seconds, the linear and the exponential codes give the same value, so
  observation 3 cannot tell them apart; a Maximum Response Delay of 32768 milliseconds or more
  would, which is the owed check of the code fields.

## A mode change cancels the pending reports

Checks: **RFC9777-COMPL-11** (description).

### Requirement

RFC 9777 §8.2.1: when a node changes its compatibility mode, it cancels all its pending
responses and retransmission timers.

### Scenario constants

- The link with an older querier. R leaves L1 at 1 second, after its MLDv1 General Query at the
  start, and joins L1 again at 406.22 seconds, after the Report of A and before its own General
  Query of 406.25 seconds. A is in MLDv2 mode again 260 seconds after the Query at the start
  (RFC9777-TIMER-17). Host A starts to listen to G at 406.2 seconds, in MLDv2 mode, and has a
  repetition of its State-Change Report pending up to 1 second later. The MLDv1 General Query
  of R at 406.25 seconds switches A to MLDv1 mode.
- Observation lasts 420 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up; break the link between R
   and L1 from 1 to 406.22 seconds.
2. At 406.2 seconds, let A start to listen to G.
3. Observe the Queries of R and the Reports of A on L1.

### Expected observations

1. On L1, from A, at 406.2 seconds, a Version 2 Report with a CHANGE_TO_EXCLUDE_MODE record for
   G, and from R, at 406.25 seconds, an MLDv1 General Query. This confirms the stimulus.
2. No Version 2 Report leaves A after the Query (RFC9777-COMPL-11).

### Notes

- The random repetition may leave A before 406.25 seconds; then the check has no verdict, and its
  run says so. The check reads only the Reports after the Query.
- R is an MLDv1 router, and a Version 2 Report is not for R. The check keeps R off L1 while A
  sends it, so that the check reads the listener alone.

## A BLOCK record for an address in MLDv1 mode

Checks: **RFC9777-COMPR-22** (description).

### Requirement

RFC 9777 §8.3.2: while an address is in MLDv1 compatibility mode, the router ignores the
BLOCK_OLD_SOURCES records of MLDv2 nodes for it.

### Scenario constants

- The link with an older host. S1 sends to G. Host C starts to listen to G at 10 seconds, so the
  address is in MLDv1 mode. Host A listens to G for S1 from 10 to 30 seconds; its stop at 30
  seconds sends BLOCK({S1}).
- Observation lasts 40 seconds from the start.

### Procedure

1. Build the link with an older host, and let R, A, B, C and S1 come up.
2. Let A and C make the requests above.
3. Observe the Reports of A, the Queries of R and the datagrams of S1 on L1 after 30 seconds.

### Expected observations

1. On L1, from A, at 30 seconds, a Version 2 Report with a BLOCK_OLD_SOURCES record for G with
   S1. This confirms the stimulus.
2. No Multicast Address and Source Specific Query for G leaves R after that Report
   (RFC9777-COMPR-22).
3. From 30 to 40 seconds, datagrams of S1 to G arrive on L1 with no gap longer than 1 second.
