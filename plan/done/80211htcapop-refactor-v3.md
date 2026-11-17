# Implementation plan: IEEE 802.11 HT capability, BSS, and peer-state contracts

> **Status:** complete · **Source:** [Proposal V3](../../report/80211htcapop-refactor-v3.md) · **Prepared:** 2026-09-18

Deliver the four contracts in V3 through four coherent implementation milestones: independent
capability preparation, committed management transitions, capability-only peer caching, and explicit
initialization dependencies. Preserve the existing HT and legacy behavior described by the proposal.
Implementation results and the consumer readiness worksheet are recorded in the
[execution evidence](80211htcapop-refactor-v3-evidence.md). The sections below preserve the original
work breakdown; the evidence records actual commands, outcomes, boundaries, and deviations.

## 1. Baseline, scope, and deliverables

The proposal references `3f89b4b439c1dafd6b217ae9beb94b604f50c615`. This plan was prepared against
`98117c3257b2e11661d2baf685c18911c8639b24`. A comparison of those commits found no differences in
`src/inet/linklayer/ieee80211`, `src/inet/physicallayer/wireless/ieee80211`, `tests/unit`, or
`tests/module`. Refresh this comparison before implementation, including newly added amendment
writers and consumers. The working tree was clean before this plan was added.

At planning time, the source exhibited the migration points identified by V3:

- `Ieee80211Mac::initialize()` selects the catalog at `LOCAL`, casts to concrete PHY contributors
  at `LINK_LAYER`, prepares MIB capabilities, and emits `modesetChanged`.
- `Ieee80211Mib::updateLocalHtCapabilities()` also resets operation, derives its basic MCS set,
  applies operation configuration, and rebuilds peer results.
- `Ieee80211Mib::setPrimaryChannel()` chooses width fallback and rebuilds peer results.
- `Ieee80211NegotiatedHtCapabilities` contains an operation copy read by the shared selector.
- `Ieee80211MgmtApBase` finalizes operation at `LAST`; simplified STA management calls
  `configureAssociation()` at both `LINK_LAYER` and `LAST`.

Implementation deliverables are:

1. Typed, read-only catalog and component-contribution contracts with compatible NED wiring.
2. A prepared local capability profile, explicit active BSS state, and relationship-scoped peer state.
3. Management-owned transition operations with complete-state notification and lifecycle behavior.
4. A shared HT compatibility filter consuming cached capabilities, eligibility, and explicit operation.
5. A verified consumer preparation map and removal of initialization signal/listener plumbing.
6. Focused regression tests, an evidence manifest, and removal of temporary migration adapters.

Use the [architecture](../../doc/project/rule/architecture.md),
[WLAN architecture](../../doc/project/design/ieee80211-model-architecture.md),
[WLAN rules](../../doc/project/domain/ieee80211.md),
[testing rules](../../doc/project/rule/testing.md), and
[contribution workflow](../../doc/project/guide/contribute-a-change.md) as the governing references.
Recheck the [seal registry](../../doc/project/audit/seal-list.md) before source edits. The currently
listed `common/packet/` seal is outside the intended change surface; no packet-core edit is planned.
Check the existing architecture and naming exception ledgers before reporting a deviation.

### Compatibility boundaries

Preserve parameter paths and precedence, built-in advertisements, association outcomes, supported-rate
elements, rate-selection behavior, frame-class dispatch, legacy operation, and existing radio command
handling. Keep protocol state inspectable in simple modules through WATCH/display facilities.

Explicitly exclude new capability switches, new PHY features, complete basic-rate policy, runtime
interface-wide mode reconfiguration, Notify Channel Width, coexistence scheduling, operating-class
transitions, and VHT/HE/EHT capability models. Do not turn a serialized field into an implementation
claim. Do not claim a performance or standards-conformance improvement from this refactor alone.

Retain the standards basis in V3. Its STA fallback/recovery, current-AP Probe Response treatment,
simplified no-air association, and immediate local channel updates are compatibility/model choices.
If implementation needs a new normative decision, verify the cited standard and scope that decision
separately rather than silently changing the expected behavior.

## 2. Dependency order and review boundaries

```text
P0: baseline, writer/reader inventory, API and readiness design
  -> P1: typed contributions + capability-only preparation + replacement operation initializer
  -> P2: committed BSS/relationship transitions + notification + lifecycle
  -> P3: capability-only cache + explicit selection context + remove operation copies
  -> P4: typed catalog dependency + migrate all preparation + remove initialization broadcast
  -> P5: final integration evidence, cleanup, documentation, review
```

