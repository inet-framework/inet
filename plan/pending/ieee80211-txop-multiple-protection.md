# Implementation plan: multiple protection in an EDCA TXOP

Implementation base: recipient NAV prerequisite `cc17ef62f4`, based on `e04a0113b383d55e5b6dbbd4dde75d825a6f2756`.
Plan revision: 2026-10-07, revision 19.
Timing: Nominal timing uses the mode set's short interframe space (SIFS) unchanged after the first transmission step. The first step adds no interframe space.
Status: nominal timing implementation is complete. Section 8 records verification for this revision.
Authorization: on 2026-10-07, the user requested removal of aligned timing from the current branch and PR #1301. The request includes simpler source, documentation, this plan, and its summary. The request authorizes publication of the revised branch and PR description. It does not authorize new fingerprint baseline values.
Contract check: source inspection confirms the owners, callers, frame sequence, and completion paths. The source paths are unsealed. The change removes the timing selector, timing state, IFS helper, and policy branches in tests. HCF uses the frame sequence's nominal IFS directly. Endpoint arithmetic, RTS phase refusal, deadline checks, and expired-reservation behavior retain their nominal results.

Dependencies: the independent [recipient NAV fix](ieee80211-recipient-nav.md). Its commit owns the Tx contract, caller choices, migration instructions, and all 24 causal fingerprint rows.
Evidence: source inspection and local IEEE Std 802.11-2024 retrieval.
Verification: section 8 records executed checks and exact fingerprint differences. Revision 19 changes explanatory text only; runtime source and tests retain revision 18 content.

The design uses the current transmission path and the vocabulary in [CONTEXT.md](../../CONTEXT.md).

## 1. Intended behavior and scope

A station will select multiple protection through configuration. EDCA means enhanced distributed channel access and gives access categories different channel access parameters. A TXOP is a transmission opportunity: an interval in which a QoS station can start frame exchange sequences. Multiple protection uses Duration/ID to reserve medium time for a sequence of frames. Other stations use that field to set their NAV, the MAC state that records a reservation of the medium.

Single protection remains the default. The holder selects one protection class at TXOP start and retains it until TXOP end. For a positive limit, multiple protection selects the standard's upper duration bound. It therefore needs no estimate of every queued frame. The holder retains nominal SIFS. HCF stops before a later exchange requires a field with an empty raw duration interval.

For example, consider a hypothetical TXOP with a 1,024 µs limit. Its first DATA PPDU takes 200 µs. The multiple protection field is 824 µs. A nonparticipating station that receives that DATA reserves the medium until the advertised endpoint. Later holder frames calculate their fields from the time left.

For a zero limit, the first field estimates the exchange selected by the existing sequence. This includes its selected data or management frame, required response, and optional RTS/CTS. The estimate uses the modes available at calculation; section 4.4 explains possible later mode changes. The current sequence stops repetition after that exchange. An unrelated queued unit needs another grant.

The change covers existing data, management, group traffic, optional RTS/CTS, and immediate Basic BAR/Block Ack. BAR requests Block Ack, which acknowledges a set of MPDUs. An MPDU is one MAC frame. An MSDU is a data unit that the MAC receives from above. Existing conventional fragments and A-MSDU use the same calculation when the current path selects them. An A-MSDU contains multiple MSDUs within one MPDU.

The standard's TXNAV timer records the holder's local transmitted reservation and is shared by the station's EDCAFs. An EDCAF manages contention for one access category. The following procedures remain outside scope:

- Full TXNAV behavior shared by all EDCAFs, strict continuation against TXNAV, complete exchange admission, and TXOP overrun repair.
- Prepared exchanges, queue snapshots, retained mode plans, and request cancellation.
- New fragmentation procedures, A-MPDU, and Compressed Block Ack.
- CTS-to-self, dual CTS, CF-End, PIFS recovery, and TXOP sharing.
- Reverse direction, beamforming, S1G procedures, and multi-user exchanges.
- General rate selection, recipient response timing, and MAC lifecycle repair.

Some excluded procedures require multiple protection in the standard. The configuration switch does not enable them.

The current group sequence sends one selected group frame, even with a positive limit. Multiple protection changes its duration field but does not add group repetition. For example, a hypothetical group PPDU takes 200 µs within a 1,024 µs limit. It advertises 824 µs, requires no ACK, and ends the TXOP. Another queued group frame needs a new grant; other stations retain the advertised reservation.

The current admission policy can start a 120 µs exchange when only 100 µs remains. This change does not repair that policy or establish a complete TXOP duration guarantee. Nominal holder SIFS remains unchanged.

A queue that becomes empty before the advertised endpoint leaves unused reservation time. For example, a hypothetical burst ends at 400 µs within a 1,024 µs reservation. Other stations retain the reservation until its endpoint because this change adds no CF-End. Each positive-limit group transmission leaves its entire `L - P` reservation unused under the current one-frame sequence. The group example above therefore leaves 824 µs unused after every transmission. That reservation blocks other stations and the holder's other EDCAFs until expiry; user documentation must state this cost.

## 2. Initial behavior and responsible components

The table describes behavior before the recipient NAV prerequisite and this feature. Source inspection establishes the call paths. Section 8 records the production assertions and their results.

IFS means interframe space: a required interval between specified frame transmissions. SIFS means short interframe space. This interval separates specified immediate responses and frames within an exchange.

| Component | Initial behavior | Required responsibility |
| --- | --- | --- |
| [TxopProcedure](../../src/inet/linklayer/ieee80211/mac/originator/TxopProcedure.cc) | The enum contains both classes, but the selector returns single protection. The owner retains that class for the active TXOP. | Retain the selected protection class and one endpoint. |
| [Hcf](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) | It selects ACK policy and mode before transmission. It rejects multiple protection. | Calculate the field with the frame sequence's nominal IFS after mode selection. Record the endpoint in its completion callback. |
| [FrameSequenceContext](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.cc) and [FrameSequenceStep](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceStep.h) | The first step uses no IFS. Later steps use SIFS. An RTS step retains its protected frame. | Supply the existing IFS and protected frame identity. |
| [TxOpFs](../../src/inet/linklayer/ieee80211/mac/framesequence/TxOpFs.cc) | It selects Normal Ack, Block Ack policy data, Basic BAR/Block Ack, and management branches. | Retain the existing exchange composition. |
| [HcfFs](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) | Its continuation predicate permits another exchange while the TXOP has time left. | Check the holder endpoint, every later start against the deadline, and the phase of a later RTS. Decline the whole exchange through normal sequence completion when either check fails. |
| [InProgressFrames](../../src/inet/linklayer/ieee80211/mac/queue/InProgressFrames.cc) | Both the selected-frame query and the pending-frame query can extract queued units. | Preserve existing sequence selection. The new duration calculation and continuation checks add no queue extraction. |
| [Tx](../../src/inet/linklayer/ieee80211/mac/Tx.cc) | It copies the finalized frame, waits the supplied IFS, and reports completion. It also passes every transmitted duration to Rx. | Retain the caller's choice of local NAV update until completion. Recipient response callers disable that update. |
| [Rx](../../src/inet/linklayer/ieee80211/mac/Rx.cc) | Local transmitted durations and received reservations extend the same NAV timer. | Retain received reservations and the holder's local busy interval. |

The missing selection and duration calculation need no new sequence tree. One existing interaction does require a scoped change. A recipient's transmitted CTS or ACK extends its own NAV through Tx. [QosCtsPolicy](../../src/inet/linklayer/ieee80211/mac/recipient/QosCtsPolicy.cc) refuses another CTS while that NAV makes the medium busy. A longer multiple protection field therefore exposes this coupling during a later RTS to the same recipient.

## 3. Duration arithmetic

