# Implementation plan: single protection and TXOP admission

Checkout: INET repository root, branch `feat/ieee80211-txop-single-protection`.
Source revision: `1b31d3dfcd252cd35e78a5f9d8398a3bf02a0434`.
Plan revision: 2026-10-07, revision 12.
Document authorization: the user approved the assessed plan and summary changes on 2026-10-07.
Local changes at review: this untracked plan and its untracked summary.
Implementation authorization: the user requested execution of revision 12 on 2026-10-07.
Implementation status: local commit series in progress. Publication and baseline changes require separate authorization.
Execution supplement: shared contention permission prevents a second grant while an active HCF sequence awaits a response.
Design evidence: source revision above and IEEE Std 802.11-2024. The sections below retain the approved design and acceptance criteria.
Examples are hypothetical. They describe the approved behavior and acceptance criteria.

## 1. Problem, intended behavior, and standards assessment

### 1.1 Confirmed problem

The hybrid coordination function (HCF) can start an exchange that exceeds the available time. Its single protection path checks for positive time before the transmission opportunity (TXOP) ends. It does not check the complete next exchange cost. Section 2 identifies the source path and other defects.

Hypothetical example: a response completes with 100 µs before the TXOP deadline. The next DATA exchange and its acknowledgment need 180 µs, before the wait that precedes DATA. The current positive-remainder test admits that exchange. Complete-cost admission must refuse it through the production HCF path.

### 1.2 Background definitions

The medium access control (MAC) sublayer controls access to the medium. The physical layer (PHY) transmits wireless signals. Quality of service (QoS) accounts for traffic requirements such as priority and delay.

This plan covers HCF's enhanced distributed channel access (EDCA) path. Each EDCA access function (EDCAF) manages contention for one access category (AC). A transmission opportunity (TXOP) is a time interval in which a QoS station has the right to start frame exchange sequences. TXNAV is the holder's transmitted reservation timer, shared by its EDCAFs. A PHY protocol data unit (PPDU) includes its preamble, PHY header, and data.

A frame exchange is a related sequence of transmitted frames and responses between stations. Single protection sets Duration/ID from the frame-specific estimates in Clause 9.2.5.2(a). Duration/ID specifies a reservation duration for the frames in this plan. A network allocation vector (NAV) records a medium reservation from protocol duration information.

A PHY mode is a defined combination of PHY format and transmission parameters. An acknowledgment (ACK) frame confirms successful reception of a frame that requires this response. The ACK policy is the rule that specifies the acknowledgment mechanism for a frame. Request to send (RTS) and clear to send (CTS) are control frames that protect a subsequent exchange.

A **forecast** is the calculation that produces a **future estimate** for a field calculation. The future estimate ends with that field calculation. A **current selection** specifies the holder frames, their PHY modes, and the ACK policy for the exchange that HCF now admits. HCF retains the current selection only through that exchange. Response modes remain local estimates under section 4.2. This distinction permits a new holder-mode selection after feedback without a second query that changes an already admitted exchange.

### 1.3 Intended behavior

HCF will estimate future protection without a commitment to transmit that future selection. For supported additional exchanges, HCF will select the current exchange and test its complete expected cost. A selection that passes admission proceeds. A refused selection ends the TXOP without a transmission failure.

HCF preserves initial transmission progress through the compatibility fallback in section 4.3. Section 7.1 explains the fragmentation gap and the required conditions for a later repair.

The **runtime invariant** applies to supported additional exchanges under single protection with `isBlockAckSupported = false`. Their expected cost must satisfy the separate TXNAV and positive TXOP time tests in section 4.3. A zero limit permits no additional exchange in this first repair. Sections 4.4 and 4.7 distinguish this boundary from deferred content rules.

The work consists of separate changes for header correctness, sequence observations, shared TXNAV, and continuation admission with field calculations. Section 5 gives their dependencies. TXNAV precedes admission because admission requires an independent transmitted reservation budget.

### 1.4 Support from the standard

These IEEE clauses support that runtime rule:

- Clause 9.2.5.2(a) defines estimated Duration/ID values for single protection.
- Clause 10.23.2.8 permits another exchange subject to a strict TXNAV comparison and the applicable TXOP restrictions.
- Clause 10.23.2.9 defines positive limits, zero-limit content, and specific overrun exceptions.

These clauses do not establish an identity requirement between an earlier estimate and a later transmission. That conclusion is an inference from the cited text. The design therefore permits a later selection to differ from the estimate, subject to the runtime rule above.

The current transmitter mode determines its airtime exactly within the selected PHY model. Response airtime remains an estimate. Clause 9.2.5.2, informative Note 4, identifies possible differences in peer PHY and MAC choices. Admission checks the supported continuation conditions from the current expected cost. It does not guarantee response reception or exact completion under every physical delay.

Clause 10.23.2.9 also requires fragmentation for applicable individually addressed units that otherwise exceed the limit. The initial compatibility fallback does not implement that requirement. This plan repairs continuation within a declared model boundary; it does not claim complete TXOP compliance.

## 2. Current behavior and confirmed defects

The source path is `channelGranted()` → `HcfFs` → `TxOpFs` → `FrameSequenceHandler` → `Hcf::transmitFrame()` → Tx. These findings describe source inspection before implementation.

| Owner and evidence | Confirmed behavior and effect |
| --- | --- |
| [HcfFs](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc), `hasMoreTxOps()` | Single protection checks frame availability and positive TXOP time. It does not cost the complete next exchange or test TXNAV. |
| [TxOpFs](../../src/inet/linklayer/ieee80211/mac/framesequence/TxOpFs.cc) and [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) | Both select the DATA ACK policy. HCF can retain a header reference from before its policy update. |
| [InProgressFrames](../../src/inet/linklayer/ieee80211/mac/queue/InProgressFrames.cc), `getPendingFrameFor()` | The query can extract frames and assign sequence and ACK state. Its FIXME identifies an incorrect DATA forecast when BAR should follow. |
| [SingleProtectionMechanism](../../src/inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.cc) | The forecast writes mode tags on retained future frames. It also relies on `isFinalFragment()`, which always returns false. |
| `SingleProtectionMechanism::computeDataOrMgmtFrameDurationField()` | The management and non-QoS DATA branch precedes the group-address branch. Group frames of those types can therefore receive an ACK-duration estimate. |
| [TxopProcedure](../../src/inet/linklayer/ieee80211/mac/originator/TxopProcedure.cc) | Its reservation endpoint belongs to one AC, retains a maximum, and clears at TXOP end. Those semantics differ from shared TXNAV. |
| [Tx](../../src/inet/linklayer/ieee80211/mac/Tx.cc) and [Rx](../../src/inet/linklayer/ieee80211/mac/Rx.cc) | HCF holder fields and received reservations extend the same NAV timer. That timer cannot supply an independent TXNAV budget. |
| [FrameSequenceHandler](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceHandler.cc) | Its abort path reports transmission failure. HCF emits sequence start after the handler already starts the first step. |

A rate query can also change state. [AarfRateControl](../../src/inet/linklayer/ieee80211/mac/ratecontrol/AarfRateControl.cc) and [OnoeRateControl](../../src/inet/linklayer/ieee80211/mac/ratecontrol/OnoeRateControl.cc) update rate state when their intervals expire. A temporary frame copy isolates frame changes, not those query effects. The design must avoid repeated holder-mode queries within one admitted exchange.

## 3. Scope and acceptance criteria

Block Ack reports reception status for multiple frames. A Block Ack request (BAR) requests that response.

An MSDU (MAC service data unit) comes from the MAC service user. An MMPDU (MAC management protocol data unit) carries management information. An MPDU (MAC protocol data unit) is a MAC frame with its header and frame check sequence (FCS). An A-MSDU (aggregate MSDU) combines MSDU subframes within one MPDU. An A-MPDU (aggregate MPDU) contains one or more MPDUs for one PHY transmission.

The first admission repair covers these single protection sequences with `isBlockAckSupported = false`:

- Individually addressed DATA with Normal Ack and management with an immediate ACK.
- Prepared fragments and prepared A-MSDUs that use those existing sequences.
- The current group-frame sequence, without a new group burst policy.
- Positive limits, ordinary RTS/CTS, and the nominal short interframe space (SIFS).
- Initial exchanges under a zero limit, with no new continuation beyond the current exchange.

