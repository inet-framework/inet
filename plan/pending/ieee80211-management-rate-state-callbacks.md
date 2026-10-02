# Implementation plan: IEEE 802.11 management callbacks and rate state

Checkout: `inet-ieee80211-txop-boundaries-rate-selection`.
Branch: `pr/ieee80211-rate-state-boundaries`.
Source revision: `e716d0d7858b1df74477732ff593e4a0f19b15f7`.
Plan revision: 2, 2026-10-02.
Scope: assess the three comments, prepare the plan, and execute the approved plan.
Implementation status: complete; this plan accompanies the implementation commit.
Authorization: the user instructed, "Execute the plan".
Commit authorization: the user instructed, "git commit".

The checkout contains a modified `AGENTS.md` and an untracked plan under `plan/done/`.
Those files are outside this change.
The repository has no `CONTEXT.md`.

## 1. Assessment and intended behavior

All three comments identify defects in the current code.
The evidence below comes from source inspection.
No runtime reproduction accompanies this assessment.

An access point (AP) manages station records.
A station (STA) manages AP records.
The management information base (MIB) stores accepted rates and peer capabilities.
High throughput (HT) denotes the IEEE 802.11 HT capability state.
A basic service set (BSS) is the network whose accepted rate policy applies to a station.
A callback is a listener call that runs before the signal emitter returns.

| Comment | Assessment | Required behavior |
| --- | --- | --- |
| AP teardown accesses erased station records | Valid. `releaseAssociationId()` emits callbacks before the handler finishes with `sta`. | Teardown uses stable values. A canceled handler sends no later outcome or response. |
| Association callback uses erased AP record | Valid. STA completion retains `ap` across MIB callbacks and the association signal. | Completion uses a local snapshot. Stop or replacement cancels its remaining work. |
| Association exposes mixed peer rate state | Valid. AP completion publishes HT state before it installs accepted peer rates. | One MIB commit installs the complete accepted state before its first notification. |

The first defect also exists inside `releaseAssociationId()`.
Its `address` argument is a reference to `sta->address` at these callers.
The first removal can erase that record through a callback.
The second removal then uses the invalid reference.
A handler-only copy does not protect other callers of this public method.

The second defect extends beyond the final rate installation.
Earlier HT updates, old-association removal, and `htNegotiationFailedSignal` also invoke callbacks.
The association signal itself can stop management before the handler creates its beacon timer.
These boundaries need the same protection.

The third defect has a concrete rate-selection consumer.
`RateSelectionBase::isAllowedByRateState()` applies the peer restriction only when a known peer rate entry exists.
An absent entry omits that restriction.
The exact incorrect rate depends on the local and BSS policies.
The proposed test will make that choice deterministic.

## 2. Current implementation and evidence

| Production path | Evidence |
| --- | --- |
| AP authentication, deauthentication, and disassociation | [Ieee80211MgmtAp.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.cc), `handleAuthenticationFrame()`, `handleDeauthenticationFrame()`, and `handleDisassociationFrame()` reuse `sta` after MIB calls. |
| AP teardown callbacks | [Ieee80211Mib.cc](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.cc), `releaseAssociationId()` calls two methods that each emit a rate signal. |
| AP stop | `Ieee80211MgmtAp::stop()` clears `staList` before MIB cleanup. |
| AP accepted response | `frameTransmissionFinished()` installs HT state before `installBssAndPeerRateSets()`. It already checks `completedAssociationTransactionId`. |
| AP acknowledged refusal | `frameTransmissionFinished()` calls `sendDisAssocNotification()` after `releaseAssociationId()` without a lifecycle check. |
| STA response completion | [Ieee80211MgmtSta.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc), `processAssociationResponse()` retains an AP-list pointer across callbacks. |
| STA stop and cleanup | `stop()` calls `clearAPList()`. `clearCurrentAssociation()` emits callbacks before it clears the timer and `assocAP`. |
| STA list cleanup | `clearAPList()` iterates `apList` across peer-rate notifications. A nested stop can clear that list. |
| Rate selection | [RateSelectionBase.cc](../../src/inet/linklayer/ieee80211/mac/rateselection/RateSelectionBase.cc), `isAllowedByRateState()`. |
| Association signal consumer | [Ieee80211VisualizerBase.cc](../../src/inet/visualizer/base/Ieee80211VisualizerBase.cc) casts signal details to `Ieee80211MgmtSta::ApInfo`. |

