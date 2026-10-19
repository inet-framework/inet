# S4 execution record

S4 completes Basic BAR work independently of DATA extraction. The initial source parent is `b29151fc8b`. The user authorized S4 implementation on 15 October 2026. Existing local edits to `AGENTS.md` and the prerequisite plans remain outside this change.

## Implementation contract

HCF previews the next eligible frame before a frame sequence extracts DATA. The originator ACK policy selects outstanding frames by peer and TID. The policy permits threshold deferral only for a candidate within that agreement's transmit window. The agreement owns the window start and size. Resolved DATA permits window progress; a received response number alone does not.

HCF treats BAR work as contention work for its access category. Frame sequences permit BAR without a DATA pointer. Internal collision retains BAR work without DATA retry feedback. At an exchange boundary, HCF queues DELBA when an accepted agreement has no queued DATA, in-progress DATA, or active BAR exchange. Expiration uses the same teardown path after unresolved DATA disappears.

The affected components are HCF, the QoS ACK policy, the ACK handler, the originator agreement, the in-progress queue, and HCF frame sequences. The optional QoS data-service contract exposes preview without a concrete dependency from the shared queue. Queue preview borrows a Packet pointer. Extraction retains the existing ownership transfer and sequence assignment. DCF extraction remains unchanged. S5 owns Compressed response validation and integration.

The checks use focused module fixtures through HCF, frame sequences, Tx, and production response callbacks. They cover small windows, short bursts, peer/TID isolation, failed BAR recovery, collision, final expiration, and transfer completion. Timing checks compare Basic BAR exchanges with the pinned parent under single and multiple protection. A new unlawful continuation requires an admission correction before completion.

The initial contract review checked source, ownership, protection, and callers before the first source edit. The affected source paths have no seal. S1 recipient progress and S3b teardown exist in the pinned parent.

## Standards evidence

The local corpus supplies IEEE Std 802.11-2024 normative Clauses 10.25.4, 10.25.6.7.1, and 10.23.2.9. Their canonical identifiers are `ieee80211-2024:clause:<clause>`. Their physical PDF pages are 2056, 2068–2070, and 2012–2014. Corpus status reports fresh input. The corpus linter result is `/tmp/s4-standard-lint.json`. Direct PDF inspection was unnecessary.

## Verification

Failed BAR recovery also requires the Retry bit on the next DATA attempt. Source inspection found that the old BAR failure path only changed ACK status. HCF now derives that bit from the failed Block Ack status before retransmission. IEEE Std 802.11-2024, Clause 9.2.4.1.6, physical PDF page 665, supplies this requirement for individual DATA.

### Direct behavior checks

The fresh debug build passes. The final focused selection passes 31 module fixtures and six unit fixtures. The module log is `/tmp/s4-module-final-limit.log`. The unit log is `/tmp/s4-unit-final-limit.log`. The new BAR fixture executes 30 scenarios across single and multiple protection. These results establish the recorded Basic paths. They do not establish Compressed support.

The unit fixture checks peer/TID selection, sequence wrap, window boundaries, response identity, failed-response release, and exact lifetime eligibility. The production fixture exercises HCF, EDCA, frame sequences, Tx, and response callbacks. Its cases cover thresholds below five, accepted windows one and four, unrelated peers, negative bitmaps, timeout, and final expiration. The collision cases require BAR contention without a DATA retry entry. The nonzero contention-window case requires contention-window growth after collision. The short-limit case permits the initial Control-frame exception.

BAR failure updates the access category's short retry counter and contention window. The one-failure limit case requires growth on the first failure. The unit fixture requires reset on the next failure. A successful Block Ack response resets both values. These checks require no DATA retry entry.

The final-expiration case creates an incomplete fragment unit before two complete units. The production recipient retains those complete units behind the gap. Final expiration triggers DELBA. The recipient delivers both complete units before it deletes the agreement. This connects A5 to the existing recipient teardown path.

Three existing fixtures require changed expectations because a short burst now resolves its final BAR and DELBA. The deferral fixture checks both remaining original DATA frames. Both retain their lifetime obligation after unsent disposal. The fragment-retirement fixture answers the unrelated peer's final BAR before its successor DATA. The multiple-protection fixture separates the DELBA management ACK from DATA ACKs.

