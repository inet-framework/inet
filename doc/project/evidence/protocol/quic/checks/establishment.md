# QUIC — English check procedures: connection establishment

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Connection establishment

- **Checks** — RFC9000-PKT-1, RFC9000-PKT-2 (description), RFC9000-SIZE-1 (must).
- **Requirement** — the packets that set a connection up carry a long header with the
  32-bit version field and both connection identifiers. Every UDP datagram that carries an
  Initial packet is at least 1200 octets, expanded with padding if its content is smaller.
  After the handshake, the packets that carry application data use the short header.

### Procedure

1. Build the mockup.
2. Let host A open a connection to host B and send the data block.
3. Observe the UDP datagrams on the path in both directions.

### Expected observations

1. The first UDP datagram from host A to host B carries a QUIC packet with a long header,
   whose version field names QUIC version 1 (RFC9000-PKT-1). *(This confirms the stimulus:
   a connection attempt started.)*
2. That same datagram is at least 1200 octets long (RFC9000-SIZE-1). The Initial packet's
   own content is far smaller, so the size can only come from expansion.
3. Host B answers with a datagram carrying a long header packet: the handshake proceeds in
   both directions.
4. Later, host A sends a datagram carrying a QUIC packet with a short header
   (RFC9000-PKT-2): the connection reached the state in which application data travels.

### Notes

- Observation 2 is the boundary value of QUIC and the reason a QUIC handshake is never
  small. It is also the requirement a simulation can most easily omit without visible harm,
  because a simulated path has no path-MTU problem to discover.
- The RFC requires the expansion of a server's ack-eliciting Initial datagrams as well.
  That half is stated separately in the notes of the results, because a server's first
  datagram is not always ack-eliciting and the check does not force the case.

## Server Initial datagram size

Checks: **RFC9000-SIZE-1** (must), the server half.

### Requirement

RFC 9000 §14.1: "Similarly, a server MUST expand the payload of all UDP datagrams carrying
ack-eliciting Initial packets to at least the smallest allowed maximum datagram size of
1200 bytes." A packet is ack-eliciting when it carries a frame other than ACK, PADDING or
CONNECTION_CLOSE; a CRYPTO frame is one.

### Scenario constants

- The common mockup. Nothing is crafted: the check watches the handshake the model
  produces on its own.

### Procedure

1. Build the mockup with a client and a server.
2. Let the client open a connection.
3. Read every datagram the server sends while the handshake runs. For each one that carries
   an Initial packet with an ack-eliciting frame, read the size of the datagram.

### Expected observations

1. The server sends a datagram that carries an Initial packet with a CRYPTO frame. This
   confirms the case the requirement names exists in this run.
2. That datagram is at least 1200 octets.

### Notes

- The client half of the same requirement is checked in
  [Connection establishment](#connection-establishment). The two halves are one catalog
  entry and two checks, because they need different observation points and the model may
  well do one and not the other.
- The floor is not decoration. A server may answer at most three times what it received
  before it trusts the address ([Address validation](address-validation.md#anti-amplification-limit));
  a small Initial datagram would leave too small a budget for a handshake to finish.

