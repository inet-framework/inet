# TCP — English check procedures: ICMP

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Soft ICMP error

Checks: **RFC9293-ICMP-3** (must not, MUST-56). Also covers: **RFC9293-ICMP-1** (must,
MUST-54).

### Requirement

RFC 9293 §3.9.2.2: an implementation must act on an ICMP error message passed up from the IP
layer, directing it to the connection that created the error. For a soft error — and the
document lists Time Exceeded codes 0 and 1 among them — the implementation must not abort
the connection.

### Scenario constants

- The common transfer, with a relay on the path and a gateway behind it.
- The relay sets the time to live of the first data segment to 1 and keeps the header
  checksum valid. The gateway then decrements it to zero and reports the expiry to host A.
- The error is therefore a real one, produced by a real node, and the header it quotes
  carries the addresses and the ports of the connection unchanged.

### Procedure

1. Build the mockup with a relay between host A and the gateway.
2. Let the connection open and the transfer start. The relay changes the first data segment.
3. Watch the report reach host A, then watch whether the connection lives.

### Expected observations

1. An ICMP Time Exceeded from the gateway reaches host A.
2. Host A ends nothing: no reset leaves it.
3. The connection carries on, and the whole stream is acknowledged in the end.

### Notes

- The quoted header is what makes the check a check of MUST-54 as well: the report can only
  reach the right connection if the implementation reads the addresses and the ports out of
  the quote.
- The data of the expired segment is lost with it, so the recovery waits for the
  retransmission timeout.

## Source Quench

Checks: **RFC9293-ICMP-2** (must, MUST-55).

### Requirement

RFC 9293 §3.9.2.2: an implementation must silently discard any received ICMP Source Quench
message.

### Scenario constants

- The same mockup as [Soft ICMP error](#soft-icmp-error), with two relays in series: one
  relay holds one rule, and this check needs two.
- The first relay sets the time to live of the first data segment to 1, so that the gateway
  produces a real report about this connection. The second relay rewrites the type of that
  report to 4, Source Quench, on its way to host A.

### Procedure

1. Build the mockup with two relays between host A and the gateway.
2. Let the connection open and the transfer start.
3. Watch the Source Quench reach host A, then watch whether the connection lives.

### Expected observations

1. The Source Quench reaches host A.
2. Host A ends nothing: no reset leaves it.
3. The connection carries on, and the whole stream is acknowledged in the end.

### Notes

- A discard changes nothing, so the check for it is the absence of every change: no answer,
  no break in the flow.
- Source Quench was deprecated for routers by RFC 6633, and RFC 9293 keeps the receive rule
  for exactly that reason: a host still meets the message and must ignore it.
