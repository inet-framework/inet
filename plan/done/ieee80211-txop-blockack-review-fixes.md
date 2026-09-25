# Plan to fix the TXOP, Block Ack, and rate-state defects

Status: complete.
Plan date: 2026-09-25.
Completion date: 2026-09-25.
Reviewed head: `96008e2c7c5be2bc1303231e2c0125ba07388eac`.

The [consolidated report](../../audit/pull-request/branch-96008e2c7c-consolidated.md) defines defects B1 through B7 and preserves their evidence.
This plan covers those seven defects and their direct regression tests.
The [Devin assessment](../../audit/pull-request/branch-96008e2c7c-devin-assessment.md) adds B6 and B7 and resolves each submitted comment.
The [implementation report](../../audit/pull-request/branch-96008e2c7c-implementation.md) records the fix commits, direct tests, and fingerprint cause.
All seven source fixes and the review controls are implemented.
The B6 trace corrects the original diagnosis: peer cleanup already exists, but its notification precedes the station transition.
The debug tests pass nine unit fixtures, 148 scenarios across four module fixtures, and five protocol fixtures.
The exact [fingerprint patch](../../audit/pull-request/branch-96008e2c7c-fingerprints.patch) changes three CSV rows because B4 corrects false acknowledgments.
The user approved this exact patch on 2026-09-25 with the reply “approved”.
All five scoped fingerprint cases pass after the approved update; the command exits 0.
Commit `49a59d53e3` contains B4 and all eight approved fingerprint values.
The source tree matches the tested source; the final update changes only the approved baselines and plan records.
The scoped architecture, naming, source-seal, commit, classification, and whitespace checks pass.
The implementation report retains all commands, logs, final commit identifiers, and the separate receive-buffer observation.
Release compilation and whole-project gates remain prerequisites for a future push.

## Completed fix commits

| Item | Commit |
| --- | --- |
| Accepted plan | `32835e8603` |
| B1: teardown identity and deferred replacement | `22b386c191` |
| B2: recipient inactivity refresh | `c7dbc3fcbf` |
| B3: internal collision with a pending BAR | `498502cc4b` |
| B4: missing-frame acknowledgment and approved fingerprints | `49a59d53e3` |
| B5: accepted timeout interval | `b7332f9030` |
| B6: AP teardown notification order | `a4c184054c` |
| B7: canceled sequence signals | `6aa354b57f` |
| Related B1 duplicate-packet ownership | `3a4b28bb7b` |
| Review controls | `13a74a8b8e` |

The following sections retain the implementation contracts and acceptance criteria for this completed work.

## B3 implementation contract — validated before source edits

HCF classifies the losing exchange with the same BAR policy as the planner after candidate preparation.
The recovery procedure updates the category retry counter and contention window for a BAR collision.
It does not change any data retry counter or acknowledgment state.
HCF requests the channel again; stop retains the outstanding data for restart.
The change touches HCF and `QosRecoveryProcedure`, with no wire, NED, or initialization changes.
IEEE 802.11-2024, 10.23.2.2, pages 1999–2001, defines the category update for an internal collision.
The source locator is `ieee80211-2024@8315619:8323576`; PDF inspection was unnecessary.
The new `Ieee80211BlockAckLoss_1.test` uses real radios for deferral, collision, restart, and successful delivery.
This fixture replaces the planned extension of `TxopExchange` for B3 and will also cover B4.
Its first scenario reproduces the null dereference in `Hcf::handleInternalCollision` before this fix.

## B1 implementation contract — validated before source edits

The handlers retain a teardown record keyed by peer and TID within each agreement direction.
The record holds a DELBA transaction ID and at most one deferred ADDBA request.
The existing management transaction tag carries the initial packet tree ID through fragmentation and packet copies.
HCF reports terminal DELBA outcomes through direct handler calls at acknowledgment, queue drop, or retry exhaustion.
Packet ownership remains with the MAC queues; the handler retains only the identifier and immutable request header.
Expiry removes the active agreement before any callback.
Replacement setup waits for the matching terminal outcome, including all DELBA retries.
Stop retains the teardown record because it retains queued packets. Handler destruction releases the retained headers.
Late and repeated terminal callbacks cannot consume a newer record with another transaction ID.
An incoming DELBA cannot terminate a pending originator agreement that has no accepted response.
The recipient lookup uses the transmitting peer when a DELBA arrives.