IEEE Std 802.11-2024, Clause 9.2.5.2 defines the four cases below. Clause 9.2.5.1 requires ceiling to whole microseconds and a zero value for a negative calculated field. Table 9-9 limits ordinary duration values to 0–32,767 µs.

Clause 9.4.2.27 encodes an advertised EDCA TXOP Limit in units of 32 µs. The examples use 1,024 µs, which that field can represent. A direct NED override can supply other values, but such an override does not establish valid advertised EDCA parameters.

Each calculation uses the projected PPDU start, `t = simTime() + ifs`. A PPDU is one PHY transmission with its PHY overhead. The frame sequence supplies nominal `ifs` to HCF. The first step has zero IFS. Tx schedules the supplied delay for later steps.

Clause 9.2.5.2 defines `E` as the remaining holder reservation and subtracts the current PPDU airtime. This subtraction places both `E` and `R` at PPDU start, before that airtime elapses. For example, an endpoint at 1,024 µs and a start at 176.4 µs give `E = 847.6us`. The 100 µs PPDU leaves 747.6 µs after transmission. If HCF measures `E` at PPDU end, it subtracts the airtime twice and incorrectly obtains 647.6 µs.

| Symbol | Meaning |
| --- | --- |
| `L` | Configured TXOP limit. |
| `R` | `max(0, txopStart + L - t)`, from the existing TXOP start and limit. |
| `E` | `max(0, reservationEnd - t)`, for the holder's transmitted reservation. |
| `P` | Current PPDU airtime from its selected mode and complete packet length. |
| `S` | Estimated complete cost of the selected zero-limit exchange. |
| `Q` | Pending transmission time defined by Clause 9.2.5.2, including required responses and IFSs. |
| `D` | Calculated field before conversion to whole microseconds. |

| Condition | Standard equation or interval | Selected duration policy |
| --- | --- | --- |
| `L = 0`, `E = 0` | `D = S - P` | Reserve the selected exchange. |
| `L = 0`, `E > 0` | `D = max(0, E - P)` | Use the established reservation. |
| `L > 0`, `E = 0` | `min(Q, L - P) <= D <= L - P` | Select `D = L - P` before the holder establishes an endpoint. The expired-reservation case below uses `R - P`. |
| `L > 0`, `E > 0` | `E - P <= D <= R - P` | Select `D = R - P`. |

The existing admission policy can leave a holder frame inside an exchange after the positive-limit reservation expires. INET must not restart a full-limit reservation in that case. HCF selects the initial positive-limit calculation only when the raw `reservationEnd` is zero. Once that endpoint exists, HCF uses `R - P`, even when `E` clamps to zero. Field conversion clamps a negative result to zero. This model choice preserves the admitted exchange without another full-limit reservation; it does not repair complete exchange admission.

For example, the production regression uses a 384 µs limit and zero propagation delay. The second RTS starts at 304 µs. Its 52 µs airtime and 28 µs field reserve the 384 µs deadline. CTS starts at 372 µs and ends at 416 µs with a zero field.

Protected DATA starts at 432 µs and uses 100 µs airtime. HCF gives DATA a zero field instead of a new 284 µs reservation. Nominal timing uses this result.

Clause 9.2.5.2(a)(9)(i) sets a final group frame's duration to zero under single protection. Within the declared EDCA scope, group frames under multiple protection use the separate equations in Clause 9.2.5.2(b). This plan retains `L - P` for an initial group frame with a positive limit. The sequence's early finish does not make that limit zero. A zero field would also require the positive-limit lower bound to permit zero; the group predicate alone does not establish that condition.

The positive-limit upper-bound policy removes the need to calculate `Q`. Keep durations at simulation precision during arithmetic. In the fourth case, compare the raw lower and upper bounds before field conversion. If the lower bound exceeds the upper bound, no raw value satisfies the interval. Section 3.1 defines the normal stop rule. Do not ignore a bound or compare separately rounded bounds.

Clause 9.2.5.1 supplies field conversion after calculation; it does not redefine the interval variables. Calculate `encodedMicroseconds = ceil(max(0, D) / 1us)`. For a holder field, reject an encoded result above 32,767. Return `SimTime(encodedMicroseconds, SIMTIME_US)` to the header setter. The result represents the specified count of microseconds.

The current [HR-DSSS duration function](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211HrDsssMode.cc) returns whole-microsecond airtime. Standard 20 MHz OFDM uses whole symbols of 4 µs, and the current HT duration function also uses whole-microsecond intervals. Fractional elapsed time can instead come from [ConstantSpeedPropagation](../../src/inet/physicallayer/wireless/common/propagation/ConstantSpeedPropagation.cc), which divides distance by propagation speed. Therefore, integer PHY airtime alone does not establish a nonempty interval.

For example, consider a hypothetical 1,024 µs TXOP with 100 µs DATA, 44 µs ACK, 16 µs SIFS, and 0.2 µs propagation each way. The first DATA advertises 924 µs and an endpoint at 1,024 µs. The second DATA starts at 176.4 µs; its 747.6 µs field becomes 748 µs and advertises an endpoint at 1,024.4 µs. At the third DATA start, 352.8 µs, the bounds are 571.6 µs and 571.2 µs. The raw interval is empty although the third exchange fits the TXOP budget.

The hypothetical example uses the existing propagation formula. The production regression checks the same frame times and fields. DATA #2 satisfies the raw interval before mandatory field ceiling. Its encoded field can exceed the fractional raw upper bound because Clause 9.2.5.1 requires that conversion. The conflict arises before DATA #3. Section 3.1 explains the normal stop for this example.

### 3.1 Nominal timing and normal stop

The frame sequence supplies the mode set's SIFS after the first transmission step. After an exchange completes, the continuation predicate compares `reservationEnd` with `txopStart + L`. If the endpoint exceeds that deadline, it declines the next exchange. This check applies to positive limits. A later RTS needs the additional check below, before its exchange starts.

The predicate also calculates `projectedStart = simTime() + context->getIfs()`. It rejects a later start at or beyond the deadline. For example, an ACK at 160 µs leaves 4 µs in a 164 µs TXOP. Nominal SIFS places the next DATA at 176 µs, so HCF stops normally before that exchange.

For a projected start before the deadline, `E > R` means that the holder endpoint exceeds the deadline. The interval remains empty when HCF subtracts the same `P` from both bounds. The predicate therefore needs no next-frame mode query or queue estimate. Derive the decision from the endpoint; retain no additional stop flag.

In the example, DATA #2 extends the endpoint to 1,024.4 µs. After its ACK completes, HcfFs declines DATA #3. HCF follows its existing normal sequence completion path and keeps that DATA available for later access. This stop reports no transmission failure and changes no retry counter. Rx retains its NAV timer; channel access cannot transmit another burst before the local reservation expires.

An RTS can create the same conflict before its protected DATA, within one exchange. The continuation predicate must therefore check a later RTS before transmission. For the declared legacy RTS modes, RTS airtime is an integer number of microseconds. Its projected start must lie on the grid `T + k * 1us`, where `T = txopStart + L`. Otherwise, ceiling of its upper-bound field advertises an endpoint beyond `T`.

For example, extend the hypothetical propagation case with a larger second DATA that requires RTS. Without the preventive check, RTS starts at 176.4 µs and takes 52 µs. Its raw field is 795.6 µs; the encoded field advertises an endpoint at 1,024.4 µs. CTS completes at the holder at 288.8 µs. At the protected DATA start, 304.8 µs, a 152 µs PPDU gives incompatible bounds of 567.6 µs and 567.2 µs.

