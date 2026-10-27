# Block Ack agreement lifecycle

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc), [implementation status](ieee80211-compressed-blockack-replacement.md)

HCF keeps each Basic Block Ack agreement consistent with its role, peer, and traffic identifier (TID). This document describes implemented setup, DATA deferral, Action handling, teardown, and peer reset. Compressed agreements use the same lifecycle owners with the HT eligibility and profile rules in the implementation status.

## Master prerequisites and S5 scope

Master contains stage S4 through merged [PR #1344](https://github.com/inet-framework/inet/pull/1344). Its lifecycle supplies BAR-only contention, peer/TID-specific feedback, window progress, DATA Retry, and completion DELBA. PR #1343 supplies setup, Action dispatch, and directional teardown. PR #1368 supplies association replacement cleanup. PR #1331 supplies the finite DATA deadline.

S5 uses these owners for immediate Compressed agreements. Both endpoints must select the same profile because ADDBA does not negotiate it. Usable local and directional peer HT state controls Compressed eligibility. Active Compressed agreements suppress fragmentation. A conflicting addressed BAR stops the simulation before agreement updates. The [implementation status](ieee80211-compressed-blockack-replacement.md) defines the complete profile and admission boundary.

## 1. Core behavior

The originator and recipient agreement handlers own separate role-specific records. HCF coordinates the exchange and DATA eligibility. ADDBA establishes an agreement; DELBA ends it. A Block acknowledgment request (BAR) requests reception status for DATA within the agreement.

HCF defers new sequence assignment while setup or teardown makes the peer state uncertain. Another peer remains eligible. An accepted transmission retains its frame context until safe exchange completion. Logical retirement does not permit deletion of a frame that Tx still references.

Hypothetical example: the recipient accepts an ADDBA Request, but the originator receives no successful response. The originator keeps that peer/TID's DATA deferred. Response timeout starts DELBA recovery. Confirmed teardown permits surviving unsent DATA to resume with Normal Ack.

## 2. Current owners

| Component | Responsibility |
| --- | --- |
| [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) | Coordinates eligibility, Action dispatch, timers, teardown, and transfer completion. |
| [Originator handler](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.cc) | Owns originator agreements, dialog tokens, and transient procedures. |
| [Recipient handler](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckAgreementHandler.cc) | Owns accepted recipient agreements and negotiated receive values. |
| [Originator data service](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc) | Previews and extracts eligible DATA with aggregation permission checks. |
| [Recipient data service](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.cc) | Applies duplicate, reassembly, receive-state, and teardown disposition rules. |
| [Reorder owner](../../src/inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.cc) | Releases complete units in cyclic order and owns reorder records. |

## 3. Agreement behavior

### Setup and negotiated values

After initial eligible DATA, HCF waits for a successor before setup. HCF resolves already numbered DATA before it obtains the next sequence number without consumption. The ADDBA Request therefore names the first DATA sequence for the new agreement. DATA extraction remains deferred through the uncertain setup interval.

The originator accepts success only for the pending peer, TID, and nonzero dialog token. The response must select immediate policy, a positive buffer size no greater than 64, and a nonnegative inactivity value. The originator uses a new token for each request to that peer and TID. The [originator policy](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementPolicy.cc) applies these checks.

The recipient validates request fields and rejects delayed or unsupported requests. It rejects a modification request while the agreement is active, without replacement of that agreement. A rejected request produces an unsuccessful response with the request token and creates no agreement.

The recipient selects `min(64, maximumAllowedBufferSize)` and its configured inactivity value. The request buffer size and timeout are advisory. The agreement uses the response capacity and inactivity value with the request starting sequence. Zero inactivity disables inactivity expiration; it does not disable DATA lifetime or completion DELBA.

Both negotiated A-MSDU flags must permit aggregation for that peer and TID. The data service otherwise extracts individual DATA. The ACK policy rejects an already-built A-MSDU without permission; Normal Ack cannot bypass that restriction. Setup and teardown deferral prevent new aggregation while peer state remains uncertain.

### Action dispatch and response waits

The recipient data service checks complete management frames and MAC duplicates before HCF dispatches accepted ADDBA or DELBA Actions. Valid duplicates retain immediate ACK behavior but produce no repeated agreement action. The existing management duplicate cache supplies this check.

During a response wait, HCF retains an addressed Block Ack Action outside the response step. A matching ADDBA Response or directional DELBA can resolve the procedure before obsolete terminal failure feedback occurs. The frame-sequence owner then cleans the interrupted exchange without obsolete retries, batch disposal, or a second terminal outcome. An unrelated Action retains ordinary exchange failure behavior.

HCF acknowledges the accepted frame after cleanup and avoids repeated dispatch or duplicate-cache updates. Tx-accepted outgoing Actions retain their context until safe completion. Replacement setup waits for obsolete accepted Actions to finish.

Hypothetical example: a successful ADDBA Response arrives during the request's final ACK wait. The originator accepts the matching response before failure notification. Cleanup starts neither request retry nor teardown for that resolved request.

### Directional teardown and recovery

Received DELBA uses its transmitter address as the peer key. `Initiator=1` ends the local recipient agreement; `Initiator=0` ends the local originator agreement. The opposite role remains unchanged. Transmitted DELBA uses its receiver address as the peer key.

HCF retires the selected active agreement once and retains the required teardown procedure. For the originator role, surviving unsent DATA waits for confirmation. ACK reception, retry-limit failure, or queue disposal completes the DELBA exchange. Transmission completion alone does not release that procedure.

Originator teardown selects discard-all for transmitted unresolved DATA, including the highest transmitted MPDU. It preserves sequence continuity for surviving unsent DATA beyond that transmission boundary. It neither reuses discarded sequence numbers nor moves allocation backwards. Inactivity teardown can therefore remove unresolved DATA before its lifetime deadline.

Confirmed teardown releases surviving unsent DATA under Normal Ack. Failed originator DELBA drops its deferred batch and leaves recovery needed. New DATA triggers one fresh DELBA recovery attempt. Failed recipient-role DELBA clears its procedure without disposal of opposite-direction DATA.

A valid Basic BAR or Block Ack-policy DATA without an active recipient agreement causes recipient-role DELBA. The DATA does not reach upper delivery. The BAR creates no scoreboard or reorder record and receives no Block Ack. Malformed input does not trigger this recovery action.

### Recipient buffer disposition at teardown

Ordinary recipient teardown delivers complete buffered MSDUs or A-MSDUs before resource release. The recipient data service preserves required duplicate state. The reorder owner detaches the record and collects complete units in cyclic order. Incomplete Basic units follow teardown disposal. HCF delivers complete units before the agreement deletion event.

This release across gaps is an INET policy choice. It applies to ordinary DELBA and inactivity retirement while the association remains valid. Detachment prevents repeated delivery. The opposite agreement and unrelated peers retain their state.

Hypothetical example: DATA 100 is absent, while complete DATA 101 and 102 remain behind that gap. The originator resolves its final outstanding DATA through lifetime expiry and sends DELBA. The recipient releases 101 and 102 in order before cleanup. Their earlier acknowledgment alone did not establish delivery.

Association reset uses the separate discard rule below. It does not deliver old buffered units into a replacement association.

### First DATA and final transfer

The originator agreement records actual DATA transmission. Extraction alone does not satisfy that condition. If the requested first DATA suffers terminal disposal before transmission, HCF retires the agreement and starts serialized teardown. Surviving unsent DATA waits; it cannot become the first transmission under the obsolete starting sequence.

HCF completes an active transfer only after first DATA transmission, with no numbered or queued DATA for that peer and TID. Completion occurs at a safe exchange boundary. HCF retires the agreement and queues originator DELBA. Zero inactivity timeout does not suppress this path.

Lifetime expiry of the final unresolved DATA can trigger the same completion policy. Later DATA does not cancel queued DELBA. Repeated bursts can therefore require new contention and setup. No general performance bound follows from that policy.

### Agreement transitions

The role owners retain setup intent, pending request, active agreement, teardown, and recovery state as separate conditions. A matching response resolves setup. Uncertain setup requires teardown before DATA resumes. Directional DELBA affects one role. Peer reset clears both roles for the selected peer before callbacks.

The [verification record](ieee80211-compressed-blockack-s4-execution.md) identifies the current runtime selection. Source behavior outside that selection remains a source observation rather than a new test result.

### Profile conflict detection and termination

HCF accepts only Basic as an operational profile in this checkout. Compressed selection fails initialization. The checkout therefore supplies no end-to-end active Basic/Compressed conflict diagnostic or recovery claim. Helper admission predicates do not enable a second production profile.

## 4. Standard requirements and INET choices

The current Basic format is a compatibility profile. IEEE Std 802.11-2024 reserves its type value. The [standards boundary](ieee80211-compressed-blockack-replacement.md#excluded-features-and-fragmentation) separates that profile from current-standard Compressed operation.

The source cites IEEE Std 802.11-2024 for shared lifecycle requirements. Clause 10.25.2 supplies setup and first-sequence conditions. Clause 10.25.4 covers completion DELBA. Clause 11.5.3.4 supplies teardown disposal alternatives; this implementation selects discard-all. Clause 11.5.3.5 covers recipient duplicate-state retention. These shared rules do not establish complete conformance for Basic.

### Response timeout and DATA lifetime

`addbaFailureTimeout` defaults to 100 ms after request acknowledgment. It bounds the wait for a response, not channel contention or DELBA recovery. The originator policy requires a positive finite timeout.

The response inactivity value controls both active roles. Each handler uses an absolute deadline. HCF schedules that absolute time and retires due or overdue agreements. Zero disables inactivity expiry.

The separate [QoS DATA lifetime](ieee80211-edca-data-lifetime.md) defaults to 0.512 seconds. Setup deferral, retries, and ACK-policy changes consume that deadline. Agreement activity cannot extend it. An accepted transmission can finish safely across its DATA deadline, but another attempt cannot start at or after that deadline.

## 5. Queue and association integration

### Selected-extraction contract

The originator data service uses `canPullPacket()` to inspect queue service order. If the selected DATA is the advertised next frame, individual extraction uses ordinary dequeue. Otherwise, filtered `canPullPacket()` and `pullPacket()` calls select eligible DATA behind a deferred frame. The service validates every borrowed aggregation candidate before extraction. Invalid candidates cause individual extraction of the selected eligible frame, without extraction of other sources.

The queue owner retains responsibility for ownership, residence metadata, pull events, and compound accounting. This checkout defines no separate optional selected-extraction interface. The [migration guide](../../doc/src/migration-guide/development/ieee80211-basic-block-ack-extension-points.rst) describes extension obligations.

### Reassociation consumers and callback order

Management calls `resetPeerAssociationContext()` before replacement of the selected peer association. The MAC passes that call to HCF's `resetPeer()`. This direct call identifies a reset even when capability values remain identical. A MIB capability-change notification cannot serve that purpose.

Both role owners detach the peer's active agreements and transient procedures before callbacks. HCF invalidates obsolete Actions and clears transmit, acknowledgment, recovery, reorder, reassembly, duplicate, and lifetime state for the old association. Cleanup discards old buffered units without delivery. Only after cleanup does HCF emit deletion events for active agreements. Transient records produce no agreement deletion event.

Another peer remains unchanged. The MIB retains capability ownership and publishes coherent committed state separately. An accepted transmission retains safe completion protection through reset.

## 6. Direct verification

Current selected module files cover Action delivery and disposal, negotiation, association reset, DATA selection, deferral, collision, lifecycle, fragment retirement, response waits, scheduling, and completion cost. The [verification record](ieee80211-compressed-blockack-s4-execution.md) gives the executed commands and counts. A fixture name alone does not establish a result for every path above.

## 7. Performance and evidence limits

Completion DELBA and a new setup can add management exchanges between bursts. Basic single protection also requires a new TXOP for unprepared DATA. Current validation supplies no general throughput, latency, or agreement-reuse cost result. MAC stop/start recovery, production Compressed exchanges, and complete NAV conformance remain outside the claim.

## 8. Standards evidence and scope limits

The clause references above identify the source requirements and current policy boundaries. The document adds no new normative interpretation or standards retrieval result. The [implementation status](ieee80211-compressed-blockack-replacement.md) states the supported profile and capability gaps. Runtime evidence remains limited to the current verification selection.
