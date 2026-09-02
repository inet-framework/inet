# Check: a gateway fragments a large datagram and the destination reassembles it

Step 3 artifact of the RFC test workflow. This document describes the check in English, from
the specification only. It names no simulation model and no code.

Checks: **R791-FRAG-1** (description), **R791-FRAG-2** (must), **R791-FRAG-3**
(description), **R791-FRAG-4** (description), **R791-REASM-1** (description).
See [`../checklist.md`](../checklist.md).

## Requirement

RFC 791 §2.3 and §3.2: a complete datagram carries MF = 0 and fragment offset 0. A gateway
that must forward a datagram over a link with a smaller MTU divides the data on 8-octet
boundaries. Each fragment keeps the identification of the original. Non-final fragments
carry MF = 1; the last fragment carries MF = 0. The destination combines the fragments by
identification, source, destination, and protocol, and it places the data by fragment
offset.

## Mockup

```
A ----------- R ----------- B
    link 1        link 2
   MTU 1500      MTU 576
```

- Host A is the source. Node R is a gateway. Host B is the destination.
- A small application on host A sends one UDP datagram with 1000 octets of application data
  to host B.
- Timing: host A sends the datagram shortly after the start. All expected events occur
  within 1 second. Observation stops after 1 second.

## Size arithmetic (from the RFC procedure)

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

## Procedure

1. Build the mockup network with MTU 1500 on link 1 and MTU 576 on link 2.
2. Start the network and let host A send the datagram.
3. Observe the datagram on link 1 and record its fragmentation fields.
4. Observe every part of the datagram on link 2. Record identification, offset, MF, and
   length of each part.
5. Observe what host B delivers to the protocol above IP.

## Expected observations

1. On link 1, the complete datagram appears with MF = 0 and fragment offset 0
   (R791-FRAG-1).
2. On link 2, a first fragment appears with offset 0 and MF = 1. Record its identification
   value (R791-FRAG-4).
3. On link 2, a second fragment appears with the same identification, offset 552 octets
   (69 blocks), and MF = 0 (R791-FRAG-2, R791-FRAG-3, R791-FRAG-4).
4. After that, no further part with that identification appears on link 2. The split gave
   exactly two fragments (R791-FRAG-2 arithmetic).
5. Host B delivers one complete UDP datagram of 1008 octets (8-octet header plus 1000
   octets of data) to the layer above IP (R791-REASM-1).

The check passes if observations 1-3 and 5 occur, and observation 4 records no extra part.

## Notes

- The offset in expected observation 3 is exact. It follows from the arithmetic above, not
  from a measurement.
- The two links form a single path, so the fragments arrive in order. Reorder tolerance is
  out of scope here.
- A deeper variant also compares the reassembled payload octet by octet. Keep that for a
  later iteration.
