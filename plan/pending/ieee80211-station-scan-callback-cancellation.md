# Implementation plan: station scan cancellation after callbacks

Checkout: `/home/user/omnetpp_ws/inet-ieee80211-txop-boundaries-rate-selection`.
Source revision: `4fa995dbb49107362a804442725566e83a6c7b89`.
Plan version: 2, 2026-10-02.
Status: implemented and verified.
Approval: the user said "OK execute the plan" on 2026-10-02.
Scope: version 2, including the source fix, tests, release note, and verification.

The checkout contains a local change to `AGENTS.md`.
It also contains the untracked `plan/done/ieee80211-pr1218-audit-corrections.md`.
These files are outside this change.

## 1. Problem and required behavior

A station scan request first removes its old association and cached access points.
An access point is a wireless peer that provides network access.
A callback is a listener call that executes before the signal emitter returns.
The rate-state signal permits a callback to stop management or replace its current operation.

`clearAPList()` detects cancellation and returns from its own cleanup.
`processScanCommand()` still continues after that return.
It writes scan state and schedules a new timer after management stops.
The earlier `disassociate()` call permits the same failure.

The scan command must return after either cleanup call if a callback cancels or replaces its operation.
It must preserve the state and timers of any replacement operation.
Normal cleanup must still permit the requested scan.

Example: a passive scan removes a cached peer rate.
Its listener stops management.
The old scan request must return with no scan timer and with `isScanning == false`.

## 2. Source evidence and owner

`Ieee80211MgmtSta` owns the scan state and timer.
The management information base, `Ieee80211Mib`, owns the peer rates and emits their change signal.

| Verified fact | Source |
| --- | --- |
| Commands from `agentIn` reach `handleCommand()`. | [Ieee80211MgmtBase.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.cc) |
| `handleCommand()` selects `processScanCommand()` from the request type. It deletes the request after that call. | [Ieee80211MgmtSta.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) |
| `clearAPList()` cancels association, detaches the list, removes rates, and checks the operation identity. | [Ieee80211MgmtSta.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) |
| `removePeerRateSet()` emits `rateStateChanged` after an existing entry disappears. | [Ieee80211Mib.cc](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.cc) |
| `disassociate()` cancels association before `clearCurrentAssociation()` emits a rate-state change. | [Ieee80211MgmtSta.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) |
| `stop()` advances `lifecycleGeneration`, cancels the scan timer, and clears scan state. | [Ieee80211MgmtSta.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) |
| `processScanCommand()` has no cancellation check after either cleanup call. | [Ieee80211MgmtSta.cc](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) |
| Existing callback coverage enters list cleanup directly through `clearDiscovery()`. | [Ieee80211MgmtStaAssociationCallbacks_1.test](../../tests/module/Ieee80211MgmtStaAssociationCallbacks_1.test) |

These facts come from source inspection.
This plan review did not execute a simulation reproduction.

## 3. Scope and acceptance criteria

1. A stop or crash during either cleanup call prevents the old command from starting a scan.
2. A stop followed by an immediate restart also cancels the old command.
3. A replacement scan within the same lifecycle retains its scan state, subscription, and timer.
4. A replacement association within the same lifecycle retains its association state and timers.
5. The old command creates no scan activity after the callback returns. A replacement scan can complete and confirm normally.
6. Ordinary active and passive scans still start and complete once.
7. Normal association cancellation during cleanup does not cancel an otherwise valid scan.
8. The command dispatcher still deletes each request exactly once.

The change covers the two cleanup boundaries in `processScanCommand()`.
It preserves the scan timing policy, rate-state signal, module parameters, and virtual method signatures.
Other management callbacks remain outside this fix.
The change asserts a model lifecycle contract, rather than a new IEEE protocol requirement.

## 4. Proposed design

Reuse `lifecycleGeneration`, `associationTransactionId`, and `isCurrentAssociationOperation()`.
An operation identity consists of the lifecycle generation and the association transaction number.
`stop()` changes the generation, so the identity also detects an immediate restart.
Association cancellation or replacement changes the transaction number within the same lifecycle.

Each base cleanup call invokes `cancelPendingAssociation()` exactly once before its first callback.
That invocation advances `associationTransactionId` even when no association is pending.
The scan command must account for this normal advance before it compares the identity.
It must not capture a fresh identity after the callback returns.
That would accept the replacement operation as its own operation.

