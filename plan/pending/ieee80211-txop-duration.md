# Implementation plan: TXOP duration without Block Ack

Checkout: `inet-ieee80211-txop-boundaries-rate-selection`.
Source commit: `dabcd78282d9043764521b0b713ea17694f29d56`.
Plan revision: 2026-10-03, split revision 3; developer readability review.
Verification update: 2026-10-04; regression coverage for a rate change on retry.
Status: implemented in PR #1273, including the retry mode test.
Approval: the user requested execution of this plan on 2026-10-04.
Approval scope: split revision 3 of this plan only.
Dependencies: no other implementation plan.
Evidence: section 9 records the original implementation and focused verification. Section 10 records the PR audit correction.

The author checkout uses the local `CONTEXT.md` glossary for protocol terms.
That glossary is not part of the committed tree.
Sections 1–8 retain the approved design. Section 9 records the resulting implementation.
This revision explains revision 2's requirements without a change to feature scope or dependencies.

## 1. Check the complete exchange before transmission

HCF must check whether a complete frame exchange fits in the available TXOP time before it transmits the first frame.
For example, DATA/ACK includes the data transmission, SIFS, and the ACK transmission.
An RTS-protected exchange also includes RTS, CTS, and their SIFS intervals.
The check must include the response, even though the recipient transmits it.

The current HCF path admits the first exchange without a duration check.
It admits a later exchange whenever any TXOP time remains.
For example, 100 microseconds of available time cannot accommodate a complete exchange that needs 120 microseconds.

The new path first prepares the exact exchange that HCF will execute.
Preparation chooses its frames, modes, ACK policy, and optional RTS/CTS branch.
`TxopProcedure` checks the duration of that prepared exchange.
HCF executes the same choices after acceptance.
In this plan, admission means that check and its result.

This first implementation uses `Hcf.isBlockAckSupported = false`.
An HT-capable station can use it with non-HT modes that the current model supports.
It needs no Block Ack agreement, BAR, Block Ack response, or A-MPDU.

The main acceptance case transmits three separate QoS Data MPDUs from one AC:

```text
one EDCA channel grant
Data #1 -> SIFS -> ACK -> SIFS ->
Data #2 -> SIFS -> ACK -> SIFS ->
Data #3 -> SIFS -> ACK
one TXOP end
```

This case has six separate PPDUs and five SIFS intervals.
Its duration is `Tdata1 + Tdata2 + Tdata3 + Tack1 + Tack2 + Tack3 + 5 * SIFS`.
Each `T` value is the airtime of the named frame at its selected PHY mode.
The test must observe one TXOP start and one TXOP end, with no contention between the three exchanges.

A second case uses a budget that cannot accommodate the third exchange and its leading SIFS.
HCF ends the TXOP after the second exchange.
The third MPDU remains available for later channel access.
That refusal creates no transmission attempt, ACK progress, or retry increment.

The implementation is complete when these conditions hold:

1. The first exchange receives a complete duration check before its first transmission.
2. Each later exchange receives that check with its leading SIFS included.
3. Execution uses the accepted frame identities, modes, lengths, ACK policy, and protection branch.
4. Duration/ID reserves only the interval permitted for the selected frames and accepted next exchange.
5. Zero-limit allowances and overrun exceptions use actual frame identity and transmission history.
6. Refusal and cancellation keep retained frames under their owner without false protocol progress.
7. The real HCF path produces the three-frame case with Block Ack and both aggregation policies disabled.
8. Changes to shared interfaces preserve DCF execution and required recipient responses.

## 2. Scope and timing limits

### 2.1 Supported exchanges

The first path supports these existing exchanges:

- Individually addressed data with Normal Ack.
- Management frames with their required ACK.
- Optional RTS/CTS protection for those exchanges.
- Existing group traffic without an immediate response.
- Conventional fragments that the current data service supplies.
- An existing A-MSDU, which counts as one transmitted MPDU.

The main three-frame test disables both aggregation policies.
A separate case checks the existing A-MSDU allowance.
The plan adds no new no-response branch for individually addressed data.

### 2.2 Conditions for the duration guarantee

The duration guarantee assumes zero propagation delay.
It also assumes responses after nominal SIFS, with actual airtime equal to the predicted airtime.
The production test configures both peers to meet those conditions.
It checks the actual response mode and complete length at transmission.

HCF uses actual elapsed time before each later exchange.
A delayed response or nonzero propagation reduces the time available for that next exchange.
The first implementation does not guarantee completion within the limit when the stated timing assumptions do not hold.

The current rate policies supply ACK and CTS predictions.
The planner does not inspect another station's policy through a remote module query.
It retains the current timeout and response-validation procedure.

### 2.3 Work assigned to later plans

Separate future work supplies complete control and response-rate rules.
Separate future work supplies response-start indications and standard timeout enforcement.
Their separate plans retain the earlier design decisions for management rates and `IIeee80211Radio`.
Those features are not prerequisites for this first duration check.

Configurations with `isBlockAckSupported = true` continue to use the existing execution path.
This first change gives that path no new TXOP duration guarantee.
Separate future work connects Compressed Block Ack to prepared execution and admission.

Self-CTS, multiple protection, empty HT alternatives, and TXOP sharing remain outside this change.
Basic BAR repairs, agreement state, and A-MPDU support also remain outside it.
Automatic fragmentation to meet an airtime budget needs a separate plan.
An oversized initial exchange without a supported exception produces an explicit model-limit error before RTS or data transmission.
That error identifies an INET limitation; it does not implement the standard's required fragmentation.

## 3. Verified current behavior

The MAC paths in this table start at `src/inet/linklayer/ieee80211/mac/`.
These facts come from source inspection at the recorded commit.