The change surface includes both handler interfaces and implementations, HCF outcome dispatch, and the two Block Ack fixtures.
No wire field, serializer, NED module, or initialization stage changes.
The source paths remain unsealed. The C++, simulator, packet, and WLAN references apply.
Direct verification uses the existing completion unit fixture and inactivity module fixture, including new scenario 16.
The regression invokes the real queues, radios, handlers, and replacement exchange.
Additional unit checks cover late completion identity and independent TIDs without reliance on contention timing.

The lost-DELBA-ACK case also exposes an owned-packet leak in `RecipientQosMacDataService::managementFrameReceived()`.
That method takes the packet but returns without deletion when duplicate removal rejects it.
The required B1 scope now includes deletion at that terminal duplicate path.
Data duplicates already use that ownership contract. The additional path remains unsealed.
Inactivity scenarios 24 and 25 cover retry exhaustion in both agreement directions.

## B5 implementation contract — validated before source edits

The recipient handler builds the accepted response before it creates an agreement.
The new agreement receives the response timeout. Duplicate requests retain the current agreement interval and deadline.
Response retries only reschedule the shared timer; they cannot change the interval.
The accepted zero interval means `SIMTIME_MAX`, as figure 9-152 of IEEE 802.11-2024 specifies.
The figure occupies page 851, locator `ieee80211-2024@3893201:3893546`; no PDF inspection was necessary.
This patch changes the recipient handler signature and implementation plus both timeout parameter comments.
No wire layout, serializer, lifecycle, or initialization changes apply.
Both timeout regression fixtures reproduce different stored intervals before this fix.
The completion unit test covers duplicate requests; inactivity scenarios 17–20 cover real negotiation and idle expiry.

B3 passes its initial three radio scenarios in commit `498502cc4b`.

## B4 implementation contract — validated before source edits

`BlockAckRecord` owns the first sequence number whose acknowledgment state remains explicit.
The recipient agreement supplies its initial sequence number to the record constructor.
`BlockAckReordering::passedUp()` remains the sole production caller that advances this boundary through `removeAckStates()`.
The boundary only advances under cyclic comparison; map order does not define it.
Missing fragments at or after the boundary return false, even when the map is empty.
Frames before the boundary retain the existing model rule for old-frame acknowledgment.
Agreement deletion also deletes the record; retries and duplicate frames cannot move the boundary backward.
The patch touches the record header, record implementation, recipient constructor, unit fixture, and radio fixture.
It changes no wire layout, NED module, initialization stage, or other Block Ack variant.
The 2024 corpus omits the former legacy Basic Block Ack procedure; it does not establish that old-frame rule independently.
This fix preserves that existing model rule and corrects the distinction between old and missing frames.
The unit test fails on a fresh missing frame before the fix.
Radio scenario 3 also fails because the sender does not retry the discarded payload.
Direct verification covers both fixtures in debug mode, with lost-response and no-loss controls.

## Order and dependencies

| Step | Defect | Result | Dependency |
| --- | --- | --- | --- |
| 0 | All | Preserve reproductions and define direct tests | None |
| 1 | B1 | Old DELBA work cannot destroy a replacement agreement | Step 0 |
| 2 | B3 | A pending BAR survives an internal collision | Step 0 |
| 3 | B4 | Missing data remains unacknowledged and retransmits | Step 0 |
| 4 | B5 | Both peers use the accepted timeout interval | Step 0 |
| 5 | B2 | Qualifying receive activity refreshes the correct deadline | Step 4 |
| 6 | B6 | AP state changes before peer cleanup emits a notification | Step 0 |
| 7 | B7 | Canceled grants leave balanced sequence signals | Step 0 |
| 8 | Review controls | Document intentional policies and fill specific test gaps | Steps 2 and 7 |
| 9 | All | Combined tests, gates, and review confirm the fixes | Steps 1–8 |

The order removes the two crashes first, followed by silent data loss and timeout errors.
B5 precedes B2 because a refreshed deadline must use the correct accepted interval.
B1 also protects replacement setup during later expiry tests.
Each fix should form one reviewable commit with its direct tests.

## Shared implementation contract

Follow the [contribution guide](../../doc/project/guide/contribute-a-change.md) and [test rules](../../doc/project/rule/testing.md).
The current [seal registry](../../doc/project/audit/seal-list.md) leaves the proposed IEEE 802.11 files open.
Resolve the current protection status again before source edits.