### Basic timing comparison

The comparison parent is `b29151fc8b8953564dbbf61b48ece12658a1475c`. A detached checkout at `/tmp/inet-s4-parent` supplies its separately built debug library. The timing observer records actual transmit modes, preceding IFS, response airtime, TXOP deadline, and DATA count. The parent fixtures supply observations without S4 assertions. Their successful runner result alone supplies no S4 behavior proof.

For the 1160-microsecond TXOP, the parent transmits a final BAR after five DATA frames. That BAR exchange ends at 2.270 ms. The deadline is 2.230 ms. This 40-microsecond overrun occurs with both protection modes. The S4 admission check rejects that continuation. EDCA later contends for a standalone BAR. The direct fixture requires every later BAR exchange to fit its applicable bounds.

A separate case gives a standalone BAR a 50-microsecond TXOP limit. The initial BAR exchange takes 120 microseconds. Its 70-microsecond overrun uses the permitted initial Control-frame exception. It has no preceding DATA or Management frame. The admission check preserves this case.

The diagnostic showcase traces also find DATA continuations beyond the TXOP bound in both parent and candidate. Each recorded violation ends with DATA. S4 does not repair general DATA admission. These observations limit the result to the changed Basic BAR paths. They do not establish general TXOP or NAV conformance.

### Completion cost

The cost fixture sends twelve UDP payloads of 700 bytes over a fixed 100-ms observation interval. One configuration sends one continuous burst. The other sends three bursts of four, with 20-ms gaps. Both configurations deliver all twelve payloads without a MAC drop. The payload goodput is 672000 bit/s in each configuration.

| Candidate measurement | Continuous burst | Three bursts |
| --- | ---: | ---: |
| ADDBA requests / responses / DELBA frames | 1 / 1 / 1 | 3 / 3 / 3 |
| Management airtime | 108 microseconds | 324 microseconds |
| Total measured deferral | 280.133424 microseconds | 822.400272 microseconds |
| Request contention | 61 microseconds | 156 microseconds |
| Response time after request | 159.066712 microseconds | 486.200136 microseconds |
| DELBA completion time | 114.066712 microseconds | 387.200136 microseconds |
| Sum of payload delivery delays | 147.632334920 ms | 43.933601088 ms |

The parent sends one request and one response in each configuration. It sends no completion DELBA. Its management airtime is 72 microseconds in each configuration. Both parent configurations deliver the same payload total. The parent and candidate also differ in extraction and BAR behavior. Thus, this comparison does not isolate an agreement-reuse cost. It supplies no general throughput or latency claim.

### Regression and source checks

The parent matches all nineteen original fingerprint rows. The evidence is `/tmp/s4-parent-fingerprints.log` and `/tmp/s4-parent-fingerprints-retry.log`. The selection preserves each original configuration, run number, simulation limit, and requested fingerprint ingredient. The final candidate run completes all nineteen rows in 227.330 seconds. Twelve rows match their baselines. Seven rows report fingerprint differences in configurations with Basic Block Ack enabled.

The final log is `/tmp/s4-fingerprints-final-limit.log`. Its runner exits with status 1 because the seven recorded baselines differ. Every simulation reaches its required limit, including the enabled ad hoc row at ten seconds. Earlier attempts hit its 600-second CPU limit. Those partial attempts supply no replacement baseline. The final retry-limit correction preserves all nineteen results from the preceding full run.

The following values are review candidates only. This execution does not approve or change a recorded baseline.

| Configuration | Candidate fingerprint values |
| --- | --- |
| `NoFragmentation` | `4a83-1e6f/tplx;09db-9f72/~tNl;0596-3a23/~tND` |
| `Fragmentation` | `91a8-1fb6/tplx;f43e-1f17/~tNl;aaae-de01/~tND` |
| `MixedTraffic` | `202c-1e73/tplx;4e7c-be6a/~tNl;5544-6a00/~tND` |
| `HCFfragblockack` | `140a-28bf/tplx;233d-c918/~tNl;5890-e570/~tND` |
| TXOP `General` | `06db-3079/tplx;3727-bbbd/~tNl` |
| Ad hoc `MacQos`, run 1 | `491a-01b6/tplx;9558-1598/~tNl;6970-8509/~tND` |
| `MacQosWithBlockAck` | `cdb0-57dc/tplx;aa99-d171/~tNl;1368-030c/~tND` |

