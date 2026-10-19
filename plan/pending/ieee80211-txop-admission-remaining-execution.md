# TXOP admission: execution record

Execution date: 2026-10-19. The user requested execution of revision 9. The active checkout is `/home/user/omnetpp_ws/inet-ieee80211-txop-admission-remaining`. Its branch is `work/ieee80211-txop-admission-remaining`. Its starting revision is `d841dc77db7d0b141bf31e5e215a0de9bb281957`.

The working tree contains the Normal Ack and Basic implementations. The user accepted R1's lifetime interpretation on 2026-10-19. B1 supplies the legal failing continuation. The [decision note](ieee80211-txop-admission-r1-decision.md) records that decision. Local focused checks pass. Combined verification with the separate peer-permission activation remains pending.

The initial task preserved the earlier `AGENTS.md` edit, peer-permission documents, and historical reproduction. It left the implementation uncommitted and changed no recorded fingerprint baseline. The later user request authorizes commits, a push, and an upstream PR. The [publication record](ieee80211-txop-admission-publication.md) supplies the commit boundaries and later checks.

## Implemented behavior

HCF now prepares individually addressed QoS DATA before the frame sequence can extract its queued sources. Enabled Basic support selects the 2016 contract with or without an accepted agreement. A disabled Block Ack handler uses the modern contract even when the unused variant parameter says `basic`. Accepted Compressed and A-MPDU paths retain their agreement contracts.

HCF selects one DATA mode for the current exchange. It reuses that mode for aggregate evaluation, ordinary fallback, fragment admission, and submission. A successor reservation uses an estimate at the current mode. The successor selects its actual mode at its own admission boundary. A later exchange must fit the remaining TXOP and its separate strict TXNAV bound.

The data service constructs duplicate fragments before commitment. The bounded policy combines the configured threshold with HCF's complete-exchange airtime bound. Commitment consumes a queued source once. It transfers the same fragment pointers with one sequence identity, one allocation identity, and the original absolute deadline. A numbered source retains those identities during replacement. Queue service metadata follows the payload regions into the fragments.

Basic and modern no-agreement paths commit before RTS after admission succeeds. A failed associated RTS therefore updates the same MSDU record before DATA transmits. The modern record contains one cumulative count across fragment ACKs and size changes. Its short retry limit controls disposal. Basic retains separate short and long counts across fragment replacement. Under an accepted Basic agreement, retry-limit exhaustion retains the allocation until success, lifetime expiry, or agreement disposal.

HCF updates the agreement exemption at actual RTS submission, including retries of an allocation that predates agreement acceptance. Normal Ack under an accepted Basic agreement uses the same exemption. Basic fallback without an agreement retains the short and long limits. A collision or RTS failure alone does not establish DATA retransmission history. The committed-frame owner retains local DATA history for the positive-limit exceptions.

HCF ends a short TXOP tail without extraction of queued sources. If no permitted form fits a fresh TXOP, HCF raises a diagnostic before extraction or sequence allocation. A zero limit permits a complete fragment burst for one MSDU. The next unrelated MSDU needs another TXOP. Source expiry and agreement disposal remove complete fragment groups through the current owners.

For example, the original disabled-Block-Ack campaign can send an A-MSDU beyond a one-millisecond TXOP. The corrected path checks the full DATA/ACK or RTS/CTS/DATA/ACK exchange first. It selects a smaller form or ends that TXOP. All four repeated Normal Ack probe runs now report zero invalid completion boundaries.

## Step status and change scope

| Step | Implemented work | Evidence and limits |
| --- | --- | --- |
| C1 | The fragmentation contract, Basic policy, QoS service, and committed-frame owner now support reversible fragment groups. The migration guide describes the interface changes. | The bounded-policy unit fixture and eight preparation runs pass. They cover cancellation, commitment, numbered replacement, stale sources, expiry, permission changes, and impossible bounds. |
| C2 | The recovery owner now supports allocation-scoped modern and 2016 records. Group cleanup and local fragment history use existing owners. | The direct recovery fixture passes. Accepted Basic agreements retain counters beyond both limits. Without that exemption, the configured limits apply. |
| C3 | Initial classification uses a read-only selector. Common admission retains the actual modes and costs the complete exchange. | Production module controls and one-tick boundary checks pass. BAR arbitration remains under the existing policy. |
| C4 | The modern no-agreement path now uses prepared admission, fragmentation, early RTS commitment, and group recovery. | The infrastructure fixture checks containment, zero limits, CTS loss, retry exhaustion, and cancellation. The extended fixture also checks collision-owned fragment replacement. |
| C5 | Basic now uses prepared admission, permitted fragments, commitment before RTS, and the accepted R1 rule. | Thirty-six production runs cover both protection classes and rates, forced failures, internal collisions, expiry, fallback, and legal mixed peers. The four B1 probe runs now report zero invalid boundaries. |
| C6 | User effects, migration text, focused controls, a protocol case, and selected fingerprints receive checks. | The selected fingerprint differences have causal evidence. The later approved baseline updates have a separate record. The publication record adds release compilation and mechanical checks. Combined peer-permission activation remains open. |

