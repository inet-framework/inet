# RIP — English check procedures: route expiry

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Route expiry after a silent neighbor (RIP version 2)

Checks: **RFC2453-TIMER-4**, **TIMER-5** (description).

### Requirement

RFC 2453 §3.8: the timeout of a route starts again with each update for the route; 180
seconds after the last one, the route expires. Then the metric becomes 16, a triggered
update goes out, and the garbage-collection timer of 120 seconds starts. Until it expires,
the route is in every update with metric 16; then it is removed. §3.9.2: a new destination
that arrives with metric 16 is not added.

### Scenario constants

- The chain. R1 fails at 100 seconds without notice. R3 is down from the start and starts at
  300 seconds.
- R1 comes up at 1 second and R2 at 16 seconds, so that the periodic updates of R2 fall about
  15 seconds after those of R1. The expiry of the route of R2 then falls between two periodic
  updates of R2, and the check does not depend on when R2 sends.
- The marker of R2 on L2 is L1.
- Observation lasts 460 seconds.

### Size or value arithmetic

Let t_last be the instant of the last update of R1 on L1 before the failure. Then:

- the route of R2 to netA expires at t_last + 180 seconds; R2 withdraws it at that instant,
  or up to 5 seconds later if the rate limit holds the triggered update back;
- the garbage collection ends 120 seconds after the expiry;
- t_last lies between 65 and 100 seconds, so the expiry lies between 245 and 280 seconds, and
  the end of the garbage collection between 365 and 405 seconds. R3, which starts at 300
  seconds, starts inside the garbage collection in every case.

### Procedure

1. Build the chain, with R3 down.
2. Let R1 and R2 start. At 100 seconds, let R1 fail; at 300 seconds, let R3 start.
3. Observe the updates of R1 on L1 and of R2 on L2.

### Expected observations

1. On L1, from R1, the updates up to 100 seconds; record the instant of the last one that
   holds netA, t_last. This confirms the first half of the stimulus.
2. On L2, from R2, before 100 seconds, an update that holds netA with metric 2. This confirms
   the second half: R2 learned netA from R1.
3. On L2, from R2, no update holds netA with metric 16 before t_last + 180 seconds
   (RFC2453-TIMER-4: the route does not expire early).
4. On L2, from R2, between t_last + 180 and t_last + 186 seconds, an update that holds netA
   with metric 16 (RFC2453-TIMER-4, TIMER-5).

### Notes

- Observation 3 is the absence that proves the timeout is 180 seconds and not shorter;
  observation 4 is the presence that proves it is not longer. The window of observation 4 is
  the 5 seconds of the rate limit and 1 second of margin.
- The 15 seconds between the start of R1 and the start of R2 matter for observation 4. If the
  periodic updates of R2 fell just after those of R1, a router that noticed the expiry only
  when it next sent a periodic update would still meet the window of 6 seconds.
- What follows the withdrawal — the garbage collection, and a router that hears the route only
  at metric 16 — is the next check. It is a check of its own so that a withdrawal at the wrong
  instant does not keep it from a verdict.

## Garbage collection after an expiry (RIP version 2)

Checks: **RFC2453-TIMER-6**, **RESP-7** (description); covers **RFC2453-TIMER-3**.

### Requirement

RFC 2453 §3.8: after the deletion starts, the route stays in every update with metric 16
until the garbage-collection timer of 120 seconds expires; then it is removed. §3.9.2: a new
destination that arrives with metric 16 is not added.

### Scenario constants

