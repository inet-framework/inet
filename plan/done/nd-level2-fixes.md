# ND level 2 — repair the model gaps

**Status:** done on 2026-09-25: eleven gaps repaired; not merged, not pushed. Started 2026-09-25 on `topic/standards-tests-nd-level2-fixes`, on top of
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
2. [x] **Gap 1** — a host sends with its CurHopLimit, 64 by default or the value of the router.
   Tests: `Rfc4861HopLimitFromRouter`, `Rfc4861HopLimitNoRouter`.
3. [x] **Gap 2** — the router defaults AdvCurHopLimit 64 and AdvLinkMTU 0 (no MTU option). Tests:
   `Rfc4861RaCurHopLimit`, `Rfc4861RaMtuOption`. Before gap 3, because with the old default of
   1280 a repair of gap 3 alone would make every host send at most 1280 octets.
4. [x] **Gap 3** — a host fragments to the advertised LinkMTU. Test: `Rfc4861MtuFromRouter`.
5. [x] **Gap 4** — address resolution waits RetransTimer, and Duplicate Address Detection reads
   it in milliseconds. Test: `Rfc4861RetransTimerFromRouter`.
6. [x] **Gap 5** — the router answers every solicitation. Test: `Rfc4861RaSolicited`.
7. [x] **Gap 6** — the first periodic advertisements come within MAX_INITIAL_RTR_ADVERT_INTERVAL.
   Test: `Rfc4861RaUnsolicited`.
8. [x] **Gap 7** — a host processes an advertisement during Duplicate Address Detection. Test:
   `Rfc4861RouterLifetimeZero`.
9. [x] **Gap 8** — the Redirect carries the Target Link-Layer Address and the Redirected Header
   options. Tests: `Rfc4861RedirectFields`, `Rfc4861RedirectedHeader`,
   `Rfc6980RedirectLargePacket`.
10. [x] **Gap 9** — a node joins its solicited-node groups. Test: `Rfc4862GroupsBeforeDad`. The
    same repair closes gap 5 of MLD, on the IGMP and MLD branch.
11. [x] **Gap 10** — a router tests its configured address. Test: `Rfc4862RouterDad`.
12. [x] **Gap 11** — an expired address is not used. Test: `Rfc4862AddressLifetime`.
13. [x] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
    `notes.md` ("Fixed on" entries) follow it; the statistics branch.
14. [x] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way

- **Gap 1** (`922926f0b1`): the CurHopLimit belongs to an interface, which routing chooses after
  the encapsulation. `encapsulate()` marks a datagram without a hop limit with a `HopLimitReq` of
  -1 (the default of the tag), and `fragmentPostRouting()` writes the CurHopLimit of the outgoing
  interface and removes the mark; a datagram that waits for Duplicate Address Detection passes
  there twice. Default 64 (`IPv6_DEFAULT_CURHOPLIMIT`). 12 fingerprints move in `~tND` only.
- **Gap 2** (`35e897ba39`): defaults 64 and 0, and no MTU option for 0. The module test
  `IPv6_RA_MTU_option` now sets `advLinkMtu = 1280`. 28 fingerprints, 33 statistical results.
- **A commit that changes `networklayer/ipv6` and `networklayer/icmpv6` takes the position
  `src.networklayer`** and the subject prefix `networklayer:` (CR-SCOPE-POSITION). Gaps 2 to 4
  were first written as `ipv6:` and `icmpv6:`, and a message filter corrected them; the IPsec
  commit of the dummy packets, which changes the NED files of IPv4 and IPv6, got the same
  correction on its branch.
- **Gap 3** (`4d7f3bc361`): the default of LinkMTU was 1280; it is now 0, the MTU of the
  interface. Nothing moves, because no router of the sets sends an MTU option any more.
- **Gap 4** (`28dfed59ef`): the RetransTimer is a `simtime_t`; the default of AdvRetransTimer
  becomes 0, else a host would retransmit every millisecond. The same unit error of
  AdvReachableTime stays (a follow-up; no test). 19 fingerprints in `~tND` only.
- **Gap 5** (`312e9e60c5`): the smallest repair, `nextScheduledRATime` set back to the periodic
  timer after an answer. 6 fingerprints, 7 statistical results.
- **Gap 6** (`3616c3db36`) **first broke five GPSR runs**: the capped interval is exactly 16 s,
  and all routers started their timer at t = 0, so twenty GPSR routers transmitted at the same
  instant, which the radio medium refuses. The timer now starts at the boot of the router
  (`routerBootupTime`), as an advertising interface needs its link-local address. A test error
  showed at the bound: `within(16.0)` fires before an event at exactly 16 s
  (`ab54e3b914`). The first fingerprint run of gap 6 hid the five errors, because I read only the
  first lines of the output; `fp-check.sh` now prints the unexpected errors. 48 fingerprints, 53
  statistical results; the EIGRP results have a limit of `3min`, which `stat-update.sh` first
  missed.
- **Gap 7** (`3690f26316`): the router part of an advertisement during DAD; the prefixes still
  wait. The new timing of gap 6 made `Rfc4861RouterLifetimeZero` pass with the defect in the
  code; a delay of 250 ms on the link of R1 now puts its answer into the detection of A
  (`c3d138a1bc`). 1 fingerprint, 2 statistical results.
- **Gap 8** (`5d7c5069ff`): the new class `Ipv6NdRedirectedHeader`. A dangling reference to the
  bytes of a temporary chunk first gave zeros. The test of the Target Link-Layer Address assumed
  that R1 knows R2; a router builds the Redirect before it resolves the next hop, so R1 now pings
  R2 at 8 s (`99359634b6`). 3 fingerprints, 3 statistical results.
- **Gap 9** (`affa7e64fa`): one membership per solicited-node group, because the join and leave
  signals drive MLDv1 on every call. The MLDv2 module tests hold the new group; the two MLDv1
  module tests pinned a seed whose roles swapped, and now pin seed 0, as their notes ask. 3
  fingerprints, 5 statistical results. The IGMP and MLD branch takes this commit for its gap 5.
- **Gap 10** (`91a91e1505`): a configured address is tentative, and Neighbor Discovery tests every
  tentative address at the boot of the node; the `dadInProgress` flag stays set while any
  detection of the interface runs. 34 fingerprints, 39 statistical results.
- **Gap 11** (`be7b698a38`): a timer removes an address when its valid lifetime ends. Nothing
  moves. The protocol suite has no unexpected failure any more, and the full statistical set at
  the tip gives the failures of the base with the statistics branch.
- **The statistics branch** `topic/standards-tests-nd-level2-fixes` of the statistics repository
  has one commit for each commit that moves a result: gaps 2, 5, 6, 7, 8, 9 and 10. The selective
  update script `update_selected.py` of the GitHub-job copy regenerates exactly the moved
  configurations.
- **The documents** (`4e1a4bfd5c`): a fresh run at `dfc67c34f4`, 37 of 37 PASS. The ledger
  script is a copy in `audit/nd-level2-fixes/` of `inet-master` whose verdicts mark the repaired
  statements; 5 features supported, 13 partial, 5 untested.
