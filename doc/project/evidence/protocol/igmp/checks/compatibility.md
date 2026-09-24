# IGMP — English check procedures: older versions

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9776/catalog.md](../../../standard/rfc9776/catalog.md), [rfc2236/catalog.md](../../../standard/rfc2236/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

An IGMPv2 message is 8 octets long. The IGMPv2 Membership Report has type 0x16 and goes to the
group that it reports; the IGMPv2 Leave Group message has type 0x17 and goes to 224.0.0.2; an
IGMPv2 Query has type 0x11, is 8 octets long, and has a Max Resp Code that is not zero.

## Host in IGMPv2 mode

Checks: **RFC9776-COMPH-1**, **GEN-8** (must), **VER-2**, **COMPH-6**, **COMPH-17**, **REP-35**,
**RFC2236-HOST-15**, **HOST-21**, **HOST-22**, **HOST-25**, **HOST-28**, **HOST-31**
(description); covers **RFC9776-COMPH-2** (must), **COMPH-3**, **COMPH-4**, **COMPH-7**,
**COMPH-13**, **COMPH-16**, **COMPH-21**, **TIMER-20**, **RFC2236-HOST-1**, **HOST-2**,
**HOST-3**, **HOST-6**, **HOST-9**, **HOST-32** (description), **RFC9776-COMPH-15** (should
(lower case)), **RFC2236-HOST-4**, **HOST-13** (may (lower case)).

### Requirement

RFC 9776 §7.1 and §7.2.1: a Query of 8 octets with a Max Resp Code that is not zero is an IGMPv2
Query; it starts the IGMPv2-Querier-Present Timer, and while that timer runs the host uses only
IGMPv2 on the interface. RFC 2236 §6: on a join, an IGMPv2 host sends a Report at once and again
when a timer of up to the Unsolicited Report Interval expires; on a Query it answers after a
random delay of up to the Max Response Time.

### Scenario constants

- The link with an older querier. R sends IGMPv2 General Queries at 0 and 31.25 seconds and every
  125 seconds after. At 40 seconds, host A joins G.
- Observation lasts 200 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up.
2. At 40 seconds, let A join G.
3. Observe the Queries of R and the IGMP messages of A on L1.

### Expected observations

1. On L1, from R, an IGMP Query of 8 octets with a Max Resp Code that is not zero. This confirms
   the stimulus.
2. On L1, from A, within 10 milliseconds of the join, an IGMPv2 Membership Report for G: type
   0x16, 8 octets, Group Address G, IPv4 destination G (RFC9776-VER-2, COMPH-6, COMPH-17, GEN-8,
   REP-35, RFC2236-HOST-15, HOST-25).
3. A second IGMPv2 Membership Report for G from A, at most 1 second after the first
   (RFC2236-HOST-22, HOST-31).
4. After the General Query of 156.25 seconds, and within its Max Response Time, an IGMPv2
   Membership Report for G from A that does not leave at the instant of the Query
   (RFC2236-HOST-21, HOST-28, HOST-31).
5. No Version 3 Membership Report leaves A in the window (RFC9776-COMPH-17).

## Report suppression in IGMPv2 mode

Checks: **RFC2236-HOST-29** (description); covers **RFC2236-HOST-10**, **HOST-12**, **HOST-19**,
**HOST-20**, **HOST-24** (description).

### Requirement

RFC 2236 §6: a host in the Delaying Member state that hears a Report for its group stops its
timer and sends no Report of its own.

### Scenario constants

- The link with an older querier. Hosts A and B join G at 40 seconds.
- The General Query under test is the one of 156.25 seconds.
- Observation lasts 170 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up.
2. At 40 seconds, let A and B join G.
3. Observe the Reports of A and B on L1 after 156.25 seconds.

### Expected observations

1. On L1, after the General Query of 156.25 seconds, an IGMPv2 Membership Report for G from A
   or B. This confirms the stimulus.
2. Within the Max Response Time of that Query, no second Report for G leaves A or B
   (RFC2236-HOST-29).

## Leave in IGMPv2 mode

Checks: **RFC9776-GEN-9** (must), **RFC2236-HOST-18**, **HOST-27** (description); covers
**RFC2236-HOST-5** (may (lower case)), **HOST-26** (description).

### Requirement

RFC 2236 §6: a host that leaves a group, and that was the last host to report it, sends a Leave
Group message to 224.0.0.2. RFC 9776 §4: a system supports the IGMPv2 Leave Group message.

### Scenario constants

- The link with an older querier. Host A joins G at 40 seconds and leaves it at 60 seconds; A is
  the only member of G.
- Observation lasts 70 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the IGMP messages of A on L1 after 60 seconds.

### Expected observations

1. On L1, from A, before 60 seconds, an IGMPv2 Membership Report for G. This confirms the
   stimulus.
2. On L1, from A, within 10 milliseconds of the leave, an IGMPv2 Leave Group message: type 0x17,
   8 octets, Group Address G (RFC9776-GEN-9, RFC2236-HOST-27).
3. Its IPv4 destination is 224.0.0.2 (RFC2236-HOST-18).

## Leave at an IGMPv2 router

Checks: **RFC2236-ROUTER-27**, **ROUTER-28**, **ROUTER-32**, **ROUTER-35**, **ROUTER-39**,
**ROUTER-43**, **ROUTER-44** (description); covers **RFC2236-ROUTER-15**, **ROUTER-16**,
**ROUTER-18**, **ROUTER-19**, **ROUTER-22**, **ROUTER-24**, **ROUTER-25**, **ROUTER-30**,
**ROUTER-33**, **ROUTER-34**, **ROUTER-38** (description).

### Requirement

RFC 2236 §7: a querier that receives a Version 2 Report for a group starts its timer and tells
the routing protocol that the network has members; a Leave in Members Present sends a
Group-Specific Query with the Max Response Time of the Last Member Query Interval, sets the
timer to [Last Member Query Interval] × [Last Member Query Count], and repeats the Query when
the retransmit timer expires; when the timer expires, the router tells the routing protocol
that no members remain.

### Scenario constants

- The link with an older querier. S1 sends to G. Host A joins G at 40 seconds and leaves it at
  60 seconds.
- Observation lasts 70 seconds from the start.

### Procedure

1. Build the link with an older querier, and let R, A, B and S1 come up.
2. Let A make the requests above.
3. Observe the Queries of R and the datagrams of S1 on L1.

### Expected observations

1. On L1, from A, at 60 seconds, an IGMPv2 Leave Group message for G. This confirms the
   stimulus.
2. From 1 second after the first Report of A to the leave, every datagram of S1 to G arrives on
   L1 (RFC2236-ROUTER-27, ROUTER-35).
3. On L1, from R, within 0.1 seconds of the Leave, an IGMPv2 Group-Specific Query: 8 octets,
   Group Address G, IPv4 destination G, Max Resp Code 10 (RFC2236-ROUTER-32, ROUTER-39).
4. A second such Query 1 second after the first, and no third one (RFC2236-ROUTER-43).
5. No datagram of S1 to G arrives on L1 later than 2.5 seconds after the Leave, and datagrams
   still arrive until 1.5 seconds after it (RFC2236-ROUTER-28, ROUTER-44).

## Host back to IGMPv3

Checks: **RFC9776-COMPH-9** (description), **TIMER-22** (should); covers **RFC9776-COMPH-10**,
**COMPH-12**, **TIMER-19** (description), **TIMER-21** (should).

### Requirement

RFC 9776 §7.2.1 and §8.12: when the IGMPv2-Querier-Present Timer expires, the host goes back to
IGMPv3. The Older Version Querier Present Interval is [Robustness Variable] × [Query Interval] +
10 × the Max Response Time of the last older Query, with the default values.

### Scenario constants

- The link with an older querier. R stops at 100 seconds, after its IGMPv2 General Queries of 0
  and 31.25 seconds. Host A joins G at 370 seconds and H at 400 seconds.
- Observation lasts 420 seconds from the start.

### Size or value arithmetic

The interval is 2 × 125 + 10 × 10 = 350 seconds after the Query of 31.25 seconds, so A is in
IGMPv2 mode until 381.25 seconds and in IGMPv3 mode after it.

### Procedure

1. Build the link with an older querier, and let R, A and B come up; at 100 seconds, stop R.
2. Let A make the requests above.
3. Observe the Reports of A on L1 after 370 seconds.

### Expected observations

1. On L1, from A, at 370 seconds, an IGMPv2 Membership Report for G. This confirms the stimulus:
   A is still in IGMPv2 mode.
2. On L1, from A, at 400 seconds, a Version 3 Membership Report with a record for H, and no
   IGMPv2 Report for H (RFC9776-COMPH-9, TIMER-22).

## Router with an IGMPv2 member

Checks: **RFC9776-COMPR-13** (must), **COMPR-17**, **COMPR-27**, **COMPR-28** (description);
covers **RFC9776-RQ-5**, **GEN-8**, **GEN-9** (must), **COMPR-14**, **COMPR-15**, **COMPR-18**,
**COMPR-22**, **COMPR-23**, **COMPR-26**, **TIMER-23**, **TIMER-24** (description), **COMPR-25**
(should).

### Requirement

RFC 9776 §7.3.2: an IGMPv2 Report sets the IGMPv2-Host-Present Timer of the group and puts the
group in IGMPv2 compatibility mode; in that mode the router treats a Version 2 Report as
IS_EX({}) and a Leave Group message as TO_IN({}).

### Scenario constants

- The link with an older host. S1 sends to G. Host C joins G at 10 seconds and leaves it at 50
  seconds; C is the only member of G.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link with an older host, and let R, A, B, C and S1 come up.
2. Let C make the requests above.
3. Observe the IGMP messages and the datagrams of S1 on L1.

### Expected observations

1. On L1, from C, at 10 seconds, an IGMPv2 Membership Report for G. This confirms the stimulus.
2. From 1 second after that Report to the leave, every datagram of S1 to G arrives on L1
   (RFC9776-COMPR-17, COMPR-27).
3. On L1, from R, within 0.1 seconds of the Leave of C, a Group-Specific Query for G, and no
   datagram of S1 to G later than 2.5 seconds after the Leave (RFC9776-COMPR-28).

## Querier with an IGMPv2 router

Checks: **RFC9776-RQRY-8**, **COMPR-1**, **COMPR-6**, **COMPR-8** (must); covers
**RFC9776-COMPR-2** (must).

### Requirement

RFC 9776 §6.6.2 and §7.3.1: a router that receives an older General Query uses the oldest IGMP
version of the network; the querier uses the lowest version among the routers. In IGMPv2 mode
it sends Queries of 8 octets with the Max Response Time in the Max Resp Code.

### Scenario constants

- The two routers of different versions. R1 (IGMPv3, 10.0.1.1) comes up at the start, R2
  (IGMPv2, 10.0.1.2) at 5 seconds, and sends its startup General Query then. R1 has the lower
  address and stays the querier.
- Observation lasts 200 seconds from the start.

### Procedure

1. Build the two routers of different versions, R2 down until 5 seconds.
2. Observe the General Queries of R1 and R2 on L1.

### Expected observations

1. On L1, from R2, at 5 seconds, a General Query of 8 octets. This confirms the stimulus.
2. Every General Query of R1 after that Query is 8 octets long (RFC9776-RQRY-8, COMPR-1,
   COMPR-6).
3. Its Max Resp Code is 100, the Max Response Time in tenths of a second without the exponential
   code (RFC9776-COMPR-8).

## A mode change cancels the pending reports

Checks: **RFC9776-COMPH-22** (description).

### Requirement

RFC 9776 §7.2.1: when a host changes its compatibility mode, it cancels all its pending response
and retransmission timers.

### Scenario constants

- The link with an older querier, with R down until 10.5 seconds. Host A joins G at 10.0
  seconds, in IGMPv3 mode, and has a repetition of its State-Change Report pending up to 1 second
  later. The IGMPv2 General Query of R at 10.5 seconds switches A to IGMPv2 mode.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with an older querier, with R down until 10.5 seconds; let A and B come up.
2. At 10.0 seconds, let A join G.
3. Observe the Queries of R and the Reports of A on L1.

### Expected observations

1. On L1, from A, at 10.0 seconds, a Version 3 Membership Report with a CHANGE_TO_EXCLUDE_MODE
   record for G, and from R, at 10.5 seconds, an IGMPv2 General Query. This confirms the
   stimulus.
2. No Version 3 Membership Report leaves A after the Query (RFC9776-COMPH-22).

### Notes

- The random repetition may leave A before 10.5 seconds; then the check has no verdict, and its
  run says so. The check reads only the Reports after the Query.

## A BLOCK record for a group in IGMPv2 mode

Checks: **RFC9776-COMPR-29** (description).

### Requirement

RFC 9776 §7.3.2: while a group is in IGMPv2 compatibility mode, the router ignores the
BLOCK_OLD_SOURCES records of IGMPv3 hosts for it.

### Scenario constants

- The link with an older host. S1 sends to G. Host C joins G at 10 seconds, so the group is in
  IGMPv2 mode. Host A joins G for S1 from 10 to 30 seconds; its leave at 30 seconds sends
  BLOCK({S1}).
- Observation lasts 40 seconds from the start.

### Procedure

1. Build the link with an older host, and let R, A, B, C and S1 come up.
2. Let A and C make the requests above.
3. Observe the Reports of A, the Queries of R and the datagrams of S1 on L1 after 30 seconds.

### Expected observations

1. On L1, from A, at 30 seconds, a Version 3 Membership Report with a BLOCK_OLD_SOURCES record
   for G with S1. This confirms the stimulus.
2. No Group-and-Source-Specific Query for G leaves R after that Report (RFC9776-COMPR-29).
3. From 30 to 40 seconds, datagrams of S1 to G arrive on L1 with no gap longer than 1 second.

