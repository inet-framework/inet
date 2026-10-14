# Implementation plan: enforce EDCA DATA lifetime

Original source observation revision: `feat/ieee80211-txop-single-protection` at `b06e8768b0e9c78b7a3665b9c69b0e0da9f30e41`.
Current integration source: local `master` at `633b071e213b825ce6019ff3e344ae49f52791ff`, with S1, R1, and S3a merged.
Plan revision: 2026-10-15, revision 11.
Source basis: IEEE Std 802.11-2024 and the revisions above. Source observations do not establish runtime results.

Approval: the maintainer's 14 October 2026 instruction, "proceed", after the revision-24 review authorizes the earlier document corrections. The maintainer's later explicit decision accepts the proposed 0.512-second DATA lifetime default. This decision selects the default and authorizes its record in the plan. These instructions authorize no implementation, recorded expectation change, or publication.

The [completed S1 record](ieee80211-compressed-blockack-replacement.md#completed-s1-and-recorded-verification) supplies Basic recipient capacity and ordered delivery. PR #1320 is merged in the integration source. Its original feature-head evidence establishes no fresh runtime result for main or S2 expiration. Lifetime observations retain `b06e8768` until the separate S2 review refreshes them against its selected parent.

Revision 11 updates the integration pin for merged R1 and S3a. Their completion does not establish S2 completion. The [umbrella source record](ieee80211-compressed-blockack-replacement.md#source-base-and-publication-boundaries) distinguishes the integration source from this checkout's separate S2 implementation and local edits. This review adds no runtime result or implementation authority.

## 1. Core intended behavior

HCF, the hybrid coordination function, will enforce a finite lifetime for all QoS DATA. This lifetime applies with either value of `isBlockAckSupported`. EDCA means enhanced distributed channel access. An EDCAF is an EDCA function that serves one access category.

An MSDU is the data unit that the MAC receives from its service user. An A-MSDU combines several MSDUs inside one MPDU. An MPDU is a MAC frame with its header and frame check sequence.

This prerequisite supplies the disposal bound for [Block Ack agreement recovery](ieee80211-blockack-agreement-lifecycle.md) and [Compressed Block Ack retransmission](ieee80211-compressed-blockack-replacement.md). It also applies to Normal Ack DATA without an agreement. A Block Ack capability switch will not select whether DATA has a lifetime.

Hypothetical example: HCF accepts QoS DATA at 0 s with the default 0.512-second lifetime. Contention prevents transmission before 0.7 s. HCF discards that DATA at its deadline, with Block Ack support either enabled or disabled. DATA that Tx already accepted follows the safe-completion rule below.

DCF lifetime and complete management-frame lifetime remain outside this change. The current MAC stop and crash handlers do not reset all child state. This plan makes no stop/start recovery claim.

## 2. Current source and standard support

[EdcaTransmitLifetimeHandler](../../src/inet/linklayer/ieee80211/mac/lifetime/EdcaTransmitLifetimeHandler.cc) stores start times by sequence number. Two peers can therefore share one key. Its current start point excludes queue residence. EDCA does not construct or call this handler in the inspected source.

[Edcaf.ned](../../src/inet/linklayer/ieee80211/mac/channelaccess/Edcaf.ned) has no `msduLifetime` parameter. HCF and the QoS data service own the entry and replacement paths that must supply lifetime identity.

IEEE Std 802.11-2024 Clause 10.23.2.12.1 requires the relevant QoS transmit timers. Its scope does not depend on Block Ack support. Clause 10.25.6.7.1 supplies the related Block Ack retransmission conditions. Their normative corpus identifiers are `ieee80211-2024:clause:10.23.2.12.1` and `ieee80211-2024:clause:10.25.6.7.1`. Physical PDF pages are 2016–2017 and 2068–2070.

Normative Annex C.3 defines `dot11EDCATableMSDULifetime` for non-AP stations and `dot11QAPEDCATableMSDULifetime` for APs. Both attributes use TU and default to 500. One TU is 1024 microseconds, so 500 TU equals 0.512 seconds. Their physical PDF pages are 5418 and 5420. The canonical node is `ieee80211-2024:clause:C.3`; source offsets are 22781782 and 22790375. Corpus text supplied both values without PDF inspection; the retrieved Clause 10.23.2.12.1 reference to Clause 10.3.2.11 resolved.

The implementation must reproduce missing queue expiration through production HCF before it changes that path. Source inspection establishes the absent integration; it does not establish a measured loss or delay.

## 3. Proposed ownership and behavior

The current `EdcaTransmitLifetimeHandler` will own deadlines for QoS DATA regardless of HCF Block Ack support. EDCA will construct this owner from a positive, finite `msduLifetime` parameter on each EDCAF. The accepted default is 0.512 seconds, which matches the standard's 500-TU defaults for AP and non-AP stations. The compatibility record below identifies its acceptance and effect. HCF will register DATA when `processUpperFrame` accepts it, before queue insertion or sequence assignment. The deadline therefore includes queue residence and setup deferral.

Management exchanges retain their existing retry procedure. This change does not claim complete management-lifetime support.

The parameter uses seconds and permits other positive, finite values within the supported simulation-time range. Initialization will reject zero, negative, and nonfinite values. Each configured value supplies a DATA deadline; it does not guarantee prompt recovery or expiry within the simulation duration. A large value can postpone lifetime disposal beyond the end of a run. This plan provides no unlimited mode because unresolved DATA must retain a deadline when no earlier disposal or successful exchange resolves it.

For compatibility studies, users can select a finite lifetime longer than their simulation duration. Hypothetical example: `**.mac.hcf.edca.edcaf[*].msduLifetime = 100s` prevents lifetime expiry for newly accepted DATA during a 10-second run. This proposed setting does not promise unchanged fingerprints or complete legacy behavior. Response acceptance, teardown, queue loss, and terminal failure can still resolve or discard DATA earlier. A recovery path that relies on lifetime expiry inherits the configured delay.

### Lifetime compatibility decision

The standards basis establishes the timer's scope and the 500-TU default. It does not measure the effect on current INET simulations. New disposal can occur with Block Ack disabled when retained DATA reaches its deadline. Congestion alone does not prove that a retained frame reaches that age.

On 14 October 2026, the maintainer explicitly accepted the proposed 0.512-second default. The value matches the 500-TU standards basis and supplies a finite disposal deadline. This acceptance includes the proposed scope with Block Ack disabled. It permits new disposal when retained QoS DATA reaches its deadline. The compatibility decision before L1 is satisfied. Source review, production reproduction, and measured expiry remain required by S2.

An opt-in or unlimited compatibility mode would require a separate design. That design must state its standards limits and supply another finite disposal bound wherever Block Ack recovery requires progress. This plan retains a finite lifetime and the long-finite compatibility setting above.

### Deadline ownership and safe completion

The handler will key retained deadlines by local packet identity, not sequence number. The QoS data service will transfer this identity information when it replaces source frames with an A-MSDU or fragments. Fragments share their source MSDU deadline. Each MSDU addition restarts the aggregate timer under Clause 10.23.2.12.1. Retransmission and ACK-policy selection will not restart a deadline. Queue discard, acknowledgment, teardown, and peer cleanup will remove the corresponding lifetime entries.

The [current aggregate builder](../../src/inet/linklayer/ieee80211/mac/aggregation/MsduAggregation.cc) adds its inputs synchronously at one simulation time. Its aggregate deadline therefore uses that construction time and the applicable AC lifetime.

Hypothetical example: an MSDU arrives at 0 seconds and joins an A-MSDU at 0.4 seconds under Normal Ack. This example has no active Block Ack agreement and no pending setup or teardown for that peer and TID. With a 0.512-second lifetime, the aggregate deadline is 0.912 seconds. The original 0.512-second deadline must not discard that aggregate. Queue expiry must remove already expired inputs before aggregation; aggregation cannot revive them.

This repair will add narrow operations to the concrete EDCA lifetime owner. It needs no change to DCF's lifetime interface. HCF will create one timer for the earliest DATA deadline across its EDCA queues and release it at destruction. Queue owners will remove expired DATA and its ACK state before further selection. HCF will also check the deadline before submission to Tx, including any proposed IFS delay. No DATA transmission may start after that deadline.

An accepted transmission can complete after its deadline; HCF will retain the referenced frame until its current exchange completes. Expiration will prohibit another attempt and trigger disposal at that safe boundary. HCF will not delete a frame that Tx or the frame sequence still references. The lifetime owner will exclude that expired retained entry from its next timer selection. A successful response to the accepted attempt can still acknowledge it.

Hypothetical example: every response reports DATA 100 as absent but acknowledges later DATA. DATA 100 remains eligible until its original deadline, even while successful Block Ack responses refresh agreement inactivity. At expiration, HCF discards DATA 100 and advances the transmit window from resolved status. Later DATA or BAR can advance the recipient past that gap. No inactivity refresh can extend DATA 100's lifetime.

## 4. Separate implementation stages

| Stage and purpose | Responsible component and change | Expected result and direct verification |
| --- | --- | --- |
| L1. Establish lifetime identity. | The lifetime handler stores local packet identities. The QoS data service transfers deadlines across frame replacement. Add the EDCAF parameter with unit seconds and default 0.512 seconds. | Two peers with identical sequence numbers retain separate deadlines. Fragments share the source deadline. A-MSDU construction follows the specified lifetime rule. Invalid lifetime values fail at initialization. |
| L2. Enforce expiration through HCF. | HCF registers DATA before queue insertion. Queue owners remove expired DATA. HCF prevents another attempt after expiration and preserves accepted exchanges until completion. | Production tests expire queued Normal Ack DATA with both Block Ack switch values. Expiration during an accepted exchange causes no premature deletion or later retry. |
| L3. Integrate consumers and document compatibility. | Agreement recovery and Compressed retransmission consume the same deadlines after their respective stages exist. Update user documentation and migration guidance. | Deferred setup and repeated Block Ack failures cannot extend a deadline. Ordinary EDCA users can select a suitable lifetime without enabling Block Ack. |

L1 and L2 form the publishable lifetime change in umbrella stage S2. Their commits must build and expose no unused public contract. L3 integration tests accompany the consumers that introduce those paths; they do not block standalone Normal Ack verification.

The lifetime compatibility decision for L1 is accepted at 0.512 seconds. Before S2 starts, refresh the older source observations and caller assessment against its pinned parent. The [umbrella decision table](ieee80211-compressed-blockack-replacement.md#maintainer-decisions) records that acceptance and the independent stage boundaries.

Finite lifetime changes Block Ack-disabled EDCA simulations as well as Block Ack-enabled simulations. Affected callers include the single-protection path, the QoS and aggregation showcases, and the `MacQos` examples. Queue residence can now cause disposal before any transmission. User documentation must state the default, parameter location, and the compatibility setting and limits above. It must distinguish aggregation's timer restart from retransmission and ACK-policy changes, which do not restart the deadline. No number of changed fingerprints is assumed before measurement.

## 5. Verification and evidence

Add a proposed `tests/module/Ieee80211EdcaDataLifetime_1.test` for production expiration and safe completion. Add focused helper assertions for identity transfer only where the production fixture cannot expose the necessary state.

Use these direct cases:

- Check the 0.512-second default for AP and non-AP EDCA. Reject zero, negative, and nonfinite values. Confirm that another positive, finite value controls expiration.
- Accept DATA before queue insertion. Hold channel access beyond its deadline. Check disposal without transmission for both Block Ack switch values.
- Give two peers the same TID and sequence number. Expire one frame. Check that the other frame retains its own deadline.
- Aggregate DATA. Fragment separate DATA. Check the resulting deadlines against Clause 10.23.2.12.1.
- A-MSDU extension: Use a peer and TID with no active Block Ack agreement or pending setup or teardown. Accept an MSDU at 0 seconds with a 0.512-second lifetime. Aggregate it under Normal Ack at 0.4 seconds. Hold the aggregate without transmission. Check retention past 0.512 seconds and expiry at 0.912 seconds.
- Expired aggregation input: Expire an input before aggregation. Check that the expired input cannot enter the aggregate.
- Long finite lifetime: Select a 100-second lifetime for a 10-second run. Check that newly accepted DATA does not expire through lifetime during that run. Check earlier acknowledgment and disposal paths separately. Do not treat a matching delivery result as proof of unchanged fingerprints.
- Expire DATA before a proposed IFS ends. Check that HCF rejects submission before transmission can start after expiration.
- Expire DATA during an accepted exchange. Permit its completion. Check that HCF retains references safely and prohibits another attempt.
- Dispose of queued DATA. Acknowledge other DATA. Reset a peer. Check removal of lifetime entries without changes to another peer.

The [replacement plan's verification scope](ieee80211-compressed-blockack-replacement.md#6-verification-and-scope-limits) supplies the common regression selection and the current S1 recipient fixture. Retain all eight Basic recipient scenarios as regression checks. Include the Block Ack-disabled EDCA rows because this prerequisite changes their supported behavior. Compare each stage with its pinned parent before any baseline proposal. S1's recorded 12-configuration result does not verify S2's proposed 19-row selection or its measured expiration requirement.

Measure delivered DATA, lifetime disposal, queue delay, and completion delay under controlled congestion. Keep traffic, seed, rates, and channel conditions identical across the comparison. Record the lifetime value and Block Ack switch for each run. A short run without expiration cannot prove this behavior.

S2 requires at least one Block Ack-disabled congestion case that demonstrates the default lifetime boundary. Select a common regression row only if measured frame ages establish that condition. Otherwise, add a controlled congestion variant with its parameters and parent comparison recorded separately. Record each affected frame's MAC acceptance time, disposal time, age, and disposal cause. Separate lifetime disposal from queue-capacity loss and retry-limit loss.

The [QoS showcase](../../showcases/wireless/qos/omnetpp.ini) offers approximately 57.7 Mbps of application payload against a configured 54 Mbps radio rate. Its [pending queues](../../src/inet/linklayer/ieee80211/mac/queue/PendingQueue.ned) default to 100 frames. That offered load does not prove that a retained frame reaches 0.512 seconds of age. The [MacQos example](../../examples/wireless/qos/omnetpp.ini) selects unlimited pending queues, but this setting alone does not prove sufficient delay. Neither configuration supplies measured expiry evidence in this plan.

Compare the parent without expiration, the 0.512-second default, and an explicit one-second setting under the same congestion. Hypothetical example: unaggregated DATA waits 0.7 seconds before channel access. The default discards it; the one-second setting still permits an attempt. This comparison measures the compatibility cost without an unlimited production mode.

Follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md) for the required build and focused test commands. Record commands, modes, selected cases, and results when implementation exists. This revision adds no execution result. The separate S1 record owns completed recipient verification; lifetime and performance results remain unverified.
