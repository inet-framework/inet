# Implementation plan: production rate records

Checkout: `inet-ieee80211-rate-context`.
Original source baseline for the design: `eea6e438007795a5410cb7089cc709f3b151b91a`.
Reviewed implementation commit: `b279b995015183d30fcbcdcc34ecf292403e1740`.
Current implementation commit after the comment correction: `d93f95530b686dd25a50d6b651847cf00dbd7862`.
Reviewed implementation parent: `d00ff4ecf5065562153775d9332564874c0082e1`.

Design revision: 2026-10-04; direct query arguments and private target records.
Text revision: 2026-10-05; checked against the committed implementation and its direct tests.
Status: committed implementation; the user authorized execution on 2026-10-04.
Dependencies: none.

The original evidence includes source inspection, local IEEE 802.11-2024 retrieval, and the test results in section 6.
The original contract inventory uses the source commit above.
Section 6 records past implementation and verification results.
Section 7 records the current consistency review and fresh verification.
This review changes the plan only; it adds no source or test change.

The recorded authorization covers rate publication, context identity, management integration, and their direct tests.
It does not authorize control-rate rules, PHY response timing, TXOP admission, Block Ack, or A-MPDU integration.
A change to a recorded test expectation needs separate approval.
This text revision changes no recorded expectation.

## 1. Publish management rate records through the MIB

Management knows which rates the local station, the BSS, and each peer support.
Rate selection needs these facts through one local owner.
At the original design baseline, the MIB contains rate record types and a change notification.
Production management does not populate these records at that baseline.
The reviewed implementation now supplies the management publication paths that this plan describes.

This plan makes management publish accepted rate facts through `Ieee80211Mib`.
The MIB stores local, BSS, peer, and target records.
A target is the BSS that a management transaction selects before association completes.
A management transaction covers one authentication, association, or reassociation attempt.
The active BSS is the BSS of the current association.
The target must remain separate from the active BSS during reassociation.

A rate context identifies the applicable relationship and its rate facts.

A rate record must distinguish three states:

- **Unknown:** Management has no accepted information for the record.
- **Known empty:** Management has accepted information that contains no rates for the set.
- **Known nonempty:** Management has accepted information that contains rates for the set.

Unknown information must not become a known empty set.
Existing Supported Rates elements supply legacy rates and basic flags.
Existing HT capability and operation records supply HT rate facts.
The implementation reuses these inputs and the existing MIB operations.

This plan supplies facts and their identities.
It does not change rate-selection rules or TXOP admission.
It adds no PHY event or response calculation.
It needs no Block Ack or A-MPDU result.
It does not cancel queued originator transmissions when a rate reference becomes invalid.
The snapshot query rejects the invalid reference; transmission policy enforcement remains outside scope.

### Example: reassociation from AP A to AP B

A station has an active association with AP A.
Management starts a reassociation transaction with AP B.
The MIB must retain A's active rates while it stores B's target rates.
An incoming binding links an expected frame subtype and peer to a pending transaction.
A context reference identifies a relationship without a copy of its rate values.

1. Management validates B's rate advertisement.
2. Management installs B's target record and the expected incoming-frame bindings in the MIB.
3. Management attaches a reference to B's transaction frame before the frame enters the queue.
4. The MAC uses that reference to request B's rate facts from the MIB.
5. Management commits the new BSS relationship only at the existing successful association transition.

Frames for A continue to use A's active context while B remains the target.
If B's transaction fails, management removes B's target record and bindings.
The failure does not remove A's active relationship or an unrelated peer record.
An old queued frame must not acquire rates from a replacement transaction, even if the replacement uses the same BSSID.

## 2. Owners and contracts

Management accepts information from its configuration, lifecycle, and frame exchanges.
The MIB owns the stored records and their identities.
The MAC and management query the new records through the MIB.
The current rate policies read existing peer HT negotiation state; they do not yet consume the new rate-context snapshots.

Future rate policies must read the MIB rather than maintain a second writable cache of rate facts.
Remote lookup of rate facts remains in management.
Management owns any remote lookup that simplified management needs.

A snapshot is a copy of rate facts and identities that the caller owns.

This section defines the rate records, publication paths, context fields, and queries that this implementation needs.
The target record remains private to the MIB.
The query takes direct arguments and returns copied facts.

### Publication and validation

Management publishes local support, operational rates, and basic flags from its capability and advertisement policy.
Each management variant publishes the facts that it represents:

- AP management publishes BSS rates and accepted peer reports.
  The new legacy-rate check rejects requests that omit a required basic legacy rate.
  The existing HT negotiation checks the basic HT MCS set separately.
- STA management validates both Supported Rates elements before it changes the AP cache or the MIB.
  It keeps discovery information, target transactions, and the current association separate.
- Adhoc management publishes its independent BSS rate view.
- Simplified management publishes the facts of its configured association.
  Any remote peer lookup remains in management.

Invalid STA discovery advertisements follow the existing rejection path before an AP-cache or MIB rate update.
An invalid successful association response is dropped; the pending transaction waits for another response or timeout.
An AP can create a refusal response transaction without publication of the invalid request as accepted peer facts.
Invalid local configuration produces an explicit model error.
Legacy Supported Rates elements do not establish HT MCS support.

The existing rate types retain these fields:

| Type or field | Meaning and validation |
| --- | --- |
| `Ieee80211RateSet::known` | State whether the set contains accepted information. `false` requires empty rate containers. `true` permits an empty set. |
| `Ieee80211RateSet::legacyRates` | Store legacy rates in bps, or bits per second. Each value must be finite, positive, and representable in 500 kbps units. The wire limit is 63.5 Mbps. |
| `Ieee80211RateSet::htMcs` | Store explicit HT MCS indexes from 0 through 76. |
| `Ieee80211RateSetState::supported` | Store the rates that the owner supports. |
| `Ieee80211RateSetState::basic` | Store the basic rates that the applicable advertisement requires. |
| `Ieee80211RateSetState::operational` | Store the rates that the applicable operation permits. |

Local legacy support in this implementation reflects the represented legacy operational modes.
Management decodes its own Supported Rates elements into identical supported and operational legacy sets.
Mandatory flags select the basic legacy subset.
HT receive capability bits supply supported and operational MCS indexes.
Local basic HT MCS indexes come from mandatory MCS indexes that the local capability supports.
This representation adds no VHT, HE, or EHT MCS record.