- The same network and the same events as
  [Route expiry after a silent neighbor](#route-expiry-after-a-silent-neighbor-rip-version-2):
  R1 fails at 100 seconds, R3 is down from the start and starts at 300 seconds, R1 comes up at
  1 second and R2 at 16 seconds. The marker of R2 on L2 is L1.
- The garbage collection starts when R2 withdraws the route, whenever that is; this check
  measures from the withdrawal it sees. The withdrawal comes at 245 seconds at the earliest,
  so the garbage collection ends at 365 seconds at the earliest, after R3 has started at 300
  seconds.
- Observation lasts 460 seconds.

### Procedure

1. Build the chain, with R3 down.
2. Let R1 and R2 start. At 100 seconds, let R1 fail; at 300 seconds, let R3 start.
3. Observe the updates of R2 on L2 and of R3 on netC.

### Expected observations

1. On L2, from R2, before 100 seconds, an update that holds netA with metric 2. This confirms
   the first half of the stimulus: R2 learned netA from R1.
2. On L2, from R2, after 100 seconds, an update that holds netA with metric 16. Record its
   instant, t_wd. This confirms the second half: the deletion has started.
3. On L2, from R2, every periodic update from t_wd to t_wd + 115 seconds holds netA with
   metric 16 (RFC2453-TIMER-6, the first half).
4. On L2, from R2, no update after t_wd + 121 seconds holds netA (RFC2453-TIMER-6, the second
   half).
5. On netC, from R3, after 300 seconds, an update that holds L1 with metric 2. This confirms
   that R3 learns routes from R2.
6. On netC, from R3, no update holds netA up to the end of the window (RFC2453-RESP-7: R3
   heard netA only with metric 16, and did not add it).

### Notes

- Observations 3 and 4 bound the garbage collection from both sides. The 1 second of margin on
  each side keeps a periodic update that falls on the boundary from deciding the check.
- R3 asks for the table when it starts, and R2 answers with netA at metric 16, as the rule of
  §3.10.2 demands. Observation 6 is what R3 does with that entry.

## Route expiry after a silent neighbor (RIPng)

Checks: **RFC2080-TIMER-4**, **TIMER-5** (description).

### Requirement

RFC 2080 §2.3 and §2.4.2, the same rules as the IPv4 twin: expiry 180 seconds after the last
update, metric 16 and a triggered update, 120 seconds of garbage collection, and no new route
from an entry with metric 16.

### Scenario constants

- The chain, on IPv6. R1 fails at 100 seconds. R3 is down from the start and starts at 300
  seconds.
- R1 comes up at 1 second and R2 at 16 seconds, as in the IPv4 twin.
- The marker of R2 on L2 is L1.
- Observation lasts 460 seconds.

### Procedure

1. Build the chain on IPv6, with R3 down.
2. Let R1 and R2 start. At 100 seconds, let R1 fail; at 300 seconds, let R3 start.
3. Observe the updates of R1 on L1 and of R2 on L2.

### Expected observations

1. On L1, from R1, the updates up to 100 seconds; record t_last. This confirms the first half
   of the stimulus.
2. On L2, from R2, before 100 seconds, an update that holds netA with metric 2.
3. On L2, from R2, no update holds netA with metric 16 before t_last + 180 seconds
   (RFC2080-TIMER-4).
4. On L2, from R2, between t_last + 180 and t_last + 186 seconds, an update that holds netA
   with metric 16 (RFC2080-TIMER-4, TIMER-5).

## Garbage collection after an expiry (RIPng)

Checks: **RFC2080-TIMER-6**, **RESP-9** (description); covers **RFC2080-TIMER-3**.

### Requirement

RFC 2080 §2.3 and §2.4.2, the same rules as the IPv4 twin: 120 seconds of garbage collection
with metric 16 after the deletion, and no new route from an entry with metric 16.

### Scenario constants

- The same network and the same events as
  [Route expiry after a silent neighbor (RIPng)](#route-expiry-after-a-silent-neighbor-ripng).
- Observation lasts 460 seconds.

### Procedure

1. Build the chain on IPv6, with R3 down.
2. Let R1 and R2 start. At 100 seconds, let R1 fail; at 300 seconds, let R3 start.
3. Observe the updates of R2 on L2 and of R3 on netC.

### Expected observations

1. On L2, from R2, before 100 seconds, an update that holds netA with metric 2. This confirms
   the first half of the stimulus.
2. On L2, from R2, after 100 seconds, an update that holds netA with metric 16. Record t_wd.
3. On L2, from R2, every periodic update from t_wd to t_wd + 115 seconds holds netA with
   metric 16 (RFC2080-TIMER-6).
4. On L2, from R2, no update after t_wd + 121 seconds holds netA (RFC2080-TIMER-6).
5. On netC, from R3, after 300 seconds, an update that holds L1 with metric 2.
6. On netC, from R3, no update holds netA up to the end of the window (RFC2080-RESP-9).

## Garbage collection of a lost network (RIP version 2)

Checks: **RFC2453-TIMER-5**, **TIMER-6** (description).

### Requirement

RFC 2453 §3.8: when a route is deleted, the garbage-collection timer is set for 120 seconds
and the metric becomes 16. Until the timer expires, the route is in every update; when it
expires, the route is deleted from the routing table. The rule holds for every route, the
route to a network of the router itself included.

### Scenario constants

- The pair with three stubs. The marker of R1 on L1 is netA.
- At 100 seconds, the link between host A1 and R1 goes down, so that R1 loses netA1.
- Observation lasts 320 seconds.

### Procedure

1. Build the pair with three stubs, and let both routers start.
2. At 100 seconds, take down the link of netA1.
3. Observe the updates of R1 on L1.

### Expected observations

1. On L1, from R1, before 100 seconds, a periodic update that holds netA1 with metric 1. This
   confirms the stimulus.
2. On L1, from R1, within 6 seconds after 100 seconds, an update that holds netA1 with metric
   16. Record its instant, t_wd.
3. Every periodic update of R1 on L1 from t_wd to t_wd + 115 seconds holds netA1 with metric
   16 (RFC2453-TIMER-6, the first half).
4. No update of R1 on L1 after t_wd + 121 seconds holds netA1 (RFC2453-TIMER-5: a timer of 120
   seconds; TIMER-6, the second half).

### Notes

- The route that R1 deletes here is a route to one of its own networks, not a route it
  learned. §3.8 makes no difference between the two: "Upon expiration of the
  garbage-collection timer, the route is finally removed from the routing table."

## Garbage collection while the next hop still withdraws (RIP version 2)

Checks: **RFC2453-RESP-10**, **TIMER-6** (description).

### Requirement

RFC 2453 §3.9.2: the deletion starts only when the metric first becomes 16; if the metric was
already 16, no new deletion starts. §3.8: the route leaves the updates when the
garbage-collection timer of 120 seconds expires.

### Scenario constants

- The pair with three stubs. The marker of R2 on netB is L1.
- At 100 seconds, the link between host A1 and R1 goes down. R1 withdraws netA1, and R2, whose
  next hop for netA1 is R1, deletes the route in turn.
- R1 keeps sending netA1 with metric 16 until its own garbage collection ends, at about the
  instant when the garbage collection of R2 ends too.
- Observation lasts 320 seconds.

### Procedure

1. Build the pair with three stubs, and let both routers start.
2. At 100 seconds, take down the link of netA1.
3. Observe the updates of R2 on netB.

### Expected observations

1. On netB, from R2, before 100 seconds, an update that holds netA1 with metric 2. This
   confirms the stimulus.
2. On netB, from R2, within 12 seconds after 100 seconds, an update that holds netA1 with
   metric 16. Record its instant, t_wd.
3. Every periodic update of R2 on netB from t_wd to t_wd + 115 seconds holds netA1 with metric
   16 (RFC2453-TIMER-6, the first half).
4. No update of R2 on netB after t_wd + 121 seconds holds netA1, whatever R1 still sends
   (RFC2453-RESP-10: an entry with metric 16 for a route that is already at 16 starts no new
   deletion; TIMER-6, the second half).

### Notes

- The 12 seconds of observation 2 are the 6 seconds of the triggered update of R1 and the 6
  seconds of the one of R2.
- A router that started its deletion again with each entry of metric 16 from R1 would keep
  netA1 in its updates for 120 seconds after the last such entry.

## Garbage collection of a lost network (RIPng)

Checks: **RFC2080-TIMER-5**, **TIMER-6** (description).

### Requirement

RFC 2080 §2.3, the same rule as the IPv4 twin.

### Scenario constants

- The pair with three stubs, on IPv6. The marker of R1 on L1 is netA.
- At 100 seconds, the link between host A1 and R1 goes down.
- Observation lasts 320 seconds.

### Procedure

1. Build the pair with three stubs on IPv6, and let both routers start.
2. At 100 seconds, take down the link of netA1.
3. Observe the updates of R1 on L1.

### Expected observations

1. On L1, from R1, before 100 seconds, a periodic update that holds netA1 with metric 1.
2. On L1, from R1, within 6 seconds after 100 seconds, an update that holds netA1 with metric
   16. Record t_wd.
3. Every periodic update of R1 on L1 from t_wd to t_wd + 115 seconds holds netA1 with metric
   16 (RFC2080-TIMER-6).
4. No update of R1 on L1 after t_wd + 121 seconds holds netA1 (RFC2080-TIMER-5, TIMER-6).

## Garbage collection while the next hop still withdraws (RIPng)

Checks: **RFC2080-RESP-12**, **TIMER-6** (description).

### Requirement

RFC 2080 §2.4.2 and §2.3, the same rules as the IPv4 twin.

### Scenario constants

- The pair with three stubs, on IPv6. The marker of R2 on netB is L1.
- At 100 seconds, the link between host A1 and R1 goes down.
- Observation lasts 320 seconds.

### Procedure

1. Build the pair with three stubs on IPv6, and let both routers start.
2. At 100 seconds, take down the link of netA1.
3. Observe the updates of R2 on netB.

### Expected observations

1. On netB, from R2, before 100 seconds, an update that holds netA1 with metric 2.
2. On netB, from R2, within 12 seconds after 100 seconds, an update that holds netA1 with
   metric 16. Record t_wd.
3. Every periodic update of R2 on netB from t_wd to t_wd + 115 seconds holds netA1 with metric
   16 (RFC2080-TIMER-6).
4. No update of R2 on netB after t_wd + 121 seconds holds netA1, whatever R1 still sends
   (RFC2080-RESP-12, TIMER-6).

## Garbage collection ended by a new route (RIP version 2)

Checks: **RFC2453-TIMER-7** (must).

### Requirement

RFC 2453 §3.8: when a new route to a destination arrives while its garbage-collection timer
runs, the new route replaces the old one, and the garbage-collection timer must be cleared.

### Scenario constants

- The pair. The marker of R2 on netB is L1.
- The link between host A and R1 goes down at 100 seconds and comes back at 130 seconds.
- R1 fails at 200 seconds without notice. After that, nothing refreshes the route of R2 to
  netA, and nothing can bring it back once it is gone.
- Observation lasts 340 seconds.

### Size or value arithmetic

R2 withdraws netA at t_del, between 100 and 112 seconds (the triggered update of R1 and then
the one of R2, each up to 6 seconds). The garbage collection that started at t_del would end
at t_del + 120, between 220 and 232 seconds. The new route arrives at about 130 seconds and
must clear it. R1 fails at 200 seconds; its last update is at 170 seconds at the earliest, so
the route of R2 through R1 is valid until 350 seconds at the earliest.

### Procedure

1. Build the pair, and let both routers start.
2. At 100 seconds take down the link between host A and R1; at 130 seconds bring it back.
3. At 200 seconds, let R1 fail.
4. Observe the updates of R2 on netB.

### Expected observations

1. On netB, from R2, before 100 seconds, an update that holds netA with metric 2. This
   confirms the stimulus.
2. On netB, from R2, between 100 and 112 seconds, an update that holds netA with metric 16.
   Record its instant, t_del. The deletion, and its garbage collection, have started.
3. On netB, from R2, after 130 seconds and before t_del + 120 seconds, an update that holds
   netA with metric 2: the new route arrived during the garbage collection.
4. On netB, from R2, every periodic update from t_del + 125 seconds to 340 seconds holds netA
   with metric 2 (RFC2453-TIMER-7: the old garbage collection did not remove the new route).

### Notes

- The failure of R1 at 200 seconds is what makes observation 4 decisive. Without it, a router
  that removed the route at t_del + 120 by mistake would learn it again from the next update
  of R1, and the gap could fall between two periodic updates of R2 and stay unseen.

## Garbage collection ended by a new route (RIPng)

Checks: **RFC2080-TIMER-7** (must).

### Requirement

RFC 2080 §2.3, the same rule as the IPv4 twin.

### Scenario constants

- The pair, on IPv6. The marker of R2 on netB is L1.
- The link between host A and R1 goes down at 100 seconds and comes back at 130 seconds. R1
  fails at 200 seconds.
- Observation lasts 340 seconds.

### Procedure

1. Build the pair on IPv6, and let both routers start.
2. At 100 seconds take down the link between host A and R1; at 130 seconds bring it back.
3. At 200 seconds, let R1 fail.
4. Observe the updates of R2 on netB.

### Expected observations

1. On netB, from R2, before 100 seconds, an update that holds netA with metric 2. This
   confirms the stimulus.
2. On netB, from R2, between 100 and 112 seconds, an update that holds netA with metric 16.
   Record t_del.
3. On netB, from R2, after 130 seconds and before t_del + 120 seconds, an update that holds
   netA with metric 2.
4. On netB, from R2, every periodic update from t_del + 125 seconds to 340 seconds holds netA
   with metric 2 (RFC2080-TIMER-7).
