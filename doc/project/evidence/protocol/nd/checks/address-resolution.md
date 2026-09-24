# ND — English check procedures: address resolution

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc4861/catalog.md](../../../standard/rfc4861/catalog.md), [rfc4862/catalog.md](../../../standard/rfc4862/catalog.md)

The common mockups, the rules and the index are in [`checks.md`](../checks.md). The
procedures come from the specification only. They name no simulation model and no code.

## Address resolution of a host

Checks: **RFC4861-AR-8**, **AR-9**, **AR-18**, **AR-31**, **AR-32**, **AR-41**, **AR-44**,
**NS-1**, **NS-2**, **NS-5**, **NS-12**, **NA-1**, **NA-4**, **NA-21** (description), **AR-12**,
**AR-14**, **AR-34**, **NS-15**, **NA-24** (must), **AR-15** (must, may), **AR-10**, **AR-24**
(should); covers **RFC4861-AR-30**, **RFC4862-DAD-18**, **DAD-26** (description).

### Requirement

RFC 4861 §7.2.2: a node with a packet for an on-link neighbor of unknown link-layer address
queues the packet and sends a Neighbor Solicitation to the solicited-node address of the target,
from the source of the queued packet, with a Source Link-Layer Address option. §7.2.3 and §7.2.4:
the target records the link-layer address of the sender and answers with a Neighbor Advertisement
to the source of the solicitation, with the same Target Address and a Target Link-Layer Address
option. §7.2.5: the sender then records the address and sends the queued packet.

### Scenario constants

- The link. At 10 seconds, host A sends one ICMPv6 echo request to the global address of host B;
  no packet between A and B precedes it.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A send an echo request to B.
3. Observe L1.

### Expected observations

1. On L1, from A, a Neighbor Solicitation with the global address of B as its target
   (RFC4861-AR-8, NS-1, NS-12). This confirms the stimulus.
2. Its IPv6 destination is the solicited-node multicast address of that target (RFC4861-AR-9,
   NS-2, NS-5).
3. It carries a Source Link-Layer Address option with the link-layer address of A
   (RFC4861-AR-12, NS-15).
4. On L1, from B, a Neighbor Advertisement to the source of that solicitation (RFC4861-AR-31,
   NA-1, NA-4), with no Neighbor Solicitation from B for that source before it (RFC4861-AR-24,
   AR-41).
5. Its Target Address is the target of the solicitation (RFC4861-AR-32, NA-21).
6. It carries a Target Link-Layer Address option with the link-layer address of B (RFC4861-AR-34,
   NA-24).
7. On L1, from A, after that advertisement, the echo request to B, with the link-layer address of
   B as its Ethernet destination (RFC4861-AR-14, AR-15, AR-18, AR-44).
8. The IPv6 source of the solicitation is the global address of A, the source of the echo request
   (RFC4861-AR-10).

### Notes

- Observation 7 shows that A kept the echo request while it resolved the address: it is the only
  echo request, so the queue of at least one packet (RFC4861-AR-15) is what brings it to B.
- Observation 8 is a `should`, and comes last.

## Neighbor Solicitation and Advertisement fields

Checks: **RFC4861-NS-4**, **NS-6**, **NS-7**, **NS-8**, **NA-3**, **NA-6**, **NA-7**, **NA-8**,
**NA-12** (description), **NS-10**, **NA-19**, **AR-35**, **AR-39** (must), **NS-13**, **NA-23**
(must not), **AR-37**, **NA-18** (should); covers **RFC4861-NS-3**, **NA-2** (encoding), **AR-2**
(description), **AR-53** (should, lower case).

### Requirement

RFC 4861 §4.3 and §4.4: a Neighbor Solicitation has hop limit 255, type 135, code 0, zero in its
Reserved field and a unicast target; a Neighbor Advertisement has hop limit 255, type 136, code
0, zero in its Reserved field and a unicast target, and leaves from an address of the interface.
§7.2.4: a host clears the Router flag, a solicited advertisement to a specified source sets the
Solicited flag, and an advertisement with a Target Link-Layer Address option for a plain unicast
address sets the Override flag.

### Scenario constants

