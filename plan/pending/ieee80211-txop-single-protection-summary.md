# TXOP single protection plan summary for human review

> **Kind:** reference · **Status:** snapshot 2026-10-07 · **Seal:** none · **Owns:** — · **Stands on:** [ieee80211-txop-single-protection.md](ieee80211-txop-single-protection.md)

This summary describes revision 12 of the [full plan](ieee80211-txop-single-protection.md), dated 2026-10-07. The user requested execution on 2026-10-07. Local implementation and focused verification are complete. The [verification report](ieee80211-txop-single-protection-verification.md) records results and remaining publication checks. The full plan owns the detailed conditions, tests, and standards evidence. All examples below are hypothetical.

## Intended result and scope

**HCF will check complete additional exchanges and preserve initial transmission progress.** HCF, the hybrid coordination function, coordinates frame exchanges. This repair covers enhanced distributed channel access (EDCA), which gives access categories different channel access parameters. A transmission opportunity (TXOP) gives a station the right to start frame exchange sequences within a time interval.

HCF currently checks positive TXOP time without the complete next exchange cost or an independent transmitted reservation budget. For example, 100 µs remains, but additional DATA and its acknowledgment need 180 µs before the initial wait. The new check refuses that exchange before transmission. HCF retains the unsent DATA for a fresh grant without a drop, retry increment, failed-rate feedback, or response timeout.

Admission applies to single protection with `isBlockAckSupported = false`. It covers DATA with Normal Ack, acknowledged management, prepared fragments, prepared A-MSDUs, and the current group-frame sequence. An A-MSDU combines service data units within one MAC frame. Ordinary request to send (RTS), clear to send (CTS), and nominal short interframe space (SIFS) remain supported. Single protection remains the default; this repair adds no group burst policy.

Multiple protection retains its equations and separate continuation policy. Block Ack reports reception status for multiple frames. A Block Ack request (BAR) requests that response. With `isBlockAckSupported = true`, HCF retains its existing admission even without an agreement. Its successor forecasts and holder-mode query points remain, subject to the independent A repairs. Shared header, observation, and TXNAV changes can affect both compatibility paths.

A3 repairs group-address classification throughout single protection, with Block Ack support enabled or disabled. C's release notes must state the admission flag dependency separately from that unconditional repair.