Use these state owners throughout the work:

- Agreement handlers own agreement identity, accepted parameters, and agreement lifetime.
- Agreement objects own absolute inactivity deadlines.
- `Hcf` owns the shared timer and dispatches frame outcomes.
- Channel-access and recovery procedures own contention and retry state.
- `BlockAckRecord` owns acknowledgment history and its retained sequence range.
- `BlockAckReordering` owns the receive buffer and notifies record removal.
- AP management owns association transitions and commits peer policy through the MIB.
- `Hcf` owns its sequence start and finish signals, including canceled grants.

Use complete identities: peer address, TID, agreement direction, and transaction identity where late work requires it.
Keep timeout intervals separate from absolute deadlines.
Keep the zero-timeout convention: `SIMTIME_MAX` represents a disabled deadline.
Keep sequence comparisons cyclic across the 12-bit sequence-number boundary.

Before each fix, complete its implementation contract from the actual call paths.
Include affected consumers, retry outcomes, timeout outcomes, cancellation, and stop/restart behavior.
Resolve the design tasks below before source edits for that fix.
Record any required scope change in this plan before work on the additional target.

## Step 0 — Preserve the failures as direct tests

1. Preserve the reviewed head and the diagnostic commands with their logs.
2. Transfer each diagnostic into a self-contained unit, module, or protocol fixture.
3. Confirm that each new regression fails at its intended assertion against the reviewed source.
4. Confirm that each control case passes against that same source.
5. Record the exact commands, configurations, runs, seed, exit status, and artifacts.

Do not include generated files from another fixture's `work/` directory in permanent tests.
Use declared test support or complete fixture inputs.
Keep seed 0 for the deterministic reproductions.
Use explicit parameter cases for zero timeout, collision timing, and sequence wrap.
These tests establish specific behavior; they do not estimate failure frequency or throughput.

| Defect | Permanent test target | Expected failure before its fix |
| --- | --- | --- |
| B1 | Extend `Ieee80211BlockAckCompletion_1.test` and `Ieee80211BlockAckInactivity_1.test` | Replacement agreement disappears; real response aborts |
| B2 | Extend `Ieee80211BlockAckInactivity_1.test` | Recipient deadline stays fixed; active agreement expires |
| B3 | Add `Ieee80211BlockAckLoss_1.test` | BAR deferral followed by an internal collision crashes |
| B4 | Add `Ieee80211BlockAckRecord_1.test` and `Ieee80211BlockAckLoss_1.test` | Empty record acknowledges missing data; delivery count is short |
| B5 | Extend `Ieee80211BlockAckCompletion_1.test` and `Ieee80211BlockAckInactivity_1.test` | Accepted zero timeout still produces recipient expiry |
| B6 | Extend `Ieee80211MgmtApLifecycle_1.test` with disassociation and acknowledged refusal | Rate-removal listeners observe stale station or transaction state |
| B7 | Correct and extend scenarios 24 and 25 in `Ieee80211TxopExchange_1.test` | The active-sequence count remains positive after cancellation |

The two new B4 test names describe planned files. They do not exist at the reviewed head.
Unit tests prove record and handler state rules.
Module tests must reach the production dispatch and observe packet delivery, queue state, or timer state.
Use protocol tests when an acceptance claim requires the exact frame order.

## Step 1 — B1: isolate old teardown from replacement setup

**Invariant:** a DELBA for a retired agreement cannot remove a replacement agreement or invalidate its setup response.

**Primary targets:** both agreement handlers under `src/inet/linklayer/ieee80211/mac/blockack/`, including their headers.
Inspect `coordinationfunction/Hcf.cc` and the handler interfaces under `mac/contract/` for outcome propagation.
The full production path is expiry, management queue insertion, transmission completion, retry outcome, and subsequent ADDBA processing.

Implementation tasks:

1. Trace every DELBA producer and every final management-frame outcome.
2. Define how the handler identifies retired teardown work after a replacement becomes possible.
3. Remove termination of an arbitrary current agreement through peer and TID alone at old transmit completion.
4. Establish an order that prevents old DELBA transmissions or retries after replacement setup.
5. Release any teardown state on acknowledgment, retry exhaustion, cancellation, and lifecycle cleanup.
6. Apply the same contract to originator and recipient completion paths.

