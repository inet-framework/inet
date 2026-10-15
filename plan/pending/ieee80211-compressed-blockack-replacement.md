# Implementation plan: add Compressed Block Ack and retain Basic Block Ack

Integration source: local `master` at `633b071e213b825ce6019ff3e344ae49f52791ff`, with S1, R1, and S3a merged.
Plan revision: 2026-10-15, revision 31.
Review checkout: `/home/user/omnetpp_ws/inet-compressed-blockack-s2`, branch `work/compressed-blockack-s2`, at `d2fcb528b67f9be376caa06d68ea0c1c028ab145`.
Source basis: the pinned integration source and IEEE Std 802.11-2024. Earlier direct PDF inspection supplies the historical IEEE Std 802.11e-2005 evidence. Local S2 source, test, and plan edits are separate from merged master. This review preserves those edits. Remote verification failed because GitHub access was unavailable; the integration pin describes local master.

Approval: the maintainer's 14 October 2026 instruction, "proceed", after the revision-24 review authorizes these corrections to four plan documents. The [S1 source record](../done/ieee80211-compressed-blockack-s1.md) owns its separate implementation, baseline, and publication authorization. This document approval authorizes no later implementation, recorded expectation change, or publication. The [decision table](#maintainer-decisions) records accepted choices and remaining stage requirements.