Basic Block Ack admission, BAR forecasts, zero-limit companions, optional overruns, and TXOP-based fragmentation require separate follow-up plans. The first repair changes no ACK-policy interface. General lifecycle and radio-command repairs remain separate. The [scope section](ieee80211-txop-single-protection.md#3-scope-and-acceptance-criteria) defines all exclusions; [Appendix A](ieee80211-txop-single-protection.md#appendix-a-deferred-design-notes) retains deferred proposals outside the implementation steps.

## Owners and change order

| Change | Responsible components and result |
| --- | --- |
| A1 | TxOpFs publishes the selected acknowledgment (ACK) policy. HCF uses the updated header. |
| A2 | The sequence handler notifies HCF and DCF after context installation, before the first step. Start and finish observations remain ordered. |
| A3 | SingleProtectionMechanism applies the group-address equation before the management and non-QoS DATA equations. The repair covers every single protection path. |
| B | Rx separates TXNAV from received NAV and the CTS query. Multiple protection and recipient NAV fixtures observe those states separately. |
| C | HCF checks complete continuations and retains selected DATA modes. SingleProtectionMechanism calculates their fields. The handler removes refused steps before normal completion. |

A1, A2, and A3 can share a repair pull request as separate commits. B and C require separate pull requests, with B before C. B starts from merged multiple protection and recipient NAV prerequisites before publication. Each change includes its direct tests and relevant documentation. The [implementation order](ieee80211-txop-single-protection.md#5-change-surface-and-implementation-order) identifies contract migrations and release notes.

A1 makes the sequence's ACK choice authoritative through the exchange. The current built-in policy's relevant inputs remain unchanged through successful RTS/CTS. For example, DATA that selects Block Ack before RTS retains that policy after CTS. Tests cover direct DATA and DATA after RTS/CTS. Custom policies must not depend on HCF's removed second DATA-submission query. A future agreement-lifecycle repair must reassess this boundary.

The agreement's sent-frame counter starts at zero, and its increment method has no caller in the tracked source. It therefore remains zero before RTS and after DATA in the built-in path. A1 leaves this separate accounting limitation unchanged. Its test preparation must recheck the callers at the implementation revision.

Earlier policy publication also affects the single protection RTS field. For example, Block Ack replaces a stale Normal Ack choice before RTS. The RTS estimate then omits the unnecessary immediate ACK airtime and its SIFS. A1 must check the emitted RTS Duration/ID and document this expected fingerprint change in its release note. The [selection design](ieee80211-txop-single-protection.md#42-select-the-current-exchange-once) records the source evidence and scope.

TXNAV is the holder's transmitted reservation timer, shared by its EDCA access functions. NAV, the network allocation vector, records a medium reservation from protocol duration information. Rx replaces TXNAV after each successful holder frame, with its endpoint at `holderPpduEnd + encodedDuration`. An accepted immediate response confirms success; a timeout does not. A frame without an immediate response succeeds at transmission completion.

For example, RTS ends at 100 µs with a 300 µs field. CTS confirms success at 160 µs, so TXNAV ends at 400 µs. A shorter successful field replaces that endpoint; a zero field clears TXNAV. TXOP completion does not clear TXNAV. Other access categories remain blocked until expiry, with normal contention intervals afterwards. DCF, the distributed coordination function, retains its local NAV behavior; recipient responses do not create TXNAV.

The CTS policies retain `isMediumFree()`, which checks the existing radio state and NAV without TXNAV. Rx supplies contention with a separate result that also requires expired TXNAV. For example, after TXOP completion, an addressed RTS arrives while TXNAV remains active, NAV is idle, and no access function owns the channel. With available radio state, the recipient sends CTS after SIFS while TXNAV still blocks local contention. An unrelated received NAV reservation still prevents CTS.

On budget refusal, the handler removes and destroys the refused step before the finish notification. It releases an owned RTS frame but retains borrowed unsent DATA. For example, DATA A and its ACK complete before refusal of B's RTS. A finish observer sees A's completed ACK as the last step; B remains available for another grant. Controlled first-step refusal leaves empty history. The [refusal design](ieee80211-txop-single-protection.md#46-refuse-synchronously-and-preserve-observations) defines that observation contract.

Source inspection confirms that the current finish handlers, result filters, and test override accept empty history. C's controlled refusal fixture establishes channel ownership. With duration and step-count filters active, immediate refusal reports zero duration and zero steps. The verification report records the successful runtime checks.

## Admission, field estimates, and compliance limits

Single protection sets Duration/ID from frame-specific estimates. A forecast determines that field without a commitment to transmit the future selection. HCF selects the current exchange after feedback and retains its DATA mode through RTS/CTS. A physical layer (PHY) mode specifies the format and transmission parameters. For example, a rate-control change during B's RTS/CTS does not replace B's admitted DATA mode; the next exchange can use the changed mode. Response airtime remains estimated, and a forecast copy does not suppress rate-query effects.

A3 retains a known group-frame limit. The current group sequence stops after one frame, but its forecast can still return queued frame B after group frame A. A3 removes A's incorrect ACK estimate while the unused reservation for B remains. C removes that ineligible estimate only with Block Ack support disabled. The limit remains with support enabled and must appear in release notes and MAC documentation. The [field design](ieee80211-txop-single-protection.md#45-calculate-durationid-fields) separates this limit from A3's classification repair.

At decision time `t0`, `I` is the next wait and `C` is the complete exchange cost after that wait. `C` includes holder transmissions, expected responses, PHY overhead, frame check sequence, and internal interframe spaces. A supported additional exchange requires a positive TXOP limit and both conditions:

```text
C < TXNAVremaining(t0)
I + C <= TXOPremaining(t0)
```

The plan samples both budgets after the previous exchange completes, before the next SIFS wait. For example, a 180 µs exchange passes with 195.6 µs of TXNAV after 0.4 µs round-trip propagation. Its initial 16 µs SIFS plus exchange must fit the positive TXOP remainder. Equality refuses TXNAV admission but passes positive TXOP containment. The timer sample instant remains an inference from Clause 10.23.2.8. The [budget section](ieee80211-txop-single-protection.md#43-define-admission-budgets) owns the equations and boundary interpretation.

**The initial exchange retains a compatibility fallback, even when it exceeds a positive limit.** This preserves progress but leaves the fragmentation compliance gap in Clause 10.23.2.9. For example, the full plan's 1,500-byte frame needs 2.084 ms at 6 Mbit/s, above the default 1.504 ms voice limit. Repeated initial refusal would retain that unchanged frame indefinitely. A separate fragmentation repair must prove progress before removal of the fallback. A zero limit permits only the initial exchange in this repair; additional-exchange overrun exceptions remain disabled.

## Verification and evidence limits

The [verification matrix](ieee80211-txop-single-protection.md#6-verification-and-expected-results) specifies production observations for budgets, retained modes, refusal, shared contention, and compatibility paths. It also requires these direct checks:

- A1 checks the emitted RTS Duration/ID as well as the subsequent DATA header and field after selection of Block Ack.
- After a forecast uses a different mode on a temporary copy, the retained frame's mode tag and ACK policy remain unchanged.
- A3 checks final group management and non-QoS group DATA without successors, with Block Ack support enabled and disabled. Each advertises zero without a current-frame ACK-mode query or an ACK timeout. Its successor-term test checks arithmetic only; it does not establish production group continuation.
- B checks CTS during active TXNAV and continued exclusion of a ready access category. The recipient NAV fixture requires zero old holder NAV updates for HCF and retains one for DCF.
- C checks finish observers after continuation refusal and controlled first-step refusal. The first-step case establishes channel ownership and runs the duration and step-count filters.
- With fixed modes, A's RTS/CTS forecast for B can leave insufficient TXNAV for B's complete exchange despite sufficient TXOP time. HCF refuses B before RTS and transmits B after a fresh grant.

The last case can reduce continuation within a TXOP. The field equation must not reserve extra time merely to force admission. Separate progress tests cover oversized voice frames and later service for another access category.

The verification report records a fresh debug build, four successful unit fixtures, and twelve successful module fixtures. Five legacy control rows verify their fingerprints. Five QoS rows complete with mismatches; the report identifies their first field and timer divergence. No baseline changed.

The local series contains seven commits with intermediate debug builds and direct tests. Publication requires the remaining release build, broader fingerprint campaign, separate pull requests, and publication gates. Baseline changes need exact affected rows, causal evidence, and separate approval. The [standards record](ieee80211-txop-single-protection.md#8-standards-and-review-evidence) identifies the IEEE clauses, corpus limitations, and source locations.