The shared advertisement decoder requires 1–8 Supported Rates entries.
An Extended Supported Rates element that is present requires 1–255 entries.
It rejects duplicate rates across both elements, invalid counts, and rates outside the represented legacy range.
It commits the decoded result only after both elements pass validation.

The existing validation helpers check all three sets before a MIB update.
If the supported set is known, every stored basic and operational rate must belong to it.
If the basic and operational sets are known, every basic rate must belong to the operational set.
These checks apply to legacy rates and HT MCS indexes separately.
Management also checks frame validity and association requirements before it calls the MIB.

The publication path has these owners and triggers:

| Owner and trigger | Data change and commit point |
| --- | --- |
| `Ieee80211MgmtBase` receives a mode-set notification, reaches final initialization, or starts. | Publish local facts from the current advertisement and HT capability inputs through `setLocalRateSet()`. Without a mode set, publication returns without an update. |
| AP or independent-BSS management publishes local rates. | The shared base also publishes its BSS view through `setBssRateSet()`. AP initialization repeats publication after it establishes the BSSID. |
| `Ieee80211MgmtSta` accepts discovery information. | Decode both legacy rate elements before any AP-cache change. Retain validated rates in `ApInfo`. Publish non-current AP facts through `setPeerRateSet()`. Discovery alone does not replace the active BSS. |
| `Ieee80211MgmtSta` accepts a Beacon from the associated AP. | Refresh active BSS and peer rates with `installBssAndPeerRateSets()`. An associated AP's Probe Response updates discovery information without this active-rate commit. |
| `Ieee80211MgmtSta` selects a transaction target. | Copy the selected target's BSS and peer rates into a separate MIB target record. Keep the current association in `assocAP`. |
| `Ieee80211MgmtSta` accepts a successful association or reassociation response. | Commit association state and response rate facts together. For a valid HT response, retain basic HT MCS facts from the target snapshot. |
| `Ieee80211MgmtAp` completes a successful response exchange with an ACK. | Commit the saved pending peer rates, association state, and applicable HT facts. Clear pending transaction state before notification. |
| Adhoc management establishes its independent BSS. | Publish the BSS view from its advertisement policy. |
| Simplified STA management configures association at link-layer initialization, final initialization, or restart. | Read AP facts in management. Install BSS and peer rates once local support and AP basic rates are known. Reject incompatible basic legacy rates with a model error. |
| STA management clears its discovery list for a scan. | Remove discovery peer facts and pending targets. Preserve the active peer and active BSS rates. Notify after the discovery list is empty. |
| Replacement or disassociation ends one relationship. | Clear the corresponding transient records. Preserve records outside that transition. |
| Stop or crash ends the management lifecycle. | Clear all target, peer-rate, and BSS-rate records owned by that MIB. Keep local rate facts. Simplified STA cleanup also removes its peer facts from the AP. |

Management uses a separate peer update for accepted peer facts before association or after an accepted request.
A peer update must not replace the active BSS.
Management uses the combined BSS and peer update only when it commits a BSS relationship.
Unrelated discovery must not use that combined update.

### Identity and lifetime

A reference identifies the relationship that a frame uses.
It contains a context kind, BSSID, transaction identifier, and generation.
A transaction identifier distinguishes management transactions, even when they use the same BSSID.
A generation is a counter that changes when the applicable facts or relationship change.
Each management owner assigns transaction identifiers that remain unique in its MIB across stop and restart.
The MIB controls the generation.

The context kind has three values:

- `ACTIVE` identifies the active BSS relationship.
- `TARGET` identifies a selected target transaction.
- `NONE` identifies a procedure with no applicable BSS.

Unknown target information must not become `NONE`.
The tuned channel alone does not identify a BSS.
Target replacement with the same BSSID requires a new transaction identifier.
An active reference uses transaction identifier 0 and the active relationship generation.
Removal or replacement invalidates only the selected relationship.
This statement applies to individual record removal; stop and crash use bulk cleanup.

Management removes the matching targets and incoming bindings at these boundaries:

- Authentication failure or association failure.
- Cancellation or target replacement.
- Stop or crash.

Transaction removal clears only that target and its incoming bindings.
It does not automatically remove a peer record that came from valid discovery.
Disassociation clears the active BSS and that active peer's rate record.
Discovery cleanup removes discovery peer records while it preserves an active association.
Stop and crash clear all transient records in the affected MIB.
Local rate facts remain available for restart.

Deauthentication during pending authentication cancels the timeout and removes the target reference.
The STA reports one `PRC_REFUSED` authentication result to its agent.
A duplicate deauthentication frame produces no second result and no later timeout result.
The existing agent uses that refusal to resume its scan path.

### Frame path and notifications

Management installs the target and expected incoming bindings before the first transaction frame enters the queue.
Incoming management frames must match their actual transmitter and transaction.

A local tag carries the outgoing reference through queues, frame copies, and fragmentation.
An RTS frame copies the reference from the frame that it protects.
The MIB checks the peer, frame subtype, available BSSID, and optional explicit reference together.
A conflicting reference or an ambiguous incoming binding returns unknown context.
The query must not substitute another BSS's rates.

Management completes its related state before the MIB announces a rate change.
A listener must see the complete relationship when it reads the MIB during notification.

Response-policy integration remains outside this implementation scope.
That integration will copy the applicable facts into a response context for mode selection.
A copied originator snapshot must not authorize transmission after its relationship expires.
Future HCF integration will subscribe through the bool listener overload for `rateStateChanged`.
It will invalidate affected future originator steps when rates or relationships change.
It must preserve an accepted required response under its protected context until normal completion or abort owns cleanup.

The MIB query returns unknown context when required identity information is absent or inconsistent.
Future response policies must report `UNSUPPORTED` before a dependent transmission if required target or BSS rate facts remain unknown.
`UNSUPPORTED` means that the policy cannot select a permitted mode for the supplied context.
The query does not choose a mandatory-rate fallback.
The response policy permits fallback only when the applicable frame's standard rule permits it.
Known information alone does not grant fallback permission.

### 2.1. Existing classes and contracts to reuse

The paths below are relative to `src/inet/linklayer/ieee80211/` unless stated otherwise.
“Extend” means that the class exists but needs the behavior in this plan.
This plan adds no abstract interface, NED module, or NED moduleinterface.

