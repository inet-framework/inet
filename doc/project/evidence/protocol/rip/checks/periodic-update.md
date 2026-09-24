# RIP — English check procedures: the periodic update

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Periodic update interval (RIP version 2)

Checks: **RFC2453-TIMER-1** (description), **RFC2453-TIMER-2** (must, "required").

### Requirement

RFC 2453 §3.8: every 30 seconds the router sends an unsolicited response with the complete
routing table, after split horizon, to every neighbor. To keep the routers of a network from
synchronizing, the 30-second updates run on a clock that load does not affect, or the timer
gets a random offset of up to 5 seconds, plus or minus, each time it is set.

### Scenario constants

- The pair. The marker of R1 is netA.
- Observation lasts 220 seconds from the start: room for the first periodic update of R1 and
  five more.
- The table of R1 has three routes: netA and L1, to which R1 is attached, and netB, which R1
  learns from R2 over L1.

### Size or value arithmetic

With either precaution of §3.8, two consecutive periodic updates are 30 seconds apart, give
or take the offset of the second one. An offset of up to 5 seconds on each setting puts the
interval between 25 and 35 seconds.

### Procedure

1. Build the pair, and let both routers start.
2. Observe the periodic updates that R1 sends on L1, and record the instant of each one.

### Expected observations

1. On L1, from R1, a first periodic update. Record its instant. This confirms the stimulus.
2. The next periodic update comes 25 to 35 seconds after the one before it (RFC2453-TIMER-2,
   RFC2453-TIMER-1).
3. The same holds for the three periodic updates after that one: each comes 25 to 35 seconds
   after the one before it.
4. Every periodic update in the window holds netA with metric 1 and L1 with metric 1: the
   networks R1 is attached to are part of the complete table (RFC2453-TIMER-1).

### Notes

- The check tests the bounds of the interval, not its distribution. Whether the offset is
  random, and whether it spreads over the whole interval, needs many runs: level 4.
- Observation 4 is the "complete routing table" half of RFC2453-TIMER-1. netB is not in the
  observation: R1 learned it over L1, so split horizon lets it go onto L1 only with metric 16,
  or not at all, and the checks of split horizon test that.

## Periodic update interval (RIPng)

Checks: **RFC2080-TIMER-1**, **RFC2080-OUT-2** (description), **RFC2080-TIMER-2** (must,
"required").

### Requirement

RFC 2080 §2.3 and §2.5: every 30 seconds the router sends an unsolicited response with the
whole routing table to every neighbor. The timer runs on a clock that load does not affect,
or it gets a random offset of up to 15 seconds, plus or minus — half the update period —
each time it is set.

### Scenario constants

- The pair, on IPv6. The marker of R1 is netA.
- Observation lasts 280 seconds from the start: room for the first periodic update of R1 and
  five more, at the longest interval.

### Size or value arithmetic

An offset of up to 15 seconds on each setting puts the interval between 15 and 45 seconds.

### Procedure

1. Build the pair on IPv6, and let both routers start.
2. Observe the periodic updates that R1 sends on L1, and record the instant of each one.

### Expected observations

1. On L1, from R1, a first periodic update. Record its instant. This confirms the stimulus.
2. The next periodic update comes 15 to 45 seconds after the one before it (RFC2080-TIMER-2,
   RFC2080-TIMER-1, RFC2080-OUT-2).
3. The same holds for the three periodic updates after that one.
4. Every periodic update in the window holds netA with metric 1 and L1 with metric 1
   (RFC2080-TIMER-1).

### Notes

- The interval of RFC 2080 is wider than the one of RFC 2453. A model that uses the offset of
  RFC 2453 for RIPng passes this check, because 25 to 35 seconds lies inside 15 to 45.