The default remains `protectionMechanism = "single"`. The protection class remains fixed for one TXOP. Complete-cost continuation admission applies only within the scope above. Multiple protection retains its duration equations and separate continuation policy. Block Ack-enabled HCF retains its existing admission, successor forecasts, and holder-mode query behavior, subject to the independent A repairs. The flag disables the new admission check even when no Block Ack agreement exists.

A3 corrects group-address classification in every single protection path, with Block Ack support enabled or disabled. It removes the incorrect acknowledgment estimate for the current group frame. C separately repairs successor eligibility and budget forecasts within its declared scope.

Both protection classes share TXNAV, the header repair, and the handler callbacks. Those changes can affect execution and observations in Block Ack-enabled HCF too. They must not select single protection equations for multiple protection. The first repair adds no ACK-policy interface method or anticipated-frame argument.

A refused continuation remains available for a fresh TXOP. The first-exchange fallback permits that frame to proceed without a new size, mode, or limit. Thus, a continuation refusal cannot become permanent refusal solely because the same exchange also exceeds a fresh positive limit. This condition preserves admission progress; it does not guarantee successful reception under losses or interference.

Acceptance requires all of these results:

- Supported additional exchanges include every holder PPDU, expected response, and internal IFS in their admission cost.
- TXNAV covers the known wait and expected exchange cost under the declared reference in section 4.3.
- A separate TXOP time test refuses additional exchanges that exceed a positive limit.
- Initial exchanges retain progress with unchanged size, mode, and limits.
- A zero-limit TXOP starts no additional exchange in the first repair.
- Supported holder fields follow their applicable single protection equations and field conversion.
- A future estimate does not alter a retained frame's mode tag or ACK policy.
- A current selection uses the normal policies after the previous exchange's feedback.
- DATA executes the holder mode used for the current exchange's cost, including after RTS/CTS.
- Refusal causes no frame drop, retry increment, failed-rate feedback, or response timeout.
- TXNAV follows the latest successful holder field and remains independent of received NAV.
- Active TXNAV blocks contention but does not itself prevent a CTS response to an addressed RTS.
- Finish observers see no refused step in the context history; first-step refusal leaves empty history.
- Single protection uses the group-address equation with Block Ack support enabled or disabled.
- Existing multiple protection fixtures observe TXNAV separately from received NAV.
- Sequence start precedes completion, including a sequence whose first preparation returns no step.
- Another AC makes progress after the relevant reservations and normal contention intervals expire.
- Documentation states the initial compatibility fallback and the outstanding fragmentation requirement.

Basic Block Ack admission, BAR forecasts, zero-limit companions, and optional overrun exceptions require separate follow-up changes. Section 4.7 states their constraints and links to the deferred design notes in Appendix A.

Self CTS, dual CTS, A-MPDU execution, delayed Block Ack, and Action No Ack remain outside scope. `TxOpFs` currently rejects a selected No Ack policy; this plan does not enable it. The plan adds no PIFS recovery, S1G procedure, HE/EHT trigger sequence, reverse direction, or mesh MCCA. MCCA would also require the independent RAV reservation check.

## 4. Design and reasons

### 4.1 Give shared TXNAV one owner

Rx supplies shared medium state to the contention functions. Rx will own TXNAV through one new expiry message, separate from its current NAV message. The message's scheduled arrival time represents the endpoint. A query derives the timer value from that endpoint; no duplicate scalar endpoint or periodic timer is necessary.

Add a TXNAV query and successful-holder update to [IRx](../../src/inet/linklayer/ieee80211/mac/contract/IRx.h). HCF needs those operations for admission and success feedback. Reuse Rx's one-shot timer and medium-state notification pattern. Keep the current NAV timer and `navChanged` simulation signal associated with their current NAV state. A TXNAV update must not appear as a received NAV update.

On a successful holder frame, set the TXNAV endpoint to `holderPpduEnd + encodedDuration`. Replace the old endpoint rather than retain its maximum. A zero or already expired endpoint cancels TXNAV. On expiry, Rx recomputes the contention state.

Keep `IRx::isMediumFree()` as the existing radio-state and NAV query; TXNAV must not affect its result. Both CTS policies retain that query, so TXNAV does not become a CTS refusal condition. The MAC radio-configuration caller also retains its current query semantics. This repair does not broaden the existing physical-state checks or NAV rules.

Rx uses a separate contention query that combines `isMediumFree()` with the absence of active TXNAV. Rx supplies that combined result to `mediumStateChanged()`, including initial contention registration. NAV, TXNAV, and radio-state changes must recompute this result and notify contention when it changes. This distinction needs no additional public IRx query; the existing notification supplies the contention consumer.

HCF also disables local contention through its active sequence, including response waits before the first successful holder frame. Rx owns this explicit permission input through `setContentionBlocked()`. Edca retains ownership of the channel. The permission does not change NAV, TXNAV, or `isMediumFree()`.

The debug QoS showcase exposed this required condition. An initial DATA frame receives no ACK, so TXNAV remains zero. Another AC completes its 50 µs AIFS before the 55 µs ACK timeout. Without the permission gate, that AC receives a grant while HCF's first sequence remains active. The sequence handler rejects that second start. The runtime trace establishes this failure; it does not establish why the ACK remains absent.

HCF sets the permission after its existing internal-collision handling, before sequence start. This order preserves simultaneous collision accounting. HCF restores the permission after channel release and TXOP end, before the next request. Rx recomputes contention state through its existing notification. A ready AC then observes normal AIFS before another grant. No new timer or event is necessary.

This execution supplement adds one Rx permission field and one IRx operation within B's existing owners. Both protection classes need the initial-failure test with another AC ready. The test requires zero TXNAV during the response wait, one channel owner, and later service for both ACs. DCF retains its existing path.

For example, the TXOP ends at 300 µs, but TXNAV remains active until 500 µs. At 350 µs, an addressed RTS arrives with idle NAV, available radio state, and no channel owner. The recipient sends CTS after SIFS, while TXNAV continues to block local contention. An unrelated received NAV reservation must still prevent CTS. Clause 10.3.2.9 supplies the CTS condition; Clause 10.23.2.2 supplies the separate TXNAV contention condition.

Record the actual PPDU end in the holder's transmit step at radio completion. For a frame that needs an immediate response, HCF publishes success after the handler accepts that response. Publish it before the handler selects the next step. For a frame without an immediate response, publish success at transmission completion. A timeout or rejected response does not publish success.

For example, RTS ends at 100 µs with a 300 µs field. CTS arrives successfully at 160 µs. The TXNAV endpoint becomes 400 µs, not 460 µs. The response confirms success; it does not move the countdown origin. This rule follows Clause 10.23.2.2.

HCF holder submissions will disable the old Tx-to-Rx NAV extension and use successful TXNAV updates instead. Recipient responses already disable that extension and retain that choice. The distributed coordination function (DCF) keeps its current local NAV behavior. Tx retains its current submission and completion contract.

An arbitration interframe space (AIFS) is the interval that an EDCAF uses before contention can proceed on an idle medium. Do not clear TXNAV at TXOP end. For example, a TXOP ends at 300 µs with TXNAV due at 500 µs. Another AC must remain unable to access the medium through that expiry. Its normal AIFS and backoff apply afterwards.

Keep `TxopProcedure::reservationEnd` for the maximum advertised endpoint used by nominal multiple protection. It is a separate fact, not a second TXNAV owner. Its value can differ from TXNAV after a shorter successful field or a failed holder frame. The multiple protection arithmetic must continue to use its advertised endpoint.

For example, the maximum advertised endpoint is 400 µs, but a later successful field ends at 250 µs. TXNAV becomes 250 µs; the advertised maximum remains 400 µs. A failed holder frame can change the advertised maximum without a successful TXNAV update. These values must not substitute for each other.

Rx's display must derive its TXNAV indication from the timer. It must not retain another authoritative copy of that state.

This prerequisite changes some HCF failure and contention trajectories in both protection classes. It preserves the multiple protection equations, not every previous trajectory. The new one-shot expiry can also change event fingerprints. Direct state and contention tests must establish correctness before any baseline proposal.

Update [Ieee80211HcfMultipleProtection_1.test](../../tests/module/Ieee80211HcfMultipleProtection_1.test) in the same TXNAV change. Its `endpoint()` hook currently observes `endNavTimer`, and `localUpdates` counts `Rx::frameTransmitted()` calls. HCF holder submissions will no longer produce those local NAV updates. Add a distinct TXNAV observation through the new Rx contract.