Proposed control flow:

```cpp
// Keep the existing isScanning precondition.
auto generation = lifecycleGeneration;
auto expectedTransactionId = associationTransactionId;
if (mib->bssStationData.isAssociated) {
    ++expectedTransactionId;
    disassociate();
    if (!isCurrentAssociationOperation(generation, expectedTransactionId))
        return;
}
++expectedTransactionId;
clearAPList();
if (!isCurrentAssociationOperation(generation, expectedTransactionId))
    return;
// Continue with the existing scan-state setup and scanNextChannel().
```

Use the existing unsigned type for both transaction values.
Use increment operations with the same wrap behavior as `cancelPendingAssociation()`.
Explain the expected advances in a short source comment.

| Option | Decision and reason |
| --- | --- |
| Check only `lifecycleGeneration`. | Insufficient. It misses replacement within the same lifecycle. |
| Compare the unchanged transaction number from before cleanup. | Incorrect. Normal cleanup changes that number. |
| Return cancellation status from both virtual cleanup methods. | Larger interface change. External subclasses would need new method signatures. |
| Add a separate scan identity. | Unnecessary state. Every nested scan already advances the existing transaction number through list cleanup. |
| Compare the expected existing identity after each cleanup call. | Selected. It guards both boundaries without a new interface or state owner. |

