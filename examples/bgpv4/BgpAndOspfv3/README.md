# BgpAndOspfv3 — BGP over an OSPFv3 IGP (IPv6)

BGP and OSPFv3 interoperating over IPv6: two autonomous systems, each running **OSPFv3** as
its interior gateway protocol (IGP), with **eBGP** between the ASes and a full **iBGP** mesh
inside each. This is the IPv6 counterpart of the IPv4 `BgpAndOspf` example, in the
"BGP over an IGP" arrangement (the way transit ASes are actually built), **without any route
redistribution** between the two protocols.

```
hostA --- rA1 --- rA2 --- rA3 ==eBGP== rB1 --- rB2 --- rB3 --- hostB
          \________ AS 65001 ________/        \____ AS 65002 ____/
              OSPFv3 + iBGP mesh                OSPFv3 + iBGP mesh
```

- AS 65001 = {rA1, rA2, rA3}, AS 65002 = {rB1, rB2, rB3}; rA3 and rB1 are the border routers.
- `hostA` (`2001:db8:1:1::/64`) sits behind rA1; `hostB` (`2001:db8:2:3::/64`) behind rB3.

## What it demonstrates

- **OSPFv3 is the IGP**: it distributes the intra-AS prefixes and, crucially, resolves the
  (i)BGP next hops. rA1 and rA3 are *not* adjacent, so their iBGP session and the next-hop
  resolution for the routes they exchange go through rA2 via OSPFv3 (symmetrically in AS 65002).
- **BGP carries the inter-AS prefixes** over IPv6 (MP-BGP, RFC 4760): rA1 originates hostA's
  subnet, rB3 originates hostB's; the border routers re-advertise across the eBGP link with
  `nextHopSelf` so the far AS resolves the next hop through its own OSPFv3.
- **No redistribution**: OSPFv3 and BGP run side by side and both install IPv6 routes into the
  same table; reachability composes. (`bgp.ospfRoutingModule = ""` because `ospf` here is
  `Ospfv3`, not the IPv4 `Ospfv2` that BGP's redistribution hooks expect.)
- **End-to-end check**: `hostA` pinging `hostB` succeeds only once OSPFv3 has converged
  (intra-AS reachability + next-hop resolution) **and** BGP has exchanged the inter-AS
  prefixes — there are no static inter-AS routes.

## Configs

- `General` — the full interop scenario above.
- `OspfOnly` — BGP disabled; `hostA` pings rA3's far interface, reachable only via the OSPFv3
  route through rA2. A quick check that OSPFv3 alone provides multi-hop intra-AS IPv6 routing.

## Notes / addressing

- Addresses are assigned by the `Ipv6NetworkConfigurator` (`Ipv6Config.xml`); OSPFv3 runs on
  those addresses (no `<IPv6Address>` in `Ospfv3Config.xml`). The configurator runs with
  `addRemoteRoutes = false`, so it provides only the on-link and host-default routes — intra-AS
  routes come from OSPFv3, inter-AS routes from BGP.
- OSPFv3 is configured on the intra-AS interfaces only; the inter-AS link rA3↔rB1 carries the
  eBGP session and is left out of OSPFv3.
- Because OSPFv3 converges relatively slowly (default hello/dead intervals), the iBGP sessions
  to non-adjacent peers are *deferred* until OSPFv3 installs a route to the peer; the ping
  therefore starts at t=70s, after convergence.