| Existing class or contract | Action | Role in this plan |
| --- | --- | --- |
| `Ieee80211Mib` in `mib/Ieee80211Mib.h` | Extend | Store validated local, BSS, peer, and target facts. Supply typed updates and queries. |
| `Ieee80211RateSet`, `Ieee80211RateSetState` in `mib/Ieee80211RateSet.h` | Reuse | Represent supported, basic, and operational rates. Preserve `known`, legacy rates, and HT MCS sets. |
| `validateIeee80211RateSet()`, `validateIeee80211RateSetState()` in the same header | Reuse | Check rate values and subset consistency before a MIB update. Management also checks advertisement validity and association requirements. |
| `Ieee80211MgmtBase` | Extend | Publish local facts from the existing capability and advertisement inputs. |
| `Ieee80211MgmtApBase`, `Ieee80211MgmtAp` | Extend | Publish AP BSS rates and validated peer reports. Preserve the existing association commit point. |
| `Ieee80211MgmtSta`, with `ApInfo` and `assocAP` | Extend | Separate discovery, target transactions, and the current association. Publish accepted facts at the corresponding transition. |
| `Ieee80211MgmtAdhoc` | Reuse inherited publication | The shared management base publishes the independent BSS rate view. This class needs no source change. |
| `Ieee80211MgmtApSimplified`, `Ieee80211MgmtStaSimplified` | Reuse AP inheritance; extend STA | The AP inherits publication through its base. The STA adds association rate publication and peer cleanup. Remote lookup remains in management. |
| `Ieee80211SupportedRatesElement`, `Ieee80211ExtendedSupportedRatesElement` in `mgmt/Ieee80211MgmtFrame.msg` | Reuse | Supply legacy rates and basic flags from existing management frames. Preserve their wire format. |
| `Ieee80211HtCapabilities`, `Ieee80211HtOperation` in `mib/Ieee80211HtCapabilities.h` | Reuse | Supply applicable HT rate facts. Legacy Supported Rates elements do not establish HT MCS support. |
| `Ieee80211MgmtTransactionTag` in `mgmt/Ieee80211MgmtTransactionTag.msg` | Reuse unchanged | Identify association-response outcomes. This tag does not replace the rate-context reference. |
| `Ieee80211Mac`, `RtsTransmitStep` | Extend | The MAC resolves the management BSSID from a tagged reference and removes sender-local metadata at reception. The RTS step copies the protected frame's tag. |
| `Fragmentation`, `RtsProcedure` | Reuse unchanged | Fragmentation already copies all tags. The RTS procedure builds the header; the RTS step owns the generated frame and its tag. |
| `IRateSelection`, `IQosRateSelection` in `mac/contract/` | Future consumers; no change in this commit | The current policies read existing peer HT state. Future response-policy integration adds `snapshotResponseRateContext()` and constructs an owned `ResponseRateContext` from the MIB snapshot. |

Reuse these public `Ieee80211Mib` operations:

| Existing operations | Required behavior |
| --- | --- |
| `setLocalRateSet()`, `setBssRateSet()` | Validate the corresponding rate state before publication. |
| `installBssAndPeerRateSets()` | Commit accepted BSS and peer states together. Unrelated discovery must not use this operation. |
| `getLocalRateSet()`, `getBssRateSet()`, `findPeerRateSet()` | Read current facts. A borrowed view remains valid only until the corresponding update or clear operation. |
| `clearBssRateSet()`, `removePeerRateSet()`, `clearPeerRateSets()` | Remove facts at the corresponding relationship or lifecycle boundary. Preserve unrelated records. |

A borrowed view refers to data that the MIB owns.
A caller that needs facts across events must use an owned snapshot instead.

### 2.2. New context types and local tag

Add `mib/Ieee80211RateContext.h` for the two shared value types below.
Keep validation helpers and rate-selection policy out of their declarations.

| New type | Contents and owner | Consumer and purpose |
| --- | --- | --- |
| `BssRateContextRef` | The kind, BSSID, transaction identifier, and generation identify one context. Management assigns the identifier. The MIB controls the generation. | Management, the local tag, and MIB queries distinguish active and target relationships. The identifier also distinguishes replacements with the same BSSID. |
| `RateContextSnapshot` | The value contains copied local, applicable BSS, and peer facts, with their identities and generation. The caller owns the value. | MIB readers retain facts across events. Future response-policy integration constructs `ResponseRateContext` without a mutable management pointer or a borrowed view that outlives an update. |

The shared values contain these fields:

| Value and field | Type | Meaning |
| --- | --- | --- |
| `BssRateContextRef::kind` | `Kind` | Identify `ACTIVE = 0`, `TARGET = 1`, or `NONE = 2`. The default is `NONE`. |
| `BssRateContextRef::bssid` | `MacAddress` | Identify the applicable BSS. |
| `BssRateContextRef::transactionId` | `uint64_t` | Identify the management transaction. `ACTIVE` and `NONE` use 0; a new target requires a nonzero identifier. |
| `BssRateContextRef::generation` | `uint64_t` | Identify the committed relationship version. The default is 0 before target installation. |
| `RateContextSnapshot::known` | `bool` | State whether the query resolved the applicable context. The default is `false`. This field does not make every copied rate set known. |
| `RateContextSnapshot::context` | `BssRateContextRef` | Retain the resolved relationship identity. |
| `RateContextSnapshot::localAddress`, `peerAddress` | `MacAddress` | Identify the local station and the peer of the query. |
| `RateContextSnapshot::generation` | `uint64_t` | Retain the MIB rate generation at the time of the query. The default is 0. |
| `RateContextSnapshot::localRates`, `bssRates`, `peerRates` | `Ieee80211RateSetState` | Retain copied local, applicable BSS, and peer facts. Each set retains its own known state. |

Reference equality compares the kind, BSSID, transaction identifier, and generation.
The snapshot's MIB generation and its relationship generation have separate roles.
A change to unrelated MIB facts can advance the MIB generation without replacement of this target reference.

A snapshot owns its copied facts, so it remains valid after the MIB removes the source record.

The MIB stores each target in a private nested record, `TargetRateContext`.
The record retains copied target BSS and peer states, their identities, and generation across events.
Its fields are `ref`, `peer`, `bssRates`, and `peerRates`.
The MIB stores incoming bindings by peer and expected frame subtype, with references to the applicable target records.
Management updates or removes the record through public MIB operations.

Readers receive an owned `RateContextSnapshot`.
No public operation exposes the stored target record or a pointer to it.

