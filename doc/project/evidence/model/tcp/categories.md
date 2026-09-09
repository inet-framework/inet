# TCP checks — category decisions

> **Kind:** decision · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [test-anatomy.md](../../../design/test-anatomy.md)

Step 9 artifact of the standards test workflow. For each check, this document records the
test category and the reason. The categories and what each one can establish are
[test-anatomy.md](../../../design/test-anatomy.md#the-categories); the mapping from
observation class to category is in the guide, step 9. The category was predicted at step 3
from the observation class of each statement, and confirmed after the run recorded in
[`results.md`](results.md).

## Decisions for the checks of pass 2, level 2

### Connection establishment (RFC9293-EST-1, EST-2, SEQ-1, HDR-1, CKSUM-1; ISS-1, OPT-1) → protocol test

Every statement is about segments on a link: which control bits are set, in which order,
what the acknowledgment field holds, how long the header is, whether a checksum is present.
The expectations are relative to captured values, so the check needs no state inside the
module. Whether the checksum value is *right* is an encoding statement; a serializer unit
test is its home.

### Data transfer (RFC9293-DATA-1, SEG-1, ACK-1; ACK-2) → protocol test

The segment size, the sequence numbers and the final cumulative acknowledgment are wire
values. The decisive observation, an acknowledgment of the octet after the last one sent,
is a single segment. The delivery to the program is implied by it and not observed
separately.

### Connection termination (RFC9293-FIN-1, FIN-2) → protocol test

The four segments of the close are ordinary packets on the link. The connection state
behind them is internal and not observed; a check of the state machine itself is a module
test with a state signal.

### Flow control (RFC9293-WND-1, WND-2; ACKD-1) → protocol test

The advertised window, the size of the bounded segment, and the absence of new data during
the acknowledgment delay are all wire observations, and the timing they rely on — 0.15 s
of silence against a 0.2 s delay — is a scenario constant, not a distribution. That is why
this check stays a protocol test although it touches a timer: it uses the timer as a fixed
condition, it does not measure it. The bound on the delay itself, RFC9293-ACKD-1, is a
distribution and a statistical test at level 4.

### Connection reset (RFC9293-RST-1) → protocol test

The reset is an ordinary segment on the link with fully determined fields, and the
abandonment of the attempt is an absence. Nothing internal is needed.

## Decisions for the checks of pass 3, level 3

Every check of this pass is a **protocol test with interception**, and one adds a gateway.
The reason is the same for all of them and is what defines the level: the event under test
is one that no program can produce. A relay on the path makes it, and the observation stays
where it was, on the wire and at the ends.

### Checksum discard, out-of-window segment, blind reset, valid reset, no reset for a reset → protocol test with interception

Each needs one crafted segment: a wrong checksum, a moved sequence number, a reset that fits
the window or does not, a reset where a SYN was. The answers are ordinary wire observations,
and the absences — no reset, no acknowledgment, no further segment — are the same class the
level 2 checks already used.

### Shrunk window, no new data past a shrunk edge → protocol test with interception

The peer that shrinks the window is the relay. The first check reads an outcome at the end
of the connection; the second reads a decision at the sending module, because an interface
reports a send when the transmission ends and that is too late to be ordered against an
arrival.

### No window shrink → protocol test

No interception. A receiver is compared with itself over a whole transfer, which needs a
predicate that remembers what it has seen and no captured value.

### Soft ICMP error, Source Quench → protocol test with interception and a gateway

Neither report is forged. The relay makes a condition — a time to live of 1 — and a real
gateway writes the message. The Source Quench check then rewrites the type of that real
message, which needs a second relay, because a relay holds one rule at a time.

This is worth keeping as a pattern: where a check needs a report from the network, it is
better to make the condition and let the network write the report than to build the message.
The report then carries what a real one carries, and the quoted header matches the connection
without any care from the test.

## Category guidance for the open catalog entries

| Entry | Likely category | Reason |
| --- | --- | --- |
| RFC9293-SEQ-2 | protocol test | two wire values compared; the establishment mockup already produces them |
| RFC9293-ISS-2 | unit and module | the clock and the pseudorandom function are properties of the generator, not of the link |
| RFC9293-ZWP-1 | statistical, or protocol with a timing bound | a zero window followed by a probe within the persist timer; level 4 |
| RFC9293-ACKD-1 | statistical | the delay is a distribution with a bound; level 4 |
| The state machine as a whole | module test with a state signal | the states are internal |
| Retransmission and the timer (RFC 6298) | statistical, or protocol with interception | needs loss, and the timer value is a distribution |
| Congestion control (RFC 5681) | statistical | the window trajectory is the observable |
| RFC9293-ICMP-4, a hard error should abort | protocol test with interception and a gateway | the same mockup as the soft-error check, with a Destination Unreachable of a hard code; what it needs first is a decision about which behaviour the model intends |
| Silly window avoidance | protocol test with a program that reads slowly | no application in the model stalls its reads |
| Simultaneous open and close | protocol test with two clients | needs both ends to open at once; a scenario question, not a toolset one |
