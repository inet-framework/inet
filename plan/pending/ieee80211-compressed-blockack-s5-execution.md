# S5 execution: Compressed Block Ack beside Basic compatibility

S5 is complete through C1, C2, C3, and C4. The authorized TXOP correction removes the active Compressed timing blocker. HCF now defaults to Compressed. Explicit Basic compatibility remains available.

The comparison parent is `75559a49661e33f727fa93a8c81c021596cfb4c9`. The active checkout is `/home/user/omnetpp_ws/inet-compressed-blockack-s5`. The maintainer requested S5 execution on 16 October 2026. That request authorizes C1 through C4 in the [replacement plan](ieee80211-compressed-blockack-replacement.md#compressed-replacement-steps).

The checkout contained changes to `AGENTS.md` and four prerequisite plans before execution. It also contained an untracked planning history. This work preserves those changes. The pinned parent contains the S4 implementation. The [S4 execution record](ieee80211-compressed-blockack-s4-execution.md) supplies its earlier evidence and limits.

## Implementation contract

HCF owns the immutable profile and validates received frame formats before agreement updates. An active agreement identifies one peer and TID. A valid conflicting BAR stops the simulation because ADDBA does not negotiate the profile. Malformed input cannot cause that diagnostic, receipt updates, or agreement recovery.

The recipient record owns explicit Compressed receipt bits and its scoreboard window. The reorder service owns delivery order and its separate window. Delivery must preserve receipt bits. An absent receipt produces zero, including positions before the first received DATA. Basic retains its fragment bitmap and existing compatibility behavior.

The rate selector consumes the MIB rate sets. It selects a non-HT Compressed response and distinguishes initial BAR from later BAR. Frame sequences supply the actual request profile. Protection, timeouts, and duration calculations use the corresponding response length. Compressed responses contain 32 bytes with FCS; Basic responses contain 152 bytes.

For an active Compressed agreement, the originator suppresses fragmentation for that peer and TID. The recipient rejects fragmented DATA before receipt or reorder updates. Normal Ack outside that agreement retains its existing fragmentation behavior.

## Recipient interframe design

The recipient must distinguish receipt evidence from the interframe relation. An agreement with start 100 and size four first receives Normal Ack DATA 100. Delivery advances the reorder start to 101. DATA 104 advances the scoreboard start to 101 and preserves receipt evidence for 100. A qualifying interframe relation permits delivery of 104 despite the gap. A false receipt bit or absent relation retains 104.

Local receive endpoints alone cannot establish the ACK-to-DATA relation with propagation delay. The medium therefore supplies one copied snapshot of physical endpoints. The snapshot contains transmission identity, transmitter identity, transmission endpoints, and cached arrival endpoints. It retains no frame, transmission, arrival, or radio pointer.

The PHY derives the interval at the current transmitter. For the same transmitter, it subtracts the preceding transmission end from the current transmission start. Otherwise, it subtracts the preceding signal's cached arrival end at the current transmitter. This calculation uses the propagation result that the medium already produced. It does not assume equal outward and return delay.

HCF owns local event history. A pending candidate preserves the interval across PHY reception completion and later MAC delivery. A receiver-created transmission identifier connects those events. Unrelated receptions replace prior history. Failed, abandoned, and overlapping receptions must not preserve stale eligibility. Every local transmission clears prior history; completed ACK transmission can establish new history.

SIFS recognition uses the reconstructed interval after signal extension. The current PHY model uses baseline timing without coverage-class adjustment. The permitted bound is one tenth of the slot time. Integer tick division rounds that bound inward. Physical propagation remains separate from this MAC timing parameter.

RIFS recognition requires permitted HT operation, the same transmitter and RA, and no preceding SIFS response obligation. It uses the preceding PPDU endpoint without signal extension. Management must preserve the advertised RIFS Mode. VHT operation does not permit RIFS. This recipient rule adds no RIFS transmitter scheduler.

The review found that ordinary HT transmission at 2.4 GHz omitted the six-microsecond signal extension. The correction belongs in the authoritative PHY duration. HCF must not add it again. Existing short-GI duration differences remain inherited PHY fidelity limits; they do not independently establish an interval error.

## Standards evidence

The standards source is IEEE Std 802.11-2024. The fresh local corpus is `/home/user/omnetpp_ws/inet/standards/processed`. The corpus lint output is `/tmp/inet-s5-standards-lint.json`.

| Canonical node | Physical PDF pages | Requirement |
| --- | --- | --- |
| `ieee80211-2024:table:9-36` | 719–720 | BAR variant Type encoding. |
| `ieee80211-2024:table:9-37` | 722–723 | Block Ack variant Type encoding. |
| `ieee80211-2024:clause:10.25.6.3` | 2060–2061 | Full-state recipient receipt state. |
| `ieee80211-2024:clause:10.25.6.6.2.1` | 2065–2066 | Recipient delivery and interframe conditions. |
| `ieee80211-2024:clause:10.3.2.3.3` | 1889–1890 | SIFS definition and tolerance. |
| `ieee80211-2024:clause:10.3.2.3.2` | 1888–1889 | RIFS definition and conditions. |
| `ieee80211-2024:clause:10.22.5` | 1997–1998 | Coverage-class timing behavior. |
| `ieee80211-2024:table:19-25` | 3493–3494 | Band-specific HT signal extension. |

These provisions are normative. The selected corpus clauses resolve uniquely. The review inspected the original PDF for Clause 11.38.1 on physical page 2886. That clause requires a VHT AP to clear RIFS Mode. The corpus result without children does not include all later prose in that clause.

## Verification boundary

The initial full debug build and subsequent focused rebuilds pass. A build supplies no exchange or timing proof. The focused wire fixture passes independent octet checks. The constructed serializer fixture passes 278 cases. The HCF input fixture passes both configured profiles with supplied recipient agreements. Those results do not establish negotiation or PHY interval recognition.

Five initial protocol simulations pass across the selected n/ac fixtures. They retain Basic coverage and add actual Compressed exchanges. This initial boundary precedes complete infrastructure timing classification and final interframe execution. Later sections record those results and the authorized correction.

An isolated observer reproduces a DATA continuation overrun in the pinned parent. With a one-millisecond TXOP, the third DATA ends 178 microseconds after the bound. The candidate reaches the same endpoints. The parent also reproduces the first observed A-MSDU overrun. These observations identify inherited DATA admission limits. They do not establish general TXOP conformance or excuse a new Compressed violation.

No recorded fingerprint baseline changed. This record does not authorize a baseline change or publication.

## Confirmed timing blocker

The candidate before the TXOP correction introduced a failing supported case that the exact parent did not reproduce. General run 6 uses HT operation, 24 Mbit/s DATA, single protection, aggregation, a one-millisecond TXOP, and seed zero. That candidate transmits two later A-MSDUs beyond their TXOP bounds. Each overrun is 310 microseconds. The affected TXOP contains two DATA frames, so the initial-MSDU exception does not apply.

The second DATA ends at 1.014426 seconds. Its TXOP bound is 1.014116 seconds. The violation exists before any Block Ack response. The aggregate contains 1532 bytes in both versions. The exact parent has no aggregate overrun in run 6. Parent run 14 demonstrates a similar admission defect under different conditions. That other case does not make candidate run 6 acceptable.

A direct run 6 reproduction also fails with the final reviewed debug library. Its output is `/tmp/inet-s5-reviewed-run6.out`; its error is `/tmp/inet-s5-reviewed-run6.err`. That run stops at the first later A-MSDU violation, at 1.014426 seconds. The earlier non-fatal observer campaign records both violations.

The inherited admission mechanism causes the new result. `HcfFs` tests positive remaining time before a later exchange. It does not establish that the complete exchange fits. HCF excludes Block Ack support from its existing complete-cost DATA guard. The corrected HT control airtime and Compressed responses change the trajectory. The comparison does not isolate their separate contributions.

All sixteen initial parent and candidate runs deliver thirty payloads, with 21000 payload bytes per run. All observed BAR/Block Ack exchanges remain within their applicable bounds. Those successes do not establish DATA containment. An independent review rejected that candidate under the [plan's timing condition](ieee80211-compressed-blockack-replacement.md#airtime-acceptance-and-admission-dependencies). The later correction and C3 acceptance below supersede that rejection.

The [separate correction proposal](ieee80211-compressed-blockack-txop-admission.md) addresses preparation and admission. A final transmit guard alone cannot solve the problem. Aggregation already deletes constituent frames before HCF selects the actual mode. An oversized aggregate can therefore remain prepared and fail every later attempt. The proposal must preserve progress through ordinary MSDUs and the permitted initial-MSDU exception.

## Focused verification after source review

The reviewed debug library has SHA-256 `1635793563ff1c0912a4a0e55a70507879af2a20ef83a0b88502562748a347db`. Its build log is `/tmp/inet-s5-reviewed-debug-build.log`. The interframe fixture and direct run 6 reproduction use this library. The earlier mixed-interface regression batch is discarded.

The unit and legacy results below use their recorded fresh libraries before the final bounded PHY review corrections. Those corrections repair empty-cache lookup, remove an unused virtual, and expose a watch. The protocol campaign uses library `1c57b8f562dbeb5cddbba49b77a3a676042fea7aef9b9e5578e1980c63109f56`. These earlier results retain their recorded source boundaries; they are not new executions against the final relink.

| Check | Result | Evidence |
| --- | --- | --- |
| Focused units | Nine fixtures pass. | `/tmp/inet-s5-final-unit.log` |
| HT Operation conversion | One fixture passes both RIFS permission values and operation equality checks. | `/tmp/inet-s5-ht-operation-unit.log` |
| Legacy and HCF input modules | Thirty fixtures pass across the main batch and two repaired-fixture reruns. | `/tmp/inet-s5-final-module.log`, `/tmp/inet-s5-repaired-module.log` |
| EDCA DATA lifetime modules | Nine fixtures pass. | `/tmp/inet-s5-lifetime-module.log` |
| Production interframe fixture | All 28 runs pass with seed zero. | `/tmp/inet-s5-interframe-final/` |
| Constructed serializers | All 278 cases pass. | `/tmp/inet-s5-serializer-roundtrip.log` |
| n/ac protocol fixtures | Five simulations pass with Basic and Compressed coverage. | `/tmp/inet-s5-exchange-protocol-final.log` |
| Lowest response mode | Two real infrastructure runs pass at 6 Mbit/s. | `/tmp/inet-s5-exchange-lowest-config.out` |

The two legacy fixtures required valid synthetic frames. The Action-dispatch fixture now supplies the BAR trailer. The fragment-retirement fixture now supplies all 64 fragment bitmap entries. Their behavioral assertions remain intact.

The interframe fixture supplies accepted agreements. It uses real PHY reception and HCF ACK completion. It verifies SIFS and RIFS bounds, sequence wrap, prior receipt truth, unrelated traffic, retirement, and cache expiry. Its BUSY control uses a signal below reception sensitivity but above energy detection. Its 168 malformed BAR receptions each leave zero pending candidates. Separate infrastructure tests establish real association and ADDBA; the interframe fixture does not claim negotiation coverage.

The source reviewers report no remaining confirmed correctness defect in their bounded core and interframe scopes. Architecture gates pass for the changed MAC and PHY subtrees. Review supplies no runtime proof beyond the executed fixtures.

## Extension contracts in this candidate

These obligations describe the current candidate. They require migration guidance before publication. They do not change the C3 blocker or activate the default.

| Contract | Obligation for a custom implementation |
| --- | --- |
| `IQosRateSelection::computeCompressedBlockAckReqMode()` | Select a legal non-HT BAR mode from the supplied initial-or-later TXOP position and current rate sets. |
| `IRecipientQosAckPolicy::computeCompressedBlockAckDurationField()` | Subtract SIFS and the actual 32-byte response airtime from the request reservation. Preserve the Basic method. |
| `IRecipientQosMacDataService::dataFrameReceived()` | Accept the new interframe argument. Preserve its meaning through recipient processing. A false value supplies no interval evidence. |
| `QoSContext` | Supply HCF's resolved profile to the exchange. Existing callers retain the Basic default unless they explicitly supply Compressed. |
| `RecipientBlockAckAgreement::setVariant()` | Set the profile during agreement creation. Do not change it during that agreement's lifetime. Compressed creation initializes explicit receipt state. |
| `IRadioMedium` | Implement the new snapshot and gap queries with copied physical facts. Unavailable transmissions or arrival endpoints produce an absent result. |
| `ICommunicationCache::findCachedArrival()` | Return a cached arrival without insertion, resizing, geometry recalculation, or other state changes. |
| `IIeee80211Mode::getSignalExtensionTime()` | Supply the nominal extension for the mode. `Ieee80211ModeBase` supplies zero; ERP and HT override it where necessary. |

## Common regression result

All nineteen selected rows match their recorded non-graphical fingerprints. The runner uses debug mode and the original configuration, run, time limit, and requested non-graphical ingredients. It completes in 182.173 seconds with exit status zero. The selected input is `/tmp/inet-s5-fingerprint-selection.csv`; the log is `/tmp/inet-s5-fingerprint.log`.

The command runs from `tests/fingerprint`:

```sh
./fingerprinttest -d \
  -m '(showcases/wireless/(blockack|aggregation|fragmentation|qos|txop)|examples/(wireless/qos|adhoc/qos))' \
  -f tplx -f '~tNl' -f '~tND' -t 6 \
  /tmp/inet-s5-fingerprint-selection.csv
```

The graphical `tyf` ingredient remains unverified. No fingerprint baseline changed. These matches preserve the selected legacy regression evidence. They do not override the direct Compressed timing failure.

These initial regression matches precede the separate TXOP correction and C4. No commit or publication occurred during this execution.

## Build state before the authorized TXOP correction

Both debug and release builds pass. The final release freshness check uses `make MODE=release -j6` from the repository root. Its log is `/tmp/inet-s5-reviewed-release-build.log`. The final debug build uses `make MODE=debug -j6`; its log is `/tmp/inet-s5-reviewed-debug-build.log`. The later NED edit corrects a comment and retains the Basic default.

The separate TXOP proposal passed a consistency review as a scope request. On 2026-10-16, the maintainer authorized that correction and subsequent S5 completion. Source validation and independent review resolved the RTS boundary, lifetime ownership, and mode validation before implementation. The retained strict fixture preserves the inherited outside-agreement failure separately from active Compressed acceptance.

## Authorized TXOP correction and C3 acceptance

The [TXOP contract](ieee80211-compressed-blockack-txop-admission.md#resolved-implementation-contract) defines source ownership and each event boundary. The service creates duplicate previews without source removal. HCF selects one DATA mode, evaluates the aggregate, and evaluates an ordinary fallback with that same mode. Commitment transfers the same preview pointer only before DATA submission. Refusal preserves queue frames, deadlines, sequence state, and allocation state.

RTS failure before DATA updates QSRC and the contention window without a provisional sequence counter. Original MSDU lifetimes bound repeated attempts. Expiry, capability loss, or changed response inputs cancel invalid preparation. Single protection reserves only the current exchange. A later BAR can require fresh contention. Basic and traffic outside active Compressed agreements retain their existing preparation and admission.

Runtime verification found two effective selection callers that bypassed the first implementation. `RtsCtsFs` still extracted originals after admission. The multicast repeat predicate could also extract a prepared unicast successor before it ended the group sequence. Both callers now use the context-selected frame. Their focused regression cases preserve original queue state when no DATA submission occurs.

The first retry policy required containment for all committed frames. A real BAR collision exposed a progress failure at 6 Mbit/s. One ordinary protected exchange needs 1.166 milliseconds under a nominal one-millisecond limit. Its first attempt qualifies for an exception, but conservative retry containment refused every later attempt. The implementation now uses the standard same-size single-MPDU exception with no earlier Data or Management frame. The supported path preserves committed MPDU size across retries.

New previews clear Retry from the previous wireless hop. Committed retries derive Retry from local acknowledgment state. An oversized first-attempt A-MSDU still cannot use the retry exception. Internal collision commits only ordinary queued DATA before its retained recovery action. Multiple protection reserves the actual complete exchange for a permitted overrun.

The corrected debug library has SHA-256 `0298dbd4c9625cd6f306865f5c1cfd0a3412230c6339b3a84289eb68eba713df`. The release library has SHA-256 `4b32335cdb2f558d60ada0761e46fde84c9879889e1c46b3c3f69bd11d9ae780`. Debug build 4 and the release build pass. Their logs are `/tmp/inet-s5-txop-debug-build-4.log` and `/tmp/inet-s5-txop-release-build.log`.

The focused admission fixture passes all 24 cases with seed zero. It checks refusal, ordinary fallback, ownership, deadlines, sequence state, one mode query, and RTS cancellation. It also checks QSRC rollover, lifetime expiry, capability changes, internal collision, and multicast-to-unicast refusal. Cases 22 and 23 require actual ordinary and A-MSDU retries with unchanged size and DATA airtime above one millisecond. Evidence is `/tmp/inet-s5-admission24.out` and `/tmp/inet-s5-admission24.err`.

All sixteen explicit-profile infrastructure cases pass the scoped timing assertions. The observer records agreement scope at DATA submission, before later teardown can change lookup results. Active Compressed checks include DATA with Normal Ack. No active later DATA or ACK overrun remains. All 98 matched BAR/Block Ack exchanges finish within their bounds. All 32 hop summaries report zero Block Ack overruns.

The former run 6 failure now passes with zero DATA, ACK, or Block Ack overruns on both hops. Those hops transmit 23 and 11 active aggregates. The separate two-run lowest-response campaign records 29 actual 32-byte responses at 6 Mbit/s. Their airtime is 74 microseconds, with exactly 10 microseconds of SIFS. No Block Ack response exceeds its bound.

The infrastructure observer separately records inherited violations outside active agreements. `StrictAllTiming` retains the original aggregate assertions and reproduces the pre-agreement aggregate failure in run 4. This acceptance establishes no timing compliance for those retained paths. The default change does not repair Basic or outside-agreement admission.

The cleanup assertion permits only the idle `WAITING_FOR_SUCCESSOR` procedure with `SIMTIME_MAX` response deadline. Queues, in-progress frames, agreements, pending BAR state, and agreement timers must remain empty or inactive. That phase records a possible future setup after a completed Normal Ack transfer. It holds no unresolved exchange.

Independent review accepts C3 for this scope and permits C4. The verdict is `/tmp/inet-s5-c3-acceptance-review.md`. Explicit-profile evidence is `/tmp/inet-s5-exchange-scoped-general16.out` and `/tmp/inet-s5-exchange-scoped-lowest2.out`. The scope table is `/tmp/inet-s5-exchange-scoped-general16.csv`. The unchanged strict failure remains in `/tmp/inet-s5-exchange-strict-all-inherited.out` and its `.err` file.

## C4 default and migration verification

`Hcf.ned` now defaults to Compressed. The infrastructure fixture omits the selector after C3 and requires actual Compressed frames. All sixteen default cases pass. Both lowest-response cases also pass after default activation. Evidence is `/tmp/inet-s5-default-exchange16.out` and `/tmp/inet-s5-default-lowest2.out`.

All six external-style migration cases pass without an explicit selector. Legacy g and a links, and HT ad hoc links without peer records, deliver twelve UDP payloads through Normal Ack. Support-disabled controls produce no irrelevant warning. The fixture checks the resolved default, fallback diagnostics, absent ADDBA/BAR/Block Ack, and empty active state. Evidence is `/tmp/inet-s5-default-migration6.out`.

All twelve initialization cases pass after default activation. They cover the omitted selector, explicit Basic, explicit Compressed, unknown values, and both delayed-policy guards. Evidence is `/tmp/inet-s5-default-variant12.out`. Retained Basic fragment, bitmap, and public API checks remain separate regression obligations.

Both `N_BlockAck` and `Ac_BlockAck` also pass three consumer runs: explicit Basic, explicit Compressed, and the NED default. The third run uses bare INI `default`, which invokes OMNeT++ `acceptDefault()`. It verifies the resolved profile at all three HCF instances. Actual Compressed BAR/Block Ack and positive receipt bits complete that run. All six simulations and both diagnostic scripts pass. Evidence is `/tmp/inet-s5-default-consumer-protocol6.log` and `/tmp/inet-s5-default-consumer-{n3,ac3}.out`.

Both final build freshness checks pass after the NED change. Their logs are `/tmp/inet-s5-c4-debug-build.log` and `/tmp/inet-s5-c4-release-build.log`. The C4 change affects the runtime NED default; the recorded debug and release library hashes remain unchanged.

## Final regression evidence

All final checks use the active checkout and seed zero where a simulation requires a seed. Debug mode retains runtime assertions. The release build supplies a separate compilation check. No fingerprint expectation changed.

| Check | Final result | Evidence |
| --- | --- | --- |
| Unit fixtures | All ten pass. | `/tmp/inet-s5-c4-unit.log` |
| Module regression batch | Forty-one pass initially. The retained Basic deferral fixture passes after its explicit profile correction. All 42 selected fixtures therefore pass across the batch and focused rerun. | `/tmp/inet-s5-c4-module-regression.log`, `/tmp/inet-s5-c4-deferral-module.log` |
| Default infrastructure and migration | Both additional module fixtures pass: sixteen infrastructure cases and six migration cases. Both lowest-response cases pass. | `/tmp/inet-s5-default-module.log`, `/tmp/inet-s5-default-lowest2.out` |
| Serializer | All 278 constructed cases pass, with zero unexpected or stale coverage entries. The separate golden-octet unit also passes. | `/tmp/inet-s5-c4-serializer.log`, `/tmp/inet-s5-c4-unit.log` |
| Wi-Fi protocol files | Five supported files pass. Four deferred files retain expected failures. No unexpected result occurs. | `/tmp/inet-s5-default-plan-protocol9.log`, `/tmp/inet-s5-default-protocol-deferred.txt` |
| Fingerprint rows | All nineteen selected rows match in 153.811 seconds. | `/tmp/inet-s5-c4-fingerprint.log` |
| Architecture | All three scoped checks pass. | `/tmp/inet-s5-c4-architecture-mac.log`, `/tmp/inet-s5-c4-architecture-common.log`, `/tmp/inet-s5-c4-architecture-wifi.log` |

The module union contains 44 distinct fixtures. It preserves Basic teardown, fragmentation, NAV, setup, association reset, response modes, and lifetime checks. It repeats the 28 physical interframe cases, 24 admission cases, and 76 scheduling cases after default activation. The scheduling fixture retains all original Basic cases. It adds actual Compressed shifted responses through active frame sequences and small-window arbitration.

The recipient integration checks DATA 100, 102, and 101 through HCF. Delivery order is 100, 101, and 102. A subsequent BAR produces three positive receipt bits and zero holes after delivery. The six focused originator cases call the HCF control boundary directly. The scheduling fixture separately supplies actual response matching. These fixtures supply accepted agreements; the infrastructure and protocol fixtures prove negotiation separately.

The legacy deferral fixture omitted a selector while it used non-HT mode `a`. The Compressed default correctly prevented its expected Basic setup. The fixture now selects Basic explicitly. All twelve original scenarios pass without assertion changes. This correction preserves the fixture's intended historical profile.

The deferred protocol results remain explicit. `N_Ampdu` and `Ac_Ampdu` first miss an A-MPDU subframe. `Ac_MultiTidBa` passes both TID observations before it misses Multi-TID Block Ack. `WifiQosDelayedBa` observes a delayed request before recipient delayed acceptance fails. No expectation changed, and these results establish no support for those features.

The interface gates report two MAC findings and three PHY findings. Their output is byte-identical to the pinned parent's output. The implicated source files remain unchanged. Logs are `/tmp/inet-s5-c4-interfaces-mac.log` and `/tmp/inet-s5-c4-interfaces-phy.log`, with corresponding `/tmp/inet-s5-parent-interfaces-*.log` files. This work adds no interface-gate finding.

The final unit command runs from the checkout root:

```sh
inet_run_unit_tests -m debug -f '(Ieee80211BlockAck(Wire|Scheduling|ActionWire)_1|Ieee80211CompressedBlockAck(Rate|Record)_1|Ieee80211(ResponseModeSelection|ResponseTimeout|HtModeSet|HtCapabilities|VhtHtCapabilities)_1)\.test$'
```

The module regression command runs from the checkout root:

```sh
inet_run_module_tests -m debug -f 'Ieee80211(BlockAck.*|EdcaDataLifetime.*|PreparedCancellation_1|RecipientNav_1|Txnav_1|Hcf(SingleProtection_1|MultipleProtection_[12]|Header_1)|SingleProtectionGroup_1|VhtHtSupport_1|HtAssociation_1|Mgmt(StateCommit|RatePublication)_1|CompressedBlockAck(Input|Interframe|Admission|Originator)_1)\.test$'
inet_run_module_tests -m debug -f 'Ieee80211BlockAckDeferral_1\.test$'
inet_run_module_tests -m debug -f '(Ieee80211CompressedBlockAckExchange_1|Ieee80211CompressedBlockAckMigration_1|Ieee80211BlockAckVariant_1)\.test$'
```

The serializer and protocol commands run from the checkout root:

```sh
inet_run_serializer_tests -m debug -f 'serializer_chunk_roundtrip\.test$'
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '(N_BlockAck|N_CompressedBa|N_Amsdu|N_Ampdu|Ac_BlockAck|Ac_BlockAckFallback|Ac_Ampdu|Ac_MultiTidBa|WifiQosDelayedBa)\.test$'
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '(N_BlockAck|Ac_BlockAck)\.test$'
```

The final fingerprint command repeats the nineteen-row command above from `tests/fingerprint`. It uses the original configurations, runs, time limits, and selected non-graphical ingredients. Graphical `tyf` remains unverified. No CSV or other recorded baseline changed.

Current user and migration guides describe the default, fallback, profile conflict, fragmentation boundary, and added interface obligations. Release notes preserve the supported `ac` path and all four deferred protocol requirements. The Basic showcase retains its explicit profile, topology, and result rows.

The evidence remains limited to the recorded deterministic cases and seed. Production HT catalogs provide no distinct legal alternate response for the tested 32-byte frame. The selector unit verifies that alternate-rate contract separately. The recipient prefix release remains an interpretation of ordered delivery together with the individual-MPDU release condition. This work establishes no complete historical Basic conformance or complete VHT transfer conformance.

The implementation checks above precede the separate commit, push, and pull request phase. The user authorized that phase after implementation. The original local plan changes and untracked history remain outside the publication scope.