The MAC passes query arguments directly to `snapshotRateContext()`.
The arguments are the peer, frame subtype, optional BSSID, and optional explicit reference for the request role.
They need no separate lookup type because they own no persistent state and enforce no additional rule.
The MIB checks the arguments together.
It returns unknown context for a conflicting reference or an ambiguous relationship.

Add `mgmt/Ieee80211RateContextTag.msg` to declare `Ieee80211RateContextTag` as a `TagBase` subclass.
Its fields are `contextKind`, `bssid`, `transactionId`, and `generation`.
The kind defaults to 2, which represents `NONE`; both counters default to 0.
The tag carries reference fields through management queues, frame copies, fragmentation, and RTS creation.
It carries no rate values and changes no wire field.

The MAC extracts the reference for the snapshot query.
The receiver resolves its own context from frame fields and local state.
The receiver must not use the sender's local tag.

Keep `Ieee80211MgmtTransactionTag` separate.
That existing tag identifies association results; the new tag identifies the applicable rate context.

### 2.3. New MIB operations

The new operations let management update peer facts without a BSS replacement.
They also let management install a target, link expected incoming frames to it, and remove it.
The query returns copied facts after it checks the supplied identities.

Add these public operations to `Ieee80211Mib`:

```cpp
void setPeerRateSet(const MacAddress& peer, const Ieee80211RateSetState& state);
void installTargetRateContext(const BssRateContextRef& ref,
        const Ieee80211RateSetState& bssRates, const MacAddress& peer,
        const Ieee80211RateSetState& peerRates);
void removeTargetRateContext(const BssRateContextRef& ref);
void clearTargetRateContexts();
void bindIncomingRateContext(const MacAddress& peer, int requestSubtype,
        const BssRateContextRef& ref);
RateContextSnapshot snapshotRateContext(const MacAddress& peer, int frameSubtype,
        const std::optional<MacAddress>& bssid,
        const std::optional<BssRateContextRef>& explicitContext) const;
```

| Operation | Required behavior |
| --- | --- |
| `setPeerRateSet()` | Validate peer facts before publication. Preserve the active BSS. |
| `installTargetRateContext()` | Validate target facts before storage. Store copies of the facts. |
| `bindIncomingRateContext()` | Record which pending transaction applies to an expected incoming frame. |
| `removeTargetRateContext()` | Remove the matching target and its incoming bindings. |
| `clearTargetRateContexts()` | Remove all target records and incoming bindings from this MIB. |
| `snapshotRateContext()` | Return an owned copy. Return unknown context when identity checks fail. |

An absent `bssid` means that the request supplies no BSSID for the query.
An absent `explicitContext` requires resolution from frame fields and the applicable local relationship or incoming binding.
Neither absence permits another BSS's rates or a change from unknown context to `NONE`.
Queries do not change records or emit simulation signals.
Management completes related state before an update operation announces the completed change.

Management creates a new target reference with generation 0.
The MIB assigns the committed generation at installation.
Management installs the expected incoming binding with that transaction identity.
Management obtains the committed reference from the snapshot query before it attaches the reference to an outgoing frame.
Removal uses the committed reference with its generation.

A stale or generation-0 removal reference does not remove a replacement or the committed target.
Generation-0 references are accepted for a new target and its initial incoming binding, not for a committed snapshot query.

The snapshot query follows these identity rules:

- An explicit reference must match the current record, peer, and available BSSID.
  A target reference must also match an applicable incoming binding.
- Association and reassociation requests and responses belong to their respective paired exchanges.
  The query considers the applicable request and response subtype bindings together.
- A query without an explicit reference can use one unambiguous incoming binding.
  Conflicting bindings return unknown context.
- An outgoing RTS retains the protected frame's reference.
  An incoming RTS uses one applicable local binding or the active BSS relationship.
  Multiple applicable target bindings return unknown context.
- A wildcard Probe Request with no selected BSS can use `NONE`.
  It requires an explicit default reference, a broadcast peer, and no supplied BSSID.
  Unknown target information cannot use this rule.
- Active context requires an applicable local BSS relationship and a matching available BSSID.
  A discovered AP or tuned channel alone is insufficient.

The update paths also need bulk lifecycle cleanup and one notification after related state changes complete.
Use `clearTargetRateContexts()` to clear target records and their bindings at the applicable lifecycle boundary.
Use `Ieee80211Mib::RateUpdate` to defer notification until management and MIB state form a complete view.
The update scope changes no simulation signal payload.

Nested update scopes defer notification until the outer scope ends.
Each actual MIB change can advance the MIB generation within that scope.
The scope emits one notification for the completed changes; it does not reduce them to one generation increment.

### 2.4. Existing signals and their use

This plan adds no simulation signal or signal payload class.
Updates and queries use direct MIB calls.
The existing simulation signals retain these roles:

| Existing simulation signal | Source and payload | Use in this plan |
| --- | --- | --- |
| `rateStateChanged`, C++ identifier `Ieee80211Mib::rateStateChangedSignal` | The local MIB emits `bool true`, with no details or ownership transfer. `Ieee80211Mib.ned` already declares the simulation signal. | Notify changes to committed peer and target facts, applicable bindings, and relationship removal or replacement. |
| `modesetChangedSignal` | `Ieee80211Mac` supplies a borrowed `Ieee80211ModeSet` object. `Ieee80211MgmtBase` already subscribes at the interface that contains it. | Preserve the existing advertisement input path. Add no second simulation signal for the mode catalog. |
| `frameTransmissionOutcome`, C++ identifier `Ieee80211Mac::frameTransmissionOutcomeSignal` | The local MAC supplies a borrowed `Packet` and `FrameTransmissionDetails`. `Ieee80211MgmtAp` already subscribes. | Reuse the association-response result path and transaction identity to commit or discard pending peer facts. |

The MIB advances the applicable generation before it emits `rateStateChanged`.
The same existing notification also covers local rates, HT capability state, and channel-operation changes.
The MIB and related management state must form one consistent view before notification.
An unchanged advertisement alone causes no rate notification.
A relationship replacement remains a change even when its rate values match.

Listeners query the committed MIB state.
Listeners must not complete the publisher's transition.
This plan's module test uses a listener to check the state visible during notification.
The HCF subscription and cancellation of future steps remain outside this implementation scope.
Existing association outcome signals retain their roles and do not replace `rateStateChanged`.

## 3. Implementation series

The implementation has three steps.
Each step uses the existing rate records before it adds another representation.
The local reference has consumers in management, MAC header construction, frame copies, and RTS creation.
The rate-selection policies do not yet consume this metadata.
The scope excludes a general rate cache and new configuration parameters.

