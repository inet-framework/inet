# ND level 2 — catalogs, feature map, checks and tests for RFC 4861, RFC 4862, RFC 5942, RFC 6980

**Status:** in progress. Started 2026-09-24 on `topic/standards-tests-nd-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-nd-level2`.

The second pass for IPv6 Neighbor Discovery and stateless address autoconfiguration, after the
level 1 survey of wave 0, and the second pass of wave 1 after RIP. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md), steps
3 to 9, at **level 2, Core**. The in-scope set is fixed in
[`nd/standards.md`](../../doc/project/evidence/protocol/nd/standards.md#in-scope-set): RFC 4861
§4, §6, §7.2 and §8; RFC 4862 §5; RFC 5942 §6; RFC 6980 §5.

The user's decision of 2026-09-24: the RIP pass found a failure rate of a third, so no RIP level
3 now; the next protocol of the wave comes first, and the failures are repaired later. The same
holds here: this pass measures the model and repairs nothing.

Commit group: `nd-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.

## What the pass takes over from the RIP pass

- The catalog is drafted in parallel: four agents write the entries of their sections with a
  shared brief, in the form of the RIP catalogs, and a script checks every quote against its
  line reference; the entries are then merged, reviewed and indexed.
- Tests come from a generator with shared network templates, and the ledger table from a
  per-statement mapping (`audit/rip-level2/` in `inet-master` holds the RIP scripts).
- A timer check fixes the start phases, so that it cannot pass by chance; a known-failing
  observation goes last, or into a check of its own.
- State signals are level 4; the checks of level 2 observe the wire.

## Steps

1. [x] **Plan** — this file; and the source paragraph of `nd/standards.md` points to the
   `standards` project, a leftover of the move of the texts.
2. [x] **Step 3, catalogs** — `standard/rfc4861/catalog.md`, `standard/rfc4862/catalog.md`,
   `standard/rfc5942/catalog.md`, `standard/rfc6980/catalog.md`. 403, 70, 10 and 6 entries,
   drafted by four agents in parallel and merged; every quote checked against its lines.
3. [x] **Step 4, feature map** — `protocol/nd/features.md`, `ND-F-*`. With step 3 in one commit.
   23 features, generated from a specification that places every one of the 489 entries.
4. [x] **Step 5, checks** — `protocol/nd/checks.md` and `protocol/nd/checks/<feature>.md`.
   34 checks in 7 files; 254 statements are in a check, and the other 235 are in the closing
   list of `checks.md` with what a check would need. A script checks that every one of the 489
   entries has exactly one place.
5. [x] **Step 6, tests** — `tests/protocol/nd/Rfc4861*.test`, `Rfc4862*.test`, and a helper.
   37 tests for the 34 checks, from a generator with the five mockups; `NdChecks.h` reads the
   ND messages, their options and their serialized octets. 22 pass and 15 fail; each failure
   has a paragraph that names its gap, and none is declared expected.
6. [x] **Step 7, run** — `model/nd/results.md`. A fresh run at `8f78f1a73c`: 37 tests, 22 PASS,
   15 FAIL, none declared; eleven gaps of the model, all defects.
7. [x] **Steps 8 and 9, and the ledger** — `model/nd/conformance.md` part 2,
   `model/nd/categories.md`, `model/nd/coverage.md`. Level 2 is reached: 14 of the 15 mandatory
   features have core checks that ran; the matrix holds 4 `confirmed`, 12 `partial`, 1
   `declined`, 1 `defect` and 5 `unverified`; 88 statements are owed.
8. [ ] Gates, then move this plan to `plan/done/`.

## Facts found before the catalogs

The one look at the code that the guide permits, to pick practical candidates:

- The `icmpv6` dissector reaches every ND message, so filter expressions can select them; the
  IPv6 tests already use `icmpv6.type`.
- `Ipv6NetworkConfigurator` with `assignAddressesToHosts = false` addresses the routers only and
  sets their advertised prefixes; the hosts then configure themselves by SLAAC. That is the
  normal path of RFC 4862, and the mockups use it.
- The ND module sends unsolicited Router Advertisements every 200 to 600 s by default, and
  hosts solicit at start; the default duplicate address detection sends one solicitation.
- The worktree build is a copy of the RIP worktree build (same `src` tree `5c4f41c600`): no
  compile; the IPv6 suite passes 27 of 27 there.

## Decisions and facts found on the way
- **The catalog drafts** came from four agents with one brief (scratchpad `nd-catalog-brief.md`):
  RFC 4861 §4, §6, §7.2 with §8, and RFC 4862 with RFC 5942 and RFC 6980. A merge script
  normalized the strength of every lowercase keyword to the form `must (lower case)` of the
  DHCP catalog, joined the "left out" lists into the out-of-scope sections, and kept the order
  of the document. Three flaws of RFC 4861 itself show in the entries and are named in the
  catalog: RetransTimer in seconds in §7.2.6, the MinRtrAdvInterval default, and the name
  CurHopLimit in §6.2.3.
- **Three levels needed the conditional-keyword refinement**: unsolicited advertisements, the
  processing of a Redirect and the change of a router's role are `optional`, because the
  `must` of each holds only once the node does the optional thing.
- **Every mandatory feature has a core check except message validation**: all its statements
  need a crafted message, so it waits for level 3, and `checks.md` says so.
- **Two readings of the text are fixed in the checks.** "About every RetransTimer" is 1 to 1.5
  times the timer. The cap of the first unsolicited advertisements holds for the intervals after
  the first and the second one; the interval after the third one is not judged, because the text
  leaves it open.
- **Two scenarios serve two checks each**, so that one rule does not hide another: the prefix
  with the L flag clear (on-link determination, then the Redirect to the on-link destination),
  and the prefix with a valid lifetime of 30 seconds and a router that stops (the prefix, then
  the address).
- **The size of a Redirect makes the check of RFC 6980**: an echo request of 1448 octets makes a
  Redirect that would exceed the link MTU if the router did not cut the redirected packet.
- **The configurator gave the hosts routes.** With its defaults, it adds a default route and an
  on-link route to every host, so the first run measured the configurator and not Neighbor
  Discovery. The mockups now set `addStaticRoutes = false` and give each router its routes in
  `<route>` elements; a host has no route until an advertisement gives it one.
- **Two router variables have no parameter.** AdvReachableTime and AdvRetransTimer can only be
  set in the interface data, so the tests that need them carry a small management module,
  `NdRouterVariables` in `NdChecks.h`. opp_test puts the NED of a test into a namespace of its
  own, so its `@class` starts with `::`.
- **MLD is off in the nodes by default** (`hasMld = false`). The two multicast checks turn it
  on, and `multicast.md` now says that every node runs MLD.
- **The Redirect checks start R2 and C at 5 seconds.** With all nodes up at once, host A dropped
  the advertisement of R1 because the one of R2 had started its Duplicate Address Detection,
  so the Redirect checks never reached their stimulus. The check of Router Lifetime zero keeps
  the simultaneous start and fails on that gap.
- **One check became two tests where a failure hid a verdict**: the Router Advertisement fields
  (three tests: fields, MTU option, Cur Hop Limit) and the Redirect fields (two tests: header
  with Target Link-Layer Address option, Redirected Header option).
- **An exploration test prints every ICMPv6 message** (scratchpad `mkexplore.py`); it made the
  causes of the failures visible and is never committed.
- **The ledger table comes from a generator** (scratchpad `gen-nd-ledger.py`, copied to
  `audit/nd-level2/` in `inet-master`): it reads the `Checks:` lines, the closing list, the tests
  and the feature map, and needs a hand mapping only for the statements of the 15 failing tests.
- **A check was stricter than the standard.** The fields check demanded the Source Link-Layer
  Address option, which RFC4861-RA-24 lets a router leave out; the check and its test now accept
  its absence (`b2822881e9`, `8f78f1a73c`).
