# TCP — English check procedures: data transfer

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

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
