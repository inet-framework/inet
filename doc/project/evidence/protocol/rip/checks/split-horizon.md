# RIP — English check procedures: split horizon

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Split horizon on the link of a learned route (RIP version 2)

Checks: **RFC2453-SH-1** (must), **RFC2453-SH-2** (description); covers **RFC2453-GEN-5**
(must).

### Requirement

RFC 2453 §3.4.3: every implementation must use split horizon, and should use split horizon
with poisoned reverse. Simple split horizon leaves a route out of the updates sent to the
neighbor it was learned from; poisoned reverse includes it there with metric 16. §3.10.2:
routes with metric 16 are included in the updates.

### Scenario constants

- The chain. The marker of R2 on L1 is L2, a network R2 is attached to.
- R2 learns netA from R1 over L1, and netC from R3 over L2.
- Observation lasts 180 seconds.

### Procedure

1. Build the chain, and let the three routers start.
2. Observe the updates of R2 on L1 and on L2.

### Expected observations

1. On L2, from R2, an update that holds netA with metric 2, and after it, on L1, from R2, an
   update that holds netC with metric 2. This confirms the stimulus: R2 has learned netA over L1
   and netC over L2. The second update comes when R3 has sent its routes, which no timer of
   the standard puts before any given update of R2.
2. On L1, from R2, the next three periodic updates after these: none of them holds netA
   with a metric below 16 (RFC2453-SH-1, SH-2). Where one holds netA, the metric is 16
   (covers RFC2453-GEN-5).
3. The same periodic updates hold netC with metric 2: split horizon leaves out only the routes
   learned over L1, and not every learned route.
4. No update of R2 on L1, periodic or triggered, holds netA with a metric below 16 at any time
   after observation 1.

### Notes

- Observation 2 accepts both forms of split horizon that RFC 2453 permits. The `should` of
  poisoned reverse is visible in the run, as metric 16 or as absence, and the results record
  which one the model uses, but a check does not fail on a `should`.
- Observation 3 separates split horizon from a router that advertises nothing it has learned.
  Such a router also passes observation 2.

## Split horizon on the link of a learned route (RIPng)

Checks: **RFC2080-SH-1**, **RFC2080-SH-2** (description); covers **RFC2080-GEN-7** (must).

### Requirement

RFC 2080 §2.6: split horizon omits a route in the updates sent to the neighbor it was learned
from; poisoned reverse includes it with metric 16 and is the preferred method. §2.5.2: routes
with metric 16 are included in the updates.

### Scenario constants

- The chain, on IPv6. The marker of R2 on L1 is L2.
- Observation lasts 180 seconds.

### Procedure

1. Build the chain on IPv6, and let the three routers start.
2. Observe the updates of R2 on L1 and on L2.

### Expected observations

1. On L2, from R2, an update that holds netA with metric 2, and after it, on L1, from R2, an
   update that holds netC with metric 2. This confirms the stimulus: R2 has learned netA over L1
   and netC over L2. The second update comes when R3 has sent its routes, which no timer of
   the standard puts before any given update of R2.
2. On L1, from R2, the next three periodic updates after these: none of them holds netA
   with a metric below 16 (RFC2080-SH-1, SH-2). Where one holds netA, the metric is 16
   (covers RFC2080-GEN-7).
3. The same periodic updates hold netC with metric 2.
4. No update of R2 on L1 holds netA with a metric below 16 at any time after observation 1.
