# IGMP — English check procedures: query responses of a host

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9776/catalog.md](../../../standard/rfc9776/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Response to a General Query

Checks: **RFC9776-HQRY-1**, **HQRY-3**, **HQRY-5**, **HQRY-10**, **REP-18**, **REP-19**,
**REP-20** (description), **HQRY-12** (must not); covers **RFC9776-HQRY-2** (must (lower case)),
**HQRY-11** (description).

### Requirement

RFC 9776 §5.2: a system answers a General Query after a random delay in (0, Max Response Time],
never at once, with one Current-State Record for each group of the interface: MODE_IS_EXCLUDE
with the blocked sources, or MODE_IS_INCLUDE with the wanted sources, packed into as few Reports
as possible.

### Scenario constants

- The link. From 10 seconds on, host A joins G, and joins H for S1: its state is EXCLUDE({}) for
  G and INCLUDE({S1}) for H.
- The General Query under test is the second startup Query of R, 31.25 seconds after the first,
  with a Max Response Time of 10 seconds.
- Observation lasts 50 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the General Queries of R and the Reports of A on L1.

### Expected observations

1. On L1, from R, after 10 seconds, a General Query. Record its instant. This confirms the
   stimulus.
2. On L1, from A, after that Query and within its Max Response Time of 10 seconds, a Report with
   Current-State Records (RFC9776-HQRY-1, HQRY-3, HQRY-5, HQRY-10, REP-18).
3. That Report does not leave at the instant of the Query (RFC9776-HQRY-12).
4. It holds a MODE_IS_EXCLUDE record for G with no sources and a MODE_IS_INCLUDE record for H with
   the source S1 (RFC9776-HQRY-10, REP-19, REP-20).

## Response to a Group-Specific Query

Checks: **RFC9776-HQRY-6**, **HQRY-13** (description); covers **RFC9776-HQRY-4** (description).

### Requirement

RFC 9776 §5.2: a system that has state for the queried group answers a Group-Specific Query with
one Current-State Record for the group after a random delay within the Max Response Time of the
Query; a system without state for the group sends nothing.

### Scenario constants

- The link. Hosts A and B join G at 10 seconds; B leaves G at 50 seconds, so that R sends
  Group-Specific Queries for G with a Max Response Time of 1 second.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A and B make the requests above.
3. Observe the Queries of R and the Reports of A and B on L1 after 50 seconds.

### Expected observations

1. On L1, from R, after 50 seconds, a Group-Specific Query for G. Record its instant. This
   confirms the stimulus.
2. On L1, from A, within 1 second of that Query and not at its instant, a Report with a
   MODE_IS_EXCLUDE record for G and no sources (RFC9776-HQRY-6, HQRY-13).
3. No Report with a Current-State Record for G leaves B after the leave (RFC9776-HQRY-13).

## Response to a Group-and-Source-Specific Query

Checks: **RFC9776-HQRY-7**, **HQRY-14**, **HQRY-15**, **HQRY-17** (description); covers
**RFC9776-HQRY-9**, **HQRY-18** (description).

### Requirement

RFC 9776 §5.2 and Table 5: a system answers a Group-and-Source-Specific Query for sources B with
a Current-State Record that the table builds from its state; from INCLUDE(A) it reports
IS_IN(A ∩ B). A record with no sources is not sent.

### Scenario constants

- The link. Host A joins G for S1 and S2 from 10 seconds on: INCLUDE({S1, S2}). Host B joins G
  for S1 from 10 to 50 seconds; its leave at 50 seconds makes R send Group-and-Source-Specific
  Queries for G and S1.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A and B make the requests above.
3. Observe the Queries of R and the Reports of A on L1 after 50 seconds.

### Expected observations

1. On L1, from R, after 50 seconds, a Group-and-Source-Specific Query for G with the source S1.
   Record its instant. This confirms the stimulus.
2. On L1, from A, within 1 second of that Query, a Report with a Current-State Record for G
   (RFC9776-HQRY-7, HQRY-14).
3. The record is MODE_IS_INCLUDE with the source S1 only, not S2 (RFC9776-HQRY-15).
4. No Report of A in the window holds a Current-State Record for G with no sources
   (RFC9776-HQRY-17).
