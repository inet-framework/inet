# Repair the defects the standards passes found

**Status:** done 2026-09-17, and landed on master. Started 2026-09-14 on
`topic/protocol-model-defects`. The two findings that stay open are blocked by serialization and
not by this work; the TCP and UDP results record them as defects. The baselines that the repairs
move are in [protocol-model-defects-baselines.md](protocol-model-defects-baselines.md).

**Where it stands:** **28 of the 30 tests pass.** Two remain, and both have one cause: a
transport checksum cannot be computed by default until every payload in the tree serializes.

| Test | Why it is still open |
| --- | --- |
| `udp/Rfc1122ChecksumDefault` | Deferred. QUIC has no serializer for its packet headers, so a computed UDP checksum stops 10 of 11 QUIC tests. |
| `tcp/Rfc9293ChecksumDefault` | Withdrawn on 2026-09-15. A computed TCP checksum serializes the segment, the payload loses its region tags, and the lwIP interop example stops. |

The suites, before this plan and on 2026-09-16 after the rebase onto master `b0c7a25e75`:

| Suite | Then | Now |
| --- | --- | --- |
| arp | 13 PASS, 3 unexpected | **16 PASS** |
| dhcp | 13 PASS, 5 expected, 8 unexpected | **21 PASS, 5 expected** |
| ipv4 | 16 PASS, 2 expected, 4 unexpected | **20 PASS, 2 expected** |
| ipv6 | 19 PASS, 8 unexpected | **27 PASS** |
| quic | 8 PASS, 1 expected, 2 unexpected | **10 PASS, 1 expected** |
| tcp | 22 PASS, 2 expected, 3 unexpected | **25 PASS, 1 expected, 1 unexpected** |
| udp | 10 PASS, 1 expected, 2 unexpected | **11 PASS, 1 expected, 1 unexpected** |

element (43), ethernet (1) and self (21) pass in full, and wifi is 39 PASS with 35 expected.

**The rebase of 2026-09-16** brought 89 master commits and one conflict-free overlap,
`DhcpMessage.msg`. It found two things. Master's ICMPv6 dissector change `5c7fc97b3f`
serialized a header to read its type, so an unknown type no longer dissected and both
`Rfc4443Unknown*Type` tests missed their first step; the dissector is repaired first on this
branch. The contract gate also showed that the probe commit gave `IArp::sendArpProbe` an
empty body, which AR-ORG-CONTRACT-PURITY forbids; the method is pure virtual now,
`GlobalArp` holds the empty override, and the migration guide says what an outside
implementation must add.

**A second rebase the same day**, onto `b0c7a25e75`, brought 9 more master commits. Only
`WHATSNEW` overlapped: master added entry 4 to the same list, so the entries of this branch
are numbered 5 to 16 now. No verdict moved.

A note on counting: an earlier figure of 257 tests was wrong. It came from a verdict list
keyed by test name, and two suites use the same name for different tests. The runner's own
per-suite totals are the ones to read.

Seven standards passes measured the model and did not repair it, because a pass measures.
Thirty tests fail for recorded reasons. This plan repairs the model so that they pass.

## The rule this work follows

**Fix the model, never the test.** A failing standards test is a finding. The proof of a
repair is that the test turns green with no edit to the test. If a test must change, the
change is a finding about the test, and it is recorded and explained before it is made. A
test is never softened, skipped, or given `%# expected-result: FAIL`; that declaration is
only for a feature the model does not implement, and none of these thirty is one.

**Every repair carries its evidence.** A verdict lives in `doc/project/evidence/model/<p>/`.
A repair that turns a test green moves `results.md` and the gap list in `notes.md` with it,
in the same commit as the source change.

## The obligation this work owes

**A model repair moves fingerprints.** These are behaviour changes in the network layer, the
transport layer and ICMP, so `tests/fingerprint/*.csv` and possibly `tests/statistical/`
move. `CR-OBL-BASELINE` says the author names which, or says none move, or says `?`. Silence
is the most common blocking finding in the audit reports.

Every commit here answers it. Where a baseline moves, the run that produced the new value is
named. A repair whose baselines are not yet run says `?` and does not pretend.

**The answer, measured on 2026-09-17: fingerprints and statistical results move, and so
does the expected output of nine module tests.** An A/B run on 2026-09-16 reported that
nothing moves, and it was wrong. [protocol-model-defects-baselines.md](protocol-model-defects-baselines.md)
holds the measurement, and each commit that moves a recorded value carries the new value and
the reason.

## The order

By how clear the defect is, not by protocol.

### Group A — the crashes (5 tests) — DONE 2026-09-14

A peer can stop the simulation with a packet. These are the worst of the thirty and the
least arguable.