### Step 1: publish local, BSS, and peer facts

**Purpose:** Make accepted rate facts available through one owner.

**Owner and files:** Management publishes the facts; the MIB stores them.
The files include `mib/Ieee80211Mib.*`, `mib/Ieee80211RateSet.h`, `mgmt/Ieee80211MgmtBase.*`, and the applicable management subclasses.

**Change:** Validate actual configuration and accepted frame inputs before publication.

**Result:** Full and simplified management publish the same known facts for the association that each represents.

**Verification:** Compare initialization paths and accepted advertisements.
Check unknown, known empty, and known nonempty sets.
Check rejection of malformed advertisements and unsupported required rates.

### Step 2: preserve target identity

**Purpose:** Keep the target's rate facts separate from the active BSS during a queued transaction.

**Owner and files:** Management owns transactions; the MIB owns target records.
The affected paths include management transaction owners, the local tag, and queue and copy consumers.

**Change:** Install target records and incoming bindings before queue entry.
Carry the reference through frame copies, conventional fragmentation, and RTS creation.

**Result:** A transaction for AP B never uses AP A's active rates.

**Verification:** Keep A active while a queued transaction targets B.
Check incoming bindings, reference copies, conflicting references, and replacement with the same BSSID.
Check that a snapshot remains valid after target removal.

### Step 3: remove transient facts at relationship and lifecycle boundaries

**Purpose:** Ensure that rate queries for old frames reject a removed or replaced relationship.

**Owner and files:** Management and MIB removal paths own the cleanup.
The paths include transaction failure, disassociation, replacement, stop, and crash.

**Change:** Remove the matching transient records and bindings.
Advance the applicable generation before notification.

**Result:** The removed reference no longer resolves.
Transaction cleanup preserves unrelated facts; lifecycle cleanup clears all transient facts in its MIB.

**Verification:** Check failure, cancellation, lifecycle cleanup, and peer isolation.
Check the complete state from a listener during notification.

## 4. Acceptance and tests

The new test is `tests/module/Ieee80211TargetRateContext_1.test`.
Sections 6 and 7 distinguish its past results from current verification.
The test uses the real management and MIB owners.
Its integration case obtains published rates from management rather than a test fixture setter.

Separate MIB-only probes install fixture records for empty sets, unknown facts, and ambiguous bindings.
The management exchange uses a controlled queue that replaces `sendDown()`.
The fixture supplies frame delivery and acknowledged-response outcomes directly.
It does not exercise the actual MAC queue, radio reception, or sender-tag removal in that exchange.
The HT association test separately exercises the radio path and asserts published legacy and HT facts.

| Condition or trigger | Required observation |
| --- | --- |
| Full management and simplified initialization represent the same association. | Both publish the corresponding known rate facts. |
| AP A remains active while a queued transaction targets AP B. | B's queued frames retain B's reference, and rate queries resolve B's target facts. A's active context remains separate. |
| A direct frame-copy, fragmentation, or RTS-step probe runs. | Each copy retains the applicable reference. RTS uses its protected frame's reference. These probes do not establish radio delivery. |
| An incoming frame matches a pending transaction. | The MIB resolves the context from the peer, subtype, available BSSID, and binding. |
| A reference conflicts or incoming bindings are ambiguous. | The MIB returns unknown context rather than another BSS's rates. |
| Management replaces a target with the same BSSID. | The new identifier prevents an old reference from resolving the replacement. An owned snapshot retains its copied facts after removal. |
| A rate set is unknown, known empty, or known nonempty. | The MIB preserves the distinction. |
| Stop/restart or crash ends the management lifecycle. | All transient facts in that MIB end. Local facts remain available, and new transaction identifiers exceed old identifiers. |
| Disassociation ends the active relationship. | Active BSS and active peer rates disappear. An unrelated discovered peer can remain. |
| Authentication or association fails, or management cancels a transaction. | The matching target and bindings disappear. |
| A discovery advertisement is malformed. | The STA rejects it before an AP-cache or MIB rate update. |
| An association request lacks a required basic legacy rate. | The AP sends a refusal response instead of a successful response. It does not commit accepted peer rates from that request. |
| The MIB emits its change notification. | A listener sees completed management and MIB state. |
| Management discovers an unrelated BSS. | Discovery does not replace the active BSS. |
| A scan clears discovery information. | Discovery peers and pending targets disappear. The active peer and BSS facts survive. A listener sees the empty discovery list. |
| A same-AP reassociation target exists. | Both request and response queries use the target binding. An explicit active reference fails for those subtypes, while active data queries still succeed. |
| Deauthentication cancels pending authentication. | The timeout and target disappear. The agent receives one refusal and resumes its scan. Duplicate frames produce no second completion. |

### Build and static checks

Run all commands from `/home/user/omnetpp_ws/inet-ieee80211-rate-context` unless a command names another directory.
Build the debug library after any compiled source or generated-code input changes.
The tests must load a library from the source revision under verification.
A documentation-only change requires no INET rebuild.

Run these checks before the focused tests:

```sh
make MODE=debug -j$(nproc)
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-interfaces.sh
git diff --check
doc/project/enforcement/check-source-seals.sh --base d00ff4ecf5065562153775d9332564874c0082e1
```

The seal check uses the parent of the reviewed rate-context commit.
This scope excludes the earlier TXOP and QUIC commits on the branch.
The affected IEEE 802.11 source paths have no listed seal at the review revision.
The common Packet subsystem has a seal; this implementation uses its public API without a change to that subsystem.

### Focused test commands and result records

Use `General`, run 0, and seed 0 for the new deterministic module case.
Preserve the declared configurations and seeds of existing cases.
Run these focused tests after implementation:

```sh
inet_run_module_tests -m debug -f '(Ieee80211TargetRateContext_1|Ieee80211MgmtStaSimplifiedInitialization_1|Ieee80211MgmtApReassociationSnapshot_1|Ieee80211MgmtStaLifecycle_1|Ieee80211HtAssociation_1|Ieee80211MgmtSta(BeaconUpdate|Deauthentication|Disassociation)_1|Ieee80211AgentStaReassociation_1)\.test'
inet_run_module_tests -m debug -f '(Ieee80211MgmtAp(Timeout|HcfRtsTimeout|QueueDrop|Lifecycle|ChannelChange|HcfQueueDrop|MalformedHtCap)_1|Ieee80211HcfManagementRecovery_1)\.test'
inet_run_unit_tests -m debug -f '(Ieee80211SupportedRates_1|Ieee80211MibPeerTeardown_1|Ieee80211MgmtStaPrimitiveDispatch_1)\.test'
```