Each milestone must build and have its directly related tests passing before the next milestone
relies on it. Add tests alongside each behavior migration. Keep broad renames and formatting out of
semantic commits. A milestone can contain several commits, but every commit must leave a coherent,
buildable model. In particular, remove an old initializer only in a commit that supplies its
replacement, and migrate a selector signature together with its callers.

| Milestone | Suggested commit concern | Temporary compatibility mechanism | Removal deadline |
|---|---|---|---|
| P0 | Characterization tests for currently supported behavior, if missing | Existing model | Before semantic changes |
| P1 | Query contracts, built-in contributors, separated local construction and operation preparation | Existing initialization signal; selector operation copies | P3/P4 |
| P2 | Shared transition contract, then detailed and simplified role migration | Old selector input adapted from the committed view | P3 |
| P3 | Cache/selection migration and obsolete state removal | Initialization signal only | P4 |
| P4 | Catalog provider and consumer migrations in buildable groups; signal removal last | Short-lived provider/signal coexistence | End of P4 |
| P5 | Remaining integration fixtures and contract documentation | None | Completion |

## 3. P0 — establish the execution baseline

**Entry:** implementation checkout and target revision are identified.

- [x] Record HEAD, working-tree status, build environment, enabled INET features, and the proposal
  baseline comparison. Preserve unrelated local changes. **Done:** baseline recorded; only this task
  changed the initially clean source.
- [x] Inventory all readers/writers of local capabilities, `PeerHtState`, BSS identity, operation,
  `isHtOperationSupported()`, `setPrimaryChannel()`, and `negotiateHtCapabilities()` across the tree.
  Classify each as configuration, management policy, shared storage, transaction snapshot, or algorithm
  state. Include tests, serializers, simplified/ad hoc paths, and any concurrent amendment additions.
- [x] Inventory every subscription, override, and inherited dependency on `ModeSetListener` and
  `modesetChangedSignal`. Use the starting inventory in section 8, then search the whole source tree.
- [x] Record effective `opMode`/`modeSet` forwarding in both interface compositions and standalone
  fixtures. Include default forwarding, an explicit MAC catalog override, radio/component parameters,
  and initialization errors. Preserve these paths instead of consolidating them speculatively.
- [x] Complete the API decisions in section 4 and the readiness worksheet in section 8. Settle contract
  placement before adding a dependency that would point PHY code back into MAC implementation types.
- [x] Build a fresh debug library and run the existing focused tests in section 10. Record pre-existing
  failures separately. Capture exact advertisements, accepted state, selection results, and current
  rejection behavior where later comparisons need an oracle.
- [x] Select the directly affected legacy fingerprint cases and peer-exchange protocol cases. Record
  their exact selectors and executed counts before production changes; avoid choosing cases merely
  because they pass. **Sequencing deviation:** protocol/fingerprint selection and execution followed
  the initial source changes; unit/module baseline runs preceded them. See execution evidence.

Useful inventory commands, run from the repository root:

```bash
git rev-parse HEAD
git status --short
rg -n 'ModeSetListener|modesetChangedSignal|modesetChanged' src tests
rg -n 'updateLocalHtCapabilities|setPeerHtCapabilities|findPeerHtState|negotiateHtCapabilities' src tests
rg -n 'localHtCapabilities|bssData|bssStationData|bssAccessPointData|getHtOperation|setPrimaryChannel' src/inet/linklayer/ieee80211
rg -n 'INITSTAGE_|Define_InitStage_Dependency' src/inet/common/InitStages.cc src/inet/linklayer/ieee80211
```

**Exit:** a finite reader/writer list, exact baseline test selection, parameter-precedence record, and
resolved preparation design exist. New-contract tests may be planned here and implemented in their
own milestone; the baseline suite must describe the old supported behavior rather than require the
new API to exist.

## 4. Contract decisions to settle before implementation

The following are recommended shapes, not mandatory new class names. Prefer extending suitable
existing value types over creating one class or module per concept.

| Contract | Required content and invariant | Decision owner |
|---|---|---|
| Configured catalog provider | Const query such as `getConfiguredModeSet()`; configuration lifetime; explicit readiness | MAC |
| Tx/Rx contribution | Typed queries for currently needed implemented abilities; identify direction, widths, stream/MCS limits, and per-width receive short GI | Implementing PHY component |
| Prepared local profile | Distinguish unprepared, prepared legacy, and prepared HT; stable across ordinary stop/restart | Assembly installs; MIB stores |
| Current BSS | Presence, identity, role-appropriate channel context, optional HT operation; legacy channel information usable independently | Management decides; MIB stores |
| Relationship | Explicit installed relationship, accepted peer information, validated HT eligibility; address equality alone is not identity | Management decides; MIB stores |
| Directional cache | Exact capability-derived results for local-Tx/peer-Rx and local-Rx/peer-Tx; immutable to consumers | MIB peer record |
| Transition/publication | Validate before mutation; commit complete shared state; finish owner bookkeeping; publish once | Management coordinates; MIB emits shared-state notification |

