# Publish the complete DTLS implementation as a review series

> **Kind:** design · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [original DTLS plan](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/plan/done/dtls.md), [extension plan](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/plan/pending/dtls-extensions.md), [series rules](../../doc/project/rule/pull-request.md#pr-series-plan), [test rules](../../doc/project/rule/testing.md#tr-plan-protocol)

## Purpose and authorization

Prepare the complete implementation, including the original model and all extensions, as independently reviewable commits and PRs. This uses the 6LoWPAN publication procedure, including its 2026-11-17 faster verification revision: focused debug checks per code commit, combined checks at each PR tip, one final acceptance campaign, an immutable input, a coverage ledger and explicit evidence reuse.

The user approved this concrete 18-PR, 41-commit plan with “proceed” on 2026-11-17. Execution covers isolated target preparation, reconstruction and verification within C00–C40, retaining the original branches and local edits. The user has also requested PR-series creation; delivery follows the explicit incremental upstream policy below, without closing or replacing #1459/#1465 automatically. No new baseline or production behavior is authorized. The cleanup skill's group/commit approval prerequisite is satisfied; it is independent of the draft project planning rules.

The model uses retained modeled content, overhead and processing costs. Publication must preserve its documented exclusions: real cryptographic security, unrestricted wire interoperability and unrestricted standards conformance are not established.

Approved ownership correction, 2026-11-17: the two decode sites repaired by `448ef4ffa7` are introduced by KeyUpdate, so their source fixes, four fixtures and reviewer document belong together at C16. The association's reassembly query first exists with recovery, so its final visibility belongs at C13. The user explicitly approved this corrective-delta ownership revision. The same 41 commits, 18 PRs, order and final source tree remain in force.

## Pinned inputs

| Input | Value |
| --- | --- |
| Original checkout | `/home/miguel/workspace/inet-dtls-extensions` |
| Immutable topic | `feat/dtls-extensions` at `5284847df92eea8451bf056d8722fabb0ddf65bd` |
| Original source base | `980f5c3f9eb903fd9155dd8d24216a1c265d5f55` |
| Original implementation tip | `feat/dtls` at `cb1691184444afa64b1819348d446d4cad610474` |
| Proposed publication target | upstream `master` at `476ab0f2fa019c0bcf36d23d9f99ec884ee9393c` |
| Input size | 43 commits and 553 changed paths |
| Proposed worktree | `/home/miguel/workspace/inet-dtls-publication` |
| Proposed clean branch | `cleanup/dtls-publication` |
| Review refs | `review/dtls-01-shared-prerequisites` through `review/dtls-18-examples-and-acceptance`, with suffixes from the PR table |
| Preserved local input | The existing modified `AGENTS.md`, excluded from all staging and history operations |

Preparation inventories are outside the checkout at `/home/miguel/workspace/.codex-artifacts/dtls-publication-plan-20261117/`. `inputs.json` records the dirty-file hash, original paths, baseline delta and exact test inventory. `source-commits.json` records every donor and its changed paths/tests/documents. `commit-plan.json`, `allocation.json`, `pr-description-drafts.md` and `static-validation.json` hold the proposed output map, per-donor path allocation, description drafts and preparation checks. They are local preparation artifacts, not published runtime evidence.

The live PRs remain open: [#1459](https://github.com/inet-framework/inet/pull/1459) has the original tip above, and [#1465](https://github.com/inet-framework/inet/pull/1465) has the immutable topic tip above. Both target upstream `master`.

## Target preparation and equality contract

The original source base and proposed publication target differ. Treat target preparation as a separate, recorded operation before fixed-base cleanup. Do not call a clean merge proof of semantic compatibility. The target adds documentation, enforcement and showcase/configuration changes; the only path also changed by the DTLS input is `doc/project/audit/seal-list.md`.

1. Pin the target exactly. Read its project map, series/baseline rules and protection registry. Preserve the original branch and dirty file without switching or resetting that checkout.
2. In the isolated worktree, apply the original-base-to-topic delta to the pinned target. Preserve all target-only paths and changes. For the seal index, preserve the target's declarations and regenerate the index using the current enforcement command. Do not copy the topic's older index over the target's current policy.
3. Verify that every other DTLS-changed path has the original topic's final bytes and mode, that every target-only path retains target bytes/mode, and that no unrelated path changed. Record the exact seal-index delta as generated metadata, without inventing a new seal or modifying sealed content.
4. Freeze that prepared target tree as T, with an immutable preparation ref, parent, manifest and full diff. The cleanup itself starts at the pinned target and must reproduce T exactly, except the explicitly listed publication documentation/metadata below. No runtime adaptation or new baseline is authorized by this preparation plan. Unexpected source overlap, build incompatibility or a required semantic change stops preparation for a bounded plan revision.

The original topic remains the oracle for DTLS source, tests, examples, release notes and stored baseline references. T adds the target's unrelated context. Final proofs include both `clean versus T` and the DTLS-path projection versus the original topic; neither is replaced by a global comparison that ignores upstream differences.

Publication metadata allowlist: this new plan and its eventual pure archive move, traceability/verification records, rewritten `Plan:` associations and incoming links, stage-accurate reviewer/evidence text, and the regenerated seal index. Explain every byte difference on these paths. Historical execution claims must keep their original source revisions; fresh reconstructed results receive separate records. The final executable inputs and test assertions must equal the topic projection. No baseline exception or production-source difference is preapproved.

## Requirements retained

Original IDs below are prefixed I for the initial plan and E for extensions. They retain their original meaning; C IDs identify publication commits only. Original documents remain accessible through the immutable URLs in this document even before local historical snapshots enter the new history.

| Original contract | Publication owner | Acceptance retained |
| --- | --- | --- |
| I-C1 profile and standards | C00, C38 | Modeling/security limits, clause coverage and provenance. |
| I-C-PREP | C01 | Typed interception predicates select actual packet targets. |
| I-C2 and helper preparations | C03–C08 | Bounded processing, record reconstruction/replay, TLS content, record representation and UDP-backed engine ownership. |
| I-C3 | C09–C10 | Shared parameter selection, cookie negotiation, certificate/PSK modeled establishment and isolated callbacks. |
| I-C4 | C11–C13 | Flight accounting, bounded reassembly and loss recovery. KeyUpdate-specific delayed decoding safety belongs to C16. |
| I-C5 | C14 | Protected datagrams, epoch/replay commitment, length/overhead and association isolation. |
| I-C6-PREP-A / I-C6-KEYS | C15 / C16 | Continuation ordering precedes actual KeyUpdate users. |
| I-C6-PREP-B / I-C6-CLOSE | C17 / C18 | Atomic cancellation precedes graceful close, stop/crash/restart and peer isolation. |
| I-C7 | C19 | Six original examples and their existing statistics pin. |
| E-C1 / E-C2 | C00, C20 | Extension contract and shared post-handshake recovery preparation. |
| E-C3 / E-C4 | C21 / C22 | Bounded tickets/resumption and explicit early-data admission, fallback and replay limits. |
| E-C5 | C23–C24 | Shared algorithm catalog precedes profile-dependent endpoint sizes/costs and resumption compatibility. |
| E-C6 / E-C7 | C25 / C26 | CID negotiation, non-reuse, updates, recovery and request limits. |
| E-C8 / E-C9 | C27 / C28 | Correlated heartbeat precedes validated path commitment and socket rebind. |
| E-C10 | C29 | Independent post-handshake client authentication and ticket/authentication snapshots. |
| E-C11a / E-C11 | C02 / C30–C32 | Generic IPv6 fragmentation requests precede bounded discovery, refragmentation and production path budgets. |
| E-C12 / E-C13 | C33 / C34 | Version-specific records precede full DTLS 1.2 selection, cookie/flight recovery and data. |
| E-C14 | C35 | Version-bound session IDs/tickets and retained final-flight recovery. |
| E-C15 / E-C15b / E-C17 | C36 | Legacy static CID/path support includes finite admission and correct heartbeat minimum from first publication. |
| E-C16a / E-C16 | C37–C38 | Scripted socket actions precede ten extension examples and final combined acceptance. |
| E-C16b | Each owning fixture commit | Always-evaluated test checks in debug/release, without changing expected observations. |
| Original archive and completion records | C38–C40 | Historical provenance, new acceptance evidence, pure archive then incoming-link updates. |

## PR boundaries and order

The table is the landing order and the local linear stack. Every commit additionally depends on its immediate predecessor. Direct prerequisites identify the behavior dependencies; a later PR is not represented as independent merely because Git applies its patch cleanly.

| PR | Review-ref suffix | Purpose | Commits | Direct prerequisites |
| --- | --- | --- | --- | --- |
| 01 | shared-prerequisites | Typed interception and IPv6 fragmentation requests | C00–C02 | Pinned target |
| 02 | foundations | Processing/replay, TLS content, record representation, local contract and engine shell | C03–C09 | PR01 |
| 03 | handshake-recovery | Establishment, flight accounting, reassembly and loss recovery | C10–C13 | PR02 |
| 04 | protected-datagrams | Protected application traffic | C14 | PR03 |
| 05 | key-update | Ordered processing continuation and epoch updates | C15–C16 | PR04 |
| 06 | closure-and-core-examples | Atomic cancellation, graceful closure and original demonstrations | C17–C19 | PR05 |
| 07 | resumption | Shared recovery and modeled tickets | C20–C21 | PR06 |
| 08 | early-data | Explicit early-datagram admission | C22 | PR07 |
| 09 | algorithm-profiles | Shared algorithm catalog and endpoint consumers | C23–C24 | PR08 |
| 10 | connection-identifiers | CID negotiation | C25 | PR09 |
| 11 | cid-updates | Recoverable CID updates | C26 | PR10 |
| 12 | mobility | Heartbeat preparation and validated migration | C27–C28 | PR11 |
| 13 | client-authentication | Post-handshake client authentication | C29 | PR07, PR09, PR11–PR12 for current interaction fixtures |
| 14 | path-mtu | Budget state, flight refragmentation and discovery | C30–C32 | PR01, PR12–PR13 |
| 15 | dtls12-foundation | Legacy records and full handshakes | C33–C34 | PR14 for the unchanged current engine interfaces |
| 16 | dtls12-resumption | Legacy abbreviated handshakes | C35 | PR07, PR15 |
| 17 | dtls12-paths | Legacy CID, mobility and discovery | C36 | PR10, PR12, PR14–PR16 |
| 18 | examples-and-acceptance | Scenario actions, extended examples and publication acceptance | C37–C40 | All preceding PRs |

## Proposed commits

All entries are proposals. A donor means its delta at that revision, not the whole final version of a shared file. U, M and P denote `tests/unit`, `tests/module` and `tests/protocol/dtls` fixtures. Exact donor-owned test selections are generated in `commit-plan.json`; the allocation rules below specify splits and later corrections. Reviewer documents accompany their corresponding production behavior. No test is postponed to the final documentation commit.

| ID | Subject | Donor and decision | Focused verification and parent comparison |
| --- | --- | --- | --- |
| C00 | `plan: add: traceable DTLS publication sequence` | Consolidate plan/standards obligations from `8b79b19bfa`, `62636d476e`, `bfe3b7561c`, `5255586578`. Add this plan before code. | Content, immutable links, dependency and donor coverage checks. No executable change. |
| C01 | `tests: add: select interception targets with typed predicates` | `1094a14462`: generic protocol-test predicate and authoring contract, independent of DTLS. | P at `tests/protocol/udp/TypedTapPredicate.test`, its reviewer document and existing UDP interception controls. Compare the same predicate fixture only where parent compilation is compatible. |
| C02 | `ipv6: add: honor requests to suppress source fragmentation` | `6937353895`: honor the generic request in IPv6 and retain existing ordinary fragmentation. | `Ipv6DontFragmentRequest`, `Ipv6DontFragmentRequestDelayed`, `Rfc8200SourceFragmentation` under `tests/protocol/ipv6`, plus the donor's document. Observe actual output/drop through the real stack. |
| C03 | `dtls: add: bounded directional processing` | `5d83a169b8`: queue ownership, limits and service accounting. Introduce final helper visibility and the test-only always-evaluated check header. | M(DtlsProcessingQueue_1), at this stage's API. Queue checks must execute in release too. No endpoint consumer yet. |
| C04 | `dtls: add: record reconstruction and replay state` | `933754a4bd`: final scoped record-state API, before receive consumers. | U(DtlsRecordState_1), number reconstruction/replay boundaries and commit semantics. |
| C05 | `tls: add: modeled handshake content serialization` | TLS MSG/serializer/package slice of `1d8c95330a`. Keep shared content separate from DTLS delivery. | TLS-only prefix of U(TlsHandshakeSerializer_1), exact valid/malformed vectors. Add its DTLS-specific half at C06. Introduce TlsCommon feature ownership with its files. |
| C06 | `dtls: add: record representation and modeled protection` | DTLS header/serializer/protection slice of `1d8c95330a`. Printer/dissector registration waits for the engine's protocol object in C08. | U(DtlsRecordSerializer_1) and completed donor-stage U(TlsHandshakeSerializer_1), exact representation/length checks. Protocol parsing waits for the actual engine in C08. |
| C07 | `dtls: add: local datagram engine contract` | IDtls/DtlsSocket/NED contract slice of `1d8c95330a`. Publish packet/callback ownership before its engine and adapter users. | Compile contract headers/NED and run C05–C06 controls. This declarative contract has no standalone wire behavior; engine runtime proof belongs to C08. |
| C08 | `dtls: add: UDP-backed engine lifecycle` | Remaining engine/app/fixture slice of `1d8c95330a`; install Dtls feature closure at its earliest required stage. | M(DtlsEngineLifecycle_1), P(Rfc9147RecordParsing), record-foundation.md. Test registration, admission handles, rejected sends, UDP parsing and cleanup. This shell does not establish a handshake. |
| C09 | `tls: add: modeled handshake parameter selection` | `95d9557110`: selection helper before association consumers. | U(TlsHandshakeParameters_1), valid choices, rejection and ordering. |
| C10 | `dtls: add: establish modeled client and server associations` | `cc5f0afbb6`: full initial association/cookie/PSK/certificate behavior. | All establishment donor P fixtures and lifecycle M updates, association-establishment.md. API absence on parent is a compatibility gap. KeyUpdate-specific malformed protected-handshake fixtures belong at C16. |
| C11 | `dtls: add: handshake flight accounting` | `eec80dc732`: independent range/record/RTT bookkeeping before recovery. | U(DtlsHandshakeFlight_1), exact flight/ACK/timeout state. |
| C12 | `dtls: add: bounded handshake reassembly` | `ec4e1a9b78`: bounded fragments before production recovery. | U(DtlsHandshakeReassembly_1), duplicate/conflict/limit/completion checks. |
| C13 | `dtls: add: recover fragmented handshakes over lossy UDP` | `4bea908d4c`: integrate flights, reassembly, delays and recovery. Introduce the association's final protected reassembly-count query when its state first exists. | All recovery donor P fixtures, U/M changes and handshake-recovery.md. Retain charged-delay, loss, duplicate and unrelated-peer assertions. KeyUpdate-specific malformed fixtures belong at C16. |
| C14 | `dtls: add: protected application datagrams` | `27cfb518f0`: protected data with epoch/replay and byte accounting. | All donor P fixtures, record unit changes, protected-datagrams.md; verify replay commitment, malformed/oversized drops and peer isolation. |
| C15 | `dtls: refactor: allow consecutive input processing stages` | `0144fcc381`: queue continuation without activating KeyUpdate. | M(DtlsProcessingQueue_1), existing queue controls and no consumer trajectory change. |
| C16 | `dtls: add: update association protection epochs` | `6f93b927e5`: directional KeyUpdate state/recovery and bounded queued sends. Fold both guarded-decode corrections from `448ef4ffa7` into their first actual source owner. | All donor P/M fixtures and key-updates.md, including request guards, costs, limits and isolation. Include all four malformed-current/retained synchronous/queued fixtures and malformed-handshake-records.md, with exact raw bytes, charged service, replay and continued delivery assertions. |
| C17 | `dtls: refactor: cancel queued work while preserving selected owners` | `31b014b147`: atomic batch cancellation before closure consumers. | M(DtlsProcessingQueue_1), preserve selected owners and exact cancellation/service controls. |
| C18 | `dtls: add: close associations without disrupting other peers` | `a23804a1e8`: graceful close, lifecycle and callback cleanup. | All donor P fixtures, M(DtlsRestartLifecycle_1), closure-and-lifecycle.md and core regression controls. |
| C19 | `dtls: add: demonstrate the supported simulation profile` | `790954468a`: six original examples, user documentation and causal existing statistics pin together. | Six original run-0 configurations, assertions/scalars and existing statistical comparison. Preserve profile limits and final original datagram observations. No new baseline. |
| C20 | `dtls: refactor: share post-handshake exchange recovery` | `5345cf5809`: independent exchange bookkeeping, retain old behavior before new consumers. | U(DtlsPostHandshakeExchange_1), original core update/close/recovery controls. |
| C21 | `dtls: add: resume associations with modeled session tickets` | `289dfc1cf2`: bounded session/ticket state and production resumption. | All donor U/M/P fixtures and resumption.md; expiry, identity isolation, retry/fallback and lifecycle. |
| C22 | `dtls: add: exchange explicitly admitted early datagrams` | `6aad92248b`: explicit early admission and rejection with bounded replay/holding. | All donor U/M/P fixtures/documents; fresh/failed/closed attempts, cookie behavior, eviction and rejection callbacks. |
| C23 | `tls: add: modeled algorithm profile catalog` | Shared TLS catalog/preference/selection slice of `08a7663154`, preserving old endpoint defaults. | U(TlsHandshakeParameters_1), shared content serialization vectors from the donor. Endpoint/store-specific tests wait for C24. |
| C24 | `dtls: add: negotiate additional modeled algorithm profiles` | Remaining `08a7663154`: profile-dependent record/handshake sizes, selected costs and saved-session compatibility. | All donor P fixtures, record/store unit updates, algorithm-profiles.md; alternate hash resumption/early data and fragmentation/cost interactions. |
| C25 | `dtls: add: negotiate connection identifiers` | `a3197c8bf9`: negotiation, directional routing/overhead and bounded non-reused identities. | All donor U/P fixtures/documents, including record parsing and lifecycle non-reuse. |
| C26 | `dtls: add: recover connection identifier updates` | `6db30c64b6`: CID-update serialization, bounded requests and independent recovery. | All donor U/P fixtures/documents; loss, ACKed-unfulfilled requests and overlap with data/update/ticket recovery. |
| C27 | `dtls: add: correlated heartbeat probes` | `ff9243db29`: protected shared probe mechanism before migration consumers. | U(DtlsHeartbeatSerializer_1), all donor P fixtures/documents, correlation, timeout, replay and single-probe bounds. Preserve internal test access rather than adding public diagnostics. |
| C28 | `dtls: add: preserve associations across validated path changes` | `7a704cfa63`: candidate-path validation and socket rebind. | All donor M/P fixtures/documents, routed IPv4/IPv6, NAT scope, old-socket indications, occupied-port outcome and timeout/isolation controls. |
| C29 | `dtls: add: authenticate clients after establishment` | `8ddcd393c2`: independent post-handshake authentication state. | All donor U/M/P fixtures/documents; malformed/timeout/admission/lifecycle and ticket/authentication snapshot interactions. |
| C30 | `dtls: add: bounded path MTU search state` | DtlsPathMtuState slice of `305cb39a08`, no endpoint caller. | M(DtlsPathMtuState_1), bounded search/probe/timeout state. |
| C31 | `dtls: add: account for refragmented handshake flight ranges` | Flight helper slice of `305cb39a08`, no production adaptation yet. | U(DtlsHandshakeFlight_1), including byte ranges, replacement/late ACK coordinates and unchanged earlier flight accounting. |
| C32 | `dtls: add: discover and adapt per-path payload budgets` | Remaining `305cb39a08`: production probe, quoted-hint and path-budget consumers, configuration/contract changes and cancellation. | All donor configuration M/P fixtures and path-mtu.md; late ACK, queued/active replacements, quoted header offsets, ordinary/short/stale quotes, auth pause, migration and unrelated-peer progress. |
| C33 | `dtls: add: version-specific DTLS 1.2 record representation` | `654a6509b2`: version-specific wire/protection helpers before legacy establishment. | All donor U/M/P fixtures and dtls12-record-foundation.md; correct explicit nonce/epoch representation, no claim of a working legacy handshake yet. |
| C34 | `dtls: add: establish DTLS 1.2 associations` | `b7ba9d9a89`: version choice, cookie/full handshakes, whole-flight recovery and protected data. | All donor U/M/P fixtures/documents, version refusals/configuration, no accidental 1.3 ACK/update behavior and retained final-flight response after 120s but before 240s. |
| C35 | `dtls: add: resume modeled DTLS 1.2 sessions` | `c3bd545bc4`: version-bound saved sessions and abbreviated handshakes. | All donor U/P fixtures/documents; ID/ticket fallback, expiry, cross-version refusal and genuine duplicate-triggered retained final recovery. |
| C36 | `dtls: add: DTLS 1.2 path mobility and discovery` | `79acb507fe`, corrected at birth by `ed08865cc9` and `5284847df9`. Keep CID resource refusal before admission and version/CID-specific heartbeat minima. | All three donors' U/M/P fixtures/documents, including Rfc9146ConnectionIdExhaustion, Rfc6347PathMtuMinimumBase and five legacy/mixed/negative-base M cases. Exact 64-byte no-CID/73-byte eight-byte-CID boundaries and six endpoint profile pairs run in debug/release. |
| C37 | `dtlsapp: add: script socket actions in scenarios` | `d68e67176a`: ScenarioManager adapter actions, supporting routed/scenario networks and first executable action consumers. | Donor's scenario configurations/verifier at that stage, valid/invalid actions, single association and protocol controls. Ten final examples are completed by C38. |
| C38 | `doc: add: complete DTLS examples and acceptance evidence` | `d8790ea933` plus completed historical records from `dc9f2274ec`/`efe6354a04`. Finalize ten extension examples, user guide, migration/release/coverage documents and fresh acceptance record. | Complete final contract below. Documentation/executable examples and their assertions remain coupled. Preserve original records as historical, with separate new-series evidence. |
| C39 | `plan: location: archive the completed DTLS publication plan` | Pure move of this plan from pending to done, after final acceptance. | Rename-only diff and executable-input identity. No content editing or repeated simulation. |
| C40 | `doc: change: link the completed DTLS publication plan` | Update incoming links and historical associations affected by C39. | Links, stored Plan paths at each historical commit, exact executable-input identity and final ledger closure. |

Large coupled endpoint features remain larger than 400 source lines after helper preparations. Before promotion, count source changes and put a specific PR-SPLIT-SIZE rationale in each affected body: name the atomic state transition, callbacks/cleanup and tests that must land together, and explain why an additional independently usable cut is unavailable. Size alone is not an exemption. If inspection exposes such a cut, revise the group plan before reconstruction rather than silently adding commits or scaffolds.

## Artifact allocation and corrective folding

The exact per-donor changed-path inventory is reproducible with `git diff-tree --no-commit-id --name-only -r <donor>`. For each main donor, all changed fixtures and their `.md` reviewer documents belong to the table's owner except the explicit partitions below. A later edit to a fixture belongs to that later behavior's commit, not automatically to the fixture's first commit. Preserve the assertions/observations and port intermediate APIs without importing future features.

- Split `1d8c95330a` by component: TLS MSG/serializer/package and TLS-only serialization assertions → C05; DTLS MSG/serializer/protection, record unit and DTLS-specific serialization assertions → C06; contract directory and `contract/IDtls.ned` → C07; engine, application adapter, printer/dissector registration, protocol fixtures/reviewer document, lifecycle fixture and remaining integration → C08. The original TLS test includes DTLS types, so split its includes/assertions at the same boundary rather than requiring future generated headers in C05. The printer/dissector register the protocol object defined by Dtls.cc, so they remain with C08. Split `.oppfeatures` by actual earliest package owner; install the Dtls switch when the first Dtls helper enters C03, TlsCommon in C05, and complete Dtls package/dependency closure in C08. Introduce DtlsExamples only with C19. No absent future package is enabled at an earlier prefix.
- Split `08a7663154`: shared TLS parameter catalog and direct selection/serialization tests → C23; endpoint, session store, costs, records and production fixtures → C24. Existing defaults and old controls must remain identical at C23.
- Split `305cb39a08`: PathMtuState source/header and its module → C30; HandshakeFlight source/header/test delta → C31; all integration/configuration/production evidence → C32. C31 must not silently enable discovery.
- `448ef4ffa7`: put both guarded-decode source changes, all four exact fixtures, DtlsTestUtils helper changes and malformed-handshake-records.md at C16. The repaired code discriminates retained epoch-two handshake traffic and decodes current KeyUpdate traffic; those sites first appear in `6f93b927e5`, not in initial establishment/recovery. Preserve the same faults/assertions; an incompatible earlier fixture is not a parent runtime failure.
- `cb16911844`: fold final helper visibility into C03 (processing queue), C04 (record state) and C13 (association reassembly). Keep protected testing access and genuine caller needs; introduce no public observation API just for tests.
- `1ad1d32de4`: introduce `tests/DtlsTestAssertions.h` in C03. Use its always-evaluated checks from each fixture's first appearance and in every later feature-owned expression. Keep expected output, state transitions and failure semantics identical. Record the full expression/token transformation proof; do not copy final tests containing future APIs into early commits. This correction is distributed across fixture owners, not published as a late repair.
- `dc9f2274ec`: distribute its fixture descriptions and reviewer explanations to their behavior owners C01/C08/C10/C13/C14/C16/C18. Restore historical aggregate evidence and completed original plans at C38. The malformed-record explanation goes with C16.
- Plan revisions and archive-only donors feed C00's consolidated immutable contract links and C38's historical snapshots. They never add later feature completion claims to an earlier prefix. The original extension plan remains pending because that is its input path; C39 moves only this new publication plan.
- Model coverage ledgers, release notes, migration instructions and reviewer explanations for a feature travel with its owner. Preserve cumulative original execution sections as historical source-revision evidence; new-prefix execution records must identify their actual reconstructed hashes. C38 owns combined acceptance, not deferred behavior documentation.

No unrelated production delta has been identified in the 43-commit DTLS input. No donor is dropped. The machine-readable allocation covers each donor/path delta; intra-file splits and distributed assertion replacements require a hunk ledger during execution. A complete path allocation is a preparation result, not a proof of final hunk equality.

## Verification procedure

### Discovery and tooling

Before execution, use the target's `doc/project/README.md`, current enforcement README, `inet.opp`, `dependency.json`, feature graph, installed `opp_repl --help` and runner help. Record exact resolved commands, mode, configuration, run/seed, time limit, generated-input and loaded-library hashes. Do not copy commands from a different opp_repl version. Each selector must enumerate its nonzero expected cases before running; avoid generated `work/` duplicates. Keep one incremental builder/runner per checkout, with no parallel feature-state or generated-output writes.

Current known capability gaps are explicit: the checkout's normalization adapter needs an importable `jsonschema`; clang-tidy was unavailable in the prior audit. Resolve tooling in an isolated tools environment or report NOT_RUN; a missing tool is never PASS. The user-supplied skill-package validators/tests are absent in this INET checkout and are not substituted for model verification.

### Per-commit and PR-tip gates

- Each code commit: incremental debug compilation, focused tests from its allocation and unchanged affected-owner controls. Behavior-preserving preparations require the same relevant signal before/after. Representation-only or declarative commits use direct helper/compilation checks rather than a future production exchange.
- Each PR tip: debug/release compilation and the union of assigned tests at their current versions, plus affected shared-owner controls. Explicitly record reused immediately preceding results only when source, feature, configuration, fixture, generated-code and loaded-library inputs match. A reuse is not a fresh execution.
- Mode-sensitive cases run in release as well: always-evaluated helper checks, admission failures, negative/small PMTU configuration, callback/lifecycle paths and the legacy boundary correction. Expected configuration-error cases pass only when the expected diagnostic and cause agree.
- Exact-parent execution targets meaningful compatible defect demonstrations, especially malformed delayed decoding, CID resource exhaustion and legacy PMTU admission. Use the reconstructed commit's fixture and expectations on both versions. If the earlier published correction is already folded into the first feature, label the immediate-parent comparison as unavailable capability and retain the original fix/parent evidence under its original hashes; do not manufacture a failing intermediate commit.
- Other parent comparisons may be source-based, explicitly unexecuted. Parent PASS is a control. Missing API, compilation error or initialization incompatibility does not establish a protocol failure.
- Documentation-only commits: content/links and executable-input identity. Token-only transformations also require a reverse-map proof and relevant controls. Do not repeat builds merely because a Plan link changes.
- Run feature-off/minimal-profile checks when `.oppfeatures`, shared compiled ownership or profile integration changes, and at final acceptance. Preserve/restore exact feature state. Check TlsCommon without Dtls once introduced, Dtls without examples, and Dtls/TlsCommon disabled with existing UDP/IPv6 controls when compatible with that prefix. Resolve package/dependency closure from the actual tree; do not assume a declared dependency is sufficient.

### Final acceptance at C38

The pinned input inventory is **352 DTLS protocol fixtures, 15 DTLS/TLS unit fixtures and 26 DTLS module fixtures**. Enumerate exact filenames from `inputs.json`. Run the complete debug inventory, not just the extension-only selections. Run all 15 units and 26 modules in release, plus the existing 60-case release interaction matrix and the new PMTU-boundary protocol fixture. Resolve the exact union/deduplication from the C16 and correction records before execution; preserve their seeds and six profile-pair assertions. Run the changed generic typed-predicate/IPv6 cases and existing affected UDP/IPv6 controls in both modes.

Run all sixteen example configurations in both modes with `python3 examples/dtls/verify.py --mode <mode> --output <new-directory>`. They are Basic, Certificate, Fragmentation, LossyHandshake, ProcessingCost, Ipv6, Reconnect, EarlyData, Migration, DeferredAuthentication, AlgorithmCost, PathMtu, Legacy, MixedVersions, LegacyReconnect and LegacyMigration. C19 owns the first six. Retain simulation/scalar/export/report artifacts and compare selected endpoint measurements between modes. Temporal MTU examples additionally run their actual asserting protocol fixtures. One deterministic seed establishes those observations, not statistical confidence or physical performance.

At C19 and C38 compare the six original statistical configurations against the existing statistics store. The source introduces `inet-framework/statistics` ref `3d3a8995a00d75f2f21ccd42a648db591974611c`, replacing `f76ce96731278d64727065b5669e4f1cab431da5`. Keep that pin with C19's causal examples/source change and preserve its exact subject. Preparation verified the remote baseline commit message: it records six added configurations, no changed existing tests, `INET-Subject: dtls: add: demonstrate the supported simulation profile` and `Attribution: first-record`. This matches C19. It reports its original CI/environment provenance and explicitly makes no parent-run claim because those configurations did not exist there. A local baseline checkout/comparison has not run. Verify the baseline's records against the pinned target's current attribution procedure before promotion. No new baseline values, chart references or fingerprint changes are authorized. A missing store or mismatch is diagnosed and reported; it is not refreshed automatically. The ten extension examples do not acquire invented statistical baselines.

Complete the feature-off/minimal-profile checks, target policy/mechanical gates, scoped architecture/naming/interface checks, source/document protection, release formatting and links. Compare whole-tree findings against the pinned target and unchanged paths, retaining inherited failures as such. Passing a bounded affected-area check does not turn an inherited global failure into a global PASS. No new seal is requested; packet-core sources remain protected and untouched.

Preserve raw results and normalized records. Required records distinguish PASS, FAIL, ERROR, NOT_RUN and reused evidence, and include exact selectors/counts. The original C16 campaign and later correction builds are historical input evidence, not checks of reconstructed commits. All proposed-prefix builds, simulations and baseline comparisons are currently **NOT_RUN**.

## Coverage ledger and recovery

Before reconstruction, create `ai-logs/executions/2026-11-17_dtls-publication.analysis.md` and the corresponding state log in the publication worktree. Use full ISO timestamps. Record immutable inputs, prepared T, group/commit IDs, donor slices, fixture/document ownership, expected effects and the exact verification commands. The current preparation inventories seed those records; there is no execution safe point yet.

After every commit, inspect the remaining diff to T and update both path and hunk ledgers. Track whole-file adoption, split hunks, metadata exceptions and every temporary detour with its opening and named closing commit. No per-group fossil branches are needed. The result must contain no unassigned donor material, unexplained residual or open detour.

On a failed gate, stop for diagnosis against the previous safe point, T and the immutable topic. Never refresh expectations to accept unexplained behavior. Preserve an already validated tip before rework as `cleanup/dtls-publication/checkpoint-<timestamp>`; never repoint/delete such checkpoints. Changed approved boundaries/order or required semantic fixes need a revised plan before continuation. Keep the original branch and dirty AGENTS bytes unchanged throughout.

Finalization records the full output SHAs/order, PR tips, source/executable equality, metadata allowlist diff, baseline identity, closed detours and risk-based middle-safe-point spot checks. C39 is a pure move and C40 only link edits; hash identity permits reuse of C38's acceptance results with explicit provenance.

## PR creation and existing PR transition

Authenticated repository permissions currently show no upstream push permission. A fork-only review branch cannot be used as the upstream repository's base branch. Do not publish eighteen nominally incremental upstream PRs with invalid fork bases.

1. Build and validate all local review refs first. Generate one self-contained description per PR: behavior/limits, direct predecessor, commit links, protocol scenarios and parent-comparison status, baseline/protection disclosures, evidence and useful diagrams. Replace C IDs with actual SHAs only after reconstruction.
2. Once publication is authorized, push the new review refs to the fork without replacing `feat/dtls` or `feat/dtls-extensions`. Verify each remote SHA. This prepares comparisons for every unit without changing upstream PR state.
3. Preferred upstream delivery: open PR01 against upstream master, then open each incremental PR as its predecessor lands, rebasing only the unpublished remainder onto the actual landing result with an explicit tree/patch comparison and affected verification. An upstream-maintained stack is possible only if maintainers provide upstream base refs. Do not silently choose a different workflow.
4. If simultaneous upstream visibility is requested before predecessors land, use explicitly dependent master-based draft PRs with cumulative scope clearly stated and an exact predecessor-tip-to-current-tip comparison linked for each review unit. That is a publication choice, not proof of clean incremental GitHub diffs. A fork-hosted stack is also not a substitute for upstream submission.
5. Leave #1459/#1465 open and unchanged until the replacement series and its evidence are ready and supersession is authorized. Then update them with the new entry point and precise replacement map; close only under that scope. Preserve their historical evidence and avoid two competing merge paths.

The local description drafts are preparation only. PR creation must pass the current description gate with its actual unit range, plus the series/message/classification/protection gates. Stored commit messages need a valid `Plan:` path in their own tree, a plain-language problem/result, scenario/parent explanation and current classification trailers. Audit the whole reconstructed series, not just the final PR body. No PR creation, push, historical branch replacement or baseline refresh is reported as completed by this plan.
