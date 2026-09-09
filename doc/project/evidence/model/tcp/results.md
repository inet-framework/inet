# TCP checks — run results and model analysis (pass 3, level 3)

> **Kind:** report · **Status:** snapshot 2026-09-09 · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9293/catalog.md), [checks.md](../../protocol/tcp/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the TCP
workflow that may reference code.

- Pass 2, level 2. Date: 2026-09-08. Tree: `inet-rfc-tests-tcp`, branch
  `topic/rfc-tests-tcp` on top of `topic/rfc-tests-ipv4`, source identical to `master`.
- Pass 3, level 3. Date: 2026-09-09. Tree: `inet-master`, branch `master`, source unchanged
  by this pass.
- Command, after the `setenv` scripts of OMNeT++ and INET:

  ```sh
  cd tests/protocol/lib && MODE=debug ./build.sh
  inet_run_protocol_tests -p inet -w tcp
  ```

## Verdicts

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9293ConnectionEstablishment.test | RFC9293-EST-1, EST-2, SEQ-1, HDR-1, CKSUM-1; ISS-1, OPT-1 | PASS |
| Rfc9293DataTransfer.test | RFC9293-DATA-1, SEG-1, ACK-1; ACK-2 | PASS |
| Rfc9293Push.test | RFC9293-PSH-1 | **FAIL (expected)** — model gap, see below |
| Rfc9293ConnectionTermination.test | RFC9293-FIN-1, FIN-2 | PASS |
| Rfc9293FlowControl.test | RFC9293-WND-1, WND-2; ACKD-1 | PASS |
| Rfc9293Reset.test | RFC9293-RST-1 | PASS |
| TcpHandshake.test (pre-existing) | — | PASS |
| TcpRetransmit.test (pre-existing) | — | PASS |

Pass 3, level 3:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9293ChecksumDiscard.test | RFC9293-CKSUM-2 | PASS |
| Rfc9293OutOfWindowSegment.test | RFC9293-SEGA-1, SEGA-2, RST-2 | PASS |
| Rfc9293BlindReset.test | RFC9293-RSTP-1 | PASS |
| Rfc9293ValidReset.test | RFC9293-RSTP-2; covers RST-1 | PASS |
| Rfc9293NoResetForReset.test | RFC9293-RST-3 | PASS |
| Rfc9293ShrunkWindow.test | RFC9293-WND-4 | PASS |
| Rfc9293NoWindowShrink.test | RFC9293-WND-3 | PASS |
| Rfc9293SoftIcmpError.test | RFC9293-ICMP-3; covers ICMP-1 | PASS |
| Rfc9293ChecksumDefault.test | RFC9293-CKSUM-1 (the value) | **FAIL**, model gap 2 |
| Rfc9293ShrunkWindowNoNewData.test | RFC9293-WND-5 | **FAIL**, model gap 3 |
| Rfc9293SourceQuench.test | RFC9293-ICMP-2 | **FAIL**, model gap 4 |

Summary after pass 3: 19 tests, 15 PASS, 4 FAIL, each failure declared with
`%# expected-result: FAIL`. The other four suites of the same tree stay where they were:
IPv4 23 (17 PASS, 6 expected FAIL), UDP 13 (10 PASS, 3 expected FAIL), IPv6 29 (21 PASS, 8
expected FAIL), QUIC 6 PASS.

Eight of the eleven new tests pass, and they are the ones that matter most for a protocol
that carries a connection: a corrupt segment is discarded and the stream still arrives; a
segment outside the window draws an acknowledgment and never a reset; a reset from a third
party who cannot guess the sequence numbers is ignored, while one that fits is obeyed; a
reset draws no reset; a shrunk window does not break the sender; and a soft ICMP error does
not end the connection. Those are the rules that keep a connection alive under attack and
under a fault, and the model keeps them.

Summary of pass 2: 8 tests, 7 PASS, 1 FAIL (expected), in 2.1 s. The runner's overall verdict is PASS,
because the one failure is declared. The SYN's header length was captured as 24 octets: the
MSS option is present, so the `should` of RFC9293-OPT-1 is met.

## Model gap 1 (pass 2): the PSH bit is never set

`Rfc9293Push.test` keeps the faithful assertion — the last segment of a 5000-octet send
carries PSH — and declares `%# expected-result: FAIL`. The classification is **model gap**,
on three pieces of evidence:

1. No call sets the PSH bit anywhere in the TCP sender. `sendSegment` carries the comment
   at the place where the bit would be set:
   [TcpConnectionUtil.cc:1008](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1008)
   `// TODO when to set PSH bit?`
2. The SEND processing says why:
   [TcpConnectionEventProc.cc:116-117](../../../../../src/inet/transportlayer/tcp/TcpConnectionEventProc.cc#L116-L117)
   `// FIXME how to support PUSH? One option is to treat each SEND as a unit of data, and
   set PSH at SEND boundaries`.
3. The receiver ignores the bit:
   [TcpConnectionRcvSegment.cc:542](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L542)
   `// FIXME observe PSH bit`, and
   [TcpConnectionRcvSegment.cc:987](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L987)
   `// Now: URG and PSH we don't support yet`.

The requirement is conditional, and the condition holds: RFC 9293 MUST-61 binds a sender
whose SEND call offers no PUSH flag, and the model's socket interface offers none. Within
a simulation the practical effect is small — the model's receiver delivers data to the
application at once in its default mode — but on the wire the model's segments differ from
every real stack's, which set PSH on the last segment of a write. Traces compared with real
captures, and peers that hold data until PSH, see the difference.

The gap was found by this pass, not by the earlier one: the earlier pass had no push
statement in the catalog. The test is separate from the data transfer test on purpose, so
that its failure cannot block the decisive observation that the whole stream was
acknowledged.

## Model gap 2 (pass 3): the default mode writes no checksum at all

**Statement.** [RFC9293-CKSUM-1](../../standard/rfc9293/catalog.md#rfc9293-cksum-1), must:
"The TCP checksum is never optional. The sender MUST generate it (MUST-2)", §3.1,
`rfc9293.txt:458-459`.

**What the model does.** The `checksumMode` parameter of the ~Tcp module is
`default("declared")` (src/inet/transportlayer/tcp/Tcp.ned:189). In that mode nothing is
computed: the field stays at zero and the header carries a flag that asserts it correct,
which `Tcp::checkChecksum` then believes without arithmetic (the CHECKSUM_DECLARED_CORRECT
branch, Tcp.cc:527). The computed mode is correct; the run shows host B, told to compute,
carrying exactly the value the document defines.

**Why it is a gap.** RFC 9293 is stronger here than RFC 768 is for UDP: the checksum is
never optional, and there is no mode in which a TCP sender may leave it out. A host in its
default state sends segments that carry no checksum at all, and two INET hosts accept each
other only because both consult the same flag.

**The same finding as UDP gap 1, one layer up.** The UDP pass found the same default and the
same placeholder mechanism (`doc/project/evidence/model/udp/results.md`). The two share a
cause and a correction. They differ in what the standard allows: RFC 768 lets a sender
generate no checksum, so the UDP default is a wrong value where a lawful one exists;
RFC 9293 allows no such thing.

**Not a defect of the module.** Five checks of this pass depend on the computed mode and
pass. What fails is the choice of default.

## Model gap 3 (pass 3): new data goes past a shrunk window edge

**Statement.** [RFC9293-WND-5](../../standard/rfc9293/catalog.md#rfc9293-wnd-5), should not:
"If this happens, the sender SHOULD NOT send new data (SHLD-15)", §3.8.6,
`rfc9293.txt:2155`, where "this" is the usable window becoming negative.

**What the model does.** The relay shrank one acknowledgment to a window of 100 octets. The
run shows host A's TCP reading it at t=0.2002652, with the acknowledgment number at 26608,
so the new right edge is 26708. At t=0.20032412, in the same moment of simulated time, host
A's TCP decided to send a further full segment at sequence 27144, which carries 536 octets
and starts 436 octets beyond the edge. The segment is new data, not a retransmission: the
sequence number had never been sent before.

**Why it is a gap.** The rule exists because a peer that shrank its window has less room
than it promised, and data sent past the new edge is data the peer must throw away. The
level 2 check of the same feature passed: with a window that only grows, the model stays
inside it (RFC9293-WND-2). The rule breaks only when the edge moves backward, which is what
level 3 is for.

**What holds.** The stronger half of the same event holds. `Rfc9293ShrunkWindow.test` shows
the connection surviving the negative window and finishing the whole transfer, which is
MUST-34. The two are separate tests on purpose, so that this failure cannot hide that
verdict.

## Model gap 4 (pass 3): a Source Quench stops the run

**Statement.** [RFC9293-ICMP-2](../../standard/rfc9293/catalog.md#rfc9293-icmp-2), must:
"TCP implementations MUST silently discard any received ICMP Source Quench messages
(MUST-55)", §3.9.2.2, `rfc9293.txt:2841-2842`.

**What the model does.** It stops the simulation:

```
Error: Unknown ICMP type 4 -- in module (inet::Icmp) Rfc9293IcmpNet.host1.ipv4.icmp
(id=48), at t=0.20016446s, event #84
```

`Icmp::processIcmpMessage` switches on the type, and the default branch throws
`cRuntimeError("Unknown ICMP type %d")` (src/inet/networklayer/ipv4/Icmp.cc). TCP is never
offered the message, so the rule that TCP must discard it is never reached.

**Why it is a gap.** A silent discard leaves the host running. RFC 9293 keeps this rule
because a host still meets the message: RFC 6633 deprecated Source Quench for routers, and
what is deprecated for senders is still received by hosts.

**The same branch as an IPv4 finding.** `tests/protocol/ipv4/Rfc1122UnknownIcmpType.test`
reaches the same default branch with type 42, a number that names no message
([ipv4/results.md](../ipv4/results.md)). This test shows that the branch is reached by a type
the standards still describe, and that a TCP connection is what meets it. One correction in
the ICMP module closes both.

## Deviations between the English observations and the test steps

1. **Connection establishment, observation 1 observed at the link layer.** The TCP checksum
   is inserted by an IPv4 post-routing hook that the TCP module creates when the computed
   mode is on ([Tcp.cc:67-69](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L67-L69),
   `TcpChecksumInsertionHook`). At the TCP module's own send signal the field is still
   zero, so the nonzero-checksum condition is checked at `host1.eth[0].mac`, which is the
   wire.
2. **Flow control, a sink instead of an echo on host B.** An echo application piggybacks
   its acknowledgment on the data it returns, which removes the acknowledgment delay the
   check relies on. The close time is also dropped from that scenario: 3000 octets through
   a 300-octet window at one round per 0.2 s take about 2 s.
3. **Data transfer, observation 3 requires data.** A pure acknowledgment that host A sends
   while its next sequence number is `ISS_A + 537` carries that same number; the step now
   requires a positive data length. Found in review, after the agent-written step had
   passed on a segment that did carry data.
4. **A tester defect, worked around.** Capturing a unit-bearing field (the SYN's header
   length) makes the tester's capture substitution fail on the next step with `Attempt to
   use the value '24B' as a dimensionless number`
   (`tests/protocol/lib/EventPattern.cc`, `formatCaptureValue`, which substitutes every
   stored capture through `intValue()`). The test captures the number without its unit
   through a lambda. Tooling, not model.

## Scenario settings beyond the templates

- `**.checksumMode = "computed"` in every test. The default,
  [Tcp.ned:189](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L189), is `"declared"`,
  a placeholder; the TCP checksum is never optional (MUST-2), so the computed mode is the
  conformant configuration.
- `*.host2.tcp.advertisedWindow = 300` and `*.host2.tcp.delayedAcksEnabled = true` in the
  flow-control test. The delay is the fixed constant
  [TcpBaseAlg.cc:32](../../../../../src/inet/transportlayer/tcp/flavours/TcpBaseAlg.cc#L32),
  0.2 s, which is what the check document assumes and within MUST-40.
- `sendBytes = 5000B` (data transfer, push) and `3000B` (flow control); `connectPort = 7000`
  with `*.host2.numApps = 0` (reset).

Pass 3 adds:

- `sim-time-limit = 10s` wherever a check takes a segment away. Host A has one segment in
  flight when the loss happens, so no duplicate acknowledgment can arrive and the recovery
  waits for the retransmission timeout, which starts at three seconds.
- `*.host2.app[0].echoFactor = 0`, so that the reverse direction carries acknowledgments
  only. An observation about a segment from host B cannot then be confused with an echo.
- `**.tcp.checksumMode = "computed"` and not `**.checksumMode`, in the two ICMP tests. The
  wildcard would also reach the ICMP module, whose declared mode is what lets the relay
  change the type of a message without computing an ICMP checksum, exactly as
  tests/protocol/ipv4/Rfc1122UnknownIcmpType.test relies on.
- `Rfc9293ChecksumDefault.test` says nothing about host A's checksum on purpose. The absence
  of the setting is the subject of that check.

## Failure history during authoring

Pass 2: three test errors and one model gap. The errors: a `notBefore` copied from the old
template onto a step whose anchor had moved; the echo application in the flow-control
scenario (deviation 2); the unit-bearing capture (deviation 4). The review added the data
condition of deviation 3. The model gap is the PSH bit, above.

Pass 3: five test errors and three model gaps. Nothing was reverted, skipped or softened;
each error was a fault of the test, and each was corrected before the verdict was recorded.

1. **The header chunk lives in its own namespace.** `TcpHeader` is `inet::tcp::TcpHeader`,
   and the helper file must say so or nothing compiles.
2. **A relay that removed the payload removed the trailer with it.** The first
   `makeReset` read the length of what followed the header, which is the payload **and** the
   link-layer trailer. The frame that left was malformed and the receiver never saw it. The
   helper now takes the payload length from the IPv4 total length.
3. **A predicate that reads IPv4 fields cannot run at a transport module.** The empty
   acknowledgment of the out-of-window check is recognised by its IPv4 total length, and TCP
   hands a segment down before that header exists. The observation moved to the interface.
4. **An interface reports a send when the transmission ends.** The first version of the
   shrunk-window check watched host A's interface, which can report a send that was decided
   before the acknowledgment arrived. The watch moved to the module, where the decision is
   made. The verdict did not change, but it would not have been trustworthy.
5. **A relay holds one rule at a time.** The Source Quench check needs two changes, one on
   the way out and one on the way back. The first version put both on one relay, and the
   second silently replaced the first: the tap reported `mutated 0`. Two relays in series
   fixed it. The lesson was already in the IPv6 notes; it is here now for TCP as well.

The last error is worth naming twice. Before it was found, the check failed for the reason a
reader would have expected — and the description in the test file already claimed the crash
that the corrected version does produce. The claim was removed until the run showed it. A
test that fails for the wrong reason teaches nothing, and a description written ahead of the
evidence is how that goes unnoticed.

## Model analysis — where INET implements the checked behavior

- **Handshake, sequence space (EST-1, EST-2, SEQ-1):** `sendSyn`
  [TcpConnectionUtil.cc:736](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L736)
  and `sendSynAck`
  [:779](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L779), each with
  `snd_nxt = iss + 1` ([:751](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L751),
  [:790](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L790)). The initial
  sequence number: [:673-676](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L673-L676).
- **MSS option, header length (OPT-1, HDR-1):** written into the SYN in
  `writeHeaderOptions`,
  [TcpConnectionUtil.cc:1454-1465](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1454-L1465);
  20 octets plus the 4-octet option, 24 observed.
- **Segmentation (SEG-1):** `sendSegment` caps a segment at `snd_mss` less the option
  length, [TcpConnectionUtil.cc:983-984](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L983-L984);
  `sendData` sends whole segments of the effective MSS,
  [:1094-1106](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1094-L1106).
  Observed: 536 octets in the first segment.
- **Cumulative acknowledgment, delivery (ACK-1, DATA-1):** `rcv_nxt` advances over
  in-order data at
  [TcpConnectionRcvSegment.cc:500](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L500);
  delivery in `sendAvailableDataToApp`,
  [TcpConnectionUtil.cc:566-577](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L566-L577).
  Observed: the acknowledgment of `ISS_A + 5001`.
- **Send window (WND-2):** `sendData` bounds new data by
  `min(snd_wnd, cwnd) - (snd_nxt - snd_una)`,
  [TcpConnectionUtil.cc:1075-1078](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1075-L1078).
  The initial congestion window is one segment,
  [TcpBaseAlg.cc:149](../../../../../src/inet/transportlayer/tcp/flavours/TcpBaseAlg.cc#L149),
  so a 300-octet receiver window is the binding limit. Observed: a 300-octet first segment,
  silence for 0.15 s, the delayed acknowledgment at 0.2 s, and the resume at the edge.
- **Advertised window (WND-1):** `updateRcvWnd`,
  [TcpConnectionUtil.cc:1650-1670](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1650-L1670),
  with the receiver's silly-window floor at
  [:1660-1661](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L1660-L1661).
  Observed: 300 in the SYN-ACK and in the acknowledgments.
- **Checksum (CKSUM-1):** inserted by the hook,
  [Tcp.cc:43-77](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L43-L77); verified in
  `checkChecksum`, [Tcp.cc:496-535](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L496-L535),
  and a failing segment is dropped with reason `INCORRECTLY_RECEIVED`,
  [Tcp.cc:149-156](../../../../../src/inet/transportlayer/tcp/Tcp.cc#L149-L156). That drop
  is RFC9293-CKSUM-2, level 3.
- **Close (FIN-1, FIN-2):** `sendFin`
  [TcpConnectionUtil.cc:922](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L922);
  `rcv_nxt` over the FIN,
  [TcpConnectionRcvSegment.cc:627-630](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L627-L630).
- **Reset for a closed port (RST-1):** `segmentArrivalWhileClosed`,
  [TcpConnectionRcvSegment.cc:62-75](../../../../../src/inet/transportlayer/tcp/TcpConnectionRcvSegment.cc#L62-L75):
  `ackNo = SEG.SEQ + data length + SynFinLen`, sequence number 0, through `sendRstAck`
  ([TcpConnectionUtil.cc:853-863](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L853-L863)).
  The connecting side goes to CLOSED on the reset, with no retry,
  [TcpConnectionBase.cc:457-464](../../../../../src/inet/transportlayer/tcp/TcpConnectionBase.cc#L457-L464).

## Model observations the checks did not claim

1. **The default mode has no flow control, by the model's own words.**
   [Tcp.ned:53-58](../../../../../src/inet/transportlayer/tcp/Tcp.ned#L53-L58): in the
   default "autoread" mode "the advertised window never decreases so there is effectively
   no flow control". The flow-control check passed because it tests the sender's respect
   for a fixed small window, which does not need the window to close. A closing window
   needs the "explicit-read" mode that the same note names, and which the applications
   expose as `autoRead = false`; that is the next flow-control check.
2. **The checksum is not computed by default**, and when it is, it is computed one layer
   down. Observers at the TCP module see a zero field (deviation 1).
3. **The initial sequence number** meets MUST-8 — the 4-microsecond clock — and not SHLD-1,
   the pseudorandom term
   ([TcpConnectionUtil.cc:676](../../../../../src/inet/transportlayer/tcp/TcpConnectionUtil.cc#L676)),
   as the earlier pass recorded.
4. **Urgent data is unsupported too.** The same comments that mark PSH unsupported name URG
   with it. RFC 9293 requires the mechanism (MUST-30 to MUST-32) and discourages its use; it
   is a level 5 candidate for a `defect`.

## Feature support

The verdicts above feed the support column and the achieved level of the coverage ledger,
[`coverage.md`](coverage.md#feature-support).

## Sharpening candidates for the next pass

The level 3 line below is done: every item of it is a test of pass 3.

- **Level 3, done:** RFC9293-CKSUM-2 with a corrupted segment; a reset on a live connection
  and the acceptance rules for a received reset; a shrunk window; ICMP errors on a
  connection.
- **Level 3, what remains:** RFC9293-ICMP-4, whether a hard error should abort a
  synchronized connection. The model decides deliberately not to, citing blind reset attacks
  (TcpConnectionUtil.cc), and RFC 9293 itself notes that implementations differ. A check
  would record a `declined` with a reason, which is worth having but settles nothing.
- **Level 4:** RFC 5681 and RFC 6298 in the in-scope set; the zero-window probe
  (RFC9293-ZWP-1) and the acknowledgment delay bound (RFC9293-ACKD-1) as statistical
  checks; the TIME-WAIT duration.
- **A closing window** with `autoRead = false` on the receiver, for the half of flow
  control the default mode cannot show.
- **RFC9293-SEQ-2** on the existing mockup; **RFC9293-ISS-2** split into its two halves.
- **The tester:** substitute captures without forcing them to dimensionless integers, so a
  unit-bearing field can be captured directly.
