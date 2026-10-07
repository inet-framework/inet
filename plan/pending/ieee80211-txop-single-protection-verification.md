# Single protection implementation and verification

> **Kind:** report · **Status:** snapshot 2026-10-07 · **Seal:** none · **Owns:** — · **Stands on:** [approved plan](ieee80211-txop-single-protection.md)

## Result and scope

A1, A2, A3, B, and C are implemented locally. The user requested execution of plan revision 12. The source started at `1b31d3dfcd252cd35e78a5f9d8398a3bf02a0434` on `feat/ieee80211-txop-single-protection`. The local series contains seven commits. The working tree retains the user's existing `AGENTS.md` change. No baseline changed.

HCF selects the DATA ACK branch once before execution. It retains the admitted DATA mode through RTS/CTS within the scoped single protection path. Additional exchanges pass independent TXNAV and positive TXOP containment checks at the plan's decision time. Forecast copies preserve retained successor tags and ACK policy. Normal refusal removes the unsent step before finish observers inspect history.

Rx owns success-only TXNAV separately from received NAV. Its endpoint uses actual holder PPDU end and encoded Duration/ID. HCF also disables contention through its active sequence. An initial failed DATA frame can leave TXNAV zero while HCF waits for ACK. Without this permission, another AC can receive a grant before ACK timeout. The required permission gate preserves internal-collision handling, CTS queries, and normal AIFS after channel release.

The initial compatibility fallback remains. TXOP-based fragmentation, Block Ack continuation admission, BAR forecasts, zero-limit companion rules, and optional overruns remain deferred. The migration guide, MAC documentation, and release notes state these limits.

## Build and focused tests

All commands below ran from the checkout root in debug mode with fresh source and generated code:

```sh
make -j$(nproc) MODE=debug
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211(SingleProtection_1|TxopProcedure_1|MultipleProtection_1|TxNavChoice_1)\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(HcfHeader_1|FrameSequenceObservations_1|SingleProtectionGroup_1|HcfSingleProtection_1|Txnav_1|HcfMultipleProtection_[12]|TxopProtectionSelectors_[12]|RecipientNav_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211Txnav_1\.test$'
```

The fresh build exits zero. Four unit fixtures pass. Twelve module fixtures pass across 104 module runs. The combined final unit/module shell exits zero. The final Txnav rerun exits zero after the AC_BE selector correction.

Tests use Cmdenv and seed 0. The continuation fixture supplies 24 runs, including fixed 0.2 µs propagation cases and one 0.48 µs case.

| Behavior | Direct evidence |
| --- | --- |
| Published ACK policy and RTS field | `Ieee80211HcfHeader_1`: four runs, custom and built-in Block Ack policy, direct DATA and RTS/CTS. |
| Ordered start/finish and refused history | `Ieee80211FrameSequenceObservations_1`, plus Txnav first-step refusal with valid ownership and active result filters. |
| Group classification and legacy successor term | `Ieee80211SingleProtectionGroup_1`: four runs with group management/non-QoS DATA and Block Ack support enabled/disabled. |
| TXNAV success, failure, replacement, CTS, and contention | `Ieee80211Txnav_1`: five runs; `Ieee80211HcfMultipleProtection_1`: eighteen runs; recipient NAV controls. |
| Independent budgets, equality, modes, forecasts, and progress | `Ieee80211HcfSingleProtection_1`: twenty-four runs through production HCF, Tx, Rx, and radios. |
| Field equations and wire values | `Ieee80211SingleProtection_1`: clamp, ceiling, 32767 µs boundary, rejection, and serialized bytes. |
| Compatibility | Existing multiple protection, selector, DCF recipient NAV, management recovery, and AP RTS timeout fixtures. |

Build logs are `/tmp/txop-build-debug.log`, `/tmp/txop-build-debug-final.log`, and `/tmp/txop-build-owner-gate.log`. Final runner logs are `/tmp/txop-unit-final.log`, `/tmp/txop-module-final.log`, and `/tmp/txop-txnav-final.log`. Each fixture retains its simulation output under `tests/unit/work/` or `tests/module/work/`.

