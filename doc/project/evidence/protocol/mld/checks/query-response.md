# MLD — English check procedures: responses of a listener

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9777/catalog.md](../../../standard/rfc9777/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Response to a General Query

Checks: **RFC9777-LQRY-2**, **LQRY-4**, **LQRY-6**, **LTIM-1**, **LTIM-2**, **REP-20**,
**REP-21**, **REP-22**, **REP-23** (description), **LTIM-5** (must, must not); covers
**RFC9777-LQRY-3** (must (lower case)), **LTIM-3**, **RQ-9** (description), **LTIM-4** (may
(lower case)).

### Requirement

RFC 9777 §6.2 and §6.3: a node answers a General Query after a random delay in (0, Maximum
Response Delay], never at once, with one Current-State Record for each multicast address with
listening state on the interface: MODE_IS_EXCLUDE with the blocked sources, or MODE_IS_INCLUDE
with the wanted sources, packed into as few Reports as possible. A MODE_IS_INCLUDE record is
never sent with an empty source list.

### Scenario constants

- The link. From 10 seconds on, host A listens to G, and to H for S1: its state is EXCLUDE({})
  for G and INCLUDE({S1}) for H. A also listens to K from 10 to 20 seconds, so at the Query it
  has no listening state for K: INCLUDE({}) (RFC9777-LSN-7).
- The General Query under test is the second startup Query of R, 31.25 seconds after the first,
  with a Maximum Response Delay of 10 seconds.
- Observation lasts 50 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the General Queries of R and the Reports of A on L1.

### Expected observations

1. On L1, from R, after 20 seconds, a General Query. Record its instant. This confirms the
   stimulus.
2. On L1, from A, after that Query and within its Maximum Response Delay of 10 seconds, a Report
   with Current-State Records (RFC9777-LQRY-2, LQRY-4, LQRY-6, LTIM-1, REP-20).
3. That Report does not leave at the instant of the Query (RFC9777-LTIM-5).
4. It holds a MODE_IS_EXCLUDE record for G with no sources and a MODE_IS_INCLUDE record for H with
   the source S1 (RFC9777-LTIM-2, REP-21, REP-23).
5. It holds no record for K (RFC9777-LTIM-1, REP-22).

## Response to a Multicast Address Specific Query

Checks: **RFC9777-LQRY-7**, **LTIM-6** (description).

### Requirement

RFC 9777 §6.2 and §6.3: a node that has listening state for the queried address answers a
Multicast Address Specific Query with one Current-State Record for the address after a random
delay within the Maximum Response Delay of the Query; a node without state for the address sends
nothing.

### Scenario constants

- The link. Hosts A and B start to listen to G at 10 seconds; B stops at 50 seconds, so that R
  sends Multicast Address Specific Queries for G with a Maximum Response Delay of 1 second.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A and B make the requests above.
3. Observe the Queries of R and the Reports of A and B on L1 after 50 seconds.

### Expected observations

1. On L1, from R, after 50 seconds, a Multicast Address Specific Query for G. Record its
   instant. This confirms the stimulus.
2. On L1, from A, within 1 second of that Query and not at its instant, a Report with a
   MODE_IS_EXCLUDE record for G and no sources (RFC9777-LQRY-7, LTIM-6).
3. No Report with a Current-State Record for G leaves B after its stop (RFC9777-LTIM-6).

## Response to a Multicast Address and Source Specific Query

Checks: **RFC9777-LTIM-7**, **LTIM-8**, **LTIM-10** (description); covers **RFC9777-LTIM-11**
(description).

### Requirement

RFC 9777 §6.3 and Table 3: a node answers a Multicast Address and Source Specific Query for
sources B with a Current-State Record that the table builds from its state; from INCLUDE(A) it
reports IS_IN(A ∩ B). A record with no sources is not sent.

### Scenario constants

- The link. Host A listens to G for S1 and S2 from 10 seconds on: INCLUDE({S1, S2}). Host B
  listens to G for S1 from 10 to 50 seconds; its stop at 50 seconds makes R send Multicast
  Address and Source Specific Queries for G and S1.
- Observation lasts 60 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A and B make the requests above.
3. Observe the Queries of R and the Reports of A on L1 after 50 seconds.

### Expected observations

1. On L1, from R, after 50 seconds, a Multicast Address and Source Specific Query for G with the
   source S1. Record its instant. This confirms the stimulus.
2. On L1, from A, within 1 second of that Query, a Report with a Current-State Record for G
   (RFC9777-LTIM-7).
3. The record is MODE_IS_INCLUDE with the source S1 only, not S2 (RFC9777-LTIM-8).
4. No Report of A in the window holds a Current-State Record for G with no sources
   (RFC9777-LTIM-10).