Choose these details explicitly:

- Keep known directional capabilities during temporary operational HT ineligibility, with all use
  gated by the validated relationship query. An accepted missing/changed advertisement must still
  update its presence/contents appropriately; retained historical information cannot become current
  merely because eligibility later changes. Teardown discards relationship-scoped state.
- Treat local capability installation as initialization-only in ordinary operation. Repeated typed
  preparation is idempotent; an unsupported attempt to replace a prepared profile must not leave
  stale peer caches. Do not add runtime reconfiguration as part of this work.
- Compute semantic equality across every derivation input, preserving presence and unknown values.
  Do not compare padded structures with `memcmp`, or compare only an MCS ceiling.
- Keep band plus internal channel index together, or provide equivalent unambiguous decoding context.
  Preserve a generic legacy radio's supported absence of an IEEE band mapping.
- Distinguish `hasPreparedLocalCapabilities`, local HT support, active BSS presence, and applicable
  HT operation in query semantics. Final names should follow repository naming guidance. Remove the
  ambiguous MIB query once callers migrate; the similarly named mode-catalog query has a different role.
- Define const-reference/pointer lifetimes: a peer pointer may become invalid on teardown or replacement.
  Selection should consume one consistent view without retaining it across callbacks.
- Keep pending responses and discovery snapshots immutable where their historical meaning requires it.
  Eliminating duplicated current operation does not authorize deleting transaction history.

## 5. P1 — separate capability construction and management operation

**Primary files:** `mac/Ieee80211Mac.{h,cc}`, `mib/Ieee80211Mib.{h,cc,ned}`,
`mib/Ieee80211HtCapabilities.h`, `mgmt/Ieee80211MgmtApBase.{h,cc}`, relevant ad hoc management,
and PHY contributor declarations/implementations under
`src/inet/physicallayer/wireless/ieee80211/packetlevel/`.
Paths beginning `mac/`, `mib/`, or `mgmt/` here and below are relative to
`src/inet/linklayer/ieee80211/`.

- [x] Add the smallest typed contribution contracts in the package owning the queried role. Keep
  interface declarations pure; place implementation in concrete/base classes. Avoid making PHY
  depend on a MAC-owned profile assembler or requiring one concrete radio class.
- [x] Implement the contracts in the built-in transmitter/receiver. Document each contribution's
  source, direction, implementation limit, readiness, and consumer. Preserve built-in answers.
- [x] Replace the MAC's casts to concrete `Ieee80211Transmitter`/`Ieee80211Receiver` with role-contract
  queries. Retain a meaningful error for HT configurations missing required contribution support.
  Do not impose new HT-only dependencies on legacy configurations.
- [x] Extract local-profile assembly from MIB operation and peer mutation. Preserve catalog/Tx/Rx
  width intersection, antenna/stream limits, exact MCS bitmap, receiver short-GI information,
  and existing MAC advertisement inputs such as A-MPDU exponent.
- [x] Preserve undefined, equal, and unequal Tx-MCS knowledge and conversion semantics. Equal Tx/Rx
  support uses the exact bitmap; summary/unknown peer Tx knowledge must not become exact empty support.
- [x] Add management's operation-preparation procedure in the same change. It derives Basic HT-MCS,
  width/secondary offset, protection configuration, and channel context from prepared inputs. Keep
  existing parameter paths even when the procedure interpreting them moves out of the MIB.
- [x] Move configured-width rejection and band-dependent fallback decisions into management. Use PHY
  band/mode legality APIs; preserve the distinction between an incapable configured PHY and a channel
  requiring fallback. The MIB checks structural consistency and stores the result.
- [x] Supply explicit AP and ad hoc preparation paths. STA local capability preparation must not
  invent an active BSS. Preserve initial radio channel information received before profile readiness.
- [x] Adapt existing callers while retaining initialization signaling temporarily. Keep this adapter
  incapable of resetting BSS operation when local-profile preparation is called again.

**Evidence:** extend `Ieee80211HtCapabilities_1.test` and `Ieee80211HtMgmtElements_1.test`; add a focused
module fixture using a replacement contributor that is not a subclass of the built-in concrete
Tx/Rx classes. Change its declared contribution and observe the assembled profile through the
production MAC path. Include local assembly with an existing active operation and verify no mutation.
Run the real built-in association fixture to compare emitted advertisements and antenna limits.

