# Single protection fingerprint analysis

> **Kind:** report · **Status:** snapshot 2026-10-08 · **Seal:** none · **Owns:** — · **Stands on:** [approved plan](ieee80211-txop-single-protection.md), [implementation verification](ieee80211-txop-single-protection-verification.md)

This snapshot precedes the [strict TXNAV admission revision](ieee80211-txop-single-protection-verification.md#strict-txnav-admission-revision). Its causes, values, and approved baseline update describe the earlier source. The revision's results identify subsequent trajectory changes separately.

The [approved strict TXNAV baselines](ieee80211-txop-single-protection-verification.md#approved-strict-txnav-baselines) replace thirty subsequent values across ten configurations. Those values match both debug and release CI. This snapshot's final values remain evidence for its preceding source.

The tracked values and linked CI runs provide reviewable evidence. The investigation's local logs are not part of this pull request.

## Result and approved scope

This section records source `5c040ce387` before the strict TXNAV admission revision. The [approved strict TXNAV baselines](ieee80211-txop-single-protection-verification.md#approved-strict-txnav-baselines) supersede its final-value and approval scope.

Four source changes account for all twenty supplied CI failures. The prerequisite verifies all twenty original stored fingerprints. The final version reproduces every supplied CI value. The user approved the exact twenty-row update on 2026-10-08 after review of its causes and values. The applied update changes 57 values in `tests/fingerprint/examples.csv` and `tests/fingerprint/showcases.csv`. The update records the approved expectations for the current source.

The changes affect both frame fields and frame exchange times. A baseline update accepts those behavior changes. It does not establish unchanged throughput or general protocol correctness. The earlier verification report owns the direct fixture results and their limits.

QoS means quality of service. It is traffic treatment that accounts for requirements such as priority, delay, and throughput. EDCA means enhanced distributed channel access. It gives access categories different channel access parameters.

HCF means hybrid coordination function. It includes EDCA and controlled channel access for QoS traffic. A TXOP is a transmission opportunity. It is a time interval in which a QoS station has the right to start frame exchange sequences. TXNAV is the holder's transmitted reservation timer, shared by its access categories. Duration/ID is the frame's advertised reservation field.

MAC means medium access control. This protocol layer controls channel access and exchanges frames. The PHY is the physical layer, which sends and receives wireless signals through the medium. A PPDU is a PHY protocol data unit. The PHY transmits that unit with its preamble, PHY header, and data. These terms follow the [project vocabulary](../../CONTEXT.md).

## Source changes and observations

The checkpoint identifiers below name the measured source revisions. The current series records each intermediate baseline with its source change. The final source and final expectations retain their measured values.

The table identifies the four commits that change the selected fingerprint ingredients. The groups overlap because one configuration can reach more than one change. Counts refer to differences from the preceding checkpoint, rather than distinct final values.

| Group | Source commit | Native cases | Rows that differ | Values that differ |
| --- | --- | ---: | ---: | ---: |
| Prerequisite | `1b31d3dfcd` | 20 | 0 | 0 |
| A1: publish the selected ACK policy | `521ad120e8` | 7 | 7 | 20 |
| A3: exclude group frames from the current ACK estimate | `348371cfcc` | 6 | 6 | 18 |
| B1: publish successful holder reservations through TXNAV | `1015799dad` | 20 | 20 | 42 |
| C1: admit complete single protection continuations | `a224e4677b` | 13 | 11 | 33 |

### A1: use the selected ACK policy in the field calculation

ACK is the acknowledgment control frame for a frame that requires this response. SIFS is the short interframe space between specified responses and frames within an exchange. HCF publishes the selected ACK policy before it executes the exchange. The protection mechanism therefore uses the selected policy when it estimates Duration/ID. This change removes an unnecessary immediate ACK estimate from current frames with Block Ack policy.

The ACK policy selects the acknowledgment mechanism for a frame. For the covered Block Ack path, the recipient does not send an immediate ACK for each DATA frame.

For example, showcase `NoFragmentation` first differs at a QoS DATA frame with sequence number 1. Both versions transmit Block Ack policy. The original Duration/ID is 384 µs; A1 transmits 340 µs at the same capture time. The 44 µs difference is the unnecessary ACK airtime plus SIFS. RTS means request to send; this control frame requests protection for a subsequent exchange. The [header fixture](../../tests/module/Ieee80211HcfHeader_1.test) also checks DATA and RTS field calculations through production HCF.

The same comparison establishes field reductions in the other six affected cases. Showcase Block Ack `Fragmentation` changes 160 µs to 116 µs. `MixedTraffic` changes 716 µs to 672 µs. `HCFfragblockack` changes 212 µs to 168 µs. The TXOP showcase changes 1284 µs to 1240 µs. The per-case capture records contain the frame identities and observation times.

### A3: apply the group equation

The protection mechanism excludes group frames from the current-frame ACK branch. A final group frame therefore advertises zero when no successor contributes to its field. The original classification adds an ACK estimate to group management and non-QoS DATA. The recipient does not send that ACK. The incorrect field creates unnecessary reservations.

For example, V2X first transmits a broadcast non-QoS DATA frame near 0.5489415 s. A3 changes its Duration/ID from 96 µs to zero. The five AP-based QoS cases change their first Beacon field from 44 µs to zero. The captures preserve each frame's other decoded header values at that first difference. The [group fixture](../../tests/module/Ieee80211SingleProtectionGroup_1.test) checks both frame classes with Block Ack support enabled and disabled.

These fields follow the single protection equations in IEEE Std 802.11-2024, Clause 9.2.5.2(a). A3 retains the legacy successor term. It does not add complete continuation admission to the Block Ack path.

### B1: separate successful TXNAV from received NAV

NAV is MAC state that records a reservation of the medium from protocol duration information. HCF stops the old holder updates to local NAV. After a successful holder transmission, HCF instead supplies the encoded field and actual PPDU end to Rx. Rx, the MAC receive component, replaces the shared TXNAV endpoint. It retains received NAV separately. HCF also excludes local contention while it owns an active sequence, including an initial ACK wait before successful TXNAV exists.

For example, `NoAggregation` originally executes a local NAV expiry event at 0.0004105 s. B1 removes that event. The short captures retain equal packet times and decoded headers. The native one-second runs also retain equal `~tNl` and `~tND` fingerprints. Only `tplx` changes. This case establishes a timer difference without a detected difference in the selected packet exchange ingredients.

Other cases also change packet exchange times. After A1, Block Ack `Fragmentation` observes fragment 13 at 0.009433617 s. B1 observes that fragment at 0.009328617 s. A successful shorter field now replaces the previous endpoint instead of retaining the old maximum local NAV. CTS means clear to send; this control frame provides protection, usually as a response to RTS. The [TXNAV fixture](../../tests/module/Ieee80211Txnav_1.test) checks replacement, failure, contention, and CTS isolation.

IEEE Std 802.11-2024, Clause 10.23.2.2, defines successful TXNAV replacement and its origin at PPDU transmission end. The received NAV and CTS conditions remain separate. Seven cases with Block Ack support enabled reach their final CI values at B1. V2X and `NoAggregation` also reach their final values at this checkpoint.

### C1: check the complete continuation and eligible successor

With single protection and Block Ack support disabled, HCF checks the complete expected cost before an additional exchange. The initial wait and exchange must fit TXNAV and the remaining TXOP limit. HCF refuses an exchange that exceeds either budget. The frame sequence handler, `FrameSequenceHandler`, removes the unsent step and ends the sequence normally. The unsent frame remains available for a later TXOP.

For example, adhoc `Fragmentation` completes its first DATA PPDU at 0.500194 s with Duration/ID equal to 292 µs. Its transmitted reservation ends at 0.500486 s. The ACK arrives at 0.500239334256 s, so about 246.665744 µs remain. The next exchange needs 248 µs: two 10 µs SIFS intervals, 194 µs DATA airtime, and 34 µs ACK airtime. HCF refuses that continuation. After reservation expiry and new contention, the next fragment's PPDU ends at 0.500830 s.

The captures and HCF log establish the refusal, channel release, reservation expiry, and subsequent transmission. The original version transmits that fragment's PPDU by 0.500443334 s. This difference explains the changed frame exchange times. It does not represent a retry or a drop of the unsent fragment.

C1 also excludes successors that cannot continue. For example, showcase `Aggregation` uses the best effort access category, whose default TXOP limit is zero. A queued successor cannot start an additional exchange in this scoped path. C1 changes the relevant DATA field from 152 µs to 44 µs. That field now estimates only the current required ACK and SIFS.

The [continuation fixture](../../tests/module/Ieee80211HcfSingleProtection_1.test) checks both budgets, selected modes, equality, forecasts, refusal, and subsequent progress. The plan records the interpretation of the check time under Clause 10.23.2.8. Block Ack support disables this admission path even before an agreement exists. C1's thirteen native cases include V2X and `NoAggregation` as unchanged controls. Eleven cases change all three selected values.

## Exact approved update

These values belong to source `5c040ce387`. The [strict TXNAV revision](ieee80211-txop-single-protection-verification.md#approved-strict-txnav-baselines) replaces thirty later values across ten configurations.

The table lists every affected configuration and its measured causes. Each numeric cell omits the ingredient suffix shown in the column header. `NoAggregation` retains its listed `~tNl` and `~tND` values. The TXOP row has no selected `~tND` expectation. The update preserves graphical fingerprints and every other CSV field.

| Case | Causes | Final `tplx` | Final `~tNl` | Final `~tND` |
| --- | --- | --- | --- | --- |
| examples/adhoc/qos: MacQos -r 0 | B1 / C1 | 2ada-70d2 | 9f56-c060 | 06a6-672b |
| examples/adhoc/qos: MacQos -r 1 | A1 / B1 | 017f-bfed | adbc-faea | 9e6e-f139 |
| examples/adhoc/qos: Fragmentation | B1 / C1 | 989b-3e77 | a665-c17a | 74ed-3de2 |
| examples/adhoc/qos: MsduAggregation | B1 / C1 | 45e5-01fb | b763-c81d | e382-5cd5 |
| examples/wireless/mactest: MacEdca -r 0 | B1 / C1 | ad78-b38b | f9a8-c2c8 | 2815-a7fb |
| examples/wireless/nic: Ieee80211MacV2X | A3 / B1 | e13c-8d37 | cfe1-e3ca | 24f3-e65f |
| examples/wireless/qos: MacQos -r 0 | A3 / B1 / C1 | 24d0-e755 | 44f5-cd51 | 614f-396c |
| examples/wireless/qos: MacQosWithoutAggregation -r 0 | A3 / B1 / C1 | 50bf-6ad3 | 5c4a-8ebf | 2d77-1fcb |
| examples/wireless/qos: MacQosWithRtsCts -r 0 | A3 / B1 / C1 | f220-d33d | c711-534c | 9c5b-c6f8 |
| examples/wireless/qos: MacQosWithBlockAck -r 0 | A1 / A3 / B1 | 27f3-3799 | bf7e-a5fd | 2e31-6cf2 |
| showcases/wireless/aggregation: NoAggregation -r 0 | B1 | 21a4-c882 | ecf2-6e33 | 5d9e-8fdf |
| showcases/wireless/aggregation: Aggregation -r 0 | B1 / C1 | 0e94-675b | 4545-c0de | a778-2f83 |
| showcases/wireless/aggregation: VoicePriorityAggregation -r 0 | B1 / C1 | ee55-0a9d | da1d-7eca | 24d2-f35a |
| showcases/wireless/blockack: NoFragmentation -r 0 | A1 / B1 | 36ed-963c | 0068-9ae9 | 3c53-7c24 |
| showcases/wireless/blockack: Fragmentation -r 0 | A1 / B1 | 2af7-c4f6 | bdc7-cfd0 | 3135-dd51 |
| showcases/wireless/blockack: MixedTraffic -r 0 | A1 / B1 | e88a-aaf5 | 5647-2cd1 | 63db-304a |
| showcases/wireless/fragmentation: HCFfrag -r 0 | B1 / C1 | 5770-024c | 706c-7174 | 8886-8c8d |
| showcases/wireless/fragmentation: HCFfragblockack -r 0 | A1 / B1 | e040-8973 | 8610-abcd | 68c2-19f0 |
| showcases/wireless/qos: Qos -r 0 | A3 / B1 / C1 | b300-b302 | f942-332c | a8a4-3d2e |
| showcases/wireless/txop: General -r 0 | A1 / B1 | 4ffd-4fbf | 3a89-2021 | not selected |

`git apply --check` accepted the proposed baseline patch before application. Its companion JSON recorded every old value, new value, file, and row. A byte comparison confirmed changes to twenty approved rows and 57 approved values. Four additional patches assigned intermediate values to A1, A3, B1, and C1. Baseline approval did not authorize a branch history rewrite or publication.

## Verification after the approved update

The official fingerprint runner selects exactly the twenty configurations in the table. All twenty pass in debug mode after the update. The runner exits zero and reports `Ran 20 tests in 108.684s`. It verifies 59 selected values, including the two unchanged `NoAggregation` values. The production source remains unchanged. `git diff --check` passes.

The command uses `./fingerprinttest -d -t 8`, the anchored selector in the [strict TXNAV command](ieee80211-txop-single-protection-verification.md#observed-fingerprint-changes), and ingredient filters `tplx`, `~tNl`, and `~tND`. It runs from `tests/fingerprint` with `examples.csv` and `showcases.csv`. The local runner log is not part of this pull request.

The debug library SHA-256 before and after execution is `b7fad1ae3f14ca74372c3f4ec76ddd46e5a416d03a250d29eb320d8050112aa2`. The metadata also records both CSV checksums and the zero exit code. A second byte comparison confirms the exact approved changes after execution. This result covers these twenty debug configurations; it does not extend the coverage to release mode or additional seeds.

## Reproduction and evidence limits

The recorded source is `5c040ce387`. Production source remains unchanged; only the approved CSV rows change after baseline approval. The final-version comparison executes the twenty supplied cases with debug binaries and reproduces the supplied values. Its library retains the original tested code; the saved copy removes debug symbols only.

The verified checkpoint campaign executes 66 native simulations. All 66 exit zero. The twenty prerequisite checks explicitly verify their stored expectations. Changed checkpoints report mismatches against those unchanged expectations. The final-version run supplies another twenty simulations, so 86 native simulations support this analysis. The causal matrix combines measured values with explicit source scope; it marks configurations without a native intermediate run.

FCS means frame check sequence. This field lets the recipient check a frame for transmission errors. Native runs use the stored configuration, run selection, stop time, checksum settings, and FCS settings. The adhoc `MacQos` cases retain their distinct run 0 and run 1 selections.

The campaign checks each fixed library's checksum after execution. The full original wireless campaign remains separate evidence with 434 passes and twenty mismatches among 454 selected cases.

The checkpoint build command is `make -C src -j12 MODE=debug 'SHLIB_POSTPROCESS=strip --strip-debug'` in the separate build directory. The final link succeeds, but its first symbol removal exceeds temporary storage. A separate `strip --strip-debug -o` operation completes that step after log compression. The corrected final build exits zero before its native runs. Each retained library has a verified checksum. This investigation uses logs and captures rather than a debugger.

The checkpoint commands run through a separate copy of `bin/inet` for each fixed library. They use `--debug -u Cmdenv` and `--fingerprintcalculator-class=inet::FingerprintCalculator`. They retain native INI files and the active checkout's NED paths. The selected ingredients are `tplx`, `~tNl`, and `~tND`, except for the TXOP row's two ingredients. These ingredients exclude graphical state, message IDs, and result scalars.

Intermediate A2 moves a simulation signal within the same event without a new frame or timer. B0 allocates an unscheduled timer; HCF does not publish to it until B1. C0 adds normal refusal support, but its false result becomes reachable through C1. These helper changes do not add selected fingerprint ingredients in the covered paths. Their earlier direct tests remain in the implementation verification report. This investigation does not claim a complete native campaign at every helper commit.

Short diagnostic runs add the same logs and interface captures to each compared version. The captures observe MAC frames at both stations and at the AP where applicable. Native and diagnostic output remain separate. The TXOP capture pair uses computed FCS and checksums because capture export cannot serialize its native declared FCS. That override limits the capture comparison; the native fingerprint comparison retains the stored settings.

The diagnostic summaries record the fixed library checksum and complete commands. They record each compared frame, decoded field difference, and first changed logged event. Compressed logs retain their original bytes after decompression; checksum records establish that equality. Interrupted shared-output attempts and incomplete capture attempts do not support the conclusions. Only the separate fixed-library campaign and complete diagnostic summaries supply the accepted intermediate evidence. These local summaries are not part of this pull request.

The standards corpus reports fresh IEEE Std 802.11-2024 content. The retrieved nodes are `ieee80211-2024:clause:9.2.5.2` and `ieee80211-2024:clause:10.23.2.2`. Their physical PDF pages are 710–712 and 1999–2001. The source locators are `ieee80211-2024@3283316:3295696` and `ieee80211-2024@8315619:8323576`. Corpus lint retains unrelated extraction findings. No PDF inspection is necessary for these retrieved clauses.

The CI comparison uses the user-supplied values. It does not establish the CI commit identity or build mode. This investigation does not repeat release fingerprints, the 434 unchanged wireless cases, or additional seeds. It does not repeat the earlier unit and module fixtures. The user approved the exact scope and reason before tracked expectations changed, as the [baseline procedure](../../doc/project/guide/change-a-baseline.md) requires.

The Ethernet error is expected. The [stored Ethernet row](../../tests/fingerprint/ethernet.csv) requires rejection of 5 Gbps half-duplex operation. That case requires no baseline change and does not enter this wireless campaign.
