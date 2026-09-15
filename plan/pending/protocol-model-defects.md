# Repair the defects the standards passes found

**Status:** in progress. Started 2026-09-14 on `topic/protocol-model-defects`, in the
worktree `/home/levy/workspace/inet-protocol-model-defects`.

**Where it stands:** groups A, B and C are done, and group D has 3 tests left of 12. **17 of
the 30 tests pass**, from eleven repairs, and one that had declared an expected failure
passes too.

Three suites are complete, with no unexpected failure left:

| Suite | Then | Now |
| --- | --- | --- |
| arp | 13 PASS, 3 unexpected FAIL | **16 PASS** |
| ipv4 | 16 PASS, 2 expected, 4 unexpected | **20 PASS, 2 expected** |
| ipv6 | 19 PASS, 8 unexpected FAIL | **27 PASS** |

What is left: dhcp 8, tcp 2, quic 2, udp 1.

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
- [ ] `quic/Rfc9000ServerInitialSize`, `quic/Rfc9000UnknownFrameType`
- [x] `tcp/Rfc9293ChecksumDefault` — done. The same one-word default change is clean for
      TCP, and it moves every TCP fingerprint.
- [ ] `tcp/Rfc6298FirstMeasurement`, `tcp/Rfc9293ShrunkWindowNoNewData`
- [x] `udp/Rfc768EmptyDatagram` — done. Four result filters stopped the run on a packet of
      zero length, which RFC 768 allows. One step of the test changed: a relay that shortens
      a frame leaves one no dissector will read.
- [ ] `udp/Rfc1122ChecksumDefault` — **blocked, and the block is the finding.** Changing the
      default to `computed` was tried on 2026-09-15 and reverted. Computing a UDP checksum
      serializes the payload, and QUIC has no serializer for its packet headers, so 10 of 11
      QUIC tests stop and two others fall with them. The test does not pass either way. UDP
      cannot default to a computed checksum until every payload in the tree serializes.

### Group E — DHCP (8 tests, 14 gaps)

The heaviest suite: 8 of its 26 tests fail. One of the fourteen, gap 11
(`Rfc2131DuplicateAddressDeclined`), is recorded as an **untestable claim** and not a
defect, so it may not belong here at all. Read the gap list before starting.

- [ ] Read `model/dhcp/notes.md` and split the eight into what is one defect and what is
      several.

## Not in this plan

The wifi suite's 36 `NOT-MODELED` findings. Each names a feature the model does not
implement, which is a different kind of work: writing a feature, not repairing one.