| Source | Current behavior | Required change |
| --- | --- | --- |
| `coordinationfunction/Hcf.cc` | `channelGranted()` starts the TXOP. `transmitFrame()` selects mode, ACK policy, and Duration/ID immediately before transmission. | The prepared path supplies those exact values before admission. |
| `framesequence/HcfFs.cc` | `hasMoreTxOps()` skips the first duration check. Later checks require only positive available time. | Check the complete selected exchange on the new path. |
| `framesequence/TxOpFs.cc` | Its existing tree has Normal Ack data with optional RTS/CTS. | Preserve this constructor composition. |
| `framesequence/GenericFrameSequences.*`, `PrimitiveFrameSequences.*` | Execution chooses branches and creates later steps as it proceeds. | Prepare the complete exchange through these same classes. |
| `framesequence/FrameSequenceContext.cc` | `getIfs()` uses the number of executed steps to determine position. | Preparation keeps its own position without changes to executed history. |
| `queue/InProgressFrames.*` | A candidate query can extract frames from the queue. | Separate real extraction from read-only inspection. |
| `protectionmechanism/SingleProtectionMechanism.cc` | Duration queries choose modes again and inspect a pending frame. | Calculate Duration/ID from the accepted frames and modes. |
| `originator/TxopProcedure.*` | `getDuration()` reports elapsed time. Fragment and initiator queries have stubs. | Add admission and transmission history; preserve elapsed-time results. |
| `Rx.cc`, `Tx.cc` | `Rx::frameTransmitted()` extends the shared NAV. There is no independent TXNAV query. | Track the reservation from this TXOP's actual transmissions separately. |
| `Tx.*`, `contract/ITx.h` | `Tx` copies a frame before a delayed transmission. It cannot cancel that pending copy. | Add cancellation for an exact transmission request. |
| `rateselection/QosRateSelection.*`, `RateSelection.*` | Existing queries provide ACK/CTS modes but omit some BSS basic-rate rules. | Use them for this first path; keep the broader correction in the rate plan. |

`coordinationfunction/Hcf.ned` sets `isBlockAckSupported` to false by default.
`OriginatorQosAckPolicy::computeAckPolicy()` chooses Normal Ack when no agreement exists.
The data service's A-MPDU extraction call is disabled.
The current Normal Ack composition therefore supplies the first path for this work.

## 4. Preparation and execution contracts

### 4.1 Keep one definition of the frame exchange

The existing sequence tree defines which frames belong to an exchange.
A tree branch can contain optional RTS/CTS followed by data and ACK.
The new preparation operation uses that tree to choose and record one complete exchange.
It must not introduce another exchange builder with a separate set of rules.

Add `FrameSequencePlan` under `mac/framesequence/` to hold that result.
A plan node identifies one sequence object and one use of that object in the selected exchange.
Composite nodes own their selected child plans.
Primitive nodes own their prepared transmit or receive steps.
The plan records relative step offsets and whether the exchange starts or continues the TXOP.

Each existing generic class prepares its own part:

| Class | Preparation behavior |
| --- | --- |
| `SequentialFs` | Prepare children in constructor order and add their durations. |
| `OptionalFs` | Evaluate its predicate once and record inclusion or exclusion. |
| `AlternativesFs` | Evaluate its selector once and prepare only that child. |
| `RepeatingFs` | Prepare one finite child exchange at each HCF admission point. Keep a separate record for each repeated use. |

Preparation does not expand the entire TXOP burst in advance.
It prepares the active exchange and at most one possible next exchange.
Any nested repetition must consume a distinct candidate or make an explicit control decision.
A repetition without progress must terminate or report an error.
Preparation must not remove steps from a selected exchange merely to make its duration fit.

### 4.2 Separate real extraction from read-only inspection

`InProgressFrames` owns the frames available for transmission.
The data service can extract queued data, assign sequence numbers, create fragments, and register ACK state.
Those operations change real state.
This plan calls that phase staging.

Add `stageForPlanning()` to perform that work explicitly before the duration calculation.
It stages enough candidates for the active exchange and at most one next exchange.
It can obtain the next candidate while the active candidate remains eligible.
Repeated inspection must not extract or fragment the same data again.
A duration refusal leaves the staged frames with `InProgressFrames` for later channel access.

Add const `inspectStagedFrames()` to return descriptions in the existing frame order.
This method performs no extraction, sequence allocation, ACK registration, or queue reordering.
Its descriptions use the proposed `StagedFrameView` record:

| Field | Why the planner needs it |
| --- | --- |
| Borrowed const frame and exact identity | Identify the frame that execution must transmit. `InProgressFrames` retains ownership. |
| Receiver and optional TID | Keep the selected peer and QoS stream separate from other candidates. |
| Sequence control | Preserve the sequence number and fragment number. |
| Complete frame length | Calculate airtime with the MAC header, payload, and FCS included. |
| Frame lifetime information | Check that the borrowed frame still belongs to the same live owner before execution. |

Frame lifetime information means owner/lifecycle identity and validity of the retained reference.
It does not mean a new frame-expiry timer.
A removed frame cannot supply a later transmission.
Restart invalidates old references even if a new frame has the same sequence number.
The context retains that identity and cancels plans before the owner removes referenced frames.
The final transmit check also verifies that the selected frame remains valid for the current plan.

### 4.3 Read ACK state without protocol progress

The planner needs more information than an eligibility boolean.
A new frame and a frame whose ACK failed can both qualify for transmission.
Their ACK phases and actual transmission histories differ.

Add `AckFrameState` under `mac/common/` to copy those facts.
It contains the exact ACK phase, transmission eligibility, outstanding status, and actual transmission evidence.
It must preserve every current phase in `AckHandler` and `QosAckHandler`.
The shared record can represent existing Block Ack phases, although this first path uses the Normal Ack subset.

Add pure virtual `snapshotFrameState()` to `IAckHandler`.
Both concrete handlers implement it with a read-only table lookup.
The query applies to staged data and management frames.
Pure virtual means each concrete handler must supply its own implementation.
The query must not insert an entry or invoke a protocol callback.

