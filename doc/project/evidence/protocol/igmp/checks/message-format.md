# IGMP — English check procedures: message format

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9776/catalog.md](../../../standard/rfc9776/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Membership Report encapsulation

Checks: **RFC9776-GEN-1**, **GEN-2**, **GEN-6**, **REP-2**, **REP-4**, **REP-7**, **REP-31**,
**REP-34** (description), **REP-14**, **REP-17** (must); covers **RFC9776-REP-6**, **REP-1**,
**REP-13** (description), **REP-9** (may (lower case)).

### Requirement

RFC 9776 §4 and §4.2: an IGMP message travels in an IPv4 datagram of protocol 2 with TTL 1. A
Version 3 Membership Report has type 0x22, zero in its Reserved field, the number of its Group
Records, no auxiliary data and no octets after the last record; it leaves from a unicast address
of the subnet to 224.0.0.22, and its checksum is the one's complement of the one's complement
sum of the whole message.

### Scenario constants

- The link. At 10 seconds, host A joins G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A join G.
3. Observe the IGMP messages of A on L1.

### Expected observations

1. On L1, from A, after 10 seconds, a Version 3 Membership Report, IGMP type 0x22
   (RFC9776-GEN-6). This confirms the stimulus.
2. Its IPv4 protocol is 2 and its TTL is 1 (RFC9776-GEN-1, GEN-2).
3. Its IPv4 destination is 224.0.0.22, and its source is 10.0.1.10, the address of A on L1
   (RFC9776-REP-31, REP-34).
4. Its Number of Group Records is the number of records that follow, each record has Aux Data
   Len zero, and no octet follows the last record (RFC9776-REP-7, REP-14, REP-17).
5. Its Reserved field is zero (RFC9776-REP-2), and its checksum is the one's complement of the
   one's complement sum of the whole message (RFC9776-REP-4).

## Membership Query encapsulation

Checks: **RFC9776-GEN-5**, **QRY-3**, **QRY-5**, **QRY-7**, **QRY-12**, **QRY-15**, **QRY-16**,
**QRY-20**, **QRY-25**, **QRY-28**, **VER-3**, **TIMER-5** (description), **QRY-24** (must);
covers **RFC9776-QRY-1**, **QRY-2**, **QRY-9**, **QRY-19**, **QRY-22**, **GEN-1**, **GEN-2**
(description).

### Requirement

RFC 9776 §4.1 and §8: a General Query of IGMPv3 has type 0x11, is at least 12 octets long,
holds group address zero and no sources, and goes to 224.0.0.1. Its Max Resp Code below 128 is
the Max Response Time in tenths of a second, 100 by default; QRV holds the Robustness Variable
of the querier and QQIC its Query Interval, both below 128 as plain values.

### Scenario constants

- The link, with the default of every value. Observation lasts 10 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the IGMP messages of R on L1.

### Expected observations

1. On L1, from R, a Membership Query, IGMP type 0x11 (RFC9776-GEN-5). This confirms the
   stimulus.
2. It is at least 12 octets long, so it is an IGMPv3 Query (RFC9776-VER-3), and no octet
   follows its last source address (RFC9776-QRY-24).
3. Its IPv4 destination is 224.0.0.1, its Group Address is zero and its Number of Sources is
   zero: a General Query (RFC9776-QRY-7, QRY-20, QRY-25, QRY-28).
4. Its Max Resp Code is 100, a Max Response Time of 10 seconds (RFC9776-QRY-3, TIMER-5).
5. Its QRV is 2 and its QQIC is 125 (RFC9776-QRY-12, QRY-15, QRY-16).
6. Its checksum is the one's complement of the one's complement sum of the whole message
   (RFC9776-QRY-5).

## Router Alert and precedence on every message

Checks: **RFC9776-GEN-4**, **GEN-3** (description).

### Requirement

RFC 9776 §4: every IGMP message is sent with IP Precedence of Internetwork Control and an IP
Router Alert option in its IP header.

### Scenario constants

- The link. At 10 seconds, host A joins G; at 20 seconds, A leaves G.
- Observation lasts 40 seconds from the start: the Queries of R, the Reports of A, and the
  Group-Specific Queries after the leave.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A join G; at 20 seconds, let A leave G.
3. Observe every IGMP message on L1.

### Expected observations

1. On L1, from A, a Membership Report, and from R, a Membership Query. This confirms the
   stimulus.
2. Every IGMP message on L1 carries an IP Router Alert option (RFC9776-GEN-4).
3. Every IGMP message on L1 has IP Precedence Internetwork Control, the value 6 in the three
   precedence bits of the Type of Service octet (RFC9776-GEN-3).

## Timer relations in the Query

Checks: **RFC9776-TIMER-3** (must not (zero), should not (one)), **TIMER-6** (must (lower case)),
**TIMER-26** (must); covers **RFC9776-TIMER-2** (description).

### Requirement

RFC 9776 §8.1, §8.3 and §8.14.2: the Robustness Variable is not zero and should not be one; the
Query Response Interval is shorter than the Query Interval; the Query Interval is at least the
Max Response Time of the General Queries.

### Scenario constants

- The link, with the default of every value. Observation lasts 10 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the General Queries of R on L1.

### Expected observations

1. On L1, from R, a General Query. This confirms the stimulus.
2. Its QRV is neither zero nor one (RFC9776-TIMER-3).
3. The Max Response Time of its Max Resp Code is shorter than the Query Interval of its QQIC
   (RFC9776-TIMER-6, TIMER-26).

### Notes

- A QRV of zero also means a Robustness Variable above 7 (RFC9776-QRY-13). The querier of
  this check keeps the default of 2, so its QRV must be 2.