Replace the sender's local-update count with an assertion that HCF holder frames cause no old local NAV update. Compare the sender's TXNAV endpoint with each relevant successful holder field and PPDU end. Keep the observer's received NAV assertion separate. Retain the receiver's zero-local-update assertion for recipient responses. Check the existing early-finish and group-frame scenarios through these separate observations.

Do not equate TXNAV with `burstEnds` in every scenario. That vector records the maximum advertised endpoint; shorter successful fields and failures can make it differ from TXNAV. Preserve direct assertions for medium exclusion until the applicable timer expires.

Update [Ieee80211RecipientNav_1.test](../../tests/module/Ieee80211RecipientNav_1.test) in B too. Its holder case currently requires one local NAV update for both HCF and DCF. Require zero old local NAV updates for HCF. Retain one for DCF. Observe HCF TXNAV separately; a zero holder field need not leave an active timer. Retain the existing recipient-response and received-NAV checks.

Add the active-TXNAV CTS case with no EDCAF channel owner and no received NAV reservation. Establish nonzero TXNAV through a successful holder exchange before the RTS arrives. Check the emitted CTS, unchanged TXNAV endpoint, and continued exclusion of another ready access category. Retain an unrelated received-NAV control that refuses CTS. Cover both protection classes. Preserve the DCF controls.

### 4.2 Select the current exchange once

`TxOpFs` will remain the owner of the sequence branch and RTS choice. HCF will remain the owner of mode selection and cost admission. Publish the chosen ACK policy to the current DATA header before the branch executes. HCF must read the updated header and must not independently choose a second ACK branch.

The header repair is an independent change for both protection classes. For example, the branch selects Block Ack while the retained header still says Normal Ack. The field calculation must use the published Block Ack policy. This repair uses the existing ACK-policy contract; it does not add BAR forecast support.

`TxOpFs` currently selects the ACK branch without publication to the DATA header. HCF independently selects the policy again before rate selection. A1 therefore needs both components: `TxOpFs` publishes its branch choice, and HCF consumes the updated header without another policy selection. `TxOpFs::selectTxOpSequence()` must publish the selected policy in one place before execution of that branch.

The sequence's ACK choice remains authoritative for that exchange, including DATA after RTS/CTS. The built-in [ACK policy](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.cc) reads frame properties and agreement fields. It does not read simulation time, agreement expiration, mode tags, or the current ACK-policy field. Without RTS, branch selection and DATA submission occur synchronously. With RTS, successful RTS/CTS does not change the agreement fields that this policy reads. These are source observations for the recorded revision, not a guarantee for every custom policy.

For example, an eligible DATA frame selects Block Ack before RTS. Successful CTS preserves the relevant policy inputs, so DATA retains the selected Block Ack branch. A non-CTS response terminates that exchange through the existing failure path. The current [expiry handler](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreementHandler.cc) queues DELBA without immediate agreement removal. A future agreement-lifecycle repair must reassess this boundary. A1's tests cover direct DATA and DATA after RTS/CTS with the built-in policy.

The agreement's [sent-frame counter](../../src/inet/linklayer/ieee80211/mac/blockack/OriginatorBlockAckAgreement.h), `numSentBaPolicyFrames`, starts at zero. Its increment method, `baPolicyFrameSent()`, has no caller in the tracked source at the recorded revision. For example, a newly created agreement retains zero before RTS and after DATA. This supports A1's stability claim but identifies a separate accounting limitation. A1 must not add a counter update. Recheck its callers before A1's test; a later accounting repair must reassess the policy inputs across RTS/CTS.

Earlier policy publication also corrects the single protection RTS field. [SingleProtectionMechanism](../../src/inet/linklayer/ieee80211/mac/protectionmechanism/SingleProtectionMechanism.cc) reads the protected DATA header to decide whether its RTS must reserve an immediate ACK. For example, the branch selects Block Ack while the original header says Normal Ack. With fixed modes, A1 removes the unnecessary ACK airtime and its SIFS from the RTS estimate. The RTS test must inspect the emitted Duration/ID as well as the later DATA header and field. The supported Block Ack DATA branch does not acquire an immediate Block Ack response through this repair.

For the scoped single protection path, HCF selects the necessary modes at the first holder step and calculates the complete cost. A direct DATA exchange needs its DATA mode and expected response mode. An RTS exchange also needs the current protected DATA mode, RTS mode, and expected CTS mode. Use the protected frame already held by `RtsTransmitStep`. Do not obtain another DATA unit for that RTS. Reuse each response estimate within that synchronous calculation for admission and the current field equation.

The current DATA frame's mode tag retains its admitted mode through RTS/CTS. The ACK policy remains the policy that selected the current sequence branch. After CTS, DATA uses that current selection. There is no second admission rule or normal rate reselection inside that admitted exchange. Current timeout and invalid-response paths retain their failure behavior.

The initial compatibility exchange also retains its selected DATA mode through RTS/CTS. Its expected cost supports the field calculation and successor forecast; that cost does not reject the initial exchange. Multiple protection and Block Ack-enabled HCF retain their existing holder-mode query points.

Response estimates remain local to each holder step's synchronous calculation. After CTS, HCF recomputes the ACK estimate for the DATA field from the retained DATA mode through the existing response-mode policy. It does not select the holder's DATA mode again or repeat admission. The built-in response-mode policy derives its estimate from the frame's mode tag, configured response mode, and applicable peer restrictions. The estimate can change if those policy inputs change; it does not promise the peer's actual response airtime. HCF needs no retained response estimate across RTS/CTS.

At a new exchange boundary, HCF selects the current mode even if the frame already has a mode tag. A tag from a refused exchange or previous attempt does not establish a current selection. The sequence position distinguishes DATA after its admitted RTS/CTS from DATA at a new exchange boundary. This uses the current step history without another selection flag.

For example, B's estimated mode during A can differ from B's current mode before RTS. Once HCF admits B, B's RTS field and DATA transmission use B's selected mode. After CTS, the ACK estimate uses that same DATA mode; unchanged response-policy inputs produce the same ACK airtime estimate. A later rate-control update affects the next exchange's holder-mode selection. This short retention keeps the holder's execution consistent with admission while response airtime remains estimated.

Reuse the current frame, its mode tag, the sequence branch, and `RtsTransmitStep` for this lifetime. Local numeric costs need no retained exchange graph. The design adds no generic future planner, queue snapshot, request ID, generation counter, or cancellation callback. Any wider validity repair needs its own supported contract and reachable failure.

### 4.3 Define admission budgets

An interframe space (IFS) is a required interval between specified frame transmissions. SIFS separates specified immediate responses and frames within an exchange. Let `t0` be the exchange decision time, after the previous exchange completes and before the next IFS wait. Let `I` be the wait before the first PPDU. Let `C` be the selected sequence's complete expected duration from its first PPDU through its final response. Internal IFSs belong to `C`; `I` does not.

| Sequence | Cost `C` |
| --- | --- |
| DATA or management with ACK | `Pdata + SIFS + Pack` |
| DATA without an immediate response | `Pdata` |
| RTS/CTS and DATA/ACK | `Prts + SIFS + Pcts + SIFS + Pdata + SIFS + Pack` |

The deferred Basic Block Ack repair also needs two costs. Response-free protected DATA needs `Prts + SIFS + Pcts + SIFS + Pdata`. BAR/Basic Block Ack needs `Pbar + SIFS + Pba`. Those costs do not activate the deferred paths in the first repair.

Each `P` uses the relevant mode object's duration for the complete frame length, with FCS and PHY overhead. Response modes use the current rate-selection rules. An ACK timeout is a failure-detection interval, not an ACK airtime estimate.

For a supported additional exchange, HCF requires `I + C <= TXNAVremaining(t0)`. HCF also requires `I + C <= TXOPremaining(t0)` for a positive limit. These are separate reservation and TXOP checks. The first repair grants no additional-exchange overrun exception.

The first exchange has no previous TXNAV reservation to pass. HCF permits the initial exchange without refusal from the positive TXOP cost check. This rule also applies to an initial RTS/CTS/DATA exchange. It preserves progress but does not establish an IEEE overrun permission. Section 7.1 explains the fragmentation gap.

