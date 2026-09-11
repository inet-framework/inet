# RFC 5681 (TCP congestion control) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC5681-*` · **Stands on:** [standards.md](../../protocol/tcp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 3 artifact of the standards test workflow, for one document of the in-scope set:
RFC 5681, the September 2009 text that obsoletes RFC 2581. It lists statements that a test
can check. The catalog comes from the RFC text only. It contains no simulation model names
and no code references.

Source, cached in this folder:

- `rfc5681.txt` — TCP Congestion Control, September 2009. Downloaded 2026-09-11
  from <https://www.rfc-editor.org/rfc/rfc5681.txt>.

Quotes are verbatim. A reference such as `rfc5681.txt:241` points to a line of the cached
file in this folder.

The document numbers three equations that the statements below refer to by number, as the
text does:

```
ssthresh = max (FlightSize / 2, 2*SMSS)            (4)
cwnd += min (N, SMSS)                              (2)
cwnd += SMSS*SMSS/cwnd                             (3)
```

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — lives in the coverage ledger,
[`tcp/coverage.md`](../../model/tcp/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC5681-USE-1](#rfc5681-use-1) | A sender uses slow start and congestion avoidance. |
| [RFC5681-WIN-1](#rfc5681-win-1) | The minimum of the congestion window and the advertised window governs transmission. |
| [RFC5681-IW-1](#rfc5681-iw-1) | The initial window follows the segment-size table. |
| [RFC5681-IW-2](#rfc5681-iw-2) | The handshake segments do not increase the congestion window. |
| [RFC5681-IW-3](#rfc5681-iw-3) | A lost SYN forces an initial window of one segment. |
| [RFC5681-SSTH-1](#rfc5681-ssth-1) | The initial slow start threshold is arbitrarily high. |
| [RFC5681-SS-1](#rfc5681-ss-1) | Slow start runs below the threshold, congestion avoidance above it. |
| [RFC5681-SS-2](#rfc5681-ss-2) | Slow start adds at most one segment per acknowledgment. |
| [RFC5681-CA-1](#rfc5681-ca-1) | Congestion avoidance adds about one segment per round trip. |
| [RFC5681-CA-2](#rfc5681-ca-2) | Congestion avoidance never adds more than one segment per round trip. |
| [RFC5681-LOSS-1](#rfc5681-loss-1) | A timeout sets the threshold to half the flight size. |
| [RFC5681-LOSS-2](#rfc5681-loss-2) | A repeated timeout holds the threshold constant. |
| [RFC5681-LOSS-3](#rfc5681-loss-3) | A timeout drops the congestion window to one segment. |
| [RFC5681-FR-1](#rfc5681-fr-1) | Three duplicate acknowledgments start a fast retransmit. |
| [RFC5681-FR-2](#rfc5681-fr-2) | The third duplicate acknowledgment sets the threshold. |
| [RFC5681-FR-3](#rfc5681-fr-3) | The retransmission inflates the window by three segments. |
| [RFC5681-FR-4](#rfc5681-fr-4) | Each further duplicate acknowledgment inflates the window by one segment. |
| [RFC5681-FR-5](#rfc5681-fr-5) | The acknowledgment of new data deflates the window to the threshold. |
| [RFC5681-IDLE-1](#rfc5681-idle-1) | An idle period reduces the window to the restart window. |
| [RFC5681-ACK-1](#rfc5681-ack-1) | An acknowledgment follows every second full-sized segment, and within 500 ms. |
| [RFC5681-ACK-2](#rfc5681-ack-2) | An out-of-order segment is acknowledged at once. |

## The two windows

### RFC5681-USE-1

**A sender uses slow start and congestion avoidance.**

> "The slow start and congestion avoidance algorithms MUST be used by a TCP sender to
> control the amount of outstanding data being injected into the network."
> — §3.1, `rfc5681.txt:209-211`

- Strength: must. Class: internal state with a scalar signal.
- Note: this is the statement that decides which algorithms of a model the rest of this
  catalog governs. An algorithm that answers to no RFC is outside the catalog.

### RFC5681-WIN-1

**The minimum of the congestion window and the advertised window governs transmission.**

> "The congestion window (cwnd) is a sender-side limit on the amount of data the sender can
> transmit into the network before receiving an acknowledgment (ACK), while the receiver's
> advertised window (rwnd) is a receiver-side limit on the amount of outstanding data.  The
> minimum of cwnd and rwnd governs data transmission." — §3.1, `rfc5681.txt:212-217`

- Strength: description. Class: wire, with a scalar signal for the window.
- Check idea: hold the advertised window below the congestion window and require the
  outstanding data to follow the advertised window, then reverse the two.

### RFC5681-IW-1

**The initial window follows the segment-size table.**

> "IW, the initial value of cwnd, MUST be set using the following guidelines as an upper
> bound.
>
>    If SMSS > 2190 bytes:
>        IW = 2 * SMSS bytes and MUST NOT be more than 2 segments
>    If (SMSS > 1095 bytes) and (SMSS <= 2190 bytes):
>        IW = 3 * SMSS bytes and MUST NOT be more than 3 segments
>    if SMSS <= 1095 bytes:
>        IW = 4 * SMSS bytes and MUST NOT be more than 4 segments"
> — §3.1, `rfc5681.txt:241-249`

- Strength: must, as an upper bound. Class: internal state with a scalar signal.
- Check idea: read the congestion window at the first data send, for a segment size in each
  of the three bands.
- Note: the table gives an upper bound, not a value. A model with a smaller initial window
  satisfies the statement. A check must fail only a window that is too large.

### RFC5681-IW-2

**The handshake segments do not increase the congestion window.**

> "As specified in [RFC3390], the SYN/ACK and the acknowledgment of the SYN/ACK MUST NOT
> increase the size of the congestion window." — §3.1, `rfc5681.txt:251-252`

- Strength: must not. Class: internal state with a scalar signal.
- Check idea: read the congestion window before and after the handshake completes.

### RFC5681-IW-3

**A lost SYN forces an initial window of one segment.**

> "Further, if the SYN or SYN/ACK is lost, the initial window used by a sender after a
> correctly transmitted SYN MUST be one segment consisting of at most SMSS bytes."
> — §3.1, `rfc5681.txt:253-255`

- Strength: must. Class: internal state with a scalar signal.
- Check idea: drop the first SYN, let the retransmitted SYN open the connection, then read
  the congestion window at the first data send.
- Note: this rule pairs with RFC6298-SYN-1. One loss at the handshake changes both control
  loops, and a single scenario can check the two together.

### RFC5681-SSTH-1

**The initial slow start threshold is arbitrarily high.**

> "The initial value of ssthresh SHOULD be set arbitrarily high (e.g., to the size of the
> largest possible advertised window), but ssthresh MUST be reduced in response to
> congestion." — §3.1, `rfc5681.txt:267-269`

- Strength: should for the height, must for the reduction. Class: internal state with a
  scalar signal.
- Check idea: read the threshold before any loss and require it to exceed the advertised
  window.

## Slow start and congestion avoidance

### RFC5681-SS-1

**Slow start runs below the threshold, congestion avoidance above it.**

> "The slow start algorithm is used when cwnd < ssthresh, while the congestion avoidance
> algorithm is used when cwnd > ssthresh.  When cwnd and ssthresh are equal, the sender may
> use either slow start or congestion avoidance." — §3.1, `rfc5681.txt:287-290`

- Strength: description, with a free choice at equality. Class: internal state with a
  scalar signal.
- Note: the free choice at equality means a check must not assert which algorithm runs when
  the two variables are equal.

### RFC5681-SS-2

**Slow start adds at most one segment per acknowledgment.**

> "During slow start, a TCP increments cwnd by at most SMSS bytes for each ACK received that
> cumulatively acknowledges new data.  Slow start ends when cwnd exceeds ssthresh (or,
> optionally, when it reaches it, as noted above) or when congestion is observed."
> — §3.1, `rfc5681.txt:292-295`

- Strength: an upper bound, with a recommended refinement:
  "we RECOMMEND that TCP implementations increase cwnd, per: cwnd += min (N, SMSS)"
  — `rfc5681.txt:297-300`.
- Class: internal state with a scalar signal.
- Check idea: send one window of data on a lossless link and require the congestion window
  to roughly double over one round trip, and never to grow by more than one segment per
  acknowledgment.

### RFC5681-CA-1

**Congestion avoidance adds about one segment per round trip.**

> "During congestion avoidance, cwnd is incremented by roughly 1 full-sized segment per
> round-trip time (RTT).  Congestion avoidance continues until congestion is detected."
> — §3.1, `rfc5681.txt:312-314`

- Strength: description of the principle, with three guidelines that follow it:
  "* MAY increment cwnd by SMSS bytes / * SHOULD increment cwnd per equation (2) once per
  RTT / * MUST NOT increment cwnd by more than SMSS bytes" — `rfc5681.txt:317-321`.
- Class: internal state with a scalar signal.
- Note: equation (3), `cwnd += SMSS*SMSS/cwnd`, is one permitted way
  — `rfc5681.txt:347-350`. A check must accept any of the permitted forms.

### RFC5681-CA-2

**Congestion avoidance never adds more than one segment per round trip.**

> "Note that during congestion avoidance, cwnd MUST NOT be increased by more than SMSS bytes
> per RTT." — §3.1, `rfc5681.txt:333-343`

- Strength: must not. Class: internal state with a scalar signal.
- Check idea: hold a connection in congestion avoidance and compare the growth of the window
  over several round trips against the segment size.

## The response to loss

### RFC5681-LOSS-1

**A timeout sets the threshold to half the flight size.**

> "When a TCP sender detects segment loss using the retransmission timer and the given
> segment has not yet been resent by way of the retransmission timer, the value of ssthresh
> MUST be set to no more than the value given in equation (4):
>
>       ssthresh = max (FlightSize / 2, 2*SMSS)            (4)"
> — §3.1, `rfc5681.txt:374-379`

- Strength: must, as an upper bound. Class: internal state with a scalar signal.
- Check idea: black-hole one segment, then read the threshold after the timer expires and
  compare it against the flight size at the moment of the loss.
- Note: the text warns against the common mistake of using the congestion window in place
  of the flight size — `rfc5681.txt:399-401`. The two differ, so a check must compute the
  flight size.

### RFC5681-LOSS-2

**A repeated timeout holds the threshold constant.**

> "On the other hand, when a TCP sender detects segment loss using the retransmission timer
> and the given segment has already been retransmitted by way of the retransmission timer at
> least once, the value of ssthresh is held constant." — §3.1, `rfc5681.txt:384-387`

- Strength: description of the required behaviour. Class: internal state with a scalar
  signal.
- Check idea: black-hole the same segment through two timer expiries and require the
  threshold to change once only.

### RFC5681-LOSS-3

**A timeout drops the congestion window to one segment.**

> "Furthermore, upon a timeout (as specified in [RFC2988]) cwnd MUST be set to no more than
> the loss window, LW, which equals 1 full-sized segment (regardless of the value of IW)."
> — §3.1, `rfc5681.txt:403-405`

- Strength: must. Class: internal state with a scalar signal.
- Note: the text cites RFC 2988 here. RFC 6298 replaced that document in June 2011, after
  RFC 5681 was published. The override table of
  [`standards.md`](../../protocol/tcp/standards.md) carries the substitution.

## Fast retransmit and fast recovery

### RFC5681-FR-1

**Three duplicate acknowledgments start a fast retransmit.**

> "The fast retransmit algorithm uses the arrival of 3 duplicate ACKs (as defined in section
> 2, without any intervening ACKs which move SND.UNA) as an indication that a segment has
> been lost.  After receiving 3 duplicate ACKs, TCP performs a retransmission of what
> appears to be the missing segment, without waiting for the retransmission timer to
> expire." — §3.2, `rfc5681.txt:439-444`

- Strength: should for the use of the algorithm — "The TCP sender SHOULD use the "fast
  retransmit" algorithm" — `rfc5681.txt:438-439`.
- Class: wire, with a scalar signal for the duplicate count.
- Check idea: drop one segment out of a window of several and require the retransmission to
  follow the third duplicate acknowledgment, well before the retransmission timeout.

### RFC5681-FR-2

**The third duplicate acknowledgment sets the threshold.**

> "2.  When the third duplicate ACK is received, a TCP MUST set ssthresh to no more than the
> value given in equation (4)." — §3.2, `rfc5681.txt:483-484`

- Strength: must. Class: internal state with a scalar signal.

### RFC5681-FR-3

**The retransmission inflates the window by three segments.**

> "3.  The lost segment starting at SND.UNA MUST be retransmitted and cwnd set to ssthresh
> plus 3*SMSS.  This artificially "inflates" the congestion window by the number of segments
> (three) that have left the network and which the receiver has buffered."
> — §3.2, `rfc5681.txt:488-491`

- Strength: must. Class: internal state with a scalar signal.

### RFC5681-FR-4

**Each further duplicate acknowledgment inflates the window by one segment.**

> "4.  For each additional duplicate ACK received (after the third), cwnd MUST be incremented
> by SMSS.  This artificially inflates the congestion window in order to reflect the
> additional segment that has left the network." — §3.2, `rfc5681.txt:493-496`

- Strength: must. Class: internal state with a scalar signal.
- Note: a model may bound this inflation — "A TCP MAY therefore limit the number of times
  cwnd is artificially inflated during loss recovery" — `rfc5681.txt:511-513`.

### RFC5681-FR-5

**The acknowledgment of new data deflates the window to the threshold.**

> "6.  When the next ACK arrives that acknowledges previously unacknowledged data, a TCP
> MUST set cwnd to ssthresh (the value set in step 2).  This is termed "deflating" the
> window." — §3.2, `rfc5681.txt:525-527`

- Strength: must. Class: internal state with a scalar signal.
- Check idea: this is the closing step of one fast recovery episode. One scenario checks
  RFC5681-FR-1 to RFC5681-FR-5 in sequence, which is why they belong together.

## Further rules

### RFC5681-IDLE-1

**An idle period reduces the window to the restart window.**

> "For the purposes of this standard, we define RW = min(IW,cwnd)." — §4.1,
> `rfc5681.txt:575`
>
> "Therefore, a TCP SHOULD set cwnd to no more than RW before beginning transmission if the
> TCP has not sent data in an interval exceeding the retransmission timeout."
> — §4.1, `rfc5681.txt:585-587`

- Strength: should. Class: internal state with a scalar signal.
- Check idea: grow the window, then stop sending for longer than the retransmission timeout,
  then send again and read the window.

### RFC5681-ACK-1

**An acknowledgment follows every second full-sized segment, and within 500 ms.**

> "When using delayed ACKs, a TCP receiver MUST NOT excessively delay acknowledgments.
> Specifically, an ACK SHOULD be generated for at least every second full-sized segment, and
> MUST be generated within 500 ms of the arrival of the first unacknowledged packet."
> — §4.2, `rfc5681.txt:592-596`

- Strength: should for the second-segment rule, must for the 500 ms bound.
- Class: wire, with a timing bound. This is the one statement of the catalog that needs a
  tolerance.
- Check idea: send one segment only and measure the delay before its acknowledgment; then
  send two and require an acknowledgment without the delay.

### RFC5681-ACK-2

**An out-of-order segment is acknowledged at once.**

> "Out-of-order data segments SHOULD be acknowledged immediately, in order to accelerate
> loss recovery.  To trigger the fast retransmit algorithm, the receiver SHOULD send an
> immediate duplicate ACK when it receives a data segment above a gap in the sequence
> space." — §4.2, `rfc5681.txt:637-640`

- Strength: should. Class: wire.
- Check idea: this is the receiver half of RFC5681-FR-1. The same scenario shows both, from
  the two ends of the link.
