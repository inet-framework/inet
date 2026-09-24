# ND — English check procedures: on-link determination

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md), [rfc5942/catalog.md](../../../standard/rfc5942/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## On-link neighbor reached directly

Checks: **RFC4861-HOST-30**, **OPT-18**, **OPT-19**, **AR-1**, **RFC5942-ONLINK-2**
(description), **RFC4861-HOST-25** (should, lower case).

### Requirement

RFC 4861 §6.3.4: a Prefix Information option with the L flag set puts its prefix into the Prefix
List, and the addresses of the prefix are on-link. §7.2: a node resolves the link-layer address
of an on-link destination and sends the packet to that neighbor directly. RFC 5942 §4: only such
explicit means make a prefix on-link.

### Scenario constants

- The link. At 10 seconds, host A sends one ICMPv6 echo request to the global address of host B.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A send an echo request to B.
3. Observe L1.

### Expected observations

1. On L1, from R, a Router Advertisement with a Prefix Information option for 2001:db8:1::/64
   with the L flag set. This confirms the stimulus.
2. On L1, from A, a Neighbor Solicitation with the global address of B as its target
   (RFC4861-HOST-25, HOST-30, OPT-18, OPT-19, RFC5942-ONLINK-2).
3. On L1, from A, the echo request to B, with the link-layer address of B as its Ethernet
   destination (RFC4861-AR-1).

## Prefix with the on-link flag clear

Checks: **RFC5942-ONLINK-1**, **RFC4861-HOST-26** (must not), **RFC4861-HOST-28**,
**RFC5942-ONLINK-8** (description).

### Requirement

RFC 5942 §4: the assignment of an address does not make the prefix of that address on-link.
RFC 4861 §6.3.4: a Prefix Information option with the L flag clear says nothing about on-link,
and a host sends a packet for a destination of unknown on-link status to a default router.

### Scenario constants

- The link. R sets AdvOnLinkFlag to FALSE for 2001:db8:1::/64, and keeps AdvAutonomousFlag TRUE,
  so that A and B form their global addresses from a prefix that is not on-link.
- At 10 seconds, host A sends one ICMPv6 echo request to the global address of host B.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with the router variable above, and let R, A and B come up.
2. At 10 seconds, let A send an echo request to B.
3. Observe L1.

### Expected observations

1. On L1, from R, a Router Advertisement with a Prefix Information option for 2001:db8:1::/64
   with the L flag clear and the A flag set. This confirms the stimulus.
2. On L1, from A, the first echo request to B, with the link-layer address of R as its Ethernet
   destination (RFC4861-HOST-26, HOST-28, RFC5942-ONLINK-1, ONLINK-8).
3. No Neighbor Solicitation from A with the global address of B as its target before that first
   echo request (RFC5942-ONLINK-1).

### Notes

- Only the first packet counts. R forwards it to B on the same link and may send A a Redirect
  that makes B on-link; the check of that Redirect is in
  [`redirect.md`](redirect.md#redirect-to-an-on-link-destination).

## No router and no on-link prefix

Checks: **RFC5942-ONLINK-5**, **ONLINK-6** (must not).

### Requirement

RFC 5942 §4: a host with an empty Default Router List and no on-link information does not assume
that a destination is on-link, and it does not do address resolution for a destination that is
not link-local.

### Scenario constants

- The link without a router. At 10 seconds, host A sends one ICMPv6 echo request to
  2001:db8:1::b; at 12 seconds, it sends one to the link-local address of host B.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link without a router, and let A and B come up.
2. At 10 seconds, let A send an echo request to 2001:db8:1::b; at 12 seconds, let A send an echo
   request to the link-local address of B.
3. Observe L1.

### Expected observations

1. On L1, from A, after 12 seconds, the echo request to the link-local address of B. This
   confirms that A sends when it has a way to the destination.
2. No Neighbor Solicitation from A with 2001:db8:1::b as its target (RFC5942-ONLINK-6).
3. No echo request from A to 2001:db8:1::b (RFC5942-ONLINK-5).

## Prefix after its valid lifetime

Checks: **RFC5942-ONLINK-3** (must), **RFC4861-HOST-36**, **OPT-24** (description); covers
**RFC4861-HOST-34** (description).

### Requirement

RFC 4861 §6.3.5: when the invalidation timer of a prefix expires, the host removes the prefix
from its Prefix List; the Valid Lifetime counts from the transmission of the advertisement, and
Neighbor Discovery sets no minimum for it. RFC 5942 §4: the addresses of that prefix are then
off-link, when no other on-link information covers them.

### Scenario constants

- The link. R sets AdvValidLifetime to 30 seconds and AdvPreferredLifetime to 20 seconds for
  2001:db8:1::/64.
- R stops at 15 seconds, without a last advertisement.
- Host A sends one ICMPv6 echo request to the global address of host B every second, from 10 to
  80 seconds.
- Observation lasts 80 seconds from the start.

### Size or value arithmetic

The last advertisement of R leaves at 15 seconds or earlier, so the prefix is invalid at
15 + 30 = 45 seconds at the latest. The check judges from 50 seconds on.

### Procedure

1. Build the link with the router variables above, and let R, A and B come up.
2. From 10 seconds, let A send the echo requests to B. At 15 seconds, stop R.
3. Observe L1.

### Expected observations

1. On L1, from A, before 15 seconds, an echo request to B with the link-layer address of B as its
   Ethernet destination. This confirms the stimulus: the prefix is on-link.
2. After 50 seconds, no echo request from A to B has the link-layer address of B as its Ethernet
   destination (RFC5942-ONLINK-3, RFC4861-HOST-36, OPT-24).
3. After 50 seconds, no Neighbor Solicitation from A has the global address of B as its target
   (RFC5942-ONLINK-3).

### Notes

- A lifetime of 30 seconds is far below the defaults; it covers RFC4861-HOST-34, that on-link
  determination accepts any Valid Lifetime.
- The global address of A expires at the same instant. A host that then sends nothing to B passes
  observations 2 and 3; [`autoconfiguration.md`](autoconfiguration.md#address-after-its-valid-lifetime)
  checks the source address in the same scenario.
