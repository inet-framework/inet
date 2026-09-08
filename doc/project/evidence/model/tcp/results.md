# TCP checks — run results and model analysis (pass 2, level 2)

> **Kind:** report · **Status:** snapshot 2026-09-08 · **Seal:** none · **Owns:** — · **Stands on:** [catalog.md](../../standard/rfc9293/catalog.md), [checks.md](../../protocol/tcp/checks.md)

Step 7 artifact of the standards test workflow. This is the first document of the TCP
workflow that may reference code.

- Date: 2026-09-08. Tree: `inet-rfc-tests-tcp`, branch `topic/rfc-tests-tcp` on top of
  `topic/rfc-tests-ipv4`, source identical to `master`.
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

Summary: 8 tests, 7 PASS, 1 FAIL (expected), in 2.1 s. The runner's overall verdict is PASS,
because the one failure is declared. The SYN's header length was captured as 24 octets: the
MSS option is present, so the `should` of RFC9293-OPT-1 is met.

## The model gap: the PSH bit is never set

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

## Failure history during authoring

Three test errors and one model gap. The errors: a `notBefore` copied from the old template
onto a step whose anchor had moved; the echo application in the flow-control scenario
(deviation 2); the unit-bearing capture (deviation 4). The review added the data condition
of deviation 3. The model gap is the PSH bit, above.

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

- **Level 3:** RFC9293-CKSUM-2 with a corrupted segment; a reset on a live connection and
  the acceptance rules for a received reset; a shrunk window; ICMP errors on a connection.
- **Level 4:** RFC 5681 and RFC 6298 in the in-scope set; the zero-window probe
  (RFC9293-ZWP-1) and the acknowledgment delay bound (RFC9293-ACKD-1) as statistical
  checks; the TIME-WAIT duration.
- **A closing window** with `autoRead = false` on the receiver, for the half of flow
  control the default mode cannot show.
- **RFC9293-SEQ-2** on the existing mockup; **RFC9293-ISS-2** split into its two halves.
- **The tester:** substitute captures without forcing them to dimensionless integers, so a
  unit-bearing field can be captured directly.
