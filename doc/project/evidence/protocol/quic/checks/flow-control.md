# QUIC — English check procedures: flow control

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9000/catalog.md](../../../standard/rfc9000/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Flow control

- **Checks** — RFC9000-FC-1 (must).
- **Requirement** — a receiver advertises a limit on the data it will accept, per stream
  and for the connection as a whole, and a sender sends no data beyond either limit. A
  sender that has reached a limit waits for the receiver to raise it.

### Scenario constants for this check

| Constant | Value | Why |
| --- | --- | --- |
| Initial stream limit at host B | 2000 octets | smaller than the data block, so the limit binds |
| Data block | 4000 octets | twice the limit, so the sender must stop and wait |

### Procedure

1. Build the mockup with host B's initial stream limit at 2000 octets.
2. Let host A open a connection and send the 4000-octet block on one stream.
3. Observe the total stream data that host A sends before host B raises the limit.

### Expected observations

1. Host A sends a datagram whose QUIC packet carries a STREAM frame. *(Confirms the
   stimulus.)*
2. The total stream data host A sends, before any frame from host B raises the limit,
   does not exceed 2000 octets (RFC9000-FC-1).
3. Host B sends a frame that raises the limit, and host A then sends the remainder, so that
   host B's application receives all 4000 octets.

### Notes

- Observation 2 is the whole of the requirement: the limit binds a sender, it does not
  merely inform it.
- QUIC has two limits at once, per stream and per connection. This check exercises the
  stream limit. A connection limit binding across several streams is a second check, and it
  needs a scenario with more than one stream.