Both AP request handlers set `pendingHtStateAvailable` before they send their association response.
The fix must retain the existing guard for delayed or replaced response transactions.

## 3. Scope and acceptance criteria

The change covers the three reported defects and the cleanup methods that these paths call.
It also covers acknowledged AP refusal, because that path resumes after the same teardown callback.
It does not change admission policy, frame formats, HT validation, or the response acknowledgment requirement.
Other discovery and beacon-update paths remain a separate follow-up unless a changed shared contract requires an edit.

The change is complete when these conditions hold:

1. No affected handler or MIB method uses an erased record after a callback.
2. Stop, crash, or a replacement transition invalidates the old handler before its next side effect.
3. An immediate restart with the same address does not restore the old handler's authority.
4. Each association commit exposes accepted HT state, peer rates, and BSS rates together.
5. Each changed commit emits one `rateStateChanged` signal. An equal update emits none.
6. Ordinary association and teardown retain their expected outcome signals and primitive confirmations.
7. A canceled STA completion creates no orphan beacon timer or stale confirmation.
8. A peer teardown preserves another peer's state and the AP's BSS rate policy.
9. A stopped module retains no transient peer state, list entry, reservation, or timer from the canceled operation.
10. Signal details remain valid for every listener in the current signal delivery.

## 4. Proposed design

### MIB commit boundary

Management retains protocol decisions and transaction ownership.
The MIB retains shared rates and capabilities.
Extend the existing combined rate installation with a typed operation that also installs or clears peer HT state.
Reuse the existing rate validation, HT negotiation, equality checks, and generation rules.

The operation must perform these steps:

1. Copy any input that aliases a MIB entry or management record.
2. Validate both rate sets and the requested HT update before any state mutation.
3. Prepare the negotiated HT state before any state mutation.
4. Commit BSS rates, peer rates, and the explicit HT install-or-clear decision.
5. Emit one rate signal if the observable state changed.
6. Return without further state mutation after the signal.

Keep admission and HT usability decisions in management.
The new MIB method must not choose a fallback policy.
Keep existing individual setters for their current callers.
Use private mutation helpers to share logic without intermediate signals.
Do not add a general notification-suppression flag or a public begin/end transaction API.
Those mechanisms permit callers to expose an incomplete state if they omit an end operation.

Apply the same commit pattern to peer removal.
`releaseAssociationId()` must copy the address before any callback.
It must erase reservations, committed IDs, HT state, and peer rates before its single notification.
Remove redundant peer removals from associated AP teardown.
Retain complete peer cleanup when the station has no committed association ID.

Provide a complete current-BSS cleanup operation for STA teardown.
Detach the local association and its timers before that operation emits a signal.
For stop, consolidate transient MIB cleanup into one final commit.
Include the inherited cleanup in [Ieee80211MgmtBase.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.cc).
This prevents an old stop continuation from clearing state that a callback creates after restart.
Preserve prepared local capabilities, local rates, and the existing cached SSID/BSSID behavior.

### AP handler identity and lifetime

A lifecycle identity is a counter that distinguishes management before and after stop.
A transition identity distinguishes two operations for the same peer within one lifecycle.
Neither identity resets on restart.

Add a lifecycle identity to AP management.
Advance it at the start of `stop()`, before any cleanup callback.
Give each affected peer transition a fresh identity from a monotonic sequence.
Store the current transition identity in `StaInfo`.
An operation for another peer must not invalidate this peer's transition.

