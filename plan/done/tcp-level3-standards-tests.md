# TCP standards tests — level 3

> **Kind:** how · **Status:** done · **Stands on:**
> [derive-tests-from-a-standard.md](../../doc/project/guide/derive-tests-from-a-standard.md),
> [tcp/standards.md](../../doc/project/evidence/protocol/tcp/standards.md)

The TCP pass reached level 2 against RFC 9293. This plan takes it to level 3. The standards
map says level 3 needs **no new document**: RFC 9293 replaced RFC 793 and the family that
updated it, so the edge rules are already in the document that is in scope. The work is on
`master`, in the worktree `inet-master`.

## What level 3 adds

The level 2 ledger named the four areas, and the RFC holds the text for each:

- §3.1 — the receiver must check the checksum (MUST-3). Level 2 checked only that a segment
  carries one.
- §3.5.2, §3.10.7.4 — an unacceptable segment on a live connection draws an empty
  acknowledgment, never a reset, and the connection stays as it was.
- §3.5.3 — a reset is valid only when its sequence number is in the window. A valid one
  aborts the connection; an invalid one is ignored. This is what stops a blind reset.
- §3.8.6 — a receiver should not shrink the window, and a sender must be robust when one
  does (MUST-34).
- §3.9.2.2 — a soft ICMP error must not abort a connection (MUST-56).

The level 3 toolset is the `PacketTap`. It puts a corrupt checksum, an out-of-window
sequence number, a crafted reset and a shrunk window on the wire. No program can do that.

## Steps

- [x] **Step 3 — catalog.** Add the edge statements of RFC 9293 to
      `standard/rfc9293/catalog.md`, and take each one out of the out-of-scope list.
- [x] **Step 4 — features.** Reset and checksum gain statements; add a feature for the
      acceptance of a segment and one for the ICMP path.
- [x] **Step 5 — checks.** Split `protocol/tcp/checks.md` into one file per feature, as
      IPv4 and UDP are, and add the new procedures.
- [x] **Step 6 — tests.** Write `tests/protocol/tcp/TcpMutations.h` and the new tests.
- [x] **Step 7 — results.** Record every verdict in `model/tcp/results.md`.
- [x] **Step 8 — conformance and ledger.** Update the four model documents.
- [x] **Step 9 — run and commit.** Build, run every suite, check the links, commit.

## The tests

Eleven tests, and the verdict each one reached:

| Test | Checks | Verdict |
| --- | --- | --- |
| Rfc9293ChecksumDiscard | CKSUM-2 | PASS |
| Rfc9293OutOfWindowSegment | SEGA-1, SEGA-2, RST-2 | PASS |
| Rfc9293BlindReset | RSTP-1 | PASS |
| Rfc9293ValidReset | RSTP-2 | PASS |
| Rfc9293NoResetForReset | RST-3 | PASS |
| Rfc9293ShrunkWindow | WND-4 | PASS |
| Rfc9293NoWindowShrink | WND-3 | PASS |
| Rfc9293SoftIcmpError | ICMP-3, ICMP-1 | PASS |
| Rfc9293ChecksumDefault | CKSUM-1 (the value) | FAIL, gap 2: the default mode writes no checksum |
| Rfc9293ShrunkWindowNoNewData | WND-5 | FAIL, gap 3: new data goes past the shrunk edge |
| Rfc9293SourceQuench | ICMP-2 | FAIL, gap 4: the run stops on an unknown ICMP type |

Three tests were added during the work and are not in the list as first planned. The window
check split in two, because the failure of the weaker rule would have hidden the verdict on
the stronger one. The no-window-shrink check was added when it became clear that a receiver
can be compared with itself and needs no relay. The Source Quench check was added when the
soft-error mockup turned out to give it its vehicle for nothing.

## Facts found before the work

- `Tcp::checkChecksum` sums the pseudo header, the header and the data only in the computed
  mode. The `checksumMode` parameter is `default("declared")` (Tcp.ned:189), which is the
  same placeholder the UDP pass found. The enum has no disabled value, because RFC 9293 says
  the checksum is never optional.
- A segment that fails the check is dropped in silence, with the header still at the front
  (Tcp.cc:149-156), so a drop record can be filtered by port. UDP needed a length instead.

## Result

Level 3 reached, the first protocol in this tree to reach it rather than level 3 partial.
19 tests in the suite: 15 PASS and 4 declared FAIL. Every `must` and `must not` of the
in-scope set has a verdict; the one statement without a check, RFC9293-ICMP-4, carries a
`should`. The full account is in `doc/project/evidence/model/tcp/`.

## Facts found during the work

- A relay holds one rule at a time: `ProtocolTester` calls `configure` once per intercept
  clause and each call replaces the last. Two changes need two relays in series.
- At a tap the frame carries the link-layer trailer behind the payload. A helper that removes
  "the rest" removes the trailer too and the frame is dropped before any protocol sees it.
- An interface reports a send when the transmission ends. A rule about what a sender does
  after it learns something must be watched at the module.
- A report from the network is better made than forged: set the time to live to 1 and let a
  real gateway write the ICMP message. The quoted header then matches the connection with no
  care from the test.

