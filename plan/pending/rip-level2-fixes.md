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
4. [x] **Gap 3** — the IPv6 routing table adds the routes of an interface again when its carrier
   returns. Test: `Rfc2080GarbageCollection`.
5. [x] **Gap 4** — the timeout of a route has a timer of its own. Tests: `Rfc2453RouteExpiry`,
   `Rfc2080RouteExpiry`.
6. [x] **Gap 5** — a router removes a network that it lost, after the garbage-collection time.
   Tests: `Rfc2453LostNetworkGarbageCollection`, `Rfc2080LostNetworkGarbageCollection`.
7. [x] **Gap 6** — a repeated withdrawal does not restart the garbage collection, which starts at
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
- **Gap 3 is two commits**, because it is two decisions (PR-SPLIT-ONE-CHANGE). The on-link route
  of netA on R1 is a static (`MANUAL`) route of `Ipv6NetworkConfigurator`, and RIPng imports it
  as `RIP_ROUTE_STATIC`, not as an interface route.
  - `9f5eb4605a` (`src.networklayer.ipv6`): `Ipv6RoutingTable` keeps the `MANUAL` and
    `OWN_ADV_PREFIX` routes of an interface aside while the interface is down, and adds them again
    when it is up with a carrier, unless an equal route is there (EIGRP for IPv6 adds its own
    on-link routes again on the same signal). A stop or a crash forgets them. The new module test
    `IPv6_routes_return_with_carrier` fails without the repair (R1 sends "unroutable") and passes
    with it. No fingerprint moves; the 38 IPv6 module tests and 27 IPv6 protocol tests pass.
  - `00ed12e42c` (`src.routing.rip`): the `routeAdded` handler attaches a returning static or
    default route to the RIP route of the same destination and type that lost its route, with the
    metric 1 of the import. No fingerprint moves (the run of both parts together).
  - OMNeT++ calls the listeners of a signal in the order of subscription. The routing table
    subscribes before RIP, so on a carrier loss the table removes the route first, and RIP only
    detaches it. If RIP were first, `Rip::invalidateRoute` would delete the static route itself.
- **Found on the way: a chain of triggered updates after an expiry.** `checkExpiredRoutes` runs
  from `sendRoutes` and calls `invalidateRoute` again for a route that is already invalid. Each
  call sets the change flag and triggers an update, so from the expiry to the purge the router
  sends a triggered update every 1 to 5 s (seen in the log of `Rfc2453ExpiryGarbageCollection`:
  R2 from 286 s on). RFC 2453 section 3.9.2 starts the deletion process only when the metric is
  first set to infinity. The repair of gap 6 needs the same rule, so gap 6 ends the chain.
- **Gap 4** (`b3c86d279d`): `Rip` has an expiry timer; `rescheduleExpiryTimer` puts it at the
  earliest expiry or purge, after each change of a route, so a refreshed route moves it and a
  stable network never fires it. Three fingerprints of `tutorials/rip` move (Step4B, Step4C, and
  Step9 in `tplx` only), and the statistics of Step4B and Step4C: the same datagrams are lost,
  but some now circle in a short loop to the hop limit, because Step4 has no split horizon.
- **Gap 5** (`210e2a03c0`): `checkExpiredRoutes` purges a lost route of the router itself
  `routePurgeTime` after its invalidation, if no route of the table is attached. A route of the
  configuration that returns after the purge is imported again (`MANUAL` or `OWN_ADV_PREFIX`,
  as at startup), and each import triggers an update. 11 fingerprints and 11 statistical results
  move. Step6, Step7 and Step7SplitHorizon do not: there the router learns the lost network
  again from its neighbor, so the route becomes a learned route.
- **Gap 6** (`19cd0a2f42`): every invalid route is purged `routePurgeTime` after its
  invalidation, only valid learned routes expire, and no path invalidates an invalid route
  again. That ends the chain of triggered updates. 15 fingerprints and 15 statistical results
  move. All 30 RIP tests pass.
- **The statistics of the first commits were measured wrong.** `inet_run_statistical_tests`
  keeps only the last `-f`, so the runs of gaps 1 and 2 with two filters saw `tutorials/rip`
  only (18 tests, not 28). `stat-check.sh` now joins the filters into one alternation. The
  correct runs: gap 1 moves nothing; gap 2 moves `examples/rip/simpletest -c IPv6` (the answer
  reaches R2, and the three solicitations on the wrong link go away), which the full run of the
  IPv6 commit found first. The messages of the earlier commits get the correct sentences at the
  end of the branch (`rip-stat-fix.py`, a message filter).
- **The statistics branch** `topic/standards-tests-rip-level2-fixes` of the statistics
  repository, on `9ab4c26`, has one commit for each commit that moves a result: gap 2
  (`6d85bba`), gap 4 (`0acdaed`), gap 5 (`1e13d02`) and gap 6 (`4c9b7b3`). Each was regenerated
  in the GitHub-job copy at its INET commit, for the moved configurations only.