**Exit:** capability installation cannot select a channel, create a BSS, change operation, or install
peers. Management supplies every removed operation-initialization effect. Built-in serialized
advertisements remain equivalent; replacement contributors work without assembler edits.

## 6. P2 — commit complete BSS and relationship transitions

**Primary files:** `Ieee80211Mib`, `Ieee80211MgmtBase`, `Ieee80211MgmtApBase`,
`Ieee80211MgmtAp`, `Ieee80211MgmtSta`, `Ieee80211MgmtStaSimplified`,
`Ieee80211MgmtApSimplified`, `Ieee80211MgmtAdhoc`, and `Ieee80211HtMgmtElements.h`.

- [x] Introduce explicit local readiness, active BSS presence, band/channel context, and relationship
  validity. Keep display/configuration identity separate where compatibility retains it after stop.
- [x] Introduce transition inputs that can be validated before mutation. Route relevant direct writes
  to shared BSS/peer state through the owner-controlled commit path. Avoid exposing mutable references
  that bypass invariants; preserve compatible inspection queries.
- [x] Implement the field-source and transition matrix below in detailed management. Keep discovery,
  pending transactions, and active state separate. Make HT eligibility a validated result of the
  relationship and applicable requirements, rather than an independently writable Boolean.
- [x] Commit AP relationships only for the matching acknowledged successful response. Evaluate current
  local operation at commit, without modifying the transmitted response snapshot. Preserve AID
  reservation/commit/cancel semantics and transaction matching.
- [x] Commit the STA's accepted successful response with response capability/operation fields plus
  the selected discovery record's Basic HT-MCS requirement and decoding context. Preserve the
  existing successful-association/legacy-fallback outcome for unusable HT information.
- [x] Route simplified management through equivalent local transition contracts. Resolve the AP
  management endpoint through an appropriate typed preparation/transition contract; do not make the
  STA an independent policy writer of AP internals. Preserve the explicit no-air abstraction.
- [x] Complete stop, crash, restart, peer replacement, and scoped transaction cleanup for all affected
  roles. Restart uses retained prepared configuration and reestablishes operational state.
- [x] Preserve agreement cleanup through its existing algorithm owners. Equal advertisements must
  not suppress same-address relationship replacement or required cleanup.
- [x] Adapt old selection inputs from the committed view until P3, with exactly one writer and a
  documented removal point. No old mutable adapter becomes a second authority.

### Required transition matrix

| Stimulus | State action | Observable acceptance condition |
|---|---|---|
| Candidate Beacon/Probe Response | Update validated discovery only | Existing active relationship unchanged |
| Matching successful Association/Reassociation Response at STA | Merge accepted response fields with discovery Basic HT-MCS and channel context; commit relationship | Correct association outcome and independently classified HT usability |
| Matching successful response ACK at AP | Commit pending peer using current local operation | AP may have channel index 11 while STA retains response index 6 |
| Current-AP accepted Beacon, width only | Commit operation and reevaluate eligibility | Subsequent selection obeys width; P3 proves no capability rederivation |
| Current-AP accepted Beacon, HT absent or unsupported basic MCS | Preserve association; mark HT unusable through validated state | Existing bounded legacy fallback |
| Valid HT information returns | Reevaluate independently of historical capability equality | HT restored with previously seen capability bytes |
| Current-AP Probe Response | Discovery update | Existing discovery-only authoritative-state policy preserved |
| Malformed Beacon | Reject before authoritative mutation | No partial BSS/peer commit; existing rejection/timer behavior preserved |
| AP radio channel update | Compute and commit band, channel, operation together | Callback sees consistent channel interpretation |
| Failed reassociation to another AP | Clear affected pending state | Old active relationship retained |
| Failed reassociation to current AP | Apply existing same-AP teardown | Correct relationship and agreement cleanup |
| Timeout, refusal, queue drop, cancellation, late completion | Match transaction and clean its scope | No unrelated peer/active relationship changed |
| Relationship replacement, including same BSSID | Apply identity and cleanup rules before installation | No stale eligibility, agreements, or cache inherited |
| Stop/crash | Clear operational/peer state and pending resources | Operational queries inactive; simplified AP peer removed where resolvable |
| Restart | Reprepare operation/relationship from stable configuration | No stale state; local profile retained |

### Publication contract

Use a combined transition or explicit commit-then-publish API. Define the shared-state signal on the
MIB boundary and declare it in NED. Specify meaningful change detection, affected BSS/peer scope,
payload ownership, and borrowed lifetime. Management completes required AID/transaction/relationship
bookkeeping before publication and preserves existing protocol-notification completion points.