A local generation check can protect local state, but DELBA carries no generation identifier on the wire.
The design must also control the order of teardown and replacement setup at the peer.
First transmission completion is not the final retry outcome.
Prefer the existing queue and outcome mechanisms when they can enforce the required order.
Add handler-owned transaction state only if the traced paths require it.

Acceptance checks:

- Diagnostic run 1 completes through replacement setup with consistent agreements at both peers.
- A late completion never removes a replacement for the same peer and TID.
- Lost DELBA acknowledgment and retry exhaustion cannot leave setup permanently blocked.
- Repeated stop/restart and repeated completion cannot delete current state twice.
- Another peer, TID, or agreement direction remains independent.
- New data completes after recovery; absence of the original assertion alone is insufficient.

## Step 2 — B3: handle collision recovery for a pending BAR

**Invariant:** every queue state that can request channel access has a valid internal-collision path.

**Primary target:** `src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc`, especially `handleInternalCollision()`.
Inspect `framesequence/TxopExchangePlanner.cc`, `queue/InProgressFrames.*`, and `originator/OriginatorQosAckPolicy.*` for exchange selection.
Inspect `originator/QosRecoveryProcedure.*` and the channel-access owner for the required contention update.
Change these supporting files only if their current interfaces cannot express that update.

Implementation tasks:

1. Classify the exchange that caused the losing category to request the channel.
2. Handle a BAR request without an eligible data packet.
3. Update the correct contention state through its owner.
4. Preserve data whose acknowledgment remains outstanding.
5. Request channel access again while a BAR remains necessary.
6. Preserve existing data and management retry-limit behavior.

A null-pointer return alone does not satisfy this contract.
The losing category must recover and complete its exchange.
Do not mark outstanding data as a failed data transmission solely because its BAR lost an internal collision.

Acceptance checks:

- The 650-microsecond TXOP case defers its BAR, loses the forced collision, and eventually drains both categories.
- The stop-before-BAR case also drains both categories after restart.
- The control without a competing category retains its successful result.
- Outstanding data does not acquire an unrelated retry increment or drop.
- Mixed eligible data and pending BAR state selects the correct recovery path.
- Existing data and management internal-collision tests retain their expected retry behavior.
- Two peer/TID groups with unequal sizes both complete their BAR exchanges across successive grants.
- Frames already in `WAITING_FOR_BLOCK_ACK` do not displace another group that still needs a BAR.

The group test retains the existing `isOutstandingFrame()` eligibility filter.
BAR continues to reserve only its immediate response under single protection.

## Step 3 — B4: represent the receive-record boundary explicitly

**Invariant:** the acknowledgment bitmap distinguishes missing frames within the retained range from frames before that range.

**Primary targets:** `BlockAckRecord.h`, `BlockAckRecord.cc`, and `RecipientBlockAckAgreement.cc` under `mac/blockack/`.
Inspect `blockackreordering/BlockAckReordering.cc` at `passedUp()` and each caller of `removeAckStates()`.
Inspect `RecipientBlockAckProcedure::buildBlockAck()` as the bitmap consumer.
Update constructor declarations and callers if the record now needs the agreement's initial sequence number.

Implementation tasks:

1. Initialize an explicit record boundary from the accepted agreement's sequence state.
2. Return false for absent sequence and fragment entries within the retained range.
3. Advance the boundary through the existing receive-window and record-removal path.
4. Preserve the model's rule for frames before the retained range.
5. Use cyclic comparisons for sequence wrap from 4095 to 0.
6. Verify the generated Basic Block Ack bitmap through the production response path.

An empty map must not imply that every frame is old.
The map's first numeric key must not substitute for a cyclic receive-window boundary.
Confirm the applicable legacy Basic Block Ack rule before any change to old-frame acknowledgment semantics.
This fix does not add new Block Ack variants.

Acceptance checks:

- A fresh empty record reports the first missing frame as unreceived.
- An empty record after removal retains the correct boundary.
- Holes and missing fragments remain unacknowledged.
- Received fragments remain acknowledged until the defined record-removal boundary.
- Tests cover the initial sequence, boundary neighbors, and wrap from 4095 to 0.
- Diagnostic run 6 retransmits the lost payload and delivers both payloads exactly once.
- The sender drains its queue only after the missing payload succeeds.
- The no-loss control retains its result; a lost response cannot cause false successful delivery.

## Step 4 — B5: create recipient state from the accepted timeout

**Invariant:** the recipient state and the ADDBA response use the same accepted timeout interval.

