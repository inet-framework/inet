# QUIC — English check procedures: address validation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Anti-amplification limit

Checks: **RFC9000-AMP-1** (must not).

### Requirement

RFC 9000 §8.1: "Prior to validating the client address, servers MUST NOT send more than
three times as many bytes as the number of bytes they have received." The same section says
when the address counts as validated: once an endpoint has successfully processed a
Handshake packet from the peer.

### Scenario constants

- The common mockup. Nothing is crafted: the check watches the handshake the model produces
  on its own.

### Procedure

1. Build the mockup with a client and a server.
2. Let the client open a connection.
3. From the first packet of the connection, count the octets the server receives and the
   octets it sends. Stop counting when the server has processed a Handshake packet from the
   client, which is where the address counts as validated.

### Expected observations

1. At every moment of that window, the octets the server has sent are at most three times
   the octets it has received.

### Notes

- The rule is a bound over a window, not a property of one packet, so the check needs a
  running count on both directions and a point at which to stop.
- A server that obeys the 1200-octet floor of
  [Server Initial datagram size](establishment.md#server-initial-datagram-size) has a large
  budget from the first packet. A server that does not obey it has a small one, so the two
  checks are worth reading together.