Copy the peer address and required state before the first callback.
Complete local status changes and pending-response cancellation before MIB publication.
After each callback, check the lifecycle identity and the peer transition identity.
Find the current record by address only after those checks.
Do not rely on pointer equality, address equality, or `isUp()` alone.
Stop followed by restart can reproduce all three values.

Cover both authentication cleanup branches, deauthentication, disassociation, and acknowledged refusal.
Authentication must also check its identity before it sends another response or updates `authSeqExpected`.
Each early return must delete or transfer the input packet exactly once.

For successful AP completion, retain the existing response transaction check.
Replace separate HT and rate setters with the combined MIB commit.
Check the lifecycle and completed response identity after that commit.
Only the current completion can emit `l2ApAssociatedSignal`.

### STA completion identity and snapshot

Add lifecycle and association transaction identities to detailed STA management.
Create a fresh transaction identity for each association or reassociation attempt.
Preserve a completion identity when the accepted response cancels its timeout.
Invalidate it on cancellation, replacement, current-association teardown, scan reset, and stop.
A new attempt for the same AP receives a different identity.

Copy the selected `ApInfo` before the first callback in response completion.
Use the local copy for the address, SSID, beacon interval, and signal details.
Set its `authTimeoutMsg` to `nullptr`; the snapshot owns no timer.
Keep the current `ApInfo` signal type for the visualizer.
Declare the snapshot as immutable borrowed data for the duration of signal delivery.
Do not pass the mutable `assocAP` member as signal details.
A callback can clear that member before another listener reads it.

Preserve the existing virtual confirmation signatures.
Pass the local `ApInfo` snapshot to those methods after the identity check.
Their base implementations need only its address.
This avoids an unnecessary signature change for external subclasses.

Finish the accepted association snapshot and install its beacon timer before the first completion notification.
Use the combined MIB commit for HT and rates.
Check the identity after every MIB or signal callback, including `htNegotiationFailedSignal` and `l2AssociatedSignal`.
Send the primitive confirmation only while the completion remains current.
A stop listener must cancel the already-owned timer through ordinary teardown.

Make old-association cleanup and refusal paths obey the same rules.
Detach list records and their timers before list cleanup emits notifications.
Preserve a pending transaction for a different AP where the current code requires it.
Do not let an old cleanup continuation erase a replacement association.

### Reasons and compatibility

The existing AP response identity already protects successful response completion.
Reuse that check there.
It does not identify authentication or teardown, so those paths need a peer transition identity.
The STA timeout pointer also cannot identify a completed transaction after its timeout is canceled.
Each new counter therefore has a specific management consumer.