Decision approval: on 14 October 2026, the maintainer accepted the 0.512-second DATA lifetime default, transfer-completion predicate, and individual-DATA scope. The maintainer subsequently selected a 100 ms ADDBA response timeout and a fatal configuration error for active profile conflict. The [decision table](#maintainer-decisions) records all five accepted choices. These instructions authorize their record in the plans, not implementation, baseline changes, or publication. The maintainer's subsequent instructions authorize the review corrections and strict application of the individual-MPDU release clause in revision 30. Its timestamp mapping, event observation, and state ownership are implementation design work, not additional policy decisions.

The earlier "proceed" for revision 30 authorizes its text corrections and explicit escalation and publication rules. The current "proceed" authorizes the remaining document corrections for merged R1 and S3a. Strict compliance remains required; no model approximation is approved.

S1 merged through [PR #1320](https://github.com/inet-framework/inet/pull/1320) at `1cdb362a2a4b97f564566487079d9d156a5d213d`. Its original verified head is `fcc06450355a52b9c8b0f8f2647e0654711bfaca`. [Appendix A](#completed-s1-and-recorded-verification) preserves that head's scope and recorded evidence. R1 and S3a are also merged, as the [completed prerequisite record](#completed-r1-and-s3a) explains. This revision adds no runtime verification of the combined source. The remaining stages retain their separate review and verification requirements.

Revision 31 follows the maintainer's request to account for merged R1 and S3a. It corrects stage status, source assumptions, and dependent verification. It grants no new implementation, baseline, or publication authority.

This plan defines Compressed support beside retained Basic support and orders its prerequisite changes. The [EDCA lifetime plan](ieee80211-edca-data-lifetime.md) owns DATA expiration. The [agreement lifecycle plan](ieee80211-blockack-agreement-lifecycle.md) owns setup, recovery, and teardown. Section 5 defines separate review and regression boundaries. The complete design is not one implementation change.

## 1. Core intended behavior

INET will default to immediate Compressed Block Ack for its supported non-DMG HT stations. An explicit Basic profile will preserve the existing legacy demonstrations. An HT station supports high throughput operation. A Block Ack agreement identifies one peer and traffic identifier (TID). ADDBA establishes that agreement. A Block acknowledgment request (BAR) asks the recipient for a Block Ack response.

The originator will send Compressed BAR only when an immediate agreement applies to the peer and TID. The recipient will send Compressed Block Ack after a valid addressed Compressed BAR for that agreement. The BAR carries a starting sequence number, not a range length. The recipient will build its 64-bit bitmap from its adjusted scoreboard window. Its response starting sequence number may differ from the BAR value.

The originator will apply bitmap outcomes only to frames that the response covers. INET will retain Basic BAR, Basic Block Ack, their C++ classes, serializers, and fragment bitmap support. Basic is a historical compatibility profile, not a variant supported by IEEE Std 802.11-2024.

Hypothetical example: two associated HT stations have an accepted agreement for TID 5 with starting sequence number 4094. The recipient's scoreboard starts at that number. The [lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#3-agreement-behavior) defines setup and DATA eligibility.

The originator sends four unfragmented QoS DATA frames with Block Ack policy. No unsent DATA remains. The originator requests channel access for a Compressed BAR with starting sequence number 4094. The recipient sends a Compressed Block Ack after SIFS. Its bits for sequence numbers 4094, 4095, 0, and 1 follow modulo-4096 order.

The originator removes acknowledged frames for that peer and TID. Missing frames remain available for retry within the [lifetime contract](ieee80211-edca-data-lifetime.md#3-proposed-ownership-and-behavior). The [lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#first-data-and-final-transfer) owns transfer completion and teardown.

Under the Compressed profile, absent usable local or peer HT state selects Normal Ack and prevents agreement setup. The Basic profile retains the existing capability assumptions, including ad hoc operation without peer HT records. It does not claim current-standard conformance. The [lifecycle prerequisite](ieee80211-blockack-agreement-lifecycle.md#3-agreement-behavior) owns recovery when a valid frame arrives without an agreement.

The current `ac` mode supplies HT operation and peer HT state through its inherited HT support. VHT means very high throughput. An associated `ac` peer with usable HT state remains eligible for this profile. The [fixture inventory](#s3a-tracked-fixture-inventory-and-vht-scope) retains `Ac_BlockAck` as a positive requirement. Normal Ack fallback requires an actual capability gap; `opMode = "ac"` alone does not establish one.

## 2. Standard support and model boundary

IEEE Std 802.11-2024 Tables 9-36 and 9-37 assign variant type 0 to Reserved. Both tables assign type 2 to Compressed. Their corpus identifiers are `ieee80211-2024:table:9-36` and `ieee80211-2024:table:9-37`. They appear on physical PDF pages 719–720 and 722–723.

IEEE Std 802.11e-2005 introduced the Block Ack mechanism and the frame format later called Basic Block Ack. Clauses 7.2.1.7 and 7.2.1.8 define BAR and Block Ack; Clause 9.10 defines their operation. Clause 7.2.1.8 specifies a 128-octet bitmap for 64 MSDUs, with fragment status. Direct inspection of `../inet-standards/80211e-2005.pdf`, relative to the repository root, confirms printed pages 30–32 and 96–100. These are physical PDF pages 52–54 and 118–122. The [IEEE publication record](https://standards.ieee.org/ieee/802.11e/3131/) identifies Amendment 8 and its 11 November 2005 publication.

Retained Basic support models that historical format and preserves existing INET studies. IEEE Std 802.11-2024 does not support Basic; type 0 remains reserved in that standard. This retention does not claim complete IEEE Std 802.11e-2005 conformance. Existing Basic demonstrations can use later PHY modes and existing INET response behavior. User documentation must distinguish the historical frame format from those model choices.

Figures 21b and 21e of IEEE Std 802.11e-2005 reserve control bits B0–B11 and assign B12–B15 to TID. INET's retained ACK-policy fields and serializer values are compatibility choices, not field semantics from those figures. C1 preserves the supported Type 0 contract below without a complete historical conformance claim.

Table 11-8 permits the Compressed profile's immediate agreement when both non-DMG stations are HT stations and ADDBA selects immediate policy. Clause 10.25.2 requires the first DATA sequence number to equal the accepted ADDBA Request starting sequence number. Clause 10.25.6.1 requires HT-immediate Block Ack support for an HT recipient. Clause 9.3.1.7.2 sets the Compressed BAR fragment subfield to zero. Clause 9.3.1.8.2 and Table 9-38 define the Compressed Block Ack bitmap. For a non-HE station, the fragment subfield is zero and the bitmap has 64 positions.

Clause 10.25.6.5 requires a response after SIFS for an addressed BAR during full-state or partial-state operation. It requires a zero bitmap when no matching partial state exists. This plan selects full-state operation for an established agreement. The [lifecycle standards evidence](ieee80211-blockack-agreement-lifecycle.md#8-standards-evidence-and-scope-limits) covers absent-agreement recovery.

The [lifetime evidence](ieee80211-edca-data-lifetime.md#2-current-source-and-standard-support) covers the DATA disposal bound. The lifecycle plan separates standard requirements from its transfer-completion interpretation. Clause 9.2.5.6 defines the response Duration/ID subtraction. Clauses 10.6.6.5.2 and 10.6.6.5.4 define primary and alternate response rates.

This change covers immediate, one-TID Compressed BAR and Compressed Block Ack in the existing non-DMG HT MAC. It uses a 64-position bitmap and an explicit BAR exchange. The Compressed recipient keeps full-state status under Clauses 10.25.6.2 and 10.25.6.3. The selected INET profile sends individual QoS DATA frames before BAR. Clause 10.25.2 describes transfer in A-MPDUs, while Clause 10.25.3 describes BAR for outstanding QoS DATA frames. This plan does not claim complete HT-immediate transfer coverage without A-MPDU.

### Purpose of the individual-DATA profile

The accepted user need is controlled study of Compressed bitmap, response, and recovery behavior in INET's current individual-frame MAC. C2 will supply a directly observed Compressed BAR and Block Ack exchange for that purpose. The frame and scoreboard contracts can also support a later A-MPDU design. That later design remains separate.

The response MPDU decreases from 152 bytes for Basic to 32 bytes for Compressed. This frame-length result does not establish lower exchange airtime or higher application goodput. Control rates, explicit BAR, contention, and repeated setup also affect those results. Section 6 requires direct exchange and airtime evidence without an assumed performance gain.

On 14 October 2026, the maintainer accepted this intermediate scope for the stated user need. The scope decision before C2 is satisfied. Complete HT-immediate transfer coverage requires a separate A-MPDU design. Wire definitions and independent prerequisite repairs do not establish that complete coverage.

### Excluded features and fragmentation

The plan excludes A-MPDU, implicit BAR, HE bitmap lengths, dynamic fragmentation, delayed Block Ack, Multi-TID Block Ack, and GCR. Complete-cost TXOP admission and BAR forecasts also need separate designs.

Clause 10.2.7 prohibits MSDU fragmentation under the Compressed profile's HT-immediate agreement, even above the fragmentation threshold. Normal Ack does not bypass this restriction. C2 will suppress fragmentation only for that profile's active agreement and peer/TID.

Basic retains historical fragment handling in the existing fragmentation showcases. Local HT support alone does not turn a Basic compatibility agreement into this Compressed agreement. Outside an agreement, fragmented DATA uses Normal Ack. The lifecycle prerequisite resolves already numbered fragments before setup; C2 tests that boundary with Compressed operation.

Hypothetical example: the threshold is 250 bytes and a queued MSDU contains 1000 bytes. With an active HT-immediate agreement, the data service extracts it without fragmentation. Without an agreement, the existing fragmentation policy applies and each fragment uses Normal Ack.

A positive bitmap bit in this profile will acknowledge a complete MSDU or A-MSDU. One fragment alone will not produce a positive bit. Table 9-38 assigns one bit to each unit when the Fragment Number subfield is zero. Clauses 9.4.1.13 and 10.25.6.2 apply the response's A-MSDU permission to DATA for the agreement's peer and TID. A change to Normal Ack alone does not permit A-MSDU DATA when that permission is absent.

## 3. Current source path and evidence limits

These observations use the pinned integration source, which contains S1, R1, and S3a. The [completed prerequisite record](#completed-r1-and-s3a) separates their evidence from proposed Compressed behavior. The [earlier branch record](#branch-verification-record) reports capability and Basic exchange checks before S1. Historical source observations in the lifetime and lifecycle plans still require review against each selected parent. Each remaining behavior repair must reproduce its old failure through production code before its implementation changes that path.

[HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) already resolves `blockAckVariant` once into private typed state. Basic remains the default. Unknown values and explicit Compressed selection stop initialization, also when Block Ack support is disabled. S3b/A2 must replace the temporary Compressed rejection before its admission checks can run. C2 supplies the complete exchange path; C4 changes the default.

Management already publishes local, BSS, and peer rates through R1. Its combined MIB operations commit related HT and rate state before notification. C2 must consume these records; A3b must preserve their callback contract. Identical AP reassociation state can produce no rate notification, so association reset requires a separate handoff for both profiles.

`Hcf` creates the Block Ack procedures when `isBlockAckSupported` is true. [TxOpFs](../../src/inet/linklayer/ieee80211/mac/framesequence/TxOpFs.cc) chooses the BAR sequence. [BlockAckReqBlockAckFs](../../src/inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.cc) builds Basic BAR. The Compressed builder in [OriginatorBlockAckProcedure](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckProcedure.cc) throws `Unsupported feature`.

[Hcf](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) dispatches Basic BAR and Basic Block Ack only. [RecipientBlockAckProcedure](../../src/inet/linklayer/ieee80211/mac/blockack/RecipientBlockAckProcedure.cc) builds only Basic responses. [RecipientQosMacDataService](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosMacDataService.cc) also accepts only Basic BAR. S1 removed the unusable Compressed request branch from its reorder service. C2 must add Compressed BAR support through the current recipient owners.

[Ieee80211Frame.msg](../../src/inet/linklayer/ieee80211/mac/Ieee80211Frame.msg) defines both variants. Its Compressed Block Ack class has no fixed chunk length. Its common BAR and Block Ack bases still name reserved control bit B0 as an ACK policy. [Ieee80211MacHeaderSerializer](../../src/inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.cc) can write and read Compressed fields, but it still treats variant type 0 as Basic. [RecipientQosAckPolicy](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosAckPolicy.cc), [SingleProtectionMechanism](../../src/inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.cc), and HCF cost a Basic response length.

[OriginatorQosAckPolicy](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.cc) groups outstanding frames by receiver, without TID. [QosAckHandler](../../src/inet/linklayer/ieee80211/mac/originator/QosAckHandler.cc) can mark other peers' frames as awaiting Block Ack after a BAR. It also has a Compressed bitmap reader. The reader does not, by itself, prove that a received response matches the pending BAR.

[BlockAckRecord](../../src/inet/linklayer/ieee80211/mac/blockack/BlockAckRecord.cc) stores per-fragment reception state. Its getter reports true for an empty map and for absent positions before its earliest retained entry. Neither inference supplies Compressed full-state receipt truth. Each delivery also calls `removeAckStates` through [BlockAckReordering](../../src/inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.cc). That call erases earlier entries independently of scoreboard movement. C2 must separate these Basic operations from Compressed receipt retention.

[ReceiveBuffer](../../src/inet/linklayer/ieee80211/mac/blockackreordering/ReceiveBuffer.cc) now counts occupied sequence positions from its indexed storage. Fragments of one MSDU count once. Before insertion of newer DATA beyond the window, `BlockAckReordering` advances the window and releases older complete units. It discards incomplete units outside the new window. These S1 operations replace the former fragment capacity limit and out-of-window rejection.

[InProgressFrames](../../src/inet/linklayer/ieee80211/mac/queue/InProgressFrames.cc) reports an in-progress frame only when that frame can transmit. [HcfFs](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) and `TxOpFs` ask for that DATA candidate before they test whether a BAR is due. [Hcf](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) also uses this test before it requests channel access after a frame sequence. Thus, outstanding Block Ack frames alone cannot start a BAR. With the default threshold of five, a finite burst of four frames also lacks a BAR trigger.

[BasicMsduAggregationPolicy](../../src/inet/linklayer/ieee80211/mac/aggregation/BasicMsduAggregationPolicy.cc) scans a contiguous prefix from queue index zero. [OriginatorQosMacDataService](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc) calls that policy before it selects one queued frame. It privately owns `QoSSequenceNumberAssignment`. These paths cannot skip a deferred queue head without a change to extraction order.

The same data service calls `fragmentIfNeeded` after sequence assignment without an agreement check. C2 will use the prerequisite's agreement-aware extraction path to check the selected peer and TID before that call. HCF supplies the current agreement state; the data service keeps no duplicate agreement store. This check prevents fragmentation before frame construction, rather than a later ACK-policy change.

[Ieee80211Mib](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.h) already owns local HT capability and peer HT state. Management updates that state on association changes. The Block Ack policies do not use it. The current [802.11n protocol test](../../tests/protocol/wifi/11n/N_CompressedBa.test) has a Compressed name, but it expects a Basic Block Ack. [Ieee80211RecipientNav_1.test](../../tests/module/Ieee80211RecipientNav_1.test) checks response NAV behavior with Basic Block Ack. [Ieee80211HcfMultipleProtection_2.test](../../tests/module/Ieee80211HcfMultipleProtection_2.test) checks response duration with synthetic Basic frames.

The [agreement lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#2-current-source-and-required-evidence) owns the setup, Action, inactivity, and teardown findings. The [lifetime plan](ieee80211-edca-data-lifetime.md#2-current-source-and-standard-support) owns missing lifetime integration and sequence-key collisions.

The Block Ack-enabled single-protection path in HCF calls `InProgressFrames::getPendingFrameFor` even for BAR. That operation can extract and number DATA. BAR duration needs no successor DATA.

[BlockAckReordering](../../src/inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.h) now returns complete units through `ReleasedFrames`, an ordered vector of sequence-number and fragment-group pairs. `RecipientQosMacDataService` preserves that order through reassembly, A-MSDU deaggregation, and delivery. A release across sequence wrap therefore delivers 4095 before 0.

The [DATA reception path](../../src/inet/linklayer/ieee80211/mac/blockackreordering/BlockAckReordering.cc) now releases every consecutive complete unit after DATA fills a gap. The reorder owner creates its record on the first valid related DATA or Basic BAR from the accepted agreement. A Basic BAR before DATA can therefore advance an empty window. The S1 fixture supplies accepted agreements; it does not prove negotiated values or a Compressed exchange.

The [response-rate method](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) accepts only Basic BAR. The originator BAR timeout and recipient response duration both call it. C2 therefore needs Compressed response-rate and duration support before its first production exchange or airtime assessment. C3 cannot supply that prerequisite after C2's required evidence.

The [IEEE 802.11 fillers](../../tests/serializer/lib/fillers/ChunkFillers_ieee80211.cc) already contain Basic and Compressed BAR and Block Ack producers. The Compressed producers set old ACK-policy fields and nonzero reserved values for field-preserving round trips. C1 must adapt those producers to its reserved-bit contract with the serializer change. [Ieee80211ResponseModeSelection_1.test](../../tests/unit/Ieee80211ResponseModeSelection_1.test) constructs Basic BAR and uses a 152-byte response length. It requires an HT Basic response mode, which Basic retention preserves. Compressed needs additional 32-byte non-HT response cases.

## 4. Proposed design

### 4.1 Select the current or historical profile

HCF already resolves `blockAckVariant` once into a private typed profile value during initialization. The values remain `"compressed"` and `"basic"`; Basic is the current default and Compressed is the final default. Preserve rejection of unknown values. A2 replaces the temporary Compressed rejection; C4 changes the default after runtime verification. The resolved value remains immutable throughout each agreement's lifetime. Both endpoints must select the same profile; this plan supports no runtime profile change.

HCF owns local profile selection. Merged S3a keeps its typed profile private and exposes no profile query. A2 must pass that value to agreement setup through the smallest interface its actual consumers require. C2 must pass it to frame exchanges and their dependent procedures. Policies, builders, records, protection, and timeouts must not independently read the parameter or infer the profile from PHY mode. The frame context supplies the expected BAR and response variant without a variant registry or second agreement store.

The serializer independently decodes the actual wire type. It must not consult HCF configuration. Format-specific bitmap and length operations remain necessary, but they consume the decoded frame or resolved exchange context. `isBlockAckSupported` still controls procedure construction. ADDBA gains no variant negotiation field.

Keep the common BAR and Block Ack bases, both derived variants, and the Basic length constant. Retain existing Basic builders and duration methods for their callers. Add Compressed operations where existing interfaces cannot serve both variants. HCF will supply its resolved profile to the existing policies and procedures. New callers must use the actual request variant for response length and rate selection.

Type 0 will produce Basic typed frames for historical compatibility. Type 2 will produce Compressed typed frames. Other unsupported types will follow the invalid-frame path.

- Decode the complete four-bit BAR Type and BA Type fields.
- For Compressed, write zero in reserved control bits.
- For Compressed, ignore received reserved-bit values.
- Preserve the common C++ fields and Basic values that encode Type 0.

The current nine-bit `reserved` field maps to B3–B11. Its two low bits therefore overlap the expanded Type field at B3 and B4. For Basic serialization, those two bits must be zero. Preserve the other Basic field values and existing round trips within that Type 0 contract. Reject a Basic object that would encode another Type before serialization. The [BAR writer](../../src/inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.cc) and Block Ack writer both encode `(getReserved() & 0x1FF) << 3`. C1 must verify this layout with direct octet assertions for both frame types. Do not guess a historical layout from frame length or local profile configuration.

Hypothetical example: a Basic object has `reserved = 1`. The old writer places that value in B3, which a complete four-bit decoder reads as Type 4. C1 rejects that inconsistent Basic object before serialization.

For this Compressed profile, B0 and B5–B11 are reserved. B3 and B4 remain Type bits, so they cannot use the reserved-bit ignore rule. Compressed ACK-policy and `reserved` model fields have no field-preserving wire contract. C1 sets those fields to zero in the existing Compressed round-trip producers. Separate octet checks verify zero writes and ignored received reserved bits. C1 updates both fixture inventories with its serializer change; C4 adds any remaining coverage.

The existing Compressed branches in `QosAckHandler` do not establish a production Compressed exchange.

The MSG generator will regenerate derived C++ files. Do not edit generated files by hand.

Set the Compressed Block Ack chunk length to 28 bytes. Its fields contain 16 MAC-header bytes, 2 control bytes, 2 starting-sequence bytes, and 8 bitmap bytes. `Ieee80211MacTrailer` supplies the 4-byte FCS, so the transmitted MPDU is 32 bytes. Keep one source for the chunk length. Use the selected PHY mode to calculate airtime. Do not add a second duration cache.

The Basic response remains 152 bytes with FCS; Compressed remains 32 bytes with FCS. Protection and timeouts must use the request's actual response variant. The Compressed rate rules below must not replace the retained Basic response contract. A received response must match the pending BAR variant before it can report success.

A profile mismatch cannot silently select Basic or acknowledge DATA through the other variant. ADDBA cannot detect this configuration mismatch. The [lifecycle mismatch contract](ieee80211-blockack-agreement-lifecycle.md#profile-conflict-detection-and-termination) requires HCF to stop the simulation on a valid conflicting BAR for an active agreement. Corrected endpoint profiles require a new run.

The prerequisite plans own eligibility queries, sequence preview, Action outcomes, lifetime operations, and duplicate-state cleanup. The recipient stage below owns ordered delivery. These changes extend beyond the Basic-specific contracts; each stage requires its own caller and compatibility checks.

### Migration diagnostics

After local capability initialization, HCF will log the resolved profile for each interface with Block Ack enabled. If Compressed has no usable local HT support, HCF will emit one initialization warning for that interface. The warning will state the Normal Ack result and the explicit Basic compatibility setting. Basic selection and disabled Block Ack must not produce that warning.

The local warning cannot detect an HT ad hoc station without peer HT records. The agreement policy's existing decision log must identify a refusal caused by absent local or peer HT state. Use that log to expose the fallback reason without warnings for every DATA frame. Normal association startup and temporary peer-state loss must retain their existing protocol behavior.

C4's migration fixture will use a user-style configuration outside the legacy override inventory. Supply a queued successor so the production setup policy actually checks eligibility. It will enable Block Ack and omit `blockAckVariant`. Test legacy `g` and `a` modes, plus HT ad hoc management without peer HT records. Require Normal Ack DATA and no ADDBA, BAR, or Block Ack. Check the resolved-profile message, applicable local warning, and policy refusal reason.

Repeat with explicit Basic at both endpoints and require successful Basic exchanges. Repeat with Block Ack disabled and require no irrelevant warning. Documentation must explain the local warning's limit and the HT ad hoc case. The mode set must not change the selected default.

### 4.2 Consume the agreement prerequisite

The [agreement lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#3-agreement-behavior) supplies DATA eligibility and accepted parameters for each peer and TID. HCF must obtain that state before it selects Compressed Block Ack policy. The Compressed builder consumes the current agreement; it cannot restore an agreement that its owner removed.

Hypothetical example: the lifecycle owner retires the agreement before DATA 101 becomes eligible. The Compressed builder must not use the old agreement to transmit DATA 101.

The Compressed addition retains the prerequisite's DATA eligibility, Action ownership, and recovery results. It does not add a second agreement store or reproduce the lifecycle state machine.

HCF will validate a Compressed BAR before it calls `RecipientQosMacDataService::controlFrameReceived`. This profile requires Fragment Number zero and the supported frame length. An invalid BAR will reach neither the reorder service nor the response procedure. Unknown variants and malformed frames follow the invalid-frame path without protocol recovery actions. A well-formed retained BAR for the other profile is a separate conflict case. For a matching active peer/TID, HCF raises the configuration error before scoreboard, reorder, response, or inactivity updates.

After format validation, HCF will use the absent-agreement path from lifecycle stage A3c. C2 will verify that valid Compressed BAR reaches that path and malformed BAR does not. The lifecycle plan owns DATA disposal, Action ownership, and the resulting teardown state.

### 4.3 Keep BAR selection and response state exact

`OriginatorBlockAckAgreement` will own `WinStartO` and `WinSizeO` under Clause 10.25.6.2. A positive accepted ADDBA Response buffer size will set `WinSizeO` to the smaller of that size and 64. The ADDBA Request starting sequence number will initialize `WinStartO`. The first DATA frame under the accepted agreement must use that request sequence number under Clause 10.25.2.

The [agreement lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#first-data-and-final-transfer) owns expiration before the first DATA transmission and its required teardown.

The existing originator ACK policy will group outstanding Block Ack frames by peer and TID. It will choose the oldest frame that needs an initial BAR within that agreement's transmit window. The comparison will use modulo-4096 arithmetic. The BAR will carry that starting sequence number and no range length.

The policy will apply the threshold separately to each peer and TID. It may defer BAR below that threshold only when the next selected DATA belongs to that agreement and fits its window. A candidate for another agreement or management work will not defer that group's BAR. With no DATA candidate, any nonempty eligible group can request BAR below threshold. Before outside-window DATA, the policy will request BAR for older frames with `BLOCK_ACK_NOT_YET_REQUESTED` status. It will transmit older retry-eligible frames before that outside-window candidate.

If several groups need BAR, the policy will select the group whose oldest outstanding frame entered the in-progress queue first. Peer address and TID will resolve equal-time ties. The policy will derive this order from existing frames without a separate scheduling queue. Hypothetical example: A has four outstanding frames while B supplies continuous DATA in the same access category. Before the next DATA for B, the policy selects BAR for A despite the threshold of five.

HCF will preview the next eligible candidate before it selects a frame sequence. For a DATA candidate, the originator ACK policy will compare its peer, TID, and sequence number with its agreement window. `HcfFs` and `TxOpFs` will select BAR without extracting DATA. They will also handle a null candidate when only BAR work remains. After older positions resolve, `TxOpFs` will extract and use the DATA candidate.

`OriginatorBlockAckAgreement` will advance `WinStartO` past contiguous acknowledged or dropped positions before HCF removes their ACK records. This update includes resolved Normal Ack DATA within the agreement's sequence space. When no unresolved frame remains, it may move `WinStartO` to the next unsent sequence number. Clause 10.25.6.7 permits transmission beyond the old window with that advance. This reuses the existing pending queue and adds no duplicate window state in `QosAckHandler`. A BAR after the final short burst needs no new timer.

After the final exchange, HCF will apply the [lifecycle plan's transfer-completion rule](ieee80211-blockack-agreement-lifecycle.md#first-data-and-final-transfer). The channel-access stage must make that final BAR possible before it enables completion-triggered DELBA.

HCF will treat an eligible BAR as channel-access work for the correct access category. It will request contention after a frame sequence when only such work remains. The frame sequence will select a BAR with no DATA pointer. The internal-collision path will keep BAR work pending without giving DATA retry feedback. The existing EDCA, TXOP, and protection decisions still control when that BAR can transmit.

When HCF transmits BAR, `QosAckHandler` will mark only transmitted frames for that BAR's peer and TID as `WAITING_FOR_BLOCK_ACK`. Those frames must have `BLOCK_ACK_NOT_YET_REQUESTED` status and lie within the active agreement window at or above the BAR number. It will leave every other peer and TID unchanged. It will also leave frames outside that window unchanged. Its current per-frame status map will identify the BAR's waiting frames; it needs no second set.

The frame sequence will build Compressed BAR for the selected peer and TID. Its receive step will match the response type, receiver, transmitter, and TID against the active BAR. Only a matched, format-valid response can reach the success callbacks for recovery, agreement, and ACK-state handlers. The response starting sequence number may differ from the BAR value under Clause 10.25.6.5. HCF will pass the active agreement window to `QosAckHandler` for this response; the handler will keep no window copy.

Before that success path, the receive step will also require Fragment Number zero, exactly 64 bitmap bits, and the supported frame length. The header chunk is 28 bytes; the MPDU includes the 4-byte FCS. These checks apply to deserialized input and directly supplied typed frames. An unsupported or malformed response cannot update the bitmap, refresh inactivity, or report exchange success. It ends the receive step through the existing failed-BAR path, which releases requested DATA for retry. Hypothetical example: type 2 with a dynamic-fragmentation encoding fails even when its peer and TID match.

If teardown or peer cleanup ends that agreement during the BAR exchange, HCF will retain the exchange context until completion. A later response or timeout will not access removed agreement state or recreate it. HCF will resolve the current agreement through its owner before it applies bitmap outcomes. This guard covers an actual event boundary and requires no duplicate window state.

Under Clause 10.25.6.8, `QosAckHandler` will accept positive bits for unacknowledged transmitted frames in that window. This includes covered frames outside the BAR's starting position and frames that did not wait for this BAR. For zero bits, it will keep covered frames unacknowledged. It will make the BAR's covered waiting frames retry eligible. It will not change the acknowledgment state of frames below the response starting sequence number.

It will clear their wait for this BAR so later retry selection can proceed. A response with no overlap will leave all requested frames unacknowledged and eligible for DATA retransmission. A timeout will do the same. This profile selects DATA retransmission rather than repeated BAR-only polling after a failed exchange. The originator agreement window will advance from resolved frame status, not from the response starting sequence alone.

Hypothetical example: the transmit window covers sequence numbers 100 through 163, and BAR starts at 104. A valid matched response starts at 100 and has a positive bit for transmitted DATA 102. The handler acknowledges DATA 102 because the received bitmap and transmit window cover it. The BAR's starting number does not exclude that acknowledgment.

### 4.4 Consume the DATA lifetime prerequisite

The [EDCA lifetime plan](ieee80211-edca-data-lifetime.md#3-proposed-ownership-and-behavior) supplies the existing DATA deadline to the Compressed consumer. A bitmap result or BAR timeout must not restart that deadline. The lifetime plan owns identity, expiration, and safe completion.

Individual Block Ack DATA retransmission will preserve its sequence number and set Retry to one under Clause 9.2.4.1.6. Its exception does not include non-A-MPDU frames. Clause 10.23.2.12.1 alone does not establish an optional Retry bit. Zero bitmap bits and BAR timeout will not become synthetic failures in the Normal Ack retry counters. Existing EDCA recovery still owns channel-access effects; the new lifetime integration supplies the missing disposal limit. Internal collisions retain applicable recovery accounting, and no DATA counter will identify frames by TID and sequence alone across different peers.

Hypothetical example: every Block Ack reports DATA 100 as absent but acknowledges later DATA. Successful responses refresh agreement inactivity, but they cannot extend DATA 100's lifetime. HCF eventually discards 100 and permits window progress.

The Compressed replacement cannot omit this disposal bound while it retains the proposed retransmission behavior.

### 4.5 Build recipient state and select response airtime

`BlockAckRecord` will own the full-state scoreboard window for each accepted agreement. At establishment, it will set `WinStartR` from the ADDBA request. It will set `WinSizeR` to the accepted response buffer size, which this profile caps at 64. It will derive `WinEndR` from those values.

The existing reorder buffer will use that same accepted size. For each valid related DATA frame, the scoreboard will apply Clause 10.25.6.3 before reorder insertion or duplicate rejection. This update applies regardless of that DATA frame's ACK policy.

Retain the Basic record's fragment status. The Compressed response will derive one complete-unit receipt bit per sequence position from the same agreement-owned record. Unfragmented DATA supplies a complete MSDU or A-MSDU in that profile. A partial Basic unit cannot become a positive Compressed bit. A received Compressed BAR will advance the scoreboard under Clause 10.25.6.3, with modulo-4096 comparisons.

Compressed receipt status remains after delivery. Scoreboard movement controls its retirement; reorder delivery must not call the Basic deletion operation for that profile. Preserve the receipt evidence at `WinStartR - 1` for the individual-MPDU delivery condition below, even outside the response window. That evidence cannot set response bits outside `WinStartR` through `WinEndR`. An empty record has no positive bit. Retain the Basic fragment representation and its existing operations.

At C2, HCF will reject fragmented DATA for an active Compressed agreement before scoreboard or reorder processing. Nonzero Fragment Number or More Fragments set identifies that input. HCF will use the invalid-frame path without agreement recovery actions. Such input cannot allocate fragment receipt state, produce a positive bitmap bit, or cause DATA delivery. Basic agreements retain fragment receipt state and reassembly after C2 and C4. Ordinary Normal Ack reassembly outside an agreement retains its existing owner.

The response will use the adjusted `WinStartR` as its starting sequence number. Bits beyond `WinEndR` will be zero.

`BlockAckReordering` owns delivery order and `WinStartB`. S1 makes `WinSizeB` count MSDU or A-MSDU sequence positions. Basic `ReceiveBuffer` retains multiple fragments at one position, which counts once against the supplied agreement size. A fragment that completes that unit remains eligible when its position occupies the last available window slot. For newer DATA beyond `WinEndB`, the reorder owner moves the window, retains the new DATA, and releases eligible older complete units. C2 will reuse these operations for the Compressed profile under Clause 10.25.6.6 and retain Basic fragment behavior.

S1 releases every consecutive complete unit from `WinStartB` when DATA fills a gap. It stops at the next absent or incomplete unit. The reorder owner advances the window after the last released unit without a later DATA reception or BAR. Its selected rule follows Clause 10.25.6.6.2.1(a)(2), with Basic retention as an INET compatibility choice. The S1 fixture buffers complete DATA 101 and 102 behind absent DATA 100. Receipt of DATA 100 releases 100, 101, and 102 in that order in the same operation.

C2 must implement Clause 10.25.6.6.2.1(a)(2)(v) with its surrounding conditions for the supported individual-MPDU path. The received DATA must belong to an HT-immediate agreement without segmentation and reassembly. The station must be non-EDMG.

RIFS means reduced interframe space. The sequence number must lie inside the reorder window and exceed `WinStartB`. The receipt bit at `WinStartR - 1` must be positive. The MPDU must follow the preceding MPDU by SIFS or RIFS, or follow ACK transmission by SIFS.

When these conditions hold, the recipient must apply the clause's sequential delivery and window-update rules despite the gap. If either receipt evidence or the interframe relation is absent, this condition does not permit release. Another applicable release rule can still permit delivery. C2 must not omit this behavior or make it optional merely to avoid event history.

The reorder owner releases eligible units in cyclic order under that clause. It continues sequentially until the next sequence position has no buffered unit. It advances `WinStartB` after the last released unit. The A-MPDU exclusion does not exclude this individual-MPDU condition. The originator retains its current SIFS frame sequences.

HCF supplies the actual interframe relation to the reorder operation. The [receiver](../../src/inet/physicallayer/wireless/common/base/packetlevel/ReceiverBase.cc) supplies reception start and end times through `SignalTimeInd`. HCF receives ACK completion through its current transmission callback. The [mode interface](../../src/inet/physicallayer/wireless/ieee80211/mode/IIeee80211Mode.h) supplies the applicable SIFS and RIFS intervals. Those receive and completion paths retain no previous-MPDU or ACK history for this decision. C2 retains only that required event history in HCF.

### Interframe history and timing design

The individual-MPDU release rule adds behavior to Compressed reception. It can change delivery times and fingerprints when its conditions hold. Basic retains S1's delivery rules. A changed Basic fingerprint requires its own causal evidence; the Compressed release rule does not justify that change.

As part of C2, its owner must derive the history and timing contract against the selected source parent. The existing callbacks identify possible inputs, but they do not establish a complete contract. Record the implementation design and its source and standards evidence:

- Derive the qualifying previous reception or ACK transmission from the complete standard procedure. Do not impose extra same-peer, address, or TID restrictions without a standards basis.
- Specify the minimal stored timestamps and validity fields. Retain values rather than frame pointers beyond their ownership lifetime.
- Define which intervening receptions and transmissions replace or invalidate history, including unrelated traffic and failed reception.
- Clear relevant validity on agreement retirement, peer cleanup, and replacement setup. Establish invalid history at initialization; add no stop/start support claim.
- Define when HCF evaluates the current frame against previous history and when it replaces that history. Preserve the evidence needed by reorder processing.

A per-peer last-frame timestamp alone cannot establish the immediately preceding event. An intervening frame from another peer can break the relation. The production trace must establish how HCF observes such events before the owner selects its storage layout. Do not add a history registry or new timer without a demonstrated need.

Define the receive-side interval from previous reception end to current reception start. Define the ACK-side interval from local ACK transmission end to current reception start. `SignalTimeInd` supplies receive endpoints; the transmission callback supplies local completion. Current-frame delivery time is not its reception start.

For a stationary sender, equal propagation delays can cancel between successive receptions. After local ACK transmission, the next DATA reception can include outward and return propagation delay. Therefore, raw equality to SIFS is not a complete rule for both paths. Map the applicable PHY timing definitions to INET timestamps, including signal extension and propagation effects. Record time resolution, supported coverage assumptions, and the applicable RIFS operation conditions.

An available RIFS duration alone does not establish permission to transmit RIFS-separated frames.

Clauses 10.3.2.3.2 and 10.3.2.3.3 define RIFS and SIFS timing; Clause 10.22.5 defines coverage-class behavior. These clauses constrain the implementation; they do not make timing tolerance a maintainer preference. A proposed receiver-side recognition interval must have a documented derivation from those rules and the model. No inferred interval or arbitrary epsilon is accepted by this plan update.

The exact timestamp mapping, event observation coverage, and stored fields still need source analysis and design. The C2 owner must resolve and review them before the affected implementation is complete. These are engineering obligations within the required behavior, not new policy decisions or grounds to omit the clause. Independent R1 work remains unaffected. Tests must cover nonzero propagation delay, intervals outside the permitted relation, and intervening unrelated frames.

If existing PHY observations cannot establish the required timing relation, the C2 owner must record the specific gap and a concrete example. The owner must propose the smallest correction to the observations or their interpretation. The code reviewer must check that proposal against the standard and the supported PHY model. Missing callback data alone does not establish that the required behavior cannot be modeled.

If the correction requires a model approximation or reduced scope, the maintainer must approve an explicit plan revision before its implementation. The revision must state the changed behavior, evidence, and compliance limits. Approval does not make an approximation equivalent to strict compliance. Until resolution, the affected C2 path remains incomplete and cannot be published. Independent work that neither exposes nor depends on that path may continue.

Local experimental code may support the design investigation. The affected Compressed release path must not enter a published implementation until its design review and direct verification pass. This publication boundary requires no runtime switch to disable an unfinished rule. These provisions add no implementation or publication authorization to this document-only update.

HCF passes the interframe result with the frame. It adds no timer, scheduler, or simulation event for this relation. The agreement-owned scoreboard supplies the bit at `WinStartR - 1` before window cleanup can erase it. The reorder owner consumes that bit with the interframe result. A false receipt bit or absent interframe relation leaves an in-window gap buffered under the ordinary rule. The two conditions remain independent.

Hypothetical example: an accepted agreement starts at 100 with buffer size four. The recipient receives and delivers Normal Ack DATA 100, then sends ACK. Its reorder start becomes 101. In this zero-propagation example, DATA 104 reception starts one SIFS after local ACK transmission ends. The scoreboard start becomes 101. Receipt evidence for 100 satisfies condition (v), so the recipient delivers 104 despite absent positions 101–103. Without that evidence or SIFS relation, it retains 104 until another applicable release condition occurs.

S1 returns released units as an ordered sequence of fragment groups through `ReleasedFrames`. Its traversal uses modulo-4096 order relative to the old reorder window. `RecipientQosMacDataService` preserves that order through defragmentation, A-MSDU deaggregation, and delivery. The receive buffer retains its indexed storage. External callers must consume the supplied order, as the [migration guide](../../doc/src/migration-guide/index.rst) requires. C2 will preserve this contract for Compressed reception, with constituent MSDUs in order within each A-MSDU.

S1 discards incomplete units outside the new window. Its Basic BAR path applies the selected reorder rule from Clause 10.25.6.6.3.1, also before DATA reception. `BlockAckReordering` initializes the record from the accepted agreement before it applies that frame's window update. A valid Basic BAR can advance an empty reorder window, whose new start remains available for later DATA. C2 will add validated Compressed BAR input to this current path.

Hypothetical example: the accepted request starts at 100 with buffer size eight, but the recipient receives no DATA. A valid BAR at 104 creates the reorder record and moves its start to 104. Subsequent DATA 104 is eligible for immediate delivery. Invalid BAR and BAR without an agreement create no reorder record. The reorder window will not depend on the scoreboard's `WinStartR`; the two owners track different state. Do not add another scoreboard store or a duplicate reorder buffer.

This profile will send the Compressed Block Ack response in a non-HT PPDU. `QosRateSelection` will use the BAR rate for a non-HT BAR. It will use the non-HT reference rate for an HT or VHT BAR. It will select the highest qualifying BSS basic rate at or below that value. If none qualifies, it will select the highest qualifying mandatory PHY rate. The MIB supplies rate-set storage, and `IIeee80211Mode` supplies the reference rate.

For a non-HT BAR, the response will use the BAR's modulation class and preamble type. For an HT or VHT BAR, it will use the OFDM or ERP-OFDM modulation class. Rate candidates must satisfy that modulation rule. A configured response-rate override must select the primary rate or a legal alternate under Clause 10.6.6.5.4. An alternate must use the same modulation class, satisfy the basic or mandatory rate rule, and preserve response duration. `QosRateSelection` will reject an invalid override with a clear configuration error before response transmission.

Originated Compressed BAR will use a non-HT PPDU as a fixed INET profile choice. This choice permits legal BAR transmission without new VHT capability negotiation or protected HT/VHT mode selection. `QosRateSelection` will select the highest eligible local legacy mode under the rules below. Other control frames retain their existing mode contract. A valid received HT or VHT BAR remains supported through the response rules above.

- When BAR initiates a TXOP, Clause 10.6.6.2 requires a BSS basic rate. If that set is empty, the selected rate must be mandatory for the attached PHY.
- For a later BAR, Clause 10.6.6.4 requires a rate from the peer's known operational rate set. If that set is unknown, use a BSS basic rate, or a mandatory rate when the basic-rate set is empty.
- Mode candidates must belong to the local legacy operational set. When BSS rate state is unknown, mandatory-rate selection is the documented INET fallback. A configured `controlFrameBitrate` must satisfy the same BAR conditions; otherwise `QosRateSelection` rejects that BAR before transmission.

At the pinned revision, [MIB rate sets](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.h) supply storage and accessors, but no production caller populates them. The [rate-set type](../../src/inet/linklayer/ieee80211/mib/Ieee80211RateSet.h) defaults to unknown state. [Management](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.cc) constructs Supported Rates elements without publication to the MIB. Ordinary association therefore does not supply the rate knowledge required here. Direct MIB injection verifies consumer logic, not production publication.

The separate [R1 stage](#r1-publish-production-rate-state) will populate the existing MIB records from advertised or accepted rate information. R1 owns full and simplified management, Beacon refresh, cleanup, and publication tests. C2 consumes the completed rate contract before its first exchange. It will repeat the subscriber interaction after S3b installs HCF's lifecycle callback.

Rate-only publication must preserve unchanged agreements in either profile. Missing HT state must not retire Basic compatibility. R1 creates no new rate store, parameter, or negotiation mechanism.

The current [TXOP initiator predicate](../../src/inet/linklayer/ieee80211/mac/originator/TxopProcedure.cc) always returns false. [QosRateSelection](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) uses that predicate to distinguish initial and later control frames. C2 cannot reuse that result for Compressed BAR classification. [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) already has the frame-sequence context before mode selection. HCF will derive whether this BAR starts the TXOP from actual frame-sequence progress. `QosRateSelection` will receive that BAR-specific condition through the mode-selection call path.

A BAR after fresh channel access will use the initial-BAR rule. A BAR after an earlier transmission in the same TXOP will use the later-BAR rule. This change preserves other control-frame decisions and adds no retained classification flag. The mode catalog supplies eligible local modes and mandatory-rate status.

Hypothetical example: BSS basic rates are 6, 12, and 24 Mbps, while the peer also supports 54 Mbps. The initial Compressed BAR uses the eligible 24 Mbps non-HT mode. A later BAR can use the eligible 54 Mbps mode. Production checks must establish both rate publication and BAR position before they compare these selected modes.

When no basic rate qualifies, Clause 10.6.6.5.2 supplies the applicable mandatory-rate rule. If the BSS basic-rate set is unknown, mandatory-rate selection is an explicit INET fallback. The response Duration/ID will use Clause 9.2.5.6 and actual Compressed airtime. The single and multiple protection paths will use the new frame length without new TXOP admission policy.

R1 supplies committed production rates. C2 supplies BAR classification, response-rate rules, override checks, and Compressed airtime calculations before its first exchange. Its BAR timeout, recipient Duration/ID, and protection paths will all use the supported Compressed response mode and MPDU length. C2 will record the selected mode with its first timing result. C3 will add focused verification across response modes and exchange boundaries; it supplies no missing runtime operation required by C2.

For BAR, HCF will bypass `getPendingFrameFor` and use the BAR duration operation in `SingleProtectionMechanism`. It needs only the selected BAR, SIFS, and that variant's actual response airtime. It will receive no successor DATA and perform no extraction. Other duration paths retain their existing scope. Hypothetical example: peer A needs BAR while peer B has queued DATA. Both BAR selection and its duration calculation leave B's queue ownership and sequence counter unchanged.

## 5. Separate changes and dependencies

### Maintainer decisions

The maintainer accepted all five decisions on 14 October 2026. Their decision conditions are satisfied; source reviews, stage prerequisites, and verification remain required. S1 recipient repair and R1 rate publication retain independent review and publication boundaries.

| Decision | Status and recorded result | Dependent boundary |
| --- | --- | --- |
| Lifetime compatibility | The maintainer accepted the 0.512-second default on 14 October 2026. It applies to QoS DATA with Block Ack enabled or disabled. The [lifetime record](ieee80211-edca-data-lifetime.md#lifetime-compatibility-decision) states the compatibility effect and required measurements. | The decision condition for S2/L1 is satisfied. |
| Response timeout | The maintainer accepted a positive, finite 100 ms default on 14 October 2026. The [lifecycle contract](ieee80211-blockack-agreement-lifecycle.md#response-timeout-and-data-lifetime) defines its start, recovery, and DATA-lifetime interaction. | The decision condition for A4 is satisfied. |
| Transfer completion | The maintainer accepted the [queue predicate](ieee80211-blockack-agreement-lifecycle.md#transfer-completion-decision) on 14 October 2026. At an exchange boundary, the agreement has no queued DATA, in-progress DATA, or pending BAR exchange. Conditional DELBA remains required. | The decision condition for A5 in S4 is satisfied. |
| Individual-DATA purpose | The maintainer accepted the [intermediate profile](#purpose-of-the-individual-data-profile) on 14 October 2026. Its purpose is bitmap, response, and recovery studies with individual DATA before explicit BAR. Complete HT-immediate transfer remains outside that scope. | The scope decision before C2 is satisfied. |
| Persistent profile conflict | The maintainer accepted a fatal configuration error on 14 October 2026. The [conflict contract](ieee80211-blockack-agreement-lifecycle.md#profile-conflict-detection-and-termination) requires termination before state updates and a new run with corrected profiles. | The decision condition for C2 mismatch implementation is satisfied. |

### Stage conditions

The table separates conditions for stage start, continuation, completion, and publication. Each condition blocks only its dependent work. Review of this umbrella plan does not establish either prerequisite plan's correctness or completed verification.

| Boundary | Responsible owner and required condition |
| --- | --- |
| Before each stage starts. | The stage owner pins the source parent and comparison base under the source-base rules below. |
| Before S2 implementation. | The lifetime reviewer completes the separate [lifetime review](ieee80211-edca-data-lifetime.md). The accepted default is 0.512 seconds; its compatibility decision is satisfied. Before the production change, the implementation owner reproduces absent queue expiration through HCF. |
| Before S2 completes. | The lifetime owner supplies [measured expiry under controlled congestion](ieee80211-edca-data-lifetime.md#5-verification-and-evidence), including a Block Ack-disabled case. A selected regression row without expiry cannot satisfy this condition. |
| S3a preparation. | S3a is merged. Preserve its recorded parent comparison and explicit Basic consumers. Repeat affected checks after shared changes. |
| Before S3b implementation, including A1. | The lifecycle reviewer completes the separate [lifecycle review](ieee80211-blockack-agreement-lifecycle.md). Select a parent that contains merged S3a and R1. Refresh selector, MIB commit, reset, and callback paths against that parent. |
| Before A3b implementation. | Refresh the [consumer assessment](ieee80211-blockack-agreement-lifecycle.md#reassociation-consumers-and-callback-order). Trace Compressed HT removal and replacement. Trace Basic association-reset notifications with the actual peer key. |
| Before A4 implementation. | The lifecycle review checks the [accepted 100 ms timeout](ieee80211-blockack-agreement-lifecycle.md#response-timeout-and-data-lifetime) against the selected source parent. The implementation owner completes S2, A1, A2, and A3a through A3c. |
| Before S4 starts. | The S4 owner confirms that S3b supplies the required teardown behavior and S1 supplies recipient progress. A5 uses the accepted transfer-completion predicate; its decision condition is satisfied. |
| Before S4 completes or its affected Basic path is published. | The S4 owner compares changed BAR timing with S4's pinned parent. Resolve any new unlawful overrun under [Section 6](#airtime-acceptance-and-admission-dependencies) before that publication. This condition requires no future C2 or C3 result. |
| Before S5 starts. | The implementation owners complete the prerequisite contracts from S1 through S4. C2 evidence is produced within S5, so it is not a start condition. |
| R1 publication prerequisite. | R1 is merged. Its [evidence record](../../doc/project/evidence/model/wifi/r1-rate-publication.md) owns the parent comparisons. C2 must repeat notification checks with S3b's subscriber. |
| Before C2 implementation. | C2 uses the accepted individual-DATA purpose and scope. Complete HT-immediate transfer requires a separate A-MPDU design. Other C2 prerequisites retain their stated boundaries. The accepted fatal profile-conflict policy applies. |
| Before C2 individual-MPDU release is complete. | Implement the required clause. Resolve and review its [history and timing design](#interframe-history-and-timing-design) against the actual parent. Verify each condition through production reception without extra unsupported restrictions. |
| Before the C2 individual-MPDU release path is published. | The code reviewer accepts the history and timing design, and direct verification passes. Resolve any observation gap through the [escalation rule](#interframe-history-and-timing-design). A model approximation or reduced scope requires an explicit maintainer-approved plan revision. |
| Before C2 mismatch validation implementation. | Apply the accepted [fatal conflict policy](ieee80211-blockack-agreement-lifecycle.md#profile-conflict-detection-and-termination). Trace production BAR validation before state updates. Require exact diagnostic and compatible-run controls before publication. |
| Before C2's first production exchange or timing assessment. | R1 supplies committed local, BSS, and peer rates. Repeat publication callbacks with S3b's subscriber. HCF supplies actual BAR position. C2 supplies legal modes, timeouts, durations, and protection airtime. |
| Before C4 completes. | The migration fixture proves the default, warning limits, capability-gap fallback, and explicit Basic recovery for external-style configurations. |
| Before C3 or C4 proceeds. | The C2 owner records the first airtime result with its selected response mode and resulting admission decision under [Section 6](#6-verification-and-scope-limits). |
| Before S5 completes or the affected Compressed path is published. | The Compressed owner satisfies Section 6's final airtime conditions, including C3's checks for response modes and any required admission correction. |

C2 belongs to S5. A new overrun outside Clause 10.23.2.9's permitted cases can require an admission correction before publication of the affected path. A permitted overrun requires its condition-specific evidence, not an admission correction. C3 repeats the assessment across supported response modes and exchange boundaries. The [airtime decision](#6-verification-and-scope-limits) controls continuation and completion; it does not require C2 results before S5 starts.

### Source base and publication boundaries

Local master at `633b071e213b825ce6019ff3e344ae49f52791ff` contains S1, R1, S3a, and the VHT/HT prerequisite from PR #1318. S1 merged through PR #1320 on 14 October 2026. Its original feature-series publication base is `ac6344cc5bcac972012759281a11b3f8255b5d46`. Original feature heads and comparison bases remain historical verification boundaries. Each remaining stage must pin its actual source parent and comparison base from the merged integration source.

The lifetime and lifecycle plans retain their separately pinned `b06e8768` source observations. Before each prerequisite starts, its owner must refresh its source observations and caller assessment against the selected stage parent. This umbrella review does not replace either prerequisite's review. The decision table records accepted policy choices; stage reviews and verification remain required.

S2 and S3b must refresh their prerequisite source observations against their actual selected parents. Those parents must include merged R1 and S3a; the refresh must not remain fixed to `1cdb362a`. Existing local S2 implementation does not establish merged S2 status or authorize a history change.

A move to another base requires a fresh caller and test assessment. Shared paths identify overlap; they do not by themselves prove a merge conflict. This plan does not authorize branch mutation or publication.

Each stage below has its own direct tests and regression evidence. Basic support remains available in every stage and in the final implementation. No prerequisite change claims completed Compressed support.

The [stage conditions](#stage-conditions) define the remaining design, implementation, and verification obligations.

### Implementation stages

| Stage and purpose | Responsible components and change | Result and direct verification |
| --- | --- | --- |
| S1. Merged in PR #1320. | `ReceiveBuffer`, `BlockAckReordering`, and `RecipientQosMacDataService` count sequence positions and release complete groups in cyclic order. Valid Basic BAR creates a record before DATA. | Original-head verification passes eight recipient scenarios, three NAV runs, and 12 selected fingerprints. The [S1 record](#completed-s1-and-recorded-verification) states the supplied-agreement scope and approved baseline changes. |
| S2. Establish DATA lifetime. | Implement the [separate EDCA lifetime plan](ieee80211-edca-data-lifetime.md). Its owner supplies deadlines independently of Block Ack support. | Normal Ack DATA expires under either switch value. Identity transfer and accepted-exchange completion preserve ownership. |
| S3a. Merged. | HCF validates the selector with Basic as the temporary default. Legacy demonstrations and twelve existing fixtures select Basic explicitly. | The [S3a record](../done/ieee80211-compressed-blockack-s3a.md) reports direct Basic exchange and capability comparisons. Compressed selection still stops initialization. |
| S3b. Repair agreement lifecycle. | Implement A1, A2, A3a through A3c, and A4 of the [separate lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#5-reviewable-stages-and-dependencies). A2 distinguishes Basic compatibility from the Compressed HT requirement. Retain Basic fixtures and add focused Compressed admission checks. | Legacy ad hoc and non-HT consumers retain Basic. Compressed admission requires valid HT state; its capability-gap cases use Normal Ack. Shared lifecycle tests cover both profiles. |
| S4. Complete BAR work independently of DATA selection. | The originator policy, ACK handler, agreement window, HCF, and frame sequences isolate peer/TID state. They permit BAR-only contention without DATA extraction. Connect lifecycle stage A5. | One or four outstanding frames cause BAR below the default threshold of five. Another peer cannot suppress this work. Final completion causes DELBA. Compare changed Basic BAR timing with S4's parent before publication. |
| R1. Merged. | Full and simplified management commit local, BSS, and peer rates through the MIB. Combined operations commit related state before notification. | The [R1 record](../../doc/project/evidence/model/wifi/r1-rate-publication.md) reports publication, refresh, cleanup, and malformed-input checks. C2 later consumes these records and repeats subscriber checks. |
| S5. Add Compressed and activate its default. | Frame definitions, serializer, procedures, recipient scoreboard, response matching, rate selection, and protection support both profiles. C4 changes the default only after C2 and C3 pass. | Compressed exchanges pass direct bitmap, rate, and timing checks. Explicit Basic still supports type 0, fragment bitmaps, existing consumers, and its response contract. |

S1, R1, and S3a are complete in the integration source. The remaining stages are S2, S3b, S4, and S5. A1, A2, and A3a through A3c can proceed without S2; A4 requires S2. Completed stages retain their separate parent comparisons. R1 satisfies the rate-publication prerequisite, but C2 still requires subscriber integration and legal rate consumption.

S1's direct tests supply an agreement with a known buffer size through the existing Basic path. S3b later verifies that negotiation supplies the correct size to that repaired recipient. Lifecycle stage A4 depends on S2; A1, A2, and A3a through A3c need not wait for S2. S4 depends on S3b teardown and S1 recipient progress.

The lifecycle plan owns the [A3 review split and publication conditions](ieee80211-blockack-agreement-lifecycle.md#5-reviewable-stages-and-dependencies). A3a covers directional teardown and inactivity; A3b covers association reset; A3c completes Action disposal and absent-agreement recovery. Required Action invalidation and safe completion accompany the first retirement or reset path that uses them. The split does not authorize an unsafe intermediate publication.

S3a already precedes S3b in the integration source. S3b must use a parent that contains both S3a and R1. Preserve S3a's original parent comparison. Attribute later lifecycle effects to S3b. The final Compressed default takes effect at C4, after the runtime path and timing checks exist.

S5 depends on the completed prerequisite contracts. C1 changes wire definitions without a new rate-publication stage. C2 consumes merged R1 and repeats its interaction with S3b. S1's and R1's recorded parent comparisons remain separate from S4's future comparison. Unresolved C2 or C4 work does not reopen completed stages.

### Completed R1 and S3a

R1 spans commits `c1973f443e` through `b385562353`. It adds combined MIB commits, production publication, membership-selector checks, and nonfinite-rate rejection. The [R1 evidence record](../../doc/project/evidence/model/wifi/r1-rate-publication.md) preserves its original source boundaries, commands, failures, and corrected results. Its final broad verification reports 46 runs; a later focused correction adds nonfinite-input checks. These recorded results do not establish a new combined-master pass.

S3a spans `4e6aa75c99` and `633b071e21`. Its [execution record](../done/ieee80211-compressed-blockack-s3a.md) reports thirteen demonstration comparisons, eight capability probes, and nineteen fingerprint rows. Every calculated fingerprint ingredient equals its parent. Fifteen rows retain graphical `tyf` baseline mismatches, so the fingerprint runner exits 1. Four protocol fixtures retain expected FAIL results with unchanged causes. S3a changes no recorded baseline and proves no Compressed exchange.

The inventories below remain preservation requirements for later stages. They describe the original S3a consumer set, not every fixture now present. Preserve the new selector, Basic observation, and R1 fixtures as additional controls. The original evidence belongs to its recorded feature source; this plan review runs no simulations.

S1 changes recipient delivery, not the Compressed bitmap representation. S4 preserves Basic fragment rules; S5 retains those rules beside the Compressed restriction. Neither stage may claim a Compressed exchange solely because a helper handles a Compressed class.

### S3a: demonstration migration and fallback inventory

Merged S3a preserves the existing Basic demonstrations through explicit profile selection. The [Block Ack network](../../showcases/wireless/blockack/BlockAckShowcase.ned) and [TXOP network](../../showcases/wireless/txop/TxopShowcase.ned) retain their two ad hoc hosts. Later stages must retain this configuration scope. The plan requires no AP, Ethernet link, channel migration, or new association interval in either showcase.

The current ad hoc management does not install peer HT records. That limitation prevents the Compressed HT profile, but it will not disable explicit Basic compatibility. The Basic profile preserves INET's existing capability assumptions. Its use with later PHY modes does not establish complete IEEE Std 802.11e-2005 conformance.

Preserve S3a's explicit Basic assignment in each legacy configuration's existing scope:

```ini
*.host*.wlan[*].mac.hcf.blockAckVariant = "basic"
```

Use the actual node patterns below for networks without `host*` names. HCF already validates the selector. Its temporary default remains Basic until C4 activates the completed Compressed implementation. A2 must revise the temporary rejection of Compressed, including the selector fixture's expected diagnostic. A2 must prevent an admitted setup from reaching the unsupported builder before C2 supplies that path.

| Configuration and source | Retained behavior | Basic selection to preserve |
| --- | --- | --- |
| Block Ack [General](../../showcases/wireless/blockack/omnetpp.ini), `NoFragmentation`, `MixedTraffic`, and `Fragmentation` | Existing ad hoc topology, modes, radio settings, traffic, and fragment demonstration. | Select Basic on both hosts in General. All named configurations inherit it. |
| TXOP [General](../../showcases/wireless/txop/omnetpp.ini) | Existing ad hoc topology and `n(mixed-2.4Ghz)`; mixed Block Ack, Normal Ack, A-MSDU, and RTS/CTS exchanges. | Select Basic on both hosts. Retain thresholds and application settings. |
| Wireless QoS [MacQosWithBlockAck](../../examples/wireless/qos/omnetpp.ini) | Existing AP, two stations, two wireless hops, and inherited `g(mixed)`. | Select Basic on all participating interfaces in this configuration only. Do not change other configurations. |
| Ad hoc QoS [MacQos](../../examples/adhoc/qos/omnetpp.ini), runs 0 and 1 | Existing topology, `g(mixed)`, and support parameter loop. Run 1 retains Basic; run 0 remains support-disabled. | Select Basic on both hosts without a new parameter loop. |
| Fragmentation [HCFfragblockack](../../showcases/wireless/fragmentation/omnetpp.ini) | Existing Basic fragment exchange and comparison with the other fragmentation cases. | Select Basic for the participating interfaces in this configuration. |
| Fragmentation [parameterstudy.ini](../../showcases/wireless/fragmentation/parameterstudy.ini) | All four Block Ack variants and their existing parameter sweeps. | Select Basic within the shared Block Ack configuration scope or each named variant, according to inheritance. |

Retain `PacketSizeHCFfragblockack`, `TxPowerHCFfragblockack`, `ThresholdHCFfragblockack`, and `OnlyPacketSizeHCFfragblockack`. Retain their names, result filters, analysis inputs, plot series, fingerprint rows, and speed records.

S3a's completed migration covers six INI files and the ad hoc `${false, true}` parameter loop. Check effective INI inheritance before a later change to these assignments. Search tracked `tests/**/*.test` sources and shared includes separately. Generated files under `work/` do not replace those sources.

For each legacy demonstration, require accepted ADDBA, Block Ack-policy DATA, Basic BAR, and a Basic response with positive receipt status. Check fragment status in the fragmentation cases. Record peer addresses and TIDs for every measured wireless hop. No peer HT record is required for Basic eligibility.

Repeat those assertions after A2 and at final S5. A fingerprint match without a Basic exchange cannot establish preservation. Check the support-disabled ad hoc run separately. Add a focused Compressed ad hoc case with absent peer HT state. That case requires Normal Ack without ADDBA, BAR, or Block Ack.

Preserve S3a's historical Basic explanation in the [Block Ack page](../../showcases/wireless/blockack/doc/index.rst), [TXOP page](../../showcases/wireless/txop/doc/index.rst), and [fragmentation page](../../showcases/wireless/fragmentation/doc/index.rst). State that Basic originated in IEEE Std 802.11e-2005 and is unsupported by IEEE Std 802.11-2024. Link to Section 2's historical and current-standard sources. Retain this distinction in affected QoS example descriptions and the parameter-study explanation.

Preserve existing images and plots when the evidence still supports them. Shared recipient, lifetime, and lifecycle repairs can change results despite unchanged topology and traffic. Regenerate only affected evidence after a measured change. Keep prior measurements as historical evidence; do not claim unchanged fingerprints or current performance without verification.

Existing infrastructure protocol fixtures will supply Compressed coverage through actual AP/STA capability exchange. This plan needs no showcase topology conversion or new IBSS capability implementation.

### S3a: tracked fixture inventory and VHT scope

S3a's recorded inventory covers tracked INI files, conditional assignments, included configuration files, and direct Basic consumers. Its original migration set contains twelve fixtures: eight protocol fixtures and four module fixtures. Five further module fixtures explicitly disable support. The separate S1 recipient fixture supplies accepted Basic agreements without an HCF enablement assignment. The response-mode unit fixture and serializer fixture also consume Basic types without that assignment. Later verification must include S3a's new selector and Basic observation fixtures in addition to this original set.

The existing protocol fixtures include [_n.ini](../../tests/protocol/wifi/ini/_n.ini) or [_ac.ini](../../tests/protocol/wifi/ini/_ac.ini), followed by shared [_base.ini](../../tests/protocol/wifi/ini/_base.ini). Their [infrastructure network](../../tests/protocol/wifi/ned/WifiInfraNetwork.ned) uses full AP/STA management. The HT configuration selects `n(mixed-2.4Ghz)`; the VHT configuration selects `ac`. Association alone therefore does not prove that either endpoint has usable HT state.

The [mode catalog](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.cc) appends 128 existing HT combinations to `ac` and enables its HT-support flag. These HT modes use 5 GHz timing. [Ieee80211Mib](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.cc) derives local capabilities from that catalog and operational limits. The [HT mode-set fixture](../../tests/unit/Ieee80211HtModeSet_1.test) requires `ac` to report HT operation. The [VHT/HT production fixture](../../tests/module/Ieee80211VhtHtSupport_1.test) checks peer HT state after association. Clause 4.3.15 defines a VHT STA as an HT STA with additional VHT features.

The [S3a record](../done/ieee80211-compressed-blockack-s3a.md) records actual modes, local HT flags, peer HT state, and pre-A2 outcomes. S3b/A2 must retain Basic compatibility and check Compressed admission with real HT state. Before C2, admission fixtures do not claim a production Compressed exchange. C2 and C4 must retain the original `Ac_BlockAck` requirement with matched Compressed exchanges and positive receipt bits. Earlier generic frame-type passes establish neither negotiated-parameter correctness nor final Compressed support.

This revision removes the proposed `Ac_BlockAck` deferral and its approval gate because their missing-HT premise no longer applies. The implementation must preserve `_ac.ini` and real capability exchange rather than replace VHT fixtures with HT-only configurations. If a later result exposes a different missing prerequisite, record its actual cause before any expectation proposal. The [recorded-expectation rule](../../doc/project/rule/testing.md#tr-baseline-deliberate) still governs such a proposal.

The proposed `tests/protocol/wifi/11ac/Ac_BlockAckFallback.test` will supply an explicit absent-peer-state condition. Full and simplified management both install peer HT state on this branch. Under full management, a later valid [Beacon](../../src/inet/linklayer/ieee80211/mgmt/Ieee80211MgmtSta.cc) can restore the STA's AP record. One-time removal therefore does not maintain the required gap throughout traffic. The fallback fixture will select simplified AP/STA management, which supplies static association without periodic Beacon refresh. Existing `Ac_BlockAck` will retain full management and real capability exchange.

Use these fixture-local overrides with the existing `_ac.ini` and `_base.ini` includes:

```ini
*.ap.wlan[*].mgmt.typename = "Ieee80211MgmtApSimplified"
*.sta*.wlan[*].mgmt.typename = "Ieee80211MgmtStaSimplified"
*.sta*.wlan[*].agent.typename = ""
*.sta*.wlan[*].mgmt.accessPointAddress = "ap"
```

After all initialization stages, the fixture will remove peer HT records before the first DATA through `removePeerHtCapabilities`. It will remove the records at both ends of each measured wireless hop. Keep association active throughout measured traffic. Keep `opMode = "ac"`, local HT support, and Block Ack support enabled.

Select the Compressed profile explicitly in the fallback fixture. Schedule no management restart, reassociation, or fixture action that reinstalls capabilities. Repeat this fallback check at A2, C2, and C4.

The fallback assertions require delivered Normal Ack QoS DATA, absent peer HT state, and no ADDBA, BAR, or Block Ack during the measured interval. Check active association and valid local and peer HT state before the explicit removal. Check absent peer HT state at both ends during traffic and at the end of the measured interval. The fixture must start no agreement before removal. The result establishes fallback under a controlled capability gap with static association. It does not establish an on-air association exchange or a limitation of every `ac` station.

Hypothetical example: full `ac` management installs valid peer HT state and A2 permits agreement setup. The fallback fixture first establishes static association through simplified management. It removes peer HT state after initialization, before DATA. No Beacon refresh restores that state, so A2 uses Normal Ack throughout the measured traffic. The positive and fallback fixtures must preserve those distinct initial conditions and results.

The table assigns fixture changes to the stage that changes their contract. S3a recorded the original outcomes; those results do not establish future assertions. S3b's lifecycle owner owns capability and agreement adaptation. C2's production owner owns Compressed exchange assertions. C4's consumer owner owns default activation, profile-specific configuration validation, and coverage of both serializers.

| Tracked fixture | Current assertion or dependency | Decision and stage |
| --- | --- | --- |
| [WifiQosDelayedBa](../../tests/protocol/wifi/common/WifiQosDelayedBa.test) | Delayed negotiation and response order; an existing deferred failure. | Select Basic explicitly and retain the deferred requirement. Add a separate Compressed initialization-rejection fixture at C4. |
| [Ac_MultiTidBa](../../tests/protocol/wifi/11ac/Ac_MultiTidBa.test) | Two TIDs and Multi-TID response; an existing deferred failure. | Retain its obligation and real `ac` management context. Record the first unmet requirement at each affected stage. |
| [N_Ampdu](../../tests/protocol/wifi/11n/N_Ampdu.test) and [Ac_Ampdu](../../tests/protocol/wifi/11ac/Ac_Ampdu.test) | A-MPDU subframe headers; two existing deferred failures. | Retain the aggregation assertions and HT/VHT capability context. Compressed support does not establish A-MPDU support. |
| [N_BlockAck](../../tests/protocol/wifi/11n/N_BlockAck.test) | Generic Action, BAR, and Block Ack frame types. | Before C2, use explicit Basic. C2 adds accepted ADDBA, matched Compressed frames, and positive bits. C4 checks the default. |
| [N_Amsdu](../../tests/protocol/wifi/11n/N_Amsdu.test) | `aMsduPresent`, without proof of accepted agreement. | Preserve that assertion. Add accepted-agreement permission checks at A2 and repeat with Compressed at C2. |
| [Ac_BlockAck](../../tests/protocol/wifi/11ac/Ac_BlockAck.test) | Generic Block Ack exchange under `ac`, currently passing. | Preserve positive Basic coverage before C2. C2 adds matched Compressed exchanges through real peer HT state; C4 checks the default. |
| [N_CompressedBa](../../tests/protocol/wifi/11n/N_CompressedBa.test) | A Basic class and positive Basic bitmap bit despite its name. | Keep its pre-C2 Basic reproduction. C2 changes this named Compressed fixture to actual Compressed frames and positive receipt assertions. Retain Basic coverage elsewhere. |
| [Ieee80211HcfHeader_1](../../tests/module/Ieee80211HcfHeader_1.test) | Ad hoc mode `a`, custom policy, and synthetic agreement. | Select Basic explicitly. Adapt initialized handlers for shared lifecycle changes without invented HT peer state. Preserve DATA/RTS and policy assertions. |
| [Ieee80211SingleProtectionGroup_1](../../tests/module/Ieee80211SingleProtectionGroup_1.test) | Group duration and no ACK-mode query, with support false and true. | Retain both controls. Explicit Basic preserves the existing support-enabled context; no successful agreement is claimed. |
| [Ieee80211RecipientNav_1](../../tests/module/Ieee80211RecipientNav_1.test) | Injected agreement and Basic BAR/response in ad hoc mode `a`. | Keep Basic types and explicit Basic selection. Add a separate Compressed case with valid HT state. Preserve NAV assertions in both cases. |
| [Ieee80211BlockAckReordering_1](../../tests/module/Ieee80211BlockAckReordering_1.test) | Eight S1 scenarios through the recipient service with supplied Basic agreements. No HCF profile parameter is present. | Retain all eight Basic scenarios through later stages. A2 adds negotiation checks through HCF. C2 and C4 add Compressed integration without removal of Basic assertions. |
| [Ieee80211HcfMultipleProtection_2](../../tests/module/Ieee80211HcfMultipleProtection_2.test) | Synthetic Basic frames and Basic duration calculations. | Keep explicit Basic cases and their duration assertions. Add Compressed duration cases at C2/C3; do not remove Basic coverage. |
| [Ieee80211ResponseModeSelection_1](../../tests/unit/Ieee80211ResponseModeSelection_1.test) | Basic BAR, 152-byte response, and current HT Basic response behavior. | Retain Basic assertions as model compatibility evidence. Add 32-byte non-HT Compressed cases, roles, peer isolation, and asymmetric state at C2/C3. Basic assertions do not establish current-standard conformance. |
| [Ieee80211VhtHtCapabilities_1](../../tests/unit/Ieee80211VhtHtCapabilities_1.test) | Local HT capability assembly from `ac`. | Retain positive capability assertions. Production Compressed fixtures must separately prove peer state and complete exchanges. |
| [serializer_chunk_roundtrip](../../tests/serializer/serializer_chunk_roundtrip.test) and [IEEE 802.11 fillers](../../tests/serializer/lib/fillers/ChunkFillers_ieee80211.cc) | Existing Basic and Compressed BAR and Block Ack producers, with field inventories. | At C1, retain Type 0 Basic round trips and adapt Compressed reserved-field expectations. Verify complete Type decoding and ignored received reserved bits. C4 adds remaining coverage without removal of Basic producers. |

S3b will adapt synthetic agreements through initialized handlers. Explicit Basic cases retain their existing capability assumptions, frame classes, and rates. Supply matching recipient agreement state when absent-agreement recovery requires it. Do not weaken NAV, policy, duration, or protection assertions to retain a pass.

Before C2, modern-profile admission tests can exercise agreement policies without a Compressed DATA/BAR exchange. Capability-gap tests must refuse setup before the unsupported builder can run. Positive admission tests must isolate setup; C2 supplies the first complete production Compressed exchange.

Five module fixtures explicitly disable support: `Ieee80211HcfMultipleProtection_1`, `Ieee80211HcfSingleProtection_1`, `Ieee80211PreparedCancellation_1`, `Ieee80211Txnav_1`, and `Ieee80211VhtHtSupport_1`. Retain these controls. They need no Basic migration or active agreement. Keep HT mode-set and VHT/HT capability unit coverage.

### R1: publish production rate state

R1 is merged and supplies production publication of local, BSS, and peer rates. The [R1 evidence record](../../doc/project/evidence/model/wifi/r1-rate-publication.md) owns its implementation contract and parent comparisons. C2 owns Compressed rate consumption. The requirements below remain regression obligations when later stages change these paths.

Use the existing [MIB](../../src/inet/linklayer/ieee80211/mib/Ieee80211Mib.h) stores and validation. The responsible management paths are:

- `Ieee80211MgmtBase` publishes local legacy operational rates after a mode-set update through `setLocalRateSet`.
- Full AP management publishes BSS rates from its advertised elements through `setBssRateSet`.
- Full AP management installs peer rates from an accepted request when the successful association response exchange commits.
- Full STA management commits BSS and AP rates from accepted advertisements when association commits.
- Full STA management refreshes those records from valid associated-AP Beacons.
- Simplified AP and STA management publish equivalent rates during their existing static association operation.

Use the merged combined MIB operations when related state commits together. `installBssAndPeerRateSets` commits rate records; `installBssAndPeerState` also commits peer HT state before notification. Preserve unknown state and a known empty basic-rate set as distinct values. Scan candidates and unsuccessful association attempts must not replace active rate state.

AP departure removes that peer's rates without loss of BSS rates. STA association loss clears its old BSS and peer records. Existing management stop paths clear their corresponding records. R1 adds no stop/start recovery guarantee.

Retain [Ieee80211MgmtRatePublication_1.test](../../tests/module/Ieee80211MgmtRatePublication_1.test) and [Ieee80211MgmtStateCommit_1.test](../../tests/module/Ieee80211MgmtStateCommit_1.test). These existing fixtures drive real management without direct rate injection. Compare published rates and basic flags with actual Supported Rates and Extended Supported Rates elements after affected changes.

The fixture must cover these conditions:

- Full AP/STA association, with observations at both endpoints.
- Simplified management publication, with equivalent static records.
- Valid associated-AP Beacon refresh and unrelated scan advertisements.
- Unsuccessful association without replacement of active records.
- Peer departure, association loss, and existing management stop paths.
- Unknown state and known empty basic-rate state.

Retain focused Basic DATA, BAR, response-mode, and support-disabled controls. Current rate selectors do not consume these records; later rate consumers must preserve the retained Basic selection contract. Compare each later changed result with the responsible stage's pinned parent. Preserve R1's original comparison separately and record each new result's cause.

R1 tests the MIB's notification contract without S3b's future subscriber. C2 must repeat that integration after both changes exist. A3b must use the merged combined-commit behavior instead of an assumed AP remove/reinstall notification pair. A rate-only update must preserve unchanged Basic and Compressed agreements. The built-in rate-control path must produce no additional reset or rate statistic solely from these notifications. External listeners retain their documented notification contract.

C2 cannot replace R1 evidence with helper injection or a successful Compressed exchange. Preserve R1's recorded commands, mode, fixture selectors, seed, parent, and outcomes. Record later subscriber integration against its actual combined source.

### Compressed replacement steps

| Step and purpose | Owner and proposed change | Expected result and verification |
| --- | --- | --- |
| C1. Define valid type 2 frames beside Basic. | Add the Compressed builder and fixed 28-byte header chunk. Decode the complete Type field. Retain Basic classes, fields, builders, and producers within the Type 0 contract. Adapt existing Compressed round-trip producers and expected fields in this step. | Golden octets and round trips verify Type 0 and Type 2. Reject Basic values that encode another Type. Compressed reserved bits write zero and received values are ignored. Basic remains 152 bytes with FCS; Compressed remains 32 bytes. |
| C2. Connect complete Compressed exchanges. | Consume R1's committed rates. HCF derives BAR position and the recipient's actual interframe relation. Procedures and ACK handling select the configured profile and match the response variant. The record retains Compressed receipt truth after delivery and supplies the bit at `WinStartR - 1`. Add the individual-MPDU release condition without loss of Basic fragment behavior. Supply fragmentation suppression, Compressed rates, timeouts, durations, and protection airtime. | Real association supplies rate state. Production checks cover receipt retention, zero holes, interframe release, and ordinary gap retention. Empty records yield zero bits. Wrong-profile and malformed input cannot report success. Explicit Basic retains fragments and its response contract. Verify the accepted fatal conflict diagnostic; record the first Compressed timing result before C3 or C4. |
| C3. Verify both profiles and Compressed timing. | Add focused Compressed rate and timing cases. Retain the Basic response-mode fixture and its 152-byte assertions. Add 32-byte Compressed cases with directional queries, peer isolation, and ACK/CTS controls. | Primary and legal alternate Compressed rates produce correct duration. Initial and later BAR use legal rates. Profile-specific lengths remain distinct. Timing checks distinguish permitted overruns from unlawful continuation. |
| C4. Activate the default and complete compatibility guidance. | Set Compressed as the final default. Add the migration diagnostics and external-style fixture. Keep explicit Basic settings and complete both serializer inventories. Add Compressed delayed-policy rejection. | Default eligible HT links use Compressed. Default links without usable HT state use Normal Ack. Explicit Basic remains usable without HT peer records. No Basic class, showcase configuration, parameter-study variant, or result row is removed. |

S5 consists of C1 through C4. Every commit must build. C4 follows runtime conversion and timing verification. The temporary Basic default in S3a prevents premature activation; the completed implementation defaults to Compressed as requested.

Retain the existing Basic methods for external consumers. Add Compressed methods or a shared variant-aware operation only where the existing contract cannot serve both formats. Document new interface obligations. Retention avoids Basic class removal; it does not guarantee that every external implementation needs no adaptation.

C2 adds all runtime operations for a supported Compressed exchange. C3 adds verification after that path exists. Basic fragment cases remain after C2 and C4. Ordinary fragmentation outside any agreement and rejected Compressed fragment input remain separate tests.

Retain these six speed records from [tests/speed/store.json](../../tests/speed/store.json). Each inspected record has `run_number = 0`.

| `working_directory` | `ini_file` | `config` |
| --- | --- | --- |
| `showcases/wireless/blockack` | `omnetpp.ini` | `Fragmentation` |
| `showcases/wireless/fragmentation` | `omnetpp.ini` | `HCFfragblockack` |
| `showcases/wireless/fragmentation` | `parameterstudy.ini` | `OnlyPacketSizeHCFfragblockack` |
| `showcases/wireless/fragmentation` | `parameterstudy.ini` | `PacketSizeHCFfragblockack` |
| `showcases/wireless/fragmentation` | `parameterstudy.ini` | `ThresholdHCFfragblockack` |
| `showcases/wireless/fragmentation` | `parameterstudy.ini` | `TxPowerHCFfragblockack` |

Retain the Block Ack `General`, `MixedTraffic`, and `NoFragmentation` records and TXOP `General`. Match records by directory, INI file, configuration, and run number. Reassess measured effects through the applicable baseline procedure. Do not delete these records because Basic is unsupported by the current standard.

Retain [Fragmentation.anf](../../showcases/wireless/fragmentation/Fragmentation.anf), its result inputs, filters, configuration order, and plot series. The inspected Block Ack and TXOP directories have no `.anf` files. Repeat the consumer search before implementation. The speed store has no records for either QoS example directory; their direct and fingerprint checks still apply.

Documentation will label every retained Basic demonstration as historical compatibility. Give the IEEE Std 802.11e-2005 origin and IEEE Std 802.11-2024 exclusion with their references. The label must accompany the relevant user-facing explanation, configuration description, and analysis caption. Do not require a new topology image solely because the standard status note changes.

Release notes and migration guidance will explain the new default and the explicit Basic compatibility setting. Compressed agreements suppress fragmentation; Basic agreements preserve it. The threshold still applies outside Compressed agreements. An HT PHY mode alone does not select the profile.

A smaller wire-only prerequisite can add type 2 serialization tests. It cannot establish a complete Compressed exchange without the required agreement, scoreboard, rate, and response behavior.

Keep `delayedAckPolicySupported` parameters and methods. Both currently default to false. The Compressed profile will reject true at initialization because this plan supplies no delayed Compressed exchange. Retain the existing deferred delayed test with explicit Basic selection. Basic retention does not make that missing behavior pass or authorize a new Basic initialization restriction.

The choices have explicit owners:

- The [lifetime plan](ieee80211-edca-data-lifetime.md#3-proposed-ownership-and-behavior) owns `msduLifetime` and its default.
- The [lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#4-standard-requirements-and-inet-choices) owns `addbaFailureTimeout`, its default, and discard-all.
- This plan owns `blockAckVariant`, its final Compressed default, and explicit Basic compatibility.
- This plan owns the fallback when the basic-rate set is unknown.

The [decision table](#maintainer-decisions) records the accepted lifetime default and transfer-completion predicate. Basic retention does not remove their compatibility effects. Required measurements remain unexecuted. The 100 ms response timeout and fatal profile-conflict policy are also accepted.

## 6. Verification and scope limits

The [lifetime plan](ieee80211-edca-data-lifetime.md#5-verification-and-evidence) owns lifetime verification. The [lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#6-direct-verification) owns setup, Action, inactivity, teardown, and association checks. Its performance section owns setup-delay and disposal measurements.

Retain these direct checks for recipient delivery, BAR channel access, and final Compressed support:

- Rate publication, R1: Establish real AP/STA association without direct MIB rate injection. Require known local operational rates, BSS basic rates, and peer operational rates at both stations. Compare committed rates and basic flags with the actual Supported Rates and Extended Supported Rates elements. STA management must update rates after a valid associated-AP Beacon; scan candidates and unsuccessful association attempts must preserve active rate state. Check removal after association loss and equivalent publication through simplified management. A rate-only update must preserve an unchanged agreement in either profile. Absent HT state must not retire a Basic compatibility agreement. Repeat these subscriber checks at C2 after R1 and S3b both exist.

- BAR classification, C2: Use BSS basic rates 6, 12, and 24 Mbps with peer operational rates that also include 54 Mbps. Exercise initial and continuation BAR through HCF. Require 24 Mbps for the initial BAR and 54 Mbps for an eligible later BAR. Repeat after fresh channel access and require initial classification again. Preserve RTS/CTS and ACK control assertions. Direct classification helper tests cannot replace the HCF path.

- Basic wire contract, C1: Check golden Type 0 octets, lengths, and fragment bitmap format. Retain Basic field round trips that encode Type 0. Reject a Basic reserved value that changes the Type. Type 0 must deserialize as Basic.

- Compressed wire contract, C1: Check golden Type 2 octets, lengths, and the 64-bit bitmap format. Adapt the current Compressed producers and expected fields with the serializer change. Check zero writes and ignored received values for B0 and B5–B11. Nonzero B3 or B4 changes the Type and cannot use that ignore rule. Check short frames and unsupported types 1, 3, and 4 through 15.

- Scoreboard validation, C2: Require zero bits for an empty scoreboard under an active agreement. A Basic frame under a Compressed agreement must fail profile validation before receipt-state changes.

- Compressed response validity: Inject matching Type 2 responses with nonzero Fragment Number, incorrect bitmap size, or incorrect length. Exercise serialized and directly supplied typed input. Require failed exchange recovery without acknowledgment, inactivity refresh, or success feedback.

- Receipt retention, C2: Drive DATA 100, 102, and 101 through the production recipient path under an accepted Compressed agreement. Require delivery in order 100, 101, and 102. Send BAR at 100 and require all three receipt bits after delivery. Retain Basic fragment checks separately.

- Scoreboard holes, C2: With agreement start 100 and size eight, receive only DATA 115. Require scoreboard window 108–115, a positive bit only for 115, and zero bits for 108–114. Repeat across sequence wrap. These checks must reject the Basic getter's inferred positives for absent earlier positions.

- Recipient fragments: Retain S1 scenario 0 to check Basic capacity and complete-unit delivery through the recipient path. At C2, inject fragmented DATA under an active HT-immediate agreement. Use nonzero Fragment Number or More Fragments set. Require no receipt state update, positive bitmap bit, fragment state, or DATA delivery. These checks prohibit fragmented Compressed reception; the originator checks below prohibit fragmented transmission. Repeat S1's Basic fragment case at C2 and C4, with fragment status and complete-unit delivery retained.

- Originator fragmentation: With an active HT-immediate agreement, extract an MSDU above the configured fragmentation threshold. Check one unfragmented DATA frame, including when the ACK policy selects Normal Ack. Without an agreement, check ordinary fragmentation and Normal Ack. Start setup while numbered fragments remain and check that all resolve before ADDBA transmission. Verify that the first DATA under the new agreement is unfragmented and uses the accepted request number.

- Basic BAR timing, S4: Compare with S4's pinned parent before publication. Test standalone BAR, final BAR after one or four DATA frames, and late continuation near a positive TXOP limit. Exercise single and multiple protection. Record modes, sizes, IFS, response airtime, completion time, and the Data/Management count.

  Check applicable TXNAV and TXOP bounds with each permitted-overrun condition explicit. Separate inherited violations from violations introduced by S4. Resolve a new unlawful overrun before publication of that affected Basic path.

- Airtime and TXOP: At C2, repeat affected S4 checks with Compressed exchanges and compare with C2's pinned parent. Test above-threshold DATA near a positive TXOP limit with Block Ack enabled. Record DATA and response modes, sizes, IFS, protection, response airtime, and completion time. Record the Data/Management count and each aggregate format within that TXOP.

  Check containment or the exact permitted-overrun condition under Clause 10.23.2.9. Measure Compressed timing through its infrastructure fixtures. Check retained Basic showcase airtime separately, with both wireless QoS hops and the current ad hoc hops. Separate permitted overruns, inherited violations, and new violations before the C3 or C4 decision.

- Final response timing: At C3, repeat affected airtime checks across primary and legal alternate response modes implemented by C2. Include the lowest supported rates and late continuation near the TXOP limit. Test a permitted initial unaggregated MSDU under Block Ack, an initial A-MSDU, and a later exchange that cannot finish within the required bound. Require containment for the A-MSDU and applicable later exchange. A permitted initial overrun must not create a false admission prerequisite. A C2 result covers only its recorded modes and exchange boundaries; a fingerprint match does not prove timing compliance.

- Demonstration coverage: Preserve S3a's explicit Basic selection and recorded direct assertions from Section 5. Repeat the assertions after A2 and at C2/S5. Every legacy Block Ack demonstration must retain actual Basic exchanges, including ad hoc QoS run 1. Retain run 0 as the support-disabled control. Check Compressed capability-gap fallback separately; it must not replace legacy coverage.

- Fixture coverage: S3a records the [tracked fixture inventory](#s3a-tracked-fixture-inventory-and-vht-scope), including HT/VHT state and actual failure causes. S3b runs the capability, agreement, A-MSDU permission, and fallback checks before final conversion. C2 requires Compressed responses in retained positive Block Ack cases. C4 runs the final inventory, serializer expectations, and delayed-policy rejection fixture. Report deferred protocol obligations separately from successful exchanges and successful fallback checks.

- Recipient gap release, completed S1: Retain scenarios 1 and 2 of the recipient fixture. Scenario 1 buffers complete DATA 101 and 102 behind absent DATA 100. Receipt of DATA 100 must release all three units in order in that operation. Scenario 2 repeats this condition across sequence wrap. Retain their assertions for delivery order and the resulting reorder window without a later DATA reception or BAR.

- Recipient interframe release, C2: Supply an agreement with start 100 and size four through HCF. Receive Normal Ack DATA 100 and observe its ACK completion. In a zero-propagation control, start DATA 104 reception one SIFS after local ACK transmission ends. Require adjusted scoreboard start 101, receipt evidence for 100, immediate delivery of 104, and reorder start 105.

  Use actual receive times and the transmission callback. A directly supplied relation flag cannot replace this production check. Repeat with applicable SIFS and RIFS relations after a previous MPDU. Repeat the positive case across sequence wrap.

- Recipient interframe controls, C2: Repeat with a false bit at `WinStartR - 1` and an interval outside the accepted timing relation. Each control must retain 104 behind the gap. Add nonzero propagation delay and intervening unrelated reception or transmission. Test history invalidation after agreement retirement, peer cleanup, and replacement setup. Check that stale history cannot release DATA. Record exact receive and transmit endpoints and the applied timing allowance. Retain Basic controls and explain each changed Compressed fingerprint through observed delivery events.

- Recipient BAR before DATA, completed S1: Retain scenario 3 with supplied agreement start 100 and buffer size eight. Before DATA, valid Basic BAR at 104 must create the record and set reorder start 104. Subsequent DATA 104 must reach immediate delivery. Invalid BAR and BAR without an agreement must create no reorder record.

Repeat both recipient cases with Compressed frames at C2 and C4. Each result must preserve the repaired delivery order and window state.

- Recipient window and BAR: Test DATA beyond `WinEndB` after an older DATA drop. Check its delivery, scoreboard bit, and released older complete units. With DATA 100 absent and DATA 101 buffered, send a valid BAR at 101. Check delivery of 101 and independent scoreboard movement. Release 4095 and 0 in one operation and assert that exact order at the next MAC process. An invalid BAR must leave both windows unchanged.

- BAR originator: Test BAR-only contention, response matching, timeout, older retries, and window advance after acknowledgment or drop. With single protection enabled, check that BAR selection and duration calculation do not extract or number queued DATA. Test two peers with the same TID and a BAR for only one peer. Under an established agreement, test one and four DATA frames with the default threshold of five. Keep peer B continuously eligible while peer A has four outstanding frames. Check that A receives BAR before another DATA for B.

- Originator identity, S4: Give one peer outstanding DATA for TIDs 2 and 5. Check separate threshold counts and BAR selection for each TID. Transmit BAR for TID 5. Require no ACK-state change for TID 2. Repeat these checks with Compressed frames at C2 and final S5.

- Small originator windows, S4: Accept buffer sizes one and four with BAR threshold five. Require BAR before DATA beyond each transmit window. Repeat these checks with Compressed frames at C2 and final S5.

- Earlier Compressed response start, C2: Use transmit window 100 through 163 and BAR start 104. Let transmitted DATA 102 remain unacknowledged without a wait for that BAR. Receive a matched response with start 100 and a positive bit for DATA 102. Require acknowledgment of 102 through HCF. Positive bits for unsent frames or frames outside the active window must leave those frames unacknowledged.

- Later or disjoint Compressed response, C2: Send a matched response whose start follows some requested frames. Require unchanged acknowledgment state below that start. Require those frames to leave their BAR wait and become retry eligible. Send a matched response without overlap. Require every requested frame to remain unacknowledged and become retry eligible.

- DATA lifetime integration: Repeat zero-bit Compressed responses and BAR timeouts until the existing DATA deadline expires. Check sequence preservation, Retry-bit policy, disposal, and window progress. Let the deadline expire during an accepted Compressed exchange. Check safe completion and absence of another DATA attempt through the lifetime owner's contract.

- Response rates, C3: Test the highest qualifying BSS basic rate and the mandatory-rate fallback when no basic rate qualifies. Test the documented INET fallback when the basic-rate set is unknown. For HT and VHT BAR inputs, assert the non-HT reference rate and the applicable OFDM or ERP-OFDM modulation class. For non-HT BAR, assert the received modulation class and preamble type. Test a legal alternate response rate with equal duration and an invalid override. Observe the transmitted response mode and Duration/ID through HCF.

- Originated BAR rates, C3: Require a non-HT PPDU for initial and later Compressed BAR. Give the BSS and peer different legacy rate sets. Check the initial basic-rate rule and the later peer-operational-rate rule. Check fallback with unknown peer state, an empty basic-rate set, and unknown BSS rate state. Reject a configured HT/VHT BAR mode or a legacy rate that fails the applicable rule. Preserve unrelated RTS, CTS, ACK, and DATA mode checks.

- Migration diagnostics, C4: Use the [external-style migration fixture](#migration-diagnostics), outside the explicit legacy override inventory. Check local non-HT warnings, HT ad hoc fallback reasons, and explicit Basic exchanges. Disabled support must produce no irrelevant warning. Keep Compressed as the selected default.

- Profile conflict, C2: Send a valid addressed BAR of the other variant to an active peer/TID. Require the fatal configuration diagnostic before agreement, scoreboard, reorder, response, or inactivity updates. Check both mismatch directions and no conflict-triggered DELBA. Require termination at the first qualifying BAR. Verify diagnostic fields and cause, not merely an expected failure status. In separate compatible runs, verify Basic and Compressed exchanges plus unaffected Normal Ack traffic. Keep malformed, unknown-variant, unaddressed, and absent-agreement cases separate.

- Compatibility and activation: Default eligible HT links must use Compressed. Explicit Basic must preserve legacy classes, bitmap fragments, response lengths, and existing NAV and protection assertions. Change `N_CompressedBa.test` to require Compressed and retain positive `Ac_BlockAck` coverage through real HT state. Keep Basic NAV and multiple-protection scenarios; add Compressed scenarios rather than removal of Basic coverage. Extend response-mode tests with 32-byte Compressed cases beside 152-byte Basic cases. Retain directional queries, peer isolation, asymmetric state, and ACK/CTS controls.

The [standalone lifetime tests](ieee80211-edca-data-lifetime.md#5-verification-and-evidence) own identity, aggregation, fragmentation, and IFS deadline checks. Run each prerequisite's tests against its Basic path before its replacement. Repeat affected integration checks with Compressed frames at S5.

Use direct production assertions for peer, TID, response timing, bitmap, channel access, agreement state, and queued-frame outcome. A unit bitmap test alone cannot prove HCF uses the result.

Add a proposed `tests/unit/Ieee80211CompressedBlockAck_1.test` for wire and scoreboard checks. Add a proposed `tests/module/Ieee80211HcfCompressedBlockAck_1.test` for the production exchange.

Use the HCF module fixture for identity, small-window, shifted-response, response-mode, and profile-mismatch cases.

- Test the default Compressed setting and explicit Basic at both endpoints.
- Test unknown selector values at initialization.
- Give two HT peers different profiles.
- Require no cross-variant acknowledgment. A valid addressed BAR that conflicts with an active agreement must cause the exact fatal configuration diagnostic before state updates. Require no conflict-triggered DELBA or repeated setup. Verify compatible traffic in separate runs; absent-agreement recovery retains its separate DELBA checks.

Retain [Ieee80211BlockAckReordering_1.test](../../tests/module/Ieee80211BlockAckReordering_1.test) through the later stages. Its current scenarios 0 through 7 assert delivery and state through `RecipientQosMacDataService`. Scenario 0 covers fragment capacity; scenarios 1 and 2 cover gap release and wrap; scenario 3 covers BAR before DATA. Scenarios 4 through 7 cover BAR order, window movement, peer/TID isolation, and A-MSDU order. The fixture supplies accepted Basic agreements. A2 must add negotiation checks through HCF, while C2 and C4 must verify Compressed integration.

After the required fresh debug build, use this current selector from the checkout root. The fixture selects all eight recipient scenarios and seed 0 internally. The command also selects the three current NAV runs. This document update does not execute the command.

```sh
inet_run_module_tests -m debug -f '(Ieee80211BlockAckReordering_1|Ieee80211RecipientNav_1)\.test$'
```

Run those exact cases with the unit and module runners in debug mode. Select all eight current protocol fixtures. Add the proposed absent-peer-state `ac` fallback fixture after it exists. Select the four Block Ack-enabled module fixtures and five support-disabled controls. Include the separate S1 recipient fixture with all eight Basic scenarios.

Include the HT mode-set, VHT/HT capability, and response-mode unit fixtures. Add the proposed delayed-policy rejection fixture at C4. Run the serializer fixture with its suite's runner when C4 changes its expected fields. A fixed seed suffices for these deterministic cases.

For each stage, record selected paths and runs, expected and observed outcomes, and the first failed assertion or initialization error. Preserve source includes and generated configurations with the evidence. Distinguish an expected protocol failure from generation, compilation, or unrelated runtime failures. Keep the four current deferred protocol requirements separate from positive `Ac_BlockAck` and absent-peer-state fallback coverage. Appendix A separates completed S1 results from the earlier capability results. Checks for the later stages stay proposed.

Use the common selection from `tests/fingerprint/showcases.csv` and `tests/fingerprint/examples.csv` through final C4. Retain all 19 recorded rows. Preserve each row's configuration, run number, time limit, and fingerprint ingredients. Explicit Basic settings prevent the default change from converting these legacy cases. Shared repairs can still change results; record their causes rather than an assumed unchanged baseline.

The four wireless QoS rows exercise shared HCF callers, including `MacQosWithBlockAck`. The two ad hoc QoS rows separate the support-disabled control from retained Basic compatibility after A2. The TXOP row preserves its intended mixed frame exchanges through topology and protocol changes. Block Ack-disabled EDCA rows check regression effects within S2's broader lifetime scope. S2 must also satisfy the lifetime plan's [measured-expiry requirement](ieee80211-edca-data-lifetime.md#5-verification-and-evidence). DCF rows serve as controls for shared changes.

| Simulation path | Retain after C4: 19 rows |
| --- | --- |
| `showcases/wireless/blockack` | `NoFragmentation`, `MixedTraffic`, `Fragmentation` |
| `showcases/wireless/aggregation` | `NoAggregation`, `Aggregation`, `VoicePriorityAggregation` |
| `showcases/wireless/fragmentation` | `DCFnofrag`, `DCFfrag`, `HCFfrag`, `HCFfragblockack` |
| `showcases/wireless/qos` | `NonQos`, `Qos` |
| `showcases/wireless/txop` | `General`; run 0 |
| `examples/wireless/qos` | `MacQos`, `MacQosWithoutAggregation`, `MacQosWithRtsCts`, `MacQosWithBlockAck`; run 0 |
| `examples/adhoc/qos` | `MacQos`; runs 0 and 1 |

The TXOP row records five seconds; each ad hoc QoS row records ten seconds. These limits come from the fingerprint CSV entries, not the INI defaults. Block Ack `General` has no separate fingerprint row; its S3a checks include direct exchanges and its retained speed record.

Record the exact selected rows before execution. Compare each stage with its pinned parent before later stages change the same paths. Extend the selection when changed callers or a failure justify more cases. Diagnose the first changed exchange in each affected case before any baseline proposal.

Record a cause for each changed result at the stage that introduces it. A stage need not change every selected fingerprint. Preserve raw results and the selected parent revision. Fingerprints check regression effects; direct production assertions establish the required behavior. S1's four approved changes remain recorded under its [separate authorization](../done/ieee80211-compressed-blockack-s1.md). This plan revision authorizes no additional baseline update.

Follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md) for builds and checks. Each later implementation will record exact commands, modes, selectors, and outcomes. Appendix A records completed S1 and earlier capability verification. Checks for the later implementation stay unexecuted.

Basic frame classes and existing builders remain public. The implementation will update user and migration guides, release notes, configuration descriptions, analysis captions, and serializer coverage. State that Compressed is the default and `blockAckVariant = "basic"` selects historical compatibility. Cite IEEE Std 802.11e-2005 as the origin and IEEE Std 802.11-2024 as the standard that reserves type 0. Do not claim current-standard support for Basic or complete historical conformance for existing INET behavior.

Document added interface obligations without removal of Basic contracts. Explain `addbaFailureTimeout`, per-EDCAF `msduLifetime`, setup deferral, final-transfer DELBA, and discard-all under their separate owners. Preserve individual-DATA and stop/start limitations. No existing showcase topology or configuration name needs migration under this decision.

The [lifecycle plan](ieee80211-blockack-agreement-lifecycle.md#3-agreement-behavior) owns recovery after rejection, uncertain setup, and failed DELBA for both profiles. Normal Ack fallback due to absent HT state belongs to Compressed. Basic compatibility must not silently become that fallback when the default changes.

Release notes must retain positive Compressed coverage for eligible `ac` peers and the four existing deferred protocol requirements. Explain originated Compressed BAR's non-HT mode and `controlFrameBitrate` validation. Keep complete VHT transfer support outside the claim.

The table gives normative IEEE Std 802.11-2024 evidence for Compressed and shared repairs. Section 2 gives the historical IEEE Std 802.11e-2005 references. Shared repairs do not establish complete Basic conformance to either edition. [Appendix B](#appendix-b-standards-source-notes) retains canonical locators and extraction limits.

| Decision | Clause or table | Physical PDF pages |
| --- | --- | --- |
| Fragmentation boundary for HT-immediate agreements | 10.2.7 | 1882 |
| VHT stations retain HT operation | 4.3.15 | 356–357 |
| Initial and later BAR transmission rates | 10.6.6.2; 10.6.6.4 | 1943–1945 |
| Non-HT responses to non-HT, HT, and VHT BAR; legal alternate rates | 10.6.6.5.2; 10.6.6.5.4 | 1945–1947; 1950 |
| HT/VHT non-HT reference-rate calculation | 10.6.11 | 1962–1963 |
| Additional exchange bounds and permitted TXOP overruns | 10.23.2.8; 10.23.2.9 | 2010–2014 |
| Compressed retransmission | 10.25.6.7.1 | 2068–2070 |
| Retry for individual DATA; scope of the QoS retry rule | 9.2.4.1.6; 10.23.2.12.1 | 665; 2016–2017 |
| Positive bitmap bits within the transmit window, relative to the received Block Ack start | 10.25.6.8 | 2071–2073 |
| Consecutive delivery when DATA fills a reorder gap | 10.25.6.6.2.1(a)(2) | 2065–2066 |
| Individual-MPDU delivery with SIFS/RIFS and receipt evidence at `WinStartR - 1` | 10.25.6.6.2.1(a)(2)(v) | 2065–2066 |
| Full-state receipt retention and zero status for absent positions | 10.25.6.3 | 2060–2061 |
| Reorder window movement after BAR, including an empty window | 10.25.6.6.3.1 | 2067 |
| HT capability advertisement in IBSS beacons is a valid alternative | Table 9-62, row 33 | 781 |

Table 9-62 requires HT Capabilities when `dot11HighThroughputOptionImplemented` is true and the station is not a 6 GHz station. The condition does not exclude IBSS. The choice to defer IBSS capability support remains an INET scope decision.

The prerequisite plans own [lifetime evidence](ieee80211-edca-data-lifetime.md#2-current-source-and-standard-support) and [lifecycle evidence](ieee80211-blockack-agreement-lifecycle.md#8-standards-evidence-and-scope-limits). Fatal profile-conflict termination is a model choice under the [lifecycle contract](ieee80211-blockack-agreement-lifecycle.md#profile-conflict-detection-and-termination), not a new requirement inferred from Clause 11.5.4.

### Airtime acceptance and admission dependencies

The current branch excludes Block Ack-enabled HCF from complete-cost single-protection TXOP admission. [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) selects that admission path only when `isBlockAckSupported` is false. [HcfFs](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) rejects continuation when no TXOP time remains. That check does not prove that the complete next exchange fits. This plan does not add complete-cost admission for Block Ack.

S4 introduces Basic BAR paths before Compressed integration. Its owner must assess those changed exchanges against S4's parent before S4 publication. A standalone BAR can qualify for a Control-frame overrun exception. A final BAR after several DATA frames cannot use the one-Data-or-Management-frame exception. Hypothetical example: four DATA frames leave 20 microseconds, but their final BAR exchange needs 80 microseconds. S4 must not introduce that unlawful continuation without an admission correction or a verified scope restriction.

S4's record owns violations introduced by S4. The C2 comparison cannot relabel such a violation as acceptable inherited behavior. An inherited violation does not itself require this plan to repair every existing TXOP path. Each stage must still prevent new violations within its supported change. C2 and C3 repeat the assessment for Compressed integration and its supported response modes.

At a fixed PHY mode, an unfragmented DATA frame takes longer than one constituent fragment. The total airtime can still be lower than the complete fragment sequence because fewer headers and responses are necessary. Hypothetical example: a previous DATA exchange ends with 100 microseconds left, but another DATA exchange needs 200 microseconds. The positive remaining interval does not authorize that continuation. Clause 10.23.2.8's applicable TXNAV bound and Clause 10.23.2.9's TXOP restriction still apply.

An overrun alone does not establish a violation. Clause 10.23.2.9 permits specified overruns when the TXOP contains at most one Data or Management frame. These cases include initial transmission of an MSDU under a Block Ack agreement, outside an A-MSDU or a multi-MPDU A-MPDU. The clause also permits specified retransmissions and Control frame transmissions under its stated conditions. The timing record must identify the exact exception and every applicable condition. It must also check any independent TXNAV requirement.

Hypothetical example: the first unaggregated MSDU under an agreement needs 200 microseconds within a 100-microsecond TXOP limit. The TXOP contains no other Data or Management frame. Clause 10.23.2.9 permits that initial-transmission case under its stated aggregate conditions. The same example with an initial A-MSDU does not qualify. A second DATA exchange also defeats the one-Data-or-Management-frame condition.

No current evidence proves timing compliance for final Compressed operation or the repaired legacy demonstrations. C2 must record its first timing result before C3 or C4 proceeds. Each result must distinguish permitted overrun, inherited violation, and new violation. A permitted overrun creates no admission-correction prerequisite.

For a new violation in a supported case, the affected stage requires a correction through the separate TXOP design before publication. Apply that condition to S4's changed Basic paths and C2's changed Compressed paths. C3 repeats the Compressed classification across response modes and exchange boundaries. An inherited violation does not justify a new violation.

If S4 evidence establishes a required correction, its affected Basic path cannot publish until that correction passes its direct checks. S5 likewise cannot complete while a required Compressed correction remains absent or unverified. A new unlawful continuation can block its dependent stage. A permitted initial overrun cannot. Record the blocked path, violated clause, required correction, and comparison parent in the responsible stage's timing result.

A restriction on supported configurations is an alternative only when direct evidence proves timing compliance throughout its stated scope. Do not select a fixed MSDU size cap as an assumed solution. Hypothetical example: a capped second DATA exchange needs 80 microseconds, but only 50 microseconds remain. The cap does not prevent that unlawful continuation. PHY rates, protection, responses, and the remaining interval all affect the required bound. No size cap or silent Normal Ack fallback is an automatic recovery policy in this plan.

## Appendix A. Executed evidence

### Completed S1 and recorded verification

S1 merged through PR #1320 on 14 October 2026 at `1cdb362a2a4b97f564566487079d9d156a5d213d`. Its original verified feature head is `fcc06450355a52b9c8b0f8f2647e0654711bfaca` on `work/compressed-blockack-s1`. Its original publication base is `ac6344cc5bcac972012759281a11b3f8255b5d46`. The [S1 source record](../done/ieee80211-compressed-blockack-s1.md) supplies scope, reproduction, and approved fingerprints. Its [human summary](../done/ieee80211-compressed-blockack-s1-summary.md) explains recipient behavior and caller requirements. The results below retain their original source boundaries and establish no fresh runtime verification at the merged revision.

The [published PR description](https://github.com/inet-framework/inet/pull/1320) records verification at each original source boundary. The local final-series audit supplied additional driver commands and raw result locations. That untracked artifact is absent from this worktree. Use the PR description and committed S1 records for available evidence. This plan update ran no new build, module test, simulation, or fingerprint comparison.

| Recorded check | Result | Scope limit |
| --- | --- | --- |
| Debug and release builds at each commit. | All recorded builds pass. | Release behavior tests remain unrun. |
| Debug recipient fixture at the final head. | All eight scenarios pass through the production recipient service. | Agreements and buffer sizes are supplied; ADDBA negotiation and Compressed exchanges remain outside this evidence. |
| Debug recipient NAV fixture at the final head. | The `hcf`, `dcf`, and `blockAck` runs pass. | The Basic HCF exchange does not establish negotiated capacity or recipient DATA delivery. |
| Debug fingerprint selection at the final head. | All 12 selected configurations pass with the approved S1 values. | This selection differs from the proposed 19-row common inventory; it does not verify that full inventory. |

The recorded module driver uses `opp_repl` and `run_opp_tests()` for `tests/module` in debug mode after the library build. Its filter is `(Ieee80211BlockAckReordering_1|Ieee80211RecipientNav_1)\.test$`. Both fixtures use seed 0. The recipient fixture selects scenarios 0 through 7 at the original final head. The NAV fixture selects `hcf`, `dcf`, and `blockAck`. These are recorded selectors and results, not fresh execution at the merged revision.

The fingerprint selection uses the command below from `tests/fingerprint`. The recorded final invocation returns exit status 0. Each selected row uses run 0, its CSV time limit, and the default seed set. The command is recorded evidence, not a new execution by this document update.

```sh
./fingerprinttest -d -t 4 \
  -m 'showcases/wireless/(blockack|fragmentation|aggregation)/|examples/wireless/qos/.*-c (MacQosWithBlockAck|MacNonQos)' \
  -f tplx -f '~tNl' -f '~tND' showcases.csv examples.csv
```

The delivery commit `53130418de` owns four baseline changes under the maintainer's separate approval. Each first changed operation releases additional complete units when DATA fills a gap. The [S1 source record](../done/ieee80211-compressed-blockack-s1.md) gives exact times and fingerprint values for these configurations:

- `showcases/wireless/blockack:NoFragmentation`, run 0.
- `showcases/wireless/blockack:MixedTraffic`, run 0.
- `showcases/wireless/fragmentation:HCFfragblockack`, run 0.
- `examples/wireless/qos:MacQosWithBlockAck`, run 0.

The capacity and BAR commits change no baseline. The final series preserves Basic frame formats, fragment bitmaps, and the ordered `ReleasedFrames` contract. The protected compatibility lookup remains deprecated for one release, as the migration guide states. S1 establishes no lifetime, lifecycle, rate publication, Compressed exchange, or performance result.

### Branch verification record

The results below are historical evidence from the VHT/HT branch before S1, the proposed selector, and the Compressed default. They retain their original source and scope limits. The separate S1 record above describes the completed recipient repair. This document revision adds no execution result.

The review executed these checks on 13 October 2026 at source revision `6aefaef1f01a70134fa8283de708353d4a6d4fd8`. The working directory was `/home/user/omnetpp_ws/inet-ieee80211-vht-ht-support`. The build and all three runner commands returned exit status 0. The fixtures use `General` with seed 0. The module fixture runs scenarios 0 through 6.

| Executed check | Observed result | Scope limit |
| --- | --- | --- |
| `make MODE=debug -j8` | The debug build passed and reported no further source build work. | This build precedes the proposed implementation. |
| Unit filter `Ieee80211(HtModeSet\|VhtHtCapabilities)_1\.test$` | Both fixtures passed. They require HT operation and local HT capabilities under `ac`. | They do not establish a production Block Ack exchange. |
| Protocol filter `/11ac/Ac_BlockAck\.test$` | The fixture passed its Action, BAR, and Block Ack frame-type assertions. The current BAR and Basic response selected 78 Mbps. | The generic assertions do not prove accepted parameters, complete bitmap semantics, or Compressed operation. |
| Module filter `Ieee80211VhtHtSupport_1\.test$` | The fixture passed all seven scenarios. Full and simplified management supply peer HT state. Scenario 1 removes that state at stop. | It disables Block Ack and therefore supplies capability and control evidence only. Stop also removes association; it is not the proposed fallback stimulus. |

The exact runner commands follow. Each output redirection preserved the raw runner result in the review directory.

```sh
inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet|VhtHtCapabilities)_1\.test$' --log-file /tmp/inet-compressed-blockack-review-20261013/unit.log > /tmp/inet-compressed-blockack-review-20261013/unit-output.txt 2>&1
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f '/11ac/Ac_BlockAck\.test$' --log-file /tmp/inet-compressed-blockack-review-20261013/protocol.log > /tmp/inet-compressed-blockack-review-20261013/protocol-output.txt 2>&1
inet_run_module_tests -m debug -f 'Ieee80211VhtHtSupport_1\.test$' --log-file /tmp/inet-compressed-blockack-review-20261013/module.log > /tmp/inet-compressed-blockack-review-20261013/module-output.txt 2>&1
```

The original record named generated stdout at `tests/protocol/wifi/work/Ac_BlockAck/test.out` and `tests/module/work/Ieee80211VhtHtSupport_1/test.out`. Those paths refer to the earlier checkout; this checkout contains neither output file. Later execution can replace generated output, and temporary-file cleanup can remove the raw review directory. The commands, source revision, selectors, and result limits above preserve the scope of this historical record. These results replace the obsolete absent-HT premise; they do not verify the proposed lifetime, lifecycle, or Compressed repairs.

### Source comparison for evidence reuse

The source comparison covers `6aefaef1f01a70134fa8283de708353d4a6d4fd8` and `c15860de93b3764c3e4ead69c6933fc5ae9ce957`. It found only these changed paths:

- `WHATSNEW`.
- `doc/src/migration-guide/index.rst`.
- `doc/src/users-guide/ch-80211.rst`.

The `src`, `tests`, and `bin` tree IDs match between those two historical revisions. Their runtime and test inputs therefore match the recorded behavioral source. This equality does not describe the current source after S1, which changes recipient source, tests, and fingerprints. The structural proof permits reuse only within the earlier checks' stated limits. It adds no Compressed, lifetime, lifecycle, or performance result.

The comparison ran from the checkout root on 14 October 2026. Both commands returned exit status 0:

```sh
git diff --name-only 6aefaef1f01a70134fa8283de708353d4a6d4fd8 c15860de93b3764c3e4ead69c6933fc5ae9ce957
git diff --quiet 6aefaef1f01a70134fa8283de708353d4a6d4fd8 c15860de93b3764c3e4ead69c6933fc5ae9ce957 -- src tests bin .oppfeatures .oppfeaturestate Makefile .envrc
```

The first command reports the three documentation paths. The second confirms equality for the selected runtime and test inputs. The three directory tree IDs were also compared directly. No behavioral rerun was necessary solely for this documentation difference.

The final review will check [the IEEE 802.11 domain rules](../../doc/project/domain/ieee80211.md), [minimal design](../../doc/project/rule/quality.md#qr-design-minimal), and [the plan procedure](../../doc/project/guide/write-an-implementation-plan.md).

## Appendix B. Standards source notes

Current-standard canonical clause identifiers use `ieee80211-2024:clause:` followed by the clause number. The IBSS table identifier is `ieee80211-2024:table:9-62`. The main evidence table gives physical PDF pages; printed pages differ by one in those ranges.

The evidence has these limits:

- IEEE Std 802.11e-2005 is present as `../inet-standards/80211e-2005.pdf`, relative to the repository root. The corpus processor does not register this document. No canonical corpus nodes or resolved reference records exist for it. Direct PDF inspection verifies normative Clauses 7.2.1.7, 7.2.1.8, and 9.10. Physical pages 52–54 and 118–122 correspond to printed pages 30–32 and 96–100. Figures 21b and 21e require visual inspection on physical pages 53 and 54.

  The source SHA-256 is `113625f95796112a44aec95378b79362de1c342ee951df5704521f11d1d227a1`. Basic retention makes no complete historical or current-standard conformance claim.

- Current control layouts use `ieee80211-2024:figure:9-47` and `ieee80211-2024:figure:9-53`, on physical pages 719 and 722. Their extracted nodes contain captions; PDF inspection supplies the bit layout. Figure 9-47's reference to Table 9-36 resolves. Tables 9-36 and 9-37 resolve at the canonical identifiers in Section 2.
- The individual-MPDU rule uses normative node `ieee80211-2024:clause:10.25.6.6.2.1`, physical pages 2065–2066. Its source locator is `ieee80211-2024@8602874:8608005`. PDF inspection confirms condition (a)(2)(v) and its continuation. This node has no extracted outgoing references.
- Full-state status uses normative node `ieee80211-2024:clause:10.25.6.3`, physical pages 2060–2061. Its source locator is `ieee80211-2024@8581659:8586175`. PDF inspection confirms its DATA and BAR update rules. This node has no extracted outgoing references.
- Clause 9.2.4.1.6 requires Retry for individual non-A-MPDU DATA. The prior PDF check on physical page 665 confirmed the exception's scope. The reference from Clause 10.23.2.12.1 to Clause 10.3.2.11 resolved.
- The extracted Clause 10.25.6.8 node ends at a figure. The PDF text on physical page 2073 supplies the positive-bit rule after that figure. That rule uses Block Ack start, the active transmit window, addresses, and TID. The shortened node is incomplete evidence.
- Relevant clause references resolved except the out-of-scope NGV reference from Clause 10.25.2 to Clause 31.2.3. This gap supplies no conformance claim for excluded features.