The patch adds a required `dropFrame()` operation to `IAckHandler`. Both built-in handlers already implement it. The committed-frame owner needs that operation to remove the old ACK entry before numbered replacement. The migration guide records this additional interface change.

The protection fixtures now use read-only successor observations. Their earlier destructive access invalidated the new preparation. Their assertions also reflect the earlier refusal boundary, one adaptive DATA query across RTS/CTS, complete exchange containment, and zero-limit fragment bursts. The non-HT aggregation case now requires ordinary MSDUs.

## Verification commands and results

All commands below run from the active checkout unless another directory is specified. All compiled tests use debug mode. Deterministic module cases use seed zero. The final library SHA-256 is `1de36100640807565d8456334072201ff3f412a3e5fbfe28e0376a5e0fc57bf2`.

```sh
make MODE=debug -j8
inet_run_unit_tests -m debug -f 'Ieee80211(BoundedFragmentation|SingleProtection|MultipleProtection|TxopProcedure)_.*\.test'
inet_run_module_tests -m debug -f 'Ieee80211(BasicTxopAdmission|TxopAdmission|TxopMsduRecovery|PreparedQosFragments|PreparedQosData|CompressedBlockAckAdmission|CompressedBlockAckExchange|BasicCompatibility|BlockAckVariant|BlockAckFragmentRetirement|QosRecoveryPeerIdentity|EdcaDataLifetime.*|HcfInternalCollision|HcfManagementRecovery|HcfSingleProtection|HcfMultipleProtection|SingleProtectionGroup|PreparedCancellation|AmpduAdmission|AmpduPreparation|AmpduRecovery|AmpduExchange)_.*\.test'
inet_run_protocol_tests -p inet -m debug -w wifi -f 'Legacy_Fragmentation\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
git diff --check
```

The build passed with exit status zero. Four unit fixtures, thirty-one module fixtures, and the selected protocol fixture passed. The scoped architecture check passed. The whitespace check passed. The final module campaign includes all 96 Normal Ack runs and all 36 Basic runs. The final logs use the `c5-` prefix in the [artifact folder](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/).

The first admission campaign contained 48 infrastructure runs. It covered both protection classes, 6 and 24 Mbit/s, RTS enabled or disabled, and zero or positive limits. CTS-loss cases removed zero, two, or seven selected responses through the production corrupt-frame path. The extended campaign adds source expiry during CTS wait, a mode-set change during CTS wait, and a real recovery call for an injected internal collision. The radio and both peer MACs remain production components.

The one-tick checks call production HCF admission with controlled budget inputs. A complete exchange fails one tick above the positive budget and passes at equality. TXNAV equality fails. An exchange one tick shorter than TXNAV passes. These direct checks do not change the budgets for actual wireless transmissions.

The direct checks also distinguish fifteen and sixteen nondynamic fragments. The sixteen-fragment exception can permit the selected first exchange beyond the positive limit. The fifteen-fragment form cannot use that exception. HCF checks a configured sixteen-fragment partition before it declares a bounded partition impossible.

Sixteen additional runs passed with the real Onoe controller. Another sixteen passed with the real AARF controller. Test subclasses count queries and retain their returned modes without a changed controller decision. HCF must submit the returned DATA mode. DATA after CTS must retain the query count recorded at RTS submission. The observer separately tracks local DATA lengths, fragment retries, and earlier holder frames to recognize permitted positive-limit exceptions.

The adaptive commands run from `tests/module/work/Ieee80211TxopAdmission_1` after the filtered module runner builds that fixture:

```sh
./Ieee80211TxopAdmission_1_dbg -u Cmdenv -r 0..7,48..55 --check-signals=false '--**.rateControl.typename="AdmissionOnoe"' '--**.rateControl.initialRate=6Mbps' '--**.rateSelection.dataFrameBitrate=-1bps'
./Ieee80211TxopAdmission_1_dbg -u Cmdenv -r 0..7,48..55 --check-signals=false '--**.rateControl.typename="AdmissionAarf"' '--**.rateControl.initialRate=6Mbps' '--**.rateSelection.dataFrameBitrate=-1bps'
```

