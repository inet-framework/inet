# TCP — English check procedures: congestion control

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc5681/catalog.md](../../../standard/rfc5681/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

Congestion control is a sender-side control loop. Almost nothing of it appears on the wire
as a field: the window is a limit on how much the sender may have outstanding, and a reader
of the link sees the consequence rather than the value. Four of the five checks below read
the window itself, and one reads the wire.

One point of scope governs every check here. RFC 5681 states the algorithms that a
conforming sender uses. A sender that runs an algorithm which no RFC defines is outside
this document, so a check of it establishes nothing about conformance. Each check below
therefore names the algorithm it applies to as a scenario constant.

## Initial window

Checks: **RFC5681-IW-1** (must, as an upper bound), **RFC5681-IW-2** (must not).

### Requirement

RFC 5681 §3.1: the initial window is at most two, three or four segments, by the segment
size. The SYN, the SYN-ACK and the acknowledgment of the SYN-ACK do not enlarge it.

### Scenario constants

- The plain mockup. Host A opens a connection and sends a block of data at once.
- The segment size falls in one of the three bands of the table. A check may repeat the
  procedure once per band.
- The advertised window of host B is large, so it never limits the sender.

### Procedure

1. Read the congestion window of host A during the handshake.
2. Read it again at the first data send.

### Expected observations

1. The window does not grow across the handshake.
2. The window at the first data send is at most the bound of the table for the segment
   size in use: two segments above 2190 bytes, three between 1096 and 2190, four at or
   below 1095.

### Notes

- The table is an upper bound, not a value. A sender with a smaller initial window
  conforms. The check fails only a window that is too large.
- The check reads the window in segments, not in bytes, because the table bounds both and
  the smaller bound governs.

## Slow start growth

Checks: **RFC5681-SS-2** (upper bound); covers **RFC5681-SS-1**, **RFC5681-SSTH-1**.

### Requirement

RFC 5681 §3.1: below the threshold the window grows by at most one segment for each
acknowledgment that covers new data. The initial threshold is set arbitrarily high.

### Scenario constants

- The plain mockup, with no loss and a receiver window large enough never to limit the
  sender.
- Enough data to fill several windows, so the growth is visible over several round trips.

### Procedure

1. Open the connection and start the transfer.
2. Read the threshold before any loss.
3. Read the congestion window after each acknowledgment for the first few round trips.

### Expected observations

1. The threshold exceeds the largest window the receiver advertises.
2. The window never grows by more than one segment between two consecutive
   acknowledgments.
3. The window roughly doubles over each round trip.

### Notes

- Observation 3 is the shape of slow start, and it is weaker than observation 2. A model
  that grows the window more slowly satisfies the `MUST` and misses the shape, which is a
  deviation worth recording rather than a violation.

## Timeout response

Checks: **RFC5681-LOSS-1** (must, as an upper bound), **RFC5681-LOSS-3** (must).

### Requirement

RFC 5681 §3.1: a loss the retransmission timer detects sets the threshold to no more than
half the data in flight, with a floor of two segments, and sets the window to one segment.

### Scenario constants

- The plain mockup with a relay. The relay discards one data segment and everything after
  it in that window, so no duplicate acknowledgment reaches the sender and only the timer
  can detect the loss.
- The window has grown well beyond one segment before the loss, so the drop is visible.

### Procedure

1. Open the connection and let the window grow.
2. Record the data in flight at the moment of the loss.
3. Let the retransmission timer expire.
4. Read the threshold and the window after the expiry.

### Expected observations

1. The threshold is at most half the data in flight at step 2, and at least two segments.
2. The window is one segment.

### Notes

- The formula uses the data in flight, not the congestion window. RFC 5681 names the
  substitution of one for the other an easy mistake, so the check must compute the flight
  size from the wire rather than read the window.
- The loss must not produce three duplicate acknowledgments, or fast retransmit repairs it
  and the timer never expires. That is why the relay discards the rest of the window too.

## Fast retransmit and fast recovery

Checks: **RFC5681-FR-1** (should), **RFC5681-FR-2** (must), **RFC5681-FR-3** (must),
**RFC5681-FR-5** (must); covers **RFC5681-ACK-2**.

### Requirement

RFC 5681 §3.2: three duplicate acknowledgments indicate a loss. The sender retransmits at
once, sets the threshold by the same formula as a timeout, sets the window to the threshold
plus three segments, and sets the window back to the threshold when the repair is
acknowledged.

### Scenario constants

- The plain mockup with a relay. The relay discards exactly one data segment in the middle
  of a window and lets every later segment through, so the receiver answers each of them
  with a duplicate acknowledgment.
- The window holds at least four segments at the moment of the loss, so three duplicates
  can arrive.

### Procedure

1. Open the connection and let the window grow to at least four segments.
2. Let the relay discard one segment in the middle of a window.
3. Watch the acknowledgments host A receives, and the segment it sends in answer.
4. Read the threshold and the window after the third duplicate.
5. Read the window again after the acknowledgment that covers the repaired data.

### Expected observations

1. Host B answers each segment after the gap with a duplicate acknowledgment, at once.
2. Host A retransmits the missing segment after the third duplicate, and well before the
   retransmission timer would expire.
3. After the third duplicate the threshold falls, and the window equals the threshold plus
   three segments.
4. After the acknowledgment of new data the window equals the threshold.

### Notes

- The five statements are one episode, in order. A check that stops at observation 2 shows
  the retransmission and says nothing about the window, which is the half that RFC 5681
  adds over a plain retransmission.
- Observation 2 needs a second bound: the retransmission must arrive before the timer would
  have fired, or a slow timer and fast retransmit look the same.

## Window after a lost SYN

Checks: **RFC5681-IW-3** (must).

### Requirement

RFC 5681 §3.1: when the SYN or the SYN-ACK is lost, the initial window after the correctly
transmitted SYN is one segment.

### Scenario constants

- The plain mockup with a relay. The relay discards the first SYN and lets the retry
  through.

### Procedure

1. Start the connection. The relay discards the first SYN.
2. Let the retransmitted SYN open the connection.
3. Read the congestion window at the first data send.

### Expected observations

1. The window is one segment.

### Notes

- This check and the timeout check of RFC6298-SYN-1 share one scenario. A lost SYN changes
  both control loops, and the two answers are read at the same instant.