These tests establish publication and context identity.
They establish no rate-selection or TXOP result.
The first module command selects nine cases, including same-AP binding, agent recovery, lifecycle, and radio publication checks.
The second selects eight cases whose management fixtures now supply valid rate advertisements.
Together they include all 16 module test files changed by the rate-context commit and one agent regression.
The unit command selects three cases, including the changed primitive-dispatch fixture.

Run the affected infrastructure and ad hoc fingerprints from `tests/fingerprint`:

```sh
./fingerprinttest -d -m 'examples/(wireless/lan80211|adhoc/ieee80211)/.*Ping1' -f tplx -f '~tNl' -f '~tND'
```

Record these details for every verification run:

- Source revision and relevant local changes.
- Command directory, exact command, selector, and build mode.
- Selected and executed case counts.
- Configuration, run number, and seed where applicable.
- Build status, test exit status, and the actual case results.
- Log paths and test artifact paths.

A zero-case selection supplies no test evidence.
A dry run supplies no executed-test evidence.
Report setup errors, expected failures, unexpected failures, and passes separately.
Separate existing failures from changes that this implementation introduces.
Record the compared source revisions and the cause of an existing failure.
Shared-source changes require direct regression checks for the affected existing consumers.

A passing test does not authorize a change to a recorded expectation.
Keep any approved expectation change separate from the result of the verification run.
Static checks do not prove frame ownership, notification order, or correct lifecycle cleanup.
The direct module and unit cases must check those behaviors.

### Checks before publication

Before a source change is pushed, compile both debug and release modes.
Run the project-wide checks against the actual publication base.
The commands below use `origin/master` as that base:

```sh
make MODE=debug -j$(nproc)
make MODE=release -j$(nproc)
doc/project/enforcement/check-architecture.sh
doc/project/enforcement/check-naming.sh --base origin/master
doc/project/enforcement/check-commits.sh origin/master..HEAD
doc/project/enforcement/check-classification.sh origin/master..HEAD
doc/project/enforcement/check-interfaces.sh
doc/project/enforcement/check-source-seals.sh --base origin/master
```

Rerun each recorded focused test command against a current debug library before publication.
Release mode is also a compilation check; release-mode behavioral tests are necessary if the changed behavior depends on build mode.
This plan's past results do not establish a pass for a later source revision.

## 5. Standard and compatibility limits

IEEE Std 802.11-2024 supplies the protocol requirements.
The local standard collection identifies `80211ax-2024.pdf` as that base standard, with the document identifier `ieee80211-2024`.
The filename does not identify an amendment.

| Clause | Local standard identifier and locator | Relevance to this plan |
| --- | --- | --- |
| 6.5.4.2.2 | `ieee80211-2024:clause:6.5.4.2.2`; physical PDF pages 487–490 | `SelectedBSS` identifies the BSS to join. Discovery and association remain separate. A pending target must not replace the active relationship. |
| 9.4.2.3 | `ieee80211-2024:clause:9.4.2.3`; physical PDF page 936 | Supported Rates and BSS Membership Selectors describes the advertisement input. This implementation decodes represented legacy rates and basic flags. |
| 9.4.2.11 | `ieee80211-2024:clause:9.4.2.11`; physical PDF page 951 | Extended Supported Rates and BSS Membership Selectors describes the second advertisement input. It carries 1–255 octets when present. |
| 11.3.5.3(f), (g) | `ieee80211-2024:clause:11.3.5.3`; physical PDF pages 2528–2531 | The new check covers required basic legacy rates. Existing HT negotiation checks basic HT MCS support. This implementation does not add membership-selector or newer-amendment admission checks. |
| 10.6.6.2 | `ieee80211-2024:clause:10.6.6.2`; physical PDF pages 1943–1944 | Initial control-rate selection consumes BSS basic-rate facts. The selection procedure remains outside this implementation scope. |
| 10.6.6.4 | `ieee80211-2024:clause:10.6.6.4`; physical PDF pages 1944–1945 | Later control frames use their applicable rate rules. This plan supplies facts but implements no control-rate calculation. |
| 10.6.6.5.2 | `ieee80211-2024:clause:10.6.6.5.2`; physical PDF pages 1945–1947 | Response-rate selection needs the applicable BSS rate facts. Its rate, modulation, and preamble procedure remains outside this implementation scope. |

These clauses supply normative protocol requirements.
The MIB records, generation counters, local tag, and query rules are INET implementation mechanisms.
The separation of active and target records implements the selected-BSS requirement within the model.

The query reports context and facts; it does not grant permission to use a rate.
Preassociation traffic uses the selected target's validated facts when that BSS applies.
Response preparation must not promote a pending target into the active BSS.

Preserve Supported Rates construction and the existing association commit points.
Association admission now rejects requests that lack required basic legacy rates.
Pending authentication now completes once when deauthentication cancels it.
Document new metadata and snapshot declarations in the migration guide when they affect external consumers.

Keep shared context declarations free of rate-selection policy and validation helpers.
Keep the private target record behind MIB operations.
Add no abstract interface or NED moduleinterface for this change.
Any later interface extension must declare its role without helper bodies or a default policy.
Put shared implementation behavior in an implementation class rather than an interface.

Block Ack and A-MPDU results remain outside this plan's prerequisites.

## 6. Implementation and verification record

This section retains the original implementation record and its historical results.
The user authorized the plan with “Execute plan ieee80211-rate-context.md”.
The record used the original design baseline in the active checkout.
The changes remained in the working tree at the time of that record.
The results below describe those runs; section 7 supplies evidence for the reviewed implementation commit.

### Implementation contract and result

- **Owner:** Management supplied accepted facts.
  The MIB owned committed rate records, references, and generations.
- **Entry path:** Mode publication supplied local rates.
  Management discovery, authentication, association, and lifecycle paths supplied relationship facts.
- **Consumers:** The MAC resolved an outgoing reference for the management BSSID.
  RTS steps copied that reference from the protected frame.
- **Frame ownership:** Management transferred outgoing frames to the existing queue.
  Fragmentation used the existing tag-copy operation.
  Reception removed sender-local metadata.