Nominal timing declines this whole second exchange before RTS, although its complete exchange fits the TXOP budget. Because RTS never transmits, the reservation still ends at 1,024 µs. Rx retains that reservation after normal sequence completion and blocks other holder EDCAFs until its expiry. This differs from the DATA-only example, where DATA #2 transmits and extends the reservation to 1,024.4 µs.

The RTS phase check needs no RTS packet, mode query, or estimate of the remaining exchange.

Clause 10.23.2.8 permits further exchanges but does not require them. Apply the endpoint, deadline, and RTS checks between exchanges, before another holder transmission starts. Neither check may omit a required response or interrupt an exchange already in progress. The declared reference scope uses whole-microsecond positive limits, integer-airtime initial PPDUs, and legacy RTS modes with integer airtime. Fractional initial or RTS airtime requires separate treatment before this policy can claim support for those multi-step exchanges.

## 4. Implementation design

### 4.1 Select the protection class

`TxopProcedure.ned` supplies `string protectionMechanism @enum("single", "multiple") = default("single")`. TxopProcedure validates the value at initialization. TxopProcedure selects the protection class at TXOP start and retains it until TXOP end. Multiple protection always uses the frame sequence's nominal IFS.

The parameter belongs to the per-AC owner. For example, AC_VI can select multiple protection while AC_BE retains single protection. An invalid value causes an initialization error before transmission.

```ini
*.host.wlan[0].mac.hcf.edca.edcaf[2].txopProcedure.protectionMechanism = "multiple"
```

[Edca.ned](../../src/inet/linklayer/ieee80211/mac/channelaccess/Edca.ned) maps index 2 to AC_VI by default. Use the network's actual host and interface paths.

### 4.2 Retain one holder endpoint

Add one absolute `reservationEnd` value to `TxopProcedure`. Clear it at TXOP start and end. Derive `E` through the formula in section 3. This value records the reservation advertised by the holder; it does not record ACK success.

This scalar represents the holder reservation within one active TXOP for the duration equations. It is not the complete TXNAV shared by all EDCAFs, as Clause 10.23.2.2 defines. Standard TXNAV uses the latest successfully transmitted holder frame's Duration/ID and counts down from PPDU end. This scalar belongs to one AC, retains the maximum endpoint, and clears at TXOP end. Rx separately retains a shared NAV that combines local transmissions and received reservations. A later full TXNAV design must resolve this ownership and lifetime difference rather than add a duplicate holder endpoint.

In HCF's existing originator completion branch, update `reservationEnd = max(reservationEnd, simTime() + transmittedDuration)`. Use the finalized field from the frame supplied by Tx. Perform this update before the sequence handler advances. Do not update it for a calculation that precedes transmission or for a recipient response.