A staged frame must already have its ACK entry from real registration.
An explicit `FRAME_NOT_YET_TRANSMITTED` entry is valid.
A missing entry for that staged frame indicates a registration or lifetime error.
Report that error.
Do not create an ACK entry as part of the snapshot query.
A copied snapshot must not treat successful preparation as evidence of actual transmission.

### 4.4 Prepare the next exchange with a private copy

Add `FrameSequencePlanningContext` under `mac/framesequence/` for temporary calculation state.
It keeps ordered frame views, copied ACK states, and a private position in that list.
A snapshot here means those copied facts at the start of preparation.
The real ACK handler and frame store remain the protocol owners.

For example, the staged frames are A and B:

```text
Real state:       A has not transmitted; B has not transmitted.
Prepared active:  Data A -> SIFS -> ACK A.
Private copy:     Assume A completes successfully; skip A in the candidate list.
Prepared next:    Data B -> SIFS -> ACK B, with leading SIFS for continuation.
```

This temporary assumed completion is the private projection.
It changes only the copied completion facts and private list position.
It changes no real ACK state, retry count, sequence number, or queue order.
The same rule applies to an existing group exchange that has no response.
The planner never invokes real success callbacks to simulate that completion.

A selector chooses a sequence branch.
An adapter translates the caller's state into that calculation's inputs.
The prepared and legacy selector adapters call one shared branch calculation.
The prepared adapter supplies the private snapshot; the legacy adapter supplies its current execution inputs.
This arrangement keeps the branch rules in one place.
Plan 7 later adds agreement snapshots and BAR decisions when those consumers exist.

Before the next exchange becomes active, HCF compares its required facts with the actual completed state.
A mismatch cancels that next exchange.
An ACK or CTS failure also cancels it.
HCF does not replace its frames or modes silently.

### 4.5 Record the values that execution must use

Each prepared transmit step records these values:

- Exact frame identity and selected mode.
- Complete length and calculated airtime.
- Preceding IFS and selected ACK policy.
- Duration/ID, after the decision about the next exchange.

Each prepared receive step records the request identity, expected ACK/CTS mode, complete response length, and nominal SIFS.
The current rate policy selects each originated mode once.
Temporary frame views carry any tags that a response-mode query needs.
Preparation must not put prediction tags on real staged frames.
Real transmission remains the point for rate feedback and `datarateSelected` notification.

The handler passes the prepared transmit record to HCF through its callback.
HCF checks frame identity, length, and IFS before it applies those values.
HCF does not choose another mode or ACK policy on this path.
DCF keeps its legacy execution path with null prepared metadata.

### 4.6 Add the sequence API and retain legacy execution

Add these pure virtual operations to `IFrameSequence`:

```cpp
virtual FrameSequencePlanResult planSequence(
    FrameSequencePlanningContext& context) const = 0;
virtual void startPlannedSequence(FrameSequenceContext *context,
    int firstStep, FrameSequencePlan& plan) = 0;
```

`planSequence()` returns one of these results:

| Result | Meaning |
| --- | --- |
| `READY` | A complete plan exists. A false optional predicate can produce a ready child with zero steps. |
| `EMPTY` | No candidate exists for the requested exchange. |
| `UNSUPPORTED` | A selected sequence or required input has no supported representation. An empty alternative set returns this result before index access. |

A duration refusal is a separate admission result.
It is not `EMPTY` or `UNSUPPORTED` preparation.
`startPlannedSequence()` installs the recorded choices for execution and maps their relative offsets through `firstStep`.
Execution does not evaluate their predicates or selectors again.

Keep `startSequence()`, `prepareStep()`, `completeStep()`, and `getHistory()` for legacy callers.
Implement the new operations in all generic and primitive classes that serve existing derived trees.
Keep the prepared-record getters pure virtual in the step contracts.
A null record identifies a legacy step.
Interface methods remain declarations without implementation bodies.
The existing interface-body deviation remains the `AV-CONTRACT-02` ledger item.

`FrameSequenceContext` owns the active plan and the accepted next plan.
Plans own prepared steps and generated RTS frames.
`InProgressFrames` owns the staged data that those steps borrow.
Callbacks can borrow the context, steps, and records only while those owners keep them alive.

## 5. Admission, protection, and cancellation

### 5.1 Calculate the complete duration

Use `mode.getDuration(complete frame length)` for each transmission or response.
A bytes-divided-by-bitrate estimate cannot include all PHY overhead.
The successful exchange cost is:

```text
transmit contribution = preceding IFS + mode.getDuration(complete frame length)
receive contribution  = SIFS + response mode.getDuration(complete response length)
exchange duration     = sum of all contributions
```

For initial DATA/ACK, the cost is `Tdata + SIFS + Tack`.
For initial RTS/CTS/DATA/ACK, it is `Trts + SIFS + Tcts + SIFS + Tdata + SIFS + Tack`.
An existing group frame without a response costs its data airtime.
Each continuation also includes SIFS before its first originator transmission.
Count each gap once.

Contention and response-start timeouts are not part of successful exchange airtime.
The zero-propagation assumption from section 2 also applies to the prepared next exchange.
Keep relative exchange costs separate from absolute simulation deadlines.
`TxopProcedure::getDuration()` retains its elapsed-time meaning for current result filters.

### 5.2 Check both the TXOP budget and the transmitted reservation

`TxopProcedure` owns the budget check and the transmission history for the current TXOP.
An ordinary exchange fits a positive limit when its complete cost is no greater than the available budget.
Equality passes this budget check.
A continuation includes its leading SIFS in that cost.

Clause 10.23.2.8 also constrains continuation through TXNAV.
TXNAV records the medium reservation that this station transmits.
It is separate from the receive NAV, which can include another station's reservation.
A successful TXOP budget check does not replace the TXNAV check.

`TxopProcedure` stores the absolute end time of this TXOP's transmitted reservation.
HCF updates it at transmission completion, before the sequence handler advances.
It uses the actual PPDU end time and the serialized Duration/ID value.
TXOP termination or lifecycle cleanup clears that endpoint.