In particular, fix the current ordering where `Ieee80211MgmtApBase::receiveSignal()` assigns
`radioBand` after calling the MIB channel setter. A synchronous notification must expose the new
band with its channel/operation, not the old band. A listener must never finish the transition.

Choose and document a reentrancy policy: synchronous read-only observation is supported; nested
mutations must either be rejected explicitly or safely handled by the transition implementation.
Do not add generation counters solely to make publication convenient. Do not retain map references
across callbacks that could invalidate them.

**Evidence:** extend the existing beacon, association, reassociation snapshot, AP/STA lifecycle,
deauthentication/disassociation, timeout, and queue-drop fixtures. Add an observer fixture that reads
identity, band, channel, operation, eligibility, and required transaction status synchronously. Vary
listener registration order and use two peers to prove scoped cleanup. Verify that an equal accepted
Beacon still refreshes applicable liveness/signal observations without a spurious operation event.

**Exit:** all affected management writers use consistent transition paths; no observer can see a
half-committed relationship; legacy/no-BSS/unusable-HT states are distinguishable. Detailed,
simplified, and ad hoc lifecycle paths have explicit ownership.

## 7. P3 — separate capability caching from operation checks

**Primary files:** `mib/Ieee80211HtCapabilities.h`, `Ieee80211Mib`,
`mac/rateselection/Ieee80211PeerModeSelection.{h,cc}`, `RateSelection`, `QosRateSelection`, and
management consumers of negotiated results.

- [x] Make directional derivation depend only on the prepared local profile and accepted peer
  capability inputs. Remove the operation argument from the core derivation function.
- [x] Install/reuse the cache on semantic input equality. A changed capability input refreshes before
  use; an operation-only change reevaluates eligibility without rebuilding directional support.
- [x] Keep capability equality separate from liveness, relationship identity, transaction completion,
  and eligibility. Reinstallation after teardown builds fresh relationship state even at the same MAC.
- [x] Change the shared selector to receive a consistent explicit context: directional capabilities,
  validated HT eligibility, and applicable local BSS operation. Preserve old frame-class and group
  dispatch; do not introduce an association requirement for every management/control transmission.
- [x] Migrate both QoS and non-QoS production call sites together. Keep rate control responsible for
  proposing/ranking modes; the helper remains a compatibility filter.
- [x] Preserve non-HT pass-through, exact sparse MCS support, directional receiver short GI, width
  checks, proposed-bitrate ceiling, deterministic tie breaking, and bounded legacy fallback/errors.
- [x] Remove `Ieee80211NegotiatedHtCapabilities::operation` after the last reader migrates. Preserve
  accepted local BSS state and immutable response/discovery snapshots.
- [x] Remove operation-triggered derivation loops and the unused peer `generation` field after a
  fresh whole-tree reader check. Do not replace it with another public counter without a consumer.

**Evidence:** extend `Ieee80211PeerModeSelection_1.test` for explicit contexts, missing/ineligible
state, sparse MCS, both short-GI directions, bitrate boundaries, and ties. Exercise both production
selection modules in module fixtures. Use test-only derivation instrumentation to establish:

| Action on a live relationship | Expected derivation behavior |
|---|---|
| First usable capability installation | Derive once |
| Repeated equal advertisement and repeated selection | Reuse |
| Width/protection/basic-MCS-only update | Reuse; reevaluate applicable constraints |
| Changed capability inputs | Refresh before consumption |
| HT ineligibility then valid information returns | Restore correctly; reuse only if cached inputs remain valid |
| Teardown then same-address installation | Fresh relationship/cache |

The 40-to-20 MHz fixture is synthetic evidence for management/selection; retain separate real
built-in 20 MHz packet-PHY association/serialization coverage. Do not describe that fixture as a
real 40 MHz PHY exchange.

**Exit:** operation-only changes change selection where appropriate without capability rederivation;
no production reader uses a peer copy of current BSS operation; recovery works after both missing HT
and unsupported Basic HT-MCS updates.

## 8. P4 — replace initialization broadcasts with declared dependencies

**Primary files:** `Ieee80211Mac`, `mac/common/ModeSetListener.*`, all consumers below, their NED
declarations, management preparation code, both interface compositions, and
`src/inet/common/Simsignals.{h,cc}`. Remove the NED signal declaration and stale TODO references too.

### Readiness design prerequisite

The implementation must fill one worksheet row per consumer, recording **input, provider path,
input-ready stage, preparation stage/call, first read, and idempotency rule**. The following constrains
that design; it is not a claim that sibling callbacks at the same stage are ordered.

