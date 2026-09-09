# IPv4 checks — fragmentation and reassembly

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [../checks.md](../checks.md), [features.md](../features.md)

The English check procedures of the features IPV4-F-FRAGMENTATION, IPV4-F-REASSEMBLY, IPV4-F-DONT-FRAGMENT, IPV4-F-MIN-SIZE. One section per check; the section
names are the anchors that the coverage ledger links to. The common mockup, the
observation rules, and the index of all checks are in [`../checks.md`](../checks.md); the
statements are in the catalogs under [`../../../standard/`](../../../standard). The
procedures come from the specification only. They name no simulation model and no code.

## Fragment and reassembly

Checks: **RFC791-FRAG-1** (description), **RFC791-FRAG-2** (must), **RFC791-FRAG-3**
(description), **RFC791-FRAG-4** (description), **RFC791-REASM-1** (description).

### Requirement

RFC 791 §2.3 and §3.2: a complete datagram carries MF = 0 and fragment offset 0. A gateway
that must forward a datagram over a link with a smaller MTU divides the data on 8-octet
boundaries. Each fragment keeps the identification of the original. Non-final fragments
carry MF = 1; the last fragment carries MF = 0. The destination combines the fragments by
identification, source, destination, and protocol, and it places the data by fragment
offset.

### Scenario constants

- Application data: 1000 octets. Link 1 MTU: 1500 octets. Link 2 MTU: 576 octets.

### Size arithmetic (from the RFC procedure)

- Internet header: 20 octets (no options). UDP header: 8 octets.
- Datagram data: 8 + 1000 = 1008 octets. Total length: 1028 octets.
- 1028 ≤ 1500, so the datagram crosses link 1 in one piece.
- 1028 > 576, so the gateway must fragment for link 2.
- Largest data portion per fragment: 576 − 20 = 556 octets; the largest multiple of 8 is
  **552** octets. NFB = 69 blocks.
- Fragment 1: 552 data octets, total length 572, MF = 1, offset 0.
- Fragment 2: 1008 − 552 = 456 data octets, total length 476, MF = 0, offset 69 blocks,
  that is, octet position 552.
- The split gives exactly two fragments.

### Procedure

1. Build the mockup network with MTU 1500 on link 1 and MTU 576 on link 2.
2. Start the network and let host A send the datagram.
3. Observe the datagram on link 1 and record its fragmentation fields.
4. Observe every part of the datagram on link 2. Record identification, offset, MF, and
   length of each part.
5. Observe what host B delivers to the protocol above IP.

### Expected observations

1. On link 1, the complete datagram appears with MF = 0 and fragment offset 0
   (RFC791-FRAG-1).
2. On link 2, a first fragment appears with offset 0 and MF = 1. Record its identification
   value (RFC791-FRAG-4).
3. On link 2, a second fragment appears with the same identification, offset 552 octets
   (69 blocks), and MF = 0 (RFC791-FRAG-2, RFC791-FRAG-3, RFC791-FRAG-4).
4. After that, no further part with that identification appears on link 2. The split gave
   exactly two fragments (RFC791-FRAG-2 arithmetic).
5. Host B delivers one complete UDP datagram of 1008 octets (8-octet header plus 1000
   octets of data) to the layer above IP (RFC791-REASM-1).

The check passes if observations 1-3 and 5 occur, and observation 4 records no extra part.

### Notes

- The offset in expected observation 3 is exact. It follows from the arithmetic above, not
  from a measurement.
- The two links form a single path, so the fragments arrive in order. Reorder tolerance is
  out of scope here.
- A deeper variant also compares the reassembled payload octet by octet. Keep that for a
  later iteration.


## Don't fragment

Checks: **RFC791-FRAG-5** (must), **RFC792-DU-4** (must + may).

### Requirement

RFC 791 §2.3: a datagram marked "don't fragment" is not to be fragmented under any
circumstances. If the datagram cannot reach its destination without fragmentation, the
gateway discards it. RFC 792: in this case the gateway must discard the datagram, and it may
return a destination unreachable message with code 4, "fragmentation needed and DF set".

### Scenario constants

- Application data: 1000 octets; the datagram total length is 1028 octets (see the size
  arithmetic above). Link 1 MTU: 1500 octets. Link 2 MTU: 576 octets.
- The datagram carries DF = 1. Because 1028 > 576, the gateway cannot forward it over
  link 2 without fragmentation.

### Procedure

1. Build the mockup network with MTU 1500 on link 1 and MTU 576 on link 2.
2. Set the don't fragment flag for the datagrams of the application on host A.
3. Start the network and let host A send the datagram.
4. Observe the datagram on link 1 and make sure DF = 1.
5. Observe link 2 for the full observation time.
6. Observe link 1 in the direction of host A for error messages.
7. Observe what host B delivers upward.

