# IPsec level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-ipsec-level2-fixes`, on top of
`topic/standards-tests-ipsec-level2` at `6d57390964`. Worktree:
`/home/levy/workspace/inet-standards-tests-ipsec-level2-fixes`.

The level 2 pass of 2026-09-24 measured the model and repaired nothing. This branch repairs the
gaps of [`results.md`](../../doc/project/evidence/model/ipsec/results.md#the-model-gaps), one gap for
each commit, as the user asked on 2026-09-25: the defects, the untestable claim, and the small or
medium missing features. Tunnel mode (gap 8) and the path MTU (gap 12) are large, and get a plan
each and no code.

Commit group: `ipsec-model-repairs`. Each repair commit carries the source change, the test whose
declaration, configuration or gap paragraph changes, the WHATSNEW entry, and the fingerprint rows
that the change moves ([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)).
Statistical baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

The same as in `plan/pending/mpls-level2-fixes.md` of the MPLS repair branch: the build of
`inet-master` at `7772a7e4ef` (src `5c4f41c600`); the IPsec protocol tests in debug; the CI
fingerprint set in release (baseline 1752 tests equal, one expected error); the statistical tests
in the GitHub-job copy, filtered to the configurations that use IPsec for each commit, and all 930
at the tip of the branch (baseline: 27 unexpected failures, all of 802.11 EDCA results that the
statistics repository has not followed since `c0314ff5aa`).

## Steps

1. [x] **Plan** — this file.
2. [x] **Gap 1** — the first packet of an SA carries the Sequence Number 1. Tests:
   `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers`.
3. [x] **Gap 2** — the AH Payload Length in 32-bit words minus 2, and the dissector that reads it.
   Tests: `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6`.
4. [x] **Gap 3** — the AH ICV inside the header, before the payload, padded to 8 octets in IPv6.
   Tests: `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6`.
5. [x] **Gap 4** — the ESP padding octets 1, 2, 3 and so on. Test: `Rfc4303EspPadding`.
6. [x] **Gap 5** — the selector check of an inbound SA after AH or ESP processing. Test:
   `Rfc4301InboundSelectorCheck`.
7. [x] **Gap 6** — a PROTECT entry without a matching SA discards the packet. Test:
   `Rfc4301ProtectWithoutSa`.
8. [x] **Gap 7** — AH and ESP on one packet: an SA can override the Protection of its entry, and
   the ingress goes on to ESP after AH. Test: `Rfc4301AhAndEsp`, whose configuration changes.
9. [x] **Gap 9** — the lifetime of an SA, in seconds and in octets. Test: `Rfc4301SaLifetime`.
10. [x] **Gap 10** — the choice of an SA by DSCP. Test: `Rfc4301ParallelSas`.
11. [x] **Gap 11** — dummy packets. Test: `Rfc4303DummyPackets`.
12. [x] **Gaps 8 and 12, a plan each** — `plan/pending/ipsec-tunnel-mode.md` and
    `plan/pending/ipv6-path-mtu.md`.
13. [x] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
    `notes.md` ("Fixed on" entries) follow it. No statistics branch: nothing moves.
14. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **Gaps 2 and 3 went into one commit.** The AH dissector read the Payload Length as a count of
  payload octets, so the field alone could not change without breaking the dissection. The payload
  of AH is no longer an `EncryptedChunk`: AH does not encrypt.
- **Gap 7 uses a Protection element of the SA**, the rule "an SA inherits its properties from its
  parent policy" of `IPsec.ned` carried one step further, in place of the `AH_ESP` value that the
  documentation named and the enum never had. The sender applies ESP before AH whatever the order
  of the SAs; the receiver goes on to ESP after AH.
- **Gap 9 keeps the hard lifetime only.** Without key management no soft lifetime can start a
  replacement. An SA counts the bytes of the transport payload that it protects, on both sides.
- **Gap 11 needs a way out of the IPsec module.** IPv4 takes back only a datagram that a hook has
  queued, so the IPsec module got gates to the dispatcher of each network layer, as ICMP has, and
  the post-routing hook lets its own dummy packets pass. A dummy packet goes to the peer of the
  last packet of its SA, because the selectors of an SA often hold no address.
- **Each repair moved no fingerprint and no statistical result of `examples/inet/ipsec`.** The
  module test `IPsec_Ipv6` passes after the last repair.
- **The tests were edited by hand**, not with the generator of the level 2 pass: four of them got
  the configuration that the missing features lacked (the AH SA, the lifetimes, the DSCP values,
  the dummy packet interval).
- **Found on the way, not repaired**: the inbound ESP path schedules its delay on
  `lastProtectedOut`, the variable of the outbound queue, where the AH path uses
  `lastProtectedIn`. No test sees it, because the delays are 0 by default.
- **The whole branch moves no statistical result.** The full suite at `3b03623b11` gives 890 PASS
  and the same 27 unexpected failures as the baseline, all of 802.11 EDCA.
- **The documents follow a fresh run** at `3b03623b11`: 28 PASS, 3 FAIL declared expected (tunnel
  mode twice, the path MTU). The ledger and the matrix come from `audit/ipsec-level2-fixes/` of
  `inet-master`. The matrix script of the level 2 pass gave `out of claim` to every feature of a
  stated refusal; the guide gives `undocumented` to a supported one, and the creation of an SA is
  such a feature now, so the script follows the guide.
