# IPsec level 2 — catalogs, feature map, checks and tests for RFC 4301, RFC 4302, RFC 4303

**Status:** in progress. Started 2026-09-24 on `topic/standards-tests-ipsec-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-ipsec-level2`.

The fourth pass of wave 1, after RIP, ND, and IGMP with MLD. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md),
steps 2 to 9, at **level 2, Core**. The level 1 pass of wave 0 wrote the standards map and part 1
of the conformance document; this pass starts from them.

The user's decision of 2026-09-24 holds here too: the pass measures the model and repairs
nothing; the failures are repaired later.

Commit group: `ipsec-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.
The pass delivers every output of the section "What a pass delivers" of the guide. That section
is on `topic/standards-tests-igmp-mld-level2` and not yet on this branch; the steps below follow
it.

## What the pass takes over from the earlier passes

- The catalogs are drafted in parallel by agents with one brief, and a script checks every quote
  against its line reference; the drafts are then merged, reviewed and indexed.
- The feature map comes from a script that must place every entry, and a script checks that every
  entry is in exactly one check or in the closing list of `checks.md`.
- Tests come from a generator with shared mockups, and the ledger table from a generator.
- An exploration test (never committed) shows the cause of a failure before it counts as a finding.
- A known-failing observation goes last, or into a test of its own; every observation must be able
  to fail in its scenario; a check that is stricter than the standard is a test error.
- `notes.md` gets each lesson when it occurs.

## Steps

1. [x] **Plan** — this file.
2. [x] **Step 2, the standards map** — `protocol/ipsec/standards.md`: target level 2, and the
   sections that level 2 needs beyond the level 1 list.
3. [x] **Step 3, catalogs** — `standard/rfc4301/catalog.md`, `standard/rfc4302/catalog.md`,
   `standard/rfc4303/catalog.md`; every quote checked against its lines. RFC 4301 370 entries,
   RFC 4302 190, RFC 4303 237: 797 in all.
4. [x] **Step 4, feature map** — `protocol/ipsec/features.md` (`IPSEC-F-*`). With step 3 in one
   commit. 29 features: 21 mandatory, 7 optional, 1 unstated; every entry is placed.
5. [ ] **Step 5, checks** — `protocol/ipsec/checks.md` and `protocol/ipsec/checks/*.md`, with
   the closing list of the statements without a check.
6. [ ] **Step 6, tests** — `tests/protocol/ipsec/Rfc430[123]*.test` and the helper header
   `IpsecChecks.h`.
7. [ ] **Steps 7 to 9, and the ledger** — `model/ipsec/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`, from one fresh run.
8. [ ] **Notes** — `model/ipsec/notes.md`: the model quirks, the scenario and tooling traps, and
   the follow-ups with every gap by number.
9. [ ] Gates, then move this plan to `plan/done/`.

Working scripts: `audit/ipsec-level2/` in `inet-master` (outside git), with a `README.md`.

## Decisions and facts found on the way

- **The in-scope sections grow for level 2.** The level 1 list left out the SA and its two modes
  (RFC 4301 §4 to §4.3, with "A host implementation of IPsec MUST support both transport and
  tunnel mode", `rfc4301.txt:873-875`), the introduction to the databases (§4.4), the location
  of each header (§3.1 of RFC 4302 and RFC 4303) with the algorithms (§3.2), and the conformance
  sections (RFC 4301 §10, RFC 4302 §5, RFC 4303 §5). `standards.md` now names them, with the
  target level 2.
- **The RIP, ND, IGMP and MLD passes left their target level at 1.** Their `standards.md` still
  said "Level 1 — Survey" and "none yet" for the catalogs. Corrected on their branches:
  `3396c80ea1` (RIP), `03f8e313c5` (ND), `8dba80c00a` (IGMP and MLD).
- **The catalogs are drafted by seven agents** with one brief (scratchpad
  `ipsec/ipsec-catalog-brief.md`): RFC 4301 in four ranges, RFC 4302 in one, RFC 4303 in two.
- **The merge normalizes the drafts.** The agents recorded a keyword that the text writes in lower
  case in two ways, as `description` or as the keyword itself; the merge writes `must (lower
  case)` for both, as the other catalogs do. It also writes each lead on one line with its
  keywords in lower case, and puts a Strength line that a draft wrapped back on one line — a
  wrapped line hid RFC4302-SPI-7 from a listing and would hide it from a generator.
- **AH is optional, ESP mandatory.** RFC 4301 says "MUST support ESP" and "MAY support AH"
  (RFC4301-OVW-5), and RFC 4302 makes all of AH a must for an implementation that offers it
  (RFC4302-CONF-1). The two AH features are `optional`; each rule inside them is a must once AH
  is there.
- **Four features depend on key management**, which the in-scope set leaves to level 5:
  SA creation with the PFP flags and lifetimes, named SPD entries, the anti-replay window's
  notification, and the negotiation of ESN and TFC padding. Their statements stay in the catalog
  and in the map; the checks decide what a statically keyed implementation can show.
