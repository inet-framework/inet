# RIP — English check procedures: learning routes

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc2453/catalog.md](../../../standard/rfc2453/catalog.md), [rfc2080/catalog.md](../../../standard/rfc2080/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Metric through a chain (RIP version 2)

Checks: **RFC2453-MET-5**, **RESP-6**, **TABLE-3** (description); covers **RFC2453-NH-1**.

### Requirement

RFC 2453 §3.9.2: a router adds the cost of the arrival network to the metric of a received
entry, and it adds a new destination with that metric and with the sender of the datagram
as next hop. §4.4: a next hop of 0.0.0.0 means the sender.

### Scenario constants

- The chain. The marker of R1 is netA, of R2 is L2, of R3 is netC.
- At 90 seconds, host A sends an ICMP echo request to host C.
- Observation lasts 120 seconds.

### Size or value arithmetic

netA has cost 1 at R1. R2 adds the cost of L1, 1, and R3 adds the cost of L2, 1. So R1
advertises netA with metric 1, R2 with metric 2, and R3 with metric 3.

### Procedure

1. Build the chain, and let the three routers start.
2. Observe the updates of R1 on L1, of R2 on L2 and of R3 on netC.
3. At 90 seconds, let host A send an echo request to host C.

### Expected observations

1. On L1, from R1, an update that holds netA with metric 1. This confirms the stimulus.
2. On L2, from R2, an update that holds netA with metric 2 (RFC2453-MET-5, RESP-6).
3. On netC, from R3, an update that holds netA with metric 3 (RFC2453-MET-5, RESP-6).
4. The echo reply of host C reaches host A (RFC2453-TABLE-3, covers NH-1): each router
   forwards to the router it learned the route from.

### Notes

- Observations 2 and 3 can come in either order relative to each other's first
  appearance; each is a search for the first such update of its router.
- Observation 4 needs routes in both directions, and it cannot tell which router got a next
  hop wrong. It is the end-to-end half of the check; the wire half is observations 2 and 3.

## Metric through a chain (RIPng)

Checks: **RFC2080-MET-5**, **RESP-8**, **TABLE-2** (description).

### Requirement

RFC 2080 §2.4.2: a router adds the cost of the arrival network to the metric of a received
entry, and it adds a new destination with that metric and with the sender of the datagram
as next hop.

### Scenario constants

- The chain, on IPv6. The markers are as in the IPv4 twin.
- At 90 seconds, host A sends an ICMPv6 echo request to host C.
- Observation lasts 120 seconds.

### Procedure

1. Build the chain on IPv6, and let the three routers start.
2. Observe the updates of R1 on L1, of R2 on L2 and of R3 on netC.
3. At 90 seconds, let host A send an echo request to host C.

### Expected observations

1. On L1, from R1, an update that holds netA with metric 1. This confirms the stimulus.
2. On L2, from R2, an update that holds netA with metric 2 (RFC2080-MET-5, RESP-8).
3. On netC, from R3, an update that holds netA with metric 3 (RFC2080-MET-5, RESP-8).
4. The echo reply of host C reaches host A (RFC2080-TABLE-2).

## Shorter path kept (RIP version 2)

Checks: **RFC2453-RESP-9**, **RFC2453-RESP-12** (description).

### Requirement

RFC 2453 §3.9.2: a router adopts the route of a datagram when its metric is lower than the
metric of the existing route, or when the datagram comes from the next hop of the existing
route. Any other entry is ignored, as it is no better than the current route.

### Scenario constants

- The triangle. The marker of R2 is L1, of R3 is netC.
- Observation lasts 200 seconds.

### Size or value arithmetic

R3 hears of netA twice: from R1 over L3, metric 1 + 1 = 2, and from R2 over L2, where R2
advertises metric 2 (1 over L1), so 2 + 1 = 3. The shorter path is the one over L3, and R3
must keep metric 2 whichever path it hears first.

### Procedure

1. Build the triangle, and let the three routers start.
2. Observe the updates of R2 on L2 and of R3 on netC.

### Expected observations

1. On L2, from R2, an update that holds netA with metric 2. This confirms the stimulus: R3
   is offered the longer path.
2. On netC, from R3, a periodic update after 60 seconds that holds netA with metric 2
   (RFC2453-RESP-9: the lower metric is adopted).
3. Every update of R3 on netC from then to the end of the window holds netA with metric 2
   (RFC2453-RESP-12: the longer path, offered every 30 seconds, is ignored).

### Notes

- R3 may hear the longer path first, at startup, and take it until the shorter one arrives.
  The first 60 seconds are therefore outside the observation.

## Shorter path kept (RIPng)

Checks: **RFC2080-RESP-11**, **RFC2080-RESP-14** (description).

### Requirement

RFC 2080 §2.4.2, the same rule as the IPv4 twin: a lower metric is adopted, the news of the
current next hop is adopted, and any other entry is ignored.

### Scenario constants

- The triangle, on IPv6. The markers are as in the IPv4 twin.
- Observation lasts 200 seconds.

### Procedure

1. Build the triangle on IPv6, and let the three routers start.
2. Observe the updates of R2 on L2 and of R3 on netC.

### Expected observations

1. On L2, from R2, an update that holds netA with metric 2. This confirms the stimulus.
2. On netC, from R3, a periodic update after 60 seconds that holds netA with metric 2
   (RFC2080-RESP-11).
3. Every update of R3 on netC from then to the end of the window holds netA with metric 2
   (RFC2080-RESP-14).
