# IPv6 checks — delivery

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV6-F-DELIVERY and IPV6-F-HEADER. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Datagram delivery

Checks: **RFC8200-HDR-1**, **RFC8200-HDR-2**, **RFC8200-HDR-3**, **RFC8200-HDR-4**,
**RFC8200-DLV-1**, **RFC8200-HL-1** (all description), **RFC8200-CKSUM-1** (must).

### Requirement

RFC 8200 §3: the header carries version 6, a payload length that counts everything after
the 40-octet header, a next header that names the header that follows, a hop limit that
every forwarding node decrements by 1, and the addresses of the originator and the intended
recipient. §4: the destination demultiplexes on the next header field to the upper-layer
protocol. §8.1: a UDP datagram over IPv6 carries a checksum that is never zero.

### Scenario constants

- Application data: 100 octets, carried by UDP (8-octet header) from port 4000 on host A
  to port 5000 on host B. With the 40-octet header and no extension headers the packet is
  148 octets long and its payload length is 108.
- The sender hop limit is 64.

### Procedure

1. Build the mockup network.
2. Let host A send one datagram to host B.
3. Observe the packet on link 1, on link 2, at host B's UDP, and at host B's program.

### Expected observations

1. On link 1, the packet appears with version 6, payload length 108, next header 17, hop
   limit 64, host A's address as source, host B's address as destination, and a UDP
   checksum that is not zero. This confirms the stimulus and covers RFC8200-HDR-1, HDR-2,
   HDR-3, HDR-4 (the originator and the recipient) and CKSUM-1. Record the two addresses.
2. On link 2, the same packet appears with the same source and destination addresses
   (RFC8200-HDR-4: the router did not rewrite them), the same payload length, and hop
   limit 63 (RFC8200-HL-1).
3. Host B's UDP receives the datagram with the 100 octets of data: the IPv6 layer handed
   the payload to the protocol that the next header named (RFC8200-DLV-1).
4. The program on host B receives the 100 octets.

The check passes if all four observations occur in this order within the time limit.

### Notes

- The hop limit on link 2 is compared against the constant 63 and against the recorded
  value of link 1 minus 1; the second comparison stays right if the sender default
  changes.
- The payload length of 108 holds only because the packet carries no extension headers.
  The extension headers are out of scope in this pass, except the fragment header.

