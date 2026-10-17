# S3b execution record

## Completed S3b

S3b is complete on branch `work/compressed-blockack-s3b` in [PR #1343](https://github.com/inet-framework/inet/pull/1343). The current source revision is `8646a9e6808ea8d21ac28bfc1271537469225fe9`, above PR base `bba1db4c8180a592894e0ab2a2dc056d9cb08082`. The broad verification below covers `8ba7c9e47bc3ded924cfa3917c8b899baccf789d`; a separate record covers the later allocation-index repair. Branch completion does not assert a merge into `master`. S4, A5, and production Compressed exchanges remain outside this completed stage.

The six implementation commits supply optional selected extraction, lifecycle integration, first-DATA collision retirement, frame-sequence interruption ownership, fragment retirement, and allocation-index comparisons. A separate documentation commit records stage completion. The [S3b summary](ieee80211-compressed-blockack-s3b-summary.md) explains the resulting behavior. The [migration guide](../../doc/src/migration-guide/index.rst) defines custom-module changes. Basic remains the default; HCF rejects Compressed selection during initialization.

The latest repair retains the highest transmitted allocation index in the agreement. The QoS data service assigns this local index before fragmentation; all fragments retain it in `Ieee80211DataSequenceTag`. Ordinary teardown discards unsent fragments through that index, even after ACK processing removes an earlier fragment. Older retransmissions cannot move the index backward, and wire sequence numbers remain unchanged.

The fragment fixture transmits DATA with sequence 0. It allocates intervening DATA through the production data service and deletes that DATA without transmission. Fragment 0 of sequence 2048 then transmits. The index advances correctly; teardown removes unsent siblings of sequence 2048 and preserves later intact DATA. This case tests an allocation gap; it does not test lifetime expiry of the intervening DATA.

| Verification at source revision `8ba7c9e47bc3ded924cfa3917c8b899baccf789d` | Result | Saved local artifact |
| --- | --- | --- |
| Debug compilation | Pass | `/tmp/pr1343-publish-debug.log` |
| Release compilation | Pass | `/tmp/pr1343-publish-release.log` |
| Selected debug module fixtures | 26 pass | `/tmp/pr1343-publish-modules.log` |
| Functional fingerprint cases, without graphical `tyf` ingredients | 19 pass | `/tmp/pr1343-publish-fingerprints.log` |
| Fragment reproduction against `53edefe95a6cc9671ef8d4864c913c2a5669306d` | Fails at the retirement assertion | `/tmp/pr1343-fragment-before.err` |
| Fragment fixture after the repair | Four scenarios pass | `/tmp/pr1343-fragment-after.log` |
| Commit, classification, include, parameter, and PR-description checks | Pass | `/tmp/pr1343-publish-mechanical.log` |
| Source-seal check | Pass | `/tmp/pr1343-publish-seals.log` |

The commands for this earlier verification run from the repository root after environment setup:

```sh
make MODE=debug -j12
make MODE=release -j12
inet_run_module_tests -m debug -f '(PacketSelectiveExtraction_1|Ieee80211BlockAck.*_1|Ieee80211EdcaDataLifetime.*|Ieee80211HcfInternalCollision_1)\.test$'
```

The functional fingerprint command runs from `tests/fingerprint`:

```sh
./fingerprinttest -d -F tyf -t 4 -m 'showcases/wireless/(blockack|aggregation|fragmentation|qos|txop)/|examples/wireless/qos/.*MacQos|examples/adhoc/qos/.*MacQos' examples.csv showcases.csv
```

Seven approved functional fingerprint rows belong to the lifecycle implementation. Later repairs retain those values and add no baseline change. Twelve controls retain their functional expectations. Graphical expectations remain unchanged and outside this functional result. The earlier verification at `667ae51dc9fbfbfac9ea68122bf65bc4f2d3e1df` includes 37 module fixtures, six unit fixtures, and 13 Basic demonstrations. Its protocol selection records four passes and four declared expected failures. Those earlier results do not establish final-head coverage for the same selections.

At `8ba7c9e47b`, whole-project architecture and naming checks retain the findings present at the PR base. The interface check retains 15 findings and nine review notes; the two new interfaces pass. The changed source paths are unsealed. Raw `/tmp` artifacts are local evidence; they are not part of the published repository.

### Allocation-index repair at `ea20a2880e`

The series now places the allocation-index repair before the final plan commit. The repair maps from `ea20a2880e0ca607e680def1a875c6702c4ef249` to `8646a9e6808ea8d21ac28bfc1271537469225fe9` without source or test changes. The results below retain their original revision. The history comparison establishes content equivalence; it is not a new behavioral test.

Commit `ea20a2880e0ca607e680def1a875c6702c4ef249` replaces cyclic comparisons with allocation indices. It adds a MSG definition for local metadata, without a wire-format change. Custom QoS data services must supply the tag, as the migration guide requires.

| Recorded verification for the allocation-index repair | Result | Saved local artifact |
| --- | --- | --- |
| Debug compilation | Pass | `/tmp/s3b-push-debug.log` |
| Release compilation | Pass | `/tmp/s3b-push-release.log` |
| Fragment-retirement, lifecycle, association-reset, and DATA-selection fixtures in debug mode | Four fixtures pass | `/tmp/s3b-push-tests.log` |

The commit records 50 successful simulation runs across these four fixtures. The fragment fixture supplies 20 runs with these parameters:

- The initial sequence is 0 or 4095.
- The allocation distance is 0, 2047, 2048, 2049, or 4096.
- Teardown occurs during transmission or after the first fragment receives its Normal Ack.

The fixture supplies accepted agreements and ACKs; production code performs fragmentation, expiry, retirement, and transmission. Later intact DATA and unrelated-peer DATA remain controls.

These archived results do not extend the earlier 26-fixture or 19-fingerprint results to this revision. Release-mode behavioral tests, sanitizers, and other platforms remain unverified at this revision. The later audit checks the repair and documentation claims through source inspection; it is not a full independent correctness audit.

This document correction changes no production source or recorded expectation. It repeats no build or simulation test. The checkout's existing `AGENTS.md`, four pending plans, and planning history remain under their existing owners.

## Earlier execution snapshot

The sections below retain the earlier implementation record, before publication and later repairs. Their candidate numbers and source pins refer to that earlier work. Their statements about absent commits, release checks, or baseline approval apply only to that snapshot. The completed-stage results above supersede those status statements.

The original implementation parent is `41eee1d61978d62cfaf9d0d9c242b9fee590c20e`. The branch is `work/compressed-blockack-s3b`. The working copies of replacement plan revision 39 and lifecycle plan revision 29 define that scope. The user authorized S3b execution on 15 October 2026.

## Implementation contract

[HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) owns agreement coordination and frame exchange order. The [originator](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.cc) and [recipient](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.cc) handlers own their agreements and transient procedures. The MIB owns capability state. Management supplies an explicit peer reset before association replacement. The [recipient data service](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.cc) owns duplicate checks and delivery. Queue components own extraction effects.

The implementation retains Basic compatibility without an HT admission requirement. HCF continues to reject Compressed selection during initialization. Isolated policies verify Compressed admission with explicit capability inputs. S3b does not implement S4 BAR arbitration, A5 transfer completion, or Compressed exchanges.

1. A1 returns complete accepted Block Ack Actions from the recipient data service. HCF dispatches each Action once after duplicate checks. A correlated Action resolves its procedure before obsolete exchange failure.
2. A2 passes the selected profile to admission policies. The handlers correlate peer, TID, and dialog token. Accepted response values determine both agreement owners' parameters. The data service enforces both A-MSDU flags.
3. A3a detaches the selected agreement before cleanup and deletion notification. Ordinary recipient teardown delivers complete buffered units in cyclic order. It discards incomplete units. HCF uses one absolute inactivity deadline across both roles.
4. A3b clears both roles and obsolete peer work before deletion notifications. Management supplies the old peer identity before association replacement. The MIB keeps its boolean notification and coherent committed state.
5. A3c routes Action disposal to its owner. HCF invalidates obsolete queued and in-progress Actions. An accepted transmission retains its context until safe completion. Valid DATA or BAR without an agreement causes recipient-role DELBA.
6. A4 defers extraction during setup and recovery. The data service supplies a non-consuming sequence query. Optional selected extraction preserves queue ownership, residence metadata, and pull events. Contention uses the same eligibility condition.

HCF obtains both roles' earliest active absolute deadlines after each agreement mutation. It schedules their minimum. It cancels the timer when neither role has a finite deadline. Expiration detaches every overdue agreement before the next deadline calculation.

The delayed-successor interpretation preserves the existing setup trigger across separate DATA arrivals. A completed eligible Normal Ack transmission supplies the first condition. A later unexpired unnumbered successor supplies the second condition. The handler retains the first condition in its procedure map without an Action or timer. HCF promotes that state to setup intent before successor extraction. A single frame without a successor therefore causes no ADDBA.

The implementation checks peer, TID, direction, modulo-4096 sequence order, and absolute versus relative time at each changed boundary. Duplicate Actions retain immediate ACK behavior. Invalid responses cannot resolve setup. Queue disposal, retry failure, response timeout, inactivity, received DELBA, and association reset have separate terminal paths. No MAC stop/start recovery claim applies.

## Source assessment

The parent recipient data service deletes Actions before it returns its accepted frames. HCF nevertheless dispatches the original management header after that call. Thus, duplicate detection does not control agreement dispatch. During a response wait, HCF passes an addressed Action to the active receive step instead of its agreement handler.

The parent policies accept unchecked setup parameters. The recipient stores request size and timeout before it builds a response. The response omits the request dialog token. The originator policy asserts that an agreement exists but performs no response correlation.

Both handlers supply absolute inactivity deadlines. HCF passes those deadlines to `rescheduleAfter`. The handlers select expiration with equality instead of an overdue comparison. Recipient DATA reception does not refresh the stored deadline. Received recipient DELBA uses the local receiver address for removal.

The parent collision controller tests frame counts. This test does not establish agreement eligibility. A4 requires a production reproduction or a cancellation trace before a shared arbitration change.

## Verification status

The separate lifecycle review accepted the A1–A4 scope against the pinned parent. It required the shared inactivity minimum and explicit peer reset before equal-state reassociation. The review supplied source evidence; it supplied no implementation or runtime verdict.

The parent debug build passed with `make MODE=debug -j12`. The command ran from the repository root and returned zero. Its log is `/tmp/s3b-parent-build.log`.

The parent reproduction command was `inet_run_module_tests -m debug -f 'Ieee80211BlockAck(ActionDelivery|ActionDispatch|Negotiation)_1.test'`. Its log is `/tmp/s3b-parent-reproductions.log`. The Action delivery fixture failed its returned-frame count assertion. The negotiation fixture failed its dialog-token assertion. Both failures reached the production component after test initialization.

The HCF dispatch fixture initially lacked a received-mode indication and a valid request sequence number. Those fixture setup errors supplied no product evidence. After correction, `inet_run_module_tests -m debug -f 'Ieee80211BlockAckActionDispatch_1.test'` reproduced duplicate dispatch. The fixture required one dispatch after an original request and its MAC retry. The parent dispatched both. Its log is `/tmp/s3b-parent-dispatch.log`.

The generated dispatch executable also ran independently with `-u Cmdenv -r 1` from `tests/module/work/Ieee80211BlockAckActionDispatch_1`. At 1 ms, the fixture supplied a 5 ms absolute deadline. The parent scheduled 6 ms. The arrival-time assertion failed with exit status one. Its log is `/tmp/s3b-parent-inactivity.log`.

New fixtures cover directional cleanup, association reset, response-wait order, deferred extraction accounting, and contention. Existing Basic recipient, selector, header, queue-disposal, lifetime, capability, and exchange fixtures remain controls. The candidate results below identify their commands and evidence limits.

The parent control command selected nine module fixtures and passed all nine. Its command was `inet_run_module_tests -m debug -f 'Ieee80211(BlockAck(Reordering|Variant)|HcfHeader|RecipientNav|HcfMultipleProtection_2|SingleProtectionGroup|MgmtApHcfQueueDrop|MgmtRatePublication|MgmtStateCommit|EdcaDataLifetimeArbitration)_.*test'`. Its log is `/tmp/s3b-parent-controls.log`. This expression did not select the multiple-protection fixture because that alternative already contained its suffix.

A second command selected that fixture and the Basic exchange control: `inet_run_module_tests -m debug -f 'Ieee80211(HcfMultipleProtection_2|BasicCompatibility_1)\.test'`. Both fixtures passed. Its log is `/tmp/s3b-parent-basic-controls.log`. Together, these results cover eleven fixtures, including all eight S1 recipient scenarios.

The parent library hash was `ed6af79596a80e6192f4ca1cb8bf91f8a5f682d508eef58e95603796aa384a6e`. The lifecycle and response-wait fixtures also reproduced nine parent failures with that library. Directional teardown emitted a null deletion payload. Absent-agreement DATA reached upper delivery. Absent-agreement BAR caused no DELBA. An incoming Action received no immediate ACK during an active request ACK wait. `/tmp/s3b-regression-parent/` retains the exact executables, inputs, hashes, and individual run logs. Later fixture changes require fresh compilation before candidate verification.

The selected protocol command was `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '(N_BlockAck|N_CompressedBa|N_Amsdu|Ac_BlockAck|WifiQosDelayedBa|Ac_MultiTidBa|N_Ampdu|Ac_Ampdu)\.test$'`. It returned zero with four passes and four declared expected failures. Its log is `/tmp/s3b-parent-protocol.log`. This run used the unchanged parent library while source implementation began. The protocol support library includes no changed agreement or HCF declaration. A new unused timeout parameter declaration was present; this result does not establish candidate behavior.

The delayed-policy fixture failed its immediate-ACK assertion after BAR. The Multi-TID fixture missed its response deadline. Both A-MPDU fixtures missed their subframe-header observation. `/tmp/s3b-parent-reproductions/protocol-failure-causes.json` preserves these first failure causes. These outcomes remain unresolved protocol obligations.

## Additional production reproductions

Candidate 6 exposed two A4 failures before the final contention change. A real VO transmission held an ADDBA Request in its queue. A later VO arrival evicted that Request. HCF cleared setup, but surviving BE DATA obtained no new contention request. Action-disposal case 6 reproduced this failure without another BE arrival. `/tmp/s3b-actiondisposal-build6-reproduction/` retains the evidence.

A separate fixture expired a VI agreement at 33 microseconds. Its queued DATA became ineligible before coincident VI and BE grants at 34 microseconds. VO had its next grant at 70 microseconds. The controller still counted VI and charged BE a collision retry. With retry limit one, BE discarded its valid DATA. `/tmp/s3b-deferred-collision-before-isolated.log` records this isolated failure.

The first contention trace also contained a legitimate VO grant at 34 microseconds. That trace supplies no evidence for the shared correction. The corrected fixture changes only VO AIFSN to six. Its eligible-VI control retains the real collision and retry disposal. Both corrected cases use seed zero and the same binary and library.

HCF now supplies each EDCAF with a query for current frame eligibility. The collision controller uses this query before it selects contenders. The query performs no expiration, extraction, or protocol mutation. HCF retains the existing Basic readiness condition; S4 still owns BAR-only work. After an Action terminal outcome, HCF requests contention for every category with eligible work.

Candidate 7 passes both corrected contention cases. The deferred-VI case transmits BE DATA without a collision or retry disposal. The eligible-VI control retains its collision and BE retry disposal. Late Request eviction and successful DELBA acknowledgment also release BE DATA without another BE arrival. `/tmp/s3b-deferral-selection-collision-final.log` and `/tmp/s3b-regression-build7-pass/` retain these results.

Candidate 7 also reproduces an Action recovery-state defect. An owned ADDBA Request causes a real RTS transmission under a low RTS threshold. The missing CTS charges its short retry counter before any Action transmission. Peer reset then leaves that counter at one because Action-attempt state remains false. `/tmp/s3b-actiondisposal-build7-rts-reproduction/` retains the exact failure. Cleanup must distinguish an untouched Action from an Action whose RTS exchange owns retry state.

The retained `MacQosWithBlockAck` demonstration exposes a separate setup trigger defect. A-MSDU aggregation drains all queued DATA for each AP transmission. The next same-TID arrival occurs after that transmission, so the immediate successor check never starts AP setup. Candidate 7 therefore supplies only the client-to-AP Basic link. The required AP-to-server link remains absent. The delayed-successor state addresses this condition without an empty setup request or changed traffic.

## Controlled setup measurements

The deferral fixture compares response timeouts of 100 ms and 250 ms under equal traffic, rates, seed, and response queue residence. It holds the real recipient response in its queue for approximately 150 ms. It does not inject a response directly. Both cases use three DATA packets and seed zero. DATA uses 6 Mbps; Actions use 24 Mbps.

| Observed interval or count | 100 ms timeout | 250 ms timeout |
| --- | --- | --- |
| Request queue admission to PHY start | 34 microseconds | 34 microseconds |
| Request ACK reception | 0.000684020016 s | 0.000684020016 s |
| Request ACK to accepted response | No accepted response | 150.070003336 ms |
| Response queue residence | 100.080006672 ms before invalidation | 150.078003336 ms before transmission |
| DELBA queue admission to ACK completion | 80.006672 microseconds | No DELBA |
| Surviving DATA PHY start | 0.100780026688 s | 0.150832023352 s |
| Recipient MAC receives surviving DATA | 0.100980030024 s | 0.151032026688 s |
| Transmitted management frames | One Request and one DELBA | One Request and one Response |
| Management airtime | 72 microseconds | 72 microseconds |
| ACK airtime for management exchanges | 56 microseconds | 56 microseconds |

The 100 ms deadline starts recovery before the queued response becomes eligible. The recipient invalidates that response when DELBA arrives. DELBA acknowledgment releases surviving DATA under Normal Ack. The 250 ms deadline permits the delayed response and releases DATA under Block Ack. That final DATA has no BAR within the fixture, so MAC reception does not establish upper delivery.

`/tmp/s3b-build11-timing-100ms.log` and `/tmp/s3b-build11-timing-250ms.log` contain the exact events and PHY durations. `/tmp/s3b-build11-setup-timing.json` preserves source lines and event numbers. The fixture does not independently timestamp the transition into setup deferral. These observations establish timeout order and separate exchange costs; they establish no delivery guarantee or preferred timeout.

Application measurements use the unchanged Block Ack showcase, General configuration, run zero, seed zero, and a fixed 0.5-second interval. The source sends 5,000 application packets of 700 bytes at 0.1 ms intervals. The debug parent delivers 330 packets, or 231,000 useful bytes. Final debug candidate 11 delivers 324 packets, or 226,800 useful bytes. Application goodput is therefore 3.696 Mbps and 3.6288 Mbps, respectively.

Both application runs use `inet --debug -u Cmdenv -c General -r 0 --seed-set=0 --sim-time-limit=0.5s --record-vector-results=false --record-scalar-results=true --cmdenv-express-mode=true`. Their result directories are `/tmp/s3b-metric-parent-debug` and `/tmp/s3b-metric-candidate-11`. `opp_scavetool export -F CSV-R` exports both scalar files into `/tmp/s3b-metric-results-final.csv`. Useful bytes come from the destination application's `packetReceived:sum(packetBytes)` scalar. Earlier runs without an explicit mode supply no matched-mode comparison.

The comparison parent checkout is `9b4b22258fff87401943d5046e0d18c18b70a558`. Its source and feature declarations equal the implementation parent. Its ignored build profile differs in unrelated optional features. This one fixed-seed comparison establishes only the observed application counts. It establishes no general performance improvement, optional-retry benefit, or agreement-reuse benefit. S4 retains transfer-completion and idle-gap comparisons.

## Final source and focused verification

Final candidate 11 contains A1, A2, A3a, A3b, A3c, and A4. HCF retains Basic as the default and rejects Compressed during initialization. The source review found no remaining correctness defect or unresolved architecture exception. No commit, publication, recorded baseline change, or seal change forms part of this execution.

The final debug build command is `make MODE=debug -j12`. It returns zero; `/tmp/s3b-candidate-build-11.log` records the build. The library SHA256 is `f1ec379bd582e9c84b87e548059068231f9381d0dbd67bc6c4908f9e174e4608`. `/tmp/s3b-final-source-manifest.json` identifies all 72 changed source, test, and migration files, including untracked headers. Its parent remains `41eee1d61978d62cfaf9d0d9c242b9fee590c20e`.

The final implementation separates Action transmission from ownership of RTS retry state. An RTS failure records ownership of the short retry counter. It does not mark the Action as transmitted. Invalidation and every terminal Action outcome clear that owned counter. A separate control preserves another peer's counter when an untouched Action uses the same Sequence Control field. The existing shared recovery map still uses that field without the peer address; this change does not redesign that map.

The delayed-successor cases now defer later individual DATA and later A-MSDU batches before extraction. Both negotiate starting sequence number one and transmit first DATA with number one. The single-frame controls send no ADDBA. Real AP reassociation removes the waiting state under both legacy and HT configurations. A lone frame in the new association cannot inherit the old trigger.

| Final debug selection | Result | Evidence |
| --- | --- | --- |
| Related module fixtures and existing controls | 29 fixtures pass | `/tmp/s3b-final-modules-6.log` |
| Lifecycle, response wait, association reset, Action disposal, retired DATA | Five fixtures; 50 scenarios pass | `/tmp/s3b-regression-build11-runner.log`, `/tmp/s3b-regression-build11-pass/` |
| Deferred selection, setup, lifetime, collision controls | Three fixtures; 18 scenarios pass | `/tmp/s3b-build11-deferral-selection-collision.log`, `/tmp/s3b-build11-*` |
| Related unit fixtures | Six fixtures pass | `/tmp/s3b-final-unit-4.log` |
| Retained Basic demonstrations | 13 cases pass | `/tmp/s3b-final-demonstrations-11/evidence.json` |
| Selected protocol requirements | Four pass; four declared expected failures | `/tmp/s3b-final-protocol-4.log` |

The module selections total 37 distinct fixtures. The lifecycle and deferral snapshots retain generated code, configuration, executables, output, and hashes. Their simulation outputs contain no runtime warnings, errors, or undisposed objects. Runner notices about optional Python packages do not describe simulation failures. All cases use debug mode; deterministic new scenarios use seed zero.

The [migration guide](../../doc/src/migration-guide/index.rst) explains the changed custom-module contracts. The [selected-extraction contract](../../src/inet/queueing/contract/ISelectivePacketCollection.h) defines queue ownership and pull effects. The [Action disposal fixture](../../tests/module/Ieee80211BlockAckActionDisposal_1.test) covers queue loss and RTS cleanup. The [deferral fixture](../../tests/module/Ieee80211BlockAckDeferral_1.test) covers timeout order and delayed successors. The [collision fixture](../../tests/module/Ieee80211BlockAckDeferredCollision_1.test) separates deferred contention from a genuine collision.

The exact root module selector is:

```sh
inet_run_module_tests -m debug -f '(Ieee80211BlockAck(ActionDelivery|ActionDispatch|Admission|Negotiation)_1|PacketSelectiveExtraction_1|Ieee80211(BasicCompatibility_1|BlockAckVariant_1|BlockAckReordering_1|HcfHeader_1|HcfMultipleProtection_[12]|SingleProtectionGroup_1|RecipientNav_1|PreparedCancellation_1|Txnav_1|HcfSingleProtection_1|VhtHtSupport_1|MgmtApHcfQueueDrop_1|MgmtRatePublication_1|MgmtStateCommit_1|EdcaDataLifetime.*))\.test$'
```

The additional module and unit selectors are:

```sh
inet_run_module_tests -m debug -f 'Ieee80211BlockAck(Lifecycle|ResponseWait|AssociationReset|ActionDisposal|RetiredData)_1.test'
inet_run_module_tests -m debug -f 'Ieee80211BlockAck(DataSelection|Deferral|DeferredCollision)_1\.test'
inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet_1|VhtHtCapabilities_1|ResponseModeSelection_1|MibPeerTeardown_1|QosDuplicateRemoval_1|BlockAckActionWire_1)\.test$'
```

All category commands run from the repository root. The Basic observer command is `python3 tests/module/basic-compatibility-evidence.py --output /tmp/s3b-final-demonstrations-11`. It retains the original topologies, traffic, aggregation settings, fragmentation cases, parameter studies, and Basic assertions. The AP-to-server link now supplies real setup, DATA, BAR, and positive receipt bits. This result closes the two-hop regression without a weaker assertion.

The final protocol command repeats the eight-case parent selector exactly. Its runner returns zero with four expected failures. Delayed-policy verification now fails at step one because admission refuses delayed policy. Its parent first failed at the later immediate-ACK requirement. Multi-TID still misses step two; both A-MPDU cases still miss the step-zero subframe-header requirement.

`/tmp/s3b-final-protocol-failure-causes.json` preserves those first failure causes. Their expected-failure markers remain unchanged. These results establish no delayed, Multi-TID, A-MPDU, or Compressed support. The passing `N_CompressedBa` fixture still exercises the explicit Basic compatibility path in this stage.

Scoped architecture checks pass for `src/inet/linklayer/ieee80211` and `src/inet/queueing`. Their logs are `/tmp/s3b-final-architecture-wifi.log` and `/tmp/s3b-final-architecture-queue.log`. `check-source-seals.sh --diff` passes; `/tmp/s3b-final-source-seals.log` records the result. Every changed source target is unsealed. `git diff --check` also passes for the owned changes.

The independent gate comparison uses the actual implementation parent. Wi-Fi naming retains the same 32 findings; queue naming retains the same one finding. Wi-Fi interfaces retain the two existing findings under `AV-CONTRACT-02`; queue interfaces pass. New compliant interfaces increase those counts from 48 to 49 and from 24 to 25. `/tmp/s3b-review-{parent,candidate}-{naming,interfaces}-{0,1}.log` retains both sides; zero denotes Wi-Fi and one denotes queueing. No new mechanical finding or exception-ledger change is required.

The C++ checker could not execute its analysis for either subtree. Both `check-cpp.sh` calls return two because this checkout has no `compile_commands.json`. `/tmp/s3b-final-cpp-wifi.log` and `/tmp/s3b-final-cpp-queue.log` retain that prerequisite error. These calls supply no clang-tidy verdict. This execution supplies debug compilation and source review; release compilation and the full pre-push gates remain outside this unpublished work.

## Queue traversal cost

Candidate 9 exceeded the CPU limit in both ten-second ad hoc fingerprint cases. A bounded Block Ack-disabled comparison reached the same event 104692 at 0.8 seconds. The parent used 3.56 CPU seconds; candidate 9 used 7.56 CPU seconds. Source inspection identified a new membership check that continued after it found each aggregation candidate.

An external counter measured indexed queue reads without a repository edit. The captured production stack reached that membership check through the normal QoS extraction path. Each indexed read starts at the linked-list head. The full scan therefore repeated increasingly long walks for every candidate. This cost also affected traffic with Block Ack disabled.

| Same-event profile | Parent | Candidate 9 | Candidate 10 |
| --- | --- | --- | --- |
| Indexed queue reads | 861 | 861,869 | 2,605 |
| Sum of requested indices | 1,285 | 775,865,982 | 2,142 |
| Largest requested index | 3 | 3,114 | 3 |

Candidate 10 stops each membership search at its first match. The service still checks every candidate's presence, uniqueness, eligibility, and A-MSDU permission before any source removal. The repair adds no state or interface. The final profile retains event 104692 and removes high-index traversal from this run. Its plain run uses 4.24 CPU seconds; concurrent host load limits interpretation of that timing.

`/tmp/s3b-baoff-profile-parent.log`, `/tmp/s3b-baoff-profile-candidate.log`, and `/tmp/s3b-baoff-profile-candidate10.log` retain the profiles. `/tmp/s3b-queue-profile.cc` supplies the external counter. The source review accepts the bounded early-break correction. Candidate 10 passed its focused checks after that correction. Candidate 11 retains the same early break.

The Block Ack-enabled profile then found another repeated indexed scan in HCF's DATA existence query. At 0.8 seconds and event 102123, candidate 10 made 7,894,493 indexed reads. Candidate 11 makes 12,236 reads at the same event. Time inside high-index traversal falls from 14.37 seconds to 0.061 seconds. `/tmp/s3b-baon-profile-candidate10.log` and `/tmp/s3b-baon-profile-candidate11.log` retain these profiles.

HCF now uses the optional predicate query to find unexpired DATA for a peer and TID. PacketQueue traverses its linked queue with an iterator. A composite reports `UnsupportedPacketSelection` when its provider cannot supply that query. HCF catches only that outcome and retains its collection fallback. Unrelated queue or predicate errors still propagate. This adds no capability cache or protocol state.

The data service first checks the ordinary advertised head at both selection points. An eligible head therefore needs no selected-extraction capability. If all frames defer, the service waits without mutation. If a deferred head hides eligible work, the service requires selected extraction before it changes ownership or sequence state. The three new composite cases verify these distinctions through a provider without the optional interface.

## Accepted exchanges across reset

The final reset checks also repair physical statistics and RTS-protected identity. Candidate 10 omits EDCAF sent or received observations after logical retirement, although accepted DATA and its ACK complete physically. The final HCF emits each physical observation once before the retired-state guard. That guard still suppresses obsolete acknowledgment, recovery, agreement, and rate effects. The three DATA completion cases verify one sent observation and one received ACK observation.

A separate reproduction resets the peer after RTS acceptance and before DATA submission. The old DATA and its lifetime leave active ownership. After CTS, candidate 10 nevertheless accepts that retired DATA because it checks no active-frame membership. Its management sequences instead select again, which can produce null or another peer's frame. `/tmp/s3b-retired-rts-build10-reproduction/` retains six clean failure cases.

Candidate 11 retains the original protected management pointer after CTS, as the DATA sequence already does. HCF refuses DATA or management submission when that packet no longer belongs to active in-progress work. The accepted RTS exchange can finish safely, but it cannot authorize a retired packet or substitute another peer's packet. All six reset-during-RTS cases pass with no post-reset protected submission. They also preserve the other peer's DATA lifetime.

The independent review closes every reported source finding in candidate 11. Its general checklist reports 20 passes and 14 inapplicable items. Its WLAN checklist reports 12 passes and one inapplicable item. Neither checklist retains a flag or question. The review adds no exception-ledger or seal change.

## Final fingerprint comparison

All 19 final debug cases complete without a CPU-limit error. The repository expectations yield one pass and 18 mismatches. The actual parent yields four passes and 15 graphical-only mismatches. Twelve final cases match every calculated parent ingredient. Seven Basic cases change functional ingredients; the table below identifies them.

| Case | Calculated `tplx` | Calculated `~tNl` | Calculated `~tND` | Calculated `tyf` |
| --- | --- | --- | --- | --- |
| Ad hoc `MacQos`, run 1 | `5fbb-9fd9` | `c0a4-97f6` | `2373-df01` | `49b6-dbab` |
| Infrastructure `MacQosWithBlockAck`, run 0 | `f8a4-9474` | `1cba-cccc` | `538e-67df` | `69e0-5af3` |
| Block Ack `Fragmentation`, run 0 | `021a-d29a` | `1597-ec6a` | `feff-7e88` | Not selected |
| Block Ack `MixedTraffic`, run 0 | `5b9e-801f` | `8731-2504` | `9724-d1ce` | Not selected |
| Block Ack `NoFragmentation`, run 0 | `b828-6bd6` | `026f-2c6d` | `2cd4-4795` | Not selected |
| Fragmentation `HCFfragblockack`, run 0 | `f509-65f1` | `6b3a-0d65` | `c474-7174` | `4cfb-b3c7` |
| TXOP `General`, run 0 | `f58d-d2fd` | `38f7-f642` | Not selected | `e520-9a34` |

Paired production traces confirm the first changed exchanges. In `NoFragmentation`, both versions transmit DATA 0 at 0.210 ms and complete it at 0.496 ms. Both also number DATA 1 through the existing successor forecast. The parent immediately requests starting number one. The candidate first resolves numbered DATA 1 under Normal Ack, then requests starting number two. This is A4's required drain-before-setup rule.

The TXOP trace likewise changes the request number from two to three after numbered DATA resolves. The fragmentation trace completes the numbered fragmented unit before ADDBA. The parent starts setup after fragment zero. The ad hoc trace changes the request number from two to three after numbered Voice fragments resolve. The infrastructure trace suppresses empty-successor setup after ARP and starts setup when later WWW DATA arrives. The retained demonstrations still show successful Basic exchanges, positive receipt bits, and fragment bits where required.

`/tmp/s3b-fp-first-divergences.json` and `/tmp/s3b-fp-qos-first-divergences.json` retain times, events, commands, fields, and source lines. These paired windows establish the changed setup boundaries. They do not attribute every later counter difference to one mechanism. The final 19 ingredients equal candidate 10 exactly, so the final cost and reset repairs add no further fingerprint change.

The parent command ran from its own `tests/fingerprint` directory after its `setenv` file. It used debug mode, four threads, and the same 19-case selector. `/tmp/s3b-parent-fingerprints-2.log` retains its complete result. `/tmp/s3b-parent-fingerprint-inventory.json` preserves its calculated ingredients before later jobs overwrote shared result paths. Interrupted invocations and overwritten shared outputs supply no comparison evidence.

The final candidate uses private copies of the unchanged input CSV files in `/tmp/s3b-final-fingerprint-inputs-11`. `input-hashes.json` records their byte hashes. Older jobs cannot replace these comparison inputs or outputs. The exact command runs from `tests/fingerprint`:

```sh
PYTHONUNBUFFERED=1 ./fingerprinttest -d -t 4 -m 'showcases/wireless/(blockack|aggregation|fragmentation|qos|txop)/|examples/wireless/qos/.*MacQos|examples/adhoc/qos/.*MacQos' -l /tmp/s3b-final-fingerprint-inputs-11/runner.log /tmp/s3b-final-fingerprint-inputs-11/examples.csv /tmp/s3b-final-fingerprint-inputs-11/showcases.csv
```

`/tmp/s3b-candidate-fingerprints-6.log` records all 19 results and the nonzero mismatch status. `/tmp/s3b-fingerprint-comparison-build11.json` records the parent and candidate 10 comparisons. `/tmp/s3b-final-verification.md` supplies the regression analysis. The seven calculated rows above are evidence, not approved baseline values. Every recorded repository expectation remains unchanged.

The separate capability observer also passes all eight inventory cases on candidate 11. Its command is `python3 tests/module/basic-compatibility-evidence.py --protocol-inventory --output /tmp/s3b-final-protocol-inventory-11`. The evidence file records actual HT state independently of the protocol assertions. Passing inventory observations do not convert the four expected protocol failures into supported features.