**Primary target:** `src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.cc` and its header.
Inspect `RecipientBlockAckAgreement.*` for constructor inputs and deadline initialization.
Update the timeout description in `RecipientBlockAckAgreementPolicy.ned` to match the selected behavior.
Check the originator policy description for the same zero-value inconsistency.

The proposed fix preserves the current response-selection behavior.
A zero recipient policy value selects a zero response timeout.
A nonzero recipient policy value accepts the timeout in the request.
The fix makes stored recipient state use that selected response value.
A separate redesign of policy selection is outside these fixes.

Implementation tasks:

1. Select the accepted timeout before construction of recipient agreement state.
2. Use that value for both the stored agreement and the response.
3. Preserve zero as a disabled inactivity timeout.
4. Define retry and duplicate-request behavior without accidental deadline refresh or stale-state replacement.
5. Align parameter documentation with the selected zero and nonzero behavior.

Acceptance checks:

- Request 20 ms and recipient policy zero produce zero accepted timeout at both peers.
- That agreement remains active through the diagnostic's 30 ms idle interval.
- Zero request and zero policy also remain active.
- A finite request with nonzero recipient policy produces the same accepted interval in both agreement objects.
- Zero request with nonzero recipient policy still disables the timeout.
- Setup response retries do not change the accepted interval.

Compare timeout intervals rather than identical absolute deadlines.
The two peers process agreement setup at different simulation times.
Clauses 10.25.2 and 9.4.1.14 provide the accepted-value and zero-value requirements.

## Step 5 — B2: refresh the recipient deadline on qualifying reception

### Implementation contract — validated before source edits

The recipient handler owns all deadline changes, keyed by the transmitting peer and TID.
`qosFrameReceived()` refreshes only Block Ack policy data for an existing agreement.
A new `blockAckRequestReceived()` operation accepts the implemented Basic BAR header.
HCF calls it from the existing Basic BAR receive branch before the response procedure.
Both operations update the agreement deadline before they request the shared timer minimum.
Zero accepted intervals remain infinite; absent agreements and other policies leave current deadlines unchanged.
Stop retains deadlines and suppresses timer scheduling through the existing HCF callback.
The patch touches the recipient interface, handler declaration and implementation, HCF, and the two existing inactivity fixtures.
No wire, serializer, NED, or initialization changes apply.
IEEE 802.11-2024, 11.5.4, page 2566, defines the qualifying receptions.
The source locator is `ieee80211-2024@10834737:10836329`; PDF inspection was unnecessary.
The new unit assertion and continuous-traffic scenario reproduce the missing refresh before this fix.
The module cases also cover isolated BAR dispatch, idle expiry, and zero timeout.

**Invariant:** the matching agreement deadline equals the last qualifying reception time plus the accepted timeout.

**Primary targets:** `RecipientBlockAckAgreementHandler.*` and the BAR dispatch path through `Hcf.cc` and `RecipientBlockAckProcedure.cc`.
Extend `IRecipientBlockAckAgreementHandler` only if BAR reception requires a new owner-level operation.
Keep deadline mutation in the agreement owner.
Keep `Hcf::scheduleInactivityTimer()` as the shared minimum-deadline scheduler.

Implementation tasks:

1. Refresh the agreement deadline on accepted Block Ack data reception.
2. Route matching BAR reception to the same deadline owner.
3. Request the shared timer update after the stored deadline changes.
4. Preserve peer, TID, and agreement-direction checks.
5. Preserve infinite deadlines and the existing stop/restart contract.
6. Check the supported acknowledgment policies against clause 11.5.4.

Devin's deadline comment duplicates B2 and confirms the data receive path.
Add braces to `if (agreement)` so both the deadline refresh and timer callback remain inside the agreement guard.

The clause also covers Implicit Block Ack Request where that policy exists.
Do not claim support for an unimplemented policy through this timer fix.
The originator's existing refresh on Block Ack reception remains a required control.

Acceptance checks:

- Continuous data at 1 ms intervals keeps the finite-timeout agreement active.
- BAR-only qualifying activity also refreshes the recipient deadline.
- Traffic for another peer or TID does not extend the deadline.
- Normal acknowledgment traffic does not count as Block Ack activity.
- Expiry occurs once at the first due timer event after a full idle interval.
- Zero-timeout agreements retain `SIMTIME_MAX` after reception.
- The shared timer selects the earliest deadline across both agreement owners.
- Stop/restart restores that minimum and expires overdue agreements once.

