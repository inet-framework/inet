# Repair the defects the standards passes found

**Status:** in progress. Started 2026-09-14 on `topic/protocol-model-defects`, in the
worktree `/home/levy/workspace/inet-protocol-model-defects`.

**Where it stands:** group A is done. 6 of the 30 tests pass, from three repairs, and a
seventh that had declared an expected failure passes too. The suite is 257 tests, 188 PASS,
44 FAIL (expected), 25 FAIL (unexpected), from 182 / 45 / 30. No input in the protocol suite
stops a simulation any more.

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

### Group B — a condition missing from a mechanism that exists (3 tests)

- [ ] `ipv4/Rfc1122NoErrorForLinkBroadcast`
- [ ] `ipv6/Rfc4443NoErrorForLinkBroadcast`
- [ ] `ipv6/Rfc4443NoErrorForLinkMulticast`

### Group C — a value that is never filled in (2 tests)

One of the two, the fragment payload length, turned out to be the same defect Group A
repaired, but on the receive side. `ipv6/Rfc8200FragmentPayloadLength` is about the send
side and still fails.

- [ ] `ipv6/Rfc4443PacketTooBigMtu` — the caller hands `createPacketTooBigMsg` a literal 0.
- [ ] `ipv6/Rfc8200FragmentPayloadLength` — every fragment carries the copied length.

### Group D — an incomplete key, field or check (12 tests)

- [ ] `ipv4/Rfc791SameIdDifferentProtocol` — the reassembly key holds three of the four
      fields RFC 791 names.
- [ ] `ipv4/Rfc1122ChecksumDiscard` — the guard short-circuits, so a well-formed header never
      reaches the checksum test.
- [ ] `arp/Rfc5494ExperimentalOpcode`, `arp/Rfc5494ExperimentalHardwareSpace`,
      `arp/Rfc826UnknownProtocolSpace`
- [ ] `quic/Rfc9000ServerInitialSize`, `quic/Rfc9000UnknownFrameType`
- [ ] `tcp/Rfc6298FirstMeasurement`, `tcp/Rfc9293ShrunkWindowNoNewData`,
      `tcp/Rfc9293ChecksumDefault`
- [ ] `udp/Rfc1122ChecksumDefault`, `udp/Rfc768EmptyDatagram`

### Group E — DHCP (8 tests, 14 gaps)

The heaviest suite: 8 of its 26 tests fail. One of the fourteen, gap 11
(`Rfc2131DuplicateAddressDeclined`), is recorded as an **untestable claim** and not a
defect, so it may not belong here at all. Read the gap list before starting.

- [ ] Read `model/dhcp/notes.md` and split the eight into what is one defect and what is
      several.

## Not in this plan

The wifi suite's 36 `NOT-MODELED` findings. Each names a feature the model does not
implement, which is a different kind of work: writing a feature, not repairing one.