| Prepared value | Earliest established prerequisite | Required preparation/first-use contract |
|---|---|---|
| Configured catalog | MAC `LOCAL` completed | Consumers query after `LOCAL`; no dependency on HT readiness |
| Local profile | Catalog plus configured PHY contributions/antenna inputs | Typed idempotent preparation after PHY readiness; complete before advertisement or peer derivation |
| Simplified identity | Interface addresses and AP SSID ready | Separate identity preparation complete before network configurators group interfaces |
| AP operation | Local profile and valid radio channel context | Typed idempotent AP preparation before advertisement or simplified HT installation |
| Simplified HT relationship | Both profiles plus prepared AP operation | Establish before its first initialization reader and before protocol operation |
| Timing/rate/policy state | Catalog and each consumer's local parameters/dependencies | Prepare once before the first protocol action; no reliance on broadcast order |

Use the actual graph in `src/inet/common/InitStages.cc`: `LINK_LAYER` depends on `PHYSICAL_LAYER`,
`NETWORK_CONFIGURATION` depends on `LINK_LAYER`, and `NETWORK_INTERFACE_CONFIGURATION` declares
only a dependency on `LOCAL`. Do not assume address initialization precedes link-layer work merely
from current numeric ordering. Trace the address provider as part of the worksheet.

The preferred design is post-`LOCAL` catalog queries and typed, idempotent preparation calls that can
prepare the peer's required inputs regardless of AP/STA declaration order. Such a call must only use
inputs already ready; it must not manually call another module's `initialize()` or lifecycle callback.
If a required address/PHY deadline cannot be met through existing declared dependencies, add the
smallest justified stage/dependency change and test it. Record that decision before removing the
signal; leaving the concrete stage/call mapping unresolved fails this milestone.

### Starting consumer inventory

Search for direct callbacks as well as classes inheriting `ModeSetListener`. Some consumers only use
the cached pointer later; others perform work in overridden callbacks.

| Consumer group | Existing work/input to preserve | Migration task |
|---|---|---|
| `Dcaf`, `Edcaf` | Slot/SIFS/IFS/EIFS, CW defaults, access-category parameters | Invoke existing timing/contention preparation after querying catalog |
| `Dcf`, `Hcf` | Mode-dependent coordination behavior; signal forwarding | Replace catalog dependency while preserving unrelated signal handling |
| `RateSelection`, `QosRateSelection` | Catalog and fastest mandatory mode cache | Prepare query/cache explicitly; retain selection-history behavior |
| `RateControlBase` and concrete algorithms | Catalog and `resetRateControl()` initialization | Run setup once after concrete prerequisites; preserve separate lifecycle reset semantics |
| `Ieee80211MgmtBase` | Supported/Extended Supported Rates construction | Prepare before first frame construction; preserve bytes/basic-rate flags |
| `OriginatorAckPolicy`, `OriginatorQosAckPolicy`, `RtsPolicy`, `QosRtsPolicy` | Mode-dependent policy queries | Declare provider and establish readiness before use |
| `RecipientAckPolicy`, `RecipientQosAckPolicy`, `CtsPolicy`, `QosCtsPolicy` | Response timing/mode queries | Same explicit provider preparation |
| `TxopProcedure`, `SingleProtectionMechanism`, `OriginatorProtectionMechanism` | TXOP/protection mode inputs | Preserve preparation and subsequent timing behavior |
| `OriginatorBlockAckAgreementPolicy` | Catalog-dependent agreement policy | Explicit preparation without changing agreement ownership/lifetime |
| Other inherited/concurrent consumers found by P0 | Any additional callback or deferred catalog use | Add individual worksheet rows and focused checks |

### Implementation tasks

- [x] Add the MAC's typed catalog provider, such as a pure `getConfiguredModeSet()` query, in the
  appropriate MAC contract package. NED `IIeee80211Mac` alone does not supply a C++ query interface.
- [x] Declare each consumer dependency using module parameters/contracts and connect defaults in
  `Ieee80211Interface.ned` and `ExtUpperIeee80211Interface.ned`. Preserve `opMode`/`modeSet` forwarding
  and allow explicitly configured standalone compositions. Do not cast to a containing-interface class.
- [x] Migrate consumer groups using the completed worksheet. Preserve callback side effects as
  explicit preparation, not merely a stored pointer. Retain unrelated signals/subscriptions.
- [x] Preserve `numInitStages()` behavior when replacing the listener base class; check overrides and
  initialization chains so later preparation stages still execute.
- [x] Split simplified identity preparation from HT relationship installation. Replace the `LAST`
  retry with a deterministic preparation path. Ensure AP operation is ready in either declaration order.
