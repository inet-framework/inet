# ND — English check procedures: parameters from the router

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Hop limit from the router

Checks: **RFC4861-HOST-16** (should), **RA-9** (should, lower case).

### Requirement

RFC 4861 §6.3.4: a non-zero Cur Hop Limit in a Router Advertisement becomes the CurHopLimit of
the host, the hop limit of the packets that it sends.

### Scenario constants

- The link. R sets AdvCurHopLimit to 50 on L1, a value that differs from every default.
- At 10 seconds, host A sends one ICMPv6 echo request to the global address of host B.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with the router variable above, and let R, A and B come up.
2. At 10 seconds, let A send an echo request to B.
3. Observe L1.

### Expected observations

1. On L1, from R, a Router Advertisement with Cur Hop Limit 50. This confirms the stimulus.
2. On L1, from A, the echo request to B, with IPv6 hop limit 50 (RFC4861-HOST-16, RA-9).

### Notes

- If the router sends another Cur Hop Limit, observation 1 fails and the check gives no verdict
  on the host.

## Hop limit without a router

Checks: **RFC4861-HOST-1**, **HOST-4** (description).

### Requirement

RFC 4861 §6.3.2: a host uses the default of a host variable when no router tells it a value; the
default of CurHopLimit is the value of the Assigned Numbers registry, 64.

### Scenario constants

- The link without a router. At 10 seconds, host A sends one ICMPv6 echo request to the
  link-local address of host B.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link without a router, and let A and B come up.
2. At 10 seconds, let A send an echo request to B.
3. Observe L1.

### Expected observations

1. On L1, from A, the echo request to the link-local address of B. This confirms the stimulus.
2. Its IPv6 hop limit is 64 (RFC4861-HOST-1, HOST-4).

## Retransmission timer from the router

Checks: **RFC4861-HOST-19** (should).

### Requirement

RFC 4861 §6.3.4: a non-zero Retrans Timer in a Router Advertisement becomes the RetransTimer of
the host, the time between two Neighbor Solicitations of address resolution (§7.2.2).

### Scenario constants

- The link. R sets AdvRetransTimer to 2000 milliseconds on L1, twice the default of the host.
- At 10 seconds, host A sends one ICMPv6 echo request to 2001:db8:1::99, an address on L1 that no
  node has.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link with the router variable above, and let R, A and B come up.
2. At 10 seconds, let A send an echo request to 2001:db8:1::99.
3. Observe the Neighbor Solicitations of A with target 2001:db8:1::99 on L1.

### Expected observations

1. On L1, from R, a Router Advertisement with Retrans Timer 2000. This confirms the stimulus.
2. On L1, from A, a first Neighbor Solicitation with target 2001:db8:1::99. Record its instant.
3. The second solicitation with that target comes 2 to 3 seconds after the first, and the third
   2 to 3 seconds after the second (RFC4861-HOST-19).

### Notes

- The lower bound tells the value of the router from the default of 1 second. The upper bound
  reads "about every RetransTimer" of §7.2.2 as at most one and a half times the timer.

## MTU from the router

Checks: **RFC4861-HOST-24** (should), **HOST-3**, **OPT-49** (description).

### Requirement

RFC 4861 §6.3.4: a host takes the value of an MTU option as its LinkMTU when the value is between
the minimum IPv6 link MTU (1280) and the maximum of the link type. A host sends no IPv6 packet
larger than its LinkMTU onto the link.

### Scenario constants

- The link. R sets AdvLinkMTU to 1400 on L1.
- At 10 seconds, host A sends one ICMPv6 echo request with 1452 octets of data to the global
  address of host B.
- Observation lasts 20 seconds from the start.

### Size or value arithmetic

The echo request is 40 (IPv6 header) + 8 (ICMPv6 header) + 1452 = 1500 octets: it fits the
Ethernet MTU, but not the advertised MTU of 1400.

### Procedure

1. Build the link with the router variable above, and let R, A and B come up.
2. At 10 seconds, let A send the echo request to B.
3. Observe the IPv6 packets of A on L1.

### Expected observations

1. On L1, from R, a Router Advertisement with an MTU option of 1400. This confirms the stimulus.
2. On L1, from A, the echo request to B leaves in fragments, and no IPv6 packet from A is larger
   than 1400 octets (RFC4861-HOST-24, HOST-3, OPT-49).