The architecture check passes. The source-seal check permits every changed source path. The naming check reports the same 32 findings as the parent. The interface check retains the parent's two frame-step findings. The new QoS service interface passes that check. The logs use the `/tmp/s4-` prefix. No recorded fingerprint baseline changes belong to this implementation.

The protocol selection executes eight fixtures. Four pass. Delayed Block Ack, Multi-TID Block Ack, and both A-MPDU fixtures retain four declared expected failures. `N_CompressedBa` selects Basic in this branch. Its pass does not establish Compressed behavior. The log is `/tmp/s4-protocol-final-limit.log`.

The frame observer compares the first changed transmission in each affected configuration. It preserves actual bytes, sequence, TID, Retry, Duration, and selected mode. Its configuration adds an observer and a shorter observation limit. Thus, these observations explain local mechanisms; they do not replace the full fingerprint runs.

| Configuration | First observed change | Confirmed mechanism |
| --- | --- | --- |
| `NoFragmentation` | ADDBA request replaces the next Normal Ack DATA at sequence 1. | Preview avoids speculative DATA extraction before setup. |
| `MixedTraffic` | ADDBA request replaces the next large Normal Ack DATA at sequence 1. | The same extraction change permits setup before the successor. |
| `Txop` | ADDBA request precedes the next large DATA at sequence 2. | The same extraction change preserves the setup boundary. |
| `Fragmentation` | Final fragment 15 of sequence 1 reserves 464 instead of 116 microseconds. | Preview reads the queued unit before aggregation or fragmentation. |
| `HCFfragblockack` | Retransmitted fragment 2 of sequence 1 sets Retry. | Failed Block Ack status now sets the DATA Retry bit. |
| `MacQosWithBlockAck` | The first changed DATA has a 332-byte A-MSDU instead of a 166-byte unit. | Later extraction changes the available aggregation set. |
| Ad hoc `MacQos`, run 1 | Fragment 2 of sequence 1 reserves 484 instead of 628 microseconds. | Preview no longer extracts the next aggregate for the Duration estimate. |

These Duration observations do not prove complete NAV correctness. The existing Basic forecast does not model every BAR boundary. The direct timing assertions establish the separate BAR admission result.

The saturated ad hoc case also exposes a host execution cost in transfer-completion checks. A temporary queue observer records 5120 predicate checks in each unrelated queue. Its stack trace identifies `completeTransfers` through `hasQueuedData`. HCF now checks the category from EDCA's existing TID mapping first. It retains the other categories as a fallback for custom classification. This changes search order without new persistent state or a different existence predicate.

The temporary HCF observer records the same event count and fingerprint values before and after that search-order change. Its observed time in `processLowerFrame` falls from 41.0384 to 4.68354 seconds. Its observed time in `frameSequenceFinished` falls from 8.56017 to 0.226506 seconds. Host load differs between these observations. These values locate the cost; they supply no controlled speed benchmark.

### Reproduction and retained evidence

The local evidence directory is `audit/2026-10-15-s4/`. It retains the raw logs, parent observations, fingerprint selection, and temporary diagnostic sources. The diagnostic sources remain outside the delivered test suite. The directory also retains source hashes for this result. The build uses OMNeT++ from `/home/user/omnetpp-6.4.0aipre2` and the checkout's `setenv`.

The category runners use these selections from the repository root:

```sh
make MODE=debug -j8
inet_run_unit_tests -m debug -f 'Ieee80211(BlockAckScheduling|MultipleProtection|HtModeSet|HtModeCache|VhtHtCapabilities|ResponseModeSelection)_1\.test'
inet_run_module_tests -m debug -f 'Ieee80211(BlockAck(ActionDelivery|ActionDisposal|ActionDispatch|AssociationReset|DataSelection|Deferral|DeferredCollision|FragmentRetirement|Lifecycle|Negotiation|RetiredData|Reordering|ResponseWait|Admission|Variant|Scheduling|CompletionCost)|EdcaDataLifetime(Arbitration|Aggregation|Congestion|Defaults)?|Hcf(Header|SingleProtection|MultipleProtection|InternalCollision)|BasicCompatibility|RecipientNav|SingleProtectionGroup|VhtHtSupport)_[12]\.test'
inet_run_protocol_tests -p inet -m debug -w wifi -f '(N_BlockAck|Ac_BlockAck|N_Amsdu|N_CompressedBa|WifiQosDelayedBa|Ac_MultiTidBa|N_Ampdu|Ac_Ampdu)\.test$'
```

