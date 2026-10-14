# Implementation plan: repair Block Ack agreement lifecycle

Original source observation revision: `feat/ieee80211-txop-single-protection` at `b06e8768b0e9c78b7a3665b9c69b0e0da9f30e41`.
Current integration source: `master` at `1cdb362a2a4b97f564566487079d9d156a5d213d` in `/home/user/omnetpp_ws/inet`.
Plan revision: 2026-10-14, revision 15.
Source basis: IEEE Std 802.11-2024 and the revisions above. Source observations do not establish runtime results.

This plan follows [Compressed plan revision 25](ieee80211-compressed-blockack-replacement.md). Approval: the maintainer's 14 October 2026 instruction, "proceed", after the revision-24 review authorizes these document corrections. This approval authorizes no later implementation, recorded expectation change, or publication.

Lifecycle observations retain their original `b06e8768` pin and require a separate review against each selected stage parent. The integration source contains S1 from merged [PR #1320](https://github.com/inet-framework/inet/pull/1320). Original S1 runtime evidence retains its feature-head boundary; this update supplies no fresh runtime result for main. The A4 timeout, A5 transfer-completion predicate, and C2 post-conflict setup disposition remain unresolved. The [umbrella decision table](ieee80211-compressed-blockack-replacement.md#maintainer-decisions) defines their dependent boundaries.

## 1. Core intended behavior

HCF, the hybrid coordination function, will keep each Block Ack agreement consistent with its peer and traffic identifier (TID). A TID identifies a traffic class or stream. ADDBA establishes an agreement. DELBA ends an agreement. A Block acknowledgment request (BAR) requests reception status.

The agreement handlers will accept only matching successful setup responses. HCF will preserve the negotiated first sequence number through deferred DATA extraction. Teardown will remove only the selected direction's state and prevent obsolete Actions from changing a later agreement.

Hypothetical example: ADDBA specifies sequence number 100, but its response is lost after recipient acceptance. HCF keeps that peer and TID's DATA deferred. It sends DELBA after setup failure before Normal Ack DATA can resume. Another peer remains eligible throughout this recovery.

This prerequisite retains Basic throughout the final [Compressed addition](ieee80211-compressed-blockack-replacement.md). Basic originated in IEEE Std 802.11e-2005 and is unsupported by IEEE Std 802.11-2024. The umbrella's [standards boundary](ieee80211-compressed-blockack-replacement.md#2-standard-support-and-model-boundary) supplies the historical clauses and online references. This prerequisite changes neither bitmap encoding nor profile-specific response rates. The [EDCA lifetime prerequisite](ieee80211-edca-data-lifetime.md) supplies the finite DATA deadline for deferred setup and recovery.

## 2. Current source and required evidence

[Hcf](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) dispatches duplicate ADDBA Actions and sends unexpected Actions into an active response step. The frame sequence can delete that Action without agreement dispatch. Queue disposal currently reaches management ownership through the tagged transaction path, which does not cover every Block Ack Action.

The agreement policies do not validate the complete accepted response contract. For received DELBA, HCF uses the local receiver address for both roles' agreement lookups before deletion-signal emission. The [recipient handler](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.cc) also uses that wrong address for agreement removal. The [originator handler](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.cc) correctly uses the peer transmitter address for removal. Reorder cleanup lacks the required direction check.

Both handlers supply absolute inactivity deadlines, but HCF schedules them as relative delays. Both handlers also select expired agreements only when the deadline equals the current time. Hypothetical example: at 5 seconds, a deadline of 7 seconds schedules an event at 12 seconds through `rescheduleAfter`. The equality check then misses that overdue agreement. The repair must schedule absolute deadlines and retire agreements whose deadlines are at or before the current time.

AP reassociation can retain identical peer capability state. A signal that reports only changed capability values therefore cannot identify every association reset. Tx also provides no cancellation operation for a frame that it already accepted.

These are source observations. Each implementation stage must supply a direct production reproduction for the old path it changes. The required original and corrected outcomes appear in the stage table and verification cases below. Runtime reachability and performance remain unverified until those cases run.

## 3. Agreement behavior

The originator and recipient policies will use HCF's immutable typed profile and their existing MIB paths. HCF resolves the parameter once; policies do not reread it or infer a profile from PHY mode. Compressed setup requires local HT operation and valid peer HT state. Explicit Basic setup retains existing capability assumptions; absent HT state does not prohibit it. Basic is a historical INET profile, not current-standard operation.

The originator agreement owner will retain the request dialog token until a response arrives. It will accept a response only for its pending peer, TID, and token, with success status, immediate policy, and positive buffer size. It will choose a new dialog token for each request to that peer and TID.

The recipient will apply the HT requirement only to the Compressed profile. Basic acceptance still validates peer, TID, token, immediate policy, and supported parameters, without a new HT capability requirement. Its built-in policy will reject a modification request while the peer and TID have an active agreement. The existing agreement will remain unchanged after that rejection, as Clause 10.25.2 requires. A rejected delayed, unsupported, or modification request will receive an ADDBA Response with `SC_UNSPECIFIED` status. The response will copy the request dialog token and create no agreement.

HCF will dispatch an ADDBA or DELBA Action only after the recipient data service accepts it as a nonduplicate complete management frame. The service will return that accepted Action to HCF instead of deleting it before dispatch. HCF will retain the existing immediate ACK behavior for a valid duplicate, but it will suppress duplicate agreement actions. The existing management sequence cache will identify MAC retries; no second duplicate cache is necessary. The dialog token identifies the setup transaction and does not replace MAC duplicate detection.

During an active response wait, HCF will recognize a valid addressed Block Ack Action before it transfers frame ownership to the response step. HCF will cancel the response timer and use the existing receive-timeout operation to end the unsuccessful exchange without that Action. After sequence cleanup, HCF will pass the retained Action through normal recipient processing, including its immediate ACK and duplicate check. The existing failure path supplies recovery feedback for the interrupted exchange. HCF will not suspend and later resume that exchange. A frame accepted by Tx still follows the completion rule below; this receive rule applies only to an active receive step.

Hypothetical example: DELBA arrives while HCF awaits Block Ack. HCF ends the unsuccessful BAR exchange, then acknowledges and dispatches DELBA through the recipient path. The agreement owner retires the selected role before deferred contention can select another DATA attempt.

Hypothetical example: the recipient accepts ADDBA, but its request ACK and successful response are lost. A request retransmission receives its ACK without a new modification decision. The existing response transmission follows its retry procedure. If that response never arrives, the originator's response timeout starts teardown. A genuine new modification request remains subject to rejection without a change to the active agreement.

For acceptance, the built-in recipient policy will select `min(64, maximumAllowedBufferSize)` as its response buffer size. The request size gives guidance but does not cap that choice under Clause 10.25.2. This INET policy uses the configured allocation and ignores the advisory request size. A request size of zero gives no size guidance under Clause 9.4.1.13. The built-in policy will reject a nonpositive configured maximum at initialization. The recipient will build the successful response before it creates the agreement.

Its agreement and reorder buffer will use the accepted response size and the request starting sequence number. Hypothetical example: request size 8 and configured maximum 64 produce response size 64. Tests with a synthetic recipient will separately check other legal response choices and their effect on the originator window.

The [completed S1 prerequisite](ieee80211-compressed-blockack-replacement.md#completed-s1-and-recorded-verification) supplies reorder record creation and window updates through Basic reception. `BlockAckReordering` creates its record on the first valid related DATA or Basic BAR from the supplied agreement values. A Basic BAR can advance that record before DATA reception. S1's fixture supplies agreements and proves no ADDBA negotiation. A2 must verify that negotiated values reach this current owner. C2 must verify Compressed integration; neither stage needs a second reorder record.

The recipient will use its configured `blockAckTimeoutValue` as the response inactivity value. The request value is advisory. Both agreement owners will use the response value, including zero, which disables inactivity expiration. The recipient will start its deadline when it creates the agreement. The originator will start its deadline when it accepts the response. This rule replaces the current request-based recipient deadline.

HCF will emit `blockAckAgreementAdded` when the recipient accepts and creates that active agreement. The originator will emit it only after it accepts the response. Pending and rejected records will produce no added event. If response delivery fails, the recipient agreement remains active until DELBA, inactivity expiration, or peer cleanup.

The ACK policy will select Block Ack only for an active agreement and a frame supported by its selected profile. Compressed selection will recheck peer HT state and use Normal Ack when usable state is absent. Basic selection will preserve its historical capability and fragment behavior. The [umbrella model boundary](ieee80211-compressed-blockack-replacement.md#2-standard-support-and-model-boundary) owns the Compressed fragmentation restriction. Normal Ack cannot bypass that restriction. Local HT operation alone does not convert a Basic compatibility agreement into a Compressed agreement.

The ADDBA request A-MSDU flag will reflect the originator's active A-MSDU aggregation policy. An accepted agreement will retain the response A-MSDU flag. Before aggregation, `OriginatorQosMacDataService` will check both negotiated A-MSDU flags for that peer and TID. If either flag denies A-MSDU, the service will extract individual DATA frames for that peer and TID. Plain DATA can then use Block Ack under the accepted agreement.

The ACK policy will also reject any already-built A-MSDU that lacks permission; it will not use Normal Ack to bypass that restriction. Pending ADDBA and DELBA deferral will prevent new A-MSDU construction while the recipient's agreement state remains uncertain. A rejected initial request with no possible earlier agreement will release DATA under Normal Ack. A rejection of modification does not establish that the peer has no agreement. Association teardown will clear the request record.

Keep the existing ADDBA trigger after the first eligible DATA transmission, provided more unnumbered DATA exists for that peer and TID. The agreement owner will record setup intent and defer further extraction for that peer and TID. Already numbered frames for that peer and TID will finish or drop under their existing ACK policy. Before request creation, HCF will cancel setup intent if no unexpired unnumbered DATA remains. Otherwise, HCF will query the QoS data service for the next unassigned number for that peer and TID. The data service alone will read `QoSSequenceNumberAssignment`; this query will not consume the number.

HCF will pass the number to the agreement owner as the request starting sequence number. The fixed HCF NED composition permits this narrow QoS data-service operation without a DCF interface change. HCF will send the request after those numbered frames resolve.

If all deferred DATA disappears during pending setup, HCF will cancel an ADDBA Request that Tx never accepted. If Tx accepted any request attempt, HCF will instead enter serialized teardown because peer agreement state remains uncertain. If a successful response leaves no DATA, HCF will immediately enter that teardown path. No final Block Ack exchange is necessary for this unused-agreement policy. Hypothetical example: one Normal Ack DATA frame with no successor causes no ADDBA. If deferred DATA expires during an existing setup, HCF cancels untouched setup or sends DELBA after possible peer acceptance.

During setup intent, already numbered DATA remains eligible until it resolves; only further extraction is deferred. While the request or a needed DELBA remains pending, `InProgressFrames` will skip that peer and TID's DATA. HCF will supply an eligibility check from the agreement owner's current state; queue components will retain no duplicate agreement state.

The QoS data service will scan its existing queue with `getPacket` and select the first eligible frame before it considers A-MSDU aggregation. If that frame is queue head, the service will check A-MSDU permission before it calls the prefix aggregation policy. With no agreement, the existing Normal Ack aggregation policy will apply. With an accepted agreement, both negotiated flags must permit A-MSDU. If that frame follows deferred DATA, the service will extract it individually with `removePacket`. It will preserve order within each peer and TID; other peers, TIDs, and management frames remain eligible.

The QoS data service will expose the first eligible queue frame's receiver without extraction. For DATA, it will also report its TID and the next sequence number without consuming that number. `InProgressFrames` will expose eligible frames that it already owns without extracting another frame. HCF will check those frames first, then ask the QoS data service about its queue. Both queries will be read-only. BAR selection will use their result before any extraction or sequence assignment.

The existing extraction path will run only after HCF selects DATA or management work instead of BAR. The queue preview and extraction will use the same eligibility check. HCF's fixed QoS data-service path needs no DCF interface change.

HCF will not request contention for an access category with only deferred DATA and no eligible BAR. It will request access again when an accepted response, rejected response, or confirmed DELBA releases that DATA. The accepted response will release the deferred DATA, so its first extracted frame receives the request number. This eligibility path is necessary because the current extractor assigns sequence numbers before transmission.

The originator policy will implement its existing `computeAddbaFailureTimeout` contract with a positive, configurable INET timeout. The proposed default is one second, subject to the [timeout and lifetime review](#response-timeout-and-data-lifetime); it is not an IEEE time limit. The agreement owner will hold one response deadline for each pending peer and TID. HCF will use one separate timer for the earliest of those deadlines. It will start the deadline after the ADDBA Request receives its Normal Ack. A response accepted before that ACK will prevent a later ACK callback from creating a response deadline.

The owner will clear the deadline on a matched response, teardown, or peer cleanup. Invalid responses will not clear it. A request retry callback cannot replace an accepted agreement with setup failure. HCF will identify each pending Action by its retained frame identity and agreement role. A terminal callback will act only on the record that still owns that Action.

HCF will also route queue rejection and eviction to that Action owner before the queue deletes the frame. It will use its existing queue-drop subscription and the retained identity to recognize owned Block Ack Actions. This route does not require an association-transaction tag. The owner will process one terminal outcome per Action; duplicate notifications have no further effect.

For intentional invalidation, the owner will detach the obsolete Action identity before HCF removes the queued frame. The resulting queue notification cannot trigger a second recovery transition. An accepted transmission retains its separate completion context, even after that detachment. Hypothetical example: an accepted response retires a queued request retry. Disposal of that retry cannot turn the newly active agreement into teardown.

For an ADDBA Request, disposal before Tx accepts any attempt permits local cancellation and Normal Ack fallback. Disposal after Tx accepts an attempt requires teardown unless a matched response already resolved setup. For a successful ADDBA Response, disposal leaves the recipient agreement active until ordinary teardown, inactivity expiration, or peer cleanup. Disposal of a rejection response creates no agreement. DELBA disposal follows the failed-DELBA rule, even if the frame never transmits. Thus, queue loss cannot leave teardown-pending state without an owned exchange.

A response can arrive after request ACK timeout while that request awaits another attempt. Acceptance will remove any queued request retry before DATA resumes. A request already accepted by Tx follows the retained-context rule below. This is the relevant response-before-request-completion case; no concurrent callback inside an ordinary synchronous call is assumed.

An explicit rejection permits immediate Normal Ack fallback only for initial setup without possible earlier recipient state. A request discarded before Tx accepts any attempt permits the same fallback. If a transmitted request reaches its retry limit without an accepted response, HCF will queue DELBA and keep DATA deferred. It will do the same when an acknowledged request gets no response before its deadline. The agreement owner will retain one teardown-pending state for that peer and TID. HCF will ignore a late ADDBA Response after this transition.

HCF will treat a DELBA Normal Ack as teardown completion for this INET profile. The ACK confirms frame reception; this policy assumes that the peer processes a valid DELBA. HCF will then release unsent DATA under the ordinary no-agreement ACK and aggregation policies. If originator-role DELBA reaches its retry limit or suffers queue disposal, HCF will drop that peer and TID's currently deferred DATA. The originator owner will retain a recovery-needed record with no scheduled retry. HCF will emit `packetDroppedSignal` for each dropped frame.

HCF will emit `linkBrokenSignal` for retry-limit failure through the existing recovery path. Queue disposal alone does not establish link failure and will not emit that signal.

A failed recipient-role DELBA clears its teardown-pending record without an originator recovery-needed record. A later valid BAR or Block Ack-policy DATA can trigger another recipient-role DELBA. This outcome does not discard DATA from the opposite-direction agreement. Hypothetical example: a full management queue drops originator DELBA before transmission. HCF drops the deferred batch and retains recovery-needed state, so later DATA can trigger a fresh exchange.

New DATA in originator recovery-needed state will trigger one new DELBA exchange through the existing management retry procedure. HCF will defer that DATA until success, terminal Action failure, or DATA lifetime expiration. Frames that arrive during that exchange will share its result; they will not start parallel DELBA exchanges. Another failure drops that deferred batch and returns to recovery-needed state. Thus, transient ACK loss does not permanently prohibit future DATA. This INET policy cannot guarantee delivery while communication remains unsuccessful.

Hypothetical example: the peer processes DELBA, but every ACK is lost. HCF drops the deferred batch and retains only recovery-needed state. A later DATA arrival triggers a fresh DELBA exchange. Its ACK releases the new DATA under Normal Ack. No periodic retry timer or permanent peer blacklist is necessary.

HCF will subscribe to `rateStateChangedSignal` and check current HT state for Compressed agreements. Loss of required HT state will retire the affected Compressed state. Absent HT state or a rate-only notification will not retire Basic compatibility state. At retirement, HCF will emit one `blockAckAgreementDeleted` event for each active agreement before removal. Pending, teardown, and recovery records will produce no deleted event. The same rule applies to DELBA and explicit association reset.

At successful reassociation, management must retire old agreement state for either profile before new DATA selection. AP management will still remove old HT state before replacement at the acknowledged response commit. The STA path already removes old HT state before replacement. Those MIB notifications cover Compressed cleanup, including identical capabilities. They cannot alone prove Basic cleanup when no HT record exists.

A3b will trace the existing AP association notifications, which carry peer addresses, and the STA association and association-loss paths. Management must pass the old peer key to HCF's retirement operation before replacement or loss of that identity. Reuse existing notifications where their order and payload meet this contract. A path without a suitable notification needs an explicit synchronous retirement handoff from management. This handoff must cover Basic peers without HT records.

The handoff must preserve the public boolean MIB signal and create no synthetic HT state. Failed reassociation follows its existing disposition; capability refresh alone does not reset an agreement.

The [consumer assessment](#reassociation-consumers-and-callback-order) defines the signal observations and direct MIB readers that A3b must preserve. Removal, HCF cleanup, and replacement installation occur within one synchronous operation. No separate simulation event intervenes, but signal callbacks observe the temporary absence of peer HT state. HCF must complete cleanup without new DATA selection or an ACK-policy decision that uses that temporary state. The implementation must trace the callback path before it adds a guard or another transaction mechanism.

When either role decides to send DELBA, HCF will retire its active agreement and emit its deletion event once. The role owner will retain one teardown-pending record before HCF queues DELBA. For the originator role, HCF will defer unsent DATA until DELBA receives an ACK. A transmission-complete callback will not erase that record. ACK reception, retry-limit failure, or queue disposal completes the exchange. An unknown-agreement DELBA will use the recipient role and create no active agreement.

For received DELBA, the peer key is the transmitter address. `Initiator=1` ends the local recipient agreement and its reorder state. `Initiator=0` ends the local originator agreement and its transmit state. HCF will apply the role decision before either data service clears state. A transmitted DELBA uses the receiver address as its peer key. The opposite-direction agreement for the same peer and TID will remain unchanged.

A received DELBA also resolves uncertain setup or teardown for its selected role because it confirms peer teardown. HCF will remove that role's queued Actions, clear its response deadline, and release surviving unsent originator DATA under Normal Ack. It will send no DELBA in response. An Action already accepted by Tx still requires safe completion without retries; replacement setup waits for that completion.

This profile selects the discard-all option in Clause 11.5.3.4 at originator teardown. HCF will discard every transmitted, unacknowledged MPDU in that agreement, including the highest transmitted MPDU. The data service will preserve sequence continuity for unsent DATA beyond the highest transmitted number. It will not reuse a discarded number or move the counter backwards. This avoids a special retry across agreement boundaries. The same rule applies to local DELBA, received DELBA, and inactivity expiration.

Inactivity expiration can therefore discard transmitted, unacknowledged DATA whose lifetime has not expired. User documentation must state that consequence alongside the discard-all choice. Hypothetical example: DATA 100 remains unacknowledged when inactivity expires, but its lifetime deadline is later. Teardown discards DATA 100 and records inactivity as the teardown cause. A5's completed-transfer predicate leaves no unresolved DATA for discard-all; its cost concerns management exchanges and later setup. Unsent DATA does not enter discard-all solely because inactivity expires.

At recipient teardown, the recipient data service will update its duplicate cache under Clause 11.5.3.5 before it releases agreement resources. It will use the highest received sequence number in modulo-4096 order, regardless of ACK policy. The agreement owner will retain that value because scoreboard movement and delivery can remove its earlier receipt evidence. Hypothetical example: DATA 100, 101, and 102 remain unacknowledged when teardown starts. The originator drops all three; new DATA starts after 102. The recipient retains duplicate state for its highest received sequence number.

The Block Ack owners will remove that peer's agreements and receive records on peer cleanup. The recipient data service will clear peer reorder and duplicate state for association reset. HCF will discard old transmitted DATA and clear its matching ACK and recovery records. This association reset differs from ordinary DELBA, which preserves the required recipient duplicate state. These operations will not change another peer's state. The MIB remains the capability owner; the agreement handlers remain the agreement owners.

Before a role accepts replacement setup, its previous outgoing DELBA must reach a terminal result. The recipient will reject new ADDBA while its outgoing teardown remains pending. The originator will not start new ADDBA during its own teardown or recovery-needed state. One pending DELBA per role, peer, and TID is sufficient. Repeated unknown-agreement frames will reuse that pending teardown work. This prevents an obsolete queued DELBA from reaching a newly established agreement.

Peer cleanup will also remove that peer's queued ADDBA Requests, Responses, and DELBA frames through the existing queue owners. HCF will invalidate matching in-progress Actions before another transmission attempt. HCF will check validity at frame selection and before it submits a frame to Tx. A frame that Tx already accepted must finish; `ITx` provides no cancellation operation. Its context must remain alive through completion, and HCF must suppress any subsequent retry or setup transition from that old work.

Every agreement retirement will invalidate its owned ADDBA work, including ordinary DELBA, inactivity expiration, unused setup, and the first-DATA failure rule below. A response that announces the retired agreement must leave the queue before another transmission attempt. A response already accepted by Tx may finish, but its completion cannot restore the retired agreement or start another retry. Serialized teardown follows that accepted exchange before replacement setup becomes eligible. Hypothetical example: recipient inactivity expires while a successful response remains queued. HCF removes that response and sends DELBA; the originator cannot subsequently accept obsolete queued success.

HCF will defer replacement setup until such an accepted transmission and its existing response wait reach a terminal result. It will retain the Action's ownership identity only for that interval. It will not use a generation counter as a substitute for queue removal or transmission eligibility. Hypothetical example: inactivity expiration queues recipient DELBA just before a new ADDBA arrives. The recipient rejects setup until the old DELBA completes. A later request can establish an agreement that no old DELBA can subsequently tear down.

HCF will own one inactivity timer for both agreement roles. The recipient owner will refresh its deadline after valid matching Block Ack-policy DATA or BAR. Duplicate DATA with that policy also refreshes inactivity, before duplicate discard. Normal Ack DATA and another peer or TID will not refresh it. The originator owner will refresh its deadline after a matched Block Ack response. A zero accepted timeout will remain disabled after every refresh.

After each agreement state change, HCF will find the earliest active absolute expiration across both owners. It will schedule that absolute time and cancel the timer when no finite deadline remains. Hypothetical example: matching BAR arrives just before expiration. The recipient sends Block Ack and moves its deadline forward by the accepted timeout. A BAR for another TID leaves the original deadline unchanged.

At expiration, each owner will retire every agreement with a deadline at or before the current time. HCF will apply the directional disposal rule and emit one deletion event per retired active agreement. It will queue DELBA with the TIMEOUT reason code through the same serialized teardown path. It will then schedule the next future deadline. This reuses the existing HCF timer and repairs its current absolute-time error.

HCF will create its response timer during initialization and delete it at module destruction. Peer cleanup will cancel the matching response deadline. The proposed one-second default bounds the response wait after an acknowledged request, not contention or teardown duration. DATA lifetime separately bounds retained DATA. The current MAC stop and crash handlers do not reset HCF timers, queues, or frame sequences. This plan makes no stop/start recovery claim; a later lifecycle change must handle those owners together.

HCF will check for an active recipient agreement before it passes a valid BAR to the recipient data service. Without that agreement, HCF will queue recipient-role DELBA through ordinary management contention. It will create no scoreboard or reorder buffer and send no immediate ACK or Block Ack. The replacement plan owns Compressed format validation before this agreement check.

HCF will similarly discard valid Block Ack-policy DATA without an active recipient agreement before the data service can deliver it. It will queue DELBA for that peer and TID. Malformed frames and unsupported variants will follow the invalid-frame path without protocol recovery actions. Hypothetical example: the recipient loses its agreement while the originator retains DATA 100. BAR causes DELBA, which ends the originator's obsolete agreement. DATA 100 then follows the teardown disposal rule.

### First DATA and final transfer

The agreement owner will retain whether its first DATA transmission occurred. Extraction alone does not satisfy that condition. If the DATA with the request number suffers terminal disposal before transmission, HCF will retire the agreement and enter serialized teardown. This rule includes lifetime expiration and terminal RTS failure that discards the protected DATA. Surviving unsent DATA will wait; sequence assignment will not reuse the discarded number. The first accepted DATA attempt can finish across its deadline under the safe-completion rule; expiration cannot cancel that accepted attempt.

Hypothetical example: request number 100 becomes DATA 100, which expires after unsuccessful RTS attempts. DATA 101 remains available, but HCF sends DELBA before it can transmit 101. DATA 101 can later use Normal Ack; a later agreement obtains a fresh request number. Thus, 101 cannot become the first DATA transmission under the agreement that specified 100.

Under the proposed predicate, HCF will consider a transfer complete at an exchange boundary with no queued DATA, in-progress DATA, or pending BAR exchange. All three conditions apply to that agreement. Clause 10.25.4 requires DELBA when the originator has no DATA and its final Block Ack exchange completes. The clause does not specify this queue predicate. After that condition holds under the accepted interpretation, HCF will retire the agreement and queue DELBA. Zero inactivity timeout does not disable that separate requirement.

If lifetime expiration removes all unresolved DATA, HCF will end the agreement through the same teardown path as an INET policy. Later DATA does not cancel queued DELBA; it waits for teardown before a new transfer. Hypothetical example: the fourth DATA receives a positive final bitmap and no fifth DATA exists at that exchange boundary. HCF sends DELBA even if another burst arrives shortly afterward. This interpretation can cause repeated setup and teardown across idle gaps. Section 7 measures that cost without an assumed acceptable threshold.

#### Transfer-completion decision

Before A5 implementation in S4, the maintainer must accept the exact predicate above or define another boundary from supported state. Any alternative must preserve Clause 10.25.4's conditional DELBA requirement. Record the boundary, the state that represents it, and the compatibility consequence. Measurements expose repeated setup cost; they do not select the predicate. Document approval leaves this decision open.

### Agreement transitions

This table summarizes the rules above and the first-DATA rule above. Each agreement owner retains its role's state for one peer and TID. The table adds no scheduler or second agreement store. Terminal Action outcomes include queue disposal. Any frame already accepted by Tx retains its exchange context until safe completion.

| Role and current state | Event | Next state | Owned Action | DATA eligibility and disposal |
| --- | --- | --- | --- | --- |
| Originator has no agreement. | Eligible DATA transmits with more queued DATA. | Setup intent. | None exists yet. | Numbered DATA resolves; extraction for this peer and TID stops. |
| Originator has setup intent. | Numbered DATA resolves. | Request pending, or no agreement if no DATA remains. | HCF queues one request only if DATA remains. | Unnumbered DATA stays deferred during the request. |
| Originator has a pending request. | Initial rejection arrives, or untouched request suffers disposal or cancellation. | No agreement. | HCF retires the request. | Surviving DATA uses Normal Ack. |
| Originator has a pending request. | Success arrives with DATA available. | Active agreement before its first DATA transmission. | HCF retires the request retry. | DATA resumes with the request sequence number. |
| Originator has uncertain setup. | Response timeout, terminal request failure, or loss of all deferred DATA occurs. | Teardown pending. | HCF retires request work and queues DELBA. | DATA remains deferred; expired DATA leaves the queue. |
| Originator accepts unused setup. | Success arrives with no DATA. | Teardown pending. | HCF queues DELBA. | No DATA exists; later DATA waits. |
| Originator has an active agreement. | First DATA suffers terminal disposal before transmission, transfer completes, or local teardown occurs. | Teardown pending. | HCF queues DELBA. | Transmitted unacknowledged DATA follows discard-all; surviving unsent DATA waits. |
| Originator has teardown pending. | DELBA receives an ACK. | No agreement. | HCF retires DELBA. | Surviving unsent DATA uses Normal Ack. |
| Originator has teardown pending. | DELBA reaches its retry limit or suffers queue disposal. | Recovery needed. | No Action remains. | HCF drops the deferred batch. |
| Originator needs recovery. | New DATA arrives. | Teardown pending. | HCF queues one new DELBA. | The new batch waits for the exchange outcome. |
| Recipient has no agreement. | A supported request arrives. | Active agreement. | HCF queues its successful response. | Related received DATA uses the new scoreboard and reorder state. |
| Recipient has an active agreement. | Its response receives an ACK or suffers terminal failure. | Active agreement. | HCF retires the response. | Agreement reception continues until a separate teardown event. |
| Recipient has an active agreement. | Local teardown or inactivity expiration occurs. | Teardown pending. | HCF invalidates success responses and queues DELBA. | HCF releases agreement resources; later Block Ack-policy DATA cannot reach delivery. |
| Recipient has no agreement. | Valid BAR or Block Ack-policy DATA arrives. | Teardown pending. | HCF queues one DELBA. | DATA is discarded; BAR creates no receive state. |
| Recipient has teardown pending. | DELBA receives an ACK or suffers terminal failure. | No agreement. | HCF retires DELBA. | New setup can proceed; the opposite role remains unchanged. |
| Either role has a record. | A received DELBA selects that role. | No agreement. | HCF retires that role's obsolete Actions. | Directional disposal applies; surviving unsent originator DATA can resume. |
| Either role has a record. | Peer association reset occurs. | No agreement or recovery record. | HCF retires that peer's Actions. | HCF drops old transmitted DATA and clears peer receive state. |

### Profile conflict recovery

A profile conflict is a well-formed Basic or Compressed BAR that differs from the local active agreement's resolved profile. The BAR must address this recipient and match that agreement's peer and TID. Unknown variants and malformed frames do not meet this condition. Agreement setup itself has no variant negotiation field.

At C2, HCF will classify that conflict before scoreboard, reorder, response, or inactivity-success updates. The recipient agreement owner will retire only the matching recipient role. It will queue one recipient-role DELBA through the existing Action ownership and retry procedure. Retain the opposite-direction agreement and other peers. Repeated conflicting BAR during pending teardown must reuse that owned work.

This DELBA rule is an INET mismatch policy. Clause 11.5.4 requires DELBA for an absent agreement; it does not directly require this active-profile-conflict rule. After retirement, valid frames without an agreement use A3c's existing recovery path.

Received DELBA ends the originator's selected agreement through ordinary directional teardown. Its transmitted, unacknowledged DATA follows the selected discard-all policy. DATA that the recipient already delivered remains a delivery result; originator disposal cannot establish application-level loss. Measure those results separately.

Hypothetical example: HT peers select Basic and Compressed but accept ADDBA. The recipient delivers unfragmented DATA 100, then receives an incompatible Basic BAR. It queues recipient-role DELBA before lifetime expiry. The originator retires its agreement and disposes of unresolved transmitted state. This sequence exposes the configuration error earlier, but does not make the profiles compatible.

A new ADDBA could repeat the same conflict. Before C2 implements this path, its owner must select the post-conflict setup disposition. Trace its production caller. The disposition must bound repeated setup under persistent conflict and define what permits a new attempt. It must preserve valid Normal Ack traffic and state for other peers.

Tests must cover unchanged conflicting settings and a separately initialized compatible configuration. If the disposition permits a later attempt within one run, test its actual supported trigger. Do not change HCF's resolved profile in place or claim new MAC stop/start support.

The plan does not yet select that disposition. Ordinary DELBA carries no new profile-negotiation field. Do not claim that the originator can infer the recipient's configuration from it. Do not invent a permanent peer blacklist or wire reason code to close the gap. The unresolved disposition blocks C2 mismatch implementation and publication, but not S1, R1, or earlier lifecycle repairs.

C2 owns variant classification because it introduces both production wire paths. A3c supplies the existing Action disposal and absent-agreement operations. The lifecycle owner retains teardown, disposal, and peer-reset ownership. Add no second recovery queue or periodic DELBA timer.

## 4. Standard requirements and INET choices

Clause 10.25.2 requires the first frame under an accepted agreement to use the request sequence number. Deferred extraction is the proposed INET mechanism for that requirement. Clause 10.25.4 supplies the conditional DELBA requirement. The [transfer-completion section](#first-data-and-final-transfer) defines INET's interpretation of its trigger and separates expiration-driven teardown.

Clause 11.5.3.4 permits one narrower alternative to discard-all. The originator can retry the highest transmitted unacknowledged MPDU outside the agreement, with its sequence number retained. It must first discard earlier unacknowledged MPDUs. It cannot retry that MPDU within a new agreement.

Hypothetical example: DATA 100 through 102 remain unacknowledged at teardown. The proposed policy discards all three. The permitted alternative can retain only 102 for the first retry outside the agreement. Comparison with that alternative therefore concerns one additional eligible MPDU, not unrestricted retry of the batch.

The plan retains discard-all to avoid retry across agreement boundaries. A comparison can use an isolated experimental implementation of the permitted alternative. This comparison does not add a second production policy or configuration parameter.

| Choice | Classification and configuration |
| --- | --- |
| ADDBA response timeout | `addbaFailureTimeout` is a positive NED parameter. The proposed one-second default requires the review below. The value is INET policy. |
| DATA lifetime | The [lifetime plan](ieee80211-edca-data-lifetime.md#3-proposed-ownership-and-behavior) owns the parameter, default, and finite disposal bound. |
| DELBA after transfer completion | Clause 10.25.4 requires DELBA under its stated condition. INET selects the queue predicate in the transfer-completion section. |
| Discard-all at teardown | This is an explicitly selected option under Clause 11.5.3.4. User documentation must explain its disposal effect. |
| Recovery after uncertain setup | HCF waits for acknowledged local DELBA or received peer DELBA for the selected role. This is INET policy. |
| Valid profile-conflict BAR | C2 queues recipient-role DELBA through the existing owner. Clause 11.5.4 covers absent agreements; this active-conflict rule is INET policy. |
| Repeated setup after profile conflict | The post-conflict disposition remains a C2 design gate. Ordinary DELBA supplies no new variant-negotiation information. |

### Response timeout and DATA lifetime

The response timeout starts after request acknowledgment; DATA lifetime starts earlier at MAC acceptance. Pending setup prohibits aggregation that could restart an A-MSDU deadline. With the proposed defaults, DATA already deferred at request acknowledgment expires before the response deadline if setup remains unresolved. Later arrivals can survive longer. One lost response does not imply this outcome: a successful response retry before DATA expiry can release the deferred DATA.

Hypothetical example with a 0.512-second DATA lifetime and one-second response timeout: DATA arrives at 0 s. Request acknowledgment occurs at 0.1 s, but no successful response reaches the originator. DATA expires at 0.512 s; the response deadline is 1.1 s unless unused-setup teardown clears it earlier. A successful response retry at 0.2 s instead releases that DATA for transmission before expiry. A different configured lifetime can change this order.

The response timeout bounds only the response wait after request acknowledgment. It does not bound earlier contention or later DELBA recovery. A shorter response timeout alone therefore cannot guarantee delivery within the remaining DATA lifetime. Tests must measure these intervals separately.

Before A4 implementation, the lifecycle review must resolve this default choice. The maintainer must either accept the one-second default with its DATA-loss consequence or select a justified shorter positive value. The plan must record that decision and its reason. User documentation must explain the accepted interaction with DATA lifetime. The decision remains pending; it does not block A1, A2, or A3a through A3c after their applicable review and prerequisites.

## 5. Reviewable stages and dependencies

| Stage and purpose | Owner and proposed change | Expected result and direct verification |
| --- | --- | --- |
| A1. Deliver accepted Actions exactly once. | HCF and the recipient data service repair duplicate dispatch and reception during an active response wait. Preserve immediate ACK and frame ownership. | A retried request produces one agreement action. DELBA during a response wait reaches its handler after one failed exchange outcome. |
| A2. Use the selected profile and negotiated values. | Policies validate peer, TID, token, status, policy, and parameters. Compressed also requires local and peer HT state. Reject unsupported modification without a state change. | Explicit Basic retains legacy ad hoc and non-HT setup. Compressed admission accepts eligible HT peers and refuses capability gaps. Response size, timeout, and A-MSDU permission control both roles. |
| A3a. Repair directional teardown and inactivity. | HCF, role handlers, and data services repair peer lookup, role selection, duplicate state, and absolute inactivity scheduling. The first retirement path also supplies the Action invalidation and accepted-exchange completion operations that it needs. | Both roles emit the correct deletion payload once. Removal preserves the opposite role. Overdue agreements retire without another expired deadline. Obsolete success responses cannot survive retirement as eligible work. |
| A3b. Repair association reset for both profiles. | Management supplies the peer reset to HCF before replacement. Use existing notifications or the required synchronous handoff. HCF reuses A3a's retirement operations. Trace consumers and callback order. | Reassociation clears old state, including Basic peers without HT records. Compressed state resets with identical capabilities. New DATA sees replacement state; another peer remains unchanged. Failed reassociation retains its disposition. |
| A3c. Complete Action disposal and absent-agreement recovery. | HCF and role handlers complete queue-disposal outcomes and obsolete-Action checks through the existing retirement operations. HCF checks recipient agreement state before DATA delivery or BAR dispatch. | Queue disposal produces one terminal outcome. Accepted transmissions finish safely without obsolete retries. Valid BAR without an agreement queues recipient-role DELBA without receive-state allocation. Valid Block Ack-policy DATA without an agreement is discarded and causes recipient-role DELBA. |
| A4. Preserve sequence identity through uncertain setup. | HCF, the originator handler, and the QoS data service implement setup intent, eligible extraction, response deadlines, and serialized recovery. Connect queue disposal and first-DATA expiration. | The first transmitted DATA uses the request number. Lost responses cannot release uncertain DATA. Failed DELBA disposes of its batch; later DATA can retry recovery. |
| A5. End a completed transfer. | Resolve the transfer-completion decision. HCF applies the accepted predicate after BAR-only work becomes available in the replacement plan's channel-access stage. | Test the accepted boundary with a finite burst and zero inactivity timeout. Later DATA waits for queued teardown. Preserve the conditional DELBA requirement. |

A3 denotes the three review units A3a, A3b, and A3c. Review them in that order, with direct reproductions and tests beside their changes. A3b and A3c reuse A3a's retirement operations. A4 depends on S2, A1, A2, and all three A3 units. A1, A2, and the A3 units do not depend on DATA lifetime. A5 belongs with the BAR channel-access integration in S4, because final completion needs the last exchange to occur.

Before S4 publication, its owner must verify changed Basic BAR timing against S4's pinned parent. This includes the final BAR that permits A5 transfer completion. The [umbrella airtime contract](ieee80211-compressed-blockack-replacement.md#airtime-acceptance-and-admission-dependencies) owns the observations, permitted-overrun classification, and any required correction. A future C2 result cannot replace S4's publication evidence.

The response-timeout decision precedes A4 implementation. The transfer-completion decision precedes A5 implementation. C2 must resolve persistent conflict before its mismatch path. These decisions do not delay independent S1 or R1 review.

These review units do not automatically define independent publication boundaries. Action invalidation and safe completion must accompany the first retirement or reset path that requires them. A3a and A3b cannot publish unsafe intermediate behavior while those operations await A3c. Hypothetical example: inactivity retires an agreement while its successful ADDBA Response remains queued. A3a must remove that obsolete response before another attempt; A3c later extends disposal and absent-agreement coverage. S3b repeats the repaired recipient tests with the accepted response buffer size after A2 establishes that negotiation result.

Complete [S3a's explicit Basic settings](ieee80211-compressed-blockack-replacement.md#s3a-demonstration-migration-and-fallback-inventory) before S3b. The umbrella owns the selector, consumer inventory, and separate parent evidence. S3b uses the completed S3a revision as its parent. Existing ad hoc topologies, PHY modes, traffic, fragmentation cases, and parameter studies remain. Basic eligibility requires no new HT capability exchange.

The umbrella owns the [fixture inventory and VHT contract](ieee80211-compressed-blockack-replacement.md#s3a-tracked-fixture-inventory-and-vht-scope). S3b adapts initialized handlers while Basic fixtures retain explicit Basic selection. Existing `ac` infrastructure fixtures must preserve real HT capability observations and successful Basic exchanges before C2. C2 adds matched Compressed exchanges and positive receipt bits; C4 verifies the final default.

A2 will separately verify the Compressed profile's admission boundary. Before C2, focused policy and setup tests must not claim a complete Compressed DATA/BAR exchange. Positive setup needs usable HT state; a capability gap must refuse setup before the unsupported builder runs. This boundary must not affect explicit Basic demonstrations.

The proposed `Ac_BlockAckFallback.test` selects Compressed and simplified AP/STA management. It removes peer HT state after initialization, before DATA. Association, local HT support, and Block Ack enablement remain. No Beacon refresh restores the removed records. Require Normal Ack DATA and no ADDBA, BAR, or Block Ack throughout the measured interval. The umbrella owns its overrides.

Hypothetical example: the existing ad hoc QoS run 1 selects Basic and has no peer HT records. It retains Basic setup and exchanges after A2. A separate Compressed case with the same capability gap uses Normal Ack. This distinction preserves legacy demonstration coverage without a current-standard claim.

Every proposed publication boundary must preserve a Basic path that passes direct checks. Shared lifecycle repairs also require profile-specific admission and cleanup checks. A partial recovery state machine is not publishable. If stages need one atomic change, record the concrete shared invariant before combination.

Basic remains in independently published prerequisites and the final implementation. Each stage records its parent revision and changed fingerprint causes. R1 rate publication has its own parent comparison. C2 repeats its notification integration after the lifecycle subscriber and publication both exist. No prerequisite alone claims complete Compressed support.

### Reassociation consumers and callback order

At the stated source revision, searches covered the `rateStateChangedSignal` symbol and `rateStateChanged` string in source, tests, configurations, and NED statistics. The inventory separates notification listeners from components that read MIB state directly. It does not establish the absence of external consumers.

| Consumer or contract | Confirmed source behavior | A3b obligation |
| --- | --- | --- |
| Production signal listeners and NED statistics. | No current production subscription or NED statistic consumes this signal. [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) will become a new subscriber under this plan. | Repeat both searches before implementation. Verify HCF cleanup through the new callback. |
| [MIB unit-test observer](../../tests/unit/Ieee80211MibPeerTeardown_1.test). | The explicit test subscription checks the boolean notification and state visible from callbacks. | Preserve its teardown assertions. Add observations for removal and replacement during reassociation. |
| [QosRateSelection](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) and [RateSelection](../../src/inet/linklayer/ieee80211/mac/rateselection/RateSelection.cc). | Both query `findPeerHtState` for peer-compatible mode selection. They do not subscribe to this signal. | Check final peer-compatible selection and the absence of production selection during the temporary gap. |
| [RateControlBase](../../src/inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.cc), AARF, and Onoe. | Current rate control handles mode-set changes and transmission feedback, without a subscription to this signal or direct peer HT lookup. | Verify that the reset causes no additional rate-control reset or data-rate statistic solely through the new MIB notifications. |
| [Public signal contract](../../doc/src/migration-guide/index.rst). | The boolean signal reports committed MIB changes without peer details. External consumers can subscribe. Basic peers can have no HT record. | Preserve this contract. Trace the separate association-reset handoff for Basic and document both HT replacement observations. |

For an existing HT peer, [removal and reinstallation](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.cc) can emit two notifications even when the replacement capabilities are identical. Removal exposes absent HT state so HCF can retire old agreement state. Reinstallation exposes replacement HT state. An absent record emits no removal notification; a non-HT reassociation need not produce this pair.

Hypothetical example: an AP commits successful reassociation for a peer with unchanged HT capabilities and an active Compressed agreement. The removal callback observes no peer HT state and HCF clears that peer's old agreement. The replacement callback observes installed HT state. New DATA selection occurs only after replacement, with valid peer-compatible modes. A listener's observation of absent state is intentional; a production transmission decision during that gap is not.

Before A3b implementation, trace all synchronous callbacks from removal through installation, including HCF queue-disposal and channel-access calls. Add a guard only when the trace shows a reachable decision that existing control flow cannot prevent. One simulation event prevents event interleaving; it does not prevent nested callbacks. The verification below must establish the proposed order through production AP and HCF paths.

## 6. Direct verification

- Completed recipient control: Retain all eight Basic scenarios in [Ieee80211BlockAckReordering_1.test](../../tests/module/Ieee80211BlockAckReordering_1.test). Use the exact selector in the [umbrella verification section](ieee80211-compressed-blockack-replacement.md#6-verification-and-scope-limits). The fixture supplies agreements and known buffer sizes. A2 must verify negotiation through HCF without removal of these recipient assertions. C2 and C4 must verify Compressed integration separately.

- Retained demonstrations: Repeat S3a's Basic assertions on every existing wireless hop before and after A2 and at C4. Record explicit Basic settings, peer addresses, TIDs, and positive fragment or frame receipt status. Ad hoc QoS run 1 must retain Basic without HT records; run 0 remains support-disabled. Check Compressed fallback separately.

- Existing fixtures: Run the umbrella inventory's S3b cases through their actual handlers. Select Basic for the retained synthetic cases. Require valid capability state only for Compressed cases. Preserve header, NAV, and protection assertions. Check A-MSDU permission only after an accepted agreement, with both allowed and denied cases. Retain successful Basic setup for eligible `ac` peers through full management and real capability exchange. Record any changed failure cause in the four existing deferred protocol fixtures.

- Absent-peer-state fallback, A2: Select Compressed in the umbrella fixture before static association and explicit peer HT removal. Require delivered Normal Ack QoS DATA with local HT support and Block Ack enablement retained. Check absent peer HT state throughout traffic at both ends of each measured hop. Require no ADDBA, BAR, or Block Ack during that interval. Repeat at C2 and C4 under the replacement plan's ownership. This test establishes fallback under the controlled gap, not a general `ac` limitation.

- ADDBA and DATA selection: Use production HCF tests for accepted and rejected requests, including zero request size. Check the built-in response rule `min(64, maximumAllowedBufferSize)` for request sizes below and above its result. Use a synthetic recipient for other legal response sizes and check the originator window. Check the first DATA number after deferred extraction. Put deferred peer A before eligible peer B in one queue. Confirm that B sends individual DATA and A remains unnumbered.

- Negotiated recipient window, A2: Accept an agreement with request start 100 and buffer size eight. Before any DATA reception, send a valid BAR at 104. Require record creation from the accepted values through the recipient prerequisite's reorder owner. Check reorder start 104. Receive DATA 104. Require immediate delivery.

This check verifies negotiation integration; S1 owns the underlying recipient repair.

- First agreement transmission: Check that A-MSDU denial prevents aggregation for the accepted peer and TID. Accept ADDBA with request number 100, then expire extracted DATA 100 before Tx accepts it while DATA 101 remains. Check teardown before 101 can transmit and absence of sequence reuse. Repeat with terminal RTS failure before DATA transmission. An accepted DATA attempt that completes across its deadline must not trigger this untransmitted-DATA rule.

- Duplicate setup: Lose the request ACK after recipient acceptance. Retry that request before the successful response reaches the originator. Check one agreement addition and no rejection caused by that MAC retry. Test loss of the successful response through its retry limit. Test a genuine new modification request separately. Its rejection must preserve the active agreement.

- Uncertain setup: Lose an ADDBA Response after recipient acceptance. Keep same-peer and same-TID DATA deferred until DELBA receives an ACK or DATA expires. Check the same rule after a transmitted ADDBA Request reaches its retry limit. A failed request attempt below that limit must preserve deferral. A request discarded before Tx accepts any attempt may release DATA immediately. A late response must not replace teardown state.

- Response retry before expiry: Lose one successful ADDBA Response. Deliver its retry before the deferred DATA deadline. Check acceptance and DATA release without timeout teardown. Permit channel access before expiry. Check DATA delivery.

- Timeout and lifetime order: Suppress successful responses through DATA expiry with no later arrivals. Check disposal of the original batch and unused-setup teardown. In a separate case, supply later DATA so the response timeout remains reachable. Record MAC acceptance, request acknowledgment, DATA expiry, response timeout, and DELBA completion times. Repeat these checks with the accepted default and a shorter positive timeout. Attribute each disposal to its actual cause.

- Setup completion order: After request ACK timeout, receive an accepted response before the queued request retry transmits. Check removal of that retry and absence of a new response deadline. Check that a stale request failure cannot remove the accepted agreement. Confirm that DATA already numbered before setup intent resolves before ADDBA starts. Confirm that unnumbered DATA remains deferred.

- Reception during a response wait: Deliver valid ADDBA and DELBA Actions while HCF awaits an exchange response. Check one unsuccessful exchange outcome before recipient dispatch, immediate ACK, duplicate suppression, and safe frame ownership. Check that recipient dispatch changes the agreement before another contention attempt can select DATA. Repeat DELBA with both Initiator values.

- Action queue disposal: Reject or evict owned ADDBA Requests, Responses, and DELBA frames. Repeat disposal before any attempt and after an earlier failed attempt. Check exactly one terminal outcome and the transition table's resulting state. A dropped successful response must leave its recipient agreement active. A dropped originator DELBA must enter recovery-needed state and dispose of its deferred batch. A dropped recipient DELBA must preserve the opposite-direction agreement.

- Failed DELBA: Let the peer process DELBA, but lose every ACK until the retry limit. Check disposal of the current deferred batch. Check that no periodic DELBA retry occurs without new DATA. Inject later DATA and permit the fresh DELBA exchange to succeed. Confirm Normal Ack delivery without peer reset. Another failure must dispose of only the new deferred batch.

- Profile conflict, C2: Keep both directions active for one peer/TID. Inject a well-formed retained BAR for the other profile. Require one recipient-role DELBA and no receipt, reorder, response, or inactivity-success update. Preserve the opposite role and another peer. Repeat BAR during pending teardown and an active exchange in the opposite direction. Preserve accepted-frame references through safe completion. Check exact deletion and Action outcomes. Record recipient delivery separately from originator disposal.

- Persistent conflict, C2: Test unchanged opposing settings and bounded repeat setup under the accepted disposition. Test compatible setup under its stated initialization or reset conditions. A finite DATA deadline alone cannot pass this recovery check. Run these assertions before publication of the conflict path.

- Missing agreement, A3c: Inject a valid addressed BAR without an active agreement. Check DELBA, no immediate response, and no scoreboard or reorder allocation. Inject DATA with Block Ack policy under the same condition. Check discard before delivery and recipient-role DELBA. Invalid BAR must cause neither allocation nor DELBA. Repeated valid frames must not create parallel DELBA exchanges. C2 repeats the BAR boundary after Compressed format validation exists.

- Directional teardown: Establish both directions for one peer and TID. Test both Initiator values with local and received DELBA. Check removal of only the selected role. Teardown with unacknowledged DATA 100 through 102 must discard all three. Check that unsent DATA preserves sequence continuity beyond 102. Check recipient duplicate state after out-of-order reception and sequence wrap.

- Deletion signals, A3a: Receive DELBA for each role with both directions active. Check one nonnull deletion payload for the selected peer, TID, and role before removal. Check removal of that agreement and preservation of the opposite role and another peer. Repeat DELBA after removal. Check that no second deletion event occurs.

- Queued Actions: Queue DELBA after inactivity expiration, then deliver new ADDBA before DELBA transmits. Check that the recipient rejects setup until teardown completes. Establish a later agreement and check that no obsolete DELBA reaches it. Repeat peer cleanup with queued ADDBA Request, Response, and DELBA. Repeat with an Action already accepted by Tx. Check safe completion without a retry or state change from obsolete work.

- Obsolete success response: Expire recipient inactivity before a queued successful ADDBA Response transmits. Check removal of that response, one deletion event, and serialized DELBA. Repeat after Tx accepts the response. Check safe completion without restored agreement state or another response retry. Repeat ordinary received DELBA while owned ADDBA work remains queued.

- Association reset: Test disassociation and reassociation on both AP and station under both profiles. Include Basic peers with no HT record. Observe the explicit management reset handoff and preserved Basic state after unrelated rate notifications. Reassociate to the same AP with identical capabilities and operation values. Check removal of old agreements, duplicate state, and obsolete Action work. Preserve another peer's state. Check that failed reassociation follows the existing association disposition. Check one deletion event per active agreement and none for inactive records.

- Reassociation consumers, A3b: Commit identical-capability reassociation through production AP management with an active agreement. Observe absent peer HT state during removal and installed state after replacement. Instrument DATA selection and ACK-policy queries for the reset peer during that interval. Require no such production decision before replacement. Check peer-compatible rate selection, preserved rate-control state, and no notification-driven data-rate statistic. Repeat with failed reassociation and no prior HT record.

- Inactivity: Use different request and response timeout values. Check that both roles use the response value, including zero. Test expiration after nonzero simulation time with two role deadlines. Receive matching BAR just before expiration and check deadline refresh. Repeat with qualifying duplicate DATA, Normal Ack DATA, and another TID. Check one timeout DELBA and no same-time reschedule loop.

- Inactivity disposal, A3a: Expire an agreement while transmitted DATA remains unacknowledged. Check discard-all for that agreement's transmitted DATA. Preserve unsent DATA and the opposite-direction agreement under their applicable contracts. After S2 integration, repeat with a DATA lifetime deadline in the future. Record inactivity-driven teardown disposal separately from lifetime expiry and other teardown causes. Check that A5 completion has no unresolved DATA for discard-all after A5 exists.

- Empty setup and defaults: Send one Normal Ack DATA frame without a successor and check that no ADDBA follows. Expire all deferred DATA during setup intent, before request transmission, and after request transmission. Check local cancellation or serialized teardown according to possible peer state. Accept a response with no DATA and check immediate unused-agreement teardown, including with zero inactivity timeout. With the configured defaults, check DATA expiration before response-timeout recovery can preserve that original batch. Keep newer DATA available in another case to exercise the response timeout itself.

- Final transfer: Under the proposed predicate, complete a finite burst with no queued or in-progress DATA for that agreement. Check DELBA after the final Block Ack exchange with a zero inactivity timeout. Inject DATA after DELBA enters the queue. Check that it waits for teardown rather than canceling DELBA. Expiration of the final unresolved DATA must also end the transfer. Another peer's queued DATA must not suppress this completion.

Before A5 implementation, revise the final-transfer assertions if the accepted boundary differs from the proposed predicate.

Use the current Basic variant for variant-independent prerequisite reproductions. Repeat the relevant cases with Compressed frames when the replacement exists. Agreement rejection and missing-agreement cases must preserve their Action and disposal results under either variant.

Add a proposed `tests/module/Ieee80211BlockAckAgreementLifecycle_1.test` for these production paths. Partition its scenarios by A1, A2, A3a, A3b, A3c, A4, and A5. Give each review unit a precise selector. Retain prerequisite invalidation checks wherever an earlier unit consumes those operations. Confirm the actual selector syntax when the fixture exists.

## 7. Performance and evidence limits

Use deterministic fault injection for successful setup, lost ADDBA responses, and lost DELBA acknowledgments. Use finite bursts and traffic with idle gaps. Record the traffic definition, offered load, seed, rates, queue capacity, and relevant timeout parameters.

Measure these results separately:

- Measure setup deferral from the start of DATA ineligibility to release or disposal.
- Measure request contention, response wait, and DELBA completion as separate intervals.
- Measure delivered DATA, lifetime disposal, teardown disposal, and management-frame airtime.
- Separate inactivity-driven teardown disposal from other teardown causes. After S2 integration, record whether each discarded frame still had an unexpired DATA lifetime deadline. Count A5 completion separately from inactivity expiration.
- Count ADDBA and DELBA exchanges per burst, including retransmissions.
- Measure application goodput as useful bytes delivered to the destination application per unit of simulated time over a fixed measurement interval.
- Measure delivery delay for the affected TID and progress for another eligible peer or TID.
- Count the highest transmitted unacknowledged MPDU eligible for the optional retry at each teardown.

For a performance estimate against optional final-MPDU retry, implement that alternative only in a controlled comparison. Do not infer recovered delivery from an eligible-frame count. Report the count alone when that experiment does not exist.

For transfer completion, compare equal payload totals in continuous traffic and finite bursts separated by controlled idle gaps. Record burst size, gap duration, and the number of agreement establishments and teardowns. Attribute management airtime and DATA deferral to the observed exchanges. These measurements expose repeated setup cost; traffic-shape differences alone do not isolate the predicate's cost.

An isolated comparison may retain agreements across idle gaps to estimate that cost, with all other inputs fixed. Such an experimental policy is not a standards exception or a second production mode. Without that experiment, report observed exchange counts, airtime, delay, and goodput without a claim about agreement-reuse performance.

The [replacement plan's verification scope](ieee80211-compressed-blockack-replacement.md#6-verification-and-scope-limits) defines common fingerprint controls. Diagnose changed exchanges separately for each stage. No particular performance improvement or acceptable loss threshold is assumed.

## 8. Standards evidence and scope limits

IEEE Std 802.11-2024 supplies the Compressed profile's normative requirements. Canonical clause identifiers use `ieee80211-2024:clause:` followed by the number. Basic is historical compatibility, unsupported by that standard. IEEE Std 802.11e-2005 Clauses 7.2.1.7, 7.2.1.8, and 9.10 introduced its format and exchange. The umbrella's [historical references](ieee80211-compressed-blockack-replacement.md#2-standard-support-and-model-boundary) link the publication record and published amendment. Shared repairs do not establish complete historical conformance.

| Requirement | Clauses | Physical PDF pages |
| --- | --- | --- |
| Setup and accepted parameters | 10.25.2; 11.5.2.2 | 2051–2056; 2561–2562 |
| Final-transfer teardown | 10.25.4 | 2056 |
| Originator disposal and recipient duplicate state | 11.5.3.4; 11.5.3.5 | 2565–2566 |
| Inactivity and absent-agreement recovery | 11.5.4 | 2566 |

The source review does not establish complete HT-immediate transfer support. A-MPDU, implicit BAR, and complete-cost TXOP admission remain separate work. Existing MAC stop/start limitations remain explicit; this plan does not claim their repair.

Follow [the plan procedure](../../doc/project/guide/write-an-implementation-plan.md) and [run-the-gates.md](../../doc/project/guide/run-the-gates.md). The implementation must record exact commands, modes, selectors, and outcomes. The separate S1 record owns completed recipient verification. This plan reports no executed lifecycle test or performance result.
