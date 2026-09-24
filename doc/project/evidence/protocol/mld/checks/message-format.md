# MLD — English check procedures: message format

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9777/catalog.md](../../../standard/rfc9777/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Multicast Listener Report encapsulation

Checks: **RFC9777-GEN-1**, **GEN-6**, **REP-2**, **REP-4**, **REP-7**, **REP-40** (description),
**GEN-2**, **GEN-3**, **REP-15**, **REP-34** (must), **REP-19** (must not); covers
**RFC9777-REP-1**, **REP-5**, **REP-8**, **REP-11**, **REP-13**, **REP-14** (description),
**REP-10** (may (lower case)).

### Requirement

RFC 9777 §5 and §5.2: an MLDv2 message is an ICMPv6 message, Next Header 58, sent from a
link-local address with Hop Limit 1. A Version 2 Multicast Listener Report has type 143, zero in
its Reserved field, the number of its Multicast Address Records, no auxiliary data and no octets
after the last record; it goes to ff02::16, and its checksum is the ICMPv6 checksum over the
message and the pseudo-header of IPv6.

### Scenario constants

- The link. At 10 seconds, host A starts to listen to G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A start to listen to G.
3. Observe the MLD messages of A on L1.

### Expected observations

1. On L1, from A, after 10 seconds, a Version 2 Multicast Listener Report, ICMPv6 type 143
   (RFC9777-GEN-6). This confirms the stimulus.
2. It is an ICMPv6 message, Next Header 58 (RFC9777-GEN-1), with Hop Limit 1 (RFC9777-GEN-3),
   and it leaves from the link-local address of A (RFC9777-GEN-2, REP-34).
3. Its IPv6 destination is ff02::16 (RFC9777-REP-40).
4. Its Nr of Mcast Address Records is the number of records that follow, each record has Aux
   Data Len zero, and no octet follows the last record (RFC9777-REP-7, REP-15, REP-19).
5. Its Reserved field is zero (RFC9777-REP-2), and its checksum is the ICMPv6 checksum over the
   message and the pseudo-header (RFC9777-REP-4).

### Notes

- With the Hop-by-Hop Options header of the Router Alert option, the value 58 is in the Next
  Header field of that header, not of the IPv6 header.

## Multicast Listener Query encapsulation

Checks: **RFC9777-GEN-5**, **QRY-2**, **QRY-4**, **QRY-7**, **QRY-8**, **QRY-10**, **QRY-13**,
**QRY-15**, **QRY-17**, **QRY-23**, **QRY-28**, **TIMER-5**, **VER-1** (description), **QRY-22**
(must not), **QRY-26**, **RQRY-10** (must); covers **RFC9777-QRY-1**, **QRY-5**, **QRY-19**
(description).

### Requirement

RFC 9777 §5.1 and §9: a General Query of MLDv2 has type 130, is at least 28 octets long, holds
the unspecified address as its Multicast Address and no sources, and goes to ff02::1 from a
link-local address. Its Maximum Response Code below 32768 is the Maximum Response Delay in
milliseconds, 10000 by default; QRV holds the Robustness Variable of the querier and QQIC its
Query Interval, below 128 as a plain value.

### Scenario constants

- The link, with the default of every value. Observation lasts 10 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the MLD messages of R on L1.

### Expected observations

1. On L1, from R, a Multicast Listener Query, ICMPv6 type 130 (RFC9777-GEN-5). This confirms
   the stimulus.
2. It is at least 28 octets long, so it is an MLDv2 Query (RFC9777-VER-1), and no octet
   follows its last source address (RFC9777-QRY-22).
3. It leaves from a link-local address of fe80::/64 (RFC9777-QRY-26, RQRY-10) to ff02::1, with
   the unspecified address as its Multicast Address and zero sources: a General Query
   (RFC9777-QRY-10, QRY-17, QRY-23, QRY-28).
4. Its Maximum Response Code is 10000, a Maximum Response Delay of 10 seconds (RFC9777-QRY-7,
   TIMER-5).
5. Its QRV is 2 and its QQIC is 125 (RFC9777-QRY-13, QRY-15).
6. Its Code and Reserved fields are zero, and its checksum is the ICMPv6 checksum over the
   message and the pseudo-header (RFC9777-QRY-2, QRY-4, QRY-8).

## Router Alert on every message

Checks: **RFC9777-GEN-4** (must); covers **RFC9777-GEN-2**, **GEN-3** (must).

### Requirement

RFC 9777 §5: every MLDv2 message is sent from a link-local address, with Hop Limit 1 and an IPv6
Router Alert option in a Hop-by-Hop Options header.

### Scenario constants

- The link. At 10 seconds, host A starts to listen to G; at 20 seconds, A stops.
- Observation lasts 40 seconds from the start: the Queries of R, the Reports of A, and the
  Multicast Address Specific Queries after A stops.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A start to listen to G; at 20 seconds, let A stop.
3. Observe every MLD message on L1.

### Expected observations

1. On L1, from A, a Report, and from R, a Query. This confirms the stimulus.
2. Every MLD message on L1 carries a Hop-by-Hop Options header with a Router Alert option
   (RFC9777-GEN-4).
3. Every MLD message on L1 has Hop Limit 1 and a link-local source address (RFC9777-GEN-2,
   GEN-3).

## Timer relations in the Query

Checks: **RFC9777-TIMER-3** (must not (zero); should not (one)), **TIMER-6** (must (lower case)),
**TIMER-20** (must); covers **RFC9777-TIMER-2** (description).

### Requirement

RFC 9777 §9.1, §9.3 and §9.14.2: the Robustness Variable is not zero and should not be one; the
Query Response Interval is shorter than the Query Interval; the Query Interval is at least the
Maximum Response Delay of the General Queries.

### Scenario constants

- The link, with the default of every value. Observation lasts 290 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the General Queries of R on L1.

### Expected observations

1. On L1, from R, a General Query. This confirms the stimulus.
2. Its QRV is neither zero nor one (RFC9777-TIMER-3).
3. The Maximum Response Delay of its Maximum Response Code is shorter than the Query Interval,
   the time from the third General Query of R to the fourth (RFC9777-TIMER-6, TIMER-20).

### Notes

- A QRV of zero also means a Robustness Variable above 7 (RFC9777-QRY-13). The querier of
  this check keeps the default of 2, so its QRV must be 2.
- Observation 3 reads the Query Interval from the behavior of R, not from the QQIC: the value of
  the QQIC is the subject of [Multicast Listener Query encapsulation](#multicast-listener-query-encapsulation).
  The third and the fourth General Query come after the startup Queries.