- [x] Repeated preparation must not reset contention, rate estimates, agreements, or transaction state.
  Stop/restart retains profiles and catalog while using the role-specific operational reset contract.
- [x] Remove signal emission only when all consumers have migrated. Remove listener implementation,
  inheritance/includes, subscriptions, signal definition/declaration, NED metadata, and stale comments.
- [x] Recheck runtime radio setters/command handling and their callers. Preserve their compatibility;
  do not reinterpret signal removal as either supporting or forbidding interface-wide reconfiguration.

**Evidence:** update `Ieee80211MgmtStaSimplifiedInitialization_1.test` to preserve early identity
assertions and verify HT readiness at the new documented first-read boundary. Run AP-before-STA and
STA-before-AP orders, repeated preparation, stop/restart, missing-AP cleanup, prepared legacy state,
ad hoc, and standard/external-upper/standalone compositions. Compare timing, supported rates, and
initial rate-control state with P0. Test missing-provider diagnostics.

**Exit:** no initialization reader depends on a sibling's or remote module's `LAST` publication;
no consumer relies on `modesetChanged`; all worksheet rows identify an implemented and tested path.

## 9. Acceptance coverage and evidence ownership

Existing test names below are starting points, not assertions of complete current coverage. New
fixtures should be added only where the existing production path cannot express the required probe.

| Evidence ID | Claim and test layer | Existing starting point / required addition | Milestone |
|---|---|---|---|
| E1 | Exact directional capability derivation and representation, unit | `Ieee80211HtCapabilities_1.test`, `Ieee80211HtMgmtElements_1.test`; unknown/unequal/equal/sparse/short-GI cases | P1/P3 |
| E2 | Replaceable contributors and independent construction, module | New focused production-MAC contributor fixture; operation unchanged by assembly | P1 |
| E3 | Built-in advertised limits, module and serialization | `Ieee80211HtAssociation_1.test`, `Ieee80211HtAntennaRateControl_1.test`; real packet PHY remains 20 MHz | P1/P5 |
| E4 | Selection invariants, unit plus production integration | `Ieee80211PeerModeSelection_1.test`; QoS and non-QoS module checks | P3 |
| E5 | Width-only cache reuse, liveness, HT fallback/recovery, malformed input, module | `Ieee80211MgmtStaBeaconUpdate_1.test`; test-only derivation observation | P2/P3 |
| E6 | Response provenance and distinct AP/STA knowledge, module | `Ieee80211HtAssociation_1.test`, `Ieee80211MgmtApReassociationSnapshot_1.test`; response Basic HT-MCS merge | P2 |
| E7 | Transaction cleanup and lifecycle scope, module | AP/STA lifecycle, AP timeout/queue-drop/HCF variants, STA deauthentication/disassociation, agent reassociation tests | P2 |
| E8 | Atomic observer contract, module | New focused commit observer; listener-order and supported reentrancy probes | P2 |
| E9 | Initialization and composition, module/network | `Ieee80211MgmtStaSimplifiedInitialization_1.test`; both orders, external-upper, standalone, legacy/ad hoc fixtures | P4 |
| E10 | Channel context/fallback and generic-radio compatibility, module | `Ieee80211MgmtApChannelChange_1.test`, `Ieee80211MgmtApGenericRadio_1.test`, `Ieee80211MgmtApUnavailableChannel_1.test` | P1/P2 |
| E11 | Preserved peer exchange sequence, protocol | Inspect `tests/protocol/wifi/common/WifiAssociation.test` and `WifiReassociation.test`; add a focused case if the changed path is absent | P2/P5 |
| E12 | Unintended legacy trajectory changes, fingerprint | Explicitly selected WLAN legacy cases/configurations from the repository fingerprint catalog | P4/P5 |

For each claim, record the production entry point and the assertion that detects a regression. A
helper-only test is not production-path evidence. Module tests prove state/owner contracts;
cross-peer sequence claims need protocol-category evidence even when an existing module fixture
provides useful complementary observations. If a fixture bypasses the PHY, label that boundary.

Use deterministic fixtures with a pinned configuration, run 0, explicit seed, relevant parameters,
and a bounded duration. Enumerate both declaration orders, QoS modes, roles, and lifecycle outcomes
where required rather than hoping random seeds exercise them. A single seed suffices for an exact
state-transition reproduction; use a finite additional seed/configuration campaign only where timing
or randomized contention is part of the claim. Do not rerun unexplained failures until green.

## 10. Verification commands and reporting

Commands below are the original execution plan. Actual build/test results and final selectors are
recorded in the linked execution evidence. Both `inet_run_unit_tests` and `inet_run_module_tests` exist in the inspected checkout.
Run from the repository root with the normal INET/OMNeT++ environment active.