Initial new-fixture failures involved namespace registration, unassigned parameters, fixed NED submodule types, and mode-tag observations. Corrected fixtures now reach their intended production paths. Preview commands supplied no behavioral evidence. The runner's dry-run path attempted support compilation or reported a task-index error.

## Legacy fingerprints

The focused inventory contains ten rows. These commands ran from `tests/fingerprint` with debug binaries and ingredients `tplx`, `~tNl`, and `~tND`:

```sh
./fingerprinttest -d -t 1 -m '/examples/wireless/(qos|ratecontrol)/|/showcases/wireless/(qos|ratecontrol)/' -f 'tplx' -f '~tNl' -f '~tND' -l /tmp/txop-fingerprint-runner.log
./fingerprinttest -d -t 1 -m '/examples/wireless/qos/.*MacQos|/showcases/wireless/qos/.*-c Qos' -f 'tplx' -f '~tNl' -f '~tND' -l /tmp/txop-fingerprint-final-runner.log
./fingerprinttest -d -t 1 -m '/examples/wireless/(qos|ratecontrol)/|/showcases/wireless/(qos|ratecontrol)/' -x '/examples/wireless/qos/.*MacQos|/showcases/wireless/qos/.*-c Qos' -f 'tplx' -f '~tNl' -f '~tND' -l /tmp/txop-fingerprint-controls-runner.log
```

The first campaign exposed four runtime errors from the missing contention permission. A targeted trace reproduced the exact showcase event and time. The corrected source removes those runtime errors. Five final HCF rows complete with fingerprint mismatches; that runner exits one. Five final control rows explicitly verify all three expected fingerprints; that runner exits zero.

The five changed rows contain fifteen changed fingerprint values: three values for each configuration. The five control rows verify fifteen values. Each row represents one simulation trajectory with three fingerprint ingredients.

| Changed row, run 0 | Final `tplx` | Final `~tNl` | Final `~tND` |
| --- | --- | --- | --- |
| Showcase `Qos` | `b300-b302` | `f942-332c` | `a8a4-3d2e` |
| Example `MacQos` | `24d0-e755` | `44f5-cd51` | `614f-396c` |
| Example `MacQosWithoutAggregation` | `50bf-6ad3` | `5c4a-8ebf` | `2d77-1fcb` |
| Example `MacQosWithRtsCts` | `f220-d33d` | `c711-534c` | `9c5b-c6f8` |
| Example `MacQosWithBlockAck` | `27f3-3799` | `bf7e-a5fd` | `2e31-6cf2` |

The controls are example/showcase non-QoS, example rate control `Mac`, and showcase `NoRateControl` and `AarfRateControl`. No recorded expectation changed. A fresh build of the original source verifies all five QoS expectations. That comparison runner exits zero.

All five rows first differ in their first Beacon. The original field is 44 µs; the final field is zero. Frame time, type, addresses, sequence number, length, and selected mode remain equal. Raw frame bytes differ only at Duration offset 2 and the four computed FCS bytes. A3 removes the incorrect current-frame ACK estimate from group management. Original AP and receiver NAV reservations consequently disappear for this zero-field Beacon.

Ten short comparison runs use the same native configuration names, run 0, computed FCS/checksums, and 1.05 s window. All ten exit zero. The first-divergence report is `/tmp/txop-field-first-divergence-report.md`. Decoded headers and raw-byte comparisons are `/tmp/txop-field-first-divergences.json` and `/tmp/txop-field-first-byte-differences.json`. Each configuration retains AP and station PCAPs, header CSVs, and logs under `/tmp/txop-field-{base,final}-` prefixes. The group production fixture directly verifies the corrected equation. Later trajectories can also reflect the other planned changes.

