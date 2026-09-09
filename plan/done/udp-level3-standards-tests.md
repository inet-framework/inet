# UDP standards tests — level 3

> **Kind:** how · **Status:** done · **Stands on:**
> [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md),
> [udp/standards.md](../../doc/project/evidence/protocol/udp/standards.md)

The UDP pass reached level 2 against RFC 768 and RFC 792. This plan takes it to level 3.
The UDP standards map names what level 3 adds: RFC 1122 §4.1, the host requirements for
UDP. The work is on `master`, in the worktree `inet-master`.

## What level 3 adds

RFC 1122 §4.1 gives UDP the rules that RFC 768 leaves out:

- §4.1.3.4 — a host must generate and check the checksum; a host must silently discard a
  datagram whose checksum is non-zero and wrong; the default is to checksum.
- §4.1.3.1 — UDP should send an ICMP Port Unreachable for a port with no listener. This
  raises the strength of the RFC 792 statement that level 2 already carries.
- §4.1.3.3 — UDP must pass every ICMP error message up to the application.
- §4.1.3.6 — a datagram with a wrong IP source address is discarded; a host sends only its
  own address as the source.
- §4.1.3.2, §4.1.3.5, §4.1.4 — the interface between UDP and the application.

The level 3 toolset is the `PacketTap`. It puts a wrong checksum, a wrong payload and a
wrong source address on the wire, which no application can do.

## Steps

- [x] **Step 3 — catalog.** Add a UDP part to `standard/rfc1122/catalog.md` for §4.1. The
      catalog holds §3 today, from the IPv4 pass. Change its scope statement and its title.
      Add the `Overridden by` field to the RFC 768 entries that RFC 1122 restates.
- [x] **Step 4 — features.** Extend `protocol/udp/features.md`. The checksum feature
      changes level, because RFC 1122 makes it mandatory. Add a feature for input
      validation.
- [x] **Step 5 — checks.** Add the English procedures to `protocol/udp/checks.md`.
- [x] **Step 6 — tests.** Write `tests/protocol/udp/UdpMutations.h` and the new tests.
- [x] **Step 7 — results.** Record every verdict in `model/udp/results.md`.
- [x] **Step 8 — conformance and ledger.** Update `model/udp/conformance.md`,
      `coverage.md`, `categories.md` and `notes.md`.
- [x] **Step 9 — run and commit.** Build, run the suite, check the links, commit.

## The tests

Ten tests, and the verdict each one reached:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc1122ChecksumDiscard | RFC1122-UCK-4 | PASS |
| Rfc768ChecksumCoversData | RFC768-CKSUM-1, RFC1122-UCK-4 | PASS |
| Rfc768ChecksumCoversPseudoHeader | RFC768-CKSUM-1, RFC1122-UCK-4 | PASS |
| Rfc768ZeroChecksumAccepted | RFC768-CKSUM-2, RFC1122-UCK-5 | PASS |
| Rfc1122ValidSourceAddress | RFC1122-UADDR-2 | PASS |
| Rfc1122ApplicationTtlAndTos | RFC1122-UAPI-1 | PASS |
| Rfc1122ApplicationSourceAddress | RFC1122-UMH-2 | PASS |
| Rfc1122ChecksumDefault | RFC1122-UCK-3 | FAIL, gap 1: the default mode writes a placeholder |
| Rfc1122MulticastSourceAddress | RFC1122-UADDR-1 | FAIL, gap 2: no layer validates the source address |
| Rfc768EmptyDatagram | RFC768-HDR-3, the minimum | FAIL, gap 3: a result filter stops the run |

Two tests were added during the work and are not in the list above as first planned. The
empty-datagram test closes a question the level 2 pass wrote down and left for level 3. The
two application-interface tests were added after the parameters of the sending program
showed that a program can name a TTL, a TOS and a source address.

## Facts found during the work

- The model checks the checksum arithmetic only when `checksumMode` is `"computed"`
  (`Udp::verifyChecksum`). The default is `"declared"`, which writes the constant 0xC00D.
- A `"declared"` header cannot be serialized. `UdpHeaderSerializer::serializeFields` throws.
  The helper must catch this, so that a check gives a verdict and not a crash.
- The model discards a datagram with a wrong checksum without an answer
  (`Udp::processUdpPacket`), which is what §4.1.3.4 demands.
- The model passes an ICMP error to the application as an `Indication`, not as a packet
  (`Udp::sendUpErrorIndication`). The test framework watches packet signals, so no test can
  see it. The ledger records the reason.

## Result

Level 3 partial. 13 tests in the suite: 10 PASS and 3 declared FAIL, each naming a model gap.
Nine statements of RFC 1122 §4.1 have no check, all of them at the interface between UDP and
a program, which is what keeps the level partial. The full account is in
`doc/project/evidence/model/udp/`.

