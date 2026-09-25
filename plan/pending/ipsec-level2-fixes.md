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
2. [ ] **Gap 1** — the first packet of an SA carries the Sequence Number 1. Tests:
   `Rfc4303SequenceNumbers`, `Rfc4302SequenceNumbers`.
3. [ ] **Gap 2** — the AH Payload Length in 32-bit words minus 2, and the dissector that reads it.
   Tests: `Rfc4302AhFormat`, `Rfc4302AhFormatIpv6`.
4. [ ] **Gap 3** — the AH ICV inside the header, before the payload, padded to 8 octets in IPv6.
   Tests: `Rfc4302AhIcvPosition`, `Rfc4302AhIcvPositionIpv6`.
5. [ ] **Gap 4** — the ESP padding octets 1, 2, 3 and so on. Test: `Rfc4303EspPadding`.
6. [ ] **Gap 5** — the selector check of an inbound SA after AH or ESP processing. Test:
   `Rfc4301InboundSelectorCheck`.
7. [ ] **Gap 6** — a PROTECT entry without a matching SA discards the packet. Test:
   `Rfc4301ProtectWithoutSa`.
8. [ ] **Gap 7** — AH and ESP on one packet: an SA can override the Protection of its entry, and
   the ingress goes on to ESP after AH. Test: `Rfc4301AhAndEsp`, whose configuration changes.
9. [ ] **Gap 9** — the lifetime of an SA, in seconds and in octets. Test: `Rfc4301SaLifetime`.
10. [ ] **Gap 10** — the choice of an SA by DSCP. Test: `Rfc4301ParallelSas`.
11. [ ] **Gap 11** — dummy packets. Test: `Rfc4303DummyPackets`.
12. [ ] **Gaps 8 and 12, a plan each** — `plan/pending/ipsec-tunnel-mode.md` and
    `plan/pending/ipv6-path-mtu.md`.
13. [ ] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
    `notes.md` ("Fixed on" entries) follow it; the statistics branch.
14. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way