- **Removal paths:** Authentication failure, association refusal, timeout, cancellation, replacement, disassociation, shutdown, and crash removed their matching state.
- **Identity and units:** References contained the BSSID, transaction identifier, and generation.
  Legacy rates used bps, or bits per second.
  HT rates used MCS indexes.
- **Verification:** The new module case exercised real management owners through a controlled queue.
  The HT association case exercised the radio path.

The source check before implementation confirmed the owners, entry paths, and existing test selectors.
The implementation added the target test to cover the missing transaction cases.
The final self-audit covered C++, OMNeT++, INET Packet behavior, and IEEE 802.11 management.
It checked copied snapshots, peer isolation, notification order, lifecycle cleanup, and both DCF and HCF regression paths.

The MIB implementation stored private target records and expected incoming bindings.
The snapshot query rejected conflicting peers, BSSIDs, subtypes, generations, and ambiguous bindings.
An owned snapshot remained valid after target removal.
`NONE` applied to a wildcard Probe Request with no selected BSS.
Unknown target information did not select the active BSS.

Management validated both legacy rate elements before an STA cache update.
The AP checked required basic legacy rates before association success.
HT rate facts came from HT capability and operation records.
Association response publication used the target snapshot for its basic HT MCS set.
Local publication required capability data but did not require an STA channel record.

### Required additions to the change surface

The implementation added an update scope, `Ieee80211Mib::RateUpdate`, to combine related changes into one notification after state completion.
It added no simulation signal or payload type.
The implementation also added `clearTargetRateContexts()` for bulk lifecycle cleanup.
A shared management helper converted and validated existing advertisement fields.
These additions implemented the plan's notification and validation requirements.

`RtsTransmitStep` copied the reference because it owned the RTS `Packet`.
`RtsProcedure` built only the RTS header, so it required no signature change.
Conventional fragmentation already copied all tags and required no production change.
The MAC removed the sender's context tag at reception.

The rate-context commit changes these source files:

- `mib/Ieee80211RateContext.h` and `mib/Ieee80211Mib.{h,cc}`.
- `mgmt/Ieee80211RateContextTag.msg` and `mgmt/Ieee80211MgmtRateSet.{h,cc}`.
- `mgmt/Ieee80211MgmtBase.{h,cc}` and `mgmt/Ieee80211MgmtApBase.cc`.
- `mgmt/Ieee80211MgmtAp.{h,cc}` and `mgmt/Ieee80211MgmtSta.{h,cc}`.
- `mgmt/Ieee80211MgmtStaSimplified.cc`.
- `mac/Ieee80211Mac.cc` and `mac/framesequence/FrameSequenceStep.h`.

These paths are relative to `src/inet/linklayer/ieee80211/`.
The migration guide documented the new value types, tag, and update scope.
Existing regression fixtures supplied valid rate advertisements where their scenario required association success.
The primitive-dispatch fixture supplied a real MIB.

The rate-context commit changes `WHATSNEW` and the migration guide.
It adds the target-context module test and updates 15 existing module test files plus one unit test file.
The fixture updates retain their existing expected outcomes.
No fingerprint baseline changes belong to this rate-context commit.
Earlier commits on the branch include other features and fingerprint changes outside this plan's scope.

### Focused evidence

The commands used `/home/user/omnetpp_ws/inet-ieee80211-rate-context`, except for the fingerprint command.
All compiled tests used debug mode.
The new deterministic module case used `General`, run 0, and seed 0.
Existing cases retained their declared configurations and seeds.

```sh
make MODE=debug -j$(nproc)

MPLCONFIGDIR=/tmp/rate-context-matplotlib inet_run_module_tests -m debug -f '(Ieee80211TargetRateContext_1|Ieee80211MgmtStaSimplifiedInitialization_1|Ieee80211MgmtApReassociationSnapshot_1|Ieee80211MgmtStaLifecycle_1|Ieee80211HtAssociation_1|Ieee80211MgmtSta(Discovery|BeaconUpdate|Deauthentication|Disassociation)_1|Ieee80211MgmtAp(Timeout|HcfRtsTimeout|QueueDrop|Lifecycle|ChannelChange|HcfQueueDrop|MalformedHtCap)_1|Ieee80211HcfManagementRecovery_1|Ieee80211AgentStaReassociation_1)\.test'

MPLCONFIGDIR=/tmp/rate-context-matplotlib inet_run_unit_tests -m debug -f '(Ieee80211SupportedRates_1|Ieee80211MibPeerTeardown_1|Ieee80211MgmtTransactionTag_1|Ieee80211MgmtStaDiscovery_1|Ieee80211MgmtStaPrimitiveDispatch_1)\.test'

MPLCONFIGDIR=/tmp/rate-context-matplotlib inet_run_module_tests -m debug -f 'Ieee80211TargetRateContext_1\.test'
```

The final build passed with exit status 0.
An initial concurrent build failed at the library link.
A later build ran without a concurrent build and passed.
The combined module run passed all 18 cases with exit status 0.
The unit run passed all five cases with exit status 0.
The final target-only run passed after the test added malformed-rate and authentication-failure assertions.

The build log was `/tmp/rate-context-build-verified.log`.
The test logs were `/tmp/rate-context-module-verified.log`, `/tmp/rate-context-unit-verified.log`, and `/tmp/rate-context-target-verified.log`.
The test artifacts used directories under `tests/module/work/` and `tests/unit/work/` that matched each test name.

The fingerprint command ran from `tests/fingerprint`:

```sh
./fingerprinttest -d -m 'examples/(wireless/lan80211|adhoc/ieee80211)/.*Ping1' -f tplx -f '~tNl' -f '~tND'
```

Both `Ping1` configurations passed, with run 0 and exit status 0.
They covered infrastructure and ad hoc operation.
The log was `/tmp/rate-context-fingerprint-verified.log`.
An earlier selector matched zero cases and supplied no evidence.

### Project checks and limits

| Command | Exit status | Recorded result |
| --- | --- | --- |
| `doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211` | 0 | Pass |
| `doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211` | 1 | 32 existing candidates in unchanged MSG files |
| `doc/project/enforcement/check-interfaces.sh` | 1 | 15 existing interface violations in unchanged files |
| `doc/project/enforcement/check-source-seals.sh --base dabcd78282d9043764521b0b713ea17694f29d56` | 0 | Pass |
| `git diff --check` | 0 | Pass |

