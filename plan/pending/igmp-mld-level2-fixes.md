# IGMP and MLD level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-igmp-mld-level2-fixes`, on
top of `topic/standards-tests-igmp-mld-level2` at `8dba80c00a`. Worktree:
`/home/levy/workspace/inet-standards-tests-igmp-mld-level2-fixes`.

The level 2 twin pass of 2026-09-24 measured the models and repaired nothing. This branch repairs
the twelve defects of IGMP and the sixteen defects of MLD of
[`igmp/results.md`](../../doc/project/evidence/model/igmp/results.md#the-model-gaps) and
[`mld/results.md`](../../doc/project/evidence/model/mld/results.md#the-model-gaps), one gap of one
protocol for each commit, and adds the two missing features that the user put in scope on
2026-09-25: the IGMPv2 querier mode of an IGMPv3 router (IGMP gap 13) and the MLDv1 mode of an
MLDv2 router (MLD gap 17).

Commit group: `igmp-mld-model-repairs`. Each repair commit carries the source change, the tests
whose gap paragraph changes, the WHATSNEW entry, and the fingerprint rows that the change moves
([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical
baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

As on the ND repair branch: the IGMP and MLD suites in debug; the whole protocol suite (base: 371
tests, 285 PASS, 44 FAIL declared expected, the 42 IGMP and MLD failures) and all module tests
after each gap; the CI fingerprint set in release (base: 1752 tests equal, one expected error);
the whole statistical set in the GitHub-job copy for each commit (base: 929 tests, 890 PASS, 10
SKIP, 27 FAIL of 802.11 EDCA, 2 expected ERROR), with the moved results regenerated at the commit
that moves them.

## Steps

The twin gaps come next to each other, IGMP first.

1. [x] **Plan** — this file.
2. [ ] **IGMP gap 1, MLD gap 1** — the Queries carry the QRV and the QQIC.
3. [ ] **IGMP gap 2** — an IGMP message has the precedence of Internetwork Control.
4. [ ] **MLD gap 2** — an MLD message leaves from a link-local address.
5. [ ] **MLD gap 3** — an MLD message carries the Router Alert option.
6. [ ] **MLD gap 4** — the ICMPv6 checksum covers the pseudo-header.
7. [ ] **MLD gap 5** — a node reports its solicited-node addresses: the change of ND gap 9
   (`affa7e64fa` on `topic/standards-tests-nd-level2-fixes`), applied again here.
8. [ ] **IGMP gap 3, MLD gap 6** — the default intervals of RFC 9776 and RFC 9777.
9. [ ] **IGMP gap 4, MLD gap 7** — the Older Version Querier Present Interval.
10. [ ] **IGMP gap 5, MLD gap 8** — a second change merges the pending records.
11. [ ] **IGMP gap 6, MLD gap 9** — the older-version mode of a host has a report delay timer,
    repeats its unsolicited Report and suppresses it.
12. [ ] **IGMP gap 7, MLD gap 10** — a change of the compatibility mode cancels the pending timers.
13. [ ] **IGMP gap 8, MLD gap 11** — an answer to a Query leaves out empty records.
14. [ ] **IGMP gap 9, MLD gap 12** — the S flag comes from the lowered timer.
15. [ ] **IGMP gap 10, MLD gap 13** — a Report does not cancel the retransmissions of the Queries.
16. [ ] **IGMP gap 11, MLD gap 14** — a source-specific Query lowers the Source Timers.
17. [ ] **IGMP gap 12, MLD gap 15** — the forwarding asks for a listener of the source.
18. [ ] **MLD gap 16** — an MLDv1 node reads an MLDv2 Query.
19. [ ] **IGMP gap 13** — the IGMPv2 querier mode of an IGMPv3 router (missing feature).
20. [ ] **MLD gap 17** — the MLDv1 mode of an MLDv2 router (missing feature).
21. [ ] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
    `notes.md` of both protocols follow it; the statistics branch.
22. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

