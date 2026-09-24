# IGMP and MLD level 2 — catalogs, feature maps, checks and tests for RFC 9776, RFC 2236, RFC 9777, RFC 2710

**Status:** in progress. Started 2026-09-24 on `topic/standards-tests-igmp-mld-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-igmp-mld-level2`.

The third pass of wave 1, after RIP and ND. IGMPv3 and MLDv2 are twins: RFC 9776 and RFC 9777
came out together and describe the same state machines for two address families, so this pass
does both, IGMP first and MLD after it with the same structure. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md),
steps 3 to 9, at **level 2, Core**. The in-scope sets are fixed in
[`igmp/standards.md`](../../doc/project/evidence/protocol/igmp/standards.md#in-scope-set) (RFC 9776
§4 to §8, RFC 2236 §6 and §7) and
[`mld/standards.md`](../../doc/project/evidence/protocol/mld/standards.md#in-scope-set) (RFC 9777 §5
to §9, RFC 2710 §5 to §7).

The user's decision of 2026-09-24 holds here too: the pass measures the model and repairs
nothing; the failures are repaired later.

Commit group: `igmp-mld-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.

## What the pass takes over from the RIP and ND passes

- The catalogs are drafted in parallel by agents with one brief, and a script checks every quote
  against its line reference; the drafts are then merged, reviewed and indexed.
- The feature maps come from a script that must place every entry.
- A script checks that every catalog entry is in exactly one check or in the closing list of
  `checks.md`.
- Tests come from a generator with shared mockups, and the ledger table from a generator with a
  hand mapping only for the statements of the failing tests.
- An exploration test (a copy of a test that prints every message; never committed) shows the
  cause of a failure before the failure counts as a finding.
- A known-failing observation goes last, or into a test of its own; a check that is stricter
  than the standard is a test error.

## Steps

1. [x] **Plan** — this file.
2. [ ] **Step 3, catalogs** — `standard/rfc9776/catalog.md`, `standard/rfc2236/catalog.md`,
   `standard/rfc9777/catalog.md`, `standard/rfc2710/catalog.md`.
3. [ ] **Step 4, feature maps** — `protocol/igmp/features.md` (`IGMP-F-*`) and
   `protocol/mld/features.md` (`MLD-F-*`). With step 3 in one commit for each protocol.
4. [x] **IGMP step 5, checks** — `protocol/igmp/checks.md` and `protocol/igmp/checks/*.md`.
   32 checks in 6 files; 260 statements are in a check, and the other 127 are in the closing
   list with what a check would need. Message validation is the one mandatory feature without a
   core check: all its statements need a crafted message.
5. [ ] **IGMP step 6, tests** — `tests/protocol/igmp/Rfc9776*.test`, `Rfc2236*.test`, and a helper.
6. [ ] **IGMP steps 7 to 9, and the ledger** — `model/igmp/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`.
7. [ ] **MLD step 5, checks** — `protocol/mld/checks.md` and `protocol/mld/checks/*.md`.
8. [ ] **MLD step 6, tests** — `tests/protocol/mld/Rfc9777*.test`, `Rfc2710*.test`, and a helper.
9. [ ] **MLD steps 7 to 9, and the ledger** — `model/mld/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`.
10. [ ] Gates, then move this plan to `plan/done/`.

## The catalog drafts

Six agents, one brief (scratchpad `igmp-mld-catalog-brief.md`), one range each:

| Draft | Range | Areas |
| --- | --- | --- |
| IGMP 1 | RFC 9776 §4 and §8, lines 444-955 and 2009-2192 | GEN, QRY, REP, TIMER |
| IGMP 2 | RFC 9776 §5 to §7, lines 956-2008 | HOST, HQRY, RQ, RST, FWD, RREP, RSW, RQRY, VER, COMPH, COMPR |
| IGMP 3 | RFC 2236 §6 and §7, lines 370-934 | HOST, ROUTER |
| MLD 1 | RFC 9777 §5 and §9, lines 734-1335 and 2515-2703 | GEN, QRY, REP, TIMER |
| MLD 2 | RFC 9777 §6 to §8, lines 1336-2514 | LSN, LQRY, LTIM, RQ, RST, FWD, RREP, RSW, RQRY, VER, COMPL, COMPR |
| MLD 3 | RFC 2710 §5 to §7, lines 377-982 | NODE, ROUTER, TIMER |

## Facts found before the catalogs

- The worktree build is a copy of the ND worktree build (same `src` tree `5c4f41c600`): `make`
  relinks the library and compiles nothing.
- The level 1 passes left four facts for each protocol to check
  ([`igmp/conformance.md`](../../doc/project/evidence/model/igmp/conformance.md),
  [`mld/conformance.md`](../../doc/project/evidence/model/mld/conformance.md)): an unrecognized
  type that stops the simulation, no Router Alert option, a stale comment, and the Group
  Membership Interval with the formula of the older documents.

## Decisions and facts found on the way
- **The six catalog drafts** came from six agents with one brief (scratchpad
  `igmp-mld-catalog-brief.md`): RFC 9776 with 105 and 188 entries, RFC 2236 with 94, RFC 9777
  with 103 and 204, RFC 2710 with 81, every quote checked against its lines. The merge script
  normalizes the strength of each lowercase keyword to the form `must (lower case)`, also where
  an agent had written a plain `must` for a lower-case word; a quote fragment of a sentence with
  a capital keyword keeps the plain form.
- **The IGMP feature map** has 20 features for the 387 entries. A new refinement: a keyword of
  the base document can make a companion behavior mandatory; the two version 2 features take
  their level from RFC9776-COMPH-1 and COMPR-13, which require IGMPv2 compatibility mode.
- **Stimuli that the model offers without a special tester** (the one look at the code before
  the checks): a UDP sink joins a group, with a source list for INCLUDE mode, and leaves at its
  stop time; a router of the older version sends the older queries.
- **The `Checks:` lines come from a script** (scratchpad `fill-checks-lines.py`): a check file
  holds only the IDs, and the script adds the strength of each ID from the catalog, so no label
  can drift from its entry.