The retained final logs include build, unit, module, protocol, adaptive-controller, architecture, and fingerprint results. Generated per-run output resides under `tests/module/work/` and `tests/unit/work/`. The artifact manifest records their hashes and the final library hash. The earlier logs remain evidence for the independent C1–C4 work.

The interface checker reports fifteen failing classes. The same checker reports the same fifteen failures in the unchanged A-MPDU checkout at the starting revision. The changed interfaces add pure virtual declarations and no default bodies. These inherited findings do not establish a new interface defect.

Clang-tidy checked all seven changed C++ implementation files through a temporary compilation database with the debug build flags. It returned zero but reported warnings in unchanged code. A comparison with added lines found no warning on an added line. A final HCF check followed the correction to Basic fragment reservations. This check does not establish a clean project-wide C++ gate.

## C5 contract and direct cases

The accepted R1 decision authorizes the Basic activation surface already specified by revision 9. HCF owns dispatch, admission, and the commitment boundary. QosRecoveryProcedure owns the short and long counts and the accepted-agreement exemption. InProgressFrames owns committed fragments and group disposal. The source trace confirmed the legacy bypass before the C5 edit. The selected owners also handle RTS failure, Normal Ack failure, internal collisions, lifetime expiry, and agreement disposal.

The production fixture uses infrastructure association and actual peer MAC responses. It removes selected CTS or ACK responses to trigger failure. It checks retention beyond the configured limit of seven attempts. The RTS cases inject nine CTS losses before or after local DATA. The DATA case injects nine ACK losses under an accepted agreement. The collision case invokes the production collision handler beyond the limit. Each retained allocation keeps its absolute deadline.

The table maps the plan's named cases to the fixture scenarios. Each scenario runs at 6 and 24 Mbit/s under single and multiple protection.

| Planned case | Fixture scenario and checks |
| --- | --- |
| BasicNonHtInitialException | Scenario 0 permits the initial ordinary MSDU under an accepted agreement. The independent completion observer excludes later DATA from that exception. |
| BasicNonHtContinuation | Scenario 0 checks complete DATA and ACK containment outside an exception. The corrected B1 probe supplies the explicit earlier failure comparison. |
| BasicNonHtRtsExhaustionBeforeData | Scenario 1 retains the allocation after nine forced CTS losses. It checks the short-counter increments without prior local DATA. |
| BasicNonHtRtsExhaustionAfterData | Scenario 2 first removes an ACK under the agreement. It then removes nine CTS responses for that transmitted allocation. |
| BasicNonHtNormalAckUnderAgreement | Scenarios 2, 3, and 8 force Normal Ack only under an accepted agreement. Setup still uses the production policy. |
| BasicNonHtDataLifetime | Scenario 4 retains failed RTS attempts beyond the short limit, then checks record removal at expiry. Scenario 8 retains nine failed DATA exchanges. |
| BasicNonHtFragments | Scenario 5 checks one allocation per zero-limit TXOP. Its Duration assertions include the next fragment and omit an absent immediate ACK. |
| BasicMsduRecoveryBoundaries | The direct recovery fixture checks distinct short and long limits, ACK resets, agreement exemption, and unrelated allocation preservation. Scenario 3 adds production collision retention. |
| Basic fallback without an agreement | Scenario 6 sets the eligibility threshold to zero. Both enabled Basic handlers use Normal Ack without setup. Both hops deliver all thirty MSDUs within the applicable bounds. |
| BasicMixedHtPeerAllowed | Scenario 7 uses HT stations and a non-HT AP with shared ERP modes. Both endpoint roles establish Basic agreements and deliver DATA. |

The mixed-peer configuration selects `g(mixed)` at the AP. Its legacy ERP modes also occur in `n(mixed-2.4Ghz)`. The `g(erp)` catalog instead contains ERP-only mode objects. The receiver requires mode-set membership. Management and control use the shared 6 Mbit/s mode so association can complete. This configuration choice changes no production permission rule.

The fragment-retirement control now permits the next fragment before agreement expiry after the first fragment receives its ACK. Its earlier assertion required a new contention period between those fragments. The zero-limit correction removes that period. The control still checks complete retirement, later sequence allocation, and preservation of the other peer.

## B1: legal Basic baseline

