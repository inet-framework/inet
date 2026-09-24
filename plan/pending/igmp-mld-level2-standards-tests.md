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
2. [x] **Step 3, catalogs** — `standard/rfc9776/catalog.md`, `standard/rfc2236/catalog.md`,
   `standard/rfc9777/catalog.md`, `standard/rfc2710/catalog.md`. RFC 9776 293 entries, RFC 2236
   94, RFC 9777 307, RFC 2710 81; every quote checked against its lines.
3. [x] **Step 4, feature maps** — `protocol/igmp/features.md` (`IGMP-F-*`) and
   `protocol/mld/features.md` (`MLD-F-*`). With step 3 in one commit for each protocol. Both maps
   have 20 features; the MLD features are the twins of the IGMP features, with
   MLD-F-LISTENING-STATE for IGMP-F-GROUP-MEMBERSHIP and the version 1 features for the version
   2 ones.
4. [x] **IGMP step 5, checks** — `protocol/igmp/checks.md` and `protocol/igmp/checks/*.md`.
   32 checks in 6 files; 260 statements are in a check, and the other 127 are in the closing
   list with what a check would need. Message validation is the one mandatory feature without a
   core check: all its statements need a crafted message.
5. [x] **IGMP step 6, tests** — `tests/protocol/igmp/Rfc9776*.test`, `Rfc2236*.test`, and a helper.
   42 tests for the 32 checks, in `tests/protocol/igmp/`, with the helper `IgmpChecks.h`. First
   run: 25 PASS, 2 FAIL declared expected (features that the model does not have), 15 FAIL.
6. [x] **IGMP steps 7 to 9, and the ledger** — `model/igmp/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`. Fresh run at `bb20f0dd5e`: 42 tests, 25 PASS, 17 FAIL, one
   of them declared expected. Thirteen gaps: twelve defects and one missing feature. Level 2 is
   reached; the matrix holds 5 `confirmed`, 13 `partial`, 1 `unverified` and 1 `out of claim`;
   49 statements are owed.
7. [x] **MLD step 5, checks** — `protocol/mld/checks.md` and `protocol/mld/checks/*.md`.
   32 checks in 6 files, the twins of the IGMP checks; 278 statements are in a check, and the
   other 110 are in the closing list. One check has an observation that its IGMP twin has not:
   the Reports for the solicited-node addresses, the second half of RFC9777-LSN-3.
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
- **The requests of a host come from a test module**, `IgmpRequests` in `IgmpChecks.h`: a
  script of timed requests calls `NetworkInterface::changeMulticastGroupMembership`, which is
  what a socket does. One module serves every host and every filter mode.
- **The multicast route of R comes from a test module**, `MulticastLeafRoute`: one route from
  L2 to L1, with L1 as a leaf. The configurator makes no leaf routes, and PIM-DM in the mockup
  did not forward ("source is not directly connected").
- **The lines of a test come before the common ini lines.** The first matching line wins, and
  the common lines have wildcards: `**.ipv4.igmp.typename = "Igmpv3"` before a line for one node
  kept every node at IGMPv3.
- **No check stops a node.** `Igmpv2` and `Igmpv3` have no lifecycle: a node that starts down
  stops the simulation at initialization ("Tag 'inet::Ipv4InterfaceData' is absent"), and a
  crashed node keeps its IGMP timers. A node joins or leaves L1 through its link instead: the
  scenario disables or enables the channel between the node and the switch. A channel that is
  disabled at initialization never comes back, because `EthernetMacBase` subscribes to the
  channel only when the channel is enabled (`EthernetMacBase.cc:428`); so every link starts
  enabled, and a check that needs a router off L1 at the start breaks the link at 1 second.
  The rule is in `checks.md`.
- **One Report starts one query sequence.** Where a check reads the Queries that a leave starts,
  the host that leaves has a Robustness Variable of 1: each repetition of its Report would start
  the sequence again (RFC 9776 §6.6.3.1). The rule is in `checks.md`.
- **"At the instant of the Query" has a margin of 1 millisecond.** The IGMPv2 mode of the model
  answers 12 microseconds after the Query, which a strict comparison accepted. The rule is in
  `checks.md`.
- **Check corrections from the runs**, each against the text of the standard: the second
  Group-and-Source-Specific Query has the S flag set when an answer came before it (RFC 9776
  §6.6.3.2); the Query Interval of "Timer relations" comes from the instants of the Queries,
  because the QQIC has its own check; "Membership timeout" cites RREP-11, not RREP-9; the check
  of the General Query answer has a fifth observation, no record for a group without reception
  state (HQRY-10); the Older Version Querier Present Interval is 350 seconds, as §8.12 says ("10
  times the Max Response Time"), and has its own observation.
- **Splits**: a check gets a second test where a failure would hide another verdict — the
  precedence, the S flag of the two query kinds, the repetition count of a join, the tail of the
  merged repetitions, the QRV, the blocked source, the IGMPv2 repetition, the interval of the
  older querier, and the answer without state.
- **Findings outside the checks**: the `Igmpv2` router stops the simulation on a Version 3
  Report ("Unhandled message type (34)"), where RFC 2236 §2 ignores unrecognized types; the
  check of the mode change keeps R off L1 while A sends its Version 3 Report. The IGMP modules
  have no lifecycle. A channel disabled at initialization never comes back.
- **Source-specific forwarding is a defect, not a missing feature.** The first design declared
  it expected. The question of the guide, "does code exist for this specific behavior?", finds
  both halves: `Igmpv3` stores the forwarded sources in the interface data, and the interface
  data answers the question for one source; only the forwarding asks for the group. Commit
  `bb20f0dd5e` removed the declaration.
- **`internal` statements get protocol tests.** 26 checks hold a statement of the class
  `internal`, whose usual category is a module test. IGMP state shows on the link — in the
  Reports, the Queries and the forwarded datagrams — so the checks read it there;
  `categories.md` records the decision.
- **The ledger comes from a script** (scratchpad `gen-igmp-ledger.py`, with the verdict table
  from `gen-igmp-verdicts.py`), as for ND: the statement tables, the feature support and the
  debt table are generated, and the prose is written around them.
