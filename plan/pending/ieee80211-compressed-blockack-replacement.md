# Basic and Compressed Block Ack implementation status

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc), [HCF parameters](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.ned)

This checkout implements Basic Block Ack agreement lifecycle and transfer completion. Basic remains the default profile. Production Compressed exchanges are absent. HCF rejects `blockAckVariant = "compressed"` at initialization with `Compressed Block Ack exchanges are not implemented`.

This document describes the source in this checkout. The [summary](ieee80211-compressed-blockack-replacement-summary.md) gives the main behavior. The [verification record](ieee80211-compressed-blockack-s4-execution.md) gives current results and their limits.

## Previous PRs and S4 scope

The branch `work/compressed-blockack-s4` contains the changes for [PR #1344](https://github.com/inet-framework/inet/pull/1344). Its master base already contains the following merged work. Those mechanisms are prerequisites, not additions from S4.

| Previous PR | Behavior inherited by S4 |
| --- | --- |
| [#1320: recipient progress](https://github.com/inet-framework/inet/pull/1320) | Receive capacity from agreement values, cyclic delivery, consecutive gap release, and Basic BAR progress before DATA. |
| [#1331: QoS DATA lifetime](https://github.com/inet-framework/inet/pull/1331) | Identity-based deadlines, the 0.512-second default, expiry, aggregation lifetime rules, and safe completion across expiry. |
| [#1343: Basic agreement lifecycle](https://github.com/inet-framework/inet/pull/1343) | Setup validation, Action dispatch, directional teardown, DATA deferral, A-MSDU permission, and recipient teardown disposition. |
| [#1368: association replacement cleanup](https://github.com/inet-framework/inet/pull/1368) | Explicit management-to-MAC peer reset before association replacement, with cleanup before deletion callbacks. |

S4 uses those existing owners to complete outstanding Basic DATA when another DATA candidate cannot proceed. Its production changes have four commit boundaries:

| S4 commit | Change relative to its parent |
| --- | --- |
| Expose DATA preview without extraction | Adds the QoS preview interface and borrowed candidate access through `InProgressFrames`, with prospective sequence lookup. The concrete data service already supports preview. |
| Mark DATA retries after failed Block Ack feedback | Sets Retry on retransmitted standalone DATA. Existing ACK status already permits that retransmission. |
| Add shared originator window operations | Adds cyclic distance, bitmap membership, and window advancement before their production callers. This preparation changes no packet trajectory. |
| Complete Basic BAR transfer resolution | Selects BAR before extraction, permits BAR-only contention, and separates outstanding groups by peer and TID. It adds matched feedback, control-failure recovery, completion DELBA, and prepared-successor reservation rules. |

The final S4 commit documents that integrated result and its verification. The filtered queue API, finite lifetime mechanism, and ordinary recipient teardown delivery remain inherited contracts. S4 connects final resolution or lifetime disposal to window progress and completion DELBA through those contracts.

For example, recipient teardown already knows how to release complete units behind a gap. S4 makes final DATA expiry trigger completion DELBA when no DATA remains. The new trigger uses the existing recipient disposition; S4 does not introduce that disposition.

Master also supplies profile validation, Basic defaults, and coherent rate-state publication. S4 preserves Basic as the default and retains Compressed initialization rejection. Production Compressed exchanges belong to the separate S5 branch and [PR #1345](https://github.com/inet-framework/inet/pull/1345), outside this checkout.

## 1. Supported behavior

Block Ack support is optional through `isBlockAckSupported`, which defaults to false. With support enabled, HCF coordinates immediate Basic agreements for one peer and traffic identifier (TID). Basic permits the existing non-HT and ad hoc configurations. Its compatibility profile does not require new peer HT capability records.

The recipient maintains receipt status separately from its reorder buffer. Receipt status controls Block Ack feedback; the reorder buffer controls delivery. A positive acknowledgment therefore does not by itself prove upper delivery. When DATA fills a gap, the recipient releases consecutive complete units in the same operation. A valid Basic BAR can create or advance the reorder record before DATA arrives.

The [agreement lifecycle](ieee80211-blockack-agreement-lifecycle.md) owns setup, Action dispatch, directional teardown, and peer reset. The [DATA lifetime contract](ieee80211-edca-data-lifetime.md) owns finite QoS DATA deadlines, with Block Ack enabled or disabled.

## 2. Standard support and model boundary

Basic represents the frame format from IEEE Std 802.11e-2005, Clauses 7.2.1.7, 7.2.1.8, and 9.10. IEEE Std 802.11-2024 Tables 9-36 and 9-37 reserve type 0 and assign type 2 to Compressed. Basic support is an INET compatibility profile. It establishes neither complete IEEE Std 802.11e-2005 conformance nor current-standard Basic support.

Basic retains its fragment bitmap and existing PHY choices. The shared code contains Compressed frame classes and HT admission predicates. Those components do not establish a production Compressed exchange because HCF rejects that profile before operation.

The lifecycle document identifies the current INET teardown and recovery choices. General Basic DATA admission and complete NAV conformance remain outside the supported claim. A-MPDU, implicit BAR, delayed Block Ack, Multi-TID Block Ack, and complete VHT transfer behavior are also outside this implementation.

## 3. Current source path and evidence limits

HCF owns coordination. The originator and recipient agreement handlers own role-specific state by peer and TID. The QoS data service selects DATA in queue service order. `InProgressFrames` holds extracted frames and exposes borrowed preview before extraction.

The ACK policy groups outstanding DATA by peer and TID within each accepted transmit window. HCF can request channel access for a due BAR without another DATA candidate. A matched response resolves the selected group. The agreement object owns cyclic sequence distance, bitmap membership, and transmit-window advancement.

Hypothetical example: the accepted window contains one sequence position and the BAR threshold is five. One outstanding DATA frame fills that window. The next candidate lies outside it, so HCF selects BAR despite the threshold. A positive response removes the acknowledged DATA and permits window progress. If no DATA remains at the safe exchange boundary, HCF starts completion DELBA.

The [S4 verification record](ieee80211-compressed-blockack-s4-execution.md) covers the current final source. It does not establish a test result for every intermediate commit, every protocol fixture, or every configuration.

### R1: publish production rate state

The MIB owns local, BSS, and peer rate and HT facts. Management publishes coherent state through the combined MIB commit operations. Rate consumers read that committed state. Association reset uses a separate management-to-MAC call because an unchanged capability value does not identify a new association. The [reset contract](ieee80211-blockack-agreement-lifecycle.md#reassociation-consumers-and-callback-order) explains the cleanup order.

## 4. DATA selection and BAR operation

Preview borrows a candidate without queue removal or sequence assignment. For an unextracted candidate, the data service reports the prospective sequence number. The original frame retains its header and queue ownership. The [preview interface](../../src/inet/linklayer/ieee80211/mac/contract/IOriginatorQosMacDataService.h) exposes these operations.

Below `blockAckReqThreshold`, the ACK policy defers a group's BAR only when the next eligible DATA belongs to that group and fits its window. An empty queue, another peer or TID, or an out-of-window candidate can therefore make BAR due. Counts from different groups do not combine to meet the threshold.

The frame sequence selects BAR before DATA extraction. Basic Block Ack response acceptance checks the local receiver, requested peer, and requested TID. Transmitted-BAR processing limits acknowledgment-state changes to the request's peer, TID, and bitmap range. The response starting sequence number alone does not advance the transmit window.

BAR collision and failure update EDCA contention state without a DATA retry entry. Failed Block Ack feedback permits DATA retransmission with Retry set. Resolution and lifetime disposal permit the agreement window to advance, without a jump over unresolved DATA.

## 5. Configuration and compatibility

| Setting or contract | Current behavior |
| --- | --- |
| `isBlockAckSupported` | Defaults to false. |
| `blockAckVariant` | Defaults to `"basic"`; `"compressed"` fails initialization. |
| `msduLifetime` | Defaults to 0.512 seconds per EDCAF and requires a positive finite value. |
| `addbaFailureTimeout` | Defaults to 100 ms after request acknowledgment; it does not bound contention or DELBA recovery. |
| Completion DELBA | Applies at a safe exchange boundary after first DATA transmission, with no queued or numbered DATA. |
| Originator teardown | Discards transmitted unresolved DATA and preserves sequence continuity for surviving unsent DATA. |
| Recipient teardown | Delivers complete buffered units before resource release. |
| Association reset | Discards old peer DATA and receive state without delivery into the replacement association. |

Custom QoS data services must expose borrowed preview and prospective sequence lookup. Custom BAR policies must accept explicit agreement context. Omitted context arguments fail compilation. The [migration section](../../doc/src/migration-guide/development/ieee80211-basic-block-ack-extension-points.rst) defines the extension obligations.

### Selected-extraction contract

The data service uses ordinary dequeue for individual extraction of the advertised next frame. Filtered `canPullPacket()` and `pullPacket()` operations permit eligible DATA behind a deferred frame while the queue owner applies normal pull effects. Aggregation validates candidate eligibility and A-MSDU permission before extraction. Invalid aggregate candidates cause individual extraction of the selected eligible frame.

This implementation uses the current queue API. It defines no separate optional selected-extraction interface. External queue implementations must preserve service order, ownership, residence metadata, pull events, and compound accounting through their queue operations.

## 6. Verification and scope limits

The [verification record](ieee80211-compressed-blockack-s4-execution.md) identifies the build, module, unit, protocol, and fingerprint results. Its commands and limits define the evidence scope. The fixture inventory is not itself proof that every fixture ran.

Basic single protection reserves only prepared successors. Prepared fragments can continue within the applicable exchange bounds. An unextracted successor starts another TXOP. Final zero-limit DATA reserves its immediate required response. Multiple protection retains its successor policy. These rules can increase contention and setup cost.

The implementation does not supply a general complete-cost admission result for Basic DATA or complete NAV correctness. Current checks establish no general throughput, goodput, latency, or agreement-reuse cost result. MAC stop/start recovery also remains outside the lifecycle contract.

## Capability gaps

The checkout lacks production Compressed setup and exchanges, a Compressed default, and their integrated rate and timing evidence. Compressed-specific fragmentation restrictions, full-state receipt behavior, profile-conflict diagnostics, and end-to-end recovery therefore have no production support claim here. Existing helper coverage cannot replace those missing paths.