A zero limit permits the current initial exchange, including its existing protection and immediate response. It permits no additional exchange in this first repair. Section 4.7 records the broader zero-limit content that a separate repair can support.

Clause 10.23.2.8 requires `C < TXNAVremaining`, but it does not explicitly define the timer sample instant. This plan interprets that instant as `t0`. Every supported additional exchange has `I = SIFS > 0`. Therefore, `I + C <= TXNAVremaining(t0)` also proves the strict sequence comparison. HCF needs only the containment test at runtime. A future zero-wait continuation would require a separate assessment of this implication.

The containment test is an explicit design condition that ensures the known wait and expected exchange fit the reservation. It is not a separate IEEE requirement. It prevents the known wait and changed selection from consuming more time than TXNAV permits under the declared reference.

Hypothetical example: DATA A reserves time for an estimated DATA B/ACK exchange of 180 µs. After A's ACK, `I = 16us` and TXNAV is 196 µs. If B's current exchange cost remains 180 µs, its complete 196 µs cost passes the containment test. The expected completion coincides with the reservation endpoint. If rate control increases B's cost to 190 µs, HCF refuses the complete 206 µs cost and retains B for later access. Sufficient TXOP time does not override that refusal.

A strict comparison at PPDU start would instead require `I + C < TXNAVremaining(t0)`. That choice would refuse the first example. It is a different interpretation, not a hidden extra test in this plan. Preserve the declared reference in code, tests, and documentation. Do not extend Duration/ID beyond its applicable equation to force continuation.

The boundary tests verify this declared implementation choice. They do not prove that the standard selects `t0` rather than PPDU start. The interpretation relies on Clause 10.23.2.8's conditional permission before the subsequent SIFS transmission. Its timer sample instant remains an inference. Review and implementation approval must cover this stated interpretation.

Observed elapsed time reduces subsequent budgets. Response airtime remains estimated. No new hardware delay, timer margin, or PHY formula is part of this repair.

### 4.4 Preserve progress and limit continuation

Section 4.3 defines admission and the initial compatibility fallback. This section defines the exchange boundaries to which those rules apply.

An exchange boundary starts a TxOpFs iteration or an iteration of the current group-frame sequence. DATA after CTS belongs to the exchange that started with RTS. RTS completion does not make its protected DATA an additional exchange. Use the current sequence position to distinguish those cases. A count of transmitted frames cannot replace this exchange distinction.

For a zero limit, retain the existing stop after the initial exchange. RTS, CTS, DATA, and its immediate ACK remain parts of that exchange. A later fragment requires another TXOP in this first repair. This choice adds no unit-identity state, anticipated-content count, or optional overrun predicate.

For example, fragments A1 and A2 await transmission under a zero limit. HCF transmits A1 and its ACK in the initial exchange. A2 waits for a fresh TXOP, where the initial fallback permits progress. The first repair does not claim support for both fragments within one zero-limit TXOP.

The current recovery path still handles actual RTS failure, invalid responses, and ACK timeout. Section 4.6 defines normal completion after budget refusal.

Keep the multiple protection continuation policy separate. Keep the Block Ack-enabled sequence predicates on their existing path. Do not broaden either path through the first repair's single protection predicates.

### 4.5 Calculate Duration/ID fields

`SingleProtectionMechanism` will retain responsibility for Duration/ID equations. HCF will supply the current response cost, an optional next-frame protection cost, and an explicit final-frame decision. These are local values. They do not form an executable future sequence.

The next-frame protection cost and complete admission cost are different. RTS uses Clause 9.2.5.2(a)(1), which covers CTS, protected DATA, any required response, and IFSs. Acknowledged DATA uses item (a)(8), which covers its acknowledgment and, unless final, the next frame and its response. BAR uses item (a)(5), which covers its immediate response and SIFS.

For example, a forecast selects RTS as the next holder frame. The current field equation uses RTS and its CTS response. Admission still costs the complete RTS/CTS/DATA exchange and can decline it. Do not inflate the field to force continuation.

This distinction can prevent another exchange within the TXOP even when modes remain unchanged and the TXOP budget is sufficient. For example, A's field reserves B's RTS/CTS estimate, but B's complete RTS/CTS/DATA exchange exceeds TXNAV after A's ACK. HCF refuses B before RTS and retains B for a fresh grant. The test must check A's field as well as B's refusal and later transmission.

Keep normal candidate extraction through `InProgressFrames`; that operation can assign sequence numbers and prepare fragments or A-MSDUs. Use its result once per decision. For field calculations that require temporary mode tags or ACK policy changes, duplicate the candidate locally. Do not write forecast fields to retained candidates. Destroy the duplicate when the calculation ends.

For example, retained B has a mode tag and ACK policy before A's forecast. The forecast selects a different mode on B's temporary copy. After the calculation, retained B must still contain its original tag and ACK policy. Verify those retained values directly; later transmission alone cannot prove that the forecast preserved them. A new exchange boundary still selects B's current mode under section 4.2. The frame copy does not suppress permitted rate-control query effects.

Determine final-frame status from the forecast's eligible successor. Queue presence or the unfinished `isFinalFragment()` method cannot establish that eligibility. Apply the current sequence branch's continuation restrictions and section 4.4. A zero-limit exchange has no successor in this first repair. The forecast must also apply the positive TXOP budget as follows.

For a positive limit, HCF calculates the current exchange's expected completion time from the current holder step. That calculation includes the wait before this step and the uncompleted holder PPDUs, expected responses, and internal IFSs. HCF subtracts that completion time from the TXOP deadline. The successor's initial wait and complete expected exchange cost must fit this remainder. The first repair applies no successor overrun exception. HCF reuses the forecast's mode and response estimates for this local calculation.

If the forecast excludes the successor, the current DATA or management frame is final under the selected estimates. Final acknowledged frames reserve only their immediate response. Final group and response-free frames reserve zero. A second unrelated zero-limit unit is not a successor. The current group-frame path does not gain a new burst policy.

A3 makes `SingleProtectionMechanism` select the group-address equation before the generic management or non-QoS DATA equation. This order keeps group frames out of the current-frame ACK-duration branch across all single protection paths. The correction does not depend on TXNAV, admission, or the Block Ack support flag. For example, a final group management frame with no successor requires zero Duration without a current-frame ACK-mode query. A separate non-QoS group DATA case must produce the same result.

A3 preserves the group equation's successor term when a successor exists. Clause 9.2.5.2(a)(9) permits a nonfinal group frame to reserve its successor and any required response. A successor can therefore require its own ACK-mode query. A3 does not force every group field to zero or repair the general `isFinalFragment()` placeholder. C owns the supported eligibility and final-frame decisions above.

The current [group sequence](../../src/inet/linklayer/ieee80211/mac/framesequence/HcfFs.cc) stops after one frame. Its group predicate requires a multicast successor, but the shared continuation predicate rejects multicast after the first iteration. A3 retains a known limit: the field calculation can reserve a queued successor even though the sequence cannot transmit it. This occurs when `getPendingFrameFor()` supplies a candidate while `isFinalFragment()` still returns false. A3's nonfinal-group case checks arithmetic only; it does not establish a reachable production continuation.

For example, group frame A has a queued frame B that the forecast returns. A3 removes A's incorrect ACK estimate but retains B's estimated airtime and any required response. The group sequence ends after A, so the reservation remains unused. C removes that ineligible successor estimate with `isBlockAckSupported = false`. The limit remains with Block Ack support enabled and must appear in the release notes and MAC documentation. A3 remains a classification repair; a broader group-finality repair is separate work.

Hypothetical example: A uses Normal Ack, and its expected completion leaves 100 µs before a positive TXOP deadline. Queued DATA B needs 180 µs plus SIFS, and no overrun exception applies. The forecast excludes B, so A reserves only its acknowledgment. The test keeps the modes fixed to make this result deterministic.

The forecast's time check does not admit or retain the future selection. At the next exchange boundary, HCF selects the current exchange again and applies section 4.3 to the actual available budgets. A successor that passes the forecast can still fail actual admission after a mode change.

Apply `ceil(max(0, rawDuration) / 1us)` under Clause 9.2.5.1. Reject ordinary fields above 32,767 µs under Table 9-9. Reuse HCF's conversion helper with error text that applies to both protection classes. For example, 180.2 µs becomes 181 µs, and successful TXNAV uses that transmitted value.