The continuation checks use different costs at different times:

| Check point | Cost and comparison |
| --- | --- |
| After the preceding exchange completes: TXOP budget | The full next exchange, with leading SIFS, must fit the available TXOP time. Equality passes. |
| At that same point: TXNAV | The next exchange without leading SIFS must be strictly shorter than the available TXNAV interval. |
| Immediately before the next preamble | Recheck the unexecuted prepared steps against actual available time. Exclude the IFS that already elapsed. |

The tests must check these boundaries independently.
An exact TXOP budget fit can still fail the separate TXNAV condition.
A reserved next exchange receives another check with actual elapsed time before transmission.

### 5.3 Handle refusal, zero limits, and exceptions

An ordinary continuation that does not fit ends the TXOP.
Its retained frames remain available for later channel access without retry progress.
An initial exchange that exceeds the full positive limit needs a supported exception.
Without one, HCF reports the model-limit error before its first transmission.
Neither duration refusal calls transmission-failure procedures.

A zero limit does not mean zero permitted airtime or an unlimited burst.
It permits one MSDU or MMPDU with the conventional fragments that the current path supports.
It also permits one existing A-MSDU where the selected sequence supports it.
The cost includes required ACKs and protection.
A second unrelated unit cannot use that allowance.

Clause 10.23.2.9 permits specific positive-limit overruns.
This first path supports these cases:

| Exception | Evidence that the check needs |
| --- | --- |
| MPDU retransmission | The same frame identity and byte length as its first transmission. |
| Initial conventional fragment after an earlier fragment retransmission | Common unit identity and actual retransmission history for that earlier fragment. |
| Conventional fragment from a unit with 16 fragments | Common unit identity and the original fragment count. |
| Group-addressed MPDU | The selected frame's actual group receiver address. |

The conventional-fragment rows correspond to the clause's nondynamic fragment cases.
Each exception permits at most one Data or Management transmission in the whole TXOP.
The check counts actual transmissions and any projected transmissions in the candidate exchange.
It must not infer an exception from the Retry or More Fragments bit alone.
The initial-MSDU exception under a Block Ack agreement belongs to Plan 9.

`InProgressFrames` keeps original identity, lengths, fragment count, and earlier actual transmission history.
The data service supplies the original fragment count when it creates the fragment set.
Actual transmission updates that history; preparation does not.
Common history remains valid while any fragment of that unit remains retained.
The owner clears it after the last fragment leaves through success, drop, or lifecycle cleanup.

Refused fragments retain their content, length, sequence number, fragment number, and order.
The implementation must not shorten a retransmission or exceed 16 fragments to force a fit.
A configured byte threshold does not guarantee that every exchange fits at every mode or protection choice.

### 5.4 Reserve one next exchange and calculate Duration/ID

The active exchange must know its accepted next exchange before it reserves medium time for that exchange.
Prepare at most one next exchange with the private assumed completion from section 4.
Check its full cost against the projected completion time of the active exchange.
Retain its exact accepted steps, identities, and modes.
Prepare another next exchange only after the current next exchange becomes active.

If no next exchange receives a reservation, HCF ends the burst after the active exchange.
A later queue arrival cannot extend that completed decision.
HCF rechecks the reserved exchange against actual state and elapsed time before its first transmission.
A failed check cancels it without a replacement selection.

`SingleProtectionMechanism` continues to calculate Duration/ID.
Its new query reads the prepared active exchange and optional accepted next exchange.
It reads those records without mode selection, frame extraction, or changes to frames and tags.
The protected intervals are:

| Transmitted frame | Interval after that frame |
| --- | --- |
| RTS | CTS, the protected frame, its required response, and the applicable gaps. |
| Final Normal Ack data or management | SIFS plus ACK. |
| Final group frame without a response | Zero additional airtime. |
| Nonfinal data or management | Its required response, the next transmitted frame, that frame's required response, and the applicable gaps. |

For example, a next exchange can start with RTS/CTS before DATA/ACK.
The preceding data field includes that RTS/CTS part, not the entire next exchange.
The query needs no Duration/ID from the exchange after that next exchange.
This keeps preparation limited to the active exchange and one next exchange.

Keep `RtsTransmitStep` and its protected-frame identity.
Finalize active Duration/ID fields after the next-exchange decision.
This finalization changes no selected length, mode, or planned cost.
Preserve the serializer's units, rounding, and range at the field boundary.

### 5.5 Cancel an exact pending transmission

`Tx` holds a copy while it waits for an IFS timer.
The plan can become invalid before that timer expires.
For example, a mode-set change can remove the mode that the prepared frame needs.
Cancellation must remove the delayed copy before it reaches the medium.
The retained original remains with `InProgressFrames`.

Add `TxRequestId` with a MAC lifecycle epoch and a serial.
An epoch identifies one lifecycle instance; restart uses a new epoch.
The MAC allocates the identity before any Tx callback can occur.
Old identities cannot refer to a new request after restart.

The shared Tx contract needs these operations:

| Owner | Proposed operation and purpose |
| --- | --- |
| `ITx` / `Tx` | The identified transmit operation replaces unidentified overloads. |
| `ITx` / `Tx` | `cancelPendingTransmission(id)` removes only the matching pending request. |
| `ITx` / `Tx` | `resetForLifecycle(epoch)` clears old copies and callbacks during lifecycle cleanup. |
| Tx callback | `isTransmissionPermitted(id)` performs the final check before the exact request reaches the medium. |
| Tx callback | `transmissionCanceled(id)` reports a request that the final check rejects. |
| Sequence handler | `pendingTransmissionCanceled(id)` ends the matching untransmitted steps without a failed attempt. |

Cancellation returns one of these outcomes:

| Result | Effect |
| --- | --- |
| `CANCELED` | The matching request still waits for IFS. Cancel its timer and delete only the Tx copy. |
| `TOO_LATE` | The matching request is already on air. Preserve its normal completion and response or failure procedure. |
| `NOT_FOUND` | No request matches. Preserve any other current request. |

