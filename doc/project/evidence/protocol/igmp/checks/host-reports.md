# IGMP — English check procedures: reports of a host

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9776/catalog.md](../../../standard/rfc9776/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Join reported at once

Checks: **RFC9776-HOST-4**, **HOST-6**, **HOST-10**, **REP-8**, **REP-10**, **REP-11**,
**REP-24** (description); covers **RFC9776-HOST-7**, **REP-22**, **HOST-1**, **HOST-3**
(description).

### Requirement

RFC 9776 §5.1: a change of the reception state of an interface makes the system send a
State-Change Report from that interface at once. A missing state counts as INCLUDE({}), so a
join, EXCLUDE({}), is a change of filter mode, reported as one CHANGE_TO_EXCLUDE_MODE record
with the new, empty source list. §5: no IGMP message is ever sent about 224.0.0.1.

### Scenario constants

- The link. At 10 seconds, host A joins G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A join G.
3. Observe the Reports of A on L1.

### Expected observations

1. On L1, from A, a Report that holds a Group Record for G. This confirms the stimulus.
2. It leaves within 10 milliseconds of the join (RFC9776-HOST-6).
3. It holds exactly one Group Record for G, of type CHANGE_TO_EXCLUDE_MODE, with no sources
   (RFC9776-HOST-10, REP-8, REP-10, REP-11, REP-24).
4. No Report of A in the window holds a record for 224.0.0.1 (RFC9776-HOST-4).

## Join repeated

Checks: **RFC9776-HOST-13**, **HOST-18**, **TIMER-18** (description), **HOST-21** (should (lower
case)), **HOST-24** (must (lower case)); covers **RFC9776-TIMER-2** (description).

### Requirement

RFC 9776 §5.1: a system sends a State-Change Report [Robustness Variable] times in all; the
repetitions come at random times within the Unsolicited Report Interval, and after a change of
filter mode each of them carries the Filter-Mode-Change Record again, a TO_EX record with the
sources that the current state blocks.

### Scenario constants

- The link, with the default Robustness Variable of 2 and Unsolicited Report Interval of 1
  second. At 10 seconds, host A joins G.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A join G.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, the first Report with a CHANGE_TO_EXCLUDE_MODE record for G. This confirms
   the stimulus.
2. A second Report with a CHANGE_TO_EXCLUDE_MODE record for G, with no sources, at most 1
   second after the first (RFC9776-HOST-13, HOST-18, HOST-21, HOST-24, TIMER-18).
3. No third State-Change Report for G in the window (RFC9776-HOST-13).

### Notes

- A Report that answers a Query holds a Current-State Record, not a Filter-Mode-Change Record,
  so observation 3 does not count it.

## Leave reported

Checks: **RFC9776-HOST-11**, **REP-23** (description), **HOST-23** (must (lower case)).

### Requirement

RFC 9776 §5.1: a leave changes the state from EXCLUDE({}) to the missing state, INCLUDE({}); the
system reports a CHANGE_TO_INCLUDE_MODE record with the new, empty source list at once, and
repeats it, with the sources that the current state forwards.

### Scenario constants

- The link. At 10 seconds, host A joins G; at 30 seconds, A leaves G.
- Observation lasts 40 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A join G; at 30 seconds, let A leave G.
3. Observe the Reports of A on L1 after 30 seconds.

### Expected observations

1. On L1, from A, after 30 seconds, a Report with a record for G. This confirms the stimulus.
2. The record is of type CHANGE_TO_INCLUDE_MODE with no sources, and the Report leaves within
   10 milliseconds of the leave (RFC9776-HOST-11, REP-23).
3. A second Report with a CHANGE_TO_INCLUDE_MODE record for G, with no sources, at most 1
   second after the first (RFC9776-HOST-23).

## Source list change

Checks: **RFC9776-HOST-8**, **HOST-12**, **HOST-27**, **REP-27**, **REP-28** (description),
**HOST-22** (should (lower case)), **HOST-26** (must (lower case)); covers **RFC9776-REP-26**,
**REP-12** (description).

### Requirement

RFC 9776 §5.1: a change from INCLUDE(A) to INCLUDE(B) is reported as ALLOW(B − A) and
BLOCK(A − B); a record with an empty source list is left out, also from a repetition.

### Scenario constants

- The link. Host A joins G for S1 from 10 seconds on, and joins G for S2 from 12 to 20 seconds.
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
   record (RFC9776-HOST-8, HOST-12, REP-27).
3. At 20 seconds, a Report that holds a BLOCK_OLD_SOURCES record for G with the source S2 only,
   and no ALLOW_NEW_SOURCES record (RFC9776-HOST-8, HOST-12, REP-28).
4. The repetition of each of the two Reports, at most 1 second later, holds the same record and
   no empty record (RFC9776-HOST-22, HOST-26, HOST-27).

## Change inside EXCLUDE mode

Checks: **RFC9776-HOST-9** (description).

### Requirement

RFC 9776 §5.1: a change from EXCLUDE(A) to EXCLUDE(B) is reported as ALLOW(A − B) and
BLOCK(B − A).

### Scenario constants

- The link. Host A joins G at 10 seconds; from 20 to 30 seconds it blocks the source S1 for G.
  The state of A is EXCLUDE({}) from 10 seconds, EXCLUDE({S1}) from 20 seconds, and EXCLUDE({})
  from 30 seconds.
- Observation lasts 40 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, at 10 seconds, a Report with a CHANGE_TO_EXCLUDE_MODE record for G. This
   confirms the stimulus.
2. At 20 seconds, a Report that holds a BLOCK_OLD_SOURCES record for G with the source S1 only,
   and no ALLOW_NEW_SOURCES record (RFC9776-HOST-9).
3. At 30 seconds, a Report that holds an ALLOW_NEW_SOURCES record for G with the source S1 only,
   and no BLOCK_OLD_SOURCES record (RFC9776-HOST-9).

## A change during the repetitions

Checks: **RFC9776-HOST-14**, **HOST-15**, **HOST-16**, **HOST-17** (description), **HOST-25**
(must (lower case)).

### Requirement

RFC 9776 §5.1: a further change of the same state before the repetitions end sends a new
State-Change Report at once, which merges the pending records with the new ones, and starts a
new count of [Robustness Variable] transmissions. A source keeps its place in the repetitions
until [Robustness Variable] Reports have held it.

### Scenario constants

- The link. Host A joins G for S1 at 10.0 seconds and for S2 at 10.3 seconds, before the
  repetition of the first Report, which comes up to 1 second later.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. Let A make the requests above.
3. Observe the Reports of A on L1 that hold a record for G.

### Expected observations

1. On L1, from A, at 10.0 seconds, a Report with an ALLOW_NEW_SOURCES record for G with S1. This
   confirms the stimulus.
2. At 10.3 seconds, a Report with an ALLOW_NEW_SOURCES record for G that holds both S1 and S2
   (RFC9776-HOST-14, HOST-15, HOST-16, HOST-25).
3. At most 1 second after it, one more Report with an ALLOW_NEW_SOURCES record for G that holds
   S2 but not S1, because S1 has been in two Reports already (RFC9776-HOST-16, HOST-17).
4. No further State-Change Report for G in the window.

### Notes

- The random interval allows the repetition of the first Report to come before 10.3 seconds.
  Then S1 has been in two Reports before the change, and the Report at 10.3 seconds holds S2
  alone. The check counts the Reports that held S1 before 10.3 seconds, and reads observations
  2 and 3 with that count: S1 belongs in a Report while fewer than two Reports have held it.
