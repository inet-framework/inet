# TCP — English check procedures: segment acceptance

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Out-of-window segment

Checks: **RFC9293-SEGA-1** and **RFC9293-SEGA-2** (description, rules of the state machine),
**RFC9293-RST-2** (must not).

### Requirement

RFC 9293 §3.10.7.4: a segment is acceptable when its sequence number falls in the receive
window. §3.5.2 and §3.10.7.4: an unacceptable segment must be answered with an empty
acknowledgment that carries the current send sequence number and the next sequence number
expected, and the connection stays in the same state. §3.5.2: a reset must not be sent when
it is not clear that the segment does not belong to the connection.

### Scenario constants

- The common transfer, with a relay on the path.
- The relay adds 100000 to the sequence number of the first data segment and computes the
  checksum again. The offset is far larger than any window either host advertises, so the
  segment falls outside the window whichever edge it is measured against.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let the connection open and the transfer start. The relay changes the first data segment.
3. Watch what host B sends back, and then watch whether the transfer finishes.

### Expected observations

1. The crafted segment reaches host B, carrying the moved sequence number.
2. Host B answers with an acknowledgment that carries no data and repeats where the stream
   stands, which is still the first octet host A sent.
3. Host B never answers with a reset.
4. The connection stays in the same state and finishes.

### Notes

- Observation 3 is the point of the rule. A receiver that answered with a reset would give
  anyone who can guess a connection the power to destroy it.
- The empty acknowledgment is not a courtesy: it is how an honest peer that lost its place
  learns where the stream stands.