### 4.6 Refuse synchronously and preserve observations

The existing handler already finishes normally when the complete sequence's `prepareStep()` returns `nullptr`. That route supports refusal if the sequence owns the necessary admission decision. Compare it with refusal at HCF's current submission boundary:

| Option | Existing mechanism and limitation | Decision |
| --- | --- | --- |
| Check the outer HCF continuation predicate. | The predicate can stop the whole sequence. Its context exposes no HCF rate-selection or cost service. | This option needs a new preparation query and reuse of its selected modes during transmission. |
| Return no step from an inner primitive. | `SequentialFs` advances to its next element; `RepeatingFs` can start another iteration. | A primitive's completion alone does not mean refusal of the complete exchange. |
| Return refusal from HCF's transmit callback. | HCF already receives the built holder step and selects modes before Tx submission. | Use this boundary to keep cost ownership in HCF and complete-exchange refusal in the handler. |

The choice follows [GenericFrameSequences](../../src/inet/linklayer/ieee80211/mac/framesequence/GenericFrameSequences.cc) and the current [sequence context](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.h). For example, the RTS primitive returns no step inside a sequential RTS/CTS/DATA exchange. The parent sequence can advance to DATA rather than finish the exchange. A refusal must terminate the complete exchange before that advance.

The sequence handler needs to distinguish submission from budget refusal. Change its current transmit callback to return a Boolean result. HCF returns false before it calls Tx for a refused selection. After submission to Tx, HCF and DCF return true. True means submission, not successful radio transmission. The handler processes false through normal completion, not its abort path that reports failure.

The handler adds the prepared step to the context before the transmit callback. HCF uses that installed step during selection and admission. On false, the handler removes the refused last step from the context through a context-owned removal operation. That operation destroys the step before the handler invokes normal sequence completion. Step destruction releases an owned RTS frame and preserves borrowed unsent DATA. Finish observers therefore receive history without the refused step.

Do not complete the refused step. Do not advance its sequence. Do not schedule its response timeout. Emit the transmission's data-rate simulation signal only for an accepted current selection. BAR refusal belongs to the deferred Basic Block Ack repair. Removal avoids a new completion state and skip rules for every history reader.

For example, DATA A and its ACK complete before HCF refuses B's RTS. The finish observer must see A's completed ACK as the last step, with the earlier step count unchanged. B's temporary RTS is destroyed, and B's DATA remains available for a fresh TXOP. A controlled first-step refusal leaves zero steps and a null last step. Test both histories through the finish simulation signal, before context destruction.

The current normal-finish consumers accept empty history. [HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) and [DCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Dcf.cc) release channel ownership without access to a last step. `FrameSequenceContext::getDuration()` returns `simTime() - startTime`; the [result filters](../../src/inet/linklayer/ieee80211/mac/framesequence/FrameSequenceContext.cc) read that duration and the step count. The [existing test override](../../tests/module/Ieee80211HcfMultipleProtection_1.test) reads TXOP state and its own counters. These source checks identify no empty-history dereference on normal completion; they do not replace C's runtime test.

Establish valid channel ownership in the controlled first-step refusal fixture. Keep the duration and step-count result filters active. For immediate refusal, require one finish observation, zero duration, zero steps, and a null last step. HCF still requires a channel owner even when history is empty. Recheck finish consumers at C's implementation head before any source change. Add no general null guard without a consumer that requires it.

HCF currently emits sequence start after `startFrameSequence()` returns. That call can finish an empty sequence and destroy its context before the caller emits start. The emitted start context is then null. A controlled first-step refusal can expose the same observation defect.

Add a start notification to the current handler callback. The handler invokes it after it establishes the context and before it starts the first step. HCF and DCF emit their current start simulation signal through that notification. Remove their later duplicate emission. No self-message or additional event is necessary.

The handler initializes the sequence through `startSequence()` before the notification. It dispatches the first step through `startFrameSequenceStep()` after the notification. Thus, a start observer can inspect both the installed context and initialized sequence.

Emission before the handler call would fix the order but report start before the handler owns its context. The notification preserves both order and established handler state. It also retains HCF and DCF as the declared simulation signal sources.

For example, an empty first preparation must emit one start followed by one finish with valid contexts. A start observer must find the installed context and active sequence. HCF releases channel ownership through its current completion path. A controlled refusal also retains the unsent frame without a radio failure.

The production admission repair refuses additional exchanges only. Its first-step refusal test verifies the handler contract through a controlled fixture. That test does not establish a production policy that rejects the initial compatibility exchange.

### 4.7 Deferred content and Block Ack work

These conditions belong to separate follow-up plans. They preserve the standards boundaries for Basic Block Ack, BAR forecasts, zero-limit companions, and optional overruns. The first repair does not implement them or change the ACK-policy interface.

A follow-up must define its supported sequences and prove progress before it enables stronger admission. Strict initial cost admission also requires a data-service fragmentation design. A follow-up that removes the compatibility fallback without that design can stop the default-limit voice case in section 7.1.

The follow-up plans must preserve these conditions:

- Zero limits constrain content under Clause 10.23.2.9. Eligible same-unit fragments and companion frames still need the applicable reservation checks.
- Positive-limit exceptions require the complete exchange's content to meet every applicable condition in Clause 10.23.2.9.
- BAR forecasts must represent BAR and its response. A required BAR must remain available even when no unsent DATA remains.

