# QUIC — English check procedures: connection close

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Connection close

- **Checks** — RFC9000-CLOSE-1 (description of the mechanism).
- **Requirement** — an endpoint terminates the connection immediately by sending a
  CONNECTION_CLOSE frame. The frame closes every stream at once; no application data
  follows it.

### Procedure

1. Build the mockup.
2. Let host A open a connection, send the data block, and close the connection.
3. Observe the datagrams on the path after the close.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a CONNECTION_CLOSE frame. *(Confirms
   the stimulus: the close really started.)*
2. From then on, no datagram carrying a STREAM frame travels in either direction: the
   streams are closed.

### Notes

- RFC 9000 gives an endpoint a second way to end a connection, the idle timeout. It is a
  timer and belongs to level 4.
- The closing and draining states that follow the frame, and the retransmission of the
  frame in answer to a late packet, are level 3.
