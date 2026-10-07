# Single protection implementation and verification

> **Kind:** report · **Status:** snapshot 2026-10-07 · **Seal:** none · **Owns:** — · **Stands on:** [approved plan](ieee80211-txop-single-protection.md)

The [fingerprint analysis dated 2026-10-08](ieee80211-txop-single-protection-fingerprints.md) extends this snapshot with causes for all twenty supplied CI failures. It also records the approved baseline update and its verification.

The older pending approval and attribution statements below are superseded by that fingerprint analysis. The [strict TXNAV revision](#strict-txnav-admission-revision) supersedes this snapshot's TXNAV containment rule and its affected expectations. The original results remain evidence for their recorded source.

The [approved strict TXNAV baselines](#approved-strict-txnav-baselines) supersede the later pending baseline statements. That section records CI agreement, explicit approval, the exact replacement scope, and verification after the update.

The local investigation logs are not part of this pull request. Use the linked CI runs and tracked tests for reviewable verification.

## Result and scope

This section records source `a224e4677b` on 2026-10-07. The [strict TXNAV admission revision](#strict-txnav-admission-revision) supersedes its containment rule. The [approved strict TXNAV baselines](#approved-strict-txnav-baselines) supersede its baseline and publication status.

A1, A2, A3, B, and C are implemented locally. The user requested execution of plan revision 12. The source started at `1b31d3dfcd252cd35e78a5f9d8398a3bf02a0434` on `feat/ieee80211-txop-single-protection`. This dated result precedes the approved baseline update below.

HCF selects the DATA ACK branch once before execution. It retains the admitted DATA mode through RTS/CTS within the scoped single protection path. Additional exchanges pass independent TXNAV and positive TXOP containment checks at the plan's decision time. Forecast copies preserve retained successor tags and ACK policy. Normal refusal removes the unsent step before finish observers inspect history.

Rx owns success-only TXNAV separately from received NAV. Its endpoint uses actual holder PPDU end and encoded Duration/ID. HCF also disables contention through its active sequence. An initial failed DATA frame can leave TXNAV zero while HCF waits for ACK. Without this permission, another AC can receive a grant before ACK timeout. The required permission gate preserves internal-collision handling, CTS queries, and normal AIFS after channel release.

The initial compatibility fallback remains. TXOP-based fragmentation, Block Ack continuation admission, BAR forecasts, zero-limit companion rules, and optional overruns remain deferred. The migration guide, MAC documentation, and release notes state these limits.

## Build and focused tests

These build and test results apply to source `a224e4677b`. The [approved strict TXNAV baselines](#approved-strict-txnav-baselines) record later builds and fixture results.

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

The local build and runner logs are not part of this pull request. The tracked fixture definitions and commands above let reviewers repeat the checks.

Initial new-fixture failures involved namespace registration, unassigned parameters, fixed NED submodule types, and mode-tag observations. Corrected fixtures now reach their intended production paths. Preview commands supplied no behavioral evidence. The runner's dry-run path attempted support compilation or reported a task-index error.

## Legacy fingerprints

These five-row results apply to source `a224e4677b`. The [fingerprint analysis](ieee80211-txop-single-protection-fingerprints.md) supersedes their pending cause analysis. The [approved strict TXNAV baselines](#approved-strict-txnav-baselines) supersede their pending baseline status.

The focused inventory contains ten rows. These commands reproduce the selections from `tests/fingerprint` with debug binaries and ingredients `tplx`, `~tNl`, and `~tND`:

```sh
./fingerprinttest -d -t 1 -m '/examples/wireless/(qos|ratecontrol)/|/showcases/wireless/(qos|ratecontrol)/' -f 'tplx' -f '~tNl' -f '~tND'
./fingerprinttest -d -t 1 -m '/examples/wireless/qos/.*MacQos|/showcases/wireless/qos/.*-c Qos' -f 'tplx' -f '~tNl' -f '~tND'
./fingerprinttest -d -t 1 -m '/examples/wireless/(qos|ratecontrol)/|/showcases/wireless/(qos|ratecontrol)/' -x '/examples/wireless/qos/.*MacQos|/showcases/wireless/qos/.*-c Qos' -f 'tplx' -f '~tNl' -f '~tND'
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

Ten short comparison runs use the same native configuration names, run 0, computed FCS/checksums, and 1.05 s window. All ten exit zero. The local diagnostic output includes decoded headers, raw-byte comparisons, and station captures. It is not part of this pull request. The group production fixture directly verifies the corrected equation. Later trajectories can also reflect the other planned changes.

The first-divergence evidence covers all five changed configurations. It does not attribute all fifteen final values to A3 alone. The intermediate campaigns also show additional changes after A1, B publication, and C admission. For example, showcase `Qos` has different values at A3, B publication, and the final source. The [fingerprint report](ieee80211-txop-single-protection-fingerprints.md) records the later causal evidence and approval.

The original-source comparison uses the same five-row fingerprint selection. Its runner exits zero against the original expectations.

The source comparison preserves the complete final patch before temporary reversal. Restoration compares that patch byte for byte before a fresh final build. The source patch checksum is `04242a01925d6ef4cbcb722db10cb3fd04ad777577c05ce86d067d55ad9fecf4`. Unrelated local changes remain outside that reversal.

The restored build exits zero. Its library matches the tested final library byte for byte. The library checksum is `1c73cc7cd0fa39db23c3a041a5de110b15369ac2fa9584bda87b6d657cd3f227`.

The local fingerprint and ownership diagnostic output is not part of this pull request.

## Rules, review, and limits

These review and pending publication statements apply to source `a224e4677b`. They do not describe the later published PR. The [approved strict TXNAV baselines](#approved-strict-txnav-baselines) record the later evidence.

The scoped architecture gate passes. The scoped naming gate exits one with 32 inherited candidates. Its final output matches the original output exactly. Source paths are unsealed. No Packet API, ledger, or seal changed. `git diff --check` passes.

The independent reviewer finds no actionable source defect after the contention correction. Its canonical checklist records 28 PASS, 19 N/A, zero FLAG, and zero QUESTION results. The reviewer verifies all 29 reviewed file hashes, the source patch checksum, and the restored library checksum. The local review report is not part of this pull request.

The original and final architecture and naming gate outputs match.

The local standards corpus is fresh. Selected IEEE Std 802.11-2024 clauses confirm field equations, successful TXNAV replacement, CTS conditions, and continuation restrictions. The timer sample instant remains the plan's declared inference. Corpus lint retains unrelated extraction warnings. No PDF inspection was necessary.

The later [approved strict TXNAV baselines](#approved-strict-txnav-baselines) replace this snapshot's pending publication and baseline status.

## Local commit series

This historical series ends at source `a224e4677b`. Later history supersedes its publication plan. The rows remain evidence for the recorded local series.

The series adds the commits listed below. B separates the Rx mechanism from HCF success publication. C separates normal refusal support from HCF admission. Each intermediate tree receives a fresh debug build and direct tests.

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

Every series build uses `make -j$(nproc) MODE=debug`. The commands below reproduce the module test selections:

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

B0's first module runner reports three PASS results and one stale generated-source error. The corrected direct Rx rerun and unit runner exit zero. No test ran before the corrected fresh B0 build.

A1, A3, and B1 use the five-row QoS fingerprint selection from the legacy section. A2, B0, and C0 select only `/showcases/wireless/qos/.*-c Qos`. Each uses debug mode and the same three ingredients.

The final source patch matches the saved tested patch byte for byte. The final debug library matches the saved tested library byte for byte. All source, test, and product-document files match the saved reviewed final files. Only plan execution records add the series results.

The backup ref is `refs/backup/txop-single-protection-before-local-series-20261007`. The local per-commit review output is not part of this pull request.

Local series checks use the commands below. These checks do not replace the global publication gates.

```sh
doc/project/enforcement/check-commits.sh 1b31d3dfcd..HEAD
doc/project/enforcement/check-classification.sh 1b31d3dfcd..HEAD
doc/project/enforcement/check-source-seals.sh --base 1b31d3dfcd
```

## Strict TXNAV admission revision

This section records strict source `24e32249` before approval of its new fingerprint baselines. The [approved strict TXNAV baselines](#approved-strict-txnav-baselines) supersede its pending approval and release-test statements.

The user approved this revision on 2026-10-08 after the source and standards assessment. The revision starts at `3bb1b2e52c73d20f06b583f18eba862df3f8e8cf`. HCF now requires the complete exchange cost to be strictly less than TXNAV at the decision before SIFS. The initial SIFS remains part of the separate positive TXOP containment check. The [revised design](ieee80211-txop-single-protection.md#43-define-admission-budgets) owns the equations, interpretation, and scope.

The production change replaces one comparison in `Hcf::transmitFrame()`. It adds no state, timer, interface, field allowance, or configuration parameter. Tests retain initial progress, zero-limit restrictions, RTS/CTS mode retention, synchronous refusal, and independent positive TXOP containment. The user guide, migration guide, release note, plan, and summary now explain the separate comparisons.

### Production evidence

The revised `Ieee80211HcfSingleProtection_1` fixture fails at scenario 2 with the original gate. Its runner exits one. That scenario samples 160.001 µs of TXNAV for a 160 µs exchange. The original gate also counts 16 µs of initial SIFS and refuses continuation. After the source change, all 26 fixture runs pass.

| Scenario | Trigger and observation |
| --- | --- |
| 1 | A 160 µs exchange equals the sampled TXNAV. HCF refuses it and preserves the unsent frame. |
| 2 | A 160 µs exchange has 160.001 µs of sampled TXNAV. HCF admits it. |
| 24 | A 160 µs exchange exceeds 159.999 µs of sampled TXNAV. HCF refuses it. |
| 21 | Fixed 0.2 µs propagation leaves 175.6 µs TXNAV. Both DATA frames execute within one grant. The initial field remains 236 µs. |
| 25 | TXNAV expires before the second ACK arrives. Another ready access category receives its grant only after that ACK and sequence completion. |
| 5, 6, 16, 22 | An expanded TXNAV query isolates positive TXOP boundaries. Equality proceeds; excess and elapsed propagation time cause refusal. |

Scenario 21 retains a 16 µs initial SIFS and a 160 µs exchange cost. The second DATA starts 176.4 µs after the first DATA starts. No field allowance compensates for propagation. Scenario 25 verifies two video DATA frames followed by one best-effort DATA frame, with no retry or failed-rate feedback. These observations establish continuation and contention exclusion, rather than a throughput claim.

A short native adhoc `Fragmentation`, run 0, comparison uses the original topology and a 0.511 s observation window. Sender and receiver captures observe fragment 1 of sequence 0 after the first DATA/ACK exchange. The sender capture records its PPDU end at 0.500443334 s. The earlier containment source records 0.500830 s in the preceding fingerprint analysis. The first DATA field remains 292 µs, and fragment 1 retains its 168 µs field. This revision restores the continuation without a propagation allowance in either field.

The diagnostic uses computed checksums and FCS, seed 0, and debug mode. The capture point is each host's `wlan[0]`, with `ieee80211mac` in the recorder's protocol selection. The local captures and decoded values are not part of this pull request. Native fingerprint runs remain separate from the short capture run.

### Build and regression results

Both the original-gate build and revised-gate build exit zero. The commands run from the checkout root:

```sh
make -j12 MODE=debug
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211(SingleProtection_1|TxopProcedure_1|MultipleProtection_1|TxNavChoice_1)\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(HcfHeader_1|FrameSequenceObservations_1|SingleProtectionGroup_1|HcfSingleProtection_1|Txnav_1|HcfMultipleProtection_[12]|TxopProtectionSelectors_[12]|RecipientNav_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mac
git diff --check
```

Four unit fixtures pass. Twelve module fixtures pass across 106 module runs. Both runners exit zero. Module fixtures use Cmdenv and seed 0. The scoped architecture check and whitespace check also exit zero. The touched source is unsealed.

The debug library SHA-256 is `43d46b890a3f15ebbab1030f23a07f0ecf4e2e5b6fbc8116d6f0dd9d7a0e7d94`. The local build and runner logs are not part of this pull request.

### Observed fingerprint changes

The focused campaign selects the twenty configurations from the preceding fingerprint report. Every native simulation exits zero. Ten configurations explicitly verify their stored values. Ten configurations differ in all three selected ingredients, for thirty changed values. The wrapper reports `Ran 20 tests in 109.774s` and exits one because of those mismatches. No tracked fingerprint expectation changes.

The command runs from `tests/fingerprint` with debug binaries:

```sh
./fingerprinttest -d -t 8 -m '/examples/adhoc/qos/.*-c (MacQos|Fragmentation|MsduAggregation)( |$)|/examples/wireless/mactest/.*-c MacEdca -r 0|/examples/wireless/nic/.*-c Ieee80211MacV2X( |$)|/examples/wireless/qos/.*-c MacQos|/showcases/wireless/aggregation/.*-c (NoAggregation|Aggregation|VoicePriorityAggregation)( |$)|/showcases/wireless/blockack/.*-c (NoFragmentation|Fragmentation|MixedTraffic)( |$)|/showcases/wireless/fragmentation/.*-c HCFfrag|/showcases/wireless/qos/.*-c Qos( |$)|/showcases/wireless/txop/.*-c General( |$)' -f 'tplx' -f '~tNl' -f '~tND' examples.csv showcases.csv
```

| Changed configuration | Calculated `tplx` | Calculated `~tNl` | Calculated `~tND` |
| --- | --- | --- | --- |
| examples/adhoc/qos: MacQos, run 0 | `898b-aa3c` | `fbaf-8b67` | `3b12-ca4e` |
| examples/adhoc/qos: Fragmentation | `2f2d-73b1` | `cc9e-d1a6` | `e740-f7d4` |
| examples/adhoc/qos: MsduAggregation | `2c80-6844` | `4e9e-cf86` | `ea3d-e802` |
| examples/wireless/mactest: MacEdca, run 0 | `5635-3aae` | `c574-69b7` | `bb83-14c4` |
| examples/wireless/qos: MacQos, run 0 | `7ae8-d762` | `7a17-6a1d` | `93a3-323a` |
| examples/wireless/qos: MacQosWithoutAggregation, run 0 | `8241-1905` | `60a4-5565` | `a06d-81c9` |
| examples/wireless/qos: MacQosWithRtsCts, run 0 | `de32-56b9` | `00f9-9ce3` | `e305-c88a` |
| showcases/wireless/aggregation: VoicePriorityAggregation, run 0 | `1ad6-ebce` | `cc40-e9c2` | `5421-bc6d` |
| showcases/wireless/fragmentation: HCFfrag, run 0 | `c99f-354d` | `8e10-a3f0` | `ac4b-3ba1` |
| showcases/wireless/qos: Qos, run 0 | `5a33-2cf4` | `0683-d978` | `f448-e820` |

The unchanged configurations are adhoc `MacQos`, run 1; V2X; example `MacQosWithBlockAck`; and showcase `NoAggregation` and `Aggregation`. The three Block Ack showcase cases, `HCFfragblockack`, and TXOP `General` also verify their stored values. These controls retain twenty-nine selected values. In particular, the zero-limit `Aggregation` field change remains independent of the revised admission gate.

Native configuration options, run selections, and stop times follow the CSV inputs. The runner's generated `.UPDATED` files are candidate output only. Baseline acceptance requires a separate review and explicit approval of the exact changes.

### Scope and evidence limits

The prior approval of twenty baseline rows applies to the preceding source, not this admission revision. The original tracked CSV files retain their initial SHA-256 values. The user's `AGENTS.md` edit also retains its initial hash. No Git history changes or publication occur.

The refreshed standards corpus confirms the DATA/ACK duration definition in normative Clause 10.23.4.2.3. Its canonical node is `ieee80211-2024:clause:10.23.4.2.3`, physical PDF pages 2028–2029, source locator `ieee80211-2024@8440046:8444111`. The timer sample remains the declared inference from Clause 10.23.2.8. Corpus lint retains unrelated extraction findings. No PDF inspection is necessary for the retrieved text.

This verification covers the strict TXNAV boundary and its production effects in debug mode. It does not establish a throughput gain, general TXOP compliance, or cross-platform determinism. Release compilation, additional seeds, the broader wireless campaign, and baseline approval remain outside this local execution.

## Approved strict TXNAV baselines

The user approved thirty replacement values across the ten configurations in the preceding table on 2026-10-08. The approval applies to `tplx`, `~tNl`, and `~tND` only. Seven rows belong to `tests/fingerprint/examples.csv`. Three rows belong to `tests/fingerprint/showcases.csv`. Graphical fingerprints, other rows, stop times, arguments, and tags retain their preceding bytes.

The source cause is the strict comparison of complete exchange duration against TXNAV before the initial SIFS. The separate positive TXOP comparison still includes that SIFS. For example, adhoc `Fragmentation` admits a 238 µs exchange with about 246.67 µs TXNAV when the positive TXOP contains 248 µs. The former extra containment condition refused it. The preceding production fixture and native capture establish this change in continuation behavior.

GitHub [fingerprint CI run 37756147450](https://github.com/inet-framework/inet/actions/runs/37756147450) tests source head `24e32249a016b842a3cba1967892066df9f91130` before baseline acceptance. Each mode selects 1,752 cases across four shards. Debug and release report the same ten mismatches. All thirty calculated values exactly match the local strict TXNAV record. These are the only unexpected failures in each mode. The 5 Gbps half-duplex Ethernet error remains an expected result.

The approved patch contains the exact values in the preceding table. It applies cleanly before the update. The admission commit owns the baseline changes because its strict comparison causes the changed trajectories.

After the update, the focused campaign passes all twenty configurations in debug and release modes. Each mode verifies all 59 selected values. Both wrappers and all forty native simulations exit zero. The selector, CSV inputs, and ingredient filters match the preceding command. Release mode omits `-d`; debug mode includes `-d`.

Both fresh builds pass with `make -j12 MODE=debug` and `make -j12 MODE=release`. Four unit fixtures and twelve module fixtures pass with the preceding explicit filters. The module fixtures cover 106 simulation runs. The debug library retains SHA-256 `43d46b890a3f15ebbab1030f23a07f0ecf4e2e5b6fbc8116d6f0dd9d7a0e7d94`. The release library has SHA-256 `ff3fe197a95c774e8a952f167fda46cc4712a99008d370cd338d597a306de0cd`.

The final topic differs from pinned master `b4675dff17d46620936d67853c10a803d807bec8` in 55 selected values across twenty configurations. This count includes the preceding causal source changes and the strict admission baselines. The admission source, production fixtures, and user documentation retain their preceding bytes. Additional seeds and per-commit behavioral verification remain unrun. Baseline acceptance does not establish unchanged throughput or general protocol correctness.

On 2026-10-08, [fingerprint CI](https://github.com/inet-framework/inet/actions/runs/37766471300), [Linux builds](https://github.com/inet-framework/inet/actions/runs/37766471069), and [project enforcement](https://github.com/inet-framework/inet/actions/runs/37766471170) pass for source head `1882e1ff430b5f87060cf3d3297eb1a81dbc0401`. The later report edits change no production source or fingerprint baseline. Production source bytes match source head `24e32249a016b842a3cba1967892066df9f91130`, which the earlier CI campaign tested before baseline approval.

## Refused DATA mode tag

HCF uses the selected DATA mode tag to estimate ACK duration. On admission refusal, HCF restores the retained DATA frame's prior mode tag. If the frame had no tag, HCF removes the new tag. A fresh grant still selects the DATA mode again. This applies to direct DATA refusal and refusal before RTS.

The production `Ieee80211HcfSingleProtection_1` fixture checks the retained tag immediately after refusal. Scenario 1 covers a frame without a prior tag. Other refusal cases cover a prior tag. The fixture also checks later transmission and the existing response estimates. A fresh release build passes with `make -j12 MODE=release`. The release module command below passes all 26 simulation runs:

```sh
./bin/inet_run_module_tests -m release --no-concurrent -f 'Ieee80211HcfSingleProtection_1\.test$'
```

The twenty selected release fingerprint configurations pass with the selector and ingredients in the [strict TXNAV command](#observed-fingerprint-changes), without `-d`. The command runs from `tests/fingerprint` against the revised local source. It selects `examples.csv` and `showcases.csv` and verifies the recorded values.

All four release shards in [fingerprint CI run 37775296696](https://github.com/inet-framework/inet/actions/runs/37775296696) pass at PR head `bc276b7de20cb68100430a42f4c559688bf6f284`. That CI run precedes the local mode-tag correction. The focused release run above checks the correction; full CI on this revised source remains unrun.