## Step 6 — B6: remove obsolete peer state on association teardown

### Corrected implementation contract — validated before source edits

`Ieee80211Mib::releaseAssociationId()` already removes peer HT capabilities and rates.
Thus the original stale-rate claim is incorrect for an associated station.
The real defect is notification order: rate removal emits its signal before AP management changes station status.
The acknowledged-refusal path also emits that signal before it clears the pending transaction.
The new observer test reproduces the stale station status during disassociation.
AP management must commit authenticated status and clear pending state before it calls `releaseAssociationId()`.
The disassociation path also removes any peer rate entry when the station already lacks an association ID.
The patch touches only the two planned transitions in `Ieee80211MgmtAp.cc` and the lifecycle fixture.
Other peers, BSS rates, signal names, and initialization stages remain unchanged.
The test observes both rate-removal and disassociation signals, followed by direct reassociation without authentication.
It also checks acknowledged refusal, HT cleanup, peer isolation, and the existing lifecycle controls.
This corrects the B6 diagnosis without an additional cleanup mechanism in the MIB.

### B7 implementation contract — validated before source edits

HCF owns the context before the frame-sequence handler starts.
A synchronous start listener can stop HCF and invalidate that grant.
HCF must mark that local context `STOPPED` and emit finish before it deletes the context.
The ordinary finish callback cannot run here because stop already released the channel.
The patch changes `Hcf::startFrameSequence()` and the two existing cancellation scenarios in `TxopExchange`.
Signal observers retain access to a valid context during finish; no steps execute for the canceled grant.
Restart creates a separate context and preserves queued data.
No NED, wire, initialization, or timer contract changes apply.
The corrected scenario 24 reproduces the unbalanced count before the fix.
Both scenarios must finish with two starts, two finishes, and one transmitted payload.

**Invariant:** a completed transition out of association must remove that association's installed peer policy before notification.

**Primary target:** `src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAp.cc`.
The MIB owns installed rate entries; AP management owns the transition that invalidates them.
Inspect `handleDisassociationFrame()` and the acknowledged-refusal branch in `frameTransmissionFinished()`.
The latter also emits rate removal before it clears the pending transaction.
Existing authentication, deauthentication, and lifecycle cleanup paths provide controls.

Implementation tasks:

1. Commit authenticated station status before disassociation releases the association ID.
2. Clear pending transaction state before acknowledged refusal releases the association ID.
3. Complete station, association-ID, transaction, and capability updates before the relevant state notification.
4. Preserve other peers and the AP's own BSS rate state.
5. Extend the lifecycle fixture through the actual management handlers and acknowledged response outcome.

Acceptance checks:

- Successful association installs the expected peer rate set.
- Disassociation removes that set immediately, while the peer remains authenticated.
- Observers see the complete state at the rate-change and disassociation notifications.
- A new association request from that authenticated peer does not use obsolete peer restrictions for its response.
- A successful replacement response installs the new accepted rates.
- An acknowledged refusal clears peer rate and HT state from the previous association.
- Stale response completions cannot restore the removed state.
- Authentication, deauthentication, and node restart retain their existing cleanup behavior.

Do not rely only on renewed authentication to prove this fix.
Authentication sequence number 1 already removes peer rates and can conceal the disassociation defect.
The new regression must inspect the interval before that existing cleanup.

## Step 7 — B7: balance sequence signals for a canceled grant

**Invariant:** every emitted sequence start has exactly one finish, including cancellation before the handler starts the sequence.

**Primary target:** `src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc`, at `startFrameSequence()`.
Inspect `Hcf.ned` and the frame-sequence result filters as consumers.
Extend `tests/module/Ieee80211TxopExchange_1.test`, especially scenarios 24 and 25.

Implementation tasks:

1. Detect the existing pre-start cancellation after start-signal delivery returns.
2. Mark the canceled context with the `STOPPED` outcome.
3. Emit one matching finish signal before destruction of that context.
4. Preserve the canceled-grant guard across synchronous stop and restart.
5. Update tests that currently expect two starts but only one finish.

Do not invoke the normal channel-release callback for this early cancellation.
`stop()` already releases the grant, and the sequence handler never acquired this context.
The finish signal must describe the canceled context while that context remains valid.
The finish signal must not release or change a replacement grant.

Acceptance checks:

