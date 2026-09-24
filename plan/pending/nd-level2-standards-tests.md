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
5. [ ] **Step 6, tests** — `tests/protocol/nd/Rfc4861*.test`, `Rfc4862*.test`, and a helper.
6. [ ] **Step 7, run** — `model/nd/results.md`.
7. [ ] **Steps 8 and 9, and the ledger** — `model/nd/conformance.md` part 2,
   `model/nd/categories.md`, `model/nd/coverage.md`.
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
