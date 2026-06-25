# BgpIpv6Basic — BGP over IPv6 (MP-BGP, RFC 4760)

A minimal IPv6 EBGP example: two single-router autonomous systems peer over IPv6 and each
advertises the IPv6 network behind it.

```
hostA --- rA (AS 65001) === rB (AS 65002) --- hostB
        2001:db8:0:10::/64   2001:db8:0:1::/64   2001:db8:0:20::/64
```

What it demonstrates / verifies:

- A BGP session is established over **IPv6 TCP** (the `Bgp` module runs with
  `addressFamily = "ipv6"` and drives the IPv6 routing table).
- IPv6 reachability is exchanged using the **`MP_REACH_NLRI`** path attribute
  (AFI=2 / SAFI=1), per RFC 4760 — rA advertises `2001:db8:0:10::/64`, rB advertises
  `2001:db8:0:20::/64`, each via a `<Network>` element in `BgpConfig6.xml`.
- The learned routes are actually **installed and used**: the configurator is run with
  `addRemoteRoutes = false`, so it adds the on-link (direct) routes and the host default
  routes automatically, but leaves out the inter-AS (remote) routes — those can come
  *only* from BGP. `hostA` pinging `hostB` therefore succeeds only once BGP has converged.

Notes:

- The BGP Identifier is a 4-octet router id even for IPv6 (RFC 4271/6286); it is set
  explicitly via `bgp.routerId` since an IPv6 router has no IPv4 address to derive it from.
- `addRemoteRoutes` (and the matching `addManualRoutes`) let the configurator install the
  directly-connected routes without the precomputed remote shortest-path routes that would
  otherwise mask the protocol under test; see `plan/pending/configurator-decouple-direct-routes.md`.