- Stop-only cancellation produces one start and one finish with zero transmitted packets.
- Immediate restart produces balanced counts for the canceled sequence and the later successful sequence.
- The active-sequence statistic equals zero whenever the station becomes idle.
- The canceled context reports `STOPPED`, zero executed steps, and a valid duration.
- Stop/restart from other callbacks does not emit duplicate finishes.
- Existing transmission, acknowledgment, queue-drain, and grant-count assertions retain their intended results.

## Step 8 — Review controls and policy documentation

These tasks address useful review gaps without adding unsupported production fixes.
The assessment records why the other proposed code changes are unnecessary.

1. Add normal ACK and Block Ack cases that change the rate revision during an active response wait.
2. Verify that valid responses resolve those waits without an extra retransmission.
3. Verify ordinary timeout recovery when the corresponding response is absent.
4. Keep the two-frame fresh-TXOP test and the single-protection RTS/data/CTS Duration assertions.
5. Complete the multiple-group BAR cases under Step 2.
6. Clarify fixed-rate precedence during permitted overruns in `QosRateSelection.ned` and the migration guide.
7. Clarify the experimental response contract in `RateSelection.ned` and the migration guide.
8. Extend `Ieee80211RateSelectionNonstandard_1.test` with known peer rates that conflict with an explicit experimental override.

The experimental option retains its explicit opt-in behavior for DCF.
Document that it bypasses negotiated peer-rate validation and requires compatible experimental settings at both peers.
Keep strict-mode rejection, automatic selection, and missing-metadata cases as controls.
Do not add this option to HCF.

The standard recommends a high rate during a permitted TXOP overrun; it does not require an override of configured rates.
Preserve the fixed-rate unit assertion and the separate primary-response rules.
The current migration guide is `doc/src/migration-guide/index.rst`.

## Step 9 — Combined verification and completion

Rebuild the matching INET library after each production source change.
Run the smallest direct tests for each fix before the combined selection.
The following commands run from the repository root after the planned fixtures exist:

```sh
make MODE=debug -j8
inet_run_unit_tests -m debug -f 'Ieee80211(BlockAckCompletion|BlockAckRecord|BlockAckActionWire|PreparedStep|TxopDuration|TxopProcedure|RateSelection|RateSelectionNonstandard|MgmtApTransaction)_1.test'
inet_run_module_tests -m debug -f 'Ieee80211(BlockAckInactivity|BlockAckLoss|TxopExchange|MgmtApLifecycle)_1.test'
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '/11n/N_Txop(BlockAck|Boundary|Burst|FragmentRetry|Fragments)\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
git diff --check
```

The protocol selection covers existing Block Ack, TXOP boundary, burst, fragmentation, and retry exchanges.
The new regressions provide the direct proof for B1 through B7.
Existing tests provide surrounding regression coverage.
Extend the selection if the final fix changes another tested contract.
Record zero selected cases as NOT_RUN.

Map the final changed paths to affected fingerprint cases through the active dependency data.
Run those cases with an explicit filter.
Explain each changed result through a corrected behavior and direct evidence.
The [baseline procedure](../../doc/project/guide/change-a-baseline.md) governs any recorded expectation change.
Do not regenerate expectations merely to make a test pass.

Before a push, run debug and release builds plus the project-wide gates in [run-the-gates.md](../../doc/project/guide/run-the-gates.md).
Retain per-commit evidence for the proposed fix series.
Update `WHATSNEW` for the user-visible corrections.
Update the migration guide if a public handler contract changes.
Review the stable diff for agreement identity, packet ownership, callback re-entry, timer order, and sequence wrap.

Completion requires all of these results:

- Each defect has a regression that fails for the original cause before its fix.
- Each regression passes through the real production path after its fix.
- The control cases retain their intended results.
- B1 and B3 complete recovery and drain the relevant queues.
- B4 proves retransmission and exactly-once delivery after the injected loss.
- B5 and B2 prove accepted intervals and correct deadline updates separately.
- B6 proves immediate peer-state cleanup before notification and before any renewed authentication.
- B7 proves balanced sequence signals and zero active sequences at idle.
- The review controls preserve the selected protection and rate policies.
- Focused tests and required gates pass, with any remaining coverage gap stated explicitly.
- The consolidated report links each defect to its fix commit and evidence.
- The completed plan moves from `plan/pending/` to `plan/done/` with its status updated.