The logs used `/tmp/rate-context-{architecture,naming,interfaces,seals,whitespace}.log`.
The source check used `doc/project/README.md`, its contribution guide, and its architecture, naming, test, and WLAN rules.
The seal registry protected no changed source path.
The exception ledgers described existing violations, such as `AV-CONTRACT-02` for `ITransmitStep` and `IReceiveStep`.
The static checks reported no new violation from the patch.
The checks across the checkout still reported existing violations.

The standards lookup used the ready `ieee80211-2024` corpus, the local collection of standard text.
It confirmed clause `6.5.4.2.2` at physical PDF pages 487–490 and clause `11.3.5.3` for AP admission.
The canonical nodes were `ieee80211-2024:clause:6.5.4.2.2` and `ieee80211-2024:clause:11.3.5.3`.
The lookup required no PDF inspection.

This recorded evidence establishes rate publication and context identity for those runs.
It does not establish control-rate selection, response timing, TXOP admission, or Block Ack behavior.
No release build or independent agent review ran for this record.

## 7. Current consistency review and verification

The review compares the plan with commit `b279b995015183d30fcbcdcc34ecf292403e1740` and its parent `d00ff4ecf5065562153775d9332564874c0082e1`.
The rate-context commit changes 35 files: 16 source files, two documentation files, and 17 test files.
The test files include one new module case, 15 changed module cases, and one changed unit case.
The reviewed source and tests have no local changes.
The plan and unrelated local documents remain outside the committed implementation.

### Corrections from the source comparison

The review corrected these descriptions without a source change:

- **Publication:** Local legacy supported and operational facts come from the represented advertisement.
  Adhoc and simplified AP management inherit publication without a change to those subclasses.
- **Commit points:** The STA commits accepted response rates with association state.
  The AP commits saved pending peer rates after an acknowledged successful response.
  Associated Beacons refresh active rates; Probe Responses do not perform that active-rate commit.
- **Cleanup:** Transaction removal, discovery cleanup, disassociation, and lifecycle cleanup have different scopes.
  Discovery cleanup preserves the active peer.
  Lifecycle cleanup removes all transient facts in the affected MIB.
- **Authentication completion:** Deauthentication removes the pending target and cancels its timeout.
  The agent receives one refusal, including when a duplicate frame arrives later.
- **Identity:** Request and response bindings share the same association or reassociation exchange.
  Same-AP target queries cannot substitute the active reference for those subtypes.
  Active data queries remain separate.
- **Generation:** A target query and removal need the committed generation.
  Nested updates can advance the MIB generation several times before one final notification.
- **Consumers and coverage:** Current code resolves metadata and copied facts.
  The MAC uses the resolved BSSID for a management header.
  Management uses a target snapshot to retain basic HT MCS facts for an accepted response.
  Current rate policies read existing peer HT state and do not consume the new rate snapshots.
  Response-rate rules and cancellation of future originator transmissions remain outside scope.
  Controlled management delivery and direct tag-copy probes do not prove the full radio path.
- **Change inventory:** The plan now identifies the changed release notes, migration text, regression fixtures, and primitive-dispatch fixture.
  Fingerprint changes in earlier branch commits remain outside the rate-context commit.

### Fresh verification at the reviewed commit

The fresh debug build completed before the tests ran.
The command directory was `/home/user/omnetpp_ws/inet-ieee80211-rate-context`, except for fingerprints.
The module and unit commands in section 4 used `MPLCONFIGDIR=/tmp/rate-context-plan-review-matplotlib`.
The target-context case used `General`, run 0, and seed 0.
The existing cases used their declared configurations and seeds.
The fingerprint command ran from `tests/fingerprint` with `Ping1`, run 0, in both selected example directories.

| Check | Exit status | Current result | Log |
| --- | --- | --- | --- |
| `make MODE=debug -j$(nproc)` | 0 | The debug library rebuilt successfully. | `/tmp/rate-context-plan-review-build.log` |
| First module selector in section 4 | 0 | All nine cases passed. | `/tmp/rate-context-plan-review-module.log` |
| Second module selector in section 4 | 0 | All eight cases passed. | `/tmp/rate-context-plan-review-module-fixtures.log` |
| Unit selector in section 4 | 0 | All three cases passed. | `/tmp/rate-context-plan-review-unit.log` |
| Fingerprint selector in section 4 | 0 | Both infrastructure and ad hoc cases passed. No expected value changed. | `/tmp/rate-context-plan-review-fingerprint.log` |
| Architecture command in section 4 | 0 | The scoped check passed. | `/tmp/rate-context-plan-review-architecture.log` |
| Naming command in section 4 | 1 | The check reported 32 candidates, all outside the changed source files. | `/tmp/rate-context-plan-review-naming.log` |
| Interface command in section 4 | 1 | The check reported 15 violations, all outside the changed source files. | `/tmp/rate-context-plan-review-interfaces.log` |
| Source-seal command in section 4 | 0 | No protected source change appeared in the reviewed commit. | `/tmp/rate-context-plan-review-seals.log` |

Test artifacts are under `tests/module/work/` and `tests/unit/work/` in directories that match the selected test names.
The fingerprint runner also wrote `tests/fingerprint/fingerprinttest.out`.
The naming and interface checks remain non-clean across the checkout.
This review establishes no release-build, statistical, or broader fingerprint result.

The general semantic checklist produced 15 `PASS` results and 13 `N/A` results, with no `FLAG` or `QUESTION`.
The IEEE 802.11 checklist produced seven `PASS` results, five `N/A` results, and one traceability `FLAG` for the comment below.
That flag requires a citation correction rather than an architecture or naming exception.
The comment correction below resolves this flag.

### Standard traceability and comment correction

The review used the ready `ieee80211-2024` collection under `/home/user/omnetpp_ws/inet/standards/processed`.
The local collection in this checkout uses an older format, so the review used the current shared collection.
The lookup confirmed the clause identifiers and physical PDF pages in section 5.
It required no PDF inspection.
The retrieved clause records are under `/tmp/rate-context-plan-review-standard-*.json`.

The reviewed source comment at `mgmt/Ieee80211MgmtRateSet.cc:13` cited clause `9.4.2.13` for the second rate element.
That clause describes Power Capability; the applicable clause is `9.4.2.11`.
Section 5 uses the correct clause.
Commit `d93f95530b686dd25a50d6b651847cf00dbd7862` corrects the source comment to clause `9.4.2.11`.
This amendment changes only the comment; it retains the reviewed parent and commit message.
The runtime source and tests are identical to the reviewed commit.
The whitespace check passed; no runtime test ran again for this comment correction.
The local plan remains outside the commit.
