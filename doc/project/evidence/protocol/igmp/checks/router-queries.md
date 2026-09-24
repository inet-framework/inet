# IGMP — English check procedures: queries of a router

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9776/catalog.md](../../../standard/rfc9776/catalog.md), [rfc2236/catalog.md](../../../standard/rfc2236/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## General Queries at startup and after it

Checks: **RFC9776-RQ-6**, **TIMER-4**, **TIMER-11**, **TIMER-12**, **RFC2236-ROUTER-8**,
**ROUTER-9**, **ROUTER-10** (description); covers **RFC9776-RQ-1**, **RFC2236-ROUTER-3**,
**ROUTER-6** (description), **ROUTER-14** (should (lower case)).

### Requirement

RFC 9776 §6.1 and §8: the querier sends General Queries periodically; at startup it sends
[Startup Query Count] of them [Startup Query Interval] apart, then one every [Query Interval].
RFC 2236 §7: a router starts as querier, sends a General Query at once to 224.0.0.1, and sends
the next one when the query timer expires.

### Scenario constants

- The link, with the defaults: Startup Query Count 2, Startup Query Interval 31.25 seconds,
  Query Interval 125 seconds.
- Observation lasts 320 seconds from the start.

### Size or value arithmetic

With R up at the start, the General Queries come at 0 and 31.25 seconds, then at 156.25 and
281.25 seconds.

### Procedure

1. Build the link, and let R, A and B come up.
2. Observe the General Queries of R on L1, and record the instant of each one.

### Expected observations

1. On L1, from R, within 1 second of the start, a General Query to 224.0.0.1 (RFC2236-ROUTER-8,
   ROUTER-9). This confirms the stimulus.
2. The second General Query comes 31.25 seconds after the first (RFC9776-TIMER-11, TIMER-12).
3. The third comes 125 seconds after the second, and the fourth 125 seconds after the third
   (RFC9776-RQ-6, TIMER-4, RFC2236-ROUTER-10).
4. No other General Query leaves R in the window (RFC9776-TIMER-12).

### Notes

- Each interval is judged with a margin of 0.1 seconds: the timers of the standard have no
  random part, and the margin only absorbs the time of the transmission.

## Querier election

Checks: **RFC9776-RQRY-5**, **RQRY-6**, **RFC2236-ROUTER-11**, **ROUTER-12**, **ROUTER-13**
(description), **RFC9776-RQRY-7** (should (lower case)), **TIMER-10** (must); covers
**RFC9776-TIMER-9** (must, should (lower case)), **RFC2236-ROUTER-1**, **ROUTER-2**,
**ROUTER-4**, **ROUTER-5**, **ROUTER-7** (description).

### Requirement

RFC 9776 §6.6.2 and §8.5: a router that hears a General Query from a lower IP address stops
sending General Queries and starts its Other-Querier-Present Timer with the Other Querier
Present Interval, [Robustness Variable] × [Query Interval] + 0.5 × [Query Response Interval]; when
the timer expires, it becomes the querier again. RFC 2236 §7 gives the same transitions.

### Scenario constants

- The two routers, both IGMPv3. R1 comes up at the start, R2 at 5 seconds. R1 stops at 100
  seconds.
- Observation lasts 350 seconds from the start.

### Size or value arithmetic

The Other Querier Present Interval is 2 × 125 + 0.5 × 10 = 255 seconds. R1 sends General
Queries at 0 and 31.25 seconds before it stops, so R2 becomes the querier at 31.25 + 255 =
286.25 seconds.

### Procedure

1. Build the two routers, R2 down until 5 seconds; at 100 seconds, stop R1.
2. Observe the General Queries of R1 and R2 on L1.

### Expected observations

1. On L1, from R1, the General Query at 31.25 seconds, which R2 hears. This confirms the
   stimulus.
2. From that Query until 255 seconds after the last General Query of R1, no General Query
   leaves R2 (RFC9776-RQRY-5, RQRY-6, RFC2236-ROUTER-11, ROUTER-13).
3. A General Query of R2 comes 255 seconds after the last General Query of R1, with a margin of
   0.1 seconds (RFC9776-RQRY-7, TIMER-10, RFC2236-ROUTER-12).

### Notes

- R2 may send its own startup Query at 5 seconds, before it hears R1; observation 2 starts at
  the first Query of R1 that R2 hears.
