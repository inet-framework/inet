# TCP — English check procedures: reset

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc9293/catalog.md](../../../standard/rfc9293/catalog.md)

The common mockups, the scenario constants and the index are in [`checks.md`](../checks.md).
The procedures come from the specification only. They name no simulation model and no code.

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

## Blind reset

Checks: **RFC9293-RSTP-1** (description, a rule of the state machine).

### Requirement

RFC 9293 §3.5.3: in all states except SYN-SENT, every reset is validated by checking its
sequence field, and a reset is valid only if its sequence number is in the window. A reset
that fails the check changes nothing.

### Scenario constants

- The common transfer, with a relay on the path.
- The relay turns the first data segment into a reset and gives it a sequence number 100000
  above the one that segment carried, which is far outside any window either host
  advertises. Everything else about the segment, its addresses and its ports, stays as it
  was; the relay computes the checksum again.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let the connection open and the transfer start. The relay changes the first data segment.
3. Watch host B, and then watch whether the transfer finishes.

### Expected observations

1. The crafted reset reaches host B, carrying a sequence number far outside the window.
2. Host B does not act on it: the connection is still there, and the whole stream is
   acknowledged after host A retransmits what the reset replaced.

### Notes

- This is what a third party can send who guesses the addresses and the ports but not the
  sequence numbers. The rule is what makes such an attempt useless.
- The data of the segment is lost with it, so the recovery waits for the retransmission
  timeout. The recovery is not what the check is about; it is how the check sees that the
  connection lived.

## Valid reset

Checks: **RFC9293-RSTP-2** (description, a rule of the state machine).

### Requirement

RFC 9293 §3.5.3: the receiver of a reset first validates it, then changes state; in a
synchronized state it aborts the connection and goes to CLOSED.

### Scenario constants

- The common transfer, with a relay on the path.
- The relay turns the first data segment into a reset and keeps its sequence number, which
  is exactly the next one host B expects.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let the connection open and the transfer start. The relay changes the first data segment.
3. Watch host B, then watch what happens when host A retransmits.

### Expected observations

1. The reset reaches host B at the sequence number it expects.
2. Host B sends nothing more for the connection.
3. Host A knows nothing of the abort and retransmits after its timeout. That segment now
   reaches a host with no such connection, and host B answers it with a reset of its own
   (RFC9293-RST-1). This is the proof that the connection is gone.

### Notes

- This check and [Blind reset](#blind-reset) are the two halves of one rule. A stack that
  obeyed every reset would be open to anyone who can guess the ports; a stack that obeyed
  none would never close.

## No reset for a reset

Checks: **RFC9293-RST-3** (description, a rule of the state machine).

### Requirement

RFC 9293 §3.5.2: if the connection does not exist, a reset is sent in response to any
incoming segment **except another reset**. §3.10.7.4 says the same of a live connection: an
unacceptable segment draws an acknowledgment unless the RST bit is set, and then the segment
is dropped.

### Scenario constants

- Host B runs no application at all, so nothing listens on the port and no connection exists
  there.
- The relay turns host A's first SYN into a reset.

### Procedure

1. Build the mockup with a relay between host A and host B.
2. Let host A open the connection. The relay changes its first SYN.
3. Watch host B.

### Expected observations

1. The reset reaches host B, where no connection exists.
2. Nothing comes back.

### Notes

- A SYN to that port would draw a reset (RFC9293-RST-1). The rule under test is the
  exception the same sentence makes.
- Two hosts that each answered a reset with a reset would exchange them without end. That is
  the reason for the exception, and it is why the check watches for silence.
