# RIP level 2 — catalogs, feature map, checks and tests for RFC 2453 and RFC 2080

**Status:** in progress. Started 2026-09-24 on `topic/standards-tests-rip-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-rip-level2`.

The second pass for RIP, after the level 1 survey of wave 0. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md), steps
3 to 9, at the target level of [`rip/standards.md`](../../doc/project/evidence/protocol/rip/standards.md#target-level):
**level 2, Core**. The in-scope set is fixed there: RFC 2453 §3 and §4.2 to §4.6, and RFC 2080 §2.

Commit group: `rip-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.

## What level 2 asks

"Every normal-path mandatory mechanism of the base document appears as a feature, and every
mandatory feature has a core check that ran and has a verdict." The toolset is observation of a
normal exchange, including the absence of a packet. A scripted topology change (a lifecycle
operation of the scenario manager) is part of a normal exchange; the DHCP pass used one for its
release check. Injection and interception are level 3.

The catalogs hold every normative statement of the in-scope sections, not only the level 2 ones:
level 3 needs no new document, so the edge statements (crafted responses) go into the catalogs
now and wait in the ledger as `later`. Statements that need a RIP version 1 router belong to the
compatibility area of level 5 and are `later` too.

## Steps

1. [x] **Plan** — this file.
2. [x] **Step 3, catalogs** — `standard/rfc2453/catalog.md` (§3, §4.2 to §4.6) and
   `standard/rfc2080/catalog.md` (§2), from the texts in `../standards/RFC/`. 88 and 80 entries; a script checked every quote
   against its line reference. Committed with step 4, because the catalogs and the feature map
   link to each other and the link gate must pass at each commit.
3. [x] **Step 4, feature map** — `protocol/rip/features.md`, `RIP-F-*`, one feature for each
   mechanism, joining the parallel sections of the two documents. 19 features; every catalog
   entry is in a feature, except RFC2453-ADDR-8, the forwarding rule of IPv4.
4. [x] **Step 5, checks** — `protocol/rip/checks.md` with the common mockups and the index, and
   `protocol/rip/checks/<feature>.md`. One commit. 24 checks in eight files, 12 for RIP version
   2 and 12 for RIPng; a script confirmed that every catalog entry is in a check or in the
   closing list of `checks.md`.
5. [x] **Step 6, tests** — `tests/protocol/rip/Rfc2453*.test`, `Rfc2080*.test` and a helper
   `RipChecks.h`. One commit per group of tests. 30 tests in three commits; a generator in the
   session scratchpad wrote 28 of them from five shared network templates.
6. [x] **Step 7, run** — the suite, `model/rip/results.md` with the run record, the class of
   each failure and the model analysis. Fresh run at `7969452e2d`: 30 tests, 20 PASS, 10 FAIL,
   0 declared; six gaps of the model, all defects.
7. [x] **Steps 8 and 9, and the ledger** — `model/rip/conformance.md` part 2,
   `model/rip/categories.md`, `model/rip/coverage.md` with the statement table, the feature
   support, the achieved level and the pass log. One commit, with step 7. Level 2 reached;
   7 features confirmed, 6 partial, 6 unverified; 21 statements owed.
8. [ ] Gates, then move this plan to `plan/done/`.

## Facts found before the catalogs

The guide permits one look at the code before step 3: to select practical candidates, and to
answer the claim question. Nothing below enters the artifacts of steps 3 to 5.

- **The UDP port table has no entry for port 520 or 521**
  (`src/inet/common/ProtocolGroup.cc:157-164`). A RIP dissector is registered, but the dissector
  of a frame never reaches it, so a filter expression over `rip.*` cannot match. The tests read
  the RIP header with a helper, as the DHCP tests do (`DhcpChecks.h`, `findChunk`). The missing
  entry is a finding for `results.md`.
- The serializer writes IPv4 entries only, so a RIPng message has no byte form in the model.
  The RIPng message format (RFC 2080 §2.1, §2.1.1) is an `encoding` statement; its category is a
  serializer unit test.
- RIP imports static routes into its table, so a router with more than 25 static routes shows
  the limit of 25 entries in one message.
- The default timers are the values of the standard (30 s, 180 s, 120 s), so a test can keep
  them and observe the values; a simulation of a few hundred seconds is cheap.

## Decisions and facts found on the way

- **The first runs corrected three check procedures** (commit `649e737b07`): the answer to the
  table request of a starting router is a response that holds the marker too, so observation
  starts at 40 s; whether the network of a link counts as learned over it is not stated, so
  the periodic checks do not ask for it; the RIPng version observations go last, so that a
  wrong version hides nothing.
- **Two test errors found and fixed before any commit of the tests concerned:** the rate
  tests anchored their windows at a first step that could match at 20 s, and the RIPng
  request test read the zero prefix as `::` where the model holds an unspecified address.
- **IPv6 mockups need three settings** that the IPv4 ones do not:
  `**.ipv6.configurator.networkConfiguratorModule = "configurator"`, and on the configurator
  `addRemoteRoutes = false` with `addDefaultRoutes = false` instead of `addStaticRoutes =
  false`, which would also drop the on-link routes that RIPng imports.
- **Four RIPng findings**, all defects of the model, none declared: RIPng messages carry
  version 2 (two tests); the answer to a table request goes to a link-local address without
  its interface and never arrives; the IPv6 routing table does not restore the on-link route
  when the carrier returns, so a RIPng router never advertises its network again.
- **A passing expiry test passed by chance.** The code looks at the timeout only when it sends
  an update (`Rip.cc:537`, `551`), and the default random start put the updates of R2 1.3 s
  after those of R1. The expiry check now fixes the start of R2 15 s after R1, and it fails,
  15 s late. The garbage collection after the expiry became a check of its own, so that the
  late withdrawal does not hide it.
- **Reading the purge code found two more defects**, and two new checks for each document
  test them: a router never purges a network it lost itself (`checkExpiredRoutes` purges only
  learned routes), and a learned route is purged 300 s after the last entry from its next hop,
  which every repeated withdrawal refreshes. Together they keep every withdrawn network in the
  updates for good. 30 checks and 30 tests in the end.
- **RFC2453-ADDR-2, ADDR-3 and ADDR-4 are `later`, not `no check`:** the TODO of `Rip.h:77-78`
  claims the subnet rules of §3.7, and a check needs a version 1 router, level 5. MASK-2 and
  QRY-1 have no claim and stay `no check`.
- **The ledger table is generated** from a mapping of all 168 statements (scratchpad
  `gen-rip-ledger.py`), so no statement can miss a row.
