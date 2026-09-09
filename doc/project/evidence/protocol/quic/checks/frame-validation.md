# QUIC — English check procedures: frame validation

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Unknown frame type

Checks: **RFC9000-ERR-1** (must).

### Requirement

RFC 9000 §12.4: "An endpoint MUST treat the receipt of a frame of unknown type as a
connection error of type FRAME_ENCODING_ERROR." §10.2: an endpoint that ends a connection
because of an error sends a CONNECTION_CLOSE frame that names the error.

### Scenario constants

- The common mockup with a relay on the path.
- The relay changes the type of one frame of a packet in flight to a number that Table 3 of
  §12.4 does not list, and changes nothing else.
- The frame it changes carries stream data, so the connection is established and running
  when the unknown frame arrives.

### Procedure

1. Build the mockup with a relay between the client and the server.
2. Let the client send its block. The relay changes the type of one frame.
3. Watch what the receiver does.

### Expected observations

1. The packet reaches the receiver carrying the unknown frame type. This confirms the
   stimulus.
2. The receiver ends the connection and says why: a CONNECTION_CLOSE frame that names the
   error.
3. The receiver keeps running. A connection error ends one connection, not the endpoint.

### Notes

- Observation 3 is a low bar, and it is here because it is the one an implementation is most
  likely to miss. A frame type is a number from a registry that grows; an endpoint meets
  unknown ones in the ordinary course of the internet and must survive them.
- The check does not require the receiver to be the server. Either endpoint owes the same
  answer, and the relay changes whichever direction the scenario makes convenient.