This choice follows [AR-EXT-REUSE](../../doc/project/rule/architecture.md#ar-ext-reuse).
Management retains state ownership under [QR-STATE-OWNER](../../doc/project/rule/quality.md#qr-state-owner).

## 5. Files, steps, and compatibility

| Step and purpose | Files and owner | Proposed change | Expected result and check |
| --- | --- | --- | --- |
| 1. Reproduce cancellation through the command. | `tests/module/Ieee80211MgmtStaAssociationCallbacks_1.test`; station callback fixture | Add scan request dispatch and scan observations. Arm a listener at each cleanup boundary. | Before the fix, stop cases expose a scan timer or active scan state after stop. |
| 2. Guard both cleanup boundaries. | `src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc`; station management | Compare the expected identity after disassociation and list cleanup. | The old command returns before further cleanup, scan-state writes, subscriptions, or channel changes. |
| 3. Prove ordinary scans and replacement preservation. | The same module test | Add the finite case matrix below. | Canceled commands leave no scan activity. Replacement operations retain their state. Valid scans complete once. |
| 4. Describe the user-visible fix. | `WHATSNEW` | State that station scans cannot restart after a listener stops or replaces management. | The release note names the corrected behavior. |
| 5. Verify the change and prepare one commit. | Source, test, release note, and this plan | Run the focused checks. Record their results before commit preparation. | The fix and its test form one reviewable commit. |

The header, NED module definition, and MIB implementation need no changes.
External subclasses retain their existing method signatures, but signatures alone do not establish behavioral compatibility.
The guard depends on the base cleanup contract: one normal transaction advance per call.
Source searches found no station overrides of `clearAPList()`, `disassociate()`, or `cancelPendingAssociation()` in the checkout.
An external override that omits base cancellation can cause the guard to reject a valid scan.
External override compatibility remains unverified.
The proposed fix requires no migration for the base class or overrides that preserve its cancellation contract.

## 6. Test design

Extend the existing module test because the claim concerns module state and lifecycle callbacks.
Use [TR-CAT-MATCH](../../doc/project/rule/testing.md#tr-cat-match) for this category choice.
Create a scan primitive on the heap and dispatch it through the production `handleCommand()` method.
Let that method retain request ownership and deletion.
Do not copy its dispatch logic into the fixture.
Expose dispatch through a fixture method with `Enter_Method`.
Use a test subclass of `Ieee80211Prim_ScanRequest` with a destructor counter for each request.
Its generated base already declares a virtual destructor.
Assert one destruction for the original request and each replacement request after dispatch returns.

The current fixture's `confirmations` counter counts association confirmations.
Its `hasTimer()` method checks the beacon timer, rather than the scan timer.
Add separate scan observations:

- Expose `scanTimer`, its name and deadline, `isScanning`, scan request values, and the active-scan subscription.
- Count probe requests in the existing `sendManagementFrame()` override.
- Count `changeChannel()` calls in its existing override.
- Count scan confirmations through a fixture override of `sendConfirm()` that consumes the scan confirmation primitive.

Retain the production `sendScanConfirm()` implementation so the test reaches its completion path.
Keep the original association counters and callback cases intact.
Use separate scan callback actions instead of changing the meaning of the existing action numbers.

Use the existing network and `seed-set = 0`.
The existing station uses `n(mixed-2.4Ghz)` with detailed management and rate-state signals.
Use internal channel index 1, `probeDelay = 10us`, `minChannelTime = 20us`, and `maxChannelTime = 40us`.
Select distinct request values for the original and replacement scans.
Clear the listener's armed flag before its action to prevent recursive actions.
Keep its signal subscription intact during signal delivery.

Test both boundaries:

- Cached peer removal during `clearAPList()`.
- Current association removal during `disassociate()`.

For list cleanup, prepare an authenticated cached peer with an installed rate set and no current association.
For disassociation, prepare a completed association with installed shared rate state.
Arm the listener only after preparation.
Assert that the selected listener action executes exactly once per case.
Allow extra rate-state notifications from stop cleanup or the replacement operation.

At each boundary, test active and passive requests with these callback actions:

| Callback action | Required observations after the original command returns |
| --- | --- |
| Stop | `isScanning == false`, no scan timer, and no active-scan subscription. |
| Crash | The same clean state as stop. |
| Stop followed by start | No scan from the old command. A later independent request succeeds. |
| Start a replacement scan without stop | The replacement timer, deadline, channel list, and request values retain their callback values. |
| Start a replacement association without stop | The replacement association state and timers retain their callback values. No scan starts. |

Record the `changeChannel()` call count when the listener finishes its action.
The original command must not increase that count afterward.
Record the replacement state before the listener returns.
Compare that state after the original command returns.
Wait 60us after dispatch to cover the complete scan interval.
A passive scan takes 40us on this channel.
An active scan takes at most `probeDelay + maxChannelTime`, or 50us.
Without a replacement scan, assert zero new scan confirmations, probe requests, and channel calls throughout that interval.
With a replacement scan, assert only its expected probe and one scan confirmation.
Use a replacement scan with the opposite active/passive choice to detect state overwrite.
For a replacement association, leave its 20ms timeout pending during the 60us observation interval.
Assert its cached peer record and timeout survive the old command.
Complete that association through the existing response path after the interval.
Stop any surviving operation before the next case.
Start management before the next case if it is down.

Add successful active and passive cases for an empty list, cached peers, an existing association, and a pending association.
Assert the initial timer kind and deadline.
Assert that each scan completes once without a residual subscription or timer.
These cases detect an incorrect guard that treats normal transaction advances as cancellation.
Limit the new cases to less than 2ms of simulation time in total.
The existing final wait uses 25ms, and the configuration limits simulation time to 30ms.
Assert that all new cases finish before that final wait.

One fixed seed is sufficient because the listener directly controls this deterministic call sequence.
The proposed test covers command dispatch, request destruction, cleanup cancellation, state preservation, and timer behavior.
It does not prove packet delivery, a probability distribution, or unchanged results across all configurations.
The fixture intercepts channel calls and management frames; it does not prove a physical radio channel change.
Direct dispatch exercises `handleCommand()` but does not test delivery through the `agentIn` gate.

## 7. Verification commands and expected results

Run these commands from the checkout root.
The runner help confirms `-m` and `-f` in this checkout.
The test targets below already exist; their new scan cases are proposed.

```sh
make MODE=debug -j$(nproc)
inet_run_module_tests -m debug -f 'Ieee80211MgmtStaAssociationCallbacks_1\.test$'
```

Run the extended test before the production edit.
Expect a direct failure in a new cancellation assertion.
Run it after the production edit against the fresh debug library.
Expect all original callback cases and new scan cases to pass.

Run the related checks after the focused test passes:

```sh
inet_run_module_tests -m debug -f 'Ieee80211(MgmtSta(Lifecycle|Disassociation|Deauthentication)|AgentStaReassociation)_1\.test$'
inet_run_unit_tests -m debug -f 'Ieee80211MgmtStaPrimitiveDispatch_1\.test$'
make MODE=release -j$(nproc)
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mgmt
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211/mgmt
git diff --check
```

The related module cases check lifecycle cleanup, peer termination, and agent reassociation.
The unit case checks the primitive dispatcher and direct list cleanup.
Release compilation checks the changed source in the second supported mode.
Debug tests remain the behavioral evidence because their assertions stay active.

Record the exact commands, modes, exit statuses, executed case counts, configuration, seed, and artifact paths.
A zero-case selection supplies no pass evidence.
Apply the general and IEEE 802.11 semantic checklists to the final diff.
Run the project-wide gates from [run-the-gates.md](../../doc/project/guide/run-the-gates.md) before any later push.

No build or behavioral test ran during plan preparation.
Runner help and the naming checker help exited with status 0.
Fingerprint and statistical effects remain unverified.
No recorded expectation changes form part of this plan.
Keep the existing test output expectation where its meaning remains correct.

## 8. Risks and plan review

The main risk is an incorrect allowance for the normal transaction advance.
The successful cases and same-lifecycle replacement cases test both sides of that distinction.
A guard after disassociation must precede list cleanup.
Otherwise, the old command can erase state that the callback creates.
The guard also assumes that virtual cleanup overrides preserve the base cancellation contract.
The current checkout has no such overrides; external override behavior remains outside the verified scope.

| Plan review question | Answer |
| --- | --- |
| Does the plan stand alone? | Yes. It states the trigger, failure, owner, design, and completion criteria. |
| Does source evidence support the owner and path? | Yes. The source table follows dispatch through cleanup and timer creation. |
| Does each significant addition need to exist? | The production change adds only local checks. Test observations establish cancellation and replacement preservation. |
| Does verification reach the failure? | Yes. The extended module test fails before the fix and passes afterward through `handleCommand()`. Section 10 records execution. |
| Are permissions separate from the plan? | The user authorized version 2 implementation. Recorded expectation changes remain outside that approval. |
| Does a seal restrict the proposed paths? | No. The current registry seals only `src/inet/common/packet/`. |
| What scope does this plan define? | Version 2 covers both cleanup guards, compatibility limits, test observations, request destruction, and time bounds. The user approved this scope. |

No unresolved source fact blocks the proposed base-class fix.
Review the single-advance contract again if the checkout changes before implementation.
The user instruction authorizes version 2 and its stated scope.
The project planning procedure remains draft; this plan does not claim its general approval policy is active.

## 9. Implementation contract

- Invariant and owner: `Ieee80211MgmtSta` must prevent an old scan command from modifying state after callback cancellation.
- Entry and control path: `agentIn` calls `handleCommand()`, which selects `processScanCommand()` and retains primitive deletion.
- Affected consumers and artifacts: station management source, its module test, `WHATSNEW`, and this plan. No generated input changes are necessary.
- Terminal paths: both cleanup boundaries cover stop, crash, immediate restart, replacement scan, replacement association, and successful scans.
- Boundaries and units: each base cleanup advances the unsigned transaction number once. Tests use channel index 1 and delays in microseconds.
- Verification: the extended callback module test must fail before the fix. It must pass after a fresh debug build.

The four preventive layers apply: C++, OMNeT++, INET, and IEEE 802.11.
The focused checks cover callback cancellation, borrowed state, request ownership, and timer cleanup.
The fix adds no normative protocol logic or production state.

## 10. Implementation report

- Behavior claim: scan commands return after callback cancellation at either cleanup boundary. Normal scans and replacement operations retain their behavior.
- Changed paths: `Ieee80211MgmtSta.cc`, `Ieee80211MgmtStaAssociationCallbacks_1.test`, `WHATSNEW`, and this plan.
- Contract deviations and scope changes: none. The guards follow the approved control flow. Fixture method names follow the project naming rules.
- Selected layers and checks: C++, OMNeT++, INET, and IEEE 802.11. The audit covers request ownership, callback cancellation, operation identity, and timer cleanup.
- Working directory: the checkout root stated above.
- Artifacts: `/tmp/inet-station-scan-verification/`. The table names each build or runner log.
- Coverage gaps: external overrides, delivery through `agentIn`, packet delivery, fingerprint effects, and statistical effects remain unverified.

The extended module test covers 20 cancellation cases, eight successful scans, and four independent scans after restart.
Its new cases use 1920us of simulation time.
The existing final wait completes before the 30ms simulation limit.
The existing output expectation remains unchanged.

The first fixture build lacked the `IRadio` declaration.
An explicit include corrected that setup error before defect reproduction.
The reproduced failure then occurred at `mgmt->channelChanges == channelsAfterAction`, at simulation time 0.
The listener already completed management stop, but the original scan command requested another channel change.
The final test passes after the two guards.

| Executed command | Mode and result | Exit status | Artifact |
| --- | --- | --- | --- |
| `make MODE=debug -j8` before the source fix | Debug library build passes. | 0 | `debug-before.log` |
| `inet_run_module_tests -m debug -f 'Ieee80211MgmtStaAssociationCallbacks_1\.test$'` before the source fix | One module case fails at the cancellation assertion. | 1 | `callback-before.log`, `callback-before.err`, `callback-before.out` |
| `make MODE=debug -j8` after the source fix | Fresh debug library build passes. | 0 | `debug-after.log` |
| `inet_run_module_tests -m debug -f 'Ieee80211MgmtStaAssociationCallbacks_1\.test$'` | One module case passes, including the scan matrix and original association cases. | 0 | `callback-final.log`, `Ieee80211MgmtStaAssociationCallbacks_1.out` |
| `inet_run_module_tests -m debug -f 'Ieee80211(MgmtSta(Lifecycle\|Disassociation\|Deauthentication)\|AgentStaReassociation)_1\.test$'` | Four related module cases pass. | 0 | `related-modules.log` |
| `inet_run_unit_tests -m debug -f 'Ieee80211MgmtStaPrimitiveDispatch_1\.test$'` | One unit case passes. | 0 | `primitive-unit.log` |
| `make MODE=release -j8` | Release library build passes. | 0 | `release.log` |
| `doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mgmt` | Scoped architecture checks pass. | 0 | `architecture.log` |
| `doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211/mgmt` | The subtree audit reports 26 pre-existing MSG naming candidates. | 1 | `naming.log` |
| `doc/project/enforcement/check-naming.sh --base HEAD src/inet/linklayer/ieee80211/mgmt` | The scoped check for changed declarations passes. No NED or MSG file changed. | 0 | `naming-changed.log` |
| `git diff --exit-code HEAD -- 'src/inet/linklayer/ieee80211/mgmt/*.msg'` | The MSG files match the source revision. | 0 | Command output is empty. |
| `git diff --check` | Whitespace checks pass. | 0 | Command output is empty. |

The callback module runs configuration `General`, run 0, with explicit `seed-set = 0`.
The four related module cases also run `General`, run 0, with their existing seed defaults.
The unit test has no simulation configuration or seed.
No baseline or existing output expectation changes form part of this implementation.

### Semantic self-audit

These verdicts cover this implementation only.
They exclude the unrelated local changes stated above.
They do not claim an independent review.

| General checklist item | Verdict and reason |
| --- | --- |
| AR-ORG-CONTRACTS: new role | N/A. No interface or role is added. |
| AR-ORG-CONTRACTS: outcomes | PASS. The base cancellation outcomes remain distinct from successful scans. External override compatibility remains unverified. |
| PR-MSG-BODY / PR-MSG-WHY | PASS. The commit message explains the symptom, cause, correction, reproduction, and verification. |
| AR-ORG-CONTRACT-PURITY | N/A. No interface header changes. |
| AR-ORG-VIS-SPLIT | PASS. Production adds no observation logic. Counters remain in the test fixture. |
| AR-ORG-KERNEL | PASS. The fix uses existing identity and module mechanisms. |
| AR-MOD-COMPOSITION | PASS. Station management retains its existing responsibility. |
| AR-COM-SOCKETS | N/A. No application or transport interaction changes. |
| AR-COM-DIRECT | PASS. The fix adds direct checks, rather than a new event. |
| AR-COM-NOTIFY | PASS. The behavioral listener uses module context. It clears its armed flag without changing subscriptions during signal delivery. |
| AR-OBS-SIGNALS | PASS. The test listener is a behavioral actor, rather than a recorder. |
| AR-OBS-NED-TRUTH | N/A. No NED declaration or corresponding parameter documentation changes. |
| AR-OBS-INTROSPECTION | N/A. No protocol representation is added. |
| AR-CFG-INFER / QR-DUP | PASS. The guard derives its expected identity from existing counters. |
| QR-OBJECT-OWNERSHIP | PASS. Dispatch deletes each tracked request once. Production retains timer ownership. The fixture deletes intercepted confirmation primitives. |
| AR-CFG-PARAMS | N/A. No production parameter or field is added. |
| AR-EXT-REUSE | PASS. The fix reuses the existing identity comparison. |
| AR-EXT-NOCORE | N/A. No protocol is added. |
| AR-EXT-MINIMAL-SURFACE | PASS. All new observation methods remain in the fixture. |
| AR-EXT-VIRTUAL-IS-A-PROMISE | PASS. New overrides fulfill existing methods. No production extension point is added. |
| AR-BUILD-DECLARATIVE | N/A. No build descriptor changes. |
| RR-NUMERIC-STABLE | N/A. The new action enum remains internal to the fixture. |
| AR-QUAL-NAMING | PASS. New C++ names follow type and method conventions. NED and MSG declarations remain unchanged. |
| AR-QUAL-LOGGING | PASS. The fixture asserts its invariants; valid cancellation returns normally. |
| AR-QUAL-DETERMINISM | PASS. Tests use fixed cases and simulation delays without host-time decisions. |
| AR-QUAL-TESTS | PASS. The module test reaches production dispatch and both cleanup boundaries. |
| AR-QUAL-TRACEABILITY | N/A. No recorded expectation changes. |
| AR-QUAL-DISPLAY | N/A. No NED module is added. |

REVIEW: 17 PASS, 11 N/A, 0 FLAG, 0 QUESTION.

| IEEE 802.11 checklist item | Verdict and reason |
| --- | --- |
| AR-WLAN-STD-TRACE | N/A. The change repairs model lifecycle cancellation, with no new normative protocol logic. |
| AR-WLAN-STD-GATING | N/A. No amendment-specific decision changes. |
| AR-WLAN-ARCH-BOUNDARIES | PASS. The guard reads state from its own management component. |
| AR-WLAN-ARCH-OWNERSHIP | PASS. Local snapshots add no second production state owner. |
| AR-WLAN-ARCH-VARIANTS | N/A. No protocol variant is added. |
| AR-WLAN-FRAME-REPRESENTATION | N/A. No frame representation changes. |
| AR-WLAN-PHY-AUTHORITY / AR-WLAN-PHY-TIMING | N/A. No production timing formula changes. Test delays use typed simulation time. |
| AR-WLAN-MAC-EXCHANGE | N/A. No frame exchange decision changes. |
| AR-WLAN-MAC-SEQUENCE | N/A. Transaction identities are not frame sequence numbers. |
| AR-WLAN-MAC-QOS | N/A. No traffic classification or access category changes. |
| AR-WLAN-MAC-MULTIUSER | N/A. No multi-user behavior changes. |
| AR-WLAN-OBS-EVENTS | N/A. No signal contract or emission changes. |
| AR-WLAN-QUAL-TESTS | N/A. The change adds no normative protocol behavior. Focused lifecycle tests pass. |

REVIEW: 2 PASS, 11 N/A, 0 FLAG, 0 QUESTION.

### Commit message

The source fix, test, release note, and plan form one change.
The following message describes that commit:

```text
ieee80211: fix: cancel station scans after management callbacks

A scan request can resume after a rate-state listener stops management.
Cleanup detects cancellation, but its return does not stop the scan command.
The old command can create a timer or overwrite a replacement operation.

Check the existing operation identity after both cleanup calls.
Allow each cleanup's normal transaction advance so valid scans still proceed.
The checks also detect immediate restart and replacement within the same lifecycle.

Reproduce the defect with the scan cases in
Ieee80211MgmtStaAssociationCallbacks_1.test against the parent source.
The first stop case fails its channel-call assertion before the fix.

Debug and release builds pass. Five module cases and one unit case pass.
Scoped architecture checks and changed-declaration naming checks pass.
The full subtree naming audit reports pre-existing MSG candidates.
Fingerprint and statistical effects remain unverified. No baseline changes occur.

Plan: plan/pending/ieee80211-station-scan-callback-cancellation.md
Change: src.ieee80211 | behavior.change.fix | test whatsnew ?
```
