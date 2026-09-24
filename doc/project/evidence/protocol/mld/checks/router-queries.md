# MLD — English check procedures: queries of a router

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9777/catalog.md](../../../standard/rfc9777/catalog.md), [rfc2710/catalog.md](../../../standard/rfc2710/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## General Queries at startup and after it

Checks: **RFC9777-RQ-8**, **RQRY-7**, **TIMER-4**, **TIMER-9**, **TIMER-10**,
**RFC2710-ROUTER-5**, **ROUTER-6**, **ROUTER-7** (description); covers **RFC9777-RQ-7**,
**RFC2710-ROUTER-3** (description).

### Requirement

RFC 9777 §7.1, §7.6.2 and §9: the querier sends General Queries periodically; at startup it
sends [Startup Query Count] of them [Startup Query Interval] apart, then one every [Query
Interval]. RFC 2710 §6: a router starts as querier, sends a General Query at once to ff02::1
with the Query Response Interval as its Maximum Response Delay, and sends the next one when the
query timer expires.

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

1. On L1, from R, within 1 second of the start, a General Query to ff02::1 with a Maximum
   Response Code of 10000 (RFC2710-ROUTER-5, ROUTER-6). This confirms the stimulus.
2. The second General Query comes 31.25 seconds after the first (RFC9777-RQRY-7, TIMER-9,
   TIMER-10).
3. The third comes 125 seconds after the second, and the fourth 125 seconds after the third
   (RFC9777-RQ-8, TIMER-4, RFC2710-ROUTER-7).
4. No other General Query leaves R in the window (RFC9777-TIMER-10).

### Notes

- Each interval is judged with a margin of 0.1 seconds: the timers of the standard have no
  random part, and the margin only absorbs the time of the transmission.

## Querier election

Checks: **RFC9777-RQRY-6**, **RQRY-9** (should (lower case)), **RQRY-8**, **RQRY-11**,
**RFC2710-ROUTER-8**, **ROUTER-9**, **ROUTER-10** (description), **RFC9777-TIMER-8** (must);
covers **RFC2710-ROUTER-1**, **ROUTER-4**, **ROUTER-33** (description), **TIMER-10** (must,
should (lower case)), **TIMER-11** (must).

### Requirement

RFC 9777 §7.6.2 and §9.5: a router that hears a Query from a lower IPv6 address stops sending
General Queries and starts its Other-Querier-Present Timer with the Other Querier Present
Interval, [Robustness Variable] × [Query Interval] + 0.5 × [Query Response Interval]; when the
timer expires, it becomes the querier again. One address is lower than another when its
interface identifier, the last 64 bits, is lower. RFC 2710 §6 gives the same transitions.

### Scenario constants

- The two routers, both MLDv2, up from the start. The interface identifier of R1 on L1 is lower
  than that of R2. R1 leaves L1 at 100 seconds.
- Observation lasts 350 seconds from the start.

### Size or value arithmetic

The Other Querier Present Interval is 2 × 125 + 0.5 × 10 = 255 seconds. R1 sends General
Queries at 0 and 31.25 seconds before it leaves L1, so R2 becomes the querier at 31.25 + 255 =
286.25 seconds.

### Procedure

1. Build the two routers; at 100 seconds, break the link between R1 and L1.
2. Observe the General Queries of R1 and R2 on L1.

### Expected observations

1. On L1, from R1, the General Query at the start, which R2 hears. This confirms the stimulus.
2. From that Query until 255 seconds after the last General Query of R1, no General Query
   leaves R2 (RFC9777-RQRY-6, RQRY-8, RQRY-11, RFC2710-ROUTER-8, ROUTER-10).
3. A General Query of R2 comes 255 seconds after the last General Query of R1, with a margin of
   0.1 seconds (RFC9777-RQRY-9, TIMER-8, RFC2710-ROUTER-9).

### Notes

- R2 may send its own startup Query at the start, before it hears R1; observation 2 starts
  1 millisecond after the first Query of R1, when R2 has heard it.
