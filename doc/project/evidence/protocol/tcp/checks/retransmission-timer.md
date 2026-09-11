# TCP — English check procedures: the retransmission timer

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc6298/catalog.md](../../../standard/rfc6298/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

These are the first checks of the TCP workflow that read a control variable rather than a
segment. The retransmission timeout is not on the wire: a reader of the link sees only the
interval between two transmissions of the same segment. Two of the five checks below
observe that interval, and three read the estimator itself.

## Initial timeout

Checks: **RFC6298-INIT-1** (should).

### Requirement

RFC 6298 §2.1: before any round-trip measurement the sender sets the timeout to one second.
The text permits a larger value, because the document it replaced used three seconds.

### Scenario constants

- The plain mockup. Host A opens a connection to host B and sends one block of data.
- The link delay is small, so the first acknowledgment returns quickly.

### Procedure

1. Open the connection.
2. Read the retransmission timeout of host A before the first acknowledgment arrives.

### Expected observations

1. The timeout is one second or more.

### Notes

- The statement is a `should`, and the permitted range is open above one second. A value
  below one second is a deviation; a value above it is not.
- The timeout must be read before the first measurement. After the first acknowledgment the
  estimator owns the value, and this check no longer applies.

## First measurement

Checks: **RFC6298-FIRST-1** (must), **RFC6298-RTO-1** (must); covers **RFC6298-MIN-1**.

### Requirement

RFC 6298 §2.2: the first measurement R sets the smoothed value to R and the variance to
R/2. §2.3: the timeout is then the smoothed value plus four times the variance, with the
clock granularity as a floor on the variance term. §2.4 raises a result below one second to
one second.

### Scenario constants

- The plain mockup, with a link whose one-way delay is fixed and known, so the round-trip
  time is a known constant.
- The delay is large enough that the smoothed value plus four times the variance exceeds
  one second. Otherwise the floor of §2.4 hides the formula.

### Procedure

1. Open the connection and send one segment.
2. Read the smoothed round-trip time, the variance and the timeout of host A after the
   first acknowledgment arrives.

### Expected observations

1. The smoothed value equals the round-trip time of the link.
2. The variance equals half of it.
3. The timeout equals the smoothed value plus four times the variance, and never less than
   one second.

### Notes

- The three observations must be read at the same moment. The estimator changes on every
  acknowledgment, so a later read gives a different answer.
- A fast link turns this check into a check of the one-second floor alone, which is why the
  delay is a scenario constant rather than a detail.

## Backoff doubling

Checks: **RFC6298-BACK-1** (must), **RFC6298-EARLY-1** (must).

### Requirement

RFC 6298 §5.5: each expiry doubles the timeout. §5: no segment is retransmitted less than
one timeout after its previous transmission.

### Scenario constants

- The plain mockup with a relay on the path. The relay discards every copy of the first
  data segment, so the segment never arrives and the timer expires again and again.
- Everything else passes, so the connection opens normally.

### Procedure

1. Open the connection and send a block of data.
2. Watch host A and record the time of each transmission of the first data segment.
3. Read the timeout after each expiry.

### Expected observations

1. The segment is transmitted several times.
2. Each interval between two consecutive transmissions is twice the interval before it.
3. The timeout after each expiry is twice the value before it.

### Notes

- Observation 2 and observation 3 say the same thing from the two sides: one from the link,
  one from the estimator. Keeping both is deliberate. A model could double the variable and
  not the interval, and one observation alone would not find it.
- The doubling may stop at an upper bound, which RFC6298-MAX-1 permits. A check must accept
  a bounded sequence as long as every step below the bound doubles.

## Karn's rule

Checks: **RFC6298-KARN-1** (must not).

### Requirement

RFC 6298 §3: a round-trip sample is never taken from a retransmitted segment, because the
acknowledgment is ambiguous. The timestamp option removes the ambiguity, and this check
therefore needs that option to be absent.

### Scenario constants

- The plain mockup with a relay. The relay discards the first copy of one data segment and
  lets the retransmission through.
- The timestamp option is not in use.
- The link delay changes after the loss, so a sample taken from the retransmission would
  give a different smoothed value than the sample rule allows.

### Procedure

1. Open the connection and let the estimator settle on one clean exchange.
2. Read the smoothed round-trip time.
3. Let the relay discard one segment, so host A retransmits it.
4. Read the smoothed round-trip time again, after the acknowledgment of the retransmission.

### Expected observations

1. The smoothed value does not change between step 2 and step 4.

### Notes

- The check is meaningful only if a sample from the retransmission would have changed the
  value. On a constant-delay link the forbidden sample and the permitted one agree, and the
  check passes without establishing anything.

## Timeout after a lost SYN

Checks: **RFC6298-SYN-1** (must).

### Requirement

RFC 6298 §5.7: when the timer expires while the sender waits for the acknowledgment of a
SYN, and the timeout in use is below three seconds, the timeout is set to three seconds
when the data transfer begins.

### Scenario constants

- The plain mockup with a relay. The relay discards the first SYN and lets the retry
  through.
- The data transfer starts after the handshake completes.

### Procedure

1. Start the connection. The relay discards the first SYN.
2. Let the retransmitted SYN open the connection.
3. Read the timeout of host A at the first data send.

### Expected observations

1. The timeout is three seconds.

### Notes

- This is the one rule that RFC 6298 added over the document it replaced. A model written
  against the older text keeps the value it had, which is the likely outcome.
- The rule pairs with RFC5681-IW-3, which forces the congestion window to one segment after
  the same loss. One scenario can carry both checks.
