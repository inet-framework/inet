# MPLS level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-mpls-level2-fixes`, on top of
`topic/standards-tests-mpls-level2` at `24675c3a37`. Worktree:
`/home/levy/workspace/inet-standards-tests-mpls-level2-fixes`.

The level 2 pass of 2026-09-24 measured the model and repaired nothing. This branch repairs the
gaps of [`results.md`](../../doc/project/evidence/model/mpls/results.md#the-model-gaps), one gap for
each commit, as the user asked on 2026-09-25: the defects and the small or medium missing
features. A large missing feature gets a plan of its own and no code.

Commit group: `mpls-model-repairs`. Each repair commit carries the source change, the test whose
declaration or gap paragraph changes, and the fingerprint rows that the change moves
([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical
baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

- **Build**: the worktree starts with the release and debug build of `inet-master` at
  `7772a7e4ef` (the same src tree, `5c4f41c600`), copied with hard links and fresh times.
- **Protocol tests**: `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/mpls$'`; the
  repaired tests pass, and no other test of the suite changes its verdict.
- **Fingerprints**: the CI set (`-f tplx -f '~tNl' -f '~tND'`), release, on a scratch copy of
  `tests/fingerprint`. Baseline on `inet-master` at `7772a7e4ef`, after its 12 stale 802.11
  objects were rebuilt: 1752 tests, equal to the expected results, one expected error.
- **Statistics**: in the GitHub-job copy (`ghci-statistical`), baseline at `7772a7e4ef`, then the
  tip of the branch; each changed result is attributed to the commit that moves it.
- **Module tests**: the MPLS, RSVP-TE and LDP module tests, debug, before the last commit.

## Steps

1. [x] **Plan** — this file.
2. [ ] **Gap 1** — the TTL of each label stack entry: the first label copies the IPv4 TTL, each
   LSR forwards with one less, and a pushed label copies the TTL below it (the Uniform Model).
   Tests: `Rfc3032FirstLabelTtl`, `Rfc3032TtlAtEachLsr`, `Rfc3032TtlWithPush`.
3. [ ] **Gap 2** — the LSP counts its hops in the IPv4 TTL: the ingress labels a datagram after
   IPv4 forwards it, and a pop that empties the stack writes the outgoing TTL into the IPv4
   header. Tests: `Rfc3032TtlAfterPop`, `Rfc3443TtlAfterPenultimatePop`, `Rfc3443TtlAfterTwoPops`,
   `Rfc3031TtlAcrossLsp`, `Rfc3031TtlAcrossLspPenultimate`.
4. [ ] **Gap 3** — an LSR does not forward a labeled packet whose outgoing TTL is zero. Test:
   `Rfc3032TtlExpiry`.
5. [ ] **Gap 4** — an MPLS router on an Ethernet link: packets of other protocols go up, and a
   labeled packet gets its Ethernet header. Test: `Rfc3032EthernetEncapsulation`.
6. [ ] **Gap 5** — the reserved labels 0 and 3. Tests: `Rfc3032ExplicitNull`,
   `Rfc3032ImplicitNull`; their declarations go.
7. [ ] **Gap 6** — the MTU check and the fragmentation of a labeled datagram, and the ICMP
   message for one with the DF bit. Tests: the three `Rfc3032TooBig*`; their declarations go.
8. [ ] **Gap 7, a plan only** — PPP LCP and the MPLS Control Protocol:
   `plan/pending/ppp-lcp-and-mplscp.md`.
9. [ ] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
   `notes.md` ("Fixed on" entries) follow it; the statistics branch.
10. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **The baseline needed a rebuild of `inet-master`.** Its objects of `Edca`, `Edcaf` and `Hcf`
  were compiled before `c0314ff5aa` (built 2026-09-23, commit pulled 2026-09-24), and the first
  baseline run gave 40 errors "Cannot cast nullptr to type NonQosRecoveryProcedure". The 12
  objects that depend on the changed files were deleted in both modes and rebuilt. A second
  cause of errors was the scratch copy: tests with the working folder `.` need the ini files of
  `tests/fingerprint`, so the scratch copy holds the whole folder.
