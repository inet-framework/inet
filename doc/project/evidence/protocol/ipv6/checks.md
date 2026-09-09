# IPv6 — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc8200/catalog.md](../../standard/rfc8200/catalog.md), [rfc4443/catalog.md](../../standard/rfc4443/catalog.md), [features.md](features.md)

Step 5 artifact of the standards test workflow (see
[derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)).
This file holds all English check procedures for IPv6, one section per check. The
procedures come from the specification only. They name no simulation model and no code.
The catalog entries are in [`rfc8200/catalog.md`](../../standard/rfc8200/catalog.md) and
[`rfc4443/catalog.md`](../../standard/rfc4443/catalog.md).

## Index

| Check | Statements |
| --- | --- |
| [Datagram delivery](#datagram-delivery) | RFC8200-HDR-1, HDR-2, HDR-3, HDR-4, DLV-1, HL-1, CKSUM-1 |
| [Hop limit expiry](#hop-limit-expiry) | RFC8200-HL-2, RFC4443-TE-1, RFC4443-ERR-2 |
| [Hop limit 1 at the destination](#hop-limit-1-at-the-destination) | RFC8200-HL-3; covers HL-1 |
| [Source fragmentation and reassembly](#source-fragmentation-and-reassembly) | RFC8200-FRAG-3, FRAG-4, FRAG-5 (the structure), FRAG-6, EXT-1, REASM-1, REASM-2, MTU-4; covers HDR-3 |
| [Fragment payload length](#fragment-payload-length) | RFC8200-FRAG-4, FRAG-5 (the payload length), HDR-2 |
| [Fragment identification](#fragment-identification) | RFC8200-FRAG-2 |
| [Packet too big](#packet-too-big) | RFC8200-FRAG-1, RFC4443-PTB-1, RFC4443-ERR-2 |
| [Packet too big, the MTU field](#packet-too-big-the-mtu-field) | RFC4443-PTB-2 |
| [Link MTU packet](#link-mtu-packet) | RFC8200-MTU-2 |

Nine checks.

## Common mockup

Three nodes in a row. Host A is the source. Node R is a router. Host B is the destination.

```
A ----------- R ----------- B
    link 1        link 2
```

- The links are Ethernet with an MTU of 1500 octets unless a check says otherwise. No
  check sets an MTU below 1280 octets on any link (RFC8200-MTU-1).
- Every node has one IPv6 address per link from one flat address plan, and the router
  knows the route to each host. Router discovery, address configuration and neighbor
  discovery run before the first datagram: their messages appear on the wire in every
  scenario, and no check judges them.
- A small application on host A sends one UDP datagram to host B. Timing: host A sends
  5 seconds after the start, when the addresses of every node are usable. All expected
  events occur within 1 second of that send. Observation stops after 10 seconds.
- Each check states its own scenario constants: data size, sender hop limit, and the MTU
  of a link. Unless a check says otherwise, the sender hop limit is 64.
- Every node computes the UDP checksum of the datagrams it sends. A simulation model may
  offer a mode that assumes checksums correct without computing them; the checks run with
  that mode off, because one observation reads the value.

### Rules every check obeys

- **One observation confirms the stimulus.** The datagram, with the field that drives the
  check, is observed where it leaves host A.
- **A discard is observed where the decision is made.** A silent discard leaves nothing to
  observe on the wire. The router of this mockup reports every discard it makes with an
  ICMPv6 message, so the origination of that message at the router is the record of the
  decision, and the absence watches open at that moment. An absence watch that opens after
  the forbidden event would have happened proves nothing; see the note under
  [hop limit expiry](#hop-limit-expiry).

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

## Hop limit expiry

Checks: **RFC8200-HL-2** (description), **RFC4443-TE-1** (must), **RFC4443-ERR-2**
(description).

### Requirement

RFC 8200 §3: when forwarding, a node discards a packet whose hop limit is decremented to
zero. RFC 4443 §3.3: a router that decrements a hop limit to zero must discard the packet
and originate a Time Exceeded message, type 3 code 0, to the source of the packet.

### Scenario constants

- Application data: 100 octets, to port 5000.
- The sender hop limit on host A is 1: the packet survives exactly one hop. The router
  decrements it to zero and must discard it.

### Procedure

1. Build the mockup network.
2. Set the sender hop limit on host A to 1.
3. Let host A send the datagram.
4. Observe link 1, the router's own origination of messages, link 2, and the messages that
   return to host A.

### Expected observations

1. On link 1, the packet appears with hop limit 1 and destination port 5000. This confirms
   the stimulus.
2. The router originates a Time Exceeded message, type 3 code 0: the message is recorded
   at the router at the moment of the discard (RFC8200-HL-2, RFC4443-TE-1). This is the
   observation that is in time — see the notes.
3. Host A receives that Time Exceeded message, type 3 code 0, addressed to host A
   (RFC4443-ERR-2).
4. From then on, the packet never appears on link 2, and host B never receives it
   (RFC8200-HL-2, the wire form of observation 2).

The check passes if observations 1 to 3 occur in this order and observation 4 records no
traffic of the packet on link 2 within the time limit.

### Notes

- Observation 2 is recorded at the router rather than on the wire. A watch on link 2 that
  starts only after the report arrives at host A starts too late: a forbidden forward would
  leave the router at the moment of the decision, before the report crosses link 1. The
  router's own origination of the report is available at that moment; the wire absence of
  observation 4 then guards the rest of the window.
- Unlike the IPv4 check, the report is a must, and its absence is a violation of RFC 4443.

## Hop limit 1 at the destination

Checks: **RFC8200-HL-3** (should). Also covers: **RFC8200-HL-1** (description).

### Requirement

RFC 8200 §3: the discard for a hop limit of zero is a forwarding rule; a node that is the
destination of a packet should not discard it for its hop limit and should process it
normally.

### Scenario constants

- Application data: 100 octets, to port 5000.
- The sender hop limit on host A is 2: exactly the hop count of the path. The router
  decrements it to 1, and host B receives the packet with hop limit 1.

### Procedure

1. Build the mockup network.
2. Set the sender hop limit on host A to 2.
3. Let host A send the datagram.
4. Observe link 1, link 2, host B's UDP, and the messages that return to host A.

### Expected observations

1. On link 1, the packet appears with hop limit 2. This confirms the stimulus.
2. On link 2, the packet appears with hop limit 1 (RFC8200-HL-1).
3. Host B's UDP receives the datagram, 108 octets (RFC8200-HL-3): the destination
   delivered a packet that arrived with the smallest hop limit a forwarded packet can have.
4. Host A receives no Time Exceeded message: host B did not treat the packet as expired.

The check passes if observations 1 to 3 occur in this order and observation 4 records no
message within the time limit.

### Notes

- A packet that arrives with hop limit 0 is the exact case of the should. It needs a
  sender that emits hop limit 0, which no ordinary sender does; that variant is a crafted
  input for a later pass. Hop limit 1 is the boundary this pass can reach.

## Source fragmentation and reassembly

Checks: **RFC8200-FRAG-3** (must), **RFC8200-FRAG-4**, **RFC8200-FRAG-5** (description; the
structure of the fragments), **RFC8200-FRAG-6** (must not), **RFC8200-EXT-1**,
**RFC8200-REASM-1**, **RFC8200-REASM-2** (description), **RFC8200-MTU-4** (must). Also
covers: **RFC8200-HDR-3**. The payload length of the fragments is a check of its own,
[fragment payload length](#fragment-payload-length), so that one wrong field does not hide
the reassembly result.

### Requirement

RFC 8200 §4.5: a source that must send a packet larger than the path MTU may divide the
fragmentable part into fragments, each but the last a multiple of 8 octets, none
overlapping. Every fragment packet carries the per-fragment headers with next header 44 and
the payload length of the fragment, then a fragment header with the original next header,
the offset in 8-octet units, the M flag and the identification, and the first fragment also
carries the upper-layer header. §4: a node en route does not process, insert or delete the
fragment header. The destination reassembles the fragments that share source, destination
and identification into the original packet, without the fragment header and with the
payload length recomputed. §5: a node must accept a fragmented packet that reassembles to
1500 octets.

### Scenario constants

- Link 1 has an MTU of 1280 octets, the minimum, on host A's side: host A must fragment.
  Link 2 keeps 1500 octets.
- Application data: 1452 octets, to port 5000. The original packet is 40 + 8 + 1452 = 1500
  octets long: payload length 1460, and a fragmentable part of 1460 octets (the UDP header
  and the data; there are no extension headers, so the per-fragment headers are the IPv6
  header alone).

### Size arithmetic (from the RFC procedure)

- Each fragment packet is the 40-octet header, the 8-octet fragment header, and a piece.
  To fit 1280 octets the piece is at most 1232 octets, and 1232 is a multiple of 8.
- Fragment 1: piece 1232 octets, offset 0 (0 units), M = 1, payload length 8 + 1232 = 1240.
- Fragment 2: piece 1460 − 1232 = 228 octets, offset 1232 octets (154 units), M = 0,
  payload length 8 + 228 = 236.
- The two pieces tile the fragmentable part: 1232 + 228 = 1460, no gap, no overlap. The
  split gives exactly two fragments. A source may choose a smaller first piece; the
  arithmetic below then changes, and the relations between the fragments stay.

### Procedure

1. Build the mockup with MTU 1280 on host A's interface to link 1.
2. Let host A send the datagram.
3. Observe every fragment on link 1 and on link 2, and what host B delivers to UDP.

### Expected observations

1. On link 1, the first fragment appears: next header 44, a fragment header with next
   header 17, offset 0 and M = 1, and the UDP header with destination port 5000 behind it.
   Record its identification (RFC8200-FRAG-4, RFC8200-HDR-3). This confirms the stimulus:
   the source fragmented.
2. On link 1, the second fragment appears: next header 44, the same identification, offset
   1232 octets (154 units), M = 0 (RFC8200-FRAG-5). The pieces are 1232 and 228 octets: the
   first is a multiple of 8, both packets fit 1280 octets, and the two tile 1460 octets
   without overlap (RFC8200-FRAG-3, RFC8200-FRAG-6).
3. On link 2, the first fragment appears with the same identification, offset 0 and M = 1:
   the router forwarded it and did not touch the fragment header (RFC8200-EXT-1).
4. On link 2, the second fragment appears with the same identification, offset 1232 and
   M = 0.
5. Host B's UDP receives one complete datagram of 1460 octets (8-octet header plus 1452
   octets of data) on port 5000: the destination reassembled the two fragments into the
   1500-octet packet and removed the fragment header (RFC8200-REASM-1, RFC8200-REASM-2,
   RFC8200-MTU-4).

The check passes if all five observations occur in this order within the time limit.

### Notes

- The offsets in expected observations 2 and 4 are exact. They follow from the arithmetic
  above under the natural choice of the largest first piece; a source that chooses a
  smaller first piece needs the relation "offset of fragment 2 equals the piece of fragment
  1" instead of the constant, and the test says which it asserts.
- The two links form a single path, so the fragments arrive in order. Reorder tolerance is
  out of scope here.

## Fragment payload length

Checks: the payload-length halves of **RFC8200-FRAG-4** and **RFC8200-FRAG-5**
(description), and **RFC8200-HDR-2** (description) for a fragment packet.

### Requirement

RFC 8200 §4.5: every fragment packet carries the per-fragment headers "with the Payload
Length of the original IPv6 header changed to contain the length of this fragment packet
only (excluding the length of the IPv6 header itself)". §3: the payload length counts the
rest of the packet after the IPv6 header, extension headers included.

### Scenario constants

- The same as [source fragmentation and reassembly](#source-fragmentation-and-reassembly):
  MTU 1280 on host A's interface to link 1, a 1500-octet packet to port 5000. By the
  arithmetic there, the first fragment packet has payload length 8 + 1232 = 1240 and the
  last one 8 + 228 = 236.

### Procedure

1. Build the mockup with MTU 1280 on host A's interface to link 1.
2. Let host A send the datagram.
3. Observe both fragments on link 1 and read their payload length fields.

### Expected observations

1. On link 1, the first fragment appears (offset 0, M = 1, the UDP header to port 5000
   behind the fragment header). Record its identification and its payload length. This
   confirms the stimulus.
2. On link 1, the last fragment appears with the recorded identification and M = 0, and its
   payload length is 236; the recorded payload length of the first fragment is 1240
   (RFC8200-FRAG-4, RFC8200-FRAG-5, RFC8200-HDR-2).

The check passes if both observations occur in this order within the time limit.

### Notes

- A source that copies the original payload length into every fragment fails this check
  while the reassembly check may still pass, if the destination measures the fragments
  instead of reading the field. On a real path the wrong field misleads every node that
  reads it, which is why the two checks are kept apart.

## Fragment identification

Checks: **RFC8200-FRAG-2** (must).

### Requirement

RFC 8200 §4.5: the identification of a fragmented packet must differ from that of any other
fragmented packet sent recently with the same source and destination.

### Scenario constants

- Link 1 has an MTU of 1280 octets on host A's side. Host A sends two datagrams of 1452
  octets at the same time, to port 5000 and to port 5001 of host B: the same source and
  destination, both fragmented.

### Procedure

1. Build the mockup with MTU 1280 on host A's interface to link 1.
2. Let host A send the two datagrams.
3. Observe the first fragments of both on link 1 and record their identifications.

### Expected observations

1. On link 1, the first fragment of datagram 1 appears (offset 0, M = 1, UDP port 5000).
   Record its identification as ID1. This confirms the stimulus.
2. On link 1, the first fragment of datagram 2 appears (offset 0, M = 1, UDP port 5001)
   with an identification that differs from ID1 (RFC8200-FRAG-2).

### Notes

- Two datagrams show that the source does not reuse a value at once. Uniqueness over the
  whole lifetime of a packet is a count over many datagrams; that is level 4 work.

## Packet too big

Checks: **RFC8200-FRAG-1** (description), **RFC4443-PTB-1** (must), **RFC4443-ERR-2**
(description).

### Requirement

RFC 8200 §4.5 and §5: fragmentation is performed by source nodes only; a router does not
fragment a packet that does not fit the next link. RFC 4443 §3.2: a router must send Packet
Too Big, type 2, to the source of a packet that it cannot forward because the packet is
larger than the MTU of the outgoing link.

### Scenario constants

- Link 2 has an MTU of 1280 octets. Link 1 keeps 1500 octets.
- Application data: 1452 octets, to port 5000: a 1500-octet packet. It crosses link 1
  whole; the router cannot forward it over link 2.

### Procedure

1. Build the mockup with MTU 1280 on link 2.
2. Let host A send the datagram.
3. Observe link 1, the router's own origination of messages, link 2, and the messages that
   return to host A.

### Expected observations

1. On link 1, the packet appears whole: next header 17, payload length 1460. This confirms
   the stimulus.
2. The router originates a Packet Too Big message, type 2: recorded at the router at the
   moment of the discard (RFC4443-PTB-1).
3. Host A receives that message, type 2, addressed to host A (RFC4443-ERR-2).
4. From then on, no part of the packet appears on link 2: neither a fragment (next header
   44) nor the packet itself (next header 17), and host B receives nothing of it
   (RFC8200-FRAG-1).

The check passes if observations 1 to 3 occur in this order and observation 4 records no
traffic of the packet on link 2 within the time limit.

### Notes

- The content of the message, the MTU field, is a check of its own
  ([packet too big, the MTU field](#packet-too-big-the-mtu-field)), so that a wrong field
  value does not hide the result of the two rules here.
- The 1280-octet MTU on link 2 is the smallest value RFC 8200 allows on an IPv6 link
  (RFC8200-MTU-1).

## Packet too big, the MTU field

Checks: **RFC4443-PTB-2** (description; the field definition of a mandatory message).

### Requirement

RFC 4443 §3.2: the MTU field of a Packet Too Big message is the maximum transmission unit
of the next-hop link, and the code is 0. Path MTU discovery (RFC 8201) reads this field.

### Scenario constants

- The same as [packet too big](#packet-too-big): MTU 1280 on link 2, a 1500-octet packet.

### Procedure

1. Build the mockup with MTU 1280 on link 2.
2. Let host A send the datagram.
3. Observe the Packet Too Big message that returns to host A.

### Expected observations

1. On link 1, the packet appears whole with payload length 1460. This confirms the
   stimulus.
2. Host A receives a Packet Too Big message, type 2, code 0, whose MTU field is 1280
   (RFC4443-PTB-2).

The check passes if both observations occur in this order within the time limit.

### Notes

- A message with an MTU field of zero, or of any value other than the next-hop MTU, fails
  this check while [packet too big](#packet-too-big) passes: the router reported, but the
  report carries nothing a source could act on.

## Link MTU packet

Checks: **RFC8200-MTU-2** (must).

### Requirement

RFC 8200 §5: from each link a node is attached to, the node must be able to accept packets
as large as that link's MTU.

### Scenario constants

- Both links keep the Ethernet MTU of 1500 octets.
- Application data: 1452 octets, to port 5000: a packet of exactly 1500 octets, the link
  MTU. Nothing fragments it.

### Procedure

1. Build the mockup network.
2. Let host A send the datagram.
3. Observe link 1, link 2 and host B's UDP.

### Expected observations

1. On link 1, the packet appears whole: next header 17, payload length 1460. This confirms
   the stimulus.
2. On link 2, the packet appears whole with the same payload length: the router accepted
   and forwarded a packet of the link MTU.
3. Host B's UDP receives the datagram of 1460 octets (RFC8200-MTU-2): host B accepted a
   packet of the link MTU.

The check passes if all three observations occur in this order within the time limit.

### Notes

- Observation 2 is the router's half of the same rule; RFC 8200 states it for every node.