The plan follows [state ownership](../../doc/project/design/ieee80211-model-architecture.md) and
[notification contracts](../../doc/project/rule/architecture.md#ar-com-notify).
The reuse choice follows [AR-EXT-REUSE](../../doc/project/rule/architecture.md#ar-ext-reuse).
Snapshot lifetime follows [QR-OBJECT-OWNERSHIP](../../doc/project/rule/quality.md#qr-object-ownership).
New helpers use the smallest visibility that their callers require.

Keep the existing signal name and Boolean payload.
Update its comments in `Ieee80211Mib.h` and `Ieee80211Mib.ned` to describe the complete commit boundary.
The intentional compatibility change is fewer rate notifications for one logical transition.
An independent listener must query the complete state, rather than count intermediate removals.
Successful outcomes can still occur when an equal MIB update emits no rate signal.
The plan introduces no new NED parameter or frame field.

## 5. Implementation sequence

Pre-write implementation contract:

- Invariant and owner: management owns operation identities and timers. The MIB publishes complete committed peer and BSS state.
- Entry and control path: AP frame handlers and response transmission completion call the MIB. STA response handlers call the MIB and emit association outcomes.
- Consumers and artifacts: AP/STA/MIB C++ declarations and definitions, base stop cleanup, the existing MIB signal declaration, and module tests.
- Siblings and terminal paths: legacy and HT association, reassociation, authentication, refusal, disassociation, deauthentication, timeout cancellation, stop, crash, and restart.
- Boundaries and units: counters identify operations, not simulation time. Counters survive restart. Beacon intervals remain relative `simtime_t` durations.
- Mapped verification: the explicit module and unit filters in section 7 select existing cases. Two proposed module cases fill the callback gaps.
- Contract self-check: complete. The current source supports the named owners, callers, consumers, and terminal paths. No proposed path is sealed.

| Step and purpose | Owner and files | Change | Expected behavior | Verification |
| --- | --- | --- | --- | --- |
| 1. Expose the defects | Module tests under `tests/module/` | Extend AP lifecycle coverage. Add STA completion callback coverage. Add an association snapshot observer. | Tests reach actual management handlers and MIB signal delivery. | Show an assertion failure or diagnostic failure against the original code. |
| 2. Make publication complete | MIB `.cc`, `.h`, and `.ned` | Add combined HT/rate installation and complete cleanup commits. | Each callback sees complete state. No commit writes after its callback. | MIB signal assertions, equal updates, invalid input, and independent peer checks. |
| 3. Protect AP continuations | `Ieee80211MgmtAp.cc` and `.h` | Add identities, stable values, and callback checks. Use the combined commit. | Stop or replacement prevents stale responses and outcomes. | AP teardown and association callback matrix. |
| 4. Protect STA continuations | `Ieee80211MgmtSta.cc`, `.h`, and required base cleanup | Add identities and local snapshots. Complete timer ownership before publication. Detach cleanup records before callbacks. | No erased AP access, stale confirmation, or orphan timer. | STA callback matrix and existing lifecycle/peer-termination tests. |
| 5. Verify integration | Focused module/unit tests and project checks | Build fresh libraries. Run the selected cases. Review callback boundaries and ownership. | All acceptance criteria have direct evidence. | Commands below and both semantic checklists. |

Keep tests with their corresponding source fix in the reviewable commit series.
Place the shared MIB change before its management callers.

## 6. Regression design

Use module tests for management behavior and synchronous signal observations.
Use a real MIB and the real management handler in each fixture.
A test subclass can expose the handler or capture an outgoing confirmation.
It must not copy the production transition logic.
These defects are deterministic; seed 0 and run 0 suffice.

| Case | Trigger and production path | Direct assertions | Failure before the fix |
| --- | --- | --- | --- |
| AP teardown | Associated legacy and HT peers enter authentication, deauthentication, disassociation, or acknowledged refusal. A rate listener stops management. | No stale response or outcome. No peer record, ID, rate entry, or pending transaction remains. | An erased record access or a stale refusal outcome. |
| AP replacement | The same listener stops and restarts management. It completes a new association for the same address before it returns. | The new association survives. Only the new operation emits its outcome. | Old cleanup uses or removes the new peer state. |
| AP same-lifecycle replacement | A callback replaces the peer transition without stop. | The old handler fails its transition check. An unrelated peer remains valid. | An address-only guard permits the old handler to continue. |
| STA completion | First association and reassociation enter `processAssociationResponse()` with changed legacy or HT state. A rate listener stops management. | No association signal or successful primitive follows cancellation. No beacon timer survives. | Invalid AP access or stale completion. |
| STA signal callbacks | A listener stops at HT failure or at `l2AssociatedSignal`. A second association listener reads the details. | The second listener receives the original snapshot. No later confirmation or timer creation occurs. | Deleted details, changed details, or an orphan timer. |
| STA restart/replacement | A listener restarts management and completes a new attempt for the same AP. | The new identity, timer, state, and confirmation survive. No old confirmation follows. | Old completion applies to the replacement. |
| STA refusal and cleanup | Refusal, old-association removal, and list cleanup invoke rate callbacks. | No erased AP or list iterator access. The applicable pending transaction remains correct. | Earlier callbacks invalidate records before completion reaches its final setter. |
| Complete AP snapshot | First association and same-AP reassociation change accepted peer rates and HT state. Observe the first rate callback. | Associated status, committed ID, detached pending state, HT state, and accepted rates agree. | The callback sees new HT state with old or absent rates. |
| Rate consumer | The AP accepts a peer rate set that excludes an otherwise selectable local rate. Query production rate selection inside the callback. | The selected rate belongs to the accepted peer set. | The missing or stale peer restriction permits the excluded rate. |
| No change and validation | Repeat an equal snapshot. Submit invalid rate state to the combined operation. | Equal input emits no rate signal. Invalid input leaves all stores unchanged. | The new API emits redundant signals or partially commits invalid input. |

Cover HT installation, HT removal during a legacy replacement, and unchanged HT with changed legacy rates.
Enable each stop listener once, before the target commit.
Disable it before nested lifecycle calls, so cleanup signals do not recursively trigger the test action.
For stop-only cases, check state after the old timer deadline as well as immediately after the callback.

Extend [Ieee80211MgmtApLifecycle_1.test](../../tests/module/Ieee80211MgmtApLifecycle_1.test).
It currently expects two rate signals for peer teardown and tests stop at either association publication boundary.
Replace those assertions with one complete commit assertion and one association publication boundary.
This change tests the specified notification contract; it does not regenerate expected output.

Add proposed module targets:

- `Ieee80211MgmtStaAssociationCallbacks_1.test` for STA completion, signal lifetime, refusal, and replacement.
- `Ieee80211MgmtApAssociationRateSnapshot_1.test` for consistent AP publication and the production rate consumer.

Retain existing lifecycle, peer-termination, reassociation, and HT association cases as adjacent coverage.
Helper unit tests alone cannot prove callback safety or production rate selection.
No packet capture or statistical campaign is necessary for these internal transition claims.

## 7. Planned commands and evidence limits

Run commands from the checkout root after the normal INET environment setup.
The two runner scripts and the architecture checker exist in this checkout.
Their implementations support the explicit filters below.
Section 9 records the executed commands and the final results.

```sh
make MODE=debug -j8
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
inet_run_module_tests -m debug -f 'Ieee80211Mgmt(ApLifecycle|StaAssociationCallbacks|ApAssociationRateSnapshot)_1\.test$'
inet_run_module_tests -m debug -f 'Ieee80211(MgmtSta(Lifecycle|Disassociation|Deauthentication)|MgmtApReassociationSnapshot|HtAssociation)_1\.test$'
inet_run_unit_tests -m debug -f 'Ieee80211(MibAssociationId|MgmtApTransaction|RateSelection|SupportedRates|HtCapabilities)_1\.test$'
make MODE=release -j8
```

Record case counts, command exit codes, configuration, run, seed, library mode, and diagnostic artifact paths.
A zero-case selection supplies no test evidence.
Use a memory sanitizer if ordinary assertions do not expose an erased access deterministically.
Record its exact build and run commands when that diagnostic becomes necessary.

Review the [general checklist](../../doc/project/enforcement/checklist/general.md) and
[IEEE 802.11 checklist](../../doc/project/enforcement/checklist/ieee80211.md) after the focused tests.
Before a push, apply the additional gates in [run-the-gates.md](../../doc/project/guide/run-the-gates.md).
No push forms part of this request.

Plan preparation used source inspection and runner inspection.
Execution added fresh builds, module tests, unit tests, and enforcement checks.
Section 9 records the evidence limits.

## 8. Risks, protection, and decisions

The [seal registry](../../doc/project/audit/seal-list.md) leaves the proposed management and MIB paths open.
The architecture and naming ledgers contain no exception that authorizes these callback defects.
The design introduces no new dependency direction or protocol policy.

The largest risk is an incomplete identity check at an earlier callback boundary.
The implementation review must trace each affected success, refusal, cancellation, and stop path through its final side effect.
Shared stop changes also need the existing simplified-STA lifecycle cases if their contract changes.
Confirm that scope from the final diff before the last test selection.

An assertion count changes intentionally from two partial rate notifications to one complete notification.
No fingerprint, statistical baseline, or recorded expected output update is proposed.
If an executed check requires such an update, present its exact scope and cause under
[TR-BASELINE-DELIBERATE](../../doc/project/rule/testing.md#tr-baseline-deliberate).

The project implementation-plan procedure remains a draft.
This plan uses its format and review questions without treating its draft approval process as active policy.
The first user request authorized plan preparation.
The later instruction, "Execute the plan", authorized the source changes and verification.

Plan review:

| Question | Result |
| --- | --- |
| Does the plan explain the problem without the conversation? | Yes. Sections 1 and 2 identify the defects and their production paths. |
| Does the evidence support each owner? | Yes. Management owns decisions and identities. The MIB owns shared committed state. |
| Does each addition have a required consumer? | Yes. Management callers need complete MIB commits. Handler continuations need lifecycle and transition identities. |
| Does the plan define completion? | Yes. Section 3 states observable criteria. Section 6 maps them to direct assertions. |
| Does the plan distinguish proposed and observed checks? | Yes. Section 7 gives the proposed commands. Section 9 records execution. |
| Does implementation require a protected-path permission? | No proposed path matches the current seal registry. Recorded expectation changes remain outside the proposed scope. |
| Does this request authorize implementation? | Yes. The user instructed, "Execute the plan". |

## 9. Local execution record

### Source result

The source addresses all three comments.
The MIB commits accepted HT state and legacy rates before one notification.
Peer removal and stop also commit all applicable transient state before one notification.
Each new commit returns without further state mutation after its notification.
Equal snapshots retain the existing no-notification rule.
Invalid rates or an incomplete HT argument pair leave the stores unchanged.

AP teardown uses copied addresses and current-operation checks after callbacks.
AP success retains the response identity from MAC completion.
AP teardown reuses the existing identity allocator for each peer transition.
The lifecycle counter distinguishes stop and restart.
Both counters retain their values across restart.

STA completion uses a local `ApInfo` snapshot with no timer pointer.
The signal and confirmation retain their existing types and virtual signatures.
The snapshot remains valid throughout synchronous signal delivery.
The STA owns its beacon timer before the first completion notification.
Ordinary stop therefore cancels that timer.

The STA uses one monotonic association counter for attempts, cancellation, and detached completion.
It also retains the transaction peer after it detaches the request timeout.
This peer identity lets deauthentication cancel completion during old-association cleanup.
It does not duplicate the authoritative association state.
Current-association teardown preserves a pending request for a different AP.
List cleanup detaches records and timers before it calls the MIB.

The new MIB methods have production callers:

| Method | Required caller |
| --- | --- |
| `installBssAndPeerState()` | AP acknowledged success and STA accepted response |
| `removePeerState()` | Association-ID release and STA refusal or peer cleanup |
| `clearBssAndPeerState()` | STA current-association cleanup |
| `clearManagementState()` | Shared management stop |

The mutation helper and identity checks remain private.
The change adds no general transaction API, virtual extension, parameter, or wire field.
The existing AP lifecycle test now expects one complete teardown notification.
The change leaves its recorded output assertions intact.

### Reproduction and verification

All commands ran from `/home/user/omnetpp_ws/inet-ieee80211-txop-boundaries-rate-selection`.
The build used Clang and OMNeT++ `6.4.0aipre2`.
The module runner used the debug library.
The three callback fixtures use configuration `General`, run 0, and seed 0.
The runners used each adjacent case's declared configuration and expected result.

The first AP snapshot assertion failed against the original source after a fresh debug build.
The failed assertion was `observedMib->findPeerRateSet(observedPeer) != nullptr`.
It failed at 25 microseconds, event 5, during the first rate notification.
The runner log is `/tmp/ieee80211-callbacks-before-test.log`.
The later test run replaced the generated `test.err`; this document preserves the observed diagnostic.
This failure directly proves the partial-publication defect.
The final callback matrices prove cancellation, replacement, and lifetime behavior through the real handlers.

The final commands were:

```sh
make MODE=debug -j8
make MODE=release -j8
inet_run_module_tests -m debug -f 'Ieee80211(MgmtAp(Lifecycle|AssociationRateSnapshot|ReassociationSnapshot|GenericRadio|UnavailableChannel)|MgmtSta(AssociationCallbacks|Lifecycle|Disassociation|Deauthentication|SimplifiedInitialization|SimplifiedResponseRates|BeaconUpdate)|HtAssociation)_1\.test$'
inet_run_unit_tests -m debug -f 'Ieee80211(MibAssociationId|MgmtApTransaction|MgmtStaPrimitiveDispatch|RateSelection|SupportedRates|HtCapabilities)_1\.test$'
inet_run_module_tests -m debug -f 'Ieee80211MgmtStaAssociationCallbacks_1\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
python3 doc/project/enforcement/check-ned-msg-naming.py --diff --scope src/inet/linklayer/ieee80211
git diff --check
```

| Check | Result | Log under `/tmp/` |
| --- | --- | --- |
| Fresh original debug build | Exit 0 | `ieee80211-callbacks-before-build.log` |
| Original AP snapshot assertion | One case failed as expected | `ieee80211-callbacks-before-test.log` |
| Final debug build | Exit 0 | `ieee80211-callbacks-debug-build.log` |
| Final release build | Exit 0 | `ieee80211-callbacks-release-build.log` |
| Selected module tests | Exit 0; 13 PASS | `ieee80211-callbacks-final-module.log` |
| Selected unit tests | Exit 0; 6 PASS | `ieee80211-callbacks-unit.log` |
| STA callback test after timeout additions | Exit 0; 1 PASS | `ieee80211-callbacks-timeout-module.log` |
| Architecture check | Exit 0; PASS | `ieee80211-callbacks-final-architecture.log` |
| Naming check for changed NED/MSG files | Exit 0; 2 applicable files; PASS | `ieee80211-callbacks-final-naming.log` |
| Whitespace check | Exit 0 | No output |

The generated module artifacts are under `tests/module/work/<case>/`.
The generated unit artifacts are under `tests/unit/work/<case>/`.
Each runner log names the selected cases and their results.
The separate STA rerun adds timeout checks to an already selected case; it does not add a fourteenth module case.

The AP callback matrix covers legacy and HT peers through four teardown paths.
It covers stop, crash, stop/restart replacement, and replacement within the same lifecycle.
The STA matrix covers association and reassociation with legacy, valid HT, and invalid HT responses.
It also covers refusal, list cleanup, old peers, peer termination, and reassociation timeout.
A second listener checks the original immutable details after the first listener changes management state.
The tests check timer absence after the old deadline.

The AP snapshot test calls production rate selection inside the notification.
It rejects 24 Mbps when the accepted peer advertises only 6 Mbps and 12 Mbps.
It also checks equal snapshots, changed rates, HT removal, independent peers, aliased inputs, and invalid input rollback.
The adjacent cases cover shared stop behavior for detailed and simplified management.
They also cover generic radios and unavailable channels.

The full subtree naming audit reports 32 existing candidates in unchanged `.msg` files.
Its log is `/tmp/ieee80211-callbacks-naming.log`.
The audit supplies no new violation from this change.
The separate check for changed files passes.
No naming ledger entry forms part of this change.

### Semantic self-audit

The audit covers the final source diff and the test evidence above.
It uses the current general and IEEE 802.11 checklists.

| General item | Verdict | Evidence or scope |
| --- | --- | --- |
| AR-ORG-CONTRACTS: new interface | N/A | No new interface |
| AR-ORG-CONTRACTS: declared outcomes | PASS | Existing confirmation signatures and result codes remain intact |
| PR-MSG-BODY / PR-MSG-WHY | PASS | The commit message states the defect, cause, regression tests, and verification |
| AR-ORG-CONTRACT-PURITY | N/A | No contract header change |
| AR-ORG-VIS-SPLIT | PASS | No product observer logic |
| AR-ORG-KERNEL | PASS | Existing timers, signals, and lifecycle operations |
| AR-MOD-COMPOSITION | PASS | Existing management and MIB owners |
| AR-COM-SOCKETS | N/A | No application or transport change |
| AR-COM-DIRECT | PASS | Typed direct MIB calls |
| AR-COM-NOTIFY | PASS | Complete commits and borrowed immutable snapshots |
| AR-OBS-SIGNALS | PASS | Independent behavioral test listeners; no product observer mutation |
| AR-OBS-NED-TRUTH | PASS | Signal lifetime comment at the NED declaration |
| AR-OBS-INTROSPECTION | N/A | No new protocol or frame |
| AR-CFG-INFER / QR-DUP | PASS | No new configuration or duplicate authoritative state |
| QR-OBJECT-OWNERSHIP | PASS | Local snapshots; timer and packet cleanup before callbacks |
| AR-CFG-PARAMS | N/A | No new parameter |
| AR-EXT-REUSE | PASS | Existing AP allocator and shared HT mutation helper |
| AR-EXT-NOCORE | N/A | No new protocol |
| AR-EXT-MINIMAL-SURFACE | PASS | Every public addition has a production caller; other helpers remain private |
| AR-EXT-VIRTUAL-IS-A-PROMISE | N/A | No new product virtual method |
| AR-BUILD-DECLARATIVE | N/A | No build descriptor change |
| RR-NUMERIC-STABLE | N/A | No external numeric code change |
| AR-QUAL-NAMING | PASS | New names and the check for changed files pass |
| AR-QUAL-LOGGING | PASS | Invalid input throws before mutation |
| AR-QUAL-DETERMINISM | PASS | Integer identities; no host time or pointer order |
| AR-QUAL-TESTS | PASS | Real management callbacks and production rate selection |
| AR-QUAL-TRACEABILITY | N/A | No recorded expectation change |
| AR-QUAL-DISPLAY | N/A | No new product module |

REVIEW: 17 PASS, 11 N/A, 0 FLAG, 0 QUESTION.

| IEEE 802.11 item | Verdict | Evidence or scope |
| --- | --- | --- |
| AR-WLAN-STD-TRACE | N/A | Infrastructure lifetime and notification repair; no new normative rule |
| AR-WLAN-STD-GATING | PASS | Existing legacy and HT admission gates remain intact |
| AR-WLAN-ARCH-BOUNDARIES | PASS | Existing typed MIB calls and rate-selection contract |
| AR-WLAN-ARCH-OWNERSHIP | PASS | Management owns identities; MIB owns committed peer state |
| AR-WLAN-ARCH-VARIANTS | N/A | No new variant |
| AR-WLAN-FRAME-REPRESENTATION | N/A | No wire change |
| AR-WLAN-PHY-AUTHORITY / AR-WLAN-PHY-TIMING | N/A | No PHY formula or constant change |
| AR-WLAN-MAC-EXCHANGE | PASS | MAC acknowledgment still controls AP completion |
| AR-WLAN-MAC-SEQUENCE | N/A | No sequence arithmetic |
| AR-WLAN-MAC-QOS | N/A | No QoS or contention change |
| AR-WLAN-MAC-MULTIUSER | N/A | No multiuser change |
| AR-WLAN-OBS-EVENTS | PASS | One publication for each changed complete commit |
| AR-WLAN-QUAL-TESTS | N/A | No normative change; focused and adjacent tests pass |

REVIEW: 5 PASS, 8 N/A, 0 FLAG, 0 QUESTION.

No sanitizer, fingerprint suite, statistical suite, or full regression suite ran.
Deterministic assertions exposed the original defect and verify the affected callbacks.
The test fixtures suppress unrelated transmission and exercise synchronous management callbacks directly.
They do not prove every callback in the wider management subsystem.
No fingerprint, statistical baseline, or existing recorded output changed.
No protected source path changed.
The user requested a local commit after verification.
The commit also includes release and migration notes for the public signal changes.
No push occurred.
The plan remains under `plan/pending/` until the change lands.
