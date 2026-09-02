# RFC 791 — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [rfc791-checklist.md](rfc791-checklist.md)

Step 3 artifact of the RFC test workflow (see
[derive-tests-from-an-rfc.md](../../../guide/derive-tests-from-an-rfc.md)).
This file holds all English check procedures for RFC 791, one section per check. The
procedures come from the specification only. They name no simulation model and no code.
The catalog entries are in [`rfc791-checklist.md`](rfc791-checklist.md).

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

## TTL decrement

Checks: **RFC791-TTL-1** (must). Also covers: **RFC791-TTL-3** (description).

### Requirement

RFC 791 §3.2: each module that processes the internet header must decrease the Time to Live
field. When no time information is available, the module must decrement the field by 1. The
sender sets the initial value.

### Scenario constants

- Application data: 100 octets. Both links carry the datagram without fragmentation.
- The sender TTL on host A is set to 32.

### Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 32.
3. Start the network and let host A send the datagram.
4. Observe the datagram on link 1 and record its TTL value.
5. Observe the same datagram on link 2 and record its TTL value.
6. Observe the arrival of the datagram at host B.

### Expected observations

1. On link 1, the datagram appears with TTL = 32. This is the sender value (RFC791-TTL-3).
2. On link 2, the same datagram appears with TTL = 31, that is, the recorded value from
   link 1 minus 1 (RFC791-TTL-1).
3. Host B receives the datagram with TTL = 31.

The check passes if all three observations occur in this order within the time limit.

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
2. The gateway returns an ICMP destination unreachable message with code 4 ("fragmentation
   needed and DF set") to host A (RFC792-DU-4).
3. At no time does a part of the datagram appear on link 2 — no fragment, and not the whole
   datagram (RFC791-FRAG-5). Host B receives nothing of it.

The check passes if observations 1 and 2 occur, and observation 3 records no traffic of the
datagram on link 2.

### Notes

- The discard (observation 3) is a *must*. The error message (observation 2) is a *may*: a
  silent gateway also conforms to RFC 791 and RFC 792. This check asserts the cooperative
  behavior. If the tested system stays silent, record the result as an implementation gap
  against the "may" clause, not as a specification violation, and keep the discard result
  separate.
- The absence check on link 2 must cover the time around the expected error message, because
  a wrong implementation could fragment first and report later.