The first-divergence evidence covers all five changed configurations. It does not attribute all fifteen final values to A3 alone. The intermediate campaigns also show additional changes after A1, B publication, and C admission. For example, showcase `Qos` has different values at A3, B publication, and the final source. The [local commit series](#local-commit-series) records those values and A1's unresolved Block Ack attribution. Further causal evidence remains necessary for a baseline proposal.

The original-source comparison uses the same five-row fingerprint command, with `/tmp/txop-fingerprint-prerequisite-runner.log` as its log argument. Its summary is `/tmp/txop-fingerprints-prerequisite.log`. The original build log is `/tmp/txop-build-prerequisite.log`. Original simulation output is preserved under `/tmp/txop-prerequisite-fingerprint-results/`.

The source comparison preserves the complete final patch before temporary reversal. Restoration compares that patch byte for byte before a fresh final build. The source patch checksum is `04242a01925d6ef4cbcb722db10cb3fd04ad777577c05ce86d067d55ad9fecf4`. Unrelated local changes remain outside that reversal.

The restored build exits zero; its log is `/tmp/txop-build-restored-final.log`. Its library matches the tested final library byte for byte. The library checksum is `1c73cc7cd0fa39db23c3a041a5de110b15369ac2fa9584bda87b6d657cd3f227`.

Final summaries are `/tmp/txop-fingerprints-final.log` and `/tmp/txop-fingerprints-controls.log`. Complete final simulation output remains under `tests/fingerprint/results/` and `/tmp/txop-final-fingerprint-results/`. The ownership diagnostic report is `/tmp/txop-qos-diagnostic-report.md`; its timeline is `/tmp/txop-qos-diagnostic.timeline.log`.

## Rules, review, and limits

The scoped architecture gate passes. The scoped naming gate exits one with 32 inherited candidates. Its final output matches the original output exactly. Source paths are unsealed. No Packet API, ledger, or seal changed. `git diff --check` passes.

The independent reviewer finds no actionable source defect after the contention correction. Its canonical checklist records 28 PASS, 19 N/A, zero FLAG, and zero QUESTION results. The reviewer verifies all 29 reviewed file hashes, the source patch checksum, and the restored library checksum. The review report is `/tmp/ieee80211-single-protection-review.md`.

The original and final gate logs are `/tmp/txop-architecture-before.log`, `/tmp/txop-architecture-final.log`, `/tmp/txop-naming-before.log`, and `/tmp/txop-naming-final.log`.

The local standards corpus is fresh. Selected IEEE Std 802.11-2024 clauses confirm field equations, successful TXNAV replacement, CTS conditions, and continuation restrictions. The timer sample instant remains the plan's declared inference. Corpus lint retains unrelated extraction warnings. No PDF inspection was necessary.

Release compilation, the broader 361-row wireless campaign, and publication gates remain unrun. Git writes now succeed. The local series separates the independent repairs, shared TXNAV, and continuation admission. No pull request exists from this execution. Publication requires the planned separate pull requests and remaining checks. Baseline changes require separate approval for the five exact rows and their causes.

## Local commit series

The series adds seven commits above the recorded source revision. B separates the Rx mechanism from HCF success publication. C separates normal refusal support from HCF admission. Each intermediate tree receives a fresh debug build and direct tests. These commits do not replace the separate pull requests required before publication.

| Step | Commit | Build | Direct test result | Fingerprint result |
| --- | --- | --- | --- | --- |
| A1 | `521ad120e8` | exit 0 | five module fixtures PASS | four QoS rows verify; Block Ack differs; exit 1 |
| A2 | `c9529ede05` | exit 0 | seven module fixtures PASS | showcase Qos verifies all three ingredients; exit 0 |
| A3 | `348371cfcc` | exit 0 | four module fixtures PASS | five QoS rows differ; exit 1 |
| B mechanism | `f002421a76` | exit 0 | one unit PASS; three compatibility modules PASS; direct Rx rerun PASS | showcase Qos values equal A3; native mismatch, exit 1 |
| B publication | `1015799dad` | exit 0 | nine module fixtures PASS | five QoS rows differ; exit 1 |
| C refusal | `975bd0b92f` | exit 0 | seven module fixtures PASS | showcase Qos values equal B publication; native mismatch, exit 1 |
| C admission | this final commit | exit 0 | four unit and twelve module fixtures PASS; both runners exit 0 | original final results remain applicable through exact source and library equality |

The intermediate fingerprint campaigns produce no runtime errors. A1's Block Ack row computes `7752-8da4/tplx`, `de3f-0e47/~tNl`, and `823c-af67/~tND`. Its header fixture proves the selected-policy and RTS-field correction. Full attribution of every intermediate Block Ack fingerprint change remains unverified. No intermediate baseline value changes.

The B mechanism's Qos values equal A3: `6e4e-b4a5/tplx`, `f648-b40f/~tNl`, and `455c-38bb/~tND`. C refusal's Qos values equal B publication: `5535-3b55/tplx`, `f140-122e/~tNl`, and `4569-8e6b/~tND`. These comparisons check unchanged calculated values; they do not verify the old recorded expectations.

Build logs use `/tmp/txop-series-<step>-build.log`, where step is `A1`, `A2`, `A3`, `B0`, `B1`, `C0`, or `C1`. Every build uses `make -j$(nproc) MODE=debug`. Module logs use `/tmp/txop-series-<step>-tests.log`. The commands are:

```sh
# A1
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(HcfHeader_1|HcfMultipleProtection_[12]|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
# A2
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(FrameSequenceObservations_1|HcfHeader_1|HcfMultipleProtection_[12]|RecipientNav_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
# A3
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(SingleProtectionGroup_1|HcfHeader_1|HcfMultipleProtection_[12])\.test$'
# B0
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(Txnav_1|HcfMultipleProtection_[12]|RecipientNav_1)\.test$'
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211TxNavChoice_1\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211Txnav_1\.test$'
# B1
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(FrameSequenceObservations_1|Txnav_1|HcfMultipleProtection_[12]|RecipientNav_1|HcfHeader_1|SingleProtectionGroup_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
# C0
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(FrameSequenceObservations_1|Txnav_1|HcfMultipleProtection_[12]|RecipientNav_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
# C1
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211(SingleProtection_1|TxopProcedure_1|MultipleProtection_1|TxNavChoice_1)\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(HcfHeader_1|FrameSequenceObservations_1|SingleProtectionGroup_1|HcfSingleProtection_1|Txnav_1|HcfMultipleProtection_[12]|TxopProtectionSelectors_[12]|RecipientNav_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
```

B0's first module runner reports three PASS results and one stale generated-source error. The corrected direct Rx rerun exits zero in `/tmp/txop-series-B0-txnav-rerun.log`. The old generated directory remains at `/tmp/txop-series-B0-stale-work/`. Its unit runner exits zero in `/tmp/txop-series-B0-unit.log`. No test ran before the corrected fresh B0 build.

A1, A3, and B1 use the five-row QoS fingerprint command from the legacy section. A2, B0, and C0 select only `/showcases/wireless/qos/.*-c Qos`. Each uses debug mode and the same three ingredients. Logs use `/tmp/txop-series-<step>-fingerprints.log` and `/tmp/txop-series-<step>-fingerprint-runner.log`.

The final source patch matches the saved tested patch byte for byte. The final debug library matches the saved tested library byte for byte. All source, test, and product-document files match the saved reviewed final files. Only plan execution records add the series results.

C1's final unit log is `/tmp/txop-series-C1-unit.log`. Its final module log is `/tmp/txop-series-C1-tests.log`. The backup ref is `refs/backup/txop-single-protection-before-local-series-20261007`. Independent per-commit reviews remain under `/tmp/txop-series-review/`.

Local series checks use the commands below. Their logs are `/tmp/txop-series-commits.log`, `/tmp/txop-series-classification.log`, and `/tmp/txop-series-source-seals.log`. These checks do not replace the global publication gates.

```sh
doc/project/enforcement/check-commits.sh 1b31d3dfcd..HEAD
doc/project/enforcement/check-classification.sh 1b31d3dfcd..HEAD
doc/project/enforcement/check-source-seals.sh --base 1b31d3dfcd
```
