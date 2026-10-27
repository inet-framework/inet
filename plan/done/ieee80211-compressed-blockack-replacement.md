# Basic and Compressed Block Ack implementation status

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc), [HCF parameters](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.ned)

This checkout implements explicit immediate Compressed Block Ack and selects it by default. Eligible HT links use Compressed agreements. Links without usable local or peer HT state use Normal Ack. Explicit Basic retains its compatibility profile.

The [summary](ieee80211-compressed-blockack-replacement-summary.md) describes the supported exchange. The [verification record](ieee80211-compressed-blockack-s5-execution.md) defines the final-commit verification scope and its limits.

## Previous PRs and S4 scope

Master contains stage S4 through merged [PR #1344](https://github.com/inet-framework/inet/pull/1344). S4 supplies borrowed DATA preview, BAR-first arbitration, BAR-only contention, peer/TID-specific feedback, shared window operations, DATA Retry, and completion DELBA. These mechanisms are prerequisites from master, outside the S5 commit series.

Master also supplies recipient progress through PR #1320, finite QoS DATA lifetime through PR #1331, and agreement lifecycle through PR #1343. PR #1368 supplies peer cleanup before association replacement. S5 uses the same state owners and the filtered queue contract.

## Supported behavior and configuration

Block Ack support remains optional through `isBlockAckSupported`, which defaults to false. `blockAckVariant` defaults to `"compressed"`. Both endpoints must select the same profile because ADDBA does not negotiate this selector. A valid addressed BAR with another profile stops the simulation before agreement updates. The diagnostic identifies the peer, TID, and profiles.

Compressed operation requires usable local HT support and directional peer HT state. An HT PHY mode alone does not supply that state. Without it, HCF uses Normal Ack. Select `mac.hcf.blockAckVariant = "basic"` at both endpoints for Basic compatibility. Basic retains its fragment bitmap and existing non-HT and ad hoc configurations.

Compressed supports immediate Block Ack only. Keep `delayedAckPolicySupported = false` in both agreement policies. HCF rejects either true value at initialization when Compressed support is enabled. Active Compressed agreements suppress fragmentation for their peer and TID. Individual MPDUs and permitted A-MSDU remain supported.

## Receipt state and delivery

The recipient agreement selects a complete-MPDU receipt record with a 64-position window. Positive receipts survive delivery cleanup. Absent receipts remain zero. The reorder service separately controls delivery. A positive receipt therefore does not itself prove upper delivery.

For example, DATA 100 and 102 arrive while DATA 101 is absent. A BAR requests feedback for that window. The response reports bits `1, 0, 1`. The originator retries 101. Delivery can release 100, 101, and 102 without loss of the positive receipt evidence.

Rx classifies SIFS and permitted RIFS from local endpoints and decoded frame information. It attaches a result to each frame that passes FCS. HCF passes that result to the recipient service. Missing or rejected interval evidence supplies false without rejection of a valid frame or loss of positive receipt evidence.

The SIFS detector supports coverage class zero. Its inclusive window uses SIFS plus or minus ten percent of the selected slot time. It adds no distance-derived allowance. After a local ACK, five-microsecond one-way propagation produces a twenty-microsecond gap for a peer that waits ten-microsecond SIFS. This gap supplies no SIFS evidence. A newer BAR can release a frame that remains behind a sequence gap.

RIFS requires permitted HT operation, the same transmitter and RA, and no preceding SIFS response obligation. VHT operation clears RIFS permission. This recipient behavior adds no RIFS transmission scheduler. HT duration at 2.4 GHz includes its six-microsecond signal extension.

## Wire format and control rates

The serializer decodes all four Type bits. Type 0 supplies Basic compatibility; Type 2 supplies Compressed. Compressed responses contain a 64-bit bitmap and occupy 32 bytes with FCS. Basic responses occupy 152 bytes. Both BAR profiles occupy 24 bytes with FCS. Invalid Types and malformed fragment fields fail validation.

Compressed BAR uses a legal non-HT rate from the applicable local, BSS, and peer sets. Initial and later BAR can have different rate constraints. A configured `controlFrameBitrate` must satisfy those constraints. Response duration, protection, and timeout calculations use the actual Compressed response length.

## DATA selection and BAR operation

The ACK policy counts outstanding DATA separately by peer and TID within the accepted transmit window. Below `blockAckReqThreshold`, it defers BAR only when the next eligible DATA belongs to that group and fits the window. An empty queue, another group, or an out-of-window candidate can make BAR due. HCF can obtain channel access for BAR without another DATA frame.

Hypothetical example: the accepted window has one position and the threshold is five. One outstanding DATA frame fills the window. The next candidate cannot proceed, so HCF selects BAR. Positive feedback resolves the DATA and permits window progress. When no queued or numbered DATA remains, HCF starts completion DELBA at a safe exchange boundary.

The [lifecycle contract](ieee80211-blockack-agreement-lifecycle.md) defines setup, Action dispatch, teardown, and peer reset. The [lifetime contract](ieee80211-edca-data-lifetime.md) defines finite DATA deadlines. S5 retains those owners and their completion rules.

## DATA admission and ownership

Active Compressed agreements use an owned preview while original MSDUs remain queued. HCF selects one DATA mode and evaluates complete exchange cost. If an aggregate cannot qualify, HCF evaluates its first ordinary MSDU with the same mode. If neither form qualifies, the sequence ends without queue removal, deadline replacement, or sequence assignment.

The data service validates source identities, eligibility, deadlines, sequence state, and A-MSDU permission. HCF validates retained mode and response inputs before RTS and before DATA submission. Commitment transfers the same preview pointer only before first DATA submission. Ordinary commitment preserves the original deadline and normal queue service effects. Aggregate commitment starts the A-MSDU lifetime.

For example, DATA that waits two milliseconds retains that queueing time and one departure after ordinary commitment. A failed RTS leaves original DATA queued. Recovery updates the access-category short retry counter and contention window without a provisional sequence counter. Original deadlines bound repeated attempts.

Single protection reserves only the current exchange. A later BAR can require fresh contention. A permitted initial ordinary MSDU or same-size retransmission can exceed a positive TXOP limit under its applicable exception. An oversized first-attempt A-MSDU cannot use the retransmission exception. The [admission contract](ieee80211-compressed-blockack-txop-admission.md) defines the event boundaries and comparisons.

## Extension contracts

Custom QoS data services must supply borrowed preview, prospective sequence lookup, and reversible preparation. Queue owners perform normal dequeue or filtered pull when commitment occurs. Custom rate selectors, recipient ACK policies, recipient data services, and frame sequences must implement the Compressed contracts.

Custom Rx implementations must support explicit history invalidation. Custom radios must supply usable local reception and completion indications for interval evidence. IEEE 802.11 modes supply nominal extension; the radio derives actual received extension. The [migration guide](../../doc/src/migration-guide/development/ieee80211-compressed-block-ack-extension-points.rst) defines these obligations. Basic classes and methods remain available.

## Excluded features and fragmentation

A-MPDU, implicit BAR, delayed Compressed Block Ack, and Multi-TID Block Ack remain outside this implementation. Basic is an INET compatibility profile for IEEE Std 802.11e-2005. IEEE Std 802.11-2024 reserves Type 0 and assigns Type 2 to Compressed. Basic retention establishes neither complete historical conformance nor current-standard Basic support.

General Basic DATA admission, admission outside active Compressed agreements, complete NAV conformance, and complete VHT transfer conformance remain outside the claim. MAC stop/start recovery and general performance results also remain outside it. Tests must distinguish those paths from active Compressed admission.

The standards basis includes IEEE Std 802.11-2024 Tables 9-36 and 9-37, Clauses 10.25.6.3 and 10.25.6.6.2.1, and Clause 10.23.2.9. The verification record separates direct fixture coverage from unverified configurations. The fixture inventory alone establishes no executed result.

## Protocol verification by commit

The [protocol verification plan](ieee80211-compressed-blockack-s5-protocol-tests.md) links one reviewer document from each S5 commit. It records direct before/after protocol comparisons where applicable. It also explains the shared preparation and memory-ownership boundaries that require other evidence.
