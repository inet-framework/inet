# ND level 2 — repair the model gaps

**Status:** done on 2026-09-25: eleven gaps repaired. Since 2026-10-28 the branch is built on the IGMP and MLD repairs, and the second half of gap 9 is open. Started 2026-09-25 on `topic/standards-tests-nd-level2-fixes`, on top of `topic/standards-tests-nd-level2` at `03f8e313c5`. Worktree: `/home/levy/workspace/inet-standards-tests-nd-level2-fixes`.

The level 2 pass of 2026-09-24 measured the model and repaired nothing. This branch repairs the eleven gaps of [`results.md`](../../doc/project/evidence/model/nd/results.md#the-model-gaps), all defects, one gap for each commit, as the user asked on 2026-09-25.

Commit group: `nd-model-repairs`. Each repair commit carries the source change, the tests whose gap paragraph changes, the WHATSNEW entry, and the fingerprint rows that the change moves ([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

The same as in the RIP repair branch: the build of `inet-master` at `7772a7e4ef` (src `5c4f41c600`); the ND protocol tests in debug; the CI fingerprint set in release (base: 1752 tests equal, one expected error); the statistical tests in the GitHub-job copy, the whole set for each commit, because the ND code runs in every IPv6 node (base: 929 tests, 890 PASS, 10 SKIP, 27 FAIL of 802.11 EDCA, 2 expected ERROR). The whole protocol suite (base: 319 tests, 262 PASS, 42 expected FAIL, the 15 ND failures) and the module suite run at the tip, and after each gap that can change other suites.

## Steps

1. [x] **Plan** — this file.
2. [x] **Gap 1** — a host sends with its CurHopLimit, 64 by default or the value of the router. Tests: `Rfc4861HopLimitFromRouter`, `Rfc4861HopLimitNoRouter`.
3. [x] **Gap 2** — the router defaults AdvCurHopLimit 64 and AdvLinkMTU 0 (no MTU option). Tests: `Rfc4861RaCurHopLimit`, `Rfc4861RaMtuOption`. Before gap 3, because with the old default of 1280 a repair of gap 3 alone would make every host send at most 1280 octets.
4. [x] **Gap 3** — a host fragments to the advertised LinkMTU. Test: `Rfc4861MtuFromRouter`.
5. [x] **Gap 4** — address resolution waits RetransTimer, and Duplicate Address Detection reads it in milliseconds. Test: `Rfc4861RetransTimerFromRouter`.
6. [x] **Gap 5** — the router answers every solicitation. Test: `Rfc4861RaSolicited`.
7. [x] **Gap 6** — the first periodic advertisements come within MAX_INITIAL_RTR_ADVERT_INTERVAL. Test: `Rfc4861RaUnsolicited`.
8. [x] **Gap 7** — a host processes an advertisement during Duplicate Address Detection. Test: `Rfc4861RouterLifetimeZero`.
9. [x] **Gap 8** — the Redirect carries the Target Link-Layer Address and the Redirected Header options. Tests: `Rfc4861RedirectFields`, `Rfc4861RedirectedHeader`, `Rfc6980RedirectLargePacket`.
10. [x] **Gap 9** — a node joins its solicited-node groups. Test: `Rfc4862GroupsBeforeDad`. The same repair closes gap 5 of MLD, on the IGMP and MLD branch. Since 2026-10-28 that branch carries the change, and the Report before the solicitation is open (see the last section).
11. [x] **Gap 10** — a router tests its configured address. Test: `Rfc4862RouterDad`.
12. [x] **Gap 11** — an expired address is not used. Test: `Rfc4862AddressLifetime`.
13. [x] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and `notes.md` ("Fixed on" entries) follow it; the statistics branch.
14. [x] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **Gap 1** (`3cf34d9394`): the CurHopLimit belongs to an interface, which routing chooses after the encapsulation. `encapsulate()` marks a datagram without a hop limit with a `HopLimitReq` of -1 (the default of the tag), and `fragmentPostRouting()` writes the CurHopLimit of the outgoing interface and removes the mark; a datagram that waits for Duplicate Address Detection passes there twice. Default 64 (`IPv6_DEFAULT_CURHOPLIMIT`). 12 fingerprints move in `~tND` only.
- **Gap 2** (`4f6c18213c`): defaults 64 and 0, and no MTU option for 0. The module test `IPv6_RA_MTU_option` now sets `advLinkMtu = 1280`. 28 fingerprints, 33 statistical results.
- **A commit that changes `networklayer/ipv6` and `networklayer/icmpv6` takes the position `src.networklayer`** and the subject prefix `networklayer:` (CR-SCOPE-POSITION). Gaps 2 to 4 were first written as `ipv6:` and `icmpv6:`, and a message filter corrected them; the IPsec commit of the dummy packets, which changes the NED files of IPv4 and IPv6, got the same correction on its branch.
- **Gap 3** (`8d9f9a9e8d`): the default of LinkMTU was 1280; it is now 0, the MTU of the interface. Nothing moves, because no router of the sets sends an MTU option any more.
- **Gap 4** (`555cdd0678`): the RetransTimer is a `simtime_t`; the default of AdvRetransTimer becomes 0, else a host would retransmit every millisecond. The same unit error of AdvReachableTime stays (a follow-up; no test). 19 fingerprints in `~tND` only.
- **Gap 5** (`2787442783`): the smallest repair, `nextScheduledRATime` set back to the periodic timer after an answer. 6 fingerprints, 7 statistical results.
- **Gap 6** (`9931374d01`) **first broke five GPSR runs**: the capped interval is exactly 16 s, and all routers started their timer at t = 0, so twenty GPSR routers transmitted at the same instant, which the radio medium refuses. The timer now starts at the boot of the router (`routerBootupTime`), as an advertising interface needs its link-local address. A test error showed at the bound: `within(16.0)` fires before an event at exactly 16 s (`59d5ef4270`). The first fingerprint run of gap 6 hid the five errors, because I read only the first lines of the output; `fp-check.sh` now prints the unexpected errors. 48 fingerprints, 53 statistical results; the EIGRP results have a limit of `3min`, which `stat-update.sh` first missed.
- **Gap 7** (`9d94072e30`): the router part of an advertisement during DAD; the prefixes still wait. The new timing of gap 6 made `Rfc4861RouterLifetimeZero` pass with the defect in the code; a delay of 250 ms on the link of R1 now puts its answer into the detection of A (`f84db43f6a`). 1 fingerprint, 2 statistical results.
- **Gap 8** (`55aa88fc4b`): the new class `Ipv6NdRedirectedHeader`. A dangling reference to the bytes of a temporary chunk first gave zeros. The test of the Target Link-Layer Address assumed that R1 knows R2; a router builds the Redirect before it resolves the next hop, so R1 now pings R2 at 8 s (`a554b72f20`). 3 fingerprints, 3 statistical results.
- **Gap 9** (dropped on 2026-10-28, see the last section): one membership per solicited-node group, because the join and leave signals drive MLDv1 on every call. The MLDv2 module tests hold the new group; the two MLDv1 module tests pinned a seed whose roles swapped, and now pin seed 0, as their notes ask. 3 fingerprints, 5 statistical results. The IGMP and MLD branch takes this commit for its gap 5.
- **Gap 10** (`e3ff43da45`): a configured address is tentative, and Neighbor Discovery tests every tentative address at the boot of the node; the `dadInProgress` flag stays set while any detection of the interface runs. 34 fingerprints, 39 statistical results.
- **Gap 11** (`ce926baec8`): a timer removes an address when its valid lifetime ends. Nothing moves. The protocol suite has no unexpected failure any more, and the full statistical set at the tip gives the failures of the base with the statistics branch.
- **The statistics branch** `topic/standards-tests-nd-level2-fixes` of the statistics repository has one commit for each commit that moves a result: gaps 2, 5, 6, 7, 8, 9 and 10. The selective update script `update_selected.py` of the GitHub-job copy regenerates exactly the moved configurations.
- **The documents** (`eadff0948b`): a fresh run at `8631810bdc`, 37 of 37 PASS. The ledger script is a copy in `audit/nd-level2-fixes/` of `inet-master` whose verdicts mark the repaired statements; 5 features supported, 13 partial, 5 untested.

## Stacked on the IGMP and MLD repairs (2026-10-28)

After the rebase onto the `master` of 2026-10-27, this branch made eight tests of other suites fail. The solicited-node change of gap 9 let an MLDv2 host send Reports before the first MLDv1 Query, and an MLDv1 router stopped on them in three MLD tests. Gap 10 makes a configured address tentative for about 2 to 3 seconds: two MLD start-up tests failed, because the Query timer of `Mld` started before the router could send, and three IPv6 IPsec tests failed, because their flows start at 1 second. The IGMP and MLD repairs (`topic/standards-tests-igmp-mld-level2-fixes`) repair both MLD causes, and they carry the same solicited-node change as gap 9. So the user decided on 2026-10-28 to build this branch on them; its pull request merges after theirs.

- **Gap 9.** The gap 9 commit is dropped: the IGMP and MLD branch carries the join of the solicited-node groups. On that base, `Rfc4862GroupsBeforeDad` still fails: MLD holds every message until the link-local address is tested, so the Report for the group leaves after the solicitation. RFC 9777 section 5.2.14, and RFC 3590 for MLDv1, send it from the unspecified address. The user decided to leave this half of gap 9 open. The first commit after the plan gives the test its new reason, and `results.md` describes the gap.
- **IPsec.** A new commit before gap 10 lets the six IPv6 IPsec tests run without Duplicate Address Detection, as the common mockups of the IPsec checks now say. The user chose this over later flows.
- **Module tests.** Gap 6 swaps the two hosts of `MLD_host_basic` and `MLD_host_groupstates` at seed 1, so gap 6 re-picks seed 0 for both, as their notes ask.
- **Baselines.** Each commit got its fingerprint values in `tests/fingerprint/store.json` again, measured at that commit on the new base; the commit messages list them. The statistics branch was computed again on the statistics branch of the IGMP and MLD repairs, in the GitHub-job copy, at each commit that it names.

The measurements use the commands of opp_repl (`opp_run_*`), because master no longer has INET's own copy of the test framework. Every measurement uses a clean build: incremental builds across many rebases left stale objects, which made runs crash or move.
