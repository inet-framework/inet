# MPLS level 2 — catalogs, feature map, checks and tests for RFC 3031, RFC 3032, RFC 3443, RFC 5462

**Status:** in progress. Started 2026-09-24 on `topic/standards-tests-mpls-level2`, from
`origin/master` at `7772a7e4ef`. Worktree: `/home/levy/workspace/inet-standards-tests-mpls-level2`.

The fifth and last pass of wave 1, after RIP, ND, IGMP with MLD, and IPsec. It follows
[`derive-tests-from-a-standard.md`](../../doc/project/guide/derive-tests-from-a-standard.md),
steps 2 to 9, at **level 2, Core**. The level 1 pass of wave 0 wrote the standards map and part 1
of the conformance document; this pass starts from them.

The user's decision of 2026-09-24 holds here too: the pass measures the model and repairs
nothing; the failures are repaired later.

Commit group: `mpls-standards-tests`. Gates before each commit: `check-links.sh`,
`check-seals.sh`, and `check-commits.sh` and `check-classification.sh` on `origin/master..HEAD`.
The pass delivers every output of the section "What a pass delivers" of the guide (on
`topic/standards-tests-igmp-mld-level2`, not yet on this branch), `notes.md` included.

## What the pass takes over from the earlier passes

- Catalogs drafted by agents with one brief, every quote checked, drafts merged with the lead
  and strength normalization of the IPsec merge.
- A feature map and a closing list from generators that refuse an unplaced entry.
- Tests from a generator with shared mockups; the exploration dump before a failure counts.
- A count with an assertion matches once; a delivered datagram is seen at `<host>.udp`
  (`packetSentToUpper`); IPsec-like per-node parameters go to the right nodes only.
- `notes.md` gets each lesson when it occurs; `standards.md` gets the target level of the pass.

## Steps

1. [x] **Plan** — this file.
2. [ ] **Step 2, the standards map** — `protocol/mpls/standards.md`: target level 2, and the
   sections that level 2 needs beyond the level 1 list.
3. [ ] **Step 3, catalogs** — `standard/rfc3031/catalog.md`, `standard/rfc3032/catalog.md`,
   `standard/rfc3443/catalog.md`, `standard/rfc5462/catalog.md`; every quote checked.
4. [ ] **Step 4, feature map** — `protocol/mpls/features.md` (`MPLS-F-*`).
5. [ ] **Step 5, checks** — `protocol/mpls/checks.md` and `protocol/mpls/checks/*.md`, with the
   closing list.
6. [ ] **Step 6, tests** — `tests/protocol/mpls/Rfc30*.test`, `Rfc3443*.test`, and the helper
   header `MplsChecks.h`.
7. [ ] **Steps 7 to 9, and the ledger** — `model/mpls/results.md`, `conformance.md` part 2,
   `categories.md`, `coverage.md`, from one fresh run.
8. [ ] **Notes** — `model/mpls/notes.md`, with every gap by number in the follow-ups.
9. [ ] Gates, then move this plan to `plan/done/`.

Working scripts: `audit/mpls-level2/` in `inet-master` (outside git), with a `README.md`.

## Decisions and facts found on the way

- **The in-scope sections grow for level 2.** RFC 3031 adds the labeled packet (§3.3), the LSP
  with its ingress and egress (§3.15) and the Implicit NULL label (§4.1.5), which §3.16 needs.
  RFC 3032 adds fragmentation and path MTU (§3, nine MUST lines) and the encapsulation on LAN
  media (§5). RFC 3443 takes §2 and §3 whole; RFC 5462 takes §2.1 and §3.
- **The catalogs are drafted by three agents** (scratchpad `mpls/mpls-catalog-brief.md`): RFC
  3031, RFC 3032, and RFC 3443 with RFC 5462.
- **The model's own static ingress binding serves the tests.** `RsvpClassifier` binds a
  destination to a label of the LIB without RSVP signaling (`<fecentry>` with a `<label>`,
  `RsvpClassifier::readItemFromXML`), and `LibTable` reads a static LIB, as the example
  `examples/mpls/testte_tunnel` does. The tests need no module of their own for the FTN.
