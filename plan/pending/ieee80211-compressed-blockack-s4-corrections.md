# S4 correction series

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [implementation status](ieee80211-compressed-blockack-replacement.md)

This series corrects the four findings from the audit of PR #1344. It preserves the final production source at `b716709335f79817542554822279461e61949938`. It adds protocol tests and their explanations. It separates Retry from transfer completion and places shared arithmetic before its callers.

## Scope and contract

HCF must set Retry when it retransmits standalone DATA after failed Block Ack feedback. IEEE Std 802.11-2024, 9.2.4.1.6 supplies this requirement. The parent ACK handler already makes that DATA eligible. The correction changes its header before Tx receives it.

HCF must resolve a final short transfer even without another DATA candidate. IEEE Std 802.11-2024, 10.25.4 requires completion DELBA. Basic remains an INET compatibility profile. This series does not claim complete current-standard Basic support.

The originator agreement owns cyclic distance, bitmap membership, and window advancement. HCF supplies unresolved identities. The final source already follows this contract. Only its introduction order changes.

## Proposed commits

### P0: Record the correction contract

Add this plan before the implementation commits. The document defines their responsibility, test tasks, and evidence limits. No protocol behavior changes. Protocol tests and a protocol reviewer document do not apply to this documentation commit. Check relative links and series rules.

### P1: Expose DATA preview without extraction

Retain the existing preview commit after P0. `IOriginatorQosMacDataService` exposes borrowed preview and prospective sequence lookup. `InProgressFrames` retains extraction as its default. The DATA-selection module fixture checks queue ownership, the unchanged header, and prospective sequence zero.

This structural commit changes no protocol behavior. New protocol tests and a protocol reviewer document do not apply. Build debug artifacts and run `Ieee80211BlockAckDataSelection_1.test`.

### P2: Mark DATA retries after failed Block Ack feedback

The parent can retransmit DATA with Retry clear after a BAR timeout or negative bitmap feedback. HCF already reads ACK status. Move the existing Retry branch into this independent commit. It changes no candidate selection, agreement completion, or reservation policy.

Add `tests/protocol/wifi/common/WifiBlockAckDataRetry.test`. Start with real association and production ADDBA. Send a continuous UDP stream through the existing infrastructure topology. Disable A-MSDU aggregation. Drop the first Block Ack to sta1 through the radio tap in run 0. Clear its bitmap in run 1. Capture the first Block Ack DATA sequence and TID before BAR. Require retransmission of that same identity with Retry set after BAR. The existing protocol suite has no direct check of this exact path.

The same fixture must fail before P2 because HCF leaves Retry clear. It must pass after P2 because HCF changes the transmitted header. Add the reviewer document `tests/protocol/wifi/blockack-data-retry.md` with P2. Add a guide and a before/after record under `doc/project/evidence/model/wifi/tests/`. Keep the failure honest; add no expected-failure declaration.

Build debug artifacts. Run `inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'WifiBlockAckDataRetry\.test$'`. Use seed 0. Check the seven existing changed fingerprint cases to determine P2's exact effects. The user approved five intermediate `~tND` values on 2026-10-22. Adopt those values with P2. Restore the accepted final values with P4.

### P3: Add shared originator window operations

Add the final agreement operations before their production callers. Move direct arithmetic assertions into a separate unit fixture. The operations have no production caller until P4. This preparation changes no packet trajectory. New protocol tests and a protocol reviewer document do not apply. Build debug artifacts and run the focused window unit fixture.

### P4: Complete Basic BAR transfer resolution

Apply the remaining behavior change through the shared agreement operations. HCF selects BAR before extraction. EDCA permits BAR-only contention. Matched feedback resolves the selected group. Final resolution or expiry starts DELBA. Duration forecasts reserve prepared successors only.

Add `tests/protocol/wifi/common/WifiBlockAckTransferCompletion.test`. Use real association, production ADDBA, and a finite UDP burst. Run a two-packet burst with an accepted window of 64 and a four-packet burst with a window of one. Keep the BAR threshold at five. Observe ADDBA success, Block Ack DATA, BAR, matched Block Ack, completion DELBA, and its ACK. The empty queue triggers the first case. A full window triggers the second case before further DATA. Existing module fixtures cover these paths, but no protocol fixture checks this complete sequence.

The fixture must fail before P4 because a short group lacks BAR or completion DELBA. It must pass after P4 through the production owners. Add `tests/protocol/wifi/blockack-transfer-completion.md` with P4. Add its guide and before/after record under `doc/project/evidence/model/wifi/tests/`. Use seed 0. Make no unsupported performance claim.

Build debug artifacts. Run both new protocol files through explicit filters. Run the seven changed module files and two existing unit files. Compare the seven fingerprint cases with P2 and the pinned final source. Attach each baseline change to its causal behavior commit. Preserve the final approved values.

The wider protocol selection also checks existing lifecycle fixtures. `WifiBlockAckBarWithoutAgreement.test` must retain its control agreement until the crafted BAR. Hold station contention at 1.002 seconds while queued DATA remains. This prevents completion before the 1.1-second probe without an agreement-state edit. Retain its response assertions.

The parent passes `WifiBlockAckSameApReassociation.test`, while S4's completion changes its traffic context. Overlap both traffic directions at 6 Mbit/s. Request reassociation at 1.008 seconds, before completion removes the old agreements. Extend fresh traffic until 1.4 seconds. Check fresh roles at 1.38 seconds. Keep the four-role, zero-role, fresh-setup, and UDP-delivery assertions. Disable Block Ack on sta2 and use small downlink packets to isolate the target link's agreements. Permit coincident transmissions as a warning because bidirectional contention can create them. This diagnostic setting changes no reception calculation.

### P5: Describe the corrected series and evidence

Update the current implementation documents, commit guide, protocol inventory, and verification record. Give each changed fingerprint row its cause and values. Distinguish fresh execution, source equality, existing evidence, and unavailable approval records. Do not assert an approval whose source is absent.

This documentation commit changes no protocol behavior. New protocol tests and a protocol reviewer document do not apply. Check links, document structure, series gates, source equality, and the final focused test contract.

## Verification limits

The proposed tests observe production MAC exchanges. The radio tap drops feedback only in the Retry test. No fixture supplies an accepted agreement. One fixed seed checks these deterministic paths; it does not establish probabilistic robustness.

Run each new fixture against its behavior commit and that commit's parent with identical inputs. Keep runtime failure separate from build or setup failure. Render diagrams when a local renderer is available. Report unavailable rendering directly.

The final source and final existing fingerprint values must equal the pinned source. New tests and documents are the intended final-tree additions. No baseline approval follows from permission to repair source or history.