The [Basic probe](../reproductions/ieee80211-txop-admission-remaining/BasicNonHtTxopAdmissionProbe.test) uses non-HT `g(erp)` peers, seed zero, and production infrastructure association. It disables both aggregation policies. Its MSDUs use the retained 700-byte application payload. The fragmentation threshold is 2346 bytes, the RTS threshold is 500 bytes, and the TXOP limit is 1024 microseconds.

The campaign uses the current debug library while Basic still selects its legacy admission path. It does not use the unavailable historical S5 library. The independent modern changes do not activate Basic admission. This evidence therefore establishes the open Basic defect on the current implementation target.

| DATA rate | Protection | Invalid completion boundaries | Invalid continuations |
| --- | --- | ---: | ---: |
| 6 Mbit/s | Single | 10 | 0 |
| 6 Mbit/s | Multiple | 10 | 0 |
| 24 Mbit/s | Single | 0 | 0 |
| 24 Mbit/s | Multiple | 18 | 18 |

The last row satisfies B1. Both originators establish accepted Basic agreements. Each of its eighteen violations occurs at DATA completion after earlier DATA in the same TXOP. Basic DATA with Block Ack policy needs no immediate ACK. The initial ordinary-MSDU exception cannot permit these continuations. Both hops together deliver all thirty source MSDUs in this diagnostic campaign.

For example, one TXOP starts at 1.003257 seconds and ends at 1.004281 seconds. Its third DATA exchange starts its first holder frame at 1.004025 seconds. That leaves 256 microseconds. The protected exchange takes 374 microseconds through DATA completion. It ends at 1.004399 seconds, which exceeds the bound by 118 microseconds. The accepted Basic agreement supplies no exception for this third DATA exchange.

The retained raw logs are [6 Mbit/s single](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/basic-6-single.txt), [6 Mbit/s multiple](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/basic-6-multiple.txt), [24 Mbit/s single](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/basic-24-single.txt), and [24 Mbit/s multiple](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/basic-24-multiple.txt).

The initial execution used the retained original probe with command-line overrides. Its run numbers 0, 1, 4, and 5 correspond to rows 0, 1, 2, and 3 of the Basic-only probe. The final campaign builds that Basic-only probe directly. It records the first holder start and elapsed cost through each violated completion boundary. The executable resides in `/tmp/inet-txop-basic-baseline/work/BasicNonHtTxopAdmissionProbe`. It links the active checkout's debug library. All four commands returned zero.

The original Normal Ack runs 2, 3, 6, and 7 also returned zero. A separate check rejected any `INVALID` line or runtime error. All four logs passed that check. Their exact RTS threshold remains 500 bytes. Those runs supplement the asserted infrastructure fixture.

## Fingerprints and remaining work

The selected fingerprint command ran from `tests/fingerprint`:

```sh
./fingerprinttest -d -m '(showcases/wireless/(aggregation|blockack)/.*-c (NoAggregation|Aggregation|Fragmentation) -r 0)'
```

The original final command returned status one after three runs. All three rows differ from the recorded CSV at that revision. The [final raw result](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/c5-fingerprints.txt) preserves the old and new values. That campaign changed no baseline. The user later authorized updates; the [baseline record](../reproductions/ieee80211-txop-admission-remaining/fingerprints-20261019/README.md) covers the broader related scope.

The [causality report](../reproductions/ieee80211-txop-admission-remaining/execution-20261019/c5-fingerprint-causality.md) explains the selected differences. Aggregation first differs when the old non-HT path forms a 354-byte A-MSDU. The corrected path sends one 192-byte ordinary MPDU. Basic fragmentation first differs when the corrected zero-limit sequence sends its next fragment after SIFS. The old path releases the channel. The NoAggregation graphical value also differs on the unchanged base library. The exact wrapper command produces identical four-value results on both libraries. That graphical mismatch is inherited.

The following work remains open:

1. Complete the combined peer-permission activation checks when P2 exists.
2. Record the approved fingerprint values with their causal commits; the later baseline record supplies this evidence.
3. Retain the publication record's distinction between passing focused checks and inherited global findings.

The standards evidence uses IEEE Std 802.11-2024, Clauses 10.4, 10.23.2.9, 10.23.2.12.1, and 10.3.2.6. Their canonical corpus nodes resolve. The corpus build resides in `/tmp/inet-txop-standards`. Its global linter reports unresolved references and ambiguities. The 2016 R1 evidence requires original PDF text because this corpus does not index that edition. The user explicitly accepted the R1 interpretation before C5 activation.
