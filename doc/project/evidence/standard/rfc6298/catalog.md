# RFC 6298 (TCP retransmission timer) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6298-*` · **Stands on:** [standards.md](../../protocol/tcp/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 3 artifact of the standards test workflow, for one document of the in-scope set:
RFC 6298, the June 2011 text that obsoletes RFC 2988 and updates RFC 1122. It lists
statements that a test can check. The catalog comes from the RFC text only. It contains no
simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc6298.txt`](../../../../../../standards/RFC/rfc6298.txt) —
  Computing TCP's Retransmission Timer, June 2011. Downloaded 2026-09-11
  from <https://www.rfc-editor.org/rfc/rfc6298.txt>.

Quotes are verbatim. A reference such as `rfc6298.txt:122` points to a line of that
file. The document numbers its own rules, so §2.1 below is rule (2.1) of the
text.

This document is small and almost entirely normative: it is one control loop, written as
seven numbered rules for the estimator and seven for the timer.

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — lives in the coverage ledger,
[`tcp/coverage.md`](../../model/tcp/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC6298-INIT-1](#rfc6298-init-1) | Before any measurement the RTO is 1 second. |
| [RFC6298-FIRST-1](#rfc6298-first-1) | The first measurement sets SRTT to R and RTTVAR to R/2. |
| [RFC6298-UPD-1](#rfc6298-upd-1) | A later measurement updates RTTVAR, then SRTT, in that order. |
| [RFC6298-UPD-2](#rfc6298-upd-2) | The gains are alpha = 1/8 and beta = 1/4. |
| [RFC6298-RTO-1](#rfc6298-rto-1) | RTO is SRTT plus four times RTTVAR, floored at the clock granularity. |
| [RFC6298-MIN-1](#rfc6298-min-1) | An RTO below 1 second is rounded up to 1 second. |
| [RFC6298-MAX-1](#rfc6298-max-1) | An upper bound on the RTO is at least 60 seconds. |
| [RFC6298-KARN-1](#rfc6298-karn-1) | A retransmitted segment gives no RTT sample. |
| [RFC6298-SAMP-1](#rfc6298-samp-1) | At least one RTT measurement is taken per round trip. |
| [RFC6298-GRAN-1](#rfc6298-gran-1) | A zero variance term is rounded to the clock granularity. |
| [RFC6298-EARLY-1](#rfc6298-early-1) | No segment is retransmitted less than one RTO after its previous send. |
| [RFC6298-TMR-1](#rfc6298-tmr-1) | A data send starts the timer when it is not running. |
| [RFC6298-TMR-2](#rfc6298-tmr-2) | The timer stops when all outstanding data is acknowledged. |
| [RFC6298-TMR-3](#rfc6298-tmr-3) | An ACK of new data restarts the timer. |
| [RFC6298-EXP-1](#rfc6298-exp-1) | An expiry retransmits the earliest unacknowledged segment. |
| [RFC6298-BACK-1](#rfc6298-back-1) | An expiry doubles the RTO. |
| [RFC6298-SYN-1](#rfc6298-syn-1) | An expiry while awaiting the SYN ACK forces an RTO of 3 seconds for the data phase. |
| [RFC6298-COLL-1](#rfc6298-coll-1) | A new measurement after a backoff collapses the RTO again. |

## The estimator

### RFC6298-INIT-1

**Before any measurement the RTO is 1 second.**

> "(2.1) Until a round-trip time (RTT) measurement has been made for a segment sent between
> the sender and receiver, the sender SHOULD set RTO <- 1 second, though the "backing off"
> on repeated retransmission discussed in (5.5) still applies."
> — §2.1, `rfc6298.txt:122-125`

- Strength: should. Class: internal state with a scalar signal.
- Note: the text allows any value above 1 second, because the earlier document used 3
  seconds: "A TCP implementation MAY still use this value (or any other value > 1 second)"
  — `rfc6298.txt:128-129`. A check must accept a value of at least 1 second, not exactly 1.
- Check idea: open a connection and read the retransmission timeout before the first
  acknowledgment returns.

### RFC6298-FIRST-1

**The first measurement sets SRTT to R and RTTVAR to R/2.**

> "(2.2) When the first RTT measurement R is made, the host MUST set
>
>            SRTT <- R
>            RTTVAR <- R/2
>            RTO <- SRTT + max (G, K*RTTVAR)
>
>         where K = 4." — §2.2, `rfc6298.txt:133-139`

- Strength: must. Class: internal state with a scalar signal.
- Check idea: run one connection over a link of known delay, so R is known, then read the
  smoothed round-trip time and the variance after the first acknowledgment.

### RFC6298-UPD-1

**A later measurement updates RTTVAR, then SRTT, in that order.**

> "(2.3) When a subsequent RTT measurement R' is made, a host MUST set
>
>            RTTVAR <- (1 - beta) * RTTVAR + beta * |SRTT - R'|
>            SRTT <- (1 - alpha) * SRTT + alpha * R'
>
>         The value of SRTT used in the update to RTTVAR is its value before updating SRTT
>         itself using the second assignment.  That is, updating RTTVAR and SRTT MUST be
>         computed in the above order." — §2.3, `rfc6298.txt:141-149`

- Strength: must, for both the formulas and their order. Class: internal state with a
  scalar signal.
- Note: the order is observable only when SRTT and R' differ. On a constant-delay link the
  two orders give the same answer, so a check of the order needs a delay that changes.

### RFC6298-UPD-2

**The gains are alpha = 1/8 and beta = 1/4.**

> "The above SHOULD be computed using alpha=1/8 and beta=1/4 (as suggested in [JK88])."
> — §2.3, `rfc6298.txt:151-152`

- Strength: should. Class: internal state with a scalar signal.
- Check idea: change the link delay once, then compare the new smoothed value against the
  value the two gains predict from the old one.

### RFC6298-RTO-1

**RTO is SRTT plus four times RTTVAR, floored at the clock granularity.**

> "After the computation, a host MUST update
>            RTO <- SRTT + max (G, K*RTTVAR)" — §2.3, `rfc6298.txt:154-155`

- Strength: must. Class: internal state with a scalar signal.
- Check idea: read the three variables at the same instant and compare them against the
  formula. K is 4, from RFC6298-FIRST-1.

### RFC6298-MIN-1

**An RTO below 1 second is rounded up to 1 second.**

> "(2.4) Whenever RTO is computed, if it is less than 1 second, then the RTO SHOULD be
> rounded up to 1 second." — §2.4, `rfc6298.txt:157-158`

- Strength: should. Class: internal state with a scalar signal.
- Check idea: use a link fast enough that SRTT plus four times RTTVAR stays far below one
  second, then read the timeout.
- Note: this rule hides RFC6298-RTO-1 on a fast link. A check of the formula needs a delay
  large enough to lift the result above the floor.

### RFC6298-MAX-1

**An upper bound on the RTO is at least 60 seconds.**

> "(2.5) A maximum value MAY be placed on RTO provided it is at least 60 seconds."
> — §2.5, `rfc6298.txt:179-180`

- Strength: may. Class: internal state with a scalar signal.
- Note: a model without an upper bound satisfies this statement. A check can only fail a
  model that bounds the RTO below 60 seconds.

### RFC6298-KARN-1

**A retransmitted segment gives no RTT sample.**

> "TCP MUST use Karn's algorithm [KP87] for taking RTT samples.  That is, RTT samples MUST
> NOT be made using segments that were retransmitted (and thus for which it is ambiguous
> whether the reply was for the first instance of the packet or a later instance)."
> — §3, `rfc6298.txt:184-188`

- Strength: must, and a must not. Class: internal state with a scalar signal.
- Check idea: drop one data segment so it is retransmitted, then require the smoothed
  round-trip time to stay unchanged when the acknowledgment of the retransmission arrives.
- Note: the text gives one exception: a sample from a retransmitted segment is safe with
  the timestamp option — `rfc6298.txt:188-191`. A check must disable that option or account
  for it.

### RFC6298-SAMP-1

**At least one RTT measurement is taken per round trip.**

> "A TCP implementation MUST take at least one RTT measurement per RTT (unless that is not
> possible per Karn's algorithm)." — §3, `rfc6298.txt:198-200`

- Strength: must. Class: internal state with a scalar signal.
- Check idea: send several windows of data and count how often the smoothed round-trip time
  changes.

### RFC6298-GRAN-1

**A zero variance term is rounded to the clock granularity.**

> "However, if the K*RTTVAR term in the RTO calculation equals zero, the variance term MUST
> be rounded to G seconds (i.e., use the equation given in step 2.3)."
> — §4, `rfc6298.txt:213-215`

- Strength: must. Class: internal state with a scalar signal.
- Note: a constant-delay link drives RTTVAR to zero, so this rule and RFC6298-MIN-1 are the
  two that decide the timeout in a simple scenario.

## The timer

### RFC6298-EARLY-1

**No segment is retransmitted less than one RTO after its previous send.**

> "An implementation MUST manage the retransmission timer(s) in such a way that a segment is
> never retransmitted too early, i.e., less than one RTO after the previous transmission of
> that segment." — §5, `rfc6298.txt:237-239`

- Strength: must. Class: wire, with a scalar signal for the bound.
- Check idea: drop one data segment, then measure the interval between the original and the
  retransmission and compare it against the timeout in force.

### RFC6298-TMR-1

**A data send starts the timer when it is not running.**

> "(5.1) Every time a packet containing data is sent (including a retransmission), if the
> timer is not running, start it running so that it will expire after RTO seconds (for the
> current value of RTO)." — §5.1, `rfc6298.txt:244-247`

- Strength: recommended. The whole of §5.1 to §5.7 is "the RECOMMENDED algorithm"
  — `rfc6298.txt:241-242`.
- Class: internal. The timer itself is not on the wire.

### RFC6298-TMR-2

**The timer stops when all outstanding data is acknowledged.**

> "(5.2) When all outstanding data has been acknowledged, turn off the retransmission
> timer." — §5.2, `rfc6298.txt:249-250`

- Strength: recommended. Class: internal.
- Check idea: after the last acknowledgment, require that no retransmission follows for
  several times the timeout.

### RFC6298-TMR-3

**An ACK of new data restarts the timer.**

> "(5.3) When an ACK is received that acknowledges new data, restart the retransmission
> timer so that it will expire after RTO seconds (for the current value of RTO)."
> — §5.3, `rfc6298.txt:252-254`

- Strength: recommended. Class: internal.

### RFC6298-EXP-1

**An expiry retransmits the earliest unacknowledged segment.**

> "(5.4) Retransmit the earliest segment that has not been acknowledged by the TCP
> receiver." — §5.4, `rfc6298.txt:258-259`

- Strength: recommended. Class: wire.
- Check idea: drop one segment out of several in flight and require the retransmission to
  carry the sequence number of the earliest unacknowledged one.

### RFC6298-BACK-1

**An expiry doubles the RTO.**

> "(5.5) The host MUST set RTO <- RTO * 2 ("back off the timer").  The maximum value
> discussed in (2.5) above may be used to provide an upper bound to this doubling
> operation." — §5.5, `rfc6298.txt:261-263`

- Strength: must. Class: internal state with a scalar signal, and the wire for the interval.
- Check idea: black-hole one segment so the timer expires more than once, then require each
  interval to be twice the one before it.

### RFC6298-SYN-1

**An expiry while awaiting the SYN ACK forces an RTO of 3 seconds for the data phase.**

> "(5.7) If the timer expires awaiting the ACK of a SYN segment and the TCP implementation
> is using an RTO less than 3 seconds, the RTO MUST be re-initialized to 3 seconds when data
> transmission begins (i.e., after the three-way handshake completes)."
> — §5.7, `rfc6298.txt:269-272`

- Strength: must. Class: internal state with a scalar signal.
- Check idea: drop the first SYN so the handshake retries, let the connection open, then
  read the timeout at the first data send.
- Note: this is the one rule of the document that a change from RFC 2988 introduced, so a
  model written against the older text is likely to miss it.

### RFC6298-COLL-1

**A new measurement after a backoff collapses the RTO again.**

> "Note that after retransmitting, once a new RTT measurement is obtained (which can only
> happen when new data has been sent and acknowledged), the computations outlined in
> Section 2 are performed, including the computation of RTO, which may result in
> "collapsing" RTO back down after it has been subject to exponential back off (rule 5.5)."
> — §5, `rfc6298.txt:287-292`

- Strength: description of a consequence, not a separate requirement. Class: internal state
  with a scalar signal.
- Check idea: after a backoff, let one clean exchange complete and require the timeout to
  fall back towards the estimator value.