- [x] `ipv4/Rfc1122UnknownIcmpType` — done. The `default:` branch discards.
- [x] `ipv6/Rfc4443UnknownInformationalType` — done, the same repair.
      It also closed `tcp/Rfc9293SourceQuench`, which had declared an expected failure:
      Source Quench is type 4, which fell into the same branch.
- [x] `ipv6/Rfc4443ErrorForUnknownProtocol` — done. `Icmpv6` kept the set of registered
      transport protocols and never read it; `Icmp` had always read its own.
- [x] `ipv6/Rfc8200AtomicFragment` — done. It also uncovered the payload length defect
      below, and a step of the test that could never have matched.
- [x] `ipv6/Rfc8200OverlappingFragments` — done, closed by the payload length repair.
      That repair reaches every IPv6 reassembly: an ordinary two-fragment datagram was
      being truncated silently to the first fragment's length.

### Group B — a condition missing from a mechanism that exists (3 tests) — DONE 2026-09-14

- [x] `ipv4/Rfc1122NoErrorForLinkBroadcast`
- [x] `ipv6/Rfc4443NoErrorForLinkBroadcast`
- [x] `ipv6/Rfc4443NoErrorForLinkMulticast`

### Group C — a value that is never filled in (2 tests) — DONE 2026-09-14

One of the two, the fragment payload length, turned out to be the same defect Group A
repaired, but on the receive side. `ipv6/Rfc8200FragmentPayloadLength` is about the send
side and still fails.

- [x] `ipv6/Rfc4443PacketTooBigMtu` — done. `sendErrorMessage` takes an MTU now.
- [x] `ipv6/Rfc8200FragmentPayloadLength` — done. Each fragment carries its own length.

### Group D — an incomplete key, field or check (12 tests)

- [x] `ipv4/Rfc791SameIdDifferentProtocol` — done. The key carries the protocol.
- [x] `ipv4/Rfc1122ChecksumDiscard` — done. The guard is an OR.
- [x] `arp/Rfc5494ExperimentalOpcode`, `arp/Rfc5494ExperimentalHardwareSpace`,
      `arp/Rfc826UnknownProtocolSpace` — done. Two crashes a neighbour controlled.
      The two RARP branches still throw and no test covers them; the ARP results say so.
- [x] `quic/Rfc9000UnknownFrameType` — done. Three stops, each hidden behind the one before
      it: the frame dispatcher, the frame loop and the datagram loop.
- [x] `quic/Rfc9000ServerInitialSize` — done, and a step of the test was wrong: two `once`
      steps on one datagram, so the second could never match.
- [ ] `tcp/Rfc9293ChecksumDefault` — **withdrawn on 2026-09-15.** The one-word default change
      passed the protocol suite, and the fingerprint run found what the suite could not: a
      computed checksum serializes the segment, the copy loses its region tags, and
      `examples/inet/nclients -c lwip__inet` stops. The same block as the UDP default.
- [x] `tcp/Rfc6298FirstMeasurement` — done. The first measurement has RFC 6298 section 2.2's
      case of its own.
- [x] `tcp/Rfc9293ShrunkWindowNoNewData` — **a test error, not a defect.** The model refuses
      the send and says so; the peer reopens the window 59 microseconds later and the watch
      ran for 0.3 seconds, catching a lawful send.
- [x] `udp/Rfc768EmptyDatagram` — done. Four result filters stopped the run on a packet of
      zero length, which RFC 768 allows. One step of the test changed: a relay that shortens
      a frame leaves one no dissector will read.
- [ ] `udp/Rfc1122ChecksumDefault` — **blocked, and the block is the finding.** Changing the
      default to `computed` was tried on 2026-09-15 and reverted. Computing a UDP checksum
      serializes the payload, and QUIC has no serializer for its packet headers, so 10 of 11
      QUIC tests stop and two others fall with them. The test does not pass either way. UDP
      cannot default to a computed checksum until every payload in the tree serializes.

### Group E — DHCP (8 tests, 14 gaps) — DONE 2026-09-15

- [x] Seven defects repaired: gaps 1, 2, 3, 4, 8, 10, 13 and 14. The transaction identifier,
      three field values the server had no right to write, a domain name server option with
      no value, the silent answer to a foreign subnet, and the whole retransmission strategy.
      Repairing gap 14 also corrected a step of its test, an ordered `never` that consumed
      the window the repeated request needed.
- [x] `dhcp/Rfc2131DuplicateAddressDeclined` — done. Gap 11 was an **untestable claim**: the
      decline was written and nothing could provoke it. The client now probes the granted
      address (RFC 5227) and declines it if somebody answers, and the server marks a declined
      address unavailable, which stopped an endless decline loop.

## Not in this plan

The wifi suite's 36 `NOT-MODELED` findings. Each names a feature the model does not
implement, which is a different kind of work: writing a feature, not repairing one.