After compiled source or generated-code inputs change, rebuild the library the tests load:

```bash
make -j$(nproc) MODE=debug
inet_run_unit_tests -m debug -f 'Ieee80211(HtCapabilities_1|HtMgmtElements_1|HtModeSet_1|PeerModeSelection_1)\.test'
inet_run_module_tests -m debug -f 'Ieee80211(HtAssociation_1|HtAntennaRateControl_1|MgmtStaBeaconUpdate_1|MgmtApReassociationSnapshot_1|MgmtStaSimplifiedInitialization_1)\.test'
inet_run_module_tests -m debug -f 'Ieee80211(MgmtAp(Lifecycle|Timeout|QueueDrop|HcfQueueDrop|HcfRtsTimeout|ChannelChange|GenericRadio|UnavailableChannel|MalformedHtCap)|MgmtSta(Lifecycle|Deauthentication|Disassociation|Discovery)|AgentStaReassociation)_1\.test'
```

Use the appropriate subset during each milestone, and add explicit filters for every new fixture.
Resolve exact protocol invocation/build requirements from
[protocol AUTHORING](../../tests/protocol/lib/AUTHORING.md#10-running-tests) and the Wi-Fi suite's
current runner; the case filenames above are candidate coverage, not a fabricated runnable selector.
Resolve fingerprint cases/configurations against the actual catalog and record the final regex before
execution. A placeholder or a zero-case selection is `NOT_RUN`.

Run focused architecture checks for the changed paths:

```bash
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/emulation/linklayer/ieee80211
doc/project/enforcement/check-interfaces.sh
```

For final integration, follow the current [run-the-gates guide](../../doc/project/guide/run-the-gates.md):
fresh debug and release compilation, focused tests, project-wide architecture/naming/interface/seal
gates, commit/classification gates against the actual PR base, and the general/WLAN semantic
checklists. Include changed common signal/stage code in the gate scope. Broad C++ changes also need
the documented C++ checker with a compilation database. Distinguish pre-existing gate findings from
new defects and reconcile them with the ledgers rather than hiding them.

Store an evidence manifest with one row per command: milestone, tested revision, working directory,
exact command/filter, build mode, configuration/run/seed, selected/executed count, exit status,
outcome classification, and log/artifact paths. Keep helper, module, protocol, and fingerprint claims
separate. Setup failures, expected failures, and missing coverage are not passes.

If a fingerprint changes, diagnose the first relevant divergence against the pinned baseline. An
intentional stage move still needs a behavioral explanation and focused proof. Changes to recorded
baselines follow [change-a-baseline](../../doc/project/guide/change-a-baseline.md); do not regenerate
fingerprints simply to make this refactor pass.

## 11. P5 — completion audit

- [x] Every P0 reader/writer is migrated or explicitly justified as a historical transaction/discovery
  snapshot. No remaining production writer bypasses the relevant transition invariant.
- [x] Search confirms no concrete built-in Tx/Rx cast in capability assembly, no obsolete initialization
  signal/listener path, and no peer copy of current operation. Review legitimate similarly named PHY
  catalog/runtime-command APIs instead of deleting them mechanically.
- [x] No operation-only path rebuilds capability intersections. No selection path rebuilds them on
  each frame. Equality cannot suppress liveness, eligibility restoration, or relationship replacement.
- [x] No new public counter exists solely for tests. WATCH/display inspection reflects authoritative
  state and does not imply active operation from retained display identity.
- [x] Both selection paths and every affected management role meet the acceptance matrix. The
  AP-current/STA-accepted channel distinction and response Basic HT-MCS provenance remain explicit.
- [x] The preparation worksheet is complete, source-backed, and exercised in both declaration orders.
  All required preparation finishes before first use without cross-module `LAST` dependence.
- [x] New provider/dependency contracts work in standard, external-upper, and explicit standalone
  compositions, including legacy behavior and useful missing-dependency diagnostics.
- [x] All migration adapters listed in section 2 are removed. Stable parameters and radio command
  semantics remain compatible; deferred feature work has not entered the patch series.
- [x] Final focused evidence, fresh debug/release build results, gate outcomes, and unresolved gaps are
  recorded. A required behavioral coverage gap prevents marking the implementation complete.
- [x] Review the final change using the canonical code/PR guides and general/WLAN checklists. Update
  canonical contract documentation only where the implementation adds concrete details; link this plan
  rather than duplicating project policy. Move the plan to `plan/done/` only after its work is complete.

Completion means the new contracts govern every changed production path and the focused evidence
supports the preserved behavior. This planning document does not authorize unrelated feature work,
baseline replacement, or modification of sealed source.