### Expected observations

1. On link 1, the datagram appears with DF = 1 and a total length above 576 octets. This
   confirms the stimulus.
2. The gateway discards the datagram: a discard of this datagram is recorded at the gateway
   (RFC791-FRAG-5). This observation is in time; a watch on link 2 that starts after the
   report arrives would start too late to see a forbidden fragment.
3. The gateway returns an ICMP destination unreachable message with code 4 ("fragmentation
   needed and DF set") to host A (RFC792-DU-4).
4. From then on, no part of the datagram appears on link 2 — no fragment, and not the whole
   datagram (RFC791-FRAG-5, the wire form of observation 2). Host B receives nothing of it.

The check passes if observations 1 to 3 occur, and observation 4 records no traffic of the
datagram on link 2.

### Notes

- The discard (observation 3) is a *must*. The error message (observation 2) is a *may*: a
  silent gateway also conforms to RFC 791 and RFC 792. This check asserts the cooperative
  behavior. If the tested system stays silent, record the result as an implementation gap
  against the "may" clause, not as a specification violation, and keep the discard result
  separate.
- The absence check on link 2 must cover the time around the expected error message, because
  a wrong implementation could fragment first and report later.


## Minimum sizes

Checks: **RFC791-FRAG-6** (must), **RFC791-REASM-2** (must).

### Requirement

RFC 791 §3.2: every internet module must be able to forward a datagram of 68 octets without
further fragmentation. §3.1: all hosts must be prepared to accept datagrams of up to 576
octets, whether they arrive whole or in fragments.

### Scenario constants

- The MTU of link 2 is 68 octets, the smallest the RFC lets a module demand.
- Datagram 1: 40 octets of application data, so 20 + 8 + 40 = 68 octets. It must cross
  link 2 whole.
- Datagram 2: 548 octets of application data, so 20 + 8 + 548 = 576 octets. It cannot cross
  link 2 whole; the gateway fragments it, and host B must reassemble and accept it.

### Size arithmetic

With a 20-octet header and a 68-octet MTU, each fragment carries 48 data octets, the
largest multiple of 8 that fits. Datagram 2 has 556 octets of internet data (8 of UDP and
548 of application data), so it becomes 11 fragments of 48 octets and one of 28: twelve
fragments in all.

### Procedure

1. Build the mockup network with the MTU of link 2 at 68 octets.
2. Let host A send datagram 1, then datagram 2.
3. Observe link 1, link 2 and host B.

### Expected observations

1. On link 1, datagram 1 appears with total length 68. This confirms the stimulus.
2. On link 2, datagram 1 appears whole: total length 68, more-fragments flag 0, fragment
   offset 0 (RFC791-FRAG-6).
3. On link 1, datagram 2 appears with total length 576.
4. On link 2, a fragment of datagram 2 appears with the more-fragments flag set: the
   gateway did fragment it.
5. Host B's UDP receives datagram 2 whole, 556 octets of UDP datagram, that is 548 octets
   of data (RFC791-REASM-2).

### Notes

- The 68-octet floor is a 60-octet maximum header plus an 8-octet minimum fragment. With a
  20-octet header the fragments carry 48 octets; the floor stays the right test value,
  because it is the one every module must accept.
- Observation 5 is the whole of RFC791-REASM-2: acceptance of 576 octets that arrive in
  fragments. Acceptance of 576 octets that arrive whole is a sharpening candidate.


## Interleaved reassembly

Checks: **RFC791-REASM-3** (description), **RFC1122-REASM-1** (must). Also covers:
**RFC791-REASM-1** (description).

### Requirement

RFC 791 §2.3: the receiver uses the identification field to make sure that fragments of
different datagrams are not mixed, and it combines the fragments that agree in
identification, source, destination and protocol by their offsets. RFC 1122 §3.3.2: the IP
layer must implement reassembly. Nothing in either text assumes that the fragments of one
datagram arrive together or in order.

### Scenario constants

- Mockup with a relay. Link 2 MTU: 576 octets.
- Host A sends two datagrams of 1000 octets of application data at the same time, to two
  ports of host B: datagram 1 to port 5000, datagram 2 to port 5001. Each one splits at the
  gateway into a 572-octet fragment (offset 0, MF = 1) and a 476-octet fragment (offset 552,
  MF = 0), by the arithmetic of [fragment and reassembly](#fragment-and-reassembly).
- The relay holds the first fragment of datagram 1 for 2 ms and forwards everything else at
  once. Host B therefore receives the fragments in the order: datagram 1 offset 552,
  datagram 2 offset 0, datagram 2 offset 552, and only then datagram 1 offset 0.

### Procedure

1. Build the mockup with the relay and MTU 576 on link 2.
2. Tell the relay to hold the first fragment with offset 0 for 2 ms.
3. Let host A send the two datagrams.
4. Observe link 2 before the relay, host B's interface, and host B's UDP.

### Expected observations

1. On link 2, the first fragment of datagram 1 appears: offset 0, MF = 1. Record its
   identification as ID1.
2. On link 2, the last fragment of datagram 1 appears: identification ID1, offset 552,
   MF = 0.
3. At host B's interface, the first frame to arrive is the last fragment of datagram 1:
   identification ID1, offset 552. The held first fragment is not in front of it.
4. At host B's interface, a first fragment arrives: offset 0, MF = 1, with an
   identification ID2 that differs from ID1. This is datagram 2.
5. At host B's interface, the last fragment of datagram 2 arrives: identification ID2,
   offset 552. Observations 3 to 5 confirm that the relay reordered the trains.
6. Host B's UDP receives datagram 2 whole: 1008 octets, port 5001 (RFC1122-REASM-1). It
   arrives before the held fragment, so the two datagrams did not mix (RFC791-REASM-3).
7. At host B's interface, the held fragment arrives: ID1, offset 0, MF = 1.
8. Host B's UDP receives datagram 1 whole: 1008 octets, port 5000 (RFC791-REASM-1).

The check passes if all eight observations occur in this order within the time limit.

### Notes

- The 2 ms hold is far below any reassembly timeout, so the timer of RFC 1122 §3.3.2 plays
  no part here; the timer is level 4 work.
- Observation 6 before observation 7 is what makes the check decisive: a receiver that
  keeps one buffer per identification only would have to wait, or would mix.

## Same identification, different protocol

Checks: **RFC791-REASM-1** (description; the four-field key), **RFC791-REASM-3**
(description).

### Requirement

RFC 791 §2.3: a module combines the fragments that have the same value for the four fields
identification, source, destination, and protocol. Two datagrams that agree in three of the
four fields and differ in the protocol are two datagrams, and their fragments must not mix.
RFC 6864 §4.3 keeps the identification unique per source, destination and protocol tuple,
so such a collision is legal on the wire.

### Scenario constants

- Mockup with a relay. Link 2 MTU: 576 octets.
- Datagram 1: UDP, 1000 octets of application data, to port 5000: total length 1028; it
  splits into 572 + 476 octets, offsets 0 and 552.
- Datagram 2: an ICMP echo request with 900 octets of data, sent right after datagram 1:
  total length 928; it splits into 572 + 376 octets, offsets 0 and 552.
- The relay rewrites the identification of both fragments of datagram 2 to the
  identification of datagram 1 and keeps the checksum valid. It also holds the last
  fragment of datagram 1 for 1 ms, so that host B receives: datagram 1 offset 0, datagram 2
  offset 0, datagram 2 offset 552, datagram 1 offset 552. At host B all four fragments
  agree in identification, source and destination; two carry protocol 17 and two carry
  protocol 1.

### Procedure

1. Build the mockup with the relay and MTU 576 on link 2.
2. Tell the relay to rewrite the identification of the ICMP fragments and to hold the last
   UDP fragment for 1 ms.
3. Let host A send the UDP datagram and then the echo request.
4. Observe link 2 before the relay, host B's interface, host B's UDP, and the messages that
   leave host B.

### Expected observations

1. On link 2, the first fragment of datagram 1 appears: protocol 17, offset 0. Record its
   identification as ID1.
2. On link 2, the first fragment of datagram 2 appears: protocol 1, offset 0, with an
   identification that differs from ID1. This confirms that two datagrams are in flight.
3. At host B's interface, a fragment with protocol 1, offset 0 and identification ID1
   arrives. This confirms that the rewrite took effect.
4. Host B's UDP receives datagram 1 whole: 1008 octets, port 5000 (RFC791-REASM-1).
5. Host B answers the echo request: an echo reply with 900 octets of data leaves host B
   toward host A (RFC791-REASM-3: datagram 2 was reassembled from its own fragments).

The check passes if observations 1 to 3 occur and then both observation 4 and observation
5 occur, in either order, within the time limit.

### Notes

- Observations 4 and 5 are two views of one rule. A receiver that keys its reassembly on
  three fields delivers neither datagram intact, or delivers one of them with the data of
  the other.
- Such a collision is rare from one source, because most sources draw the identification
  from one counter for all protocols. The rule is stated for the receiver, and crafted
  input is the way to check a receiver.