Clear pending state, callback, and identity before a cancellation notification.
Explicit cancellation produces no transmission-complete callback.
The caller reports a successful explicit cancellation to the matching handler once.
The handler preserves actual history for any earlier transmitted part of the exchange.
It retains untransmitted data without ACK progress, retry increment, or rate feedback.

The final send check also runs for zero IFS.
It checks the current lifecycle, plan, step, frame identity, mode inputs, and cost of the unexecuted prepared steps.
The cost starts at the pending preamble and excludes any IFS that already elapsed.
The check performs no new mode selection or preparation.
If its callback replaces or cancels the request, Tx must recheck identity before the send-down call.

Plan replacement, abort, stop, and incompatible mode-set changes invalidate future originator steps.
CTS or ACK failure discards the reserved next exchange.
An on-air request or an existing response wait follows its current completion or actual abort procedure.
Later plans add rate-context, PHY-epoch, and agreement-generation checks when those owners exist.

### 5.6 Keep objects alive across callbacks

A synchronous callback can invalidate a plan before the function that uses its step returns.
Immediate deletion would leave that function with a reference to a deleted object.
The handler must therefore keep the context, step, metadata, and sequence tree alive until all such callbacks return.

The handler tracks callback depth at each nested call.
Cancellation invalidates the plan identity immediately, so a nested call cannot execute another step from it.
Objects with active borrows wait for disposal until callback depth returns to zero.
HCF and DCF also defer handler destruction until those callbacks return.

Tx commits its phase and copy ownership before notification.
The protocol owners commit actual ACK state and transmitted-mode history before a completion callback selects another exchange.
After each callback, the caller checks lifecycle, sequence generation, and request identity again.
A changed identity stops further work on the old exchange.
Pending Tx copies must disappear before their staged originals or plan metadata disappear.

A required recipient response has its own accepted context.
HCF and DCF keep that copied context separate from originator plans.
Ordinary reconfiguration preserves that response until transmission completes.
A conflicting change waits; it cannot cancel or postpone the required response.
Actual stop, crash, or PHY abort uses the existing lifecycle failure procedure.
This plan retains the current PHY response procedure.

## 6. Small implementation series

Each row defines one change with its direct checks.
Declarations and their implementations must enter the same compilable change.

| Change | Main files | Result and verification |
| --- | --- | --- |
| Prepare existing sequences | Sequence contracts, generic/primitive classes, step/context records, `FrameSequencePlan.h` | One recorded branch and complete duration. Unit cases check sums, empty alternatives, step ownership, and legacy execution. |
| Stage frames and prepare one next exchange | `InProgressFrames`, ACK snapshot contracts/implementations, HCF selector adapters, handler/context | Extraction occurs once. Only private completion facts change. Unit/module cases inspect real state and retained identities. |
| Execute and cancel prepared transmissions | Handler callback, HCF/DCF, `ITx`, `Tx`, protection mechanism | Actual transmission uses accepted values. Module cases inspect Duration/ID, pending cancellation, and callback lifetime. |
| Enforce complete admission | `TxopProcedure`, `HcfFs`, fragment history, documentation | The real three-frame case obeys its budget. Direct cases cover exact limits, refusal, zero limits, exceptions, and legacy consumers. |

Use the existing rate and PHY contracts for this series.
The series adds no new NED parameter, module, or frame format.
Block Ack snapshots and aggregate APIs belong to later plans.

Changes to shared interfaces include all generic classes and every primitive.
The DCF/HCF/HT/PCF/MCF trees retain their existing constructor grammar.
Adapt HCF, DCF, PCF/MCF stubs, direct Tx callers, and direct step constructors to changed declarations.
PCF/MCF stubs report unsupported procedures explicitly.
Update the three sequence fixtures in `FrameSequence_1.test`.

External sequence, step, handler, ACK, and Tx implementations need migration entries and a rebuild.
Document the supported first path, cancellation callbacks, and initial-size limit in the migration guide and `WHATSNEW`.
Existing TXOP duration statistics retain their elapsed-time meaning.

## 7. Direct verification

### 7.1 Test targets and production path

The new targets below do not exist at the source commit.
Their tests must use the real owners for the behavior that they claim.

| Test | Required observation |
| --- | --- |
| `Ieee80211TxopDuration_1.test` — new unit | Independent complete sums, one mode/branch selection, leading SIFS, zero-limit allowances, and exception history. |
| `Ieee80211PreparedOriginatorPolicy_1.test` — new unit | Every ACK phase survives the snapshot. Inspection preserves order and owner state. Only the private completion copy advances. |
| `Ieee80211TxopExchange_1.test` — new module | Real EDCA, HCF handler, Tx, radios, and recipient produce one TXOP with three separate Data/ACK exchanges. |
| `Ieee80211PreparedCancellation_1.test` — new module | Pending IFS cancellation, mode-set changes, callbacks, abort, and restart preserve ownership and prevent stale transmission. |
| `Ieee80211RetryMode_1.test` — new module | After an ACK timeout, HCF replaces the retained mode tag. The predicted ACK, radio airtime, and Duration/ID match the new mode. |

`N_TxopBurst.test` checks short gaps between frames.
It does not establish complete admission or exact TXOP membership.
Keep it as a related regression, not the sole acceptance test.

The main production case uses zero propagation and a supported non-HT OFDM mode.
Both peers use response policies whose actual mode and length match the prediction.
Set `isBlockAckSupported = false` and both aggregation policy typenames to the empty string.
Choose unfragmented MPDUs below the RTS threshold for the six-PPDU case.
Add another case above that threshold for RTS/CTS/DATA/ACK.
Use seed 0 for new deterministic module cases.
Execute all parameter runs, including both rate-change directions in `Ieee80211RetryMode_1.test`.

### 7.2 Cases that expose incorrect accounting or state changes

