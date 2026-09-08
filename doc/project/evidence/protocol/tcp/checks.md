# TCP — English check procedures

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9293/catalog.md)

Step 5 artifact of the standards test workflow. This file holds the English check
procedures for TCP, one section per check. The procedures come from the specification only.
They name no simulation model and no code. The catalog entries are in
[`rfc9293/catalog.md`](../../standard/rfc9293/catalog.md).

The governing document is RFC 9293, not RFC 793. The reason is in
[`standards.md`](standards.md#override-table).

## Common mockup

Two hosts on one link. Host A opens a connection to host B, sends a block of data, and
closes. Host B accepts, receives what arrives, and closes when host A closes.

```
   host A  ------------  host B
           one link L
    (client)              (server)
```

Scenario constants, shared unless a check says otherwise:

| Constant | Value | Why |
| --- | --- | --- |
| Open time | 0.1 s | after the link and the address resolution settle |
| Send time | 0.2 s | clearly after the handshake, so a data segment cannot be confused with a handshake segment |
| Close time | 0.4 s | clearly after the data exchange |
| Maximum segment size | 536 octets | the default send MSS over IPv4 (RFC 9293 §3.7.1, MUST-15); no check sends a larger MSS option |
| Checksums | computed by every node | the TCP checksum is never optional (MUST-2); a model that offers a mode that assumes checksums correct runs with that mode off |
| Observation limit | 2 s | generous; nothing in the scenario needs more |

No check assumes a value for any sequence number. Each side picks its own initial sequence
number (RFC9293-ISS-1), so every expectation below is **relative**: a captured value plus a
computed offset.

## Connection establishment

- **Checks** — RFC9293-EST-1, RFC9293-EST-2, RFC9293-SEQ-1, RFC9293-HDR-1 (description),
  RFC9293-CKSUM-1 (must). Also covers: RFC9293-ISS-1 (description), RFC9293-OPT-1
  (should).
- **Requirement** — a connection opens with three segments. Host A sends a SYN carrying its
  own initial sequence number and a checksum. Host B answers with a segment that carries
  both SYN and ACK, and whose acknowledgment field is host A's initial sequence number plus
  one, because the SYN occupies one sequence number. Host A answers with an ACK, without
  SYN, whose acknowledgment field is host B's initial sequence number plus one. Every
  header is a whole number of 32-bit words, at least five.

### Value arithmetic

Write `ISS_A` for the sequence number of host A's SYN and `ISS_B` for the sequence number
of host B's SYN-ACK. Both are chosen by their sender and are unknown before the run.

| Segment | Sequence number | Acknowledgment field |
| --- | --- | --- |
| 1, A to B | `ISS_A` | none (the ACK bit is clear) |
| 2, B to A | `ISS_B` | `ISS_A + 1` |
| 3, A to B | `ISS_A + 1` | `ISS_B + 1` |

The plus-one in row 2 and row 3 is the whole of RFC9293-SEQ-1: the SYN occupied exactly one
sequence number.

### Procedure

1. Build the common mockup.
2. Let host A open a connection to host B at the open time.
3. Observe the segments that host A sends and receives on link L.

### Expected observations

1. Host A sends a segment with SYN set and ACK clear, a header length of at least 20
   octets, and a nonzero checksum. Record its sequence number as `ISS_A` and its header
   length. *(This observation confirms the stimulus, and covers HDR-1 and CKSUM-1. A header
   length of 24 octets means the SYN carries the MSS option, RFC9293-OPT-1; the option is a
   `should`, so its absence is a note and not a failure.)*
2. Host A receives a segment with SYN set and ACK set, whose acknowledgment field equals
   `ISS_A + 1`. Record its sequence number as `ISS_B`.
3. Host A sends a segment with ACK set and SYN clear, whose acknowledgment field equals
   `ISS_B + 1`, with a header length of 20 octets: no option, five words.

### Notes

- The order of the three observations is part of the requirement.
- Simultaneous open is a separate case in the RFC and is out of scope in this pass.

## Data transfer

- **Checks** — RFC9293-DATA-1, RFC9293-SEG-1 (must), RFC9293-ACK-1 (description). Also
  covers: RFC9293-ACK-2 (description).
- **Requirement** — the sender cuts the byte stream into segments no larger than the
  effective send MSS and sends them in sequence; every segment after the handshake carries
  an acknowledgment; the receiver's acknowledgment names the next octet it expects, so an
  acknowledgment of the end of the stream means that every octet arrived, in order.

### Value arithmetic

Write `ISS_A` for host A's initial sequence number. The data block is 5000 octets and the
effective send MSS is 536, so the stream occupies sequence numbers `ISS_A + 1` to
`ISS_A + 5000`, in nine segments of 536 octets and one of 176. The acknowledgment of the
whole stream is `ISS_A + 5001`.

### Procedure

1. Build the common mockup and open the connection as above.
2. Let host A send the 5000-octet data block at the send time.
3. Observe host A's data segments and the acknowledgments that return to host A.

### Expected observations

1. Host A sends its SYN. Record its sequence number as `ISS_A`. *(Confirms the connection
   stimulus.)*
2. After the send time, host A sends a segment with sequence number `ISS_A + 1` that
   carries exactly 536 octets of data and has ACK set (RFC9293-SEG-1: a full segment is
   the effective send MSS and not more; RFC9293-ACK-2).
3. Host A sends a segment with sequence number `ISS_A + 537` that carries data: the stream
   continues where the first segment ended, with no gap.
4. Host A receives a segment with ACK set whose acknowledgment field equals `ISS_A + 5001`
   (RFC9293-ACK-1, RFC9293-DATA-1: the receiver accepted the whole stream in order).

### Notes

- Observation 4 is decisive: a cumulative acknowledgment of the last octet cannot occur
  unless every earlier octet was received in order.
- The PSH bit on the last segment of the stream is a separate check, "Push on the last
  segment", so that its outcome cannot block the decisive observation here.
- The delivery of the octets to the program on host B is implied by observation 4 and not
  observed separately: the acknowledged octets are the ones the receiver holds for the
  program.
- With loss on the path the sender retransmits; that machinery is level 3 and out of scope
  here. The check runs on a link without loss.

## Push on the last segment

- **Checks** — RFC9293-PSH-1 (must, under the condition that the SEND call has no PUSH
  flag).
- **Requirement** — a sender whose interface offers no PUSH flag must not buffer data
  indefinitely and must set PSH on the last buffered segment, that is, on the segment it
  sends when no more queued data remains.

### Procedure

1. Build the common mockup and open the connection as in the data transfer check.
2. Let host A send the 5000-octet block at the send time.
3. Observe host A's data segments.

### Expected observations

1. Host A sends its SYN. Record its sequence number as `ISS_A`. *(Confirms the connection
   stimulus.)*
2. Host A sends the last segment of the stream, the one whose last data octet is
   `ISS_A + 5000`, with PSH set (RFC9293-PSH-1).

### Notes

- Observation 2 identifies the last segment by its end, not by a fixed sequence number, so
  it holds however the sender cut the stream.
- The condition of the requirement holds for an interface with no PUSH flag on its send
  operation; an interface that offers the flag may leave the bit to the application. The
  results record which case the model is.
- This check is separate from the data transfer check on purpose: a failure here must not
  block the observation that the whole stream was acknowledged.

## Connection termination

- **Checks** — RFC9293-FIN-1 (description), RFC9293-FIN-2 (description).
- **Requirement** — a normal close carries a FIN in each direction, and each FIN is
  acknowledged. The FIN occupies one sequence number, so the acknowledgment that answers a
  FIN whose sequence number is `F` equals `F + 1`. The two directions close one at a time.

### Value arithmetic

Write `F_A` for the sequence number of host A's FIN and `F_B` for the sequence number of
host B's FIN.

| Segment | Direction | Acknowledgment field |
| --- | --- | --- |
| FIN of A | A to B | — |
| ACK of that FIN | B to A | `F_A + 1` |
| FIN of B | B to A | — |
| ACK of that FIN | A to B | `F_B + 1` |

### Procedure

1. Build the common mockup, open the connection, and send a 100-octet block as above.
2. Let host A close the connection at the close time.
3. Observe the four segments of the close on link L.

### Expected observations

1. Host A sends a segment with FIN set. Record its sequence number as `F_A`. *(This
   observation confirms the stimulus: the close really started.)*
2. Host A receives a segment with ACK set whose acknowledgment field equals `F_A + 1`. The
   plus-one is RFC9293-FIN-1: the FIN occupied one sequence number.
3. Host A receives a segment with FIN set. Record its sequence number as `F_B`.
4. Host A sends a segment with ACK set whose acknowledgment field equals `F_B + 1`.

### Notes

- Observation 2 and observation 3 may arrive in one segment, because a peer may set FIN and
  ACK together. The check requires two matches and one segment can satisfy only one of
  them, so the check as written expects the peer to answer with a separate acknowledgment
  first. Figure 12 of the RFC shows exactly that separation.
- Simultaneous close and the TIME-WAIT duration are out of scope.

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

## Connection reset

- **Checks** — RFC9293-RST-1 (description, a rule of the state machine).
- **Requirement** — a segment for a connection that does not exist is answered with a
  reset, except when the segment is itself a reset. For a SYN, the reset carries RST and
  ACK, sequence number zero, and an acknowledgment of the SYN's sequence number plus its
  length: `<SEQ=0><ACK=SEG.SEQ+SEG.LEN><CTL=RST,ACK>`. The connection attempt ends.

### Scenario constants for this check

- Host A opens a connection to port 7000 of host B. No program on host B listens on
  port 7000.

### Procedure

1. Build the common mockup with no listener on port 7000 of host B.
2. Let host A open a connection to port 7000 at the open time.
3. Observe link L in both directions.

### Expected observations

1. Host A sends a SYN to port 7000. Record its sequence number as `ISS_A`. *(Confirms the
   stimulus.)*
2. Host A receives a segment from port 7000 with RST set and ACK set, sequence number 0,
   and acknowledgment field `ISS_A + 1` (RFC9293-RST-1: the SYN occupies one octet of
   sequence space, so SEG.SEQ + SEG.LEN is `ISS_A + 1`).
3. For 0.3 s after the reset, host A sends nothing more to port 7000: the attempt is
   abandoned, and no acknowledgment or data follows.

### Notes

- The reset here answers a SYN; a reset that answers a segment with ACK set uses the
  acknowledgment's value as its sequence number, which is the other branch of the same
  rule and out of scope.
- Reset handling on a live connection, and the rules for accepting a received reset, are
  level 3.
