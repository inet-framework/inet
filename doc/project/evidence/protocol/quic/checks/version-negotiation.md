# QUIC — English check procedures: version negotiation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Version negotiation

Checks: **RFC9000-VER-1** (description, what a server does).

### Requirement

RFC 9000 §6.1: if the version selected by the client is not acceptable to the server, the
server responds with a Version Negotiation packet, which lists the versions it will accept.
§17.2.1 marks such a packet by a version field of zero.

### Scenario constants

- The common mockup with a relay on the path.
- The relay changes the version field of the client's first Initial packet to a number that
  names no QUIC version, and changes nothing else.

### Procedure

1. Build the mockup with a relay between the client and the server.
2. Let the client open a connection. The relay changes the version of its first packet.
3. Watch what the server sends back.

### Expected observations

1. The packet reaches the server carrying the unknown version. This confirms the stimulus.
2. The server answers with a Version Negotiation packet: a long header whose version field
   is zero.
3. The connection does not open: no stream data crosses the path.

### Notes

- The version is one of the few fields a QUIC endpoint reads before it decrypts anything, so
  this answer is possible without any state. That is the point of the mechanism.
- The companion prohibition, that no endpoint answers a Version Negotiation packet with
  another (RFC9000-VER-2), needs such a packet delivered to an endpoint. The relay could
  make one, and the check is not written yet; the ledger says so.
