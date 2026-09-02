# Check: a gateway decreases the TTL of a forwarded datagram

Step 3 artifact of the RFC test workflow. This document describes the check in English, from
the specification only. It names no simulation model and no code.

Checks: **R791-TTL-1** (must). Also covers: **R791-TTL-3** (description).
See [`../checklist.md`](../checklist.md).

## Requirement

RFC 791 §3.2: each module that processes the internet header must decrease the Time to Live
field. When no time information is available, the module must decrement the field by 1. The
sender sets the initial value.

## Mockup

Three nodes in a row. Host A is the source. Node R is a gateway. Host B is the destination.

```
A ----------- R ----------- B
    link 1        link 2
```

- Both links carry datagrams of the used size without fragmentation.
- A small application on host A sends one UDP datagram to host B. The application data is
  100 octets.
- The sender TTL on host A is set to 32.
- Timing: host A sends the datagram shortly after the start. All expected events occur
  within 1 second. Observation stops after 1 second.

## Procedure

1. Build the mockup network.
2. Set the sender TTL on host A to 32.
3. Start the network and let host A send the datagram.
4. Observe the datagram on link 1 and record its TTL value.
5. Observe the same datagram on link 2 and record its TTL value.
6. Observe the arrival of the datagram at host B.

## Expected observations

1. On link 1, the datagram appears with TTL = 32. This is the sender value (R791-TTL-3).
2. On link 2, the same datagram appears with TTL = 31, that is, the recorded value from
   link 1 minus 1 (R791-TTL-1).
3. Host B receives the datagram with TTL = 31.

The check passes if all three observations occur in this order within the time limit.

## Notes

- Compare the TTL on link 2 against the *recorded* value from link 1, not against the
  constant 31. The relative comparison stays correct if the sender default changes.
- The RFC permits a decrement larger than 1 when a module holds a datagram for more than one
  second. The forward delay in this mockup is far below one second, so the expected
  decrement is exactly 1. If the observed decrement is larger, examine the gateway before
  you change the expectation.
