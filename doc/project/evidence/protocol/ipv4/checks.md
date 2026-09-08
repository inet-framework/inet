# RFC 791 — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc791/catalog.md](../../standard/rfc791/catalog.md)

Step 5 artifact of the standards test workflow (see
[derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)).
This file holds all English check procedures for RFC 791, one section per check. The
procedures come from the specification only. They name no simulation model and no code.
The catalog entries are in [`rfc791/catalog.md`](../../standard/rfc791/catalog.md).

## Common mockup

Three nodes in a row. Host A is the source. Node R is a gateway. Host B is the destination.

```
A ----------- R ----------- B
    link 1        link 2
```

- A small application on host A sends one UDP datagram to host B.
- Timing: host A sends the datagram shortly after the start. All expected events occur
  within 1 second. Observation stops after 1 second.
- Each check below states its own scenario constants: data size, sender TTL, the MTU of
  link 2, and the DF flag.
- Every node computes the header checksum of the datagrams it sends and forwards. A
  simulation model may offer a mode that assumes checksums correct without computing them;
  the checks run with that mode off, because one observation reads the value.

## TTL decrement

Checks: **RFC791-TTL-1** (must), **RFC791-CKSUM-1** (description). Also covers: **RFC791-TTL-3** (description).

### Requirement

RFC 791 §3.2: each module that processes the internet header must decrease the Time to Live
field. When no time information is available, the module must decrement the field by 1. The
sender sets the initial value. RFC 791 §3.1: the header checksum is recomputed at each point
that the header is processed, because fields such as the time to live change.

### Scenario constants

- Application data: 100 octets. Both links carry the datagram without fragmentation.
- The sender TTL on host A is set to 32.

### Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 32.
3. Start the network and let host A send the datagram.
4. Observe the datagram on link 1 and record its TTL value and its header checksum.
5. Observe the same datagram on link 2 and record its TTL value and its header checksum.
6. Observe the arrival of the datagram at host B.

### Expected observations

1. On link 1, the datagram appears with TTL = 32. This is the sender value (RFC791-TTL-3).
2. On link 2, the same datagram appears with TTL = 31, that is, the recorded value from
   link 1 minus 1 (RFC791-TTL-1).
3. Host B receives the datagram with TTL = 31.
4. The header checksum on link 2 differs from the header checksum on link 1
   (RFC791-CKSUM-1). The TTL changed by one, so a recomputed checksum cannot equal the old
   one: the two values differ by 256 in the one's complement sum.

The check passes if all four observations occur in this order within the time limit.

### Notes

- Compare the TTL on link 2 against the *recorded* value from link 1, not against the
  constant 31. The relative comparison stays correct if the sender default changes.
- The RFC permits a decrement larger than 1 when a module holds a datagram for more than one
  second. The forward delay in this mockup is far below one second, so the expected
  decrement is exactly 1. If the observed decrement is larger, examine the gateway before
  you change the expectation.

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

## TTL expiry

Checks: **RFC791-TTL-2** (must), **RFC792-TE-1** (must + may).

### Requirement

RFC 791 §3.2: a datagram whose time to live reaches zero must be destroyed. RFC 792: a
gateway that finds the time to live zero must discard the datagram, and it may notify the
source with a time exceeded message (type 11, code 0).

### Scenario constants

- Application data: 100 octets.
- The sender TTL on host A is set to 1: the datagram survives exactly one hop. The gateway
  decrements it to zero and must destroy it.

### Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 1.
3. Let host A send the datagram.
4. Observe link 1, link 2, host B, and the messages that return to host A.

### Expected observations

1. On link 1, the datagram appears with TTL = 1. This confirms the stimulus.
2. The gateway discards the datagram: a discard of this datagram is recorded at the gateway
   (RFC791-TTL-2). This is the observation that is in time — see the notes.
3. Host A receives an ICMP time exceeded message, type 11, code 0 (RFC792-TE-1).
4. From then on, the datagram never appears on link 2 and host B never receives it
   (RFC791-TTL-2, the wire form of observation 2).

### Notes

- Observation 2 is recorded at the gateway rather than on the wire. A watch on link 2 that
  starts only after the report arrives at host A starts too late: a forbidden forward would
  leave the gateway at the moment of the decision, before the report crosses link 1. The
  discard record is available at that moment; the wire absence of observation 4 then guards
  the rest of the window.
- Observation 3 checks a `may` clause. Its absence would not be a specification violation.
  The check asserts it because the cooperative behavior is the one worth confirming; a
  model that stays silent gets a note in the results and a `declined` in the matrix, not a
  defect. The two halves of RFC792-TE-1 carry different strengths, and the ledger keeps them
  apart.
- RFC 791 leaves open whether a gateway destroys a datagram that arrives with TTL 1 before
  or after the decrement. The observable is the same: nothing crosses link 2.

## Identification

Checks: **RFC791-ID-1** (must).

### Requirement

RFC 791 §2.3: the sender sets the identification field to a value that must be unique for
the source, the destination and the protocol for the time the datagram is active.

### Scenario constants

- Host A sends two datagrams of 100 octets each to host B, 0.1 s apart, from one source
  port to one destination port: the same source, destination and protocol.

### Procedure

1. Build the mockup network.
2. Let host A send two datagrams in succession.
3. Observe both on link 1 and record their identification fields.

### Expected observations

1. On link 1, the first datagram appears. Record its identification as ID1. This confirms
   the stimulus.
2. On link 1, the second datagram appears with an identification that differs from ID1
   (RFC791-ID-1).

### Notes

- Two datagrams show that the sender does not reuse a value at once. Uniqueness over the
  whole active time of a datagram needs a long run with many datagrams; that is a
  sharpening candidate, not a level 2 check.
- RFC 6864 rewrites this requirement for atomic datagrams. It is outside the in-scope set;
  see [`standards.md`](standards.md#override-table).

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