AIFS means arbitration interframe space, the interval that an EDCAF uses before contention can proceed on an idle medium. AIFSN means arbitration interframe space number: the number of slot times that supplements SIFS to determine AIFS. This ownership invariant needs only an effective AIFSN of at least 1 for every EDCAF that uses contention. [Edcaf](../../src/inet/linklayer/ieee80211/mac/channelaccess/Edcaf.cc#L62) computes `AIFS = SIFS + effectiveAIFSN * slotTime`, so AIFS exceeds SIFS by at least one slot. The default AIFSN values satisfy this condition.

Clause 10.23.2.4 requires AIFSN ≥ 1 for APs and AIFSN ≥ 2 for non-AP stations under normal EDCA. These protocol requirements apply independently of multiple protection. For example, a hypothetical non-AP station with AIFSN 1 satisfies the ownership proof but violates the standard's minimum.

[Edcaf.ned](../../src/inet/linklayer/ieee80211/mac/channelaccess/Edcaf.ned#L30) permits `aifsn = 0`, but the existing debug assertion requires AIFS greater than SIFS; release builds omit that assertion. This plan excludes effective AIFSN values below 1 and adds no general parameter validation. For example, hypothetical zero-backoff contention with AIFSN 0 can reach its grant time at SIFS, alongside the recipient response.

Under these conditions and the current supported EDCA path, HCF sends recipient responses only when no EDCAF owns the channel. Channel grants start the holder sequence synchronously; normal sequence completion releases ownership. While that sequence owns the channel, [HCF's receive path](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc#L185) routes received frames into the sequence rather than recipient processing. For example, a hypothetical RTS received while HCF awaits ACK stays on the sequence path and does not produce a recipient CTS.

The reception that triggers a response keeps the medium busy until it ends. [Contention](../../src/inet/linklayer/ieee80211/mac/contention/Contention.cc#L157) records both busy and free notifications in `lastChannelBusyTime`, despite that name. After reception ends, contention requires AIFS from that end, which exceeds the response's SIFS. Once the response starts, [Rx](../../src/inet/linklayer/ieee80211/mac/Rx.cc#L158) treats its transmission as busy. For example, a hypothetical reception ends at 100 µs with 16 µs SIFS and 34 µs AIFS. The response starts at 116 µs, before a zero-backoff grant could occur at 134 µs, so ownership remains consistent with the recipient role.

`Hcf::transmissionComplete()` selects this branch whenever an EDCAF owns the channel, including a group transmission. The sequence handler then calls `originatorProcessTransmittedFrame()`, whose multicast branch processes the group frame. Thus, the endpoint update also covers group frames before the sequence ends. For the 200 µs group example, completion records `reservationEnd = 200us + 824us = 1024us`. Normal TXOP completion subsequently clears that owner value while Rx retains the reservation.

The maximum retains reservations that existing NAV processing does not shorten. The value needs no timer because each query subtracts the current projected start. It also supplies the continuation decision in section 3.1, which prevents an empty duration interval rather than implements full TXNAV admission. Existing start and limit values cannot represent a zero-limit reservation. Rx's combined NAV cannot represent the holder endpoint because another station can extend that NAV.

For example, a hypothetical received reservation ends later than the holder's own reservation. Rx retains that received reservation for medium access. The holder's field calculation still uses only `reservationEnd`. When TxopProcedure clears that value after an early finish, it must retain Rx's NAV timer.

### 4.3 Calculate directly in HCF

HCF supplies protected helpers for duration arithmetic and the zero-limit estimate. A local conversion function handles holder and response fields. No holder IFS helper, timing enum, or timing parameter is necessary. Unit tests expose the protected helpers through a test subclass.

HCF first executes its existing ACK policy and mode selection. The holder branch uses that selected mode for `P` and reads the updated header. The duration helper calculates the field for `simTime() + ifs`, where the frame sequence supplies nominal `ifs`. HCF sets the field before its existing Tx call with the same IFS. Positive-limit calculation needs no pending packet or queue inspection.

Tx schedules the supplied IFS through its existing path. The calculation adds no event, timer, or rate query. For example, the fractional propagation case retains its 16 µs holder gap and stops after the second exchange.

In `HcfFs::hasMoreTxOps()`, apply the continuation checks after the first exchange and before another exchange starts. Read the endpoint through TxopProcedure's owner accessors in the existing QoS context. Keep the first-exchange rule and single protection branch. The group predicate already calls this function, but the current sequence refuses a second group frame. Existing sequence completion releases channel ownership; Rx's retained NAV still controls subsequent medium access.

The preventive RTS check reuses the next frame that this predicate already selects. First check whether the existing ACK policy requires BAR, because TxOpFs selects that branch before DATA. That read-only query preserves BAR precedence; the current BAR branch has no RTS. Otherwise, use the existing RTS policy on the selected frame to determine whether its exchange requires RTS. For RTS, calculate its projected start with the frame sequence's nominal IFS and test its phase against `T`.

For example, a large pending DATA might exceed the RTS threshold while outstanding frames require BAR first. The new predicate admits BAR under the endpoint check, without applying the DATA's RTS phase condition. After BAR completes, the predicate checks the selected DATA exchange before RTS. The checks reuse existing sequence selection and add no queue extraction. Neither check predicts the DATA mode.

### 4.4 Estimate the zero-limit exchange

A zero limit needs `S` when no holder reservation time remains at the projected start. This includes an initial step and a later step after an estimate expires. Estimate the exchange already selected by the current sequence. Use existing PHY duration and response-rate APIs. Keep temporary packet copies and calculated values local to the call.

| First holder frame | Estimated interval after that frame |
| --- | --- |
| DATA or management with Normal Ack | SIFS plus ACK airtime. |
| Group frame or Block Ack policy DATA without an immediate response | Zero. |
| RTS | SIFS, CTS, SIFS, the protected frame, and its immediate response when required. |
| Basic BAR | SIFS plus Basic Block Ack airtime for the existing immediate-response path. |

For RTS, obtain the protected frame from the current `RtsTransmitStep`. Do not call the pending-frame query that can extract another unit. For protected DATA, initialize the agreement pointer to null. Query the originator agreement handler only if that handler exists. Call `originatorAckPolicy->computeAckPolicy()` with the frame, its header, and that agreement. Use its result rather than the retained header's old Ack Policy value.

The default `Hcf.isBlockAckSupported = false` creates no agreement handler. The existing originator policy returns Normal Ack for a null agreement. For example, the default zero-limit RTS estimate includes DATA, ACK, and the required gaps without any agreement lookup. If configuration enables Block Ack, HCF can query the handler, but a matching agreement might not exist.

Place the predicted policy and mode on a local copy when existing response-rate queries require packet tags. For Normal Ack, include SIFS and the predicted ACK airtime. For Block Ack policy DATA, include no immediate ACK. Management uses its existing required ACK rule. Release the local copy before the calculation returns.

For example, a hypothetical retained DATA header still says Normal Ack when its active agreement selects Block Ack policy. The zero-limit RTS estimate includes CTS and DATA, but no ACK after DATA. With no such agreement, the estimate includes ACK and its preceding SIFS. Both cases use the existing originator policy without changes to real ACK state.

Clause 9.2.5.2 defines `S` as an estimated exchange cost. HCF still selects the actual protected-frame mode when that later step executes. The estimate uses the mode available during RTS calculation and does not retain or force it. Those two queries can return different modes, so the estimate need not equal the later exchange airtime. Tests use fixed compatible modes, nominal SIFS, and zero propagation delay for exact estimates.

Rate queries can change their owner's state. [AarfRateControl::getRate()](../../src/inet/linklayer/ieee80211/mac/ratecontrol/AarfRateControl.cc) queries the adaptive rate controller and resets an expired interval. It increases the rate only when a faster mode exists, as [RateControlBase](../../src/inet/linklayer/ieee80211/mac/ratecontrol/RateControlBase.cc) specifies. At the maximum rate, it retains that rate but still resets the interval. The existing single protection RTS estimate also queries the pending frame's mode.

Retain these query effects; a packet copy isolates packet changes, not rate-control changes. Do not clone or restore the rate owner to make prediction appear free of state changes.

For example, a hypothetical AARF interval expires before a zero-limit RTS estimate, and the current rate is below the maximum. The mode query increases the rate even if CTS later fails and DATA never transmits. The estimate must not roll back that increase or report a DATA transmission. A focused adaptive-rate failure case will compare this effect with master single protection.

For a distinct hypothetical case, set the AARF interval to 100 µs with faster modes available. Let the RTS estimate increase the DATA rate from 6 Mbps to 9 Mbps. The query finds an expired interval and resets the timer. A 52 µs RTS, 16 µs SIFS, and 44 µs CTS place the DATA mode query 112 µs later. The query can select 12 Mbps, so the actual DATA airtime differs from the estimate. DATA starts at 128 µs after its own SIFS.

A successful CTS case must observe both selected modes and the estimate formed at RTS calculation. The 50 ms default interval does not expire across this particular exchange. HCF selects each mode before it passes the frame and IFS to Tx; the subsequent IFS is not elapsed query time.

An actual protected-frame mode can also be slower than the estimated mode. The [IRateControl contract](../../src/inet/linklayer/ieee80211/mac/contract/IRateControl.h) does not require successive queries to return the same or a faster mode. Under a zero limit, the DATA field remains `max(0, E - P)`. A longer actual PPDU reduces that field and can leave its required ACK beyond the advertised endpoint. If `P` exceeds `E`, the field becomes zero. User documentation must state that the estimate does not guarantee protection through the actual response.

For a hypothetical decrease, use a complete 96-byte MPDU with fixed 6 Mbps RTS, CTS, and ACK modes. The estimate uses 9 Mbps DATA with 108 µs airtime and advertises an endpoint at 296 µs. A later 6 Mbps DATA takes 152 µs, starts at 128 µs, and carries a 16 µs field. Its ACK ends at 340 µs, 44 µs beyond that reservation. A deterministic test double for rate control can select these two modes through production HCF. This case tests the supported rate-control contract; it does not claim that AARF decreases during a successful RTS/CTS exchange.

The current zero-limit sequence transmits one selected exchange. If it selects a conventional fragment, estimate that fragment and its required response. Do not reserve the remaining fragment set. A later fragment receives another grant under the existing sequence policy.

### 4.5 Separate recipient responses from the local NAV update

At transmission completion, Tx currently passes every frame's duration to `Rx::frameTransmitted()`. That call extends the local NAV even when the frame is a recipient's CTS, ACK, or Basic Block Ack. The CTS policy then treats that self-created reservation as a reason to refuse another RTS.

HCF and DCF already distinguish holder transmissions from recipient responses at their transmit entry points. Pass their choice through a required `bool updateLocalNav` argument on both `ITx::transmitFrame()` overloads. Update the [ITx contract](../../src/inet/linklayer/ieee80211/mac/contract/ITx.h), Tx implementation, and affected test doubles. Omit default arguments from the interface and its overrides. Holder callers explicitly pass true; recipient response callers explicitly pass false.

Default arguments on virtual functions depend on the caller's static type, not the selected override. An interface default therefore does not supply an omitted argument for a call through `Tx *`. For example, explicit true gives a holder transmission the same NAV behavior through either `ITx *` or `Tx *`. The required argument removes that source of different caller behavior.

All four current production callers supply IFS. The table below identifies their explicit choices. The current MAC and focused tests contain no caller of the overload without IFS. That overload still accepts the required flag and forwards it with `SIMTIME_ZERO` to the overload that accepts IFS.

Retain the immediate overload because it is an existing public entry point. Its one forwarding call preserves that entry point within the required argument change. For example, a hypothetical external immediate-transmission caller adds true without also supplying IFS. The focused forwarding test verifies the zero IFS and the unchanged caller choice.

| Current caller | Transmission role | Required argument |
| --- | --- | --- |
| `Hcf::transmitFrame()` | The frame belongs to the holder. | Pass true. |
| `Hcf::transmitControlResponseFrame()` | The frame is a recipient response. | Pass false. |
| `Dcf::transmitFrame()` | The frame belongs to the holder. | Pass true. |
| `Dcf::transmitControlResponseFrame()` | The frame is a recipient response. | Pass false. |

Tx retains this argument with the accepted frame through IFS and transmission. At completion, Tx copies the completed frame's choice into a local value before it clears retained state. It calls the completion callback in the existing order. Tx applies `Rx::frameTransmitted()` only when that saved choice is true. All transmitted fields retain their values. A callback that starts another transmission must not replace the completed frame's choice.

The caller supplies the role; Tx does not infer it from CTS, ACK, or Block Ack frame types. The current [SelfCtsFs](../../src/inet/linklayer/ieee80211/mac/framesequence/PrimitiveFrameSequences.cc) remains unimplemented. If a later holder path uses CTS-to-self or standalone Block Ack, that caller explicitly passes true. This argument adds no new frame type or sequence path.

For example, a hypothetical recipient completes CTS and ACK for DATA #1. Those response completions do not set its local NAV. It can therefore return CTS for RTS #2 from the same holder when no received reservation blocks it. A NAV established by unrelated received traffic still makes the medium busy and suppresses CTS. Keep the existing CTS refusal policy; do not bypass received reservations.

The holder's local NAV continues to block its other EDCAFs until reservation expiry. A nonparticipating station updates its receive NAV from the advertised fields. Neither endpoint cleanup nor the response exclusion clears an existing NAV timer.

This shared Tx change also affects DCF and default single protection responses. Their response bytes remain unchanged by this exclusion. Their local NAV state can change after response completion. Focused compatibility tests must cover DCF RTS/CTS and single protection HCF.

Step 3 includes migration instructions in [index.rst](../../doc/src/migration-guide/index.rst) for this C++ API change. Custom implementations must accept the required flag in both overloads. Custom callers must pass true for holder frames or false for recipient responses. The instructions must show how Tx preserves the completed frame's choice across the completion callback. A [WHATSNEW](../../WHATSNEW) entry will identify the required update for custom C++ models and the changed local NAV behavior. This follows the [release rules](../../doc/project/rule/release.md#rr-break-migrate); the documentation belongs to the same implementation commit as the API change.

### 4.6 Convert response fields at their transmission boundary

[QosCtsPolicy](../../src/inet/linklayer/ieee80211/mac/recipient/QosCtsPolicy.cc) and [RecipientQosAckPolicy](../../src/inet/linklayer/ieee80211/mac/recipient/RecipientQosAckPolicy.cc) already subtract SIFS and response airtime. Reuse those calculations. At `Hcf::transmitControlResponseFrame()`, clamp the final value to zero and apply ceiling before Tx copies the response. Do not substitute the recipient's own protection selection.

This operation covers HCF CTS, ACK, and immediate Basic Block Ack under both protection classes. It supplies the missing Basic Block Ack clamp and consistent field precision. It does not alter DCF response conversion. For a valid ordinary eliciting duration, subtraction and ceiling cannot exceed that received integer value. Therefore, no additional response range validator is necessary in this change.

For example, a hypothetical Basic BAR field is shorter than SIFS plus Block Ack airtime. Its response calculation is negative, so the transmitted Block Ack field is zero. A positive response result of 31.2 µs becomes 32 µs. Compare these shared corrections against master separately from unintended regressions.

The [header serializer](../../src/inet/linklayer/ieee80211/mac/Ieee80211MacHeaderSerializer.cc) writes whole-microsecond values into the ordinary field. It does not supply the required ceiling. Conversion before Tx also precedes its FCS calculation, so model fields and serialized bytes agree.

## 5. Implementation steps and tests

The independent recipient NAV fix precedes the feature. It supplies the Tx contract, migration instructions, HCF/DCF caller choices, and all 24 approved fingerprint rows. The feature retains that prerequisite until the prerequisite merges into master.

| Step and purpose | Responsible component | Change and expected result | Direct verification |
| --- | --- | --- | --- |
| 1. Normalize response fields. | HCF and the response regression. | Clamp negative subtraction to zero. Apply ceiling before Tx calculates FCS. | Negative and fractional Basic Block Ack fields under single protection. |
| 2. Prepare duration arithmetic. | HCF and the unit fixture. | Add the four duration cases as a helper without a production caller. HCF retains its preceding behavior. | Arithmetic, zero clamp, ceiling, range errors, and field bytes. |
| 3. Add nominal multiple protection. | TxopProcedure, HCF, HcfFs, and focused tests. | Add the protection selector, one endpoint, zero-limit estimates, and normal continuation checks. HCF calls the prepared arithmetic. Include deadline and expired-reservation boundaries in this source commit. | Nominal propagation, repeated RTS, group reservations, failures, endpoint reset, and deadline boundaries. |
| 4. Explain use and limits. | User guide, WHATSNEW, plan, and plan summary. | Explain nominal timing, estimate limits, unused reservations, and support conditions. | Compare selectors, examples, and limits with source and direct test results. |

### 5.1 Commit sequence

Each source commit includes its direct tests. The recipient NAV prerequisite owns the fingerprint values under PR-SPLIT-BASELINE. The feature changes no fingerprint value. Commit messages reference this plan under [PR-MSG-PLAN](../../doc/project/rule/pull-request.md#pr-msg-plan).

| Feature commit | Decision and dependency | Files and direct verification |
| --- | --- | --- |
| 1. Normalize HCF response fields. | HCF clamps subtraction and applies ceiling before Tx. This source prerequisite follows the recipient NAV fix. | HCF, release note, and the response case under single protection. |
| 2. Prepare duration arithmetic. | The isolated helper uses the preceding response encoder. Direct tests establish its equations before production use. | HCF helper declaration and implementation, plus arithmetic and field encoding assertions. |
| 3. Add nominal multiple protection. | HCF calls the prepared arithmetic for holder fields. HcfFs rejects illegal continuation starts and duration intervals. This commit includes the expired-reservation calculation and depends on arithmetic preparation. | TxopProcedure, HCF, HcfFs, production regressions, endpoint and predicate assertions, protection selector checks, and release note. |
| 4. Document use and limits. | Documentation describes the complete nominal feature. | User guide, full plan, and plan summary. |

The Tx contract unit fixture belongs to the independent prerequisite. The response fixture compiles before the nominal feature exists. No feature commit changes received NAV rules.

The arithmetic fixture initially checks only the isolated helper and field bytes. For example, a 1,024 µs limit and 200 µs PPDU produce an 824 µs field before HCF uses that helper. The nominal commit extends the fixture with endpoint and predicate assertions. It also adds the production scenarios.

### 5.2 Focused cases

The following cases define direct verification. The existing [TxopProcedure unit test](../../tests/unit/Ieee80211TxopProcedure_1.test) covers time and default limits but does not establish multiple protection.

| Case | Initial condition and action | Required observation |
| --- | --- | --- |
| `Ieee80211MultipleProtection_1.test`, unit | Supply all four cases, nonzero IFS, negative values, and encoding endpoints. | Correct equation, single ceiling, zero clamp, and holder range error. |
| `Ieee80211TxNavChoice_1.test`, migration signatures | Implement the changed `ITx` interface in a local test class. Call both overloads with true and false through interface and concrete pointers. | The implementation and calls compile with required explicit arguments. The migration snippets match these compiled versions. |
| Same unit case, time origin | Use a 1,024 µs endpoint, a 176.4 µs projected start, and a 100 µs PPDU. | `E` is 847.6 µs before the PPDU. One subtraction of `P` gives 747.6 µs. |
| Same unit case, propagation history | Use the 176.4 µs and 352.8 µs nominal starts from section 3. | The second raw value is legal before ceiling. Its encoded endpoint makes the continuation predicate decline the third exchange. |
| Same unit case, fractional initial DATA | Supply synthetic 100.2 µs initial DATA airtime with a 1,024 µs limit and nominal timing. | Its 923.8 µs raw field becomes 924 µs, with an endpoint at 1,024.2 µs. HcfFs declines another exchange. |
| `Ieee80211HcfMultipleProtection_1.test`, module | Queue several same-AC units with a positive limit and fixed modes. | Real HCF selects multiple protection and uses its current transmission path. |
| Same module case, positive-limit group frame | Queue two group frames with a 1,024 µs limit. Select 200 µs airtime for the first frame. Queue another holder AC and a nonparticipating station during that transmission. | One group frame advertises 824 µs and receives no ACK. The owner endpoint is 1,024 µs before TXOP cleanup and empty afterward. All 824 µs remains unused. Other stations and holder EDCAFs stay blocked until expiry. The second group frame needs another grant. |
| Same module case, repeated RTS | Use zero propagation delay. Force RTS/CTS for at least two DATA/ACK exchanges to the same recipient. | Both CTS responses occur within one TXOP. The recipient's own responses do not create a blocking NAV. |
| Same module case, expired reservation inside an exchange | Use a 384 µs limit, zero propagation, and two RTS/CTS/DATA/ACK exchanges with 6 Mbps modes. The second RTS starts at 304 µs; protected DATA starts at 432 µs. | The second RTS advertises 28 µs. CTS and protected DATA advertise zero. Both exchanges complete without a failure with nominal timing. |
| Same module case, mixed DATA/RTS | Use section 3.1's propagation example. Set the RTS threshold between the first and second DATA lengths. Queue traffic in another holder AC. | Nominal timing stops before RTS for DATA #2. The reservation stays at 1,024 µs and blocks the other AC until expiry. |
| Same module case, Fractional RTS start | Arrange a later RTS nominal start 0.96 µs above its prior grid point under 20 MHz OFDM. | HCF declines the whole exchange before RTS. The DATA remains available; no failure or retry increment occurs. |
| Same module case, BAR precedence | Require BAR while the selected DATA exceeds the RTS threshold and has an unsupported RTS start phase. | BAR retains its existing precedence. The RTS check applies only when the DATA exchange becomes next. |
| Same module case, unrelated reservation | Establish a received NAV at the recipient before another RTS. | The CTS policy still refuses that RTS. No received reservation is cleared or bypassed. |
| Same module case, zero-limit variants | Select DATA/ACK, management/ACK, group traffic, and RTS before DATA with each supported ACK policy. | The default absent-handler case uses Normal Ack. Block Ack policy DATA has no invented immediate ACK. |
| Same module case, fragment variant | Select one conventional fragment under a zero limit. | The estimate covers that fragment exchange without another queue extraction. |
| `Ieee80211HcfMultipleProtection_2.test`, module | Establish the existing immediate Basic Block Ack agreement and transmit policy DATA plus BAR. | Holder fields use multiple protection. Block Ack fields use subtraction, clamp, and ceiling. |
| Same fixtures, early finish | End the burst before reservation expiry. Queue another holder AC and a nonparticipating station. | Both remain blocked by their existing NAV mechanisms until expiry. |
| Same fixtures, failure and reset | Record each transmitted holder endpoint. Lose CTS or ACK. Follow existing recovery. Obtain another grant after reservation expiry. | Existing retry callbacks occur. TXOP cleanup clears the owner endpoint but retains Rx's holder reservation until its advertised expiry. CTS failure leaves the RTS endpoint; ACK failure can include a later DATA endpoint. The new TXOP starts with an empty endpoint and uses its own limit. No general restart support is assumed. |
| Same fixtures, adaptive-rate failure | Use AARF with an initial rate below the maximum. Establish per-peer state before the idle interval. Wait until the interval expires. Trigger a zero-limit RTS estimate. Suppress CTS. | The query selects the next faster mode and resets the interval without DATA transmission. Compare subsequent recovery with master single protection. |
| Same fixtures, adaptive-rate success | Use section 4.4's 100 µs AARF interval and 112 µs interval between mode queries. Permit CTS and DATA transmission. | The RTS estimate uses the 9 Mbps DATA mode. The DATA query at 112 µs selects 12 Mbps. DATA starts at 128 µs. The transmitted RTS field uses the earlier estimate without mode retention. |
| Same fixtures, rate decrease | Use section 4.4's 96-byte MPDU with a deterministic rate-control double. Select 9 Mbps for the estimate and 6 Mbps for actual DATA. | The RTS reservation ends at 296 µs. DATA carries 16 µs; ACK ends at 340 µs. The test establishes the documented estimate limit. |
| Same fixtures, nominal propagation | Select nominal timing with the section 3 example. Queue at least three DATA units. | The first burst sends two exchanges and stops normally. DATA #3 remains available for later access. The local reservation ends at 1,024.4 µs. |
| Same fixtures, compatibility | Omit the protection selector, select single explicitly, and exercise DCF RTS/CTS. | Default selection stays single. Single protection and DCF retain their IFS. Received NAV still suppresses CTS. |
| Same fixtures, caller NAV choice | Transmit HCF and DCF recipient responses through their real callers. Observe holder transmissions separately. Exercise forwarding through the overload without IFS. | Every caller supplies the required flag explicitly. Responses supply false; holders supply true. The immediate overload forwards the same flag with zero IFS. Tx uses the completed frame's saved choice. |
| Same fixtures, conversion | Observe final HCF response headers and serialized bytes; exercise invalid holder range inputs. | Response fields use microseconds and zero clamp. A holder range error precedes transmission. |

Pin seed 0, addresses, packet lengths, PHY modes, error outcomes, and run numbers. Use production HCF, Tx, Rx, recipient policies, and packet-level radios. Use fixed modes except in the adaptive-rate cases and the declared rate-control double. Disable aggregation in the main burst case. Add one existing A-MSDU case separately. Observe actual fields, bytes, channel grants, and NAV expiry.

For the group case, read the owner's endpoint through the existing `frameSequenceFinishedSignal` before `endTxop()` clears it. `Hcf::frameSequenceFinished()` emits that signal while the EDCAF still owns the channel. Assert 1,024 µs at that observation point. Assert an empty owner endpoint after cleanup. Check Rx's reservation separately until expiry; a NAV check alone does not prove that the owner recorded its endpoint.

The independent prerequisite supplies `tests/unit/Ieee80211TxNavChoice_1.test`. Its local class implements the changed `ITx` interface. The fixture calls both overloads with both explicit flag values through interface and concrete pointers. Compare migration snippets with that fixture and the accepted-frame lifetime.

For the propagation case, 20 MHz OFDM at 6 Mbps gives 100 µs DATA for a complete 57-byte MPDU and 44 µs ACK. Set the distance to `propagationSpeed * 0.2us` with fixed positions. Confirm actual PPDU airtimes and propagation delays before the field assertions. For the mixed case, a complete 96-byte second MPDU gives 152 µs airtime; RTS takes 52 µs. Choose an RTS threshold between 57 and 96 bytes.

For the AARF cases, set `initialRate` below the fastest mode in the selected mode set. Initialize the peer state before the idle interval. Record the next faster mode and the interval reset time. Observe the owner state without another rate query that could itself change it.

For adaptive-rate success, initialize the peer at 6 Mbps before the idle interval. Let the expired RTS estimate query select 9 Mbps. Select fixed 6 Mbps control frames and zero propagation delay. Confirm 52 µs RTS and 44 µs CTS airtimes. Check the later 12 Mbps DATA mode against the estimate from the first query, not against exact later airtime.

For the rate-decrease case, replace only rate control with the deterministic test double. Retain production mode selection, HCF, Tx, Rx, and recipient response policies. Select a zero limit and zero propagation delay. Fix RTS, CTS, and ACK at 6 Mbps. Observe the actual DATA mode and the ACK end, not an assumed AARF decrease. Add a unit input with `P > E` to verify the DATA field's zero clamp.

The fractional initial DATA case supplies synthetic airtime to the arithmetic helper. It does not claim that current HT short GI produces fractional PPDU airtime. It also does not establish a fallback for an initial fractional RTS followed by protected DATA; that case remains outside the declared scope.

Choose complete exchanges that fit the budget in the field correctness cases. Run recipient integration and failure/reset cases with nominal timing. Confirm nominal stop keeps pending frames and preserves the existing NAV after TXOP endpoint cleanup. The admission defect remains the limit stated in section 1. These deterministic assertions need no throughput or fairness campaign.

From the implementation checkout root:

```sh
make -j$(nproc) MODE=debug
./bin/inet_run_unit_tests -m debug -f 'Ieee80211(TxopProcedure|MultipleProtection)_1\.test'
./bin/inet_run_module_tests -m debug -f 'Ieee80211HcfMultipleProtection_[12]\.test'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
```

The runners support the shown mode and filter options. Confirm that each filter selects its intended cases after implementation. Record build status, configuration, run, seed, exit status, and artifacts. A zero-case selection is not a pass.

Compare compatibility cases against the recorded master baseline. Expected shared changes are the response exclusion from local NAV updates and HCF response clamp or ceiling. Before publication, follow [run-the-gates.md](../../doc/project/guide/run-the-gates.md), including debug and release compilation. Select related Wi-Fi fingerprint rows explicitly. A baseline update requires the cause, exact scope, and approval specified in [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md).

The table below names candidate rows in [examples.csv](../../tests/fingerprint/examples.csv) and [showcases.csv](../../tests/fingerprint/showcases.csv). These checks retain single protection. The independent recipient NAV fix owns the confirmed fingerprint changes from recipient responses. These candidates are not a prediction that every row changes. Establish actual changes through a master comparison before any baseline update.

| Simulation path | Configuration | Run and duration | Relevant shared behavior |
| --- | --- | --- | --- |
| `/examples/wireless/qos/` | `MacQosWithRtsCts` | Run 0, 10 s. | HCF RTS/CTS and the recipient's local NAV. |
| `/showcases/wireless/hiddennode/` | `WallOnRtsOn` | Run 0, 5 s. | DCF RTS/CTS with an obstacle and positive CTS durations. |
| `/showcases/wireless/hiddennode/` | `WallOffRtsOn` | Run 0, 5 s. | DCF RTS/CTS without an obstacle and positive CTS durations. |
| `/showcases/wireless/blockack/` | `NoFragmentation` | Run 0, 1 s. | HCF Basic Block Ack response fields and local NAV. |
| `/showcases/wireless/blockack/` | `Fragmentation` | Run 0, 1 s. | HCF fragment exchanges and Basic Block Ack responses. |
| `/showcases/wireless/blockack/` | `MixedTraffic` | Run 0, 1 s. | HCF response behavior with two traffic streams. |
| `/showcases/wireless/fragmentation/` | `DCFfrag` | Run 0, 1 s. | DCF response exclusion from local NAV updates. |
| `/showcases/wireless/fragmentation/` | `HCFfragblockack` | Run 0, 1 s. | HCF fragments, Basic Block Ack fields, and local NAV. |

The QoS configuration sets the RTS threshold to 100 bytes and enables HCF through its inherited configuration. Both hidden-node configurations explicitly set DCF's RTS threshold to 100 bytes. The two fragmentation rows also exist at the recorded source revision. Prioritize the RTS/CTS rows for the response NAV change. Retain the other rows for compatibility coverage.

Confirm each row's effective configuration and relevant frame exchanges before attributing a mismatch to this change. Add another row only when configuration or dependency evidence identifies a missed production path. A final ACK with zero duration can remain unchanged; a CTS with positive duration can lose a local NAV event. No candidate row is a verified pass in this plan.

## 6. Acceptance and lifecycle limits

Completion requires nominal timing through the production path, arithmetic boundaries, and documented compatibility evidence. Fractional examples must stop before another exchange requires an empty raw interval. A later start at or beyond the deadline must cause normal completion. An unsupported RTS start phase must also cause normal completion before RTS. No check may discard an existing reservation to admit another exchange. The initial-airtime, RTS-airtime, admission, and lifecycle limits still prevent a complete TXOP compliance claim.

The current [Ieee80211Mac](../../src/inet/linklayer/ieee80211/mac/Ieee80211Mac.cc) stop and crash handlers are empty. This change adds no lifecycle operation or timer. An unsupported stop or crash can therefore leave the scalar unchanged until the next TXOP start. The existing TXOP start and end paths clear it. Tests verify that a new grant clears the previous endpoint; they do not claim safe MAC stop/start behavior.

## 7. Guidance and standards evidence

The design follows the current [plan procedure](../../doc/project/guide/write-an-implementation-plan.md), [minimal design rule](../../doc/project/rule/quality.md#qr-design-minimal), and [WLAN rules](../../doc/project/domain/ieee80211.md). The source paths do not match a sealed path in the current [seal registry](../../doc/project/audit/seal-list.md).

The authority is IEEE Std 802.11-2024, document ID `ieee80211-2024`. The source PDF is named `80211ax-2024.pdf`; it contains the base standard. These are normative clauses within the selected scope.

| Evidence | Physical PDF pages | Requirement |
| --- | --- | --- |
| `ieee80211-2024:clause:9.2.5.2` | 710–712 | Protection class and four duration cases. |
| `ieee80211-2024:clause:9.2.5.1` | 709–710 | Field ceiling and negative-value conversion. |
| `ieee80211-2024:table:9-9` | 670 | Ordinary duration range and encoding. |
| `ieee80211-2024:clause:9.2.5.6` | 713–714 | CTS, ACK, and immediate Block Ack subtraction. |
| `ieee80211-2024:clause:9.4.2.27` | 1069 for the TXOP Limit text | EDCA TXOP Limit encoding in units of 32 µs. |
| `ieee80211-2024:clause:10.3.2.3.3` | 1889–1890 | Nominal SIFS measurement point. |
| `ieee80211-2024:clause:10.23.2.2` | 1999–2001 | TXNAV for holder transmissions and backoff conditions. |
| `ieee80211-2024:clause:10.23.2.4` | 2001 for the AIFS and AIFSN text | AIFS formula and normal EDCA minima: AIFSN ≥ 1 for APs and ≥ 2 for non-AP stations. |
| `ieee80211-2024:clause:10.23.2.8` | 2010–2012 | Continuation and shared busy interval. |
| `ieee80211-2024:clause:10.23.2.9` | 2012–2014 | Zero-limit exchanges and overrun restrictions. |

Clause 9.2.5.2 has source locator `ieee80211-2024@3283316:3295696`. Its nine extracted outgoing references resolve. PDF inspection confirmed the equations on page 711 and the encoding table on page 670. The corpus linter reports ambiguities elsewhere and 425 unresolved references across the corpus. The evidence supports these scoped claims; it does not establish a clean full corpus.

The corpus places the TXOP Limit paragraph under `ieee80211-2024:figure:9-377`, within Clause 9.4.2.27. Its source locator is `ieee80211-2024@4768377:4769428`. This paragraph supplies the 32 µs unit; the figure itself defines the ECWmin/ECWmax field format.

The SIFS clause has locator `ieee80211-2024@7847629:7852209`; its extracted outgoing reference resolves. The nominal policy retains the frame sequence's SIFS without a correction algorithm.

Clause 10.23.2.2 has locator `ieee80211-2024@8315619:8323576`; its five extracted outgoing references resolve. Its TXNAV definition establishes the shared owner and the countdown from PPDU end. Clause 9.2.5.2 defines the zero-limit cost as an estimate; it does not require that a later mode query reproduce the estimated mode. The review of these time and state definitions used the corpus text and required no additional PDF inspection.

Clause 10.23.2.4 has source locator `ieee80211-2024@8324467:8337387`. Its seven extracted outgoing references resolve.

The group-frame check uses Clause 9.2.5.2(a)(9)(i) for single protection and Clause 9.2.5.2(b) for multiple protection. No separate group-frame zero requirement overrides the multiple-protection equations within the declared EDCA scope. The retained upper-bound policy therefore keeps the 824 µs example, with its unused reservation cost stated in section 1. The required explicit Tx argument also follows the C++ rule for [virtual default arguments](https://eel.is/c++draft/dcl.fct.default#10).

## 8. Implementation and executed verification

TxopProcedure retains the selected protection class and holder endpoint until TXOP end. HCF uses nominal IFS for field calculation and transmission. HcfFs rejects incompatible endpoints, starts at or beyond the deadline, and unsupported RTS phases. HCF gives an admitted positive-limit holder frame a zero field after its reservation expires.

For example, scenario 17 uses a 384 µs limit. Its second RTS starts at 304 µs and reserves the deadline. Protected DATA starts at 432 µs and advertises zero. The exchange completes without another full-limit reservation.

Fresh debug and release library builds exit 0. Three debug unit fixtures pass. Seven debug module fixtures pass across the initial selection and the corrected feature rerun, with 54 runs. The feature fixtures supply 18 holder runs and seven Basic Block Ack/rate runs.

Management recovery supplies 24 runs with seeds 0, 1, and 2. Recipient NAV supplies two HCF/DCF runs; cancellation and default selection supply one run each. The invalid protection selector supplies one expected initialization error.

The first module invocation reports two setup failures because the feature run ranges still include the removed timing dimension. The corrected ranges are `0..17` and `0..6`. The focused rerun below exits 0. The five other fixtures pass the initial invocation. This revision changes no recorded fingerprint value.

Run the focused checks from the repository root:

```sh
make MODE=debug -j4
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211(TxNavChoice|TxopProcedure|MultipleProtection)_1\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(HcfMultipleProtection_[12]|TxopProtectionSelectors_[12]|RecipientNav_1|PreparedCancellation_1|HcfManagementRecovery_1)\.test$'
make MODE=release -j4
```

The corrected feature rerun uses:

```sh
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211HcfMultipleProtection_[12]\.test$'
```

The response-conversion commit `88667a6964` also passes a fresh debug build and one production response case. That checkpoint uses single protection and scenario 5. Its source and test files match the original response commit. The nominal source commit has the same runtime source and tests as the tested final tree. The last commit adds documentation only.

At the response-conversion checkpoint, run:

```sh
make MODE=debug -j4
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211HcfMultipleProtection_2\.test$'
```

The scoped architecture and source seal checks exit 0. Global architecture, naming, and interface gates exit 1. Comparison with the pinned base retains 36 architecture findings, 23 naming findings, and 15 interface findings. The local naming gate also reports six old, ignored generated headers whose MSG sources are absent. Those files are outside the committed diff. The committed change adds no finding.

The production cases use seed 0. Management recovery also uses seeds 1 and 2. The invalid protection selector test expects an initialization error. Arithmetic tests include 12 DATA, RTS, and BAR predicate boundaries before, at, and after the deadline.

### 8.1 Fingerprint comparison and approved update

The diff against `c3ab2dda1322bfe3735ac420ff5532a5d203fb38` changes 24 rows and 37 values. All changed values belong to the recipient NAV prerequisite. The expired-reservation correction changes no baseline. Graphical `tyf` values remain unchanged and unverified.

| Ingredient | Changed values | Meaning |
| --- | ---: | --- |
| `tplx` | 24 | Selected simulation event fingerprints. |
| `~tNl` | 7 | Selected network packet time, node-path, and length fingerprints. |
| `~tND` | 6 | Selected network packet time, node-path, and byte fingerprints. |

Seventeen rows change only `tplx`. Seven rows also change network packet ingredients. A fingerprint difference detects a change; it does not establish correctness or identify which selected field changed. Source traces, event logs, captures, and production assertions supply the cause and correctness evidence.

The seven rows with changed network packet ingredients are:

- `examples/adhoc/qos:MacQos`, run 1.
- `examples/wireless/qos:MacQos`, run 0.
- `examples/wireless/qos:MacQosWithoutAggregation`, run 0.
- `examples/wireless/qos:MacQosWithRtsCts`, run 0.
- `examples/wireless/qos:MacQosWithBlockAck`, run 0.
- `showcases/wireless/qos:Qos`, run 0.
- `showcases/wireless/txop:General`, run 0.

The first six change both network packet ingredients. The TXOP row changes `~tNl`; its CSV row has no `~tND` ingredient. For `MacQosWithRtsCts`, `~tNl` changes from `7c57-849b` to `87ba-f56a`. Its `~tND` changes from `06e3-7901` to `e13a-b18f`.

On 2026-10-07, the user replied "yes" to the initial eight-row update. That approval covers eight event values and two network packet values. The user separately approved 16 more rows with 27 values on the same date. These approvals cover the complete 24-row, 37-value update. The earlier eight-row count describes only the initial subset.

All 24 rows pass explicit simulator fingerprint verification in debug mode. Sixteen rows also pass release verification at the recipient NAV checkpoint. The other eight rows pass release verification after the expired-reservation correction. That release selection exits 0 with eight explicit simulator verification successes. The runs retain each CSV time limit and the existing checksum/FCS overrides.

Run that eight-row selection from `tests/fingerprint` after the library build for the selected mode:

```sh
./fingerprinttest -d -t 4 -m 'MacQosWithRtsCts|Wall(On|Off)RtsOn|/showcases/wireless/blockack/.*(NoFragmentation|Fragmentation|MixedTraffic)|/showcases/wireless/fragmentation/.*(DCFfrag |HCFfragblockack )' -f 'tplx' -f '~tNl' -f '~tND'
./fingerprinttest -s -t 4 -m 'MacQosWithRtsCts|Wall(On|Off)RtsOn|/showcases/wireless/blockack/.*(NoFragmentation|Fragmentation|MixedTraffic)|/showcases/wireless/fragmentation/.*(DCFfrag |HCFfragblockack )' -f 'tplx' -f '~tNl' -f '~tND'
```

The remaining 16 rows cover QoS, MAC, aggregation, fragmentation, visualizer, and TXOP configurations in `examples.csv` and `showcases.csv`. The recorded debug and release selections each exit 0 with 16 explicit simulator verification successes. Their committed values match those outputs.

Checkpoint comparisons attribute these changes to recipient NAV exclusion before multiple protection exists. For example, an ACK with a 104 µs duration creates a local NAV expiry event before the correction. Recipient NAV exclusion removes that event. The `MacDcf` captures contain the same 98 observed frames through 6.01 s.

In `examples/wireless/qos:MacQos`, an AP's 140 µs ACK reservation delays the next observed frame by 140 µs. Recipient NAV exclusion removes that delay. The captures show one observation point; they do not establish all upper-layer deliveries.

These fingerprint configurations retain single protection or DCF. Their results establish compatibility evidence for the shared prerequisites, not correctness of multiple protection. The production module assertions establish the new policy's direct behavior. The full wireless suite, additional feature seeds, other production PHY profiles, and graphical fingerprints remain unverified.

### 8.2 Evidence limits

Section 8.1 records prior fingerprint verification for the independent recipient NAV fix. Those configurations use single protection or DCF. They supply prerequisite compatibility evidence, not direct proof of multiple protection. The nominal production assertions supply that direct evidence.

This revision removes the timing selector, timing state, IFS helper, and tests for the removed policy. It retains the existing nominal checks without additional state, rate queries, timers, or events. The feature series contains response conversion, isolated arithmetic, nominal multiple protection with its boundary checks, and documentation.

The implementation retains the admission, TXNAV, initial-airtime, RTS-airtime, and lifecycle limits in sections 1 and 6. The full wireless suite, additional feature seeds, other production PHY profiles, feature-off builds, cross-platform results, and full TXOP compliance remain unverified.
