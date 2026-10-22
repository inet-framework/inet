# Basic Block Ack transfer completion: current verification

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [implementation status](ieee80211-compressed-blockack-replacement.md), [correction plan](ieee80211-compressed-blockack-s4-corrections.md)

The final production source passes debug and release compilation, seven module files, three unit files, seven protocol files, and seven fingerprint cases. The verified `src` tree is `20aade533739a1877a87391909da7dd2a91c79c7`. The correction preserves that production tree and the accepted final fingerprint rows. It adds direct protocol evidence and separates the commit responsibilities.

## Behavior and ownership

HCF selects due Basic BAR work before DATA extraction and can contend without queued DATA. The ACK policy separates outstanding DATA by peer and TID within the accepted window. Below the threshold, it defers BAR only for the next eligible DATA in that group and window.

For example, a window of one and a BAR threshold of five leave one DATA outstanding. The next sequence lies outside the window. HCF sends BAR. Positive feedback resolves that DATA and permits window progress. With no queued or numbered DATA at a safe exchange boundary, HCF queues completion DELBA.

The originator agreement owns cyclic distance, bitmap membership, and window advancement. HCF supplies unresolved identities. BAR failure updates contention without a DATA retry charge. DATA retransmission sets Retry after failed feedback. Lifetime expiry can resolve outstanding positions and trigger completion teardown.

Basic single protection reserves only prepared successors. Unextracted DATA starts another TXOP. Duration prediction borrows candidates without extraction. The [migration section](../../doc/src/migration-guide/development/ieee80211-basic-block-ack-extension-points.rst) defines the custom data-service and policy contracts.