| Case | Required check |
| --- | --- |
| Exact budget | Test available time one tick below, equal to, and above the complete cost for initial and later exchanges. |
| TXNAV | Test its strict reservation boundary independently from the TXOP budget. |
| Third-exchange refusal | Transmit only the first two exchanges in this TXOP. Transmit the retained third frame after later channel access without false retry progress. |
| Stable selection | A test provider changes its mode on a second query. The frame must use its first selected mode without another query. |
| Rate change on retry | Suppress the first ACK. Check 6-to-24 Mbps and 24-to-6 Mbps retries through real rate selection and radios. Compare prepared and transmitted modes, airtime, Duration/ID, and TXOP duration. |
| Correct successor | Queue another frame with a different length and mode. Duration/ID must use the accepted next frame. |
| Complete sums | Check management/ACK, group traffic, DATA/ACK, and RTS/CTS/DATA/ACK independently. |
| One extraction | Repeat preparation after refusal. Extraction, fragmentation, sequence allocation, and ACK registration must not repeat. |
| Zero limit | Check one unfragmented unit, one supported fragment set, and one existing A-MSDU. Reject an unrelated second unit. |
| Exceptions | Check each supported exception with valid evidence and one invalid condition. Check the whole-TXOP Data/Management count. |
| Fragment history | Compare 15 and 16 fragments. Preserve earlier retransmission history after that fragment leaves. Clear it after the last fragment leaves. |
| Refused fragments | Compare content, length, sequence control, and order before and after refusal. |
| Protection fields | Check RTS, final ACK data, final group data, accepted/refused next exchanges, and a next exchange that starts with RTS. |
| ACK/CTS loss | After reservation, fail the actual response. The reserved next exchange must not transmit. |
| Pending cancellation | Check matching, wrong, and stale identifiers during IFS. Check zero IFS and a request already on air. |
| Equal-time cancellation | Reverse event insertion order. Inspect actual committed state and charge elapsed IFS only once. |
| Callback replacement | Invalidate the plan from a synchronous callback. Keep borrowed objects alive and prevent nested execution or duplicate completion. |
| Lifecycle | Abort or stop during an exchange. Restart must reject old timers and callbacks. |
| Required response | Apply ordinary reconfiguration during an accepted response. Preserve its context; use the failure procedure for actual lifecycle abort. |
| Shared consumers | Exercise DCF's null-metadata legacy path and the unchanged HCF path with Block Ack support enabled. |

The shared-consumer check does not require a repair of pre-existing Block Ack failures.
Record such failures with compared revisions and cause under the index's verification rules.
A pre-existing failure is not a successful regression result.

### 7.3 Initial-refusal diagnostics

The duration check returns an admission result before HCF reports `cRuntimeError` for the initial model limit.
A pure duration query does not throw merely because the budget is too small.
The error must identify these facts:

- Peer and TID where present, sequence number, and fragment number.
- Frame lengths, prepared modes, ACK policy, and protection choice.
- TXOP limit, complete cost, available time, and response contribution.
- Failed condition and exception evidence.
- Fragmentation policy and configured byte threshold, or their absence.

Retain staged frames until their normal owner cleanup.
Check that rejection emits no `datarateSelected`, retry increment, or repeated identical contention after a terminal error.
Check that generated control frames are disposed of once.
The implementation must not change rates, limits, or fragmentation settings automatically to hide that error.

### 7.4 Commands after implementation

Run from the checkout root after the new targets exist:

```sh
make MODE=debug -j$(nproc)
inet_run_unit_tests -m debug -f '(FrameSequence_1|Ieee80211TxopProcedure_1|Ieee80211TxopDuration_1|Ieee80211PreparedOriginatorPolicy_1)\.test'
inet_run_module_tests -m debug -f '(Ieee80211TxopExchange_1|Ieee80211PreparedCancellation_1|Ieee80211RetryMode_1|Ieee80211MgmtApHcfRtsTimeout_1|Ieee80211HcfManagementRecovery_1)\.test'
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '/(Legacy_DataAck|Legacy_RtsCts|Legacy_Fragmentation|N_TxopBurst)\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-interfaces.sh
git diff --check
```

Runner help confirms debug mode and explicit filters.
Record case counts, exit statuses, seeds, and evidence paths.
A zero-case selection supplies no test evidence.
Use the [gate procedure](../../doc/project/guide/run-the-gates.md) before a push.
Baseline changes need their separate recorded permission.
No build or simulation ran for this plan rewrite.

## 8. Standards, review, and approval

The prior local retrieval used IEEE Std 802.11-2024, corpus document ID `ieee80211-2024`.
The table below retains the normative source locators for this plan.
These normative clauses govern this first plan:

| Clause and canonical node | Physical PDF pages | Use |
| --- | --- | --- |
| `ieee80211-2024:clause:10.23.2.8` | 2010–2012 | Same-AC continuation, SIFS, and medium reservation conditions. |
| `ieee80211-2024:clause:10.23.2.9` | 2012–2014 | Complete duration, zero-limit allowances, fragmentation, and overrun exceptions. |
| `ieee80211-2024:clause:9.2.5.2` | 710–712 | Single-protection Duration/ID. |
| `ieee80211-2024:clause:10.4` | 1935–1936 | Conventional fragment identity, order, and unchanged retransmission content. |

The readability rewrite adds no standard allowance.
Response-rate rules and PHY timing keep their separate owner plans and coverage limits.
The public additions serve real preparation, execution, or cancellation consumers on this first path.