The fingerprint command runs from `tests/fingerprint`. The retained CSV contains the exact nineteen original rows.

```sh
./fingerprinttest -d -t4 -m '^/(showcases/wireless/(blockack|aggregation|fragmentation|qos|txop)|examples/(wireless|adhoc)/qos)/ ' -f tplx -f '~tNl' -f '~tND' -l /tmp/s4-fingerprints-final-limit.out /tmp/s4-fingerprint-final-limit.csv
```

### Scope limits

S5 owns Compressed response integration. Delayed Block Ack, Multi-TID Block Ack, and A-MPDU remain outside this result. Recorded fingerprint baselines remain unchanged. The existing prerequisite plans and unrelated local files retain their prior edits. The initial implementation checks precede the commit and push request. The publication checks extend that initial evidence.

## Initial checks before publication

The user authorized commit and push on 15 October 2026. The series separates DATA preview from its first BAR selection consumer. The preview commit is `e2d20d6be6`. Its isolated debug build passes. Five focused module fixtures pass, including the direct preview checks for ownership and sequence assignment. The behavior commit contains BAR selection, window progress, transfer completion, and their direct fixtures.

The final debug build passes. Its fresh selection passes 31 module fixtures and six unit fixtures. A separate scheduling run also passes after removal of the temporary parent-comparison directive. These checks retain the same runtime assertions. The logs are `/tmp/s4-publish-debug-build.log`, `/tmp/s4-publish-modules.log`, `/tmp/s4-publish-units.log`, and `/tmp/s4-publish-scheduling-clean.log`.

The preview commit matches all nineteen retained fingerprint rows. Its log is `/tmp/s4-preview-fingerprints.log`. The final protocol rerun returns four passes and four declared expected failures across eight fixtures. Its log is `/tmp/s4-publish-protocol.log`.

The final release compilation also passes. Its log is `/tmp/s4-publish-release-build.log`. The final fingerprint rerun completes nineteen rows. Twelve match their baselines; seven retain exactly the candidate values in this record. Its runner exits with status 1 for those differences. Its log is `/tmp/s4-publish-fingerprints.log`.

The whole-tree architecture gate reports 36 findings. The whole-tree naming gate reports 23 findings. The interface gate reports 15 findings. Each sorted finding set matches the selected parent. The source-seal check passes.

The series gates pass against `origin/master`; their two notes concern the earlier lifecycle commit `667ae51dc`. No pull request description check runs because this task creates no pull request.

The C++ quality gate does not run because this checkout lacks `src/compile_commands.json`. Its setup exits with status 2. The log is `/tmp/s4-publish-cpp.log`. The compiler checks and the other gates do not replace this static analysis.

## Rebase onto the updated S3b branch

The user authorized this rebase after the initial publication. The pinned S3b target is `f6f12c5c9fed341f5834d4ccdeedeec7a562e102`. The original S4 head is `65804c762e524f64d07cfa336da0ceeb0d2bd425`. The backup ref is `refs/backup/compressed-blockack-s4-before-s3b-rebase-20261015`. The two S4 commits retain their order and purpose.

The target adds local allocation indices for DATA retirement across large sequence gaps. The rebase retains that production mechanism alongside S4 window progress and completion DELBA. The collision fixture now records the extracted frame's allocation index. The fragment-retirement fixture retains all twenty gap scenarios and the Basic response path. A deferred successor keeps the earlier transfer open until the fixture advances the allocator. This prevents completion DELBA before the intended inactivity check.

The user permits builds and tests only at the final commit. No intermediate commit receives a build or test. The earlier results above remain historical evidence. Final verification uses the last rebased commit. Its results and preserved-file hashes belong to `audit/2026-10-15-s4-rebase/`.

## Approved fingerprints and final Basic boundary

