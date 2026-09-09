# QUIC — English check procedures: acknowledgment

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Acknowledgment

- **Checks** — RFC9000-ACK-1 (must, for an ack-eliciting packet).
- **Requirement** — a packet that carries a frame other than ACK, PADDING or
  CONNECTION_CLOSE elicits an acknowledgment. The peer sends an ACK frame that covers that
  packet's number, within the maximum delay it declared.

### Procedure

1. Build the mockup.
2. Let host A open a connection and send the data block.
3. Observe the ACK frames that return from host B.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame, which is
   ack-eliciting. Record its packet number. *(Confirms the stimulus.)*
2. Host B sends a datagram whose QUIC packet carries an ACK frame whose largest
   acknowledged number is at least the recorded one (RFC9000-ACK-1).

### Notes

- The timing half of the requirement, the declared maximum delay, is a statement about a
  distribution. It belongs to level 4 with RFC 9002.
- An ACK frame acknowledges packet numbers, not octets. That is the difference from TCP's
  cumulative acknowledgment, and it is why observation 2 names a number and not a stream
  offset.