- As in [Address resolution of a host](#address-resolution-of-a-host).

### Procedure

1. As in [Address resolution of a host](#address-resolution-of-a-host).

### Expected observations

1. On L1, from A, the Neighbor Solicitation with the global address of B as its target. This
   confirms the stimulus.
2. It has hop limit 255, ICMPv6 type 135, code 0, zero in its Reserved field, a target that is
   not a multicast address, and an address of A as its source (RFC4861-NS-4, NS-6, NS-7, NS-8,
   NS-10, NS-13).
3. On L1, from B, the Neighbor Advertisement that answers it, with hop limit 255, ICMPv6 type 136,
   code 0, zero in its Reserved field, a target that is not a multicast address, and an address of
   B as its source (RFC4861-NA-3, NA-6, NA-7, NA-8, NA-19, NA-23).
4. Its Solicited flag is set (RFC4861-AR-39, NA-12).
5. Its Router flag is clear (RFC4861-AR-35).
6. Its Override flag is set (RFC4861-AR-37, NA-18).

## Address resolution of a router

Checks: **RFC4861-NA-10** (description); covers **RFC4861-AR-35** (must, the half of a router).

### Requirement

RFC 4861 §4.4 and §7.2.4: a router sets the Router flag of its Neighbor Advertisements.

### Scenario constants

- The link. At 10 seconds, host A sends one ICMPv6 echo request to the global address of R on L1.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the link, and let R, A and B come up.
2. At 10 seconds, let A send an echo request to R.
3. Observe L1.

### Expected observations

1. On L1, from A, a Neighbor Solicitation with the global address of R as its target. This
   confirms the stimulus.
2. On L1, from R, a Neighbor Advertisement with that target and the Router flag set
   (RFC4861-NA-10, AR-35).

### Notes

- The Router Advertisement of R gives A the link-layer address of the link-local address of R,
  not of its global address, so A resolves the global address.

## Address resolution failure

Checks: **RFC4861-AR-21**, **HOST-7** (description), **AR-20**, **AR-22**, **AR-23**, **RCFG-23**
(must), **AR-19** (should), **AR-11** (should, lower case).

### Requirement

RFC 4861 §7.2.2: a node sends at most one Neighbor Solicitation for a neighbor every
RetransTimer, and about every RetransTimer while it waits; after MAX_MULTICAST_SOLICIT (3)
solicitations without an answer, and RetransTimer after the last one, address resolution fails,
and the node returns an ICMPv6 Destination Unreachable with code 3 (address unreachable) for each
queued packet. A router uses the host rules for RetransTimer (§6.2.1), and the default is
RETRANS_TIMER, 1000 milliseconds. §7.2.3: a node silently discards a solicitation for an address
that is not its own.

### Scenario constants

- The path, with the default of every variable. At 10 seconds, host C sends one ICMPv6 echo
  request to 2001:db8:1::99, an address on L1 that no node has.
- Observation lasts 20 seconds from the start.

### Procedure

1. Build the path, and let R, A and C come up.
2. At 10 seconds, let C send the echo request.
3. Observe L1 and L2.

### Expected observations

1. On L1, from R, a Neighbor Solicitation with target 2001:db8:1::99 to its solicited-node
   multicast address. This confirms the stimulus.
2. Its IPv6 source is an address of R on L1, not the source of the echo request
   (RFC4861-AR-11).
3. Exactly three such solicitations leave R in the window (RFC4861-AR-21).
4. The second comes 1 to 1.5 seconds after the first, and the third 1 to 1.5 seconds after the
   second (RFC4861-AR-19, AR-20, HOST-7, RCFG-23).
5. No Neighbor Advertisement with target 2001:db8:1::99 on L1 (RFC4861-AR-23).
6. On L2, from R, an ICMPv6 Destination Unreachable with code 3 to C, which holds the start of
   the echo request, at least 1 second after the third solicitation (RFC4861-AR-21, AR-22).

### Notes

- The upper bound of observation 4 reads "about every RetransTimer" as at most one and a half
  times the timer, as in
  [`parameters.md`](parameters.md#retransmission-timer-from-the-router).
