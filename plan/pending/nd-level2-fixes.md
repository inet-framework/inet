# ND level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-nd-level2-fixes`, on top of
`topic/standards-tests-nd-level2` at `03f8e313c5`. Worktree:
`/home/levy/workspace/inet-standards-tests-nd-level2-fixes`.

The level 2 pass of 2026-09-24 measured the model and repaired nothing. This branch repairs the
eleven gaps of [`results.md`](../../doc/project/evidence/model/nd/results.md#the-model-gaps), all
defects, one gap for each commit, as the user asked on 2026-09-25.

Commit group: `nd-model-repairs`. Each repair commit carries the source change, the tests whose
gap paragraph changes, the WHATSNEW entry, and the fingerprint rows that the change moves
([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical
baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

The same as in the RIP repair branch: the build of `inet-master` at `7772a7e4ef` (src
`5c4f41c600`); the ND protocol tests in debug; the CI fingerprint set in release (base: 1752
tests equal, one expected error); the statistical tests in the GitHub-job copy, the whole set for
each commit, because the ND code runs in every IPv6 node (base: 929 tests, 890 PASS, 10 SKIP,
27 FAIL of 802.11 EDCA, 2 expected ERROR). The whole protocol suite (base: 319 tests, 262 PASS,
42 expected FAIL, the 15 ND failures) and the module suite run at the tip, and after each gap that
can change other suites.

## Steps

1. [x] **Plan** — this file.
2. [ ] **Gap 1** — a host sends with its CurHopLimit, 64 by default or the value of the router.
   Tests: `Rfc4861HopLimitFromRouter`, `Rfc4861HopLimitNoRouter`.
3. [ ] **Gap 2** — the router defaults AdvCurHopLimit 64 and AdvLinkMTU 0 (no MTU option). Tests:
   `Rfc4861RaCurHopLimit`, `Rfc4861RaMtuOption`. Before gap 3, because with the old default of
   1280 a repair of gap 3 alone would make every host send at most 1280 octets.
4. [ ] **Gap 3** — a host fragments to the advertised LinkMTU. Test: `Rfc4861MtuFromRouter`.
5. [ ] **Gap 4** — address resolution waits RetransTimer, and Duplicate Address Detection reads
   it in milliseconds. Test: `Rfc4861RetransTimerFromRouter`.
6. [ ] **Gap 5** — the router answers every solicitation. Test: `Rfc4861RaSolicited`.
7. [ ] **Gap 6** — the first periodic advertisements come within MAX_INITIAL_RTR_ADVERT_INTERVAL.
   Test: `Rfc4861RaUnsolicited`.
8. [ ] **Gap 7** — a host processes an advertisement during Duplicate Address Detection. Test:
   `Rfc4861RouterLifetimeZero`.
9. [ ] **Gap 8** — the Redirect carries the Target Link-Layer Address and the Redirected Header
   options. Tests: `Rfc4861RedirectFields`, `Rfc4861RedirectedHeader`,
   `Rfc6980RedirectLargePacket`.
10. [ ] **Gap 9** — a node joins its solicited-node groups. Test: `Rfc4862GroupsBeforeDad`. The
    same repair closes gap 5 of MLD, on the IGMP and MLD branch.
11. [ ] **Gap 10** — a router tests its configured address. Test: `Rfc4862RouterDad`.
12. [ ] **Gap 11** — an expired address is not used. Test: `Rfc4862AddressLifetime`.
13. [ ] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
    `notes.md` ("Fixed on" entries) follow it; the statistics branch.
14. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

