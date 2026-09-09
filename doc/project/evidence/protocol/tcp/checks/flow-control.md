# TCP — English check procedures: flow control

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

## Flow control

- **Checks** — RFC9293-WND-1 (description), RFC9293-WND-2 (description, with the SHLD-15
  rule). Also covers: RFC9293-ACKD-1 (must, as a scenario condition).
- **Requirement** — every segment carries the number of data octets, counted from the
  acknowledged one, that its sender is willing to accept. The peer sends new data only up
  to that edge. When the advertised window is smaller than a full segment, the window and
  not the segment size bounds what the sender transmits, and the sender waits for the
  window to move before it sends more.

### Scenario constants for this check

| Constant | Value | Why |
| --- | --- | --- |
| Receiver window on host B | 300 octets | smaller than a 536-octet segment, so the window is the binding limit; also smaller than any initial congestion window, which is at least one segment |
| Acknowledgment delay on host B | 0.2 s | permitted: the delay must be less than 0.5 s (RFC9293-ACKD-1, MUST-40); it opens a gap in which the sender has no new window |
| Data block | 3000 octets | ten times the window, so the window binds many times over |

### Value arithmetic

Write `ISS_A` for host A's initial sequence number and `W` for the window host B
advertises, 300. Before any data is acknowledged, the sender may send octets `ISS_A + 1`
to `ISS_A + W` and no further. After host B acknowledges `ISS_A + W + 1` with the window
reopened, the sender may continue from `ISS_A + W + 1`.

### Procedure

1. Build the common mockup with the receiver window of host B at 300 octets and its
   acknowledgment delayed by 0.2 s.
2. Open the connection and let host A send the 3000-octet block at the send time.
3. Observe host B's window advertisement and host A's data segments.

### Expected observations

1. Host A sends its SYN. Record its sequence number as `ISS_A`. *(Confirms the connection
   stimulus.)*
2. Host A receives host B's SYN-ACK with a window field of 300 (RFC9293-WND-1). Record it
   as `W`.
3. After the send time, host A sends a segment with sequence number `ISS_A + 1` that
   carries exactly `W` octets of data: the window, not the segment size, bounded it
   (RFC9293-WND-2).
4. For 0.15 s after that segment, host A sends no segment that carries new data beyond
   `ISS_A + W` (RFC9293-WND-2). The acknowledgment that would open the window is still
   0.05 s away.
5. Host A receives an acknowledgment of `ISS_A + W + 1` with a window field of at least
   `W`: the receiver accepted the octets and reopened the window.
6. Host A sends a segment with sequence number `ISS_A + W + 1` that carries data: the
   sender resumed exactly at the edge.

### Notes

- A window smaller than the segment size is unusual in practice and exactly right for a
  check: it separates the window from the two other limits on a sender, the segment size
  and the congestion window, which are both at least one segment.
- The delayed acknowledgment is a receiver behavior the RFC permits. Without it, the
  acknowledgment would arrive before observation 4 could be stated, because the window
  would move at once.
- A shrunk window, a zero window and its probes, and the silly window rules are level 3
  and level 4.
