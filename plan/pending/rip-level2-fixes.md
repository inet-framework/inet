# RIP level 2 — repair the model gaps

**Status:** in progress. Started 2026-09-25 on `topic/standards-tests-rip-level2-fixes`, on top of
`topic/standards-tests-rip-level2` at `3396c80ea1`. Worktree:
`/home/levy/workspace/inet-standards-tests-rip-level2-fixes`.

The level 2 pass of 2026-09-24 measured the model and repaired nothing. This branch repairs the
six gaps of [`results.md`](../../doc/project/evidence/model/rip/results.md#the-model-gaps), all
defects, one gap for each commit, as the user asked on 2026-09-25.

Commit group: `rip-model-repairs`. Each repair commit carries the source change, the test whose
gap paragraph changes, the WHATSNEW entry, and the fingerprint rows that the change moves
([PR-SPLIT-BASELINE](../../doc/project/rule/pull-request.md#pr-split-baseline)). Statistical
baselines live in the statistics repository, on a branch of the same name.

## How each repair is measured

The same as in the MPLS and IPsec repair branches: the build of `inet-master` at `7772a7e4ef`
(src `5c4f41c600`); the RIP protocol tests in debug; the CI fingerprint set in release (baseline
1752 tests equal, one expected error); the statistical tests in the GitHub-job copy, filtered to
the configurations of `examples/rip` and `tutorials/rip` for each commit, and all 930 at the tip
of the branch (baseline: 27 unexpected failures, all of 802.11 EDCA results).

## Steps

1. [x] **Plan** — this file.
2. [x] **Gap 1** — RIPng messages carry the version 1. Tests: `Rfc2080UpdateFields`,
   `Rfc2080LostNetwork`.
3. [x] **Gap 2** — the RIPng answer to a request leaves on the interface of the request, from its
   link-local address. Test: `Rfc2080TableRequest`.
4. [ ] **Gap 3** — the IPv6 routing table adds the routes of an interface again when its carrier
   returns. Test: `Rfc2080GarbageCollection`.
5. [ ] **Gap 4** — the timeout of a route has a timer of its own. Tests: `Rfc2453RouteExpiry`,
   `Rfc2080RouteExpiry`.
6. [ ] **Gap 5** — a router removes a network that it lost, after the garbage-collection time.
   Tests: `Rfc2453LostNetworkGarbageCollection`, `Rfc2080LostNetworkGarbageCollection`.
7. [ ] **Gap 6** — a repeated withdrawal does not restart the garbage collection, which starts at
   the first one. Tests: `Rfc2453WithdrawnRouteGarbageCollection`,
   `Rfc2080WithdrawnRouteGarbageCollection`.
8. [ ] **The documents** — a fresh run; `results.md`, `coverage.md`, `conformance.md` part 2 and
   `notes.md` ("Fixed on" entries) follow it; the statistics branch.
9. [ ] Gates, then move this plan to `plan/done/`.

## Decisions and facts found on the way
- **Gap 1** (`f6b216f210`): `Rip::getMessageVersion()` gives 1 for RIPng and 2 for RIP; the four
  `setVersion` calls use it. No fingerprint and no statistical result moves.
- **Gap 2 made a test error visible.** With the RIPng answers delivered, `Rfc2080SplitHorizon`
  failed: the triggered update of R3 draws a random delay, so the first periodic update of R3
  comes after the update of R2 at 47.8 s. The check and the tests of both protocols now wait for
  the update of R2 on L1 with netC at metric 2. This repair is its own commit (`7a273fced8`),
  before gap 2.
- **Gap 2** (`780e15b324`): for RIPng, `sendPacket` binds a unicast answer to the interface of the
  request and gives it the link-local source; `processRequest` sends the answer to specific
  entries through `sendPacket` too. One fingerprint moves: `examples/rip/simpletest -c IPv6`
  (`tplx`, `~tNl`), because the routers get the answers to their startup requests. The 18
  statistical results of `examples/rip` and `tutorials/rip` do not move.