The user approved the seven final fingerprint changes on 2026-10-15 after review of their causes in PR #1344. This approval covers the non-graphical ingredients in the table below. Graphical `tyf` values remain unchanged and unverified. This section supersedes the earlier candidate values and the statements that S4 includes no baseline update.

The final correction limits Basic single protection to an already prepared successor. HCF ends the TXOP before further DATA extraction can change aggregation, fragmentation, or agreement setup. Prepared fragments retain their continuation path. Final DATA in a zero-limit TXOP reserves only an immediate response when required. The policy can require more contention; it preserves standalone BAR progress.

The source and tests at `8745330c1cc398f3dbc0a43be0c60cd7c2d5c529` contain this correction. Debug and release builds pass at that revision. All 31 module fixtures and six unit fixtures pass. Protocol execution reports four passes and four declared expected failures. The nineteen fingerprint simulations complete; twelve match the previous baselines and seven produce the approved values below. The raw evidence belongs to `audit/2026-10-15-s4-duration/`.

The final observer shows zero Duration for sequence 1, fragment 15, in the fragmentation showcase. The TXOP ends with that DATA; BAR follows in a separate TXOP. The ad hoc observer shows 44 microseconds for sequence 1, fragment 2. Its ACK advertises zero Duration, and the TXOP ends. These observations replace the earlier unverified 464- and 484-microsecond forecasts. They do not establish general Basic DATA admission or complete NAV conformance.

| Configuration | Approved fingerprints | Cause of the movement |
| --- | --- | --- |
| Block Ack `NoFragmentation`, run 0 | `9161-625d/tplx;6a02-09aa/~tNl;cbc0-3b0a/~tND` | Final Normal Ack DATA reserves only its ACK; setup and BAR exchange order also change. |
| Block Ack `Fragmentation`, run 0 | `36f4-5c22/tplx;66b0-94cd/~tNl;45f3-af4f/~tND` | Final Block Ack DATA reserves zero; BAR obtains another TXOP. |
| Block Ack `MixedTraffic`, run 0 | `351b-d4ba/tplx;5c1f-69a8/~tNl;6182-6da3/~tND` | The current reservation excludes the unextracted large successor; setup can precede that DATA. |
| Fragmentation `HCFfragblockack`, run 0 | `2597-1e2e/tplx;d269-c6ae/~tNl;2f5c-4cca/~tND` | Failed Block Ack feedback sets Retry on retransmitted DATA; the final burst boundary also changes. |
| TXOP `General`, run 0 | `ab91-87f3/tplx;f30c-6aa4/~tNl` | The reservation excludes an unextracted successor, and the frame sequence enforces that boundary. |
| Ad hoc `MacQos`, run 1 | `4fd2-bb56/tplx;ecaf-89f7/~tNl;9398-0fae/~tND` | Final Normal Ack DATA reserves SIFS plus ACK; further extraction requires another TXOP. |
| Wireless `MacQosWithBlockAck`, run 0 | `6edb-0326/tplx;2a8a-50ee/~tNl;4301-882d/~tND` | ADDBA reserves its immediate ACK without unrelated queued DATA; later extraction can change aggregation. |

The final history keeps the preview preparation separate. One behavior commit contains BAR selection, the final Duration boundary, and these seven baseline rows. Its production source and tests match `8745330c1c`. This placement avoids an intermediate behavior commit with obsolete expectations. Final verification after the baseline update uses only the final commit, as requested.

## Explicit agreement arguments

BAR policy calls now require an explicit agreement handler through both the interface and concrete policy types. Transmitted-BAR processing also requires an explicit originator agreement. An omitted argument fails compilation instead of silently selecting behavior without agreement state. Explicit null values retain their documented behavior. Production callers already supply these arguments, so their runtime behavior stays unchanged.

Compile-time checks cover omitted arguments through both policy types and the ACK handler. The scheduling fixture also checks explicit null policy calls with outstanding DATA. Custom test policies follow the same explicit contract. The migration guide describes the source compatibility change. The seven approved fingerprint rows stay unchanged.

The source-equivalence statement in the preceding section describes baseline acceptance at `fcedba79e3`. This later correction changes API declarations, compile-time checks, and documentation. It changes no production C++ function body.