The design follows [reuse](../../doc/project/rule/architecture.md#ar-ext-reuse) and [contract purity](../../doc/project/rule/architecture.md#ar-org-contract-purity).
State ownership follows [the state rule](../../doc/project/rule/quality.md#qr-state-owner).
The tests follow [focused evidence](../../doc/project/rule/testing.md#tr-focused-evidence).
The plan follows the project's [plan guide](../../doc/project/guide/write-an-implementation-plan.md).

The seal registry lists no protected path in this source change surface.
The common packet subsystem remains outside it.
Approval must identify this plan revision.
Approval of this first plan does not approve a later plan.

The scope exclusions in section 2.3 retain the boundary with separate future work.
QoS Null has no declared frame subtype at the source commit.
Its representation and overrun exception remain a separate follow-up.
Separate control and Block Ack agreement work adds the applicable overrun exceptions.


## 9. Implementation record — 2026-10-04

Historical local commit `4224f059f4` contained the implementation and the retry mode test before the PR series rewrite.
The user authorized execution of split revision 3.
The user also requested the retry regression test and its inclusion in that commit.
This plan stays under `pending/` until the change lands.

### Behavior claim

HCF prepares and admits each complete exchange when `isBlockAckSupported` is false.
The existing sequence tree supplies the selected steps.
Execution uses those exact frames, modes, lengths, ACK policy, and Duration/ID values.
Admission includes the required responses and the leading SIFS of each continuation.
A separate TXNAV check reads this station's actual transmitted reservation.

The frame store retains refused originals and actual fragment history.
Private completion changes only the preparation snapshot.
Exact request identities prevent stale delayed copies after cancellation or lifecycle cleanup.
Callback scopes retain borrowed contexts, steps, and originals until all synchronous calls return.
An invalidated on-air request retains its response wait.

The duration guarantee keeps the timing assumptions in section 2.2.
The model still reports an oversized initial exchange without a supported exception.
Automatic fragmentation by airtime, response-rate reform, and Block Ack preparation remain outside this change.

### Required integration details

- `Contention` clears its old callback before it reports a channel grant.
  Zero-IFS cancellation can start new contention inside that callback.
  The former cleanup erased the replacement callback.
- `InProgressFrames` calls `IInProgressFramesCallback` before frame removal.
  This direct call follows `AR-COM-DIRECT` and `AR-COM-NOTIFY`.
  Context references defer deletion of borrowed originals.
- Tx and handler callback scopes share `beginCallback()` and `endCallback()`.
  These scopes protect delayed permission checks as well as zero-IFS calls.
- The handler reports `frameSequenceStarted()` before synchronous execution.
  The module tests check this order before each actual transmission.
- Response timeout policies accept the already selected response mode.
  These methods preserve configured timeout overrides without another rate query.

These changes implement the approved ownership and cancellation requirements.
They add no NED parameter, module, or frame format.
The migration guide lists the affected external interfaces.

### Focused evidence

All commands use this worktree:
`/home/user/omnetpp_ws/inet-ieee80211-txop-boundaries-rate-selection`.
The environment uses `source setenv -q` before build and test commands.
The final debug build completed with exit status 0.

```sh
make MODE=debug -j8
inet_run_unit_tests -m debug -f '(FrameSequence_1|Ieee80211TxopProcedure_1|Ieee80211TxopDuration_1|Ieee80211PreparedOriginatorPolicy_1)\.test'
inet_run_module_tests -m debug -f '(Ieee80211TxopExchange_1|Ieee80211PreparedCancellation_1|Ieee80211MgmtApHcfRtsTimeout_1|Ieee80211HcfManagementRecovery_1)\.test'
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '/(Legacy_DataAck|Legacy_RtsCts|Legacy_Fragmentation|N_TxopBurst|N_BlockAck)\.test$'
```

| Target group | Result | Runs and seeds |
| --- | --- | --- |
| Four unit targets | 4 PASS; exit 0 | One default run per target |
| `Ieee80211TxopExchange_1` | PASS | General runs 0–8; seed 0 |
| `Ieee80211PreparedCancellation_1` | PASS | General runs 0–14; seed 0 |
| `Ieee80211MgmtApHcfRtsTimeout_1` | PASS | 12 runs; seeds 0, 1, 2 |
| `Ieee80211HcfManagementRecovery_1` | PASS | 24 runs; seeds 0, 1, 2 |
| Five protocol targets | 5 PASS; exit 0 | Default run 0 per target |

The module runner completed all four targets with exit status 0.
Its 60 runs include all selected parameter combinations.
`N_BlockAck` explicitly enables Block Ack support and exercises the retained legacy HCF path.
The three legacy protocol targets exercise DCF.
No recorded expectation or fingerprint changed.

The exchange cases cover these distinct paths:

1. Three separate Data/ACK exchanges within one TXOP.
2. Third-exchange refusal and later channel access.
3. RTS/CTS protection and its transmitted reservation.
4. A zero limit with unrelated units.
5. A zero limit with conventional fragments.
6. A zero limit with an existing A-MSDU.
7. Group traffic without an ACK.
8. Management frames with ACKs.
9. Initial refusal before any transmission or retry progress.

The cancellation cases cover these conditions:

1. Matching, wrong, and stale request identities during SIFS.
2. Refusal at zero IFS.
3. A mode-set change during pending IFS.
4. Cancellation after transmission starts.
5. Stop and restart during pending IFS.
6. Ordinary radio configuration during a required response.
7. Cancellation first at the IFS deadline.
8. Transmission first at the same deadline.
9. ACK loss after a successor reservation.
10. CTS loss after a successor reservation.
11. Removal of a retained frame during pending IFS.
12. Cancellation inside the zero-IFS permission callback.
13. Stop during an actual transmission and later restart.
14. Cancellation inside a delayed permission callback.
15. A mode-set change during an actual transmission, with the ACK wait preserved.

The unit cases independently check exact duration boundaries and strict TXNAV boundaries.
They check one-time selection, successor identity, fragment history, ACK phases, and private projection.
They also reject empty alternatives, unsupported preparation inputs, and nested repetition without a distinct candidate.

### Retry mode regression evidence

The additional [module test](../../tests/module/Ieee80211RetryMode_1.test) passed after the original four-target module campaign.
The test uses deterministic rate control that changes its rate after real HCF failure feedback.
The recipient suppresses the first ACK to cause the timeout.
Production rate selection prepares the retry of the same MPDU.

The assertions check these facts:

1. Preparation leaves the original packet's previous mode tag intact until HCF commits the new mode.
2. HCF updates the retained tag, and the radio transmits at the selected retry mode.
3. The prepared ACK mode and airtime match the recipient's actual radio transmission.
4. Data airtime, Duration/ID, and complete exchange duration match the prepared values.
5. The successful retry finishes within the 500-microsecond TXOP limit.

The following command ran from the checkout root in debug mode with exit status 0:

```sh
inet_run_module_tests -m debug -f 'Ieee80211RetryMode_1.test'
```

General run 0 checks 6-to-24 Mbps; run 1 checks 24-to-6 Mbps.
Both runs use seed 0 and passed.
The output is `tests/module/work/Ieee80211RetryMode_1/test.out`.
The runner log is `/tmp/inet-retry-mode-test.log`.
This adds one target and two runs to the earlier evidence: five module targets and 62 runs in total.

The test uses zero propagation, Normal ACK, no RTS, and no aggregation.
It checks consistency between preparation and transmission; it does not validate other response-rate policies or PHY families.
No production source change was necessary for this test.

### Rule checks and self-audit

```sh
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-interfaces.sh
git diff --check
```

The architecture check passed with exit status 0.
The whitespace check passed with exit status 0.
The naming check returned 1 for 32 existing mechanical candidates.
The interface check returned 1 for 15 existing violations.
An isolated archive of the pinned commit produced the same findings.
The interface check also accepts the new callback interface.
`AV-CONTRACT-02` still records the existing step-interface method bodies.

The final C++ scan checked all 112 translation units in the IEEE 802.11 subtree.
It returned 1 for 4,487 distinct diagnostics outside changed lines.
It reported no compiler errors and no findings on changed lines.
This result does not classify every unchanged-line diagnostic as a verified baseline finding.

The compilation database uses the actual commands from `make -C src -n -B MODE=debug`.
A read-only wrapper runs four independent `clang-tidy` processes at once.
It preserves the gate's checks and performs no source edits.
The evidence directory contains the wrapper, compilation database, diagnostic summary, and complete compressed output.
The final scan used a fixed source tree.
An earlier scan overlapped edits and supplies no final evidence.

```sh
PATH=/tmp/txop-tidy-tools:$PATH INET_COMPILE_DB=/tmp/txop-compdb \
    doc/project/enforcement/check-cpp.sh src/inet/linklayer/ieee80211
```

The self-audit covered the C++, OMNeT++, INET, and IEEE 802.11 references from `inet-code-authoring`.
It traced ownership, callback replacement, lifecycle epochs, same-time event order, response waits, and direct removal coordination.
The source seal registry protects no changed source path.
The common packet subsystem remains unchanged.
No independent agent review ran.

### Artifacts and limits

The evidence directory is `tests/module/work/txop-duration-evidence/20261004/`.
It contains the complete changed-path manifest, source hashes, runner summaries, individual test output, and gate comparisons.
`txop-duration-contract.md` records the implementation contract and required integration additions.
The source and test manifest excludes the pre-existing `AGENTS.md` change and unrelated plans.
`txop-cpp-diagnostics.json` maps the C++ diagnostics against the exact changed-line ranges.

The check does not claim a duration guarantee for nonzero propagation or mismatched response airtime.
The Block Ack path retains its previous behavior and has no new duration guarantee.
The release build and complete pre-push campaign remain separate checks before a push.

## 10. PR #1273 audit correction — 2026-10-04

The user requested corrections to the audit findings with: "Fix the findings".
This correction preserves the approved exchange behavior and scope.
The TXOP commit must retain this plan with a `Plan:` trailer.
References to separate, uncommitted plans no longer form part of this document's links.
Section 9 retains the original evidence as a historical record.

### Teardown ownership

An active `FrameSequenceContext` borrows its `InProgressFrames` owner.
OMNeT++ deletes child modules before the parent module destructor.
The former HCF and DCF destructors thus released a frame reference after its owner ceased to exist.
This defect occurs at simulation termination or node deletion during an active exchange.

HCF and DCF now delete their sequence handler in `preDelete()`.
OMNeT++ calls this hook while all frame stores still exist.
The handler releases active and retired contexts under the coordination module context.
The hook clears the handler pointer, so the later destructor remains safe.
Ordinary stop, restart, cancellation, and callback retention keep their existing paths.
Two public getters without callers are removed: `getActiveFirstStep()` and `getOffset()`.

### Direct regression evidence

`Ieee80211ExchangeTeardown_1.test` uses real radios with DCF, legacy HCF, and prepared HCF.
Each profile ends the simulation or deletes the sender during transmission or a response wait.
A model-change listener checks the frame store before its destructor.
The test rejects a live context that still borrows that store.
The first case failed against the former library at this exact assertion.
All 12 cases pass with the correction, with seed 0 and no ownership warnings.
The fixture discards received payloads at the MAC boundary because it supplies no upper-layer protocol header.

The final focused commands ran from the checkout root:

```bash
source setenv -q
make MODE=debug -j8
make MODE=release -j8
inet_run_unit_tests -m debug -f '(FrameSequence_1|Ieee80211TxopProcedure_1|Ieee80211TxopDuration_1|Ieee80211PreparedOriginatorPolicy_1)\.test'
inet_run_module_tests -m debug -f '(Ieee80211(ExchangeTeardown|TxopExchange|PreparedCancellation|RetryMode|LifecycleGroup)_1|Quic(StreamPayload|OrderedPayload)_1)\.test'
inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '/(Legacy_DataAck|Legacy_RtsCts|Legacy_Fragmentation|N_TxopBurst|N_BlockAck)\.test$'
```

Both builds pass. Four unit targets, seven module targets with 45 runs, and five protocol targets pass.
The local logs reside under `audit/pull-request/pr-1273-fix-evidence/`.
No fingerprint or statistical baseline changes form part of this correction.
The correction adds no new duration guarantee or PHY timing behavior.
The tests prove the specified teardown paths; they do not establish arbitrary external callback behavior during node deletion.
