# Resolve the audit findings of PR #1148

Status: **complete** — the repairs, approved reconstruction, per-commit checks,
feature builds, final union and middle-commit checks are complete.
Prepared: 2026-09-14.
PR: [INET #1148](https://github.com/inet-framework/inet/pull/1148).
Audit evidence: local, ignored `audit/pull-request/pr-1148.md`,
`pr-1148-summary.md`, and `pr-1148-evidence/` beside them.

Reviewed head: `991f626a4f5a031d5b507f6ff430cca36fb794ce`.
Reviewed merge base: `cbbba4d487649cfbef8bc4d7c225452699eb563c`.
Branch: `cleanup/fix-ieee80211-addba-transaction-minimal`.

## 1. Outcome and order

Make the ADDBA transaction series safe under synchronous queue callbacks, complete its logical
queue-departure contract, and produce a tested, reviewable series against a pinned upstream base.
The audit reproduced two simulator crashes and one missing departure notification; the existing
eight focused tests passed and therefore do not cover these failures.

Execute in this order: preserve the reviewed checkpoint → repair F1 → repair F2 → repair interface
contracts → validate the repaired checkpoint → rebase in isolation → attribute baselines and
reconstruct the final series → verify and refresh the review artifacts.

| Finding | Required change | Completion evidence |
| --- | --- | --- |
| F1, P1 | Make both agreement-expiry loops safe when callbacks erase or replace agreements | Both crash regressions pass; real HCF queue-overflow regression passes |
| F2, P2 | Publish exactly one logical departure for packets rejected before a compound queue's leaf queue | Deterministic RED regression and HCF transaction cleanup regression pass |
| S1 | Attribute each fingerprint transition and place it with its causal source change | Per-row evidence and passing intermediate commits |
| S2 | Split C12's null BAR result guard from its recipient timeout-policy correction | Independently testable commits |
| S3 | Supply C17's missing classification trailer | Valid `Change:` trailer; classify all newly authored commits |
| S4 | Remove newly introduced interface method bodies | Explicit implementations; interface scans and feature builds |
| S5 | Give queue-removal reasons explicit stable values | Existing numeric values preserved; departure tests pass |
| S6 | Restore unrelated whitespace hunks | Per-commit whitespace inspection |

Subject scope/kind prefixes are optional under
[CR-TAG-SUBJECT](../../doc/project/rule/classification.md#cr-tag-subject).
S3 does not require adding a subject prefix. C01–C16 predate that classification rule; their
original missing trailers are not retroactive findings.

## 2. Preserve evidence and establish the starting tree

- [x] Record HEAD, base, working-tree state, toolchain, build mode and feature configuration.
  Preserve a local ref to the reviewed head before any subsequent history operation.
- [x] Keep the existing audit and failing-probe artifacts intact. Store new results in a separate
  directory keyed by the candidate commit and phase.
- [x] Check the current seal registry for the actual repair paths. The audited diff touches no
  sealed source path; expanding the repair surface requires checking again.
- [x] Turn the diagnostic cases into small maintained regressions. Avoid copying the probes'
  large ADDBA harness wholesale. Each regression must demonstrate its intended failure on the
  reviewed head before applying its repair.

## 3. Repair F1: agreement expiry under synchronous callbacks

Primary files: `src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.cc`
and `RecipientBlockAckAgreementHandler.cc` in the same directory; focused unit/module tests.

- [x] Snapshot expired agreement identities as values: peer, TID and generation. Do not retain
  map iterators or agreement pointers as the work list.
- [x] Before processing each identity, look it up again and confirm generation, state and expiry.
  Skip entries deleted, replaced or made ineligible by an earlier callback.
- [x] Inspect every outward call in the expiry path, including originator
  `releaseBlockAckAgreementFrames` and both roles' `processMgmtFrame`. Capture necessary values
  before the call; relookup before any subsequent access to live agreement state.
- [x] Preserve the generation attached to DELBA. A callback that installs a replacement agreement
  at the same peer/TID must not cause teardown of that replacement.
- [x] Preserve shared inactivity-timer scheduling after callbacks alter the agreement collection.
  Advancing the iterator before calling out is insufficient: a callback may erase the next entry.

Tests and acceptance:

- [x] Both roles: the existing expiry probes' erase-current callback completes without a crash.
- [x] Two expired agreements: erase a sibling during the first callback; process each surviving
  eligible generation at most once. Also replace the same peer/TID with a new generation.
- [x] Mix expired, future and disabled deadlines; verify teardown selection and the next shared
  timer deadline, including removal of the final agreement.
- [x] Use real HCF and a bounded queue to drop the generated DELBA synchronously. Assert terminal
  cleanup, absence of stale agreement use, and continued simulation progress for both roles.

## 4. Repair F2: compound queue rejection notifications

Primary files: `src/inet/queueing/queue/CompoundPacketQueueBase.{h,cc}`, the relevant queueing
contracts, queueing tests, and IEEE 802.11 module tests. `RedDropperQueue` is declared in
`src/inet/queueing/queue/RedMarkerQueue.ned` and provides the deterministic reproduction.

- [x] Write a source/ownership matrix for leaf queue departures, prequeue filter drops,
  compound-owned overflow, nested compounds and shared-buffer eviction. Identify which source
  owns each packet and which component emits the one logical departure.
- [x] Extend the compound notification path to include drops from nonqueue components outside
  the nearest child-queue boundary. Resolve emitters through generic contracts/composition; keep
  IEEE 802.11 tags and concrete RED types out of generic queueing logic.
- [x] Forward a borrowed packet only while it is alive and before deletion. Do not translate every
  bubbling `packetDropped` indiscriminately: leaf and shared-buffer cases can already emit a
  departure. Preserve listener subscription/cleanup and existing statistics semantics.
- [x] Verify HCF/DCF consume the corrected event exactly once. Change consumer code only if the
  production regression exposes a separate defect.

Tests and acceptance:

- [x] RED: capacity 1, `minth=0`, `maxth=1`, `wq=1`, `maxp=0`; enqueue twice without dequeue.
  Assert one drop and one logical departure with the same packet identity and `DROPPED` reason.
- [x] Leaf drops, nested queues, shared-buffer eviction, selective extraction and ordinary dequeue
  retain exactly-once departure semantics at each exposed logical queue boundary.
- [x] Real HCF with RED in the management access-category queue: reject a pending tagged ADDBA
  request. Assert transaction removal, no response timer armed for the unsent request, no stale
  eligibility entry, and resumed same-TID data eligibility under the configured policy.
- [x] Exercise terminal AP management outcomes through DCF and HCF; assert one outcome per
  transaction and no duplicate cancellation/drop handling.

## 5. Repair interface and numeric contracts

- [x] S4: make the new methods pure in `IBlockAckAgreementHandlerCallback`,
  `IFrameSequenceHandler`, `IOriginatorBlockAckAgreementHandler`,
  `IRecipientBlockAckAgreementHandler`, and nested `IPacketBuffer::ICallback`.
- [x] Inventory every in-tree implementor and call site. Put required behavior in concrete
  implementations, including test doubles; introduce a shared implementation base only where
  actual reuse justifies it. Document any external implementor migration obligation.
- [x] Keep generic queueing contract changes separate from IEEE 802.11 contracts. Leave the
  pre-existing ledgered `ITransmitStep`/`IReceiveStep` defaults outside this repair.
- [x] S5: assign `DEQUEUED=0`, `REMOVED=1`, `DROPPED=2` explicitly in
  `src/inet/queueing/contract/IPacketQueue.h`; preserve those values in future changes.
- [x] Compile affected implementations with the relevant optional features both enabled and
  disabled. Run the interface scan and manually inspect nested callbacks, which the audit's
  scanner did not detect.

## 6. Proposed change units and final placement

These are repair units, not a prescription to append fixups to the published series. During final
reconstruction, incorporate each correction and its regression into the commit introducing the
contract. Follow [PR-SERIES-ORDER](../../doc/project/rule/pull-request.md#pr-series-order).

| Proposed subject | Single decision and rationale | Dependency / surface | Evidence and expected effect |
| --- | --- | --- | --- |
| `queueing: report prequeue rejection as a logical departure` | Complete the generic departure contract for rejected input | Existing departure API; compound queue and queueing tests; fold into C01 | RED plus composition tests; one formerly missing event, possible fingerprint movement |
| `queueing: require explicit buffer callback implementations` | Remove implicit behavior from the new callback contract | Buffer API introduction; contract and all implementors; fold into introducing unit | Interface inventory, feature builds, queueing tests; preserve behavior |
| `queueing: assign stable packet removal reason values` | Stabilize externally observed enum values | Departure API; `IPacketQueue.h`; fold into C01 | Numeric inspection and departure tests; no behavior/baseline movement |
| `ieee80211: clear transactions after compound queue rejection` | Establish cleanup through the complete producer contract | Generic departure repair; C02 consumer and later ADDBA/module coverage placed where prerequisites exist | Real RED/HCF and AP terminal-outcome tests; production edits only if needed |
| `ieee80211: make agreement expiry safe under queue callbacks` | Preserve generation identity across reentrant expiry callbacks | Agreement-generation/teardown introduction; both handlers and their tests | Two crash regressions, replacement tests and HCF overflow; possible trajectory movement |
| `ieee80211: require explicit transaction callback implementations` | Make new MAC contract obligations explicit | Respective interface introductions and every implementor | Interface scan, feature builds and transaction tests; preserve behavior |
| `ieee80211: reject null BAR defragmentation results` | Stop dispatch after unsuccessful BAR reassembly | Split source/test subset of C12 | Direct null-result regression; determine fingerprint effect from affected receive paths |
| `ieee80211: honor recipient Block Ack timeout policy` | Correct timeout policy selection | Independent source/test subset of C12 | Test both policy branches; determine timer/fingerprint effect |

Retain unaffected original decisions in their dependency order. Do not commit failing tests before
their fixes. Add this plan to history before using
`Plan: plan/pending/pr-1148-resolve-audit-findings.md` in implementing commits. Write valid
`Change:` trailers from each actual diff, and record reproduction, rationale and evidence in the
messages. The final commit count follows the resulting decisions, not a target of 17.

## 7. Validate, rebase and attribute expectations

- [x] Build a fresh debug library and run the focused tests on the repaired original-base tree.
  Preserve that tested checkpoint before rebasing.
- [x] Pin the then-current upstream target. The audit observed a merge conflict against
  `0c85e5dd6bc2969987286210962983b19384670e`; do not assume that is still the target.
  Resolve the rebase in an isolated candidate, using the INET rebase workflow and scoped
  `opp_repl` evidence. Record conflict decisions and rerun affected tests after resolution.
- [x] Inventory the 16 fingerprint rows added/changed by the audited series, then recompute the
  inventory against the pinned new base. Map changed production paths through dependency data
  to affected configurations, runs and seeds; include all declared ingredients (`tplx`, `~tNl`,
  `~tND` where applicable).
- [x] For each moving row, record configuration/run, old and candidate values, ingredient,
  causal source commit, observed behavioral difference, correctness argument and artifact paths.
  Multiple transitions of the same row need evidence at each responsible commit.
- [x] Investigate mismatches before replacing expectations. Present the exact proposed baseline
  changes for acceptance under [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md).
  Candidate calculations remain evidence until the required approval is recorded.
- [x] Reconstruct the final series on the fixed new base using the INET branch-cleanup workflow.
  Split C12, fold each accepted baseline transition into its causal commit, remove standalone
  C14, restore S6's incidental whitespace and complete S3's trailers. Do not squash all baseline
  changes into C13 merely because it precedes C14.
- [x] Verify each final intermediate tree with its directly applicable build/tests and scoped
  fingerprints. Compare the reconstructed final source tree with the tested post-rebase tree;
  account explicitly for intended whitespace, documentation and baseline differences.

## 8. Final verification and handoff

Use the active test/build skill interfaces when executing. The existing focused filters are:

```bash
inet_run_unit_tests -m debug -f '(Ieee80211AddbaTransaction_1|Ieee80211MgmtFrameSerializer_1|Ieee80211MgmtTransactionTag_1)\.test$'
inet_run_module_tests -m debug -f '(Ieee80211BlockAckInactivityTimer_1|Ieee80211MgmtApCancellation_1|Ieee80211MgmtApHcfQueueDrop_1|Ieee80211MgmtApQueueDrop_1)\.test$'
inet_run_queueing_tests -m debug -f 'PacketQueueDepartureSignal_1\.test$'
```

- [x] Add explicit filters for the new regressions and retain commands, mode, seeds, exit status
  and logs. Repeat tests after relevant changes; do not reuse pre-rebase results as final evidence.
- [x] Complete debug and release compilation and the affected feature matrix using the supported
  build workflow. Run source-seal, architecture, interface, naming, classification, commit and
  whitespace gates as described in [run-the-gates.md](../../doc/project/guide/run-the-gates.md).
- [x] Resolve the audit's usage/override inventory and affected standards-traceability gaps.
  Inspect authoritative local clauses for claims this series makes; record unavailable evidence
  explicitly rather than marking it passed. Keep pre-existing ledgered findings separate.
- [x] Refresh the audit and commit summary against the final exact head/base. Close F1/F2 only
  with the production-path tests above; close S1 only with per-row and per-commit evidence.
- [x] Correct the PR description: malformed AID handling is
  `Ieee80211MgmtFrameSerializer::decodeAssociationId` calling `markIncorrect`; the teardown tag
  carries `generationId`, while role/peer/TID come from DELBA/context. Include actual reading
  order, architectural/API surface, baseline causes and verified test scope.
- [x] Hand off the final diff, revised description and evidence table for review. Move the plan
  to `plan/done/` only after its required repairs and verification are complete.

Completion means both reproduced defects are covered and repaired, all six policy findings are
resolved, the series builds and passes its applicable tests at every commit, and the final audit
clearly distinguishes verified claims from any remaining evidence limitations.

## Historical execution checkpoint — 2026-09-15

Reviewed head preserved at `archive/pr-1148-reviewed-991f626a4f`. The original-base
repair checkpoint is `d3045c9780`, preserved at
`archive/pr-1148-repaired-original-base`. Repair commits: `6adbd1d53f` (F2/S5),
`75a0e20b24` (F1), `fec8ad0ba6` (generic callback purity), `d3045c9780` (MAC purity).

New maintained regressions:

- `Ieee80211OriginatorExpiryReentrant_1` and `Ieee80211RecipientExpiryReentrant_1`:
  crash on reviewed implementations; pass with repairs.
- `Ieee80211HcfAgreementQueueDrop_1`: real HCF expiry plus bounded management
  queues crashes on reviewed implementations; repaired version passes both roles.
- `Ieee80211HcfAddbaRedDrop_1`: reviewed compound queue leaves the pending ADDBA;
  repair removes it without arming a response timer and restores data eligibility.
- `CompoundPacketQueueRedDeparture_1` and
  `CompoundPacketQueueCompositionDeparture_1`: deterministic rejection identity,
  reason and nested/leaf/shared-buffer/dequeue/removal coverage pass.

The existing five unit tests (including the two new expiry tests), six module
tests (including the two new HCF tests) and three queueing tests pass on the
repaired original-base tree. Debug build passes; release/feature builds ongoing.
Scoped architecture gates pass. Interface scan retains only `AV-CONTRACT-02`.

Upstream master was verified via GitHub and pinned to
`0c85e5dd6bc2969987286210962983b19384670e`. Isolated replay at
`/tmp/inet-pr1148-rebase`, branch `repair/pr-1148-rebase-candidate`, reaches
`3eb505006b9966648c1d46b7cccd8d9d965669f5`. Only WHATSNEW conflicted; all repaired
production and focused-test paths are identical to the original-base checkpoint.
Candidate build/testing and release-note numbering normalization are pending.

Evidence: local ignored `audit/pull-request/pr-1148-repair/991f626a4f/` and
`ai-logs/executions/2026-09-14_pr1148-rebase.md`. The original audit/probes remain
intact. No new fingerprint expectation has been accepted or written. S1, final
C12 split, S3 classification, S6 restoration, per-commit verification, and final
review handoff remain open. Do not move this plan to done at this checkpoint.

## Historical diagnostic checkpoint — 2026-09-15 01:25

All 17 source prefixes build in debug and release and pass their available
focused cases (202 case executions). All 994 declared fingerprint ingredients
were measured, with each moving candidate reproduced. Final scope: 14 focused
cases and 59 ingredients. All final values equal the repaired rebase oracle.
C03 response-token echo was moved alongside originator validation; C12 is split;
C14 baseline-only changes are distributed to their causes. The exact baseline
and 20-commit history proposals are in the local ignored repair evidence.
No tracked expectations have been updated. Explicit acceptance, final history
authoring, per-commit verification and final handoff remain pending.

## Accepted reconstruction — 2026-09-15

The user accepted the exact baseline and ordered 20-commit proposals with “yes”.
Apply inherited upstream graphical drift in a prerequisite baseline commit,
then this plan, then the 17 measured source decisions with their causal
expectations and migration notes, and finally the verified closure. Preserve
the original topic and all diagnostic checkpoints.

## Final verification — 2026-09-15

The user accepted the exact baseline and history proposals. The reconstructed
source series is verified through `ab2531bf32c20fe6fd7efb0299ce8ea00058907e`, on pinned
base `0c85e5dd6bc2969987286210962983b19384670e`. The preserved original topic remains
`37c6119f7ee6a090b2aa0999d1d9ffc3dca94144`.

All 19 prerequisite/plan/source commits have passing debug/release and scoped
verification. The final union passes five unit, six module and three queueing
cases plus 59 fingerprint ingredients across 16 mapped rows. Middle checks
cover C03 transactions/RED, C08 reentrant expiry/HCF overflow and C13 A-MSDU
validity. Queueing-only and Wi-Fi-required debug builds pass.

Every source-owned baseline transition is in its causal commit; the ten
inherited graphical tokens are in the approved upstream-control prerequisite.
C12 is split, C14 is removed, C17 and the whole series have classification
trailers, incidental whitespace is restored, and new public contracts have
migration-guide and release notes. The final source matches the tested rebase
except for the planned whitespace and three verified figure-reference comments.

Full-tree architecture/interface/naming gates retain the same 25/15/23 findings
as pinned upstream, with no new findings. Scoped architecture, queue interfaces,
changed NED/MSG declarations, source seals, history and whitespace checks pass.
The two existing Wi-Fi interface defaults remain ledgered by AV-CONTRACT-02.
The source-based usage inventory replaces the unavailable historical helper;
external implementations and every optional IEEE procedure are not certified.

The completion record and its pure move are separate under PR-SPLIT-MOVE, so
the final series contains 21 commits without changing the accepted source
order or baseline scope. Local final audit, commit summary, revised PR
description, exact commands/results and tree proof are under ignored
`audit/pull-request/pr-1148-repair/`; the original audit remains preserved.
No remote publication is part of this execution.
