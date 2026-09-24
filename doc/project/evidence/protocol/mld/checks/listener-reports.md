# MLD — English check procedures: reports of a listener

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9777/catalog.md](../../../standard/rfc9777/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Start of listening reported at once

Checks: **RFC9777-LSN-3** (must), **LSN-5**, **LSN-6**, **LSN-10**, **REP-9**, **REP-11**,
**REP-12**, **REP-25**, **REP-27** (description), **LSN-23** (must (lower case)); covers
**RFC9777-LSN-1**, **LSN-2**, **LSN-7**, **RQ-10** (description).

### Requirement

RFC 9777 §6.1: a change of the listening state of an interface makes the node send a
State-Change Report from that interface at once. A missing state counts as INCLUDE({}), so a
start of listening, EXCLUDE({}), is a change of filter mode, reported as one
CHANGE_TO_EXCLUDE_MODE record with the new, empty source list. §6: no MLD message is ever sent
about the link-scope all-nodes address ff02::1 or an address of scope 0 or 1, and MLD messages
are sent for every other address that the node listens to — also the solicited-node addresses
that a node listens to for Neighbor Discovery.

### Scenario constants

- The link. At 10 seconds, host A starts to listen to G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A start to listen to G.
3. Observe the Reports of A on L1 from the start.

### Expected observations

1. On L1, from A, a Report that holds a Multicast Address Record for G. This confirms the
   stimulus.
2. It leaves within 10 milliseconds of the request (RFC9777-LSN-5).
3. It holds exactly one record for G, of type CHANGE_TO_EXCLUDE_MODE, with no sources
   (RFC9777-LSN-6, LSN-10, LSN-23, REP-9, REP-11, REP-12, REP-25, REP-27).
4. No Report of A in the window holds a record for ff02::1 (RFC9777-LSN-3).
5. Before 10 seconds, a Report of A holds a record for the solicited-node address of each
   address of A on L1, ff02::1:ffXX:XXXX with the last 24 bits of the address (RFC9777-LSN-3).

### Notes

- Observation 5 reads the state that the interface of A has from the start: the solicited-node
  addresses of its link-local and its global address. It is the second half of RFC9777-LSN-3, and
  it has no stimulus of its own.

## Start of listening repeated

Checks: **RFC9777-LSN-13**, **LSN-18**, **TIMER-15** (description), **LTIM-12** (should (lower
case)), **LTIM-16** (must (lower case)); covers **RFC9777-LSN-19**, **LTIM-13**, **TIMER-2**
(description).

### Requirement

RFC 9777 §6.1 and §6.3: a node sends a State-Change Report [Robustness Variable] times in all;
the repetitions come at random times within the Unsolicited Report Interval, and after a change
of filter mode each of them carries the Filter-Mode-Change Record again, a TO_EX record with the
sources that the current state blocks.

### Scenario constants

- The link, with the default Robustness Variable of 2 and Unsolicited Report Interval of 1
  second. At 10 seconds, host A starts to listen to G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A start to listen to G.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, the first Report with a CHANGE_TO_EXCLUDE_MODE record for G. This confirms
   the stimulus.
2. A second Report with a CHANGE_TO_EXCLUDE_MODE record for G, with no sources, at most 1
   second after the first (RFC9777-LSN-13, LSN-18, LTIM-12, LTIM-16, TIMER-15).
3. No third State-Change Report for G in the window (RFC9777-LSN-13, LSN-18).

### Notes

- A Report that answers a Query holds a Current-State Record, not a Filter-Mode-Change Record,
  so observation 3 does not count it.

## Stop of listening reported

Checks: **RFC9777-LSN-11**, **REP-26** (description), **LSN-22**, **LTIM-15** (must (lower
case)).

### Requirement

RFC 9777 §6.1 and §6.3: a stop of listening changes the state from EXCLUDE({}) to the missing
state, INCLUDE({}); the node reports a CHANGE_TO_INCLUDE_MODE record with the new, empty source
list at once, and repeats it, with the sources that the current state forwards.

### Scenario constants

- The link. At 10 seconds, host A starts to listen to G; at 30 seconds, A stops.
- Observation lasts 40 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A start to listen to G; at 30 seconds, let A stop.
3. Observe the Reports of A on L1 after 30 seconds.

### Expected observations

1. On L1, from A, after 30 seconds, a Report with a record for G. This confirms the stimulus.
2. The record is of type CHANGE_TO_INCLUDE_MODE with no sources, and the Report leaves within
   10 milliseconds of the request (RFC9777-LSN-11, LSN-22, REP-26).
3. A second Report with a CHANGE_TO_INCLUDE_MODE record for G, with no sources, at most 1
   second after the first (RFC9777-LTIM-15).

## Source list change

Checks: **RFC9777-LSN-8**, **LSN-12**, **LTIM-19**, **REP-30**, **REP-31** (description),
**LSN-25**, **LTIM-17**, **LTIM-18** (must (lower case)), **LTIM-14** (should (lower case));
covers **RFC9777-REP-29** (description).

### Requirement

RFC 9777 §6.1 and §6.3: a change from INCLUDE(A) to INCLUDE(B) is reported as ALLOW(B − A) and
BLOCK(A − B); a record with an empty source list is left out, also from a repetition.

### Scenario constants

- The link. Host A listens to G for S1 from 10 seconds on, and for S2 from 12 to 20 seconds.
  The state of A is INCLUDE({S1}) from 10 seconds, INCLUDE({S1, S2}) from 12 seconds, and
  INCLUDE({S1}) again from 20 seconds.
- Observation lasts 30 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, at 12 seconds, a Report with a record for G. This confirms the stimulus.
2. It holds an ALLOW_NEW_SOURCES record for G with the source S2 only, and no BLOCK_OLD_SOURCES
   record (RFC9777-LSN-8, LSN-12, REP-30).
3. At 20 seconds, a Report that holds a BLOCK_OLD_SOURCES record for G with the source S2 only,
   and no ALLOW_NEW_SOURCES record (RFC9777-LSN-8, LSN-12, LSN-25, REP-31).
4. The repetition of each of the two Reports, at most 1 second later, holds the same record and
   no empty record (RFC9777-LTIM-14, LTIM-17, LTIM-18, LTIM-19).

## Change inside EXCLUDE mode

Checks: **RFC9777-LSN-9** (description).

### Requirement

RFC 9777 §6.1: a change from EXCLUDE(A) to EXCLUDE(B) is reported as ALLOW(A − B) and
BLOCK(B − A).

### Scenario constants

- The link. Host A starts to listen to G at 10 seconds; from 20 to 30 seconds it blocks the
  source S1 for G. The state of A is EXCLUDE({}) from 10 seconds, EXCLUDE({S1}) from 20
  seconds, and EXCLUDE({}) from 30 seconds.
- Observation lasts 40 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, at 10 seconds, a Report with a CHANGE_TO_EXCLUDE_MODE record for G. This
   confirms the stimulus.
2. At 20 seconds, a Report that holds a BLOCK_OLD_SOURCES record for G with the source S1 only,
   and no ALLOW_NEW_SOURCES record (RFC9777-LSN-9).
3. At 30 seconds, a Report that holds an ALLOW_NEW_SOURCES record for G with the source S1 only,
   and no BLOCK_OLD_SOURCES record (RFC9777-LSN-9).

## A change during the repetitions

Checks: **RFC9777-LSN-14**, **LSN-15**, **LSN-16**, **LSN-17** (description), **LSN-24** (must
(lower case)).

### Requirement

RFC 9777 §6.1: a further change of the same state before the repetitions end sends a new
State-Change Report at once, which merges the pending records with the new ones, and starts a
new count of [Robustness Variable] transmissions. A source keeps its place in the repetitions
until [Robustness Variable] Reports have held it.

### Scenario constants

- The link. Host A listens to G for S1 from 10.0 seconds and for S2 from 10.3 seconds, before
  the repetition of the first Report, which comes up to 1 second later.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, at 10.0 seconds, a Report with an ALLOW_NEW_SOURCES record for G with S1. This
   confirms the stimulus.
2. At 10.3 seconds, a Report with an ALLOW_NEW_SOURCES record for G that holds both S1 and S2
   (RFC9777-LSN-14, LSN-15, LSN-16, LSN-24).
3. At most 1 second after it, one more Report with an ALLOW_NEW_SOURCES record for G that holds
   S2 but not S1, because S1 has been in two Reports already (RFC9777-LSN-16, LSN-17).
4. No further State-Change Report for G in the window.

### Notes

- The random interval allows the repetition of the first Report to come before 10.3 seconds.
  Then S1 has been in two Reports before the change, and the Report at 10.3 seconds holds S2
  alone. The check counts the Reports that held S1 before 10.3 seconds, and reads observations
  2 and 3 with that count: S1 belongs in a Report while fewer than two Reports have held it.