The [scope table](ieee80211-compressed-blockack-replacement.md#previous-prs-and-s4-scope) separates inherited lifecycle and recipient behavior from S4. These results describe the integrated source. They do not establish separate results for prerequisite PR heads.

## Direct protocol evidence

The [Retry comparison](../../doc/project/evidence/model/wifi/tests/blockack-data-retry-comparison.md) records lost-feedback and negative-bitmap cases. Both fail at the Retry predicate on the pinned base. Both pass with only HCF's Retry branch. The tests retain the captured sequence and TID across retransmission.

The [completion comparison](../../doc/project/evidence/model/wifi/tests/blockack-transfer-completion-comparison.md) records queue exhaustion and a one-position window. Both fail before transfer completion. Both pass through BAR, matched feedback, completion DELBA, and ACK on the final source. Neither fixture supplies an accepted agreement.

The wider protocol selection also checks duplicate ADDBA, dialog tokens, BAR without an agreement, inactivity, and same-AP reassociation. Two inherited fixtures need traffic controls because S4 completes agreements earlier. Their guides describe those controls. All seven files pass in `/tmp/pr1344-correction-final-protocol.log`.

## Executed final checks

| Check | Result | Scope |
| --- | --- | --- |
| Debug compilation | PASS | Final production source. |
| Release compilation | PASS | Final production source; no release runtime campaign. |
| Module tests | Seven files PASS | Completion cost, selection, scheduling, deferral, retirement, headers, and protection. |
| Unit tests | Three files PASS | Scheduling, shared window operations, and multiple protection. |
| Protocol tests | Seven files PASS | Thirteen seed-0 simulation runs; four runs exercise the new fixtures. |
| Fingerprints | Seven cases PASS | Existing non-graphical values; 274.663 seconds. |

The final module log is `/tmp/pr1344-correction-final-modules.log`. The unit log is `/tmp/pr1344-opp-repl-final-contract-v2.log`. The fingerprint log is `/tmp/pr1344-correction-final-fingerprint.log`. Local `/tmp` artifacts can be absent in another checkout.

The author ran these commands from the checkout root, before the [rebase onto the fingerprint store](#rebase-onto-the-fingerprint-store). Master has since replaced the `inet_run_*` commands, `fingerprinttest`, and the CSV files.

```sh
make MODE=debug -j12
make MODE=release -j12
inet_run_module_tests -m debug -f '(Ieee80211BlockAckCompletionCost_1|Ieee80211BlockAckScheduling_1|Ieee80211BlockAckDataSelection_1|Ieee80211BlockAckDeferral_1|Ieee80211BlockAckFragmentRetirement_1|Ieee80211HcfHeader_1|Ieee80211HcfMultipleProtection_2)\.test$'
inet_run_unit_tests -m debug -f '(Ieee80211OriginatorBlockAckWindow_1|Ieee80211BlockAckScheduling_1|Ieee80211MultipleProtection_1)\.test$'
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f 'WifiBlockAck(DataRetry|TransferCompletion|DuplicateAddba|DialogToken|BarWithoutAgreement|Inactivity|SameApReassociation)\.test$'
```

The fingerprint command runs from `tests/fingerprint`:

```sh
./fingerprinttest -d -F tyf -t 3 -m 'showcases/wireless/blockack/|showcases/wireless/fragmentation/.*-c HCFfragblockack |showcases/wireless/txop/|examples/adhoc/qos/.*-c MacQos -r 1|examples/wireless/qos/.*-c MacQosWithBlockAck ' showcases.csv examples.csv
```

The CSV files supply each run, seed, limit, and computed-FCS setting. Graphical `tyf` values remain excluded.

## Commit verification contract

The [six-commit plan](ieee80211-compressed-blockack-s4-corrections.md) requires an incremental debug build at each output commit. The retained control worktree reuses compatible objects. The `opp_repl` selection uses the checkout's explicit `inet_project` instance.

| Commit | Direct selection |
| --- | --- |
| P0: correction contract | Existing multiple-protection unit fixture. |
| P1: preview | DATA-selection module fixture. |
| P2: Retry | Retry protocol fixture; seven fingerprints with five approved intermediate values. |
| P3: shared arithmetic | New window unit fixture and unchanged Retry protocol fixture. |
| P4: completion | Both new protocol fixtures, three unit files, and seven module files. |
| P5: current documents | Both new protocol fixtures and the final related selection. |

The execution log under `ai-logs/executions/2026-10-22_pr1344-corrections.md` records exact safe points and results. That directory is local workflow state. It does not form portable test evidence by itself.

## Fingerprint causes and acceptance

The final source retains the seven accepted non-graphical baseline rows. The original S4 execution record states that the user approved these values on 2026-10-15. Its immutable source is commit `18bba0a41ab8c00aefaeca7de233fd68cbc4ee7f`, before the current documentation replacement. This correction restores that acceptance record. It grants no approval for a new intermediate value.

| Configuration and run | Final fingerprints | Cause |
| --- | --- | --- |
| Block Ack `NoFragmentation`, run 0 | `9161-625d/tplx;6a02-09aa/~tNl;cbc0-3b0a/~tND` | Final Normal Ack DATA reserves its immediate ACK. Setup and BAR order also change. |
| Block Ack `Fragmentation`, run 0 | `36f4-5c22/tplx;66b0-94cd/~tNl;45f3-af4f/~tND` | Final Block Ack DATA reserves zero. BAR obtains a separate TXOP. |
| Block Ack `MixedTraffic`, run 0 | `351b-d4ba/tplx;5c1f-69a8/~tNl;6182-6da3/~tND` | The current reservation excludes an unextracted large successor. Agreement setup can precede that DATA. |
| Fragmentation `HCFfragblockack`, run 0 | `2597-1e2e/tplx;d269-c6ae/~tNl;2f5c-4cca/~tND` | Retransmitted DATA sets Retry after failed feedback. The final burst boundary also changes. |
| TXOP `General`, run 0 | `ab91-87f3/tplx;f30c-6aa4/~tNl` | HCF excludes an unextracted successor from the reservation. The frame sequence ends at that same boundary. |
| Ad hoc `MacQos`, run 1 | `4fd2-bb56/tplx;ecaf-89f7/~tNl;9398-0fae/~tND` | Final Normal Ack DATA reserves SIFS plus ACK. Further extraction requires another TXOP. |
| Wireless `MacQosWithBlockAck`, run 0 | `6edb-0326/tplx;2a8a-50ee/~tNl;4301-882d/~tND` | ADDBA reserves its immediate ACK without unrelated queued DATA. Later extraction can change aggregation. |

The causal behavior commit must name these rows and their causes. A matching fingerprint checks the recorded trajectory. Direct protocol and module assertions establish the specific correctness claims.

## Intermediate fingerprints

The user approved five intermediate `~tND` values on 2026-10-22. They belong to the isolated Retry commit. All seven event and packet-length fingerprints remain equal to their parent values. TXOP General and ad hoc MacQos run 1 retain every selected value.

| Configuration | Parent `~tND` | Retry-only `~tND` |
| --- | --- | --- |
| Block Ack NoFragmentation | `2cd4-4795` | `0f38-6042` |
| Block Ack Fragmentation | `feff-7e88` | `ddec-9e3d` |
| Block Ack MixedTraffic | `9724-d1ce` | `2839-8753` |
| Fragmentation HCFfragblockack | `c474-7174` | `bbe5-0771` |
| Wireless MacQosWithBlockAck | `538e-67df` | `1914-cd4d` |

Each row uses run 0. Retransmitted Block Ack DATA changes its Retry header and computed FCS. The later completion commit replaces these intermediate values with the accepted final rows above. Before the rebase, the final CSV files were byte-identical to the original PR head.

## Rebase onto the fingerprint store

On 2026-10-27, the series moved onto master `7c48c1c35e`. That master replaced `fingerprinttest`, the CSV files, and the `inet_run_*` commands with `tests/fingerprint/store.json` and the opp_repl commands. The 802.11 source of each commit did not change. The CSV changes of P2 and P4 became store updates in the same commits.

The store has a run-0 entry of `tplx`, `~tNl`, and `~tND` for each concrete configuration. Thus more runs change than the CSV rows above show. Block Ack `General` has the settings of `NoFragmentation`. Three configurations of `showcases/wireless/fragmentation/parameterstudy.ini` use HCF fragmentation with Block Ack, as `HCFfragblockack` does. TXOP `General` also has a `~tND` entry. Ad hoc `MacQos` run 0 keeps its values.

| Run 0 of | P2 `~tND`: parent, Retry-only | P4 fingerprints |
| --- | --- | --- |
| Block Ack `General` | `2cd4-4795`, `0f38-6042` | `9161-625d/tplx;6a02-09aa/~tNl;cbc0-3b0a/~tND` |
| Fragmentation `PacketSizeHCFfragblockack` | `3ab3-0718`, `8a84-f295` | `81d3-d3a1/tplx;0ac9-0371/~tNl;0d96-a269/~tND` |
| Fragmentation `OnlyPacketSizeHCFfragblockack` | `fff0-d15d`, `b5a5-f08f` | `c0fd-d674/tplx;e816-9097/~tNl;1641-e12d/~tND` |
| Fragmentation `ThresholdHCFfragblockack` | `ee7e-0789`, `65f9-f592` | `13cf-6755/tplx;70ef-e3ab/~tNl;44bd-1fd5/~tND` |
| TXOP `General` | `25db-d49a`, unchanged | `ab91-87f3/tplx;f30c-6aa4/~tNl;0b46-385f/~tND` |

The runs of the CSV rows get the values of the two tables above.

A podman copy of the GitHub fingerprint job made these values: Ubuntu 24.04, clang 16, OMNeT++ `omnetpp-6.x` at `21d208ddca`, and opp_repl `github_ci` at `dddd45a`. It ran the full release suite of 1840 runs at master and at each code commit. Master, P1, and P3 give 1838 PASS and 2 expected ERROR. P2 changes the `~tND` value of 9 runs. P4 changes all three values of 11 runs. With the updated store, the final source gives 1838 PASS and 2 expected ERROR again. The debug runs of the five changed folders give 34 PASS with the same store.

The debug suites in the same container compare the final source with master:

| Suite | Master | Final source |
| --- | --- | --- |
| Unit tests | 128 PASS | 130 PASS: the two new files pass. |
| Module tests | 752 PASS, 7 failures | 754 PASS: the two new files pass. The same 7 failures remain. |
| Wifi protocol tests | 58 PASS, 30 expected FAIL | 60 PASS: the two new files pass. The same 30 expected FAILs remain. |

The 7 module failures come from the container. The image does not have `ping` and `killall`, which `ExternalProcess_1` to `ExternalProcess_4` need, or `gawk`, which three other tests need.

`WifiLateBlockAck.test` failed at first after P4. After a BAR exchange of sta1 at about 1.006 s, sta1 and sta2 got the channel in the same slot. The radio medium stops a run when two transmissions have the same start time. P4 sets this check to `"warning"`, as `WifiBlockAckSameApReassociation.test` does, and gives the new times in the test description. The assertions of the test did not change.

These commands run the checks from the checkout root, after the OMNeT++ and INET setenv. The project file `inet.opp` gives the debug mode of the unit, module, and protocol tests.

```sh
make MODE=debug -j$(nproc)
make MODE=release -j$(nproc)
opp_run_unit_tests -p inet
opp_run_module_tests -p inet
opp_run_protocol_tests -p inet -w '^tests/protocol/wifi'
opp_run_fingerprint_tests -p inet -m release --no-build
opp_update_fingerprint_test_results -p inet -m release --no-build -f 'showcases/wireless/blockack|showcases/wireless/fragmentation|showcases/wireless/txop|examples/wireless/qos|examples/adhoc/qos'
```

## Limits

Basic remains the operational profile and default. HCF rejects Compressed selection during initialization. General Basic DATA admission, complete NAV conformance, A-MPDU, delayed Block Ack, and Multi-TID Block Ack remain outside this claim.

A fixed seed establishes these deterministic exchange paths. It does not establish probabilistic robustness or performance. Build and mechanical gate passes do not establish protocol correctness. Local verification supplies no CI result for a new commit. The correction self-check does not replace an independent review.
