# TCP — English check procedures: window

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Shrunk window

Checks: **RFC9293-WND-4** (must, MUST-34).

### Requirement

RFC 9293 §3.8.6: a sending peer must be robust against window shrinking, which may cause the
usable window to become negative.

### Scenario constants

- The common transfer, with a relay on the path.
- The relay rewrites the window field of the fourth acknowledgment from host B to 100 octets
  and computes the checksum again. By then host A has several segments in flight, so the
  right edge the relay writes falls behind the sequence number host A has reached: the
  usable window is negative.
- Host B itself never shrinks its window. The relay is what a peer that does would look like
  from host A's side.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let the connection open and the transfer start. The relay changes one acknowledgment.
3. Watch whether host A survives it, and whether the transfer finishes.

### Expected observations

1. The shrunk acknowledgment reaches host A.
2. The connection carries on, and the whole stream is acknowledged in the end.

### Notes

- The requirement is robustness, which is an absence of failure. The transfer that finishes
  is the shape that absence takes here.
- What host A may send while the usable window is negative is a separate rule, and a
  separate check: [No new data past a shrunk edge](#no-new-data-past-a-shrunk-edge). The two
  are apart on purpose, so that a failure of the second cannot hide the verdict of the
  first.

## No new data past a shrunk edge

Checks: **RFC9293-WND-5** (should not, SHLD-15).

### Requirement

RFC 9293 §3.8.6: when the usable window becomes negative, the sender should not send new
data, but should retransmit normally the old unacknowledged data between the left edge and
the right edge.

### Scenario constants

- The same as [Shrunk window](#shrunk-window): the relay shrinks the fourth acknowledgment
  to a window of 100 octets.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let the connection open and the transfer start. The relay changes one acknowledgment.
3. From the moment host A reads the shrunk acknowledgment, watch what it sends.

### Expected observations

1. The shrunk acknowledgment reaches host A. Its acknowledgment number plus 100 is the new
   right edge.
2. Host A sends no segment of new data that starts at or beyond the new right edge.

### Notes

- A retransmission of data the peer has not acknowledged starts at the left edge, which is
  below the new right edge, so the watch does not catch it. That is right: the document
  allows the retransmission and forbids only new data.
- The watch sits at the sending module and not at its interface. An interface reports a send
  when the transmission ends, which can fall after an acknowledgment that arrived while the
  frame was on the wire; the module reports when it decides to send, which is the moment
  this rule is about.

## No window shrink

Checks: **RFC9293-WND-3** (should not, SHLD-14).

### Requirement

RFC 9293 §3.8.6: a receiver should not shrink the window, that is, should not move the right
window edge to the left.

### Scenario constants

- The plain mockup, with no relay. The check watches what a receiver advertises of its own
  accord.

### Procedure

1. Build the mockup with host A and host B.
2. Let the connection open, the transfer run and the connection close.
3. For every segment host B sends, compute the right edge: the acknowledgment number plus
   the window field. Keep the largest edge seen so far.

### Expected observations

1. No segment from host B carries a right edge to the left of one it advertised earlier.

### Notes

- The rule needs no captured value and no relay. It compares a receiver with itself.
- Bound: the check establishes the rule for a transfer in which the receiving program
  consumes everything at once. A receiver whose program reads slowly is where an
  implementation is tempted to shrink, and that scenario needs a program that stalls.
