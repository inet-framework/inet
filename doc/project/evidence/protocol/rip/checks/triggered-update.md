# RIP — English check procedures: triggered updates

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Triggered update for a lost network (RIP version 2)

Checks: **RFC2453-TRIG-1**, **TRIG-5**, **TRIG-9**, **GEN-5** (must), **RFC2453-TRIG-6**
(should), **RFC2453-TIMER-5**, **RESP-9** (description).

### Requirement

RFC 2453 §3.4.4: every implementation must send a triggered update for a deleted route.
§3.8: a route is deleted when the metric becomes 16; the router sets the change flag and
signals the output process. §3.10.1: the triggered update holds at least the changed routes,
goes out on every directly-connected network, and follows every rule of a response. §3.10.2:
routes with metric 16 are included. §3.9.2: a router adopts the metric that the next hop of
a route sends, 16 included, and then starts the deletion itself.

### Scenario constants

- The chain.
- At 100 seconds, the link between host A and R1 goes down, so that R1 loses netA.
- The rate limit of §3.10.1 may hold a triggered update back for up to 5 seconds after a
  previous one. A triggered update of each router is therefore due within 6 seconds of the
  change that causes it.
- Observation lasts 130 seconds.

### Procedure

1. Build the chain, and let the three routers start.
2. At 100 seconds, take down the link between host A and R1.
3. Observe the updates of R1 on L1 and of R2 on L2.

### Expected observations

1. On L2, from R2, before 100 seconds, an update that holds netA with metric 2. This confirms
   the stimulus: the route of netA has spread to R2.
2. On L1, from R1, within 6 seconds after 100 seconds, an update that holds netA with metric
   16 (RFC2453-TRIG-1, TIMER-5, GEN-5, TRIG-5).
3. That update is a response to 224.0.0.9 with UDP ports 520 and 520, command 2 and version 2
   (RFC2453-TRIG-9: the rules of every response).
4. On L2, from R2, within 6 seconds after observation 2, an update that holds netA with
   metric 16 (RFC2453-RESP-9: the metric of the next hop is adopted; TIMER-5 and TRIG-6 at
   R2).

### Notes

- Observation 2 does not ask whether the update is triggered or periodic; it asks whether the
  withdrawal leaves within 6 seconds. A periodic update is 25 to 35 seconds away, so a router
  that waited for its next periodic update would meet the 6 seconds only when the change fell
  just before one. The results record, for the run, which kind of update carried the
  withdrawal.
- A triggered update that holds the whole table is "strongly discouraged" and not forbidden;
  the check does not assert it, and the results record the number of entries.

## Triggered update for a lost network (RIPng)

Checks: **RFC2080-TRIG-4**, **TRIG-8**, **GEN-7** (must), **RFC2080-TRIG-5** (should),
**RFC2080-OUT-2**, **TIMER-5**, **RESP-11** (description).

### Requirement

RFC 2080 §2.5: whenever the metric of a route changes, an update is triggered. §2.5.1: the
triggered update holds at least the changed routes, goes out on every directly-connected
network, and follows every rule of a response — the link-local source and the hop limit of
255 included. §2.4.2: a router adopts the metric that the next hop sends.

### Scenario constants

- The chain, on IPv6.
- At 100 seconds, the link between host A and R1 goes down.
- Observation lasts 130 seconds.

### Procedure

1. Build the chain on IPv6, and let the three routers start.
2. At 100 seconds, take down the link between host A and R1.
3. Observe the updates of R1 on L1 and of R2 on L2.

### Expected observations

1. On L2, from R2, before 100 seconds, an update that holds netA with metric 2. This confirms
   the stimulus.
2. On L1, from R1, within 6 seconds after 100 seconds, an update that holds netA with metric
   16 (RFC2080-OUT-2, TIMER-5, TRIG-4, GEN-7).
3. That update goes to FF02::9 with UDP ports 521 and 521, from the link-local address of the
   L1 interface of R1, with hop limit 255 and command 2 (RFC2080-TRIG-8).
4. On L2, from R2, within 6 seconds after observation 2, an update that holds netA with
   metric 16 (RFC2080-RESP-11, TIMER-5, TRIG-5).
5. The update of observation 2 carries version 1 (RFC2080-TRIG-8).

## Rate of triggered updates (RIP version 2)

Checks: **RFC2453-TRIG-2** (must), **RFC2453-TRIG-3** (should).

### Requirement

RFC 2453 §3.4.4 and §3.10.1: an implementation must limit the rate of triggered updates.
After a triggered update, a timer of a random 1 to 5 seconds runs; changes before it expires
go out in a single update when it expires.

### Scenario constants

- The pair with three stubs. The marker of R1 is netA, which stays up.
- The links of netA1, netA2 and netA3 go down at 100.0, 100.3 and 100.6 seconds: three
  changes within 0.6 seconds.
- Observation lasts 120 seconds.

### Size or value arithmetic

With the timer of §3.10.1, the first change can go out at once and the next two wait for the
timer, 1 to 5 seconds later: two updates at most. A router without a rate limit sends three,
one for each change, within 0.6 seconds.

### Procedure

1. Build the pair with three stubs, and let both routers start.
2. Take down the links of netA1, netA2 and netA3 at the three instants.
3. Observe the updates of R1 on L1 from 100 seconds to 110 seconds.

### Expected observations

1. On L1, from R1, before 100 seconds, a periodic update that holds netA1, netA2 and netA3
   with metric 1. This confirms the stimulus.
2. From 100 to 110 seconds, the triggered updates of R1 on L1 that hold a metric of 16 for one
   of the three networks are at most two (RFC2453-TRIG-2).
3. Two such updates are at least 1 second apart (RFC2453-TRIG-3).
4. Each of the three networks appears with metric 16 in an update of R1 on L1 before 110
   seconds: the held changes are sent, not lost.

### Notes

- Observations 2 and 3 count triggered updates only: an update without the marker netA. A
  periodic update in the window also holds the three withdrawals, and it is not rate-limited
  by §3.10.1.
- The randomness of the timer is a distribution, level 4; the check tests the lower bound.

## Rate of triggered updates (RIPng)

Checks: **RFC2080-TRIG-1** (must, "requires"), **RFC2080-TRIG-2** (should).

### Requirement

RFC 2080 §2.5.1, the same rule as the IPv4 twin.

### Scenario constants

- The pair with three stubs, on IPv6. The marker of R1 is netA.
- The links of netA1, netA2 and netA3 go down at 100.0, 100.3 and 100.6 seconds.
- Observation lasts 120 seconds.

### Procedure

1. Build the pair with three stubs on IPv6, and let both routers start.
2. Take down the links of netA1, netA2 and netA3 at the three instants.
3. Observe the updates of R1 on L1 from 100 seconds to 110 seconds.

### Expected observations

1. On L1, from R1, before 100 seconds, a periodic update that holds netA1, netA2 and netA3
   with metric 1. This confirms the stimulus.
2. From 100 to 110 seconds, the triggered updates of R1 on L1 that hold a metric of 16 for one
   of the three networks are at most two (RFC2080-TRIG-1).
3. Two such updates are at least 1 second apart (RFC2080-TRIG-2).
4. Each of the three networks appears with metric 16 in an update of R1 on L1 before 110
   seconds.
