# TCP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

What this document is for. The workflow's artifacts each answer one question: what the
standard says, what a run showed, what the model claims. This one holds what a person
learned while doing the work and would otherwise have to learn again — the behaviors of the
model that surprised the author, the traps in the tooling that cost a debugging cycle, and
the ordered list of what to do next.

The tooling quirks that apply to every protocol are in
[`ipv4/notes.md`](../ipv4/notes.md#tooling-quirks): the release-mode default of the test
library, the ini pattern that silently matches nothing, and the negative step that opens too
late. What follows is what TCP added.

## Model quirks

### The claimed standards are one generation behind

`Tcp.ned` names its RFCs openly, which is good practice, and every one it names in the areas
this pass checks has since been replaced:

| Claimed | Replaced by | Since | Mentions of the old / the new |
| --- | --- | --- | --- |
| RFC 793 | RFC 9293 | 2022 | RFC 9293 appears **nowhere** in `src/` |
| RFC 2581 | RFC 5681 | 2009 | 41 / 1 |
| RFC 1323 | RFC 7323 | 2014 | 27 / 0 |
| RFC 2988 | RFC 6298 | 2011 | 2 / 0 |
| RFC 3782 | RFC 6582 | 2012 | — / 0 |

For the nine features of this pass it costs nothing: RFC 9293 collects RFC 793 and its
updates, and the two agree. It costs something where they do not, and those places are
exactly the next levels — the congestion rules, the retransmission timer, the timestamp and
window scale rules all changed.

### The PSH bit is never set

RFC 9293 MUST-61 binds a sender whose SEND call offers no PUSH flag to set PSH on the last
buffered segment. This model's interface offers no such flag, and the bit is never set. The
code marks both halves itself:
[TcpConnectionUtil.cc:1008](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1008)
`// TODO when to set PSH bit?` and
[TcpConnectionEventProc.cc:116-117](../../../../../src/inet/transportlayer/tcp/TcpConnectionEventProc.cc#L116-L117)
`// FIXME how to support PUSH?`. The receiver ignores it too. `Rfc9293Push.test` keeps the
faithful assertion and declares `expected-result: FAIL`. Urgent data is unsupported by the
same comments, and is a level 5 candidate for the same treatment.

### The checksum is inserted one layer down

With `checksumMode = "computed"`, the TCP checksum is written by a hook that the TCP module
registers in IPv4's post-routing stage
([Tcp.cc:67-69](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L67-L69)). At the TCP
module's own `packetSentToLower` the field is still zero. Any check that reads the checksum
must observe at the link layer, not at the transport module. The default mode is again
`"declared"`, a placeholder.

### The default mode has no flow control, by the model's own words

[Tcp.ned:53-58](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L53-L58): in the default
"autoread" mode "the advertised window never decreases so there is effectively no flow
control". The flow-control check works because it tests the sender's respect for a **fixed
small window**, which does not need the window to close. A closing window needs the
"explicit-read" mode that the same note names, and which the applications expose as
`autoRead = false`.

### The initial congestion window is one segment

[TcpBaseAlg.cc:149](../../../../../src/inet/transportlayer/tcp/flavours/TcpBaseAlg.cc#L149).
This is what lets a 300-octet receiver window be the binding limit in the flow-control
check: it is smaller than a segment and smaller than the congestion window, so neither of
the other two limits can be mistaken for it.

## Scenario and tooling quirks

### An echo application defeats a delayed-acknowledgment scenario

`TcpEchoApp` piggybacks its acknowledgment on the data it returns, so the delay never
happens. The flow-control check uses `TcpSinkApp`, a pure receiver, and its close time is
dropped: 3000 octets through a 300-octet window at one round per 0.2 s take about 2 s.

### A unit-bearing capture breaks the next step

Capturing a field that carries a unit — the SYN's header length, `24B` — makes the tester's
capture substitution fail on the **next** step with `Attempt to use the value '24B' as a
dimensionless number`. The cause is in `tests/protocol/lib/EventPattern.cc`,
`formatCaptureValue`, which pushes every stored capture through `intValue()` whether or not
the step's expression uses it. The workaround is to capture the number without its unit
through a lambda. A fix in the framework would remove the trap.

### A step can pass on the wrong segment

Found in review, not by a failure: the data-transfer step for "the stream continues at
`ISS_A + 537`" would also match a **pure acknowledgment**, which carries that same sequence
number and no data. With an echoing peer the sender does send such acknowledgments. The step
now requires a positive data length. A sequence number alone does not identify a data
segment.

## Follow-ups, in the order I would do them

1. **A closing window**, with `autoRead = false` on the receiver. It is the half of flow
   control the default mode cannot show, and the model documents the mode itself.
2. **Level 3 needs no new document.** RFC 9293 already holds the text for RFC9293-CKSUM-2
   (a corrupted segment), reset on a live connection, a shrunk window and ICMP handling.
   All need interception or injection.
3. **Level 4 needs RFC 6298 and RFC 5681** in the in-scope set: the retransmission timer,
   congestion control, the zero-window probe and the acknowledgment delay bound, all as
   statistical checks.
4. **Name the current documents in `Tcp.ned`.** The module is already the model for how to
   state standards; updating the five obsolete numbers, and saying where the model
   deliberately keeps older behavior, would make it exemplary.
5. **RFC9293-SEQ-2** on the existing mockup — a pure ACK occupies no sequence space — and
   **RFC9293-ISS-2** split into its MUST-8 and SHLD-1 halves.
6. **Fix the capture substitution** in the test framework so a unit-bearing field can be
   captured directly.
