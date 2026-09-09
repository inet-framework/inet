# TCP — English check procedures: checksum

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Checksum discard

Checks: **RFC9293-CKSUM-2** (must, MUST-3).

### Requirement

RFC 9293 §3.1: the TCP checksum is never optional; the sender must generate it and the
receiver must check it. A segment that fails the check carries no data the receiver can
trust.

### Scenario constants

- The common transfer, with a relay on the path. Every host computes real checksums.
- The relay replaces the checksum of the first data segment with its one's complement and
  changes nothing else, so the value is the only thing wrong with the segment.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let the connection open and the transfer start. The relay changes the first data segment.
3. Watch host B, then watch whether the transfer finishes.

### Expected observations

1. Host B discards the segment.
2. Host B does not acknowledge the data of that segment. The acknowledgment that would cover
   it can only follow a retransmission.
3. The connection recovers: the whole stream is acknowledged in the end.

### Notes

- Observation 3 is what tells a discard from a break. A receiver that answered the corrupt
  segment with a reset would also "not deliver" it, and would be wrong.

## Checksum by default

Checks: **RFC9293-CKSUM-1** (must, MUST-2).

### Requirement

RFC 9293 §3.1: "The TCP checksum is never optional. The sender MUST generate it." The value
is the one's complement sum over a pseudo header of the addresses and the length, the TCP
header, and the data.

### Scenario constants

- The plain mockup, with no relay.
- Host A is the subject: nothing in the scenario says anything about its checksum, so it
  sends in the state a host has when no one configures it.
- Host B is a control: it is told to compute the value the document defines.

### Procedure

1. Build the mockup with host A and host B.
2. Let the connection open and the transfer start.
3. Read each segment on the wire. Compute the value the document defines for that segment
   and compare it with the checksum field.

### Expected observations

1. A segment from host B carries the value the document defines. This step establishes that
   the arithmetic of the check is right.
2. A segment from host A carries that same value, with nothing configured.

### Notes

- The two steps let two failures be told apart. If the first fails, the arithmetic in the
  check is wrong. If the first passes and the second fails, the arithmetic is right and the
  default state of a host is the finding.
- Level 2 established that a segment carries a checksum field. This check establishes what
  is in it. For TCP the difference matters more than for UDP, because RFC 9293 says the
  checksum is never optional.