[Appendix A](#appendix-a-deferred-design-notes) retains the detailed proposals for those follow-ups. Those proposals require review against the source and supported sequences of each follow-up. They add no implementation step or ACK-policy contract change to A1 through C.

## 5. Change surface and implementation order

The source paths below are unsealed in the current registry. The sealed Packet API remains outside scope. No new NED parameter, frame type, or `.msg` field is necessary. Rx NED documentation must describe the separate transmitted reservation state.

| Separate change and purpose | Owner and files | Change and expected result | Direct verification |
| --- | --- | --- | --- |
| A1. Repair the published header. | HCF and TxOpFs. | Publish the chosen ACK policy before execution. Use the updated header for rate selection and fields. | A branch selects Block Ack while the original header says Normal Ack; the emitted field follows the selected branch. |
| A2. Repair sequence observations. | Handler callback contract, FrameSequenceHandler, HCF, and DCF. | Notify start after context installation and before first-step execution. Preserve each declared emitter. | Normal, empty-first-preparation, and synchronous-finish cases have one ordered start/finish pair and valid handler state. |
| A3. Repair group-address field classification. | SingleProtectionMechanism and its direct fixture. | Apply the group equation before management and non-QoS DATA equations throughout single protection. Preserve its successor term. | Group management and non-QoS DATA omit a current-frame ACK estimate with Block Ack support enabled and disabled. |
| B. Establish shared TXNAV. | `Rx.{h,cc,ned}`, `contract/IRx.h`, HCF, transmit steps, and multiple protection and recipient NAV fixtures. | Publish successful endpoints from actual holder PPDU end. Separate contention state from the CTS query. Replace holder NAV assertions with separate TXNAV observations. | Success, failure, shorter and zero fields, NAV isolation, CTS during active TXNAV, early finish, and another AC after expiry. |
| C. Admit continuations and calculate their fields. | HCF, HcfFs, TxOpFs, SingleProtectionMechanism, handler contract, FrameSequenceContext, DCF, and relevant frame steps. | Retain the selected DATA mode. Remove refused steps before normal completion. Preserve initial progress and zero-limit stops. | Budget boundaries, retained forecast inputs, RTS successor refusal, finish-observer history, queued voice progress, another AC's service, and unchanged deferred paths. |

Each change includes its direct tests, release notes, and relevant MAC documentation. A1, A2, and A3 can proceed independently of TXNAV or admission. B must precede C because C needs the independent TXNAV budget. C includes forecast repairs because its admission rule and advertised fields must agree.

A1's release note must explain the effect of earlier ACK-policy publication on single protection RTS Duration/ID. When Block Ack replaces a stale Normal Ack header, the RTS estimate omits the unnecessary immediate ACK and its SIFS. This field correction can change fingerprints in Block Ack configurations. Record it as an expected field change during fingerprint analysis under section 6.

C's release notes and MAC documentation must state that `isBlockAckSupported = true` disables complete-cost continuation admission even without an agreement. For example, a station with Block Ack support enabled but no agreement retains the existing admission path for Normal Ack DATA. The notes must distinguish that limit from A3's unconditional single protection classification repair and B's shared TXNAV behavior.

Use separate pull requests for the independent repairs, shared TXNAV, and continuation admission. A1, A2, and A3 may share one repair pull request as separate commits. Build each intermediate tree. Keep the multiple protection and recipient NAV prerequisites in their own pull requests. Start the TXNAV follow-up from their merged source before publication.

External IRx and sequence-callback implementations need migration in the change that modifies their contract. A2 adds the start notification; C adds the submission result. B adds the TXNAV operations. The ACK-policy method signatures remain unchanged in A1 through C. A1 removes HCF's second DATA-submission policy query; custom policies must not depend on that call or its effects. Search all callers and overrides before implementation.

The work in section 4.7 needs separate plans and approval. A fragmentation repair must establish progress before any later change removes the initial compatibility fallback.

Apply [minimal design](../../doc/project/rule/quality.md#qr-design-minimal), [state ownership](../../doc/project/rule/quality.md#qr-state-owner), and [reuse](../../doc/project/rule/architecture.md#ar-ext-reuse). Use the [IEEE 802.11 rules](../../doc/project/domain/ieee80211.md) for timing and protocol evidence. The [plan procedure](../../doc/project/guide/write-an-implementation-plan.md) supplies the review route.

## 6. Verification and expected results

The criteria below retain the approved verification design. Reuse [Ieee80211HcfMultipleProtection_1.test](../../tests/module/Ieee80211HcfMultipleProtection_1.test) for production HCF, Tx, Rx, and radios. Its hooks observe emitted mode, airtime, field, and retry state. They must observe the actual production decisions, not copy the admission algorithm.

Create `Ieee80211HcfHeader_1.test` for A1 and `Ieee80211FrameSequenceObservations_1.test` for A2 under `tests/module/`. A3 adds `Ieee80211SingleProtectionGroup_1.test` in the same directory. B adds `Ieee80211Txnav_1.test` and revises the multiple protection and recipient NAV fixtures. C adds `Ieee80211HcfSingleProtection_1.test` and extends the observation fixture for refused history. Add `tests/unit/Ieee80211SingleProtection_1.test` only for numeric boundaries that the production cases do not cover.

Use Cmdenv, `seed-set = 0`, fixed response modes, and explicit scenario numbers. Record each case's protection class and `isBlockAckSupported` value. Admission cases use single protection with Block Ack support disabled. Compatibility cases include DCF, multiple protection, and Block Ack-enabled HCF.

| Change, scenario, and action | Required observation |
| --- | --- |
| A1: Select Block Ack from a DATA header that initially says Normal Ack. Test direct DATA and DATA after successful RTS/CTS. | The emitted header, field calculation, and response sequence use the selected branch's ACK policy. HCF does not repeat policy selection at DATA submission. |
| A1: Use single protection and fixed modes for RTS before DATA whose selected policy is Block Ack. | The emitted RTS Duration/ID excludes the unnecessary immediate ACK airtime and its SIFS. The subsequent DATA header and field use the same selected policy. |
| A2: Start normal and empty-first-preparation sequences. | One start precedes one finish. Observers see valid context and installed handler state. |
| A3: Transmit final group management and non-QoS group DATA, each without a successor, with Block Ack support enabled and disabled. | Each emitted Duration/ID is zero. The calculation makes no current-frame ACK-mode query, and the sequence schedules no ACK timeout. |
| A3: Supply a successor to the group field calculation in a direct arithmetic case. | The field contains only the successor and its required response and IFSs. It contains no current-frame acknowledgment estimate. This does not establish production group continuation. |
| B: Complete successful RTS, DATA/ACK, and response-free DATA; also force response failure. | TXNAV updates only on success, from holder PPDU end. |
| B: Replace TXNAV with a shorter or zero field while received NAV stays longer. | TXNAV follows the latest successful field; received NAV remains unchanged. |
| B: Run the revised multiple protection early-finish and group-frame scenarios. | Holder TXNAV persists where required. The observer retains received NAV. Old holder local NAV updates are absent. |
| B: Compare a failed holder field and a shorter successful field with the advertised maximum. | The maximum endpoint and TXNAV retain their distinct semantics. |
| B: End early with another AC ready. | TXNAV prevents access until expiry; normal AIFS and backoff follow. |
| B: Receive an addressed RTS during active TXNAV with idle NAV, available radio state, and no EDCAF owner. | CTS transmits after SIFS. TXNAV retains its endpoint and blocks the ready AC. Both protection classes pass. |
| B: Repeat with an unrelated received NAV reservation. | CTS remains refused. TXNAV isolation does not bypass received NAV. |
| B: Run the recipient NAV holder case for HCF and DCF. | HCF causes zero old local NAV updates and uses TXNAV. DCF retains one local NAV update. Recipient-response checks remain valid. |
| C: Change B's mode after A's estimate but before B's exchange starts. | B uses its current selection; the future estimate does not fix it. |
| C: Record B's retained mode tag and ACK policy. Run A's forecast with a different mode on B's temporary copy. | B's retained tag and ACK policy remain unchanged immediately after the forecast. Permitted rate-control query effects remain intact. |
| C: Change rate-control state during B's RTS/CTS. | DATA executes its selected mode. The next exchange can select the updated mode. |
| C: Observe ACK estimates before RTS and after CTS with unchanged response-policy inputs. | Both estimates use the retained DATA mode and give the same ACK airtime. |
| C: Supply sufficient TXOP but insufficient TXNAV, then reverse the budgets. | The appropriate test refuses the additional exchange before its first PPDU. |
| C: Test `I + C` below, equal to, and above TXNAV at `t0`, with positive SIFS. | Equality proceeds under the declared interpretation. The fixture records `t0`; success also proves `C < TXNAVremaining(t0)`. |
| C: Test `C >= TXNAVremaining(t0)` and both mode choices from section 4.3. | Excess causes refusal. The unchanged example proceeds; the increased cost does not. |
| C: Test `I + C` below, equal to, and above the positive-limit remainder. | Equality proceeds; excess refuses the additional exchange without an optional overrun exception. |
| C: Give an initial DATA exchange too little time, with and without RTS. | The compatibility fallback permits transmission; it does not establish an IEEE overrun exemption. |
| C: Give an additional RTS/CTS/DATA exchange too little time. | HCF refuses before RTS. RTS's control type does not exempt its protected DATA. |
| C: Keep modes fixed. Supply sufficient TXOP time. A forecasts B's RTS/CTS, but TXNAV cannot contain B's complete RTS/CTS/DATA exchange. | A's field contains only the applicable estimate. HCF refuses B before RTS. B transmits after a fresh grant without a refusal-induced retry. |
| C: Execute the fixed-mode forecast example from section 4.5. | A reserves only its ACK. B remains queued and proceeds as an initial exchange after another grant. |
| C: Transmit group frame A with queued frame B and Block Ack support disabled. | The forecast excludes B because the group sequence cannot continue. A advertises zero; B needs a fresh TXOP. |
| C: Queue three voice frames from section 7.1, followed by a smaller voice frame. | At least three fresh grants transmit the oversized frames. The later frame also proceeds; budget refusal does not stop the queue. |
| C: Add a best-effort frame to that finite voice workload. | Best effort receives service within the fixed observation window. No ownership or reservation persists indefinitely after the finite voice workload. |
| C: Run the video and best-effort controls with the same frame and modes. | Both initial exchanges proceed. |
| C: Queue unrelated zero-limit units A and B with Normal Ack. | A reserves only its ACK. B waits for another TXOP and proceeds there. |
| C: Supply prepared fragments and a prepared A-MSDU through the current DATA/ACK sequence. | Positive-limit continuations obey cost admission. Zero-limit fragments use fresh TXOPs without a new companion policy. |
| C: Refuse additional DATA or RTS, then change DATA's mode before a later grant. | No drop, retry increment, failed-rate feedback, or response timeout occurs from refusal. The new grant selects the mode again. |
| C: Refuse DATA or RTS after a completed DATA/ACK exchange. Inspect the context in a finish observer. | The last step is the completed ACK. The refused step does not increase the observed count. RTS ownership ends; unsent DATA remains available. |
| C: Force immediate first-step refusal with valid channel ownership and active duration and step-count filters. Inspect the finish context. | One start precedes one finish with zero duration, zero steps, and a null last step. This proves the callback outcome, not production rejection of an initial exchange. |
| C: Test fractional, negative, maximum, and excessive fields. | Clamp, ceiling, and range validation have the specified results. |
| C: Run DCF, recipient responses, multiple protection, and Block Ack-enabled HCF. | Each uses its stated compatibility path. Shared TXNAV effects use the separate B assertions. |

The deferred follow-ups need their own direct tests before implementation. Those tests must cover BAR threshold forecasts and BAR-only availability, same-unit zero-limit companions, anticipated content, and all supported exception conditions. They must also refuse BAR after two DATA transmissions where the common one-frame restriction applies. No first-repair result establishes those deferred guarantees.

Use zero propagation for exact nominal boundaries. Add fixed 0.2 µs propagation cases for elapsed-budget and field-conversion effects. Do not require an inexact estimate to guarantee a peer's completion time. One fixed seed is sufficient for these deterministic conditions. Finite budget, mode, and AC variations supply the relevant boundaries.

Run these proposed commands from the checkout root after the selected test files exist:

```sh
make -j$(nproc) MODE=debug
./bin/inet_run_unit_tests -m debug --no-concurrent -f 'Ieee80211(SingleProtection_1|TxopProcedure_1|MultipleProtection_1)\.test$'
./bin/inet_run_module_tests -m debug --no-concurrent -f 'Ieee80211(HcfHeader_1|FrameSequenceObservations_1|SingleProtectionGroup_1|HcfSingleProtection_1|Txnav_1|HcfMultipleProtection_[12]|TxopProtectionSelectors_[12]|RecipientNav_1|HcfManagementRecovery_1|MgmtApHcfRtsTimeout_1)\.test$'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211
```

The current runner declares these options in [python/inet/main.py](../../python/inet/main.py). Each separate change selects its new fixture and directly affected existing fixtures. Preview the selected cases before execution. Report zero selected cases as not run. Record configurations, runs, seed, build mode, commands, results, and artifact paths.

[N_TxopBurst.test](../../tests/protocol/wifi/11n/N_TxopBurst.test) observes a coarse burst interval. It does not prove TXNAV or SIFS correctness. Use it only as supplementary regression evidence. Before publication, apply [run-the-gates.md](../../doc/project/guide/run-the-gates.md) with fresh debug and release builds.

Run directly affected legacy fingerprints after each source change. Include the QoS configurations with ordinary DATA, RTS/CTS, Block Ack, and no aggregation. Include rate control and DCF controls. Before publication of B and C, also run the broader recorded wireless campaign below from `tests/fingerprint`:

```sh
./fingerprinttest -d -t 1 -m '/(examples|showcases|tests)/.*(wireless|ieee80211|wifi)' -f 'tplx' -f '~tNl' -f '~tND'
./fingerprinttest -s -t 1 -m '/(examples|showcases|tests)/.*(wireless|ieee80211|wifi)' -f 'tplx' -f '~tNl' -f '~tND'
```

The recorded inventory reports 361 rows: 86 examples, 95 showcases, and 180 wireless-combo rows. This document revision does not verify that count. The selector uses recorded directory or command names; it does not establish coverage of every HCF configuration. Add directly affected cases outside this name filter when the configuration inventory identifies them. Recheck the inventory at each implementation head.

Compare the prerequisite and changed source with the same configurations, runs, seeds, ingredients, and build modes. Require the simulator's explicit fingerprint-success output for unchanged expectations. A mismatch needs its first divergence and a direct correctness check. It does not by itself establish a regression or justify a new value.

A1's expected field changes include the corrected single protection RTS Duration/ID when the branch selects Block Ack from a stale Normal Ack header. Check that emitted field against the direct A1 fixture before a fingerprint baseline proposal. The expected change does not establish the cause of every mismatch in a Block Ack configuration.

No fingerprint receives a new value under this plan alone. Separate intended field, contention, observation-order, and timer-event changes from regressions. Any baseline proposal needs exact affected rows, causal evidence, and approval under [change-a-baseline.md](../../doc/project/guide/change-a-baseline.md).

## 7. Support limits and implementation decisions

### 7.1 Progress and fragmentation boundary

[BasicFragmentationPolicy](../../src/inet/linklayer/ieee80211/mac/fragmentation/BasicFragmentationPolicy.cc) uses a configured byte threshold, not a TXOP budget. [OriginatorQosMacDataService](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosMacDataService.cc) prepares units before admission. The first repair preserves initial transmission progress and supports cost checks for prepared units in its stated scope. It does not add optional overrun predicates or fragmentation based on the TXOP budget.

Consider a hypothetical initial, individually addressed 1,500-byte MAC frame, including its header and FCS. It uses Normal Ack, no RTS, and no Block Ack agreement. Both DATA and ACK use legacy 20 MHz OFDM at 6 Mbit/s. Each OFDM symbol carries 24 data bits in 4 µs. The preamble and PHY header together require 20 µs; SIFS requires 16 µs.

The [OFDM duration calculation](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211OfdmMode.cc) includes service bits, tail bits, and symbol padding:

```text
DATA = 20 + 4 * ceil((16 + 8 * 1500 + 6) / 24) = 2024 us
ACK  = 20 + 4 * ceil((16 + 8 * 14 + 6) / 24)   =   44 us
DATA + SIFS + ACK = 2024 + 16 + 44             = 2084 us
```

[TxopProcedure](../../src/inet/linklayer/ieee80211/mac/originator/TxopProcedure.cc) supplies these default limits for OFDM:

| Access category | TXOP limit | Result for this initial exchange |
| --- | --- | --- |
| Voice, `AC_VO` | 1.504 ms | The initial fallback transmits the 2.084 ms exchange despite its excess. |
| Video, `AC_VI` | 3.008 ms | The exchange fits. |
| Best effort, `AC_BE` | 0 | The initial exchange proceeds; no additional exchange follows in this TXOP. |

The [default fragmentation threshold](../../src/inet/linklayer/ieee80211/mac/fragmentation/BasicFragmentationPolicy.ned) is 1,500 bytes. Fragmentation starts only above that threshold, so this exact frame remains whole. Strict initial cost refusal would retain it unchanged and refuse it at every fresh grant. The first repair instead transmits it through the initial compatibility fallback. An additional exchange with that cost still fails the applicable continuation tests.

The proposed module test queues three oversized voice frames and a smaller voice frame. It checks progress through at least three fresh grants and subsequent service for the smaller frame. A second case adds best effort traffic to that finite voice workload.

Use 100-byte MAC frames for the smaller voice frame and the best effort frame. Use fixed 6 Mbit/s DATA and ACK modes. Disable reception errors in this deterministic fixture. Observe all five frames for 10 ms after injection. Require completion of the finite voice workload and service for best effort within that window. Best effort can receive service before the final voice frame; the test imposes no additional AC order.

The source does not establish permanent blockage of another AC by the oversized voice frame. Each AC owns its queues and contention state. HCF releases ownership on normal sequence completion. The mixed-AC test must observe subsequent service, rather than assume permanent blockage as its expected result.

The fragmentation requirement in section 1.4 needs a separate data-service change that forms fragments from the applicable exchange budget. That change must preserve prepared-unit ownership and the current sequence and retry state. Keep the initial fallback until that repair proves progress for the oversized case. Verify the relevant RTS, rate, and unit conditions before removal. The first repair's tests establish its compatibility behavior, not compliance with the deferred fragmentation requirement. Release notes and MAC documentation must state this limit.

### 7.2 Other limits and decisions

Section 4.3 defines the chosen timer reference and containment test. A change to that interpretation requires corresponding changes to its boundary tests before implementation.

Current Block Ack lifecycle and peer response-rate limitations remain separate. The active originator expiry handler queues DELBA and retains the agreement until its current removal path runs. Deferred Block Ack overrun permission requires an accepted agreement with `getExpirationTime() > simTime()`; a zero timeout supplies `SIMTIME_MAX`. That permission check does not repair agreement retirement. A follow-up selection contract must not claim general agreement validity across new, unsupported lifecycle behavior.

MAC stop and crash handlers are currently empty. The proposed Rx expiry message must follow normal initialization and destruction ownership. This plan does not establish general stop/start, crash, or pending radio-command support. No lifecycle epoch or additional reset state belongs in this repair.

The first repair's implementation scope consists of changes A1 through C in section 5. Section 4.7 records deferred work only. The header records authorization and implementation status.

## 8. Standards and review evidence

All normative citations refer to IEEE Std 802.11-2024, document ID `ieee80211-2024`. Its source filename is `80211ax-2024.pdf`; the filename does not identify an amendment.

| Canonical node | Physical PDF pages | Source-span locator | Requirement |
| --- | --- | --- | --- |
| `ieee80211-2024:clause:9.2.5.1` | 709–710 | `ieee80211-2024@3280770:3283316` | Field conversion. |
| `ieee80211-2024:clause:9.2.5.2` | 710–712 | `ieee80211-2024@3283316:3295696` | Protection class and field-specific estimates. |
| `ieee80211-2024:table:9-9` | 670–671 | `ieee80211-2024@3099456:3101027` | Ordinary field range. |
| `ieee80211-2024:clause:10.3.2.9` | 1900–1903 | `ieee80211-2024@7893212:7909071` | CTS response conditions use NAV separately from TXNAV contention. |
| `ieee80211-2024:clause:10.23.2.2` | 1999–2001 | `ieee80211-2024@8315619:8323576` | Successful transmission, shared TXNAV, and its countdown origin. |
| `ieee80211-2024:clause:10.23.2.8` | 2010–2012 | `ieee80211-2024@8360326:8369678` | Strict continuation permission. |
| `ieee80211-2024:clause:10.23.2.9` | 2012–2014 | `ieee80211-2024@8369678:8378128` | Limits, content, overrun restrictions, and fragmentation. |

The clauses and table are normative; their notes are informative. The corpus is fresh. Its lint reports unrelated ambiguities and extraction warnings, so it is not globally clean. The six selected clauses have 9, 9, 14, 5, 15, and 10 extracted outgoing references, respectively; those references resolve. Clause 10.3.2.9 includes a resolved reference to 10.23.2.4 for the NAV holder condition. No source PDF inspection was necessary for the retrieved passages.

The source review covers component ownership, empty-history consumers, ACK-policy inputs across RTS/CTS, CTS queries, and the group sequence's finality limit. It also checks the absent sent-frame counter callers and the RTS field's use of the protected DATA policy. The standards evidence above supplies the normative conditions and declared interpretation. The plan preserves the initial fallback and the stated `t0` interpretation. No production source, test, baseline, or Git history changed. No build or simulation ran.

Document checks found no broken local links, missing linked anchors, trailing whitespace, or excessive sentence lengths in revised prose. Comparison with revision 11 confirmed unchanged admission budgets, initial fallback, command blocks, prerequisite wording, and deferred design details. The recorded fingerprint inventory requires a fresh preview before execution. Use the tracked `inet_process_standards` launcher with the explicit document ID to reproduce standards retrieval.

## Appendix A. Deferred design notes

This appendix preserves proposals for separate follow-up plans. Its implementation directions apply only if a later plan adopts them after review. Sections 3 through 6 define the current repair; this appendix does not extend that scope. The standards conditions remain constraints on any future design.

### A.1 Zero-limit content and overrun conditions

Use the completed transmit steps already owned by `FrameSequenceContext` to derive the DATA and management count. Count steps that reach radio completion, even if their required response later fails. An accepted transmit step records that completion; an accepted submission alone does not. Include the prospective candidate in each overrun check. A forecast or refused step adds no transmitted count. `TxopProcedure` retains ownership of the start and limit; no second mutable content counter is necessary.

For zero limits, derive the first transmitted unit's identity from that completed step's header and prepared representation. The permitted unit can be an MSDU, MMPDU, or prepared A-MSDU. Its current fragments retain the same unit identity. Required protection, acknowledgments, and eligible BARs remain separate companion frames under Clause 10.23.2.9.

A field forecast occurs before the current selection completes. HCF supplies that selection as anticipated content for the forecast's local content check. The check includes completed history, the current selection's uncompleted DATA or management transmission, and the prospective successor. Count each transmission once. Exclude the current selection from anticipated content once its transmit step records radio completion. Use its protected DATA or management frame when the current step is RTS.

If no DATA or management unit completed, the current selection supplies the zero-limit unit identity for this calculation. HCF passes the relevant frame through the local calculation; it does not mark that frame as transmitted. Actual admission continues to use completed history and its own prospective candidate. No retained counter or unit snapshot is necessary.

The handler keeps completed steps until sequence completion. `InProgressFrames` also retains dropped frames until that cleanup. Thus, those headers remain available throughout admission in the active sequence. Reuse that lifetime instead of a new unit snapshot or parallel identity record.

For example, A and B are unrelated MSDUs in a zero-limit TXOP. A uses Normal Ack. Before A completes, the forecast treats A as the permitted unit and excludes B. A therefore reserves only its immediate acknowledgment.

A later fragment of A can remain eligible, as can a BAR that the current ACK policy requires. Each additional exchange still passes the TXNAV test. Zero means a content limit, not a zero-time deadline.

For a positive limit, cost the whole selected sequence before its first PPDU. An initial ordinary DATA frame has no general overrun exemption. An RTS's control-frame classification does not authorize its protected DATA to exceed the limit. Overrun permission must apply to the selected exchange's content, not merely to its first frame type.

The common restrictions require at most one DATA or management transmission in the TXOP and prohibit a DL MU-MIMO PPDU. Supported exceptions must also prove these specific conditions:

| Exception | Additional evidence |
| --- | --- |
| Retransmission | The MPDU retains its initial size and satisfies the aggregate exclusions. |
| Initial MSDU under Block Ack | A valid agreement applies. The MSDU is not in an A-MSDU or multi-MPDU A-MPDU. |
| Control frame or QoS Null | The frame satisfies the applicable aggregate exclusions. |
| Group-addressed MPDU | The frame satisfies the applicable aggregate exclusions. |
| Nondynamic fragment | A previous fragment was retransmitted, or the MSDU/MMPDU has 16 fragments, as the clause specifies. |

Dynamic-fragment and S1G exceptions remain outside scope. Use a fragment exception only when the current representation proves its condition. The current fragment's Retry bit does not prove a previous fragment's history. Decline an optional overrun whose required evidence is absent; do not add speculative fragment-history machinery for it.

For retry-size evidence, trace the retained in-progress frame and the retry path. Reuse their unchanged-frame contract where it proves the size condition. Add an original-size record only if a supported mutation makes that evidence insufficient.

For example, BAR after two DATA transmissions cannot use the one-frame overrun exception. Likewise, an ordinary first DATA frame cannot borrow the exemption of its previous RTS. Both cases need direct refusal assertions.

For single protection, `HcfFs` must delegate budget admission rather than reject solely because `getRemaining()` is zero. Otherwise, it hides eligible zero-limit companions and optional overruns. Keep the multiple protection continuation policy separate. A budget refusal remains a normal end.

### A.2 BAR forecasts and availability

For example, a forecast whose next holder frame is BAR supplies BAR/Block Ack to the previous DATA equation. It does not supply another DATA frame. If the forecast selects RTS as that next frame, the field equation uses that frame and its response. Admission still costs the complete RTS/CTS/DATA sequence and can decline it. Do not inflate the field to force continuation.

For Basic Block Ack, the current DATA transmission can make the outstanding count reach the BAR threshold. Extend the current ACK-policy queries to accept that one anticipated frame for the forecast. Actual queries pass no anticipated frame. The policy combines it with its current outstanding frames locally. It owns the threshold and BAR parameter rules; HCF does not copy them.

The anticipated frame supplies one local calculation and has no retained forecast state. The policy needs no copy of future queue history. Update the policy contract and all implementations together. Preserve query effects permitted by the contract; a frame duplicate does not make rate selection read-only.

Check BAR eligibility before a DATA candidate is necessary. `HcfFs::selectHcfSequence()` and `TxOpFs::selectTxOpSequence()` must test BAR before they dereference a DATA candidate. The continuation and channel-request predicates must also recognize an eligible BAR when no unsent DATA remains. A refused temporary BAR frame can disappear, but its ACK-policy requirement must remain available for later access. Do not add a new BAR threshold or final-flush policy.

For example, a zero-limit DATA unit reaches the BAR threshold at radio completion. A follow-up must preserve the required BAR for subsequent access if its current reservation cannot admit it. A DATA placeholder must not determine whether that required BAR remains available.
