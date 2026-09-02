# Check: a gateway discards a too-large datagram that carries the don't fragment flag

Step 3 artifact of the RFC test workflow. This document describes the check in English, from
the specification only. It names no simulation model and no code.

Checks: **R791-FRAG-5** (must), **R792-DU-4** (must + may).
See [`../checklist.md`](../checklist.md).

## Requirement

RFC 791 §2.3: a datagram marked "don't fragment" is not to be fragmented under any
circumstances. If the datagram cannot reach its destination without fragmentation, the
gateway discards it. RFC 792: in this case the gateway must discard the datagram, and it may
return a destination unreachable message with code 4, "fragmentation needed and DF set".

## Mockup

```
A ----------- R ----------- B
    link 1        link 2
   MTU 1500      MTU 576
```

- Host A is the source. Node R is a gateway. Host B is the destination.
- A small application on host A sends one UDP datagram with 1000 octets of application data
  to host B. The datagram carries DF = 1.
- The datagram total length is 1028 octets (see the arithmetic in
  [`fragment-reassembly.md`](fragment-reassembly.md)). 1028 > 576, so the gateway cannot
  forward it over link 2 without fragmentation.
- Timing: host A sends the datagram shortly after the start. All expected events occur
  within 1 second. Observation stops after 1 second.

## Procedure

1. Build the mockup network with MTU 1500 on link 1 and MTU 576 on link 2.
2. Set the don't fragment flag for the datagrams of the application on host A.
3. Start the network and let host A send the datagram.
4. Observe the datagram on link 1 and make sure DF = 1.
5. Observe link 2 for the full observation time.
6. Observe link 1 in the direction of host A for error messages.
7. Observe what host B delivers upward.

## Expected observations

1. On link 1, the datagram appears with DF = 1 and a total length above 576 octets. This
   confirms the stimulus.
2. The gateway returns an ICMP destination unreachable message with code 4 ("fragmentation
   needed and DF set") to host A (R792-DU-4).
3. At no time does a part of the datagram appear on link 2 — no fragment, and not the whole
   datagram (R791-FRAG-5). Host B receives nothing of it.

The check passes if observations 1 and 2 occur, and observation 3 records no traffic of the
datagram on link 2.

## Notes

- The discard (observation 3) is a *must*. The error message (observation 2) is a *may*: a
  silent gateway also conforms to RFC 791 and RFC 792. This check asserts the cooperative
  behavior. If the tested system stays silent, record the result as an implementation gap
  against the "may" clause, not as a specification violation, and keep the discard result
  separate.
- The absence check on link 2 must cover the time around the expected error message, because
  a wrong implementation could fragment first and report later.
