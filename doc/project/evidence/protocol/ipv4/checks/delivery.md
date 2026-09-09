# IPv4 checks — delivery

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV4-F-DELIVERY. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Datagram delivery

Checks: **RFC791-FWD-1**, **RFC791-DLV-1**, **RFC791-PROTO-1**, **RFC791-HDR-1**,
**RFC791-HDR-2**, **RFC791-HDR-3** (all description).

### Requirement

RFC 791 §2.2 and §2.4: a datagram addressed to a host in another network is forwarded by
the gateway, and the destination's internet module passes the data to the addressed
program together with the source address. §3.1: the header carries version 4, a header
length of at least 5 words, a total length that counts header and data, and the number of
the next level protocol.

### Scenario constants

- Application data: 100 octets, carried by UDP (8-octet header). With a 20-octet header and
  no options the datagram is 20 + 8 + 100 = 128 octets long.
- Both links carry it without fragmentation. The sender TTL is the sender's default.

### Procedure

1. Build the mockup network.
2. Let host A send one datagram to host B.
3. Observe the datagram on link 1, on link 2, at host B's UDP, and at host B's program.

### Expected observations

1. On link 1, the datagram appears with version 4, a header length of 20 octets, a total
   length of 128 octets, the protocol number of UDP (17), host A as source address and
   host B as destination address. This confirms the stimulus and covers RFC791-HDR-1,
   HDR-2, HDR-3 and the wire half of PROTO-1.
2. On link 2, the same datagram appears with the same source and destination addresses
   (RFC791-FWD-1): the gateway forwarded it and did not rewrite the addresses.
3. Host B's UDP receives the datagram with the 100 octets of data: the internet module
   handed the data to the protocol the header named (RFC791-PROTO-1).
4. The program on host B receives the 100 octets (RFC791-DLV-1). The source address is
   confirmed on the wire in observations 1 and 2; at the program interface it is a
   parameter of the delivery.

### Notes

- Host B's UDP receiving the datagram is the observable form of "passes the data to the
  application program": the UDP header and the data are what the internet module passed
  upward.
- The total length of 128 octets holds only because the datagram carries no options. The
  options are out of scope in this pass.

