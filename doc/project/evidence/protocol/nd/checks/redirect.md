# ND — English check procedures: redirect

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md), [rfc6980/catalog.md](../../../standard/rfc6980/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Redirect from the first-hop router

Checks: **RFC4861-RDM-1**, **RDM-4**, **RDM-11**, **RDM-14**, **RDR-1**, **RDR-6**
(description), **RDM-3**, **RDM-13**, **RDR-2**, **RDR-4** (must), **RDVAL-2**, **ADV-46**
(must, lower case), **RDR-3** (should).

### Requirement

RFC 4861 §8.2: a router that forwards a packet from a neighbor to a unicast destination, when a
better first hop for that destination is on the same link, sends the source of the packet a
Redirect. The Redirect leaves from the link-local address of the router; its Target Address is
the link-local address of the better router, the address from which that router sends its Router
Advertisements; its Destination Address is the destination of the packet.

### Scenario constants

- The two routers. R2 sets AdvDefaultLifetime to zero on L1, so R1 is the only default router of
  A; R1 reaches L2 through R2.
- R2 and C come up at 5 seconds, when A has finished its router discovery with R1. The check is
  about the Redirect, so it does not depend on the order in which a host hears the advertisements
  of two routers; [A router with Router Lifetime zero](router-discovery.md#a-router-with-router-lifetime-zero)
  checks that order.
- At 10 seconds, host A sends one ICMPv6 echo request to the global address of host C.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the two routers, and let R1 and A come up; at 5 seconds, let R2 and C come up.
2. At 10 seconds, let A send the echo request to C.
3. Observe L1.

### Expected observations

1. On L1, from A, the echo request to C, with the link-layer address of R1 as its Ethernet
   destination. This confirms the stimulus.
2. On L1, from R1, the same echo request, forwarded to the link-layer address of R2. This
   confirms that R1 forwards on the link that the packet came from.
3. On L1, from R1, a Redirect, ICMPv6 type 137, to the IPv6 source of the echo request
   (RFC4861-RDR-3, RDR-1, RDM-1, RDM-4).
4. Its IPv6 source is the link-local address of R1 on L1 (RFC4861-RDM-3, RDVAL-2).
5. Its Target Address is the link-local address of R2 on L1, which is also the source of the
   Router Advertisements of R2 (RFC4861-RDR-2, RDR-4, RDM-11, RDM-13, ADV-46).
6. Its Destination Address is the global address of C (RFC4861-RDM-14, RDR-6).

### Notes

- RFC4861-RDR-3 is a `should`: a router that sends no Redirect fails observation 3, and the
  ledger keeps the strength.

## Redirect fields

Checks: **RFC4861-RDM-5**, **RDM-6**, **RDM-7**, **RDR-7**, **OPT-12**, **OPT-38**, **OPT-39**
(description), **RDM-9**, **OPT-40** (must), **RDM-15** (should); covers **RFC4861-RDM-2**,
**OPT-37** (encoding).

### Requirement

RFC 4861 §4.5 and §4.6: a Redirect has hop limit 255, type 137, code 0 and zero in its Reserved
field; it carries a Target Link-Layer Address option with the link-layer address of the target
when the router knows it, and a Redirected Header option of type 4, with its length in units of
8 octets, zero in its Reserved fields, and the start of the packet that caused the Redirect.

### Scenario constants

- As in [Redirect from the first-hop router](#redirect-from-the-first-hop-router).

### Procedure

1. As in [Redirect from the first-hop router](#redirect-from-the-first-hop-router).

### Expected observations

1. On L1, from R1, the Redirect to A. This confirms the stimulus.
2. It carries a Target Link-Layer Address option, type 2, with the link-layer address of R2
   (RFC4861-RDM-15, RDR-7, OPT-12).
3. It carries a Redirected Header option of type 4, whose length times 8 octets is its size, with
   zero in its Reserved fields, and whose data starts with the IPv6 header of the echo request
   (RFC4861-OPT-38, OPT-39, OPT-40).
4. It has hop limit 255, code 0 and zero in its Reserved field (RFC4861-RDM-5, RDM-6, RDM-7,
   RDM-9).

## Redirect of a large packet

Checks: **RFC4861-RDM-17**, **RDR-8**, **OPT-42** (description), **RFC6980-FRAG-1** (must not).

### Requirement

RFC 4861 §4.5 and §8.2: the Redirected Header option holds as much of the packet as fits, so that
the Redirect is not larger than the minimum IPv6 MTU, 1280 octets. RFC 6980 §5: a node does not
use IPv6 fragmentation to send a Neighbor Discovery message.

### Scenario constants

- As in [Redirect from the first-hop router](#redirect-from-the-first-hop-router), but the echo
  request carries 1400 octets of data.

### Size or value arithmetic

The echo request is 40 + 8 + 1400 = 1448 octets. A Redirect that carried it whole would be 40
(IPv6 header) + 40 (Redirect with its Target Address and Destination Address) + 8 (Target
Link-Layer Address option) + 8 (header of the Redirected Header option) + 1448 = 1544 octets:
more than the link MTU of 1500 as well as 1280. With the Target Link-Layer Address option, the
option has room for 1280 − 40 − 40 − 8 − 8 = 1184 octets of the packet, and without it for
1192 octets; the option size is a multiple of 8, so the Redirect is between 1273 and 1280 octets
in both cases.

### Procedure

1. As in [Redirect from the first-hop router](#redirect-from-the-first-hop-router), with the
   larger echo request.

### Expected observations

1. On L1, from R1, a Redirect to A. This confirms the stimulus.
2. It has no Fragment header (RFC6980-FRAG-1).
3. Its IPv6 packet is at most 1280 octets (RFC4861-RDR-8, OPT-42).
4. Its IPv6 packet is at least 1273 octets: the option holds as much of the echo request as fits
   (RFC4861-RDM-17, RDR-8).

## Host follows a Redirect

Checks: **RFC4861-RDH-1**, **RDH-2** (should), **RDH-10** (must not).

### Requirement

RFC 4861 §8.3: a host that receives a valid Redirect updates its Destination Cache, and creates
an entry when none exists, so that later packets for the destination go to the target. A host
never sends a Redirect.

### Scenario constants

- As in [Redirect from the first-hop router](#redirect-from-the-first-hop-router), but host A
  sends one echo request to C every second, from 10 to 14 seconds.

### Procedure

1. As in [Redirect from the first-hop router](#redirect-from-the-first-hop-router), with the five
   echo requests.

### Expected observations

1. On L1, from R1, a Redirect to A with the global address of C as its Destination Address. This
   confirms the stimulus.
2. On L1, from A, every echo request to C after that Redirect has the link-layer address of R2 as
   its Ethernet destination (RFC4861-RDH-1, RDH-2).
3. No Redirect leaves A or C in the window (RFC4861-RDH-10).

## Redirect to an on-link destination

Checks: **RFC4861-RDM-12**, **RDR-5**, **RDH-6** (must).

### Requirement

RFC 4861 §8.2: when the better first hop is the destination itself, the Target Address of the
Redirect equals its Destination Address. §8.3: a host that receives such a Redirect treats the
target as on-link.

### Scenario constants

- The link. R sets AdvOnLinkFlag to FALSE for 2001:db8:1::/64, and keeps AdvAutonomousFlag TRUE,
  so that A sends its first packet for B to R.
- Host A sends one ICMPv6 echo request to the global address of host B every second, from 10 to
  14 seconds.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with the router variable above, and let R, A and B come up.
2. From 10 seconds, let A send the echo requests to B.
3. Observe L1.

### Expected observations

1. On L1, from A, the first echo request to B, with the link-layer address of R as its Ethernet
   destination. This confirms the stimulus.
2. On L1, from R, a Redirect to A whose Target Address and Destination Address are both the
   global address of B (RFC4861-RDM-12, RDR-5).
3. On L1, from A, every echo request to B after that Redirect has the link-layer address of B as
   its Ethernet destination (RFC4861-RDH-6).
