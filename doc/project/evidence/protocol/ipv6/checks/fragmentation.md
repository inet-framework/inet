# IPv6 checks — fragmentation and reassembly

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV6-F-SOURCE-FRAGMENTATION and IPV6-F-REASSEMBLY. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

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


## Atomic fragment

Checks: **RFC8504-NR-4**, the receiver half (should), which governs **RFC8200-REASM-6**.

### Requirement

RFC 8200 §4.5 and RFC 8504 §5.1: a fragment whose offset is 0 and whose M flag is 0 is a
whole datagram; it should be processed as a fully reassembled packet, and any other
fragments that match it should be processed independently.

### Scenario constants

- Mockup with a relay. Link 1 has MTU 1280 on host A's side; application data 1452 octets
  to port 5000, so host A sends two fragments (see
  [source fragmentation and reassembly](#source-fragmentation-and-reassembly)).
- The relay clears the M flag of the first fragment. Host B receives a fragment with offset
  0 and M = 0 that carries the UDP header and the first 1224 octets of data, followed by
  the unchanged second fragment.

### Procedure

1. Build the mockup with the relay and MTU 1280 on host A's interface.
2. Tell the relay to clear the M flag of the first fragment.
3. Let host A send the datagram.
4. Observe host B's interface and the handoff from host B's internet layer to UDP.

### Expected observations

1. At host B's interface, a fragment arrives with offset 0 and M = 0: an atomic fragment.
   Record its identification. This confirms the stimulus.
2. Host B's internet layer hands the content of that fragment to UDP as a whole packet: the
   UDP header with destination port 5000 reaches UDP, without a fragment header
   (RFC8504-NR-4, RFC8200-REASM-6).

The check passes if both observations occur in this order within the time limit.

### Notes

- What UDP does with the datagram afterwards is not part of this check: its length field
  names 1460 octets and only 1232 arrived, so UDP may discard it by its own rules.
- The second fragment, which arrives later with the same identification, is to be
  processed independently; it never completes and is not observed.

## Overlapping fragments

Checks: **RFC8504-NR-3**, the receiver half (must), which governs **RFC8200-REASM-5**.

### Requirement

RFC 8504 §5.1: when reassembling a datagram, if one or more of its fragments is found to
overlap another, the entire datagram and all its fragments must be silently discarded.
RFC 8200 §4.5 says the same without "silently".

### Scenario constants

- Mockup with a relay, MTU 1280 on host A's side, 1452 octets of data to port 5000: two
  fragments with pieces at octets 0 to 1231 and 1232 to 1459.
- The relay rewrites the offset of the second fragment from 1232 to 1224 octets (153
  units). Its piece now covers octets 1224 to 1451 and overlaps the first piece by 8
  octets.

### Procedure

1. Build the mockup with the relay and MTU 1280 on host A's interface.
2. Tell the relay to rewrite the offset of the second fragment to 1224.
3. Let host A send the datagram.
4. Observe host B's interface, the handoff from host B's internet layer to UDP, and every
   ICMPv6 message host B sends.

### Expected observations

1. At host B's interface, the first fragment arrives: offset 0, M = 1. Record its
   identification.
2. At host B's interface, a fragment with the same identification, offset 1224 and M = 0
   arrives: it overlaps the first by 8 octets. This confirms the stimulus.
3. From then on, nothing of the datagram reaches UDP at host B, and host B sends no ICMPv6
   message about it (RFC8504-NR-3, RFC8200-REASM-5).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- A receiver that merges the overlapping pieces and delivers a datagram of 1452 octets
  fails this check; that is the behavior RFC 5722 was written against.

## Short fragment

Checks: **RFC8200-REASM-4** (must discard; should report).

### Requirement

RFC 8200 §4.5: if the length of a fragment, as derived from its payload length, is not a
multiple of 8 octets and its M flag is 1, the fragment must be discarded, and a Parameter
Problem code 0 message should be sent to the source, pointing to the payload length field.

### Scenario constants

- Mockup with a relay, MTU 1280 on host A's side, 1452 octets of data to port 5000.
- The relay shortens the first fragment by 4 octets: its piece becomes 1228 octets, which
  is not a multiple of 8, and its payload length becomes 8 + 1228 = 1236. The M flag stays
  1.

### Procedure

1. Build the mockup with the relay and MTU 1280 on host A's interface.
2. Tell the relay to shorten the first fragment by 4 octets.
3. Let host A send the datagram.
4. Observe host B's interface, the messages that return to host A, and host B's UDP.

### Expected observations

1. At host B's interface, a first fragment (offset 0, M = 1) arrives with payload length
   1236. Record its identification. This confirms the stimulus.
2. Host A receives a Parameter Problem message, type 4 code 0 (the should half of
   RFC8200-REASM-4).
3. From then on, nothing of the datagram reaches UDP at host B (the must half).

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.

### Notes

- The second fragment arrives after the first was discarded and never completes a
  datagram on its own; the absence watch covers it.
- The pointer field of the report is not asserted; it is a sharpening candidate.

## Oversized fragment offset

Checks: **RFC8200-REASM-8** (must discard; should report).

### Requirement

RFC 8200 §4.5: if the length and offset of a fragment are such that the payload length of
the reassembled packet would exceed 65,535 octets, the fragment must be discarded, and a
Parameter Problem code 0 message should be sent to the source, pointing to the fragment
offset field.

### Scenario constants

- Mockup with a relay, MTU 1280 on host A's side, 1452 octets of data to port 5000.
- The relay rewrites the offset of the second fragment to 65,528 octets (8191 units, the
  largest the field carries). With its 228-octet piece the reassembled payload would reach
  65,756 octets.

### Procedure

1. Build the mockup with the relay and MTU 1280 on host A's interface.
2. Tell the relay to rewrite the offset of the second fragment to 65,528.
3. Let host A send the datagram.
4. Observe host B's interface, the messages that return to host A, and host B's UDP.

### Expected observations

1. At host B's interface, a fragment with offset 65,528 and M = 0 arrives. This confirms
   the stimulus.
2. Host A receives a Parameter Problem message, type 4 code 0.
3. From then on, nothing of the datagram reaches UDP at host B.

The check passes if observations 1 and 2 occur in this order and observation 3 records
nothing within the time limit.
