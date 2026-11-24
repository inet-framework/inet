# Publish the completed 6LoWPAN implementation as a review series

> **Kind:** design · **Status:** draft · **Seal:** none · **Owns:** — · **Stands on:** [first-release plan](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-implementation-plan.md), [completion plan](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md), [series rules](../../doc/project/rule/pull-request.md#pr-series-plan), [test rules](../../doc/project/rule/testing.md#tr-plan-protocol)

## Purpose and current authorization

This plan packages the completed implementation for review while preserving its original requirements and acceptance criteria. `P0`–`P15` identify the original implementation packages. `PR01`–`PR17` identify proposed review units. `C00`–`C43` identify proposed publication commits. A PR number does not replace an original package identifier.

The user authorized preparing this plan on 2026-11-16 and authorized its execution with “proceed” on 2026-11-17. Execution covers isolated target preparation, the two specified test additions, three feature-requirement additions, reconstruction and verification within the listed groups/boundaries/order. The original topic branch and its local edits remain intact. Baseline changes, replacing the original branch, PR creation and pushing require their separately stated scope. No approval procedure is inferred from the draft planning policy. Existing source seals remain in force.

Checkout: `/home/miguel/workspace/inet-6lowpan-standard`. Input head: `9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61`. Pinned target: `91219b0fa7d2794ee67ac8a3dd45c3a09cfc45b0`, the local `origin/master` used for the completed campaign. The range has 95 commits. Eleven pre-existing local edits, comprising `AGENTS.md` and ten artifact-path maintenance edits, are excluded from publication input and must remain byte-identical.

The original plans remain authoritative for behavior:

- [P0–P7 first-release contracts](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-implementation-plan.md#4-implementation-sequence-and-exit-gates), including their [test matrix](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-implementation-plan.md#5-required-test-matrix).
- [P8–P15 completion contracts](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#completion-sequence-and-contracts), including the P10 stages, P11 server/host separation, forwarding prerequisites and original-MTU follow-up.
- The [completion ledger](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/doc/project/evidence/model/6lowpan/completion.md#final-combined-campaign) and [final record](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/doc/project/evidence/model/6lowpan/final-verification.json) own executed evidence for the original completed branch. They do not report results for this proposed series.

For early reconstructed commits, original plans and reviewer artifacts not yet present in that tree must be referenced through immutable input-revision URLs, for example [the original completion contract at the input head](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md). C00 must make those references resolve before any behavior commit is written. C41 restores the historical snapshots as local artifacts once the complete implementation exists. Do not introduce broken forward links or describe future packages as implemented at an earlier PR tip.

## Relationship to the implementation plans

| Original package or obligation | Publication owner | Acceptance retained |
| --- | --- | --- |
| P0: scope, standards, contracts and provenance | C00, then C07/C41 acceptance records | Clause/profile selection and evidence provenance, with no broader conformance claim. |
| Shared initialization and generic IPv6/ICMPv6 prerequisites | PR01 | Preserve the shared contract and verify its actual consumers. The initialization change originated in the Wi-Fi plan; its LoWPAN reuse is documented below. |
| P1 and the value/wire preparations of P2a/P6 | PR02 | Legacy receive metadata and explicit opaque dispatch; independent native address and frame vectors. |
| P2a/P2b/P3/P4/P5, production P6, and P7 | PR03 | Actual native MAC/adaptation composition, 1280-byte IPv6 service, fragmentation, stateless IPHC/UDP NHC and first-release acceptance. |
| P8 | PR04 | Stateful context selection, validity and reassembly snapshots. |
| P9 | PR05, using C03 from PR01 | Wire helpers, codec and production caller remain separate commits; generic decoded-Fragment ownership is available first. |
| P10 | PR06–PR10 | Preserve generic wire, binding, server, host, authority, relay and context-distribution responsibilities. |
| P11 | PR06, PR11 and PR12 | Generic representation precedes registrar/authority behavior, which precedes automatic updated hosts. |
| P12 | PR13 | Mesh identities, hop/duplicate/resource bounds and native forwarding. |
| P13 | PR14 | Packet-local page/ESC interpretation and opaque transit. |
| P14 prerequisites | PR15 | Native completion ownership and paced source behavior. |
| P14 and its original-MTU follow-up | PR16 | Early forwarding, ordinary fallback and frozen-path cancellation, including actual egress MTU. |
| P15 | PR17, using PR15 | Recovery wire/coordinate/state contracts and their direct production evidence. |
| Combined acceptance and publication | C41–C43 | Final feature matrix, provenance, archive and incoming links. |

P2b's temporary single-frame bring-up state is not a separate publication deliverable: C06 publishes the completed P2b–P7 implementation together, so packets admitted by MTU 1280 must already have fragmentation support. This preserves the final first-release contract rather than recreating an obsolete intermediate limitation.

## Proposed PR boundaries and prerequisites

The intended landing order is the table order. Direct prerequisites describe semantic dependencies; a simple linear stack additionally contains all preceding PRs. Every future commit also depends on the preceding commit within its PR. Reordering beyond this table requires another dependency review.

| PR | Purpose | Commits | Direct prerequisites |
| --- | --- | --- | --- |
| PR01 | Shared initialization and IPv6/ICMPv6 correctness | C00–C03 | Pinned target. |
| PR02 | Legacy opaque dispatch and native address/frame representations | C04–C05 | PR01. |
| PR03 | Complete native MAC and base adaptation | C06–C07 | PR01, PR02. |
| PR04 | Stateful IPHC | C08 | PR03. |
| PR05 | Extension and nested-IPv6 NHC | C09–C12 | PR03, PR04, C03. |
| PR06 | Generic ND wire and validity contracts | C13–C14 | PR01. |
| PR07 | Native registration server and router responses | C15–C21 | PR03, PR06. |
| PR08 | Automatic legacy hosts and retry timing | C22–C23 | PR07. |
| PR09 | Routed authority, native border admission and relay | C24–C27 | PR06–PR08. |
| PR10 | Discovery compression and authoritative context distribution | C28–C29 | PR04, PR05, PR07–PR09. |
| PR11 | Updated registrar/authority/relay behavior | C30–C31 | PR06–PR10. |
| PR12 | Common source guard and updated automatic hosts | C32–C33 | PR08, PR11. |
| PR13 | Mesh-under forwarding | C34 | PR03–PR05, PR12 for the unchanged current fixture/configuration interfaces. |
| PR14 | Paging and ESC dispatch | C35 | PR13. |
| PR15 | Native completion and paced source fragmentation | C36–C37 | PR03–PR05, PR12–PR14 for the unchanged current owner/fixture interfaces. |
| PR16 | Virtual forwarding and actual-MTU correction | C38–C39 | PR15 and the native source/policy owners from earlier PRs. |
| PR17 | Selective recovery and final acceptance | C40–C43 | PR16 and the complete preceding source/fixture surface. |

The earlier PR02 proposal included the native MAC. Source inspection narrows it to independently usable wire representations: `LowpanNativeMac` consumes `LowpanTransmissionReq`, `lowpanProtocol`, `LowpanPacketDropDetails`, `ILowpanMac` and the native link domain, while the native NIC, printer and dissector also include adaptation types. PR03 therefore retains that coupled production composition. No temporary feature, public API or standalone-MAC fixture is needed to manufacture a PR02 cut.

The existing two-file constant rename is split between C12 and C21 so each published feature uses its final constant names. This is a publication-only sequencing adjustment to the completion plan's final naming row. Both pieces remain token-only commits with their own reverse-map proof and relevant controls; their union must reproduce `f32ebb4a54` exactly.

## Commit and evidence conventions

Each C entry below is a future commit, not a claim that a corresponding tree has already been built. The source SHA identifies the original delta and artifact versions. Port that delta onto its proposed parent; do not copy an entire later source file containing future features. Preserve subsequent changes to shared fixtures only in their responsible C entries.

Every entry inherits these tasks:

1. Give the reconstructed commit a `Plan:` trailer to this publication plan at a path present in that commit, with C identifier and original P-package/contract links in its body. Preserve original IDs in the traceability record, not as claims about new commit hashes.
2. Port all behavior-owned tests, reviewer documents, release notes and necessary feature/NED/MSG/registration inputs from the source commit. Update the reviewer document's plan/commit association. For a protocol feature, its test and explanation ship in that behavior commit, as required by [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol).
3. Compare the exact reconstructed commit with its immediate parent using its fixture and assertions. Describe missing APIs/fixture build errors as compatibility gaps. A parent pass is a control result; do not invent a failing-parent result from source inspection or a crash that was not observed.
4. Execute the commit-specific selection below against a fresh matching library and record configuration, run, seed, observed/expected exits, generated inputs and loaded-library hashes. All proposed-series execution is currently **NOT_RUN**.
5. Keep baseline bytes unchanged. A mismatch requires diagnosis and the separate authorization required by the [baseline procedure](../../doc/project/guide/change-a-baseline.md).

Existing source-commit artifact inventories can be reproduced with `git diff-tree --no-commit-id --name-only -r <source-sha>` and `git show <source-sha>:<path>`. For every `.test` changed by an original behavior delta, assign its exact version and companion document to the same C entry unless an explicit split is specified below. Intermediate API/metadata adaptation must preserve the scenario, observations and assertions; log any changed fixture bytes.

In the entries, `U(...)`, `M(...)` and `P(...)` name unit, module and protocol fixture basenames, respectively. The shared verification section supplies commands. Existing paths are relative to `tests/unit/`, `tests/module/` and `tests/protocol/6lowpan/` unless a different directory is given. A named `.md` belongs beside its protocol fixture. The original reviewer document owns full case-by-case setup and limits; the entry supplies the publication-specific action and parent comparison.

## PR01

### C00

Subject: `plan: add: traceable 6LoWPAN publication sequence`. Dependencies: none. Introduce this plan, source IDs and immutable original-contract references before behavior commits. Consolidate the 43 historical contract-plan revisions into their current applicable requirements; retain their IDs in the source map below. Bring standards-only references/catalogs from `4a5fe385a0` without future implementation-status claims. This is documentation only, with link, coverage and dependency checks; it changes no protocol expectation or baseline.

### C01

Subject: `common: change: prepare interface identity before link-layer users`. Source: `3d7b8d9e14`. Dependency: C00. Owner: `InitStages`; original provenance is `plan/done/80211htcapop-refactor-v3.md`, not an invented original LoWPAN step. Reuse it for P0/P2a's readiness contract. `MacProtocolBase` assigns/registers its interface at NETWORK_INTERFACE_CONFIGURATION, but starts at LINK_LAYER and dereferences that interface. The native MAC and link-domain/adaptation consumers need that ordering too. Add the existing dependency edge without changing stage consumers or importing Wi-Fi refactors.

Verification: compile both modes; use the **pinned-base versions** of `M(Ieee80211MgmtStaSimplifiedInitialization_1|Ieee80211MgmtApLifecycle_1)` and run-0 `MacNonQos|MacQos` configurations under `examples/adhoc/qos`. Confirm both module fixtures exist at the pinned base before execution. These are shared-readiness regression controls; no fresh failing-parent observation is claimed. C06 adds the actual native consumer controls. A new protocol exchange/document is inapplicable to this initialization dependency itself. Preserve the original source's Wi-Fi provenance in the new commit body.

### C02

Subject: `icmpv6: fix: compute checksums with final IPv6 addresses and bound error size`. Source: `00b1253950`. Dependency: C01. Owner: `Icmpv6` checksum hook/error construction. The original gap computes without the final pseudoheader and budgets an error without the outer IPv6 header. Preserve final-address checksum completion, extension-header offset handling, the local-indication bypass and the 1280-byte complete-error bound.

Port `U(Icmpv6PseudoHeader)` and `M(Icmpv6LocalChecksum|Icmpv6ExtensionChecksum)`. Add proposed `tests/protocol/ipv6/Ipv6ComputedIcmp.test` and `computed-icmp.md`, derived from the existing `Rfc4443PacketTooBig.test` hostA–router–hostB network. With computed checksums and seed 0, send ordinary Echo traffic plus a 1500-byte original across an egress MTU 1280 link. Independently recompute the received Echo/PTB pseudoheader checksums using final addresses; require no oversized/fragmented error, correct advertised MTU and a bounded matching quote. Existing module vectors separately cover extension and local-indication branches; this protocol case must observe actual IPv6/MAC output. Compare the same new fixture on the parent; old source establishes the gap, while its exact runtime outcome remains unexecuted. Use the original protocol suite's existing topology/configurator/support library; no optional LoWPAN dependency is permitted.

### C03

Subject: `ipv6: fix: retain decoded Fragment header during local delivery`. Source: `a012505af0`. Dependency: C02. Original contract: completion plan P9's shared ownership prerequisite. Move this generic repair before its consumers. `Ipv6::localDeliver` must keep the decoded owning chunk alive while the raw Fragment-header pointer is used by reassembly. It adds no wire semantics or new state machine.

Port `M(Ipv6SerializedFragment)`. Add proposed `tests/protocol/ipv6/Ipv6SerializedFragmentDelivery.test` and `serialized-fragment-delivery.md`, using the existing `Rfc8200SourceFragmentation.test` network/tap support. Deliver a source-fragmented UDP datagram through the full receiving IPv6 stack. At the tap, preserve exact frame bytes while replacing the IPv6 region's typed chunks by their serialized `BytesChunk` representation, forcing receive-time decode. Require one complete byte/length/identity-matching UDP delivery and no extra delivery. The existing module case isolates the owner lifetime; the new protocol case proves normal continuation. A freed-pointer defect need not fail deterministically on every allocator/run: report the actual parent outcome, retaining the source-based ownership explanation. Do not change packet-core sources.

## PR02

### C04

Subject: `ieee802154: add: explicit opaque payload boundaries and receive identities`. Source: `4a09854a62`. Dependency: PR01. Original contract: P1 and P2a. Preserve the legacy MAC's configured upper-protocol boundary and source/destination receive metadata. Port all four module fixtures: `Ieee802154ReceiveMetadata`, `LowpanOpaquePayload`, `LowpanPayloadProtocolMismatch`, `LowpanPayloadUnknownProtocol`. The three Lowpan-prefixed harnesses use a local test protocol, not the Lowpan package. Correct metadata and opaque delivery must pass; unknown/mismatched protocols must produce their declared error exits. These direct production-MAC module cases own this boundary; native adaptation/radio protocol proof arrives in C06.

### C05

Subject: `ieee802154: add: native address and frame representations`. Source: the eight `src/inet/linklayer/ieee802154/Ieee802154Address*`, `Ieee802154FrameFormat.h`, `Ieee802154FrameHeader.msg` and `Ieee802154FrameHeaderSerializer.{h,cc}` inputs from `6a11f885ac`. Dependency: C04. Original contract: P2a/P6 wire preparation. Keep address mode/full EUI bits, PAN metadata, pinned frame fields and serializer registration together, with `U(LowpanNativeAddress|LowpanNativeFrame)`. Their literal address/PAN/header vectors, truncation/unsupported-control outcomes and CRC reference are independent of the native MAC and adaptation package.

Do not introduce `LowpanNativeMac`, Lowpan feature declarations, a native NIC, or the combined native printer/dissector here. This commit supplies independently tested representation helpers; C06 supplies their production consumer and protocol proof. Unit success does not complete P6.

## PR03

### C06

Subject: `lowpan: add: the complete native IPv6 adaptation data plane`. Sources: the remainder of `6a11f885ac` after C05, plus behavior-owned coverage from `fde0627634`, `9332039b2d`, `a36e789a70`, `52407824e6`. Dependency: PR02 and C03. Original contract: P2a–P7. Keep the coupled native MAC, native/compatibility domains/boundaries, lower envelopes, registered protocol/chunks, dispatch/codecs, fragment/reassembly owners, NED interfaces and feature declarations together. Introduce all three original Lowpan feature entries here. This is the first complete consumer of the native frame representations.

Static publication inventory exposes two declaration gaps in the original final source: `LowpanRadioProfile` includes/casts `StationaryMobility`, but Lowpan's required feature closure does not include Mobility; both leaves configure `PingApp`, but their required closure does not include PingApplication. Add `Mobility` to core `Lowpan` requirements and `PingApplication` to each leaf's requirements when C06 first declares them. This is an explicit, narrow correction to the original feature dependency inventory; it adds no protocol behavior. Current full/UDP-disabled execution did not test a minimal required-feature closure and does not prove this issue harmless. Preserve the existing core independence from UDP/applications.

The parent has representations and opaque legacy dispatch but no complete adaptation path. The result must deliver fitting and fragmented IPv6 through real native radios with MTU 1280, stateless IPHC and carried-checksum UDP NHC; preserve original fragment coordinates, bounded ownership, lifecycle cleanup and static native identities. The production decoder, header coverage, wire budgets and queue/MAC ownership must agree, which explains the single coupled behavior commit after extracting C05. Do not split it into unusable staged feature/API declarations merely to reduce line count.

Port every remaining unit/module fixture from `6a11f885ac` and the P7 protocol fixtures `LowpanNativeReassembly`, `LowpanNativeFragmentBoundaries`, `LowpanNativeIdentityIsolation`, `LowpanNativeReceiveRestart`, `LowpanNativeTagGuards`. Port their exact source-version reviewer documents `native-reassembly.md`, `native-identity.md`, `native-receive-restart.md`, `native-tag-guards.md` in this behavior commit. Fold the later fragmentation-unit and native receive-matrix expansions into this same commit so its first public tree has the P7 boundary/identity/lifecycle proof. Use the **pre-NHC** versions of `LowpanNativeReassembly` and `LowpanNativeIpv6Fragment`; C11 owns their later NHC-specific updates.

In addition to the original acceptance matrix, build the actual required closure of Lowpan with UDP/leaves disabled. Run its eight remaining original units plus `M(LowpanNativeDeadlineBoundary|LowpanNativeCcaPulse|LowpanIphcReceiveMatrix|LowpanMalformedReceive|LowpanReassemblyExpiry|LowpanPeerReceive)` and all five folded bare native protocol fixtures. These selections use bare NIC/radio/interface-table compositions rather than StandardHost/application modules. Then build each leaf's actual required closure and run `examples/lowpan/omnetpp.ini` and `tests/networks/lowpan/omnetpp.ini` PingApp compositions, plus the separate `examples/lowpan/route.ini` UDP composition. Record feature IDs and package exclusions, not just the four named flags. Use feature-tool reconciliation and exact restoration; these minimal-profile results are NOT_RUN. No new protocol fixture is required merely to mirror a dependency declaration: existing production compositions provide the direct check.

Assertions must cover all native sender-size profiles, independent PAN/sender/receiver identities, FRAGN-first/overlap/expiry behavior, exact bytes, computed UDP/ICMP wire checks, tag-wrap/guard exhaustion/restart and expected invalid native compositions. Preserve the peer-vector compatibility limit. Build/test generation failure on the pre-feature parent is API incompatibility, not a fabricated behavioral failure. Runtime parent comparison is claimed only for a compatible fixture/control actually executed.

### C07

Subject: `doc: add: first-release publication acceptance`. Sources: the first-release portion of `32bf7b81d7` and relevant P0/P7 evidence/documentation from C06's input. Dependency: C06. Record new per-PR source/library/configuration hashes and actual first-release enabled/off/core-without-UDP results; retain the original record as historical provenance. Do not relabel its old source hashes as the new PR's results. User documentation may describe only P0–P7 here; P8–P15 remain later publication features. This commit adds no protocol behavior or postponed tests. Original archive operations are accounted for as historical snapshot provenance in C41.

Retain the original P6/P7 independent-byte acceptance explicitly: regenerate the six native ICMPv6/UDP captures from their assigned module fixtures and run `doc/project/evidence/protocol/6lowpan/checks/check-capture.py` with computed-checksum validation through TShark. Reproduce the checked-in fragmented route-over adaptation vectors against the pinned peer as described in its README; preserve the separately characterized elided-EUI comparison as a failure, not an expected pass. These are bounded capture/serialized-vector checks, not a live external MAC/radio exchange or an ns-3 runtime dependency. Missing tool/peer execution is NOT_RUN and cannot discharge the corresponding acceptance row.

## PR04

### C08

Subject: `lowpan: add: stateful IPHC compression contexts`. Source: `58b446708d`. Dependency: PR03. Contract: [P8](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#p8-implementation-contract). Owners: `LowpanContextTable`, codec, layer/configuration. The parent rejects context-dependent forms. Supply per-interface prefix/permission/validity state to the codec and invalidate only the affected partial receive generations on replacement. Keep reconstructed FRAG1 snapshots stable after expiry.

Port `U(LowpanStatefulContexts)` and `P(LowpanStatefulContexts)` with `stateful-contexts.md`. Retain independent CID/address/multicast vectors, literal native input, exact expiry, receive-only permission, prefix replacement and fragmentation observations. Adapt the test/document association to C08. A missing context-install API on the parent is fixture incompatibility, already disclosed by the original record; use that limit instead of a runtime failing-parent claim. Run stateless and native reassembly controls too.

## PR05

### C09

Subject: `lowpan: add: extension NHC wire representation`. Source: `72f4df44f5`. Dependency: PR04. Contract: original P9 wire preparation. Add extension/nested marker chunks and serializers plus nested-IID derivation support in the codec. Port `U(LowpanExtensionNhcWire)`, preserving literal EID/NH/length/truncation and enclosing-address vectors. This independently useful wire preparation has no live adaptation consumer yet; it must not advertise production extension compression. Unit serialization proof is appropriate here; C11 owns the production protocol test/document.

### C10

Subject: `lowpan: add: contiguous extension NHC chain codec`. Source: `96d63e5156`. Dependency: C09. Contract: original P9 chain preparation. Owner: `LowpanNhcCodec`; add bounded contiguous encode/decode with separate original/compressed coverage and nested recursion. Port `U(LowpanNhcChains)`, including padding, nested IPv6/UDP, malformed depth/length and coverage vectors. The parent has representations but no chain transformation. Keep the independently callable codec separate from C11's layer wiring; no live radio claim is made by these unit vectors.

### C11

Subject: `lowpan: add: extension NHC transmission and reconstruction`. Source: `68b2b07355`. Dependencies: C10 and C03. Contract: original P9 production integration. Owner: `LowpanLayer` encode/decode/fragment coverage. Connect the chain codec; if the complete chain cannot fit FRAG1, retain base IPHC with NH inline. Preserve original IPv6 fragment coordinates and nested IID semantics.

Port `P(LowpanExtensionNhc)` and `extension-nhc.md`, plus this commit's updates to `M(LowpanNativeIpv6Fragment)` and `P(LowpanNativeReassembly)` and their existing explanations. Keep all eleven literal inputs and three real senders, every supported EID/nested chain, malformed inputs and both oversized and legal-but-over-budget fallback stimuli. Exact IPv6 bytes and the first transmitted representation must match the assertions. The parent lacks production NHC wiring; establish the actual parent result with a compatible fixture. Generic decoded Fragment ownership is already repaired by C03, so do not treat that unrelated failure as NHC evidence.

### C12

Subject: `lowpan: name: align the extension protocol table constant`. Source: only `LowpanNhcCodec.cc` from `f32ebb4a54`. Dependency: C11. Contract: completion plan's final naming row, explicitly sequenced earlier for publication. Rename `protocols` to `EXTENSION_PROTOCOLS` at exactly three tokens. Reverse-map to the C11 bytes; preserve types, values and control flow. Build both modes and run `U(LowpanNhcChains|LowpanExtensionNhcWire)` and `P(LowpanExtensionNhc)`. No new fixture/document or behavioral/baseline change is justified for this mechanical commit. C21 owns the other three tokens of the source commit.

## PR06

### C13

Subject: `networklayer: add: ICMPv6 registration wire support`. Sources: `9e670b6fd4` plus the enum representation and same-surface tests/document from `79f0762b95`. Dependency: PR01. Contract: [generic wire prerequisite of P10/P11](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#progress) and P10's explicit option-value contract. Owners: generic ND messages, ICMPv6 serializers/dispatch and ordinary ND rejection. Preserve raw option bytes and variable link addresses; represent DAR/DAC and the declared verifier widths without a dependency on optional Lowpan. Give the MSG option enum its fixed eight-bit underlying representation in this same commit: the new registration wire values exceed the original enum's range, so a later representation repair would leave an unnecessary unsafe intermediate. Preserve the enum API and every intended wire value. Ordinary Ethernet ND must safely reject unsupported representations.

Port `U(Ipv6NdRegistrationWire)`, including `79f0762b95`'s full-byte-value extensions, `M(Ipv6NdUnsupportedRegistration)`, `tests/protocol/6lowpan/nd-option-values.md` and `doc/project/evidence/model/6lowpan/nd-wire.md`. Literal vectors check exact fields/widths/truncation, raw reserialization and all intended option-byte values; the ordinary IPv6-to-ND receive fixture requires seven safe drops. These are generic representation/dispatch prerequisites, not native registration policy; C16 and later protocol fixtures supply that consumer proof. Historical combined native checks beyond this cut remain historical and are excluded from PR06's claimed execution scope.

### C14

Subject: `ipv6: add: query interface address validity`. Source: `90d1e36711`. Dependency: C13. Contract: P10 host/registrar lifetime prerequisites. Owner: `Ipv6InterfaceData`. Add read-only absolute-deadline and explicit-time validity queries; exact expiry is invalid, zero remains infinite, and querying cannot erase assignments or change `hasAddress` semantics. Port `U(Ipv6AddressLifetimes)` with finite/infinite/missing/tentative/update/removal boundaries. This query API adds no autonomous protocol policy, so no protocol fixture/document is required until its C20/C22 consumers.

## PR07

### C15

Subject: `lowpan: add: owned expiring neighbor bindings`. Source: `d37289ade0`. Dependencies: PR03 and PR06. Contract: [P10 binding prerequisite](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#p10-execution-stages). Owner: `LowpanNativeLinkDomain`. Add owner-specific expiring bindings whose lookup rejects expiry and whose updates cannot replace another live owner or static configuration. Port `M(LowpanOwnedBindings)` with competing owners, exact expiry, removal and static preservation. This storage/API commit does not change packet handling; C16 adds its protocol consumer.

### C16

Subject: `lowpan: add: native address registration server`. Source: `2fd73a474a`. Dependency: C15. Contract: P10 local-server stage. Owners: native ND registration table/timer and binding/route admission. A native router with forwarding enabled validates ARO requests, installs owner-specific native bindings and routes on success, and returns duplicate/cache-full/refusal without changing another owner. Static mode remains the default.

Port `M(LowpanDynamicBindings)`, `P(LowpanNeighborRegistration)` and `neighbor-registration.md` at this source revision. Literal native requests and actual radio replies/data must prove success/refresh/deregistration, malformed rejection, owner-specific errors, exact expiry and stop/crash cleanup. Keep native reply checksums computed. The parent has binding storage but no registration server; direct storage success is not the protocol result.

### C17

Subject: `lowpan: add: native router advertisements`. Source: `29214e905b`. Dependencies: C16 and C14. Contract: P10 independent router responder. Owner: native ND pending responses. Validate RS input, honor advertising policy, bound/coalesce owned delayed responses and emit native SLLAO/off-link PIO with remaining lifetimes. Port `P(LowpanRouterDiscovery)` and `router-discovery.md`: literal solicitations must produce real unicast RAs with the stated fields, bounded delay and lifecycle cancellation, while bindings remain unchanged. The parent server does not supply this discovery response.

### C18

Subject: `lowpan: add: native neighbor solicitation responses`. Source: `71d8fb4091`. Dependency: C17. Contract: P10 ordinary native reachability. Owner: native ND NS/NA dispatch. Respond to ordinary probes using validated receive identity, without creating ARO ownership; preserve multicast/DAD response rules. Carry this commit's `LowpanNeighborRegistration` and `neighbor-registration.md` extensions with the delta. Require actual ordinary unicast/multicast/DAD replies and unchanged registration state. Do not use the later final fixture version wholesale.

### C19

Subject: `lowpan: fix: reject malformed neighbor discovery before dispatch`. Source: `07b862cfd6`. Dependency: C18. Contract: P10 separate malformed-wire receive repair. Owner: `LowpanIpv6::handleReceivedIcmp`. Classify incomplete/malformed ND before ordinary checked dispatch while preserving valid ND and other ICMPv6 handling. Port `P(LowpanNdWireValidation)` and `nd-wire-validation.md`; static-role literal messages must drop safely for zero/truncated options and unsupported DAR/DAC suffixes while valid ordinary NS still receives a reply. No future authority configuration is needed.

### C20

Subject: `lowpan: fix: enforce registrar target and lifetime validity`. Source: `d2565ce9f5`. Dependencies: C16 and C14. Contract: P10 registrar validity stage. Port only the local admission delta onto the PR07 owner: validate the solicitation target at the current time and bound requested duration against the remaining clock range. Do not copy the original later ND file, which already contains authority code.

Port `P(LowpanRegistrarValidity)` and `registrar-validity.md`. The literal native fixture selects a local router, not a border router/DAD authority. Exact target expiry must refuse admission, valid targets must register, and extreme lifetimes must saturate safely without overflow. Static trace establishes that PR09 is not needed; runtime on this reordered tree remains NOT_RUN.

### C21

Subject: `lowpan: name: align the ICMP peek flags constant`. Source: only `LowpanIpv6.cc` from `f32ebb4a54`. Dependency: C19. Contract: the same final naming row as C12. Rename `flags` to `PEEK_FLAGS` at exactly three tokens and independently reverse-map to parent bytes. Build both modes and run `P(LowpanNdWireValidation)` plus the current registrar validity control. Do not select future `LowpanPinnedSourceValidity` merely because the original final naming campaign did. No new behavior, fixture or baseline is introduced. The C12/C21 union must equal the original six-token transformation.

## PR08

### C22

Subject: `lowpan: add: native router discovery and registration refresh`. Source: `bced36c7d3`. Dependency: PR07. Contract: P10 host/discovery stage. Owner: `LowpanHostRegistration` and its ND integration. The parent can answer discovery and registration but cannot originate automatic host discovery/refresh. Learn native next hops/default routes and M=0 /64 addresses, register usable addresses, refresh accepted leases and bound retries/retirement. Bootstrap ND remains available while ordinary host data is pending or refused.

Port `P(LowpanHostRegistration)` and `host-registration.md`. Observe real RS/RA/NS/NA, accepted/refused registrations and data, address lifetime updates, router choice and cleanup with the stated static/ordinary-interface controls. Carry all source/NED configuration inputs with this owner. Preserve exact-time valid/preferred lifetime behavior and source eligibility; do not add PR11's updated transaction state here.

### C23

Subject: `lowpan: fix: preserve registration response timing`. Source: `53b126d2a9`. Dependency: C22. Contract: P10 advertised retry timing. Owner: host retry/deadline calculation and its NED documentation. Respect a nonzero advertised Retrans Timer while retaining the local floor; allow the declared final response grace and saturate clock arithmetic. Port `P(LowpanHostRetryTiming)` and `host-retry-timing.md`. A literal RA/NA peer must observe the three requests at the correct intervals and a final grace period before failure. This fixture uses no authority/relay, so port the host delta without future authority context. C27 consumes the corrected timing relationship.

## PR09

### C24

Subject: `lowpan: add: routed registration authority`. Source: `112b8db04d`. Dependencies: PR06–PR08. Contract: P10 authority/relay separation. Owner: bounded ordinary-interface DAR/DAC authority state. Admit matching owners and expiring minute-based leases, reject duplicates/full capacity, and clear lifecycle state without inventing native bindings/routes. Port `P(LowpanRegistrationAuthority)` and `registration-authority.md`; actual ordinary IPv6 requests/replies must expose ownership, status, expiry and cleanup. The parent has native local registration but no routed authority.

### C25

Subject: `lowpan: fix: bound routed registration lifetimes`. Source: `d865f9f6bd`. Dependency: C24. Contract: P10 independent routed-lifetime range correction. Bound requested seconds before constructing `SimTime`. Port `P(LowpanAuthorityValidity)` and `authority-validity.md`: normal-epoch femtosecond clock, lifetime 65535, real DACs, saturated ownership, matching removal and replacement. The parent can overflow while constructing the duration; record the actual identical-fixture result. Do not couple this primitive repair to native border admission.

### C26

Subject: `lowpan: add: native border registration`. Source: `250cbcb68d`. Dependency: C25. Contract: P10 native border admission. Couple local native binding/route admission with authoritative ownership without reserving refused bindings. Port `P(LowpanBorderRouterRegistration)` and `border-router-registration.md`. Native requests and routed authority probes must prove both directions of ownership conflict, independent lease expiry, removal and no authority residue after failed local admission. Preserve static/default profiles.

### C27

Subject: `lowpan: add: routed registration relay`. Source: `936e4b9e7f`. Dependencies: C26 and C23. Contract: P10 bounded relay confirmation/fallback. Owner: relay pending operations, routing source choice and reply correlation. Confirm through the configured authority before binding installation; bound attempts/operation lifetime, preserve valid refresh data and support the original unanswered-request success fallback. Port `P(LowpanMultihopRegistration)` and `multihop-registration.md`. Observe real native NS/NA and ordinary routed DAR/DAC across the declared topology, authority rejection/fallback, retry/cancel paths and actual return data. A literal generic wire or local server test cannot discharge this multihop behavior.

## PR10

### C28

Subject: `lowpan: fix: keep discovery compression context independent`. Source: `d995bfefb8`. Dependencies: PR04, PR05 and PR09. Contract: P10 discovery before context knowledge. Owner: adaptation compression selection for RS/RA, including supported extension chains. Force stateless discovery encoding while ordinary registered data may use installed contexts. Port `P(LowpanDiscoveryCompression)` and `discovery-compression.md`: peer has no prior context, sender has matching configured contexts, and real discovery frames must remain independently decodable. Require normal discovery/registration progress and keep unrelated data compression behavior intact.

### C29

Subject: `lowpan: add: authoritative context advertisement distribution`. Source: `15656097de`. Dependency: C28. Contract: P10 complete authority-group distribution. Owners: configured context catalog in adaptation and bounded advertised/learned groups in ND. Export complete ABRO/PIO/6CO sets with separate origin/version spaces, remaining lifetimes, withdrawals and receive-only grace; install accepted contexts before downstream advertisement. Port `P(LowpanContextAdvertisements)` and `context-advertisements.md`. Native multihop RAs, exact deadline/version updates, authority expiry, replacement/deletion and host-admission independence must match their assertions. The parent has contextless discovery but no authoritative distribution. Update the overview portion sourced from `071ece0758` to this PR's actual scope.

## PR11

### C30

Subject: `lowpan: add: extended address registration`. Source: `4c573e0ffc`. Dependency: PR10. Contract: [P11 transaction-order policy](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#p11-transaction-order-policy). Owners: native registrars, authority/relay projections and bounded location/history state. Preserve full ownership verifier identity, transaction recency, wire/status precedence, stale/pending operation handling and legacy-authority compatibility.

Port `U(LowpanRegistrationInfo)`, `P(LowpanExtendedRegistration|LowpanExtendedRegistrationRaces)` and source-version `extended-registration.md`/`extended-registration-races.md`. Retain literal updated peers, real legacy host data and the independent cross-interface/lifetime/unknown-recency race stimuli. This boundary supports updated registrars but still rejects automatic updated-host activation: keep that restriction in NED/docs and in the no-radio rejection control. Do not copy the later host-enabled final file or its documentation backward from C33.

### C31

Subject: `lowpan: fix: mark asynchronous Moved advertisements unsolicited`. Source: `9810acc142`. Dependency: C30. Contract: P11 independent sender repair. Clear S on asynchronous Moved, preserve S on direct replies, and carry the responsible `LowpanExtendedRegistration`/`extended-registration.md` changes. Observe actual native NA flags for both triggers. This correction precedes the strict automatic-host consumer instead of hiding the sender defect inside that consumer.

## PR12

### C32

Subject: `lowpan: fix: validate native sources before every egress`. Source: `4663b7ca3d`. Dependency: PR11. Contract: P11 common source eligibility prerequisite. Owner: common IPv6 fragmentation/send entry after source/interface selection, including post-routing reinjection. Ordinary and interface-pinned output must use the same native registration predicate; bootstrap ND remains exempt.

Port `P(LowpanPinnedSourceValidity)` and `pinned-source-validity.md`. The exact fixture selects `extendedRegistration=false`, whose parameter already exists after C30; it uses the legacy host owner and real native UDP output. Preserve refusal of pending/expired source data, valid-source delivery and ND exemption. Keep this fixture after C30 unless an explicit configuration adaptation is separately recorded. It does not depend on C33's new host-history APIs.

### C33

Subject: `lowpan: add: extended host registration transactions`. Source: `389090ca3e`. Dependency: C32. Contract: [P11 automatic-host contract](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#p11-automatic-host-contract). Owner: per-address persistent host identity/TID history, frozen registrar copies, cooldown/retirement and reply correlation. Automatic updated hosts consume the registrar behavior introduced by C30/C31; default legacy behavior remains available.

Port `P(LowpanExtendedHostRegistration|LowpanExtendedHostLifecycle)`, `extended-host-registration.md`, `extended-host-lifecycle.md` and this source's modification of `extended-registration.md`. Actual RS/RA/NS/NA/DAR/DAC and return data must prove LL bootstrap, parallel registrars/legacy fallback, exact history/cooldown limits, stale replies and lifecycle cleanup. Update the earlier registrar document to describe its host consumer only now. The parent intentionally rejects the updated-host configuration; distinguish that profile rejection from a runtime packet defect.

## PR13

### C34

Subject: `lowpan: add: mesh-under forwarding`. Source: `c1ecdf38d6`. Dependency: PR12. Contract: original P12 and the [advanced forwarding contracts](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/plan/done/6lowpan-completion.md#advanced-forwarding-contracts). Owners: Mesh/BC0 representations, adaptation dispatch/identity, bounded broadcast duplicate/range and hop state. Keep logical origin/final identities separate from immediate link peers; forward opaque fragments before endpoint reassembly.

Port `U(LowpanMeshWire)`, `P(LowpanMeshForwarding)` and `mesh-forwarding.md`, together with printer/dissector registrations. Require multihop native unicast/broadcast delivery, independent wire identities, duplicate/loop/hop-limit handling, fragment coexistence, fixed resource/expiry and lifecycle boundaries. Default route-over remains unchanged. Static mesh data does not establish multihop optimized ND. Use the source-version fixture/configuration so newer recovery selectors are not introduced early.

## PR14

### C35

Subject: `lowpan: add: paging and extended dispatch`. Source: `ad2f88d50d`. Dependency: PR13. Contract: original P13. Owners: packet-local prefix parser, page/ESC chunks and structural packet tooling. Preserve separate FRAGN/NHC grammars and original IPv6 coordinates. Unsupported types fail at interpreting endpoints while permitted mesh transit preserves opaque inner bytes.

Port `U(LowpanPagingWire)`, `P(LowpanPagingDispatch)` and `paging-dispatch.md`. Literal page switches/ESC inputs and actual mesh transit must prove per-packet reset, header order, monotonic page-one history, fragmented reconstruction, unknown-type rejection and opaque forwarding. No EET payload protocol is added. The parent's fixed dispatch parser lacks these selected semantics; compare the compatible fixture and retain its exact-byte assertions.

## PR15

### C36

Subject: `lowpan: add: native transmission completion callbacks`. Source: `f09c571a4a`. Dependency: PR14. Contract: P14 native-completion prerequisite. Owners: native MAC/boundary terminal notification, borrowed original ownership and cancellation/drain eligibility. Unicast success requires matched ACK; broadcast success means local transmission only. Copy identities before transferring ownership and preserve safe synchronous observer cancellation and standalone native MAC composition.

Port `M(LowpanNativeTransmissionCompletion)` with all declared success/failure/STOP/CRASH/callback ownership cases and its source-version documentation in the model evidence. This is a local callback/lifecycle contract; the module harness is the direct proof. Do not equate success with endpoint/application delivery. C37 adds the protocol consumer.

### C37

Subject: `lowpan: add: paced native source fragmentation`. Source: `61d2a16426`. Dependency: C36. Contract: P14 source prerequisite. Owner: bounded retained originals and ACK-relative source scheduling. Freeze representation, gate later fragments on first success, retain the original deadline/tag guard and cancel unsent work on refusal/failure/lifecycle.

Port `P(LowpanPacedFragmentation)` and `paced-fragmentation.md`. Real native output/ACKs and endpoint bytes must prove spacing, first-frame failure, header-budget partitioning, admission/resource/deadline bounds, observer cancellation, context expiry and lifecycle cleanup. Keep the raw no-UDP-compatible stimuli in this version. The parent completion API has no paced-source owner; helper/unit success is not cadence proof.

## PR16

### C38

Subject: `lowpan: add: virtual fragment forwarding`. Source: `063659d694`. Dependency: PR15. Contract: P14 VRB plus native source/policy contracts. Owner: bounded `LowpanFragmentForwarding` state and its actual adaptation caller. Decode/preflight the supported first fragment, select/freeze the route, decrement outer Hop Limit, swap tags and forward before full reassembly under the declared compatibility profile. Retain ordinary processing for unsupported first fragments and fixed protection after forwarding starts.

Port `P(LowpanFragmentForwarding|LowpanFragmentForwardingLifecycle)`, `fragment-forwarding.md`, `fragment-forwarding-lifecycle.md`, and this source's `LowpanDatagramTags`/`LowpanReassembly` unit updates. Real two-PAN/three-node outputs must prove early byte-exact forwarding, all specified ND/control fallbacks, header expansion/budgets, ACK barriers, route/hook/source policy, tag/resource/deadline and reentrant lifecycle behavior. Source guard and ordinary IPv6 hooks are real admission/continuation predicates, not test bypasses. Preserve the historical P14 record as that checkpoint.

### C39

Subject: `lowpan: fix: honor IPv6 MTU before virtual forwarding`. Source: `d571d35530`. Dependency: C38. Contract: the explicit P14 actual-egress-MTU follow-up, originally executed after P15. Move the narrow header-based private path query/refusal here because it is a forwarding correction, not a recovery dependency. It adds no retained state or public API.

Port `P(LowpanFragmentForwardingMtu)` and `fragment-forwarding-mtu.md`: source adaptation injection represents a foreign paced peer; original 1281/encoded 1276 against actual MTU1280 must yield zero optimized output and a normal computed-checksum PTB1280; a 1200-byte control must forward early; an MTU1000 runtime invalidation after the first optimized frame must cancel the rest and leave one partial expiry. Preserve raw NH59, no UDP/TCP applications and the source-MTU-bypass explanation. The parent comparison must use a compatible final checker/library pair, not a different fixture or implicit ABI assumption. Run all three cases in the actual UDP-disabled build as well.

## PR17

### C40

Subject: `lowpan: add: selective fragment recovery`. Source: `66192266b8`. Dependency: PR16. Contract: original P15. Keep RFRAG/RFRAG-ACK wire formats, source partitions/credit/probe/repair, relay offset/tag/ACK routing and receiver/history/resource bounds in one usable protocol behavior commit. Preserve copied completion identities, actual route/source/policy/MTU/cap checks, frozen bytes and fixed cancellation/tag-history deadlines. Their coupled coordinates and operation state explain the commit boundary; no unused protocol subset is introduced.

Port `U(LowpanRecoveryPartition|LowpanRecoveryTags|LowpanRecoveryWire)`, `P(LowpanSelectiveRecovery|LowpanSelectiveRecoveryNoUdp|LowpanSelectiveRecoveryLifecycle)` and all three matching `.md` documents. Preserve actual eight-link window1/2/32 measurements and computed-checksum delivery, loss beyond MAC retry, validation of ACK/unsent bits, ordered repairs, reset/ECN/overlap/resource/history/tag reuse, region-tag provenance and reentrant lifecycle tests. Preserve the raw fixture's deleted-owner/tag-reuse assertions and its compatible pre-fix checker/library comparison as historical evidence; create a new exact-parent comparison only if actually executed. The raw 37-case and MTU fixtures must run in the actual UDP-disabled build. Keep the nine explicit invalid-configuration controls separate from normal completions.

### C41

Subject: `doc: add: complete 6LoWPAN publication verification`. Sources: final acceptance/overview/provenance portions of `4d4f211590`, `071ece0758`, `32bf7b81d7` and the historical plan/document snapshots listed below. Dependency: C40. Reconcile current user-facing configuration, selected-profile limits, standards source provenance and every new PR tip's result records. Retain original completed plans and historical evidence unchanged in meaning, identifying their original source/library cohorts. The original archive operations are provenance for these historical snapshots; they are not proof that the new series has run.

Perform the final whole-series acceptance in the verification section, record expected-error cases separately and compare printed release observations without enabling assertions by implication. Preserve inherited global architecture/naming/interface findings, the actual C++ warning exit and the historical ns-3 elided-EUI disagreement. No new external-peer or broad conformance result is inferred. User documentation must reflect implemented features only; no ns-3 runtime dependency is introduced. Record every old-source-to-new-commit mapping and every allowed extra test/document path before closing this entry.

### C42

Subject: `plan: location: archive the completed publication plan`. Dependency: C41 and successful new-series acceptance. This is a **new** archive of this publication plan, not a replay of either original implementation-plan archive. First complete its execution checklist and required evidence in C41. Then move `plan/pending/6lowpan-publication-plan.md` to `plan/done/6lowpan-publication-plan.md` with exactly the same Git blob in a separate location-only commit. Preserve a content hash and verify no other path is staged. The `Plan:` trailer must resolve at its new path.

### C43

Subject: `doc: change: link 6LoWPAN publication evidence to its archived plan`. Dependency: C42. Update only current incoming links and the new publication record associations. Keep historical original commit trailers, source hashes and archived input records historical. Validate every relative target/anchor, exact user-edit preservation, the final source/test projection and the complete new commit series. This documentation-only commit needs mechanical/link checks, not another simulation campaign when executable inputs are unchanged.

## Shared verification and evidence

### Existing evidence and proposed execution

The input branch's completed campaign is executed evidence for `9908c007bd` and its explicitly recorded source cohorts: enabled debug 22 unit fixtures, 69 module fixtures/110 runs (93 normal and 17 declared error exits), 33 protocol fixtures/285 runs; targeted release 16 protocol fixtures/226 runs plus 20 completion runs, with 391 matched observation rows; feature-off five shared modules and nine comparisons in three fingerprint simulations; core-without-UDP 22 units, three modules/22 runs and eight protocols/98 runs; exact feature restoration, both builds and three examples. The original [final record](https://github.com/mgonzalezlopezudc/inet/blob/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61/doc/project/evidence/model/6lowpan/final-verification.json) owns commands, hashes and limitations. No one of those counts is a claimed result for a proposed PR cut.

No new-prefix compilation, simulation, failing-parent comparison, baseline comparison or reconstructed-tree fingerprint has run during this planning task. Static dependency/fixture inventories support the boundaries; they do not supply runtime PASS. The two new generic IPv6 protocol fixtures and their reviewer documents in C02/C03 are **proposed and absent** at the input head. Their existing topology/support templates are present at the pinned target. Their exact source/API integration remains an implementation/test-authoring task, with no new production API planned.

### Commands

Use the actual checkout environment and explicit mode. These are command templates for later execution, not executed evidence:

```sh
source .envrc
make MODE=debug -j8
opp_run_unit_tests --load . -p inet -m debug --no-build --no-concurrent \
  -f '(FixtureOne|FixtureTwo)\.test$'
opp_run_module_tests --load . -p inet -m debug --no-build --no-concurrent \
  -f '(FixtureOne|FixtureTwo)\.test$'
opp_run_protocol_tests --load . -p inet -m debug --no-build --no-concurrent \
  -w '^tests/protocol/6lowpan$' -f '(LowpanFixtureOne|LowpanFixtureTwo)\.test$'
make MODE=release -j8
```

Substitute the exact U/M/P basenames in the relevant C entry. The two new C02/C03 protocol cases use `-w '^tests/protocol/ipv6$'` instead; they must exist and select nonzero cases before their execution can be reported. Build the matching mode before any test after compiled changes. Confirm CLI help, local `inet.opp` selection and test discovery in the new checkout. `--no-build` does not skip support/test compilation. Preserve every work directory/log/library before another runner clears or rebuilds it. One builder/runner per checkout.

C01 additionally uses the stored run-0 selection:

```sh
opp_run_fingerprint_tests --load . -p inet -m debug --no-build --no-concurrent \
  -w '^examples/adhoc/qos$' -c '^(MacNonQos|MacQos)$' -r '^0$'
```

Confirm stored ingredients, limits and expected results at the pinned target before execution. A missing stored case or zero selection is NOT_RUN. No baseline update is authorized.

### At every reconstructed PR tip

Record planned new evidence under `doc/project/evidence/model/6lowpan/publication/prNN-verification.json` (these files do not exist yet). Keep original records untouched as historical sources. Each new record must identify its exact parent/tip SHA, C IDs, original P contracts, source and configuration hashes, active features, loaded library hashes, selected fixtures and actual simulation counts/seeds, commands/exits, expected error cases and raw artifacts. Keep old-source-to-new-commit mappings separate from test verdicts.

Build debug/release and run every directly assigned test plus already introduced controls for the shared owners it changes. Run the current mechanical series gates over that PR's actual base/tip; do not inherit a PASS from the final input branch. Scope architecture/naming/interfaces/source seals appropriately, with the project-wide checks before publication. Preserve actual warning/failure statuses and compare inherited findings with the **new pinned base**, not an unrelated older gate log.

At PR03 and later feature-changing PRs, snapshot the complete feature state and generated NED inputs, exercise feature-off compilation/shared controls and core-without-UDP whenever those owners change, then reconcile through the feature tool and restore exact bytes in a finally path after all children terminate. Leaf features must be unavailable until their actual code/NED/examples exist. Before C06 the Lowpan feature is absent, so do not invoke its feature toggles or absent fixtures. At PR06 its generic wire/API checks must compile with Lowpan disabled.

Run release scenarios when mode-sensitive paths change, retaining unconditional observations outside ASSERT. Do not claim release assertions ran or compare all release cases indiscriminately: the historical legacy host fixture contains setup inside ASSERT. For every exact-parent comparison, the fixture/configuration/checker must be compatible on both sides. A compile error or absent API is not a protocol failure. New C03 ownership regression evidence may be a passing parent control on a particular allocator; report the result honestly.

### Final acceptance at C41

Repeat the original final campaign against the **new series**, adding C02/C03's new generic protocol cases. Discover/count the actual fixture set; the old 22/69/33 counts are a reference, not preset expected totals if additional fixtures are introduced. Require all assigned obligations to be represented without double-counting stale work directories.

Retain the explicit targeted release selection and observed-row comparisons; actual feature-off five shared modules and three `examples/wireless/nic` CSMA run-0 configurations/three ingredients; actual core-without-UDP codec/native/module and eight compatible LoWPAN protocol selections, including raw recovery and original-MTU; exact restoration, fresh builds and the three documented examples. Reconfirm each selector/dependency against the reconstructed tree. Preserve normal completions versus expected initialization/runtime errors separately. Run current architecture/naming/interfaces/source/document seals, full series checks and canonical C++ analysis with a matching compilation database. Do not suppress findings or rewrite baselines to manufacture a clean result.

Run the same-clock/seed/profile eight-link recovery bounds and the declared negative configurations. A local MAC ACK, fragmentation-endpoint FULL response and application delivery remain different observations. No new ns-3, radio interoperability, AP-ND or other excluded-profile result is implied by reconstruction.

### Final source and history checks

The selected 92 original commits and the three excluded Wi-Fi commits touch disjoint path sets in the input range. The target source projection is therefore explicit: on every path touched only by an excluded commit, retain the pinned target's blob (or absence); on selected model/feature/example paths, retain the input head's final bytes. Compare the actual reconstructed tree with that projection, not with the whole input `src` tree containing the intentionally excluded Wi-Fi refactors. Shared C01 remains selected.

Original executable fixtures must reach their input final versions after every assigned delta, except for explicitly documented plan-association/configuration adaptations that are necessary for reconstruction and validated as preserving their scenarios. The only planned production-input exception is C06's `.oppfeatures` requirements: add Mobility to Lowpan and PingApplication to each leaf. Verify the exact three requirement additions independently from model source equality. Additional executable code outside the projection is limited to the two proposed C02/C03 protocol fixtures and their test-local harnesses; they add no model behavior. New reviewer documents, publication plan, old/new map and per-PR verification records are separately allowlisted. Any other model/fixture discrepancy reopens the responsible C entry; do not silently call it a mechanical rewrite.

Preserve the input ref and its complete records before executing any future reconstruction. Use an isolated checkout; do not stash, rewrite or commit the eleven unrelated local edits. Verify a per-path hash snapshot before/after the task. Keep actual original sources reachable after any separately authorized history replacement. A future push/PR operation needs its own task scope.

## Planning checks and execution readiness

The 17 PR boundaries are supported by the native representation/consumer split and the P10/P11 source/fixture dependency review. The original four follow-up test commits add **five** protocol fixtures; all five are assigned to C06. The 44 future commits above have stable identifiers, a linear landing order and stated semantic dependencies. The old/new map will receive actual new hashes only during an authorized reconstruction.

Known publication-only changes relative to the executed sequence are explicit: C03 moves a generic P9 prerequisite earlier; C05 extracts independently usable native representations; C06 folds later first-release tests into their behavior; C20 and C23 move narrow independent fixes before future authority code; C12/C21 split and advance the final constant rename; C39 moves the P14 MTU correction before its P15 consumer. Original package requirements and selected profiles are unchanged. These adjustments need exact-parent/tip execution before their correctness can be reported as tested.

The two additional generic protocol harnesses close publication proof gaps; their results are NOT_RUN. C06 also corrects the explicitly identified required-feature closure gaps; both minimal-profile results are NOT_RUN. Plan review may check these bounded designs and existing support without claiming they compile. If implementation uncovers a missing required mechanism, revise the affected C entry before writing model changes. No protected-source or baseline exception is granted by this plan.

Planning acceptance:

- [x] Account for all 95 input commits: 92 selected, three explicitly excluded.
- [x] Check every original behavior-owned fixture/document against its publication C entry, including split-file ownership and stage-specific document versions.
- [x] Check all original P0–P15 acceptance obligations against the PR/C matrix without changing their meaning.
- [x] Validate local links, source refs, stable C identifiers, dependency ordering and proposed/existing fixture distinctions.
- [x] Complete an independent publication-plan review and resolve actionable findings.

Planning validation on 2026-11-16 found all 17 PR owners, 44 ordered C identifiers and 95 original dispositions consistent. Independent native-split and ND dependency inventories support the narrowed native-representation boundary and the specified reorders; independent plan review resolved the companion filename, unsafe enum intermediate and commit-anchor issues. Local link checks pass. Remote availability of immutable source URLs and every reconstructed-prefix build/runtime result remain unverified. The eleven pre-existing edited files, input HEAD and empty index are unchanged; the only checkout addition from this task is this plan.

Future execution (all NOT_RUN):

- [ ] Reconstruct the isolated PR01–PR17 stack and record actual new commit IDs.
- [ ] Implement the two proposed generic protocol fixtures/documents with C02/C03.
- [ ] Verify each exact PR tip, compatible parent comparisons and feature/profile state restoration.
- [ ] Prove final selected-source/fixture projection and preserved user edits.
- [ ] Execute/record C41 acceptance, then perform the separate C42 archive and C43 link commits.

## Original commit dispositions

The table below inventories the pinned input, not a cherry-pick script. `C00/C41` means consolidate the applicable original contract in the publication plan and retain the executed plan revision as historical provenance; it does not replay a broad plan-only commit into an arbitrary feature PR. Original test-only commits are folded into C06. Original archive/link operations supply historical snapshot provenance; C42/C43 operate on the new publication plan only. Each row must remain accounted for even when no old commit survives verbatim.

| Original commit | Original subject | Publication disposition |
| --- | --- | --- |
| [3d7b8d9e14](https://github.com/mgonzalezlopezudc/inet/commit/3d7b8d9e143198ec52058d61bd78a4979abdea6e) | common: change: prepare interface identity before link-layer users | [C01](#c01). Port delta with its original test/document/registration obligations. |
| [34a25f8f6a](https://github.com/mgonzalezlopezudc/inet/commit/34a25f8f6a99d278563f23290e0bf2c9937ecc91) | tests: location: move VHT HT capability checks to module tests | **Exclude**. Excluded: separate Wi-Fi work; retain pinned-target contents on its exclusive paths. |
| [e413d4d9a6](https://github.com/mgonzalezlopezudc/inet/commit/e413d4d9a6652602db35a53c20bee2a0addda090) | ieee80211: change: separate capability and BSS state ownership | **Exclude**. Excluded: separate Wi-Fi work; retain pinned-target contents on its exclusive paths. |
| [ae681ef355](https://github.com/mgonzalezlopezudc/inet/commit/ae681ef355f67e3160a15fb0b9eccfe3e506506e) | ieee80211: refactor: separate the read-only mode-set provider | **Exclude**. Excluded: separate Wi-Fi work; retain pinned-target contents on its exclusive paths. |
| [5d493a9a7a](https://github.com/mgonzalezlopezudc/inet/commit/5d493a9a7aece470b0571f02d9edae0616f39899) | plan: add: actionable 6LoWPAN implementation sequence | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [4a5fe385a0](https://github.com/mgonzalezlopezudc/inet/commit/4a5fe385a0ffcf91e8b64db541fd2b19d9d83f0a) | doc: add: 6LoWPAN standards references and statement catalogs | [C00](#c00), [C41](#c41). Standards-only sources first; preserve historical provenance at final acceptance. |
| [3e36c1f24a](https://github.com/mgonzalezlopezudc/inet/commit/3e36c1f24ade98c4882423c485bfffd3f176b6f8) | plan: change: clarify 6LoWPAN implementation contracts and milestone gates | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [00b1253950](https://github.com/mgonzalezlopezudc/inet/commit/00b12539506e075dff2733add2324845f44b0589) | icmpv6: fix: computed wire checksums and error packet sizing | [C02](#c02). Port delta with its original test/document/registration obligations. |
| [4a09854a62](https://github.com/mgonzalezlopezudc/inet/commit/4a09854a62b81c246e82237d5f9d025303e58c40) | ieee802154: add: support explicit opaque payload boundaries | [C04](#c04). Port delta with its original test/document/registration obligations. |
| [6a11f885ac](https://github.com/mgonzalezlopezudc/inet/commit/6a11f885acd503273abd53ab47d546f12b38b41e) | lowpan: add: the native IPv6 adaptation data plane | [C05](#c05), [C06](#c06). C05 owns only eight generic address/frame inputs and two units; C06 owns all remaining native/adaptation behavior. |
| [fde0627634](https://github.com/mgonzalezlopezudc/inet/commit/fde06276347eb9e5f0ab308d1e345540a5f4f3cf) | tests: add: native 6LoWPAN boundary protocol coverage | [C06](#c06). Fold all behavior-owned fixtures/documents and unit/matrix expansions into C06. |
| [9332039b2d](https://github.com/mgonzalezlopezudc/inet/commit/9332039b2ddfcc449a66d92f9e767eca620f2734) | tests: add: native 6LoWPAN reassembly identity isolation | [C06](#c06). Fold all behavior-owned fixtures/documents and unit/matrix expansions into C06. |
| [a36e789a70](https://github.com/mgonzalezlopezudc/inet/commit/a36e789a702f07dff6aab2a66c58ebe061796e1c) | tests: add: native 6LoWPAN receive restart coverage | [C06](#c06). Fold all behavior-owned fixtures/documents and unit/matrix expansions into C06. |
| [52407824e6](https://github.com/mgonzalezlopezudc/inet/commit/52407824e633f986a6054a041baae7fb1817ceff) | tests: add: native 6LoWPAN sender tag guard coverage | [C06](#c06). Fold all behavior-owned fixtures/documents and unit/matrix expansions into C06. |
| [c839f4328f](https://github.com/mgonzalezlopezudc/inet/commit/c839f4328f8320b86883dc002beb459a3d9dc544) | plan: location: archive the native 6LoWPAN first-release plan | [C41](#c41). Retain original archived snapshot; C42 archives a different, new publication plan. |
| [32bf7b81d7](https://github.com/mgonzalezlopezudc/inet/commit/32bf7b81d76081bda810065643b2f51afa55ad54) | doc: change: close bounded 6LoWPAN first-release validation | [C07](#c07), [C41](#c41). First-release acceptance and historical record, with new results kept separate. |
| [58b446708d](https://github.com/mgonzalezlopezudc/inet/commit/58b446708de858028dfd538b602d87c36264590e) | lowpan: add: stateful IPHC compression contexts | [C08](#c08). Port delta with its original test/document/registration obligations. |
| [72f4df44f5](https://github.com/mgonzalezlopezudc/inet/commit/72f4df44f5fcf8ca29070502782c88a4b6ab3d71) | lowpan: add: extension NHC wire representation | [C09](#c09). Port delta with its original test/document/registration obligations. |
| [96d63e5156](https://github.com/mgonzalezlopezudc/inet/commit/96d63e5156894c46bc6ac5f929d3e1d00cee8a70) | lowpan: add: contiguous extension NHC chain codec | [C10](#c10). Port delta with its original test/document/registration obligations. |
| [a012505af0](https://github.com/mgonzalezlopezudc/inet/commit/a012505af0c78460910d53dc65be310546acfb44) | ipv6: fix: retain decoded Fragment header during local delivery | [C03](#c03). Port delta with its original test/document/registration obligations. |
| [68b2b07355](https://github.com/mgonzalezlopezudc/inet/commit/68b2b073558066a9f43ffd154313fc2a8cbaa508) | lowpan: add: extension NHC transmission and reconstruction | [C11](#c11). Port delta with its original test/document/registration obligations. |
| [9e670b6fd4](https://github.com/mgonzalezlopezudc/inet/commit/9e670b6fd475955f42d4778df1878eba2b909f4b) | networklayer: add: ICMPv6 registration wire support | [C13](#c13). Port delta with its original test/document/registration obligations. |
| [47453f2525](https://github.com/mgonzalezlopezudc/inet/commit/47453f25255b87c0d1505e674bbf2e7d4b6d5e2b) | plan: change: define staged native registration and forwarding checks | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [d37289ade0](https://github.com/mgonzalezlopezudc/inet/commit/d37289ade03049e0473f726df7425f49c077fd04) | lowpan: add: owned expiring neighbor bindings | [C15](#c15). Port delta with its original test/document/registration obligations. |
| [2fd73a474a](https://github.com/mgonzalezlopezudc/inet/commit/2fd73a474a4b7ca00ac20097c2c2790e71ddaa41) | lowpan: add: native address registration server | [C16](#c16). Port delta with its original test/document/registration obligations. |
| [f2519ddc8c](https://github.com/mgonzalezlopezudc/inet/commit/f2519ddc8c729a1108ad8148de3b15ca250e1a0d) | plan: change: pin discovery timing and host lifetime contracts | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [90d1e36711](https://github.com/mgonzalezlopezudc/inet/commit/90d1e36711d80ffd68fb5589e66295667972a6fe) | ipv6: add: query interface address validity | [C14](#c14). Port delta with its original test/document/registration obligations. |
| [29214e905b](https://github.com/mgonzalezlopezudc/inet/commit/29214e905b4d82557628b80b8e26b5b51815de17) | lowpan: add: native router advertisements | [C17](#c17). Port delta with its original test/document/registration obligations. |
| [41f03384bb](https://github.com/mgonzalezlopezudc/inet/commit/41f03384bba898de966566bbddeaaddff441c4cb) | plan: change: define routed registration and native reachability contracts | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [e2539d5173](https://github.com/mgonzalezlopezudc/inet/commit/e2539d5173c21e2259b5b38e70f0896b006b5d5a) | plan: change: separate registration authority from relay | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [9a39e5d1e4](https://github.com/mgonzalezlopezudc/inet/commit/9a39e5d1e486d4aa585569e57031e6205b6bf869) | plan: change: separate malformed neighbor discovery receive repair | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [557b176bdb](https://github.com/mgonzalezlopezudc/inet/commit/557b176bdb753f0af8ff0f56489de92766097e05) | plan: change: define the complete neighbor option value contract | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [79f0762b95](https://github.com/mgonzalezlopezudc/inet/commit/79f0762b95200b80e715ae218d9a3168476096ad) | icmpv6: fix: retain every neighbor option byte value | [C13](#c13). Port delta with its original test/document/registration obligations. |
| [71d8fb4091](https://github.com/mgonzalezlopezudc/inet/commit/71d8fb40914f40d8af96df621165cff0316b694c) | lowpan: add: native neighbor solicitation responses | [C18](#c18). Port delta with its original test/document/registration obligations. |
| [bced36c7d3](https://github.com/mgonzalezlopezudc/inet/commit/bced36c7d35cdbcfcef0ae082a8e9691d555e125) | lowpan: add: native router discovery and registration refresh | [C22](#c22). Port delta with its original test/document/registration obligations. |
| [c9c1b596e2](https://github.com/mgonzalezlopezudc/inet/commit/c9c1b596e2ec5db49a67b183f4b343e3d5c8a185) | plan: change: define native border admission and relay timing stages | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [07b862cfd6](https://github.com/mgonzalezlopezudc/inet/commit/07b862cfd60d32eb1c0e1f63661eb7b5de33125c) | lowpan: fix: reject malformed neighbor discovery before dispatch | [C19](#c19). Port delta with its original test/document/registration obligations. |
| [112b8db04d](https://github.com/mgonzalezlopezudc/inet/commit/112b8db04d18e6e14e49c9cc81ef40316b7df248) | lowpan: add: routed registration authority | [C24](#c24). Port delta with its original test/document/registration obligations. |
| [d2565ce9f5](https://github.com/mgonzalezlopezudc/inet/commit/d2565ce9f57fbfac2598bc8ca7b62a624a6ed185) | lowpan: fix: enforce registrar target and lifetime validity | [C20](#c20). Port delta with its original test/document/registration obligations. |
| [c1edb6492b](https://github.com/mgonzalezlopezudc/inet/commit/c1edb6492b2ccb03f8e3b29d60940ebeeb232f89) | plan: change: pin independent border registration leases | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [05757cb069](https://github.com/mgonzalezlopezudc/inet/commit/05757cb06960a4b0c53559b75cb066207b875418) | plan: change: separate routed lifetime range correction | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [d865f9f6bd](https://github.com/mgonzalezlopezudc/inet/commit/d865f9f6bd0dc5a00dddbe957be451b5ab419b98) | lowpan: fix: bound routed registration lifetimes | [C25](#c25). Port delta with its original test/document/registration obligations. |
| [250cbcb68d](https://github.com/mgonzalezlopezudc/inet/commit/250cbcb68d10309ccacc33bfe9dde81866890df5) | lowpan: add: native border registration | [C26](#c26). Port delta with its original test/document/registration obligations. |
| [5af2837839](https://github.com/mgonzalezlopezudc/inet/commit/5af283783916f5232b609f9fc312dd7a3a4a3f88) | plan: change: pin context advertisement ownership | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [53b126d2a9](https://github.com/mgonzalezlopezudc/inet/commit/53b126d2a9fbe7851a2da2dc4c4ddf312bf21c26) | lowpan: fix: preserve registration response timing | [C23](#c23). Port delta with its original test/document/registration obligations. |
| [94656cba64](https://github.com/mgonzalezlopezudc/inet/commit/94656cba6497647c8aa5b3d3c034decc39a1febe) | plan: change: pin relay cancellation and source validity | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [a551050399](https://github.com/mgonzalezlopezudc/inet/commit/a551050399537555e036117e12a2eb81362b1b1e) | plan: change: pin context origin and discovery integration | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [936e4b9e7f](https://github.com/mgonzalezlopezudc/inet/commit/936e4b9e7f439b4df1370c935137b8e5748897be) | lowpan: add: routed registration relay | [C27](#c27). Port delta with its original test/document/registration obligations. |
| [49a17720c8](https://github.com/mgonzalezlopezudc/inet/commit/49a17720c8f75d1090c69f19b02f32cb8b34cded) | plan: change: separate advertised information from host admission | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [d995bfefb8](https://github.com/mgonzalezlopezudc/inet/commit/d995bfefb8357a104068dd6ff02dd5e6ca64707d) | lowpan: fix: keep discovery compression context independent | [C28](#c28). Port delta with its original test/document/registration obligations. |
| [795e10273d](https://github.com/mgonzalezlopezudc/inet/commit/795e10273d06f7d5ea070d1581c349e5c707da86) | plan: change: bound authoritative version updates | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [15656097de](https://github.com/mgonzalezlopezudc/inet/commit/15656097dec9f488a4184079b5400a49a30b2045) | lowpan: add: authoritative context advertisement distribution | [C29](#c29). Port delta with its original test/document/registration obligations. |
| [704c909f86](https://github.com/mgonzalezlopezudc/inet/commit/704c909f8608e3e151d744a5e3ee3b3babe1e686) | plan: change: pin extended registration state ownership | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [db23c7a00c](https://github.com/mgonzalezlopezudc/inet/commit/db23c7a00cb92e87a34790f885b3a4d63c76cff3) | plan: change: preserve unknown-recency notification safety | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [071ece0758](https://github.com/mgonzalezlopezudc/inet/commit/071ece0758dddf8da501f1c0ccbf584dd0ec66b2) | doc: change: reconcile the 6LoWPAN evidence overview | [C29](#c29), [C41](#c41). Context-stage overview and final reconciliation. |
| [7c0608549e](https://github.com/mgonzalezlopezudc/inet/commit/7c0608549efa767335ba4ff9407d22a62bee4205) | plan: change: define automatic extended host transactions | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [6a033e6264](https://github.com/mgonzalezlopezudc/inet/commit/6a033e6264e5328743e3249a9b04fed2cf66555f) | plan: change: bound mesh forwarding and broadcast state | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [424160301f](https://github.com/mgonzalezlopezudc/inet/commit/424160301f2151d933d81f6984d6163e67dcdfda) | plan: change: separate native completion before fragment forwarding | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [4c573e0ffc](https://github.com/mgonzalezlopezudc/inet/commit/4c573e0ffc8db555b7e4ca5b548a0b473329aba4) | lowpan: add: extended address registration | [C30](#c30). Port delta with its original test/document/registration obligations. |
| [d2d9b69921](https://github.com/mgonzalezlopezudc/inet/commit/d2d9b69921df8add1b634bd1393ec45e7bb8ce06) | plan: change: pin extended host retirement and cooldowns | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [d7011506fe](https://github.com/mgonzalezlopezudc/inet/commit/d7011506fe1f6b83d91cf6c6672fbb146624c0bd) | plan: change: split independent extended host lifecycle coverage | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [794bcde2b1](https://github.com/mgonzalezlopezudc/inet/commit/794bcde2b12966c2250c82f9a8fd489c8abd0abf) | plan: change: separate unsolicited Moved sender correction | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [e02031da9e](https://github.com/mgonzalezlopezudc/inet/commit/e02031da9e7c656e68909cfc2704f3766a8898af) | plan: change: verify common native source validation before hosts | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [9810acc142](https://github.com/mgonzalezlopezudc/inet/commit/9810acc14212faee2479d52775b358ee9c996ac0) | lowpan: fix: mark asynchronous Moved advertisements unsolicited | [C31](#c31). Port delta with its original test/document/registration obligations. |
| [4663b7ca3d](https://github.com/mgonzalezlopezudc/inet/commit/4663b7ca3daea8bff4be18aa6b4380586d0886ef) | lowpan: fix: validate native sources before every egress | [C32](#c32). Port delta with its original test/document/registration obligations. |
| [389090ca3e](https://github.com/mgonzalezlopezudc/inet/commit/389090ca3ef5c1057cc370d23e13b959f0574ba8) | lowpan: add: extended host registration transactions | [C33](#c33). Port delta with its original test/document/registration obligations. |
| [1256a061f7](https://github.com/mgonzalezlopezudc/inet/commit/1256a061f734f674656a3690abfae5b68cf34f60) | plan: change: pin native mesh profile and tag lifetime | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [df749e1c4c](https://github.com/mgonzalezlopezudc/inet/commit/df749e1c4c861b0d23dea872d3966f9d4b0542aa) | plan: change: bound broadcast fragment reuse and suppression | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [d28e10f460](https://github.com/mgonzalezlopezudc/inet/commit/d28e10f460802508ae196724f94f0069a01080d7) | plan: change: include mesh packet introspection | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [da3900d6c8](https://github.com/mgonzalezlopezudc/inet/commit/da3900d6c80c0119a728fb08341ea41852b8cb88) | plan: change: align mesh reservations with clock quantization | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [c1ecdf38d6](https://github.com/mgonzalezlopezudc/inet/commit/c1ecdf38d6979b3117f84645949f5b4047ed8909) | lowpan: add: mesh-under forwarding | [C34](#c34). Port delta with its original test/document/registration obligations. |
| [ad2f88d50d](https://github.com/mgonzalezlopezudc/inet/commit/ad2f88d50db542c4e056c7075cbb2499f83925b8) | lowpan: add: paging and extended dispatch | [C35](#c35). Port delta with its original test/document/registration obligations. |
| [7cb0dc3c52](https://github.com/mgonzalezlopezudc/inet/commit/7cb0dc3c52c929fc74b6ca0b430ea4dcdcdcd756) | plan: change: pin native completion drain eligibility | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [ad7b719290](https://github.com/mgonzalezlopezudc/inet/commit/ad7b719290aa20ed7a4676dcee109d3f442e2cdf) | plan: change: account for complete recovery header expansion | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [0083d05d91](https://github.com/mgonzalezlopezudc/inet/commit/0083d05d910b1f3a50013dc1c3552959b3a0ee5e) | plan: change: preserve standalone native MAC composition | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [f09c571a4a](https://github.com/mgonzalezlopezudc/inet/commit/f09c571a4a5a39f1d4b636a92f2df11a46af8782) | lowpan: add: native transmission completion callbacks | [C36](#c36). Port delta with its original test/document/registration obligations. |
| [6f74aaff74](https://github.com/mgonzalezlopezudc/inet/commit/6f74aaff7473bc2b297c196566327cc2a6218ba8) | plan: change: split bounded paced source fragmentation | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [61d2a16426](https://github.com/mgonzalezlopezudc/inet/commit/61d2a1642666291567b39a2986ff13d64e8a68ca) | lowpan: add: paced native source fragmentation | [C37](#c37). Port delta with its original test/document/registration obligations. |
| [bffa10f8f8](https://github.com/mgonzalezlopezudc/inet/commit/bffa10f8f8e4595471a626a6e8f6e3365a52b2d6) | plan: change: pin native fragment forwarding policy | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [99729354e9](https://github.com/mgonzalezlopezudc/inet/commit/99729354e9b20d0f9ca351d3b8b6a886c9f4ad72) | plan: change: preserve IPv6 policy hooks during fragment forwarding | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [8584a3238f](https://github.com/mgonzalezlopezudc/inet/commit/8584a3238fe90161f74895cf76e47f6470729922) | plan: change: bound recoverable fragment header growth | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [37b4503301](https://github.com/mgonzalezlopezudc/inet/commit/37b45033019ad48bd08e5ef37d6800fef5637c18) | plan: change: pin bounded fragment recovery scheduling | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [2f53450e0d](https://github.com/mgonzalezlopezudc/inet/commit/2f53450e0dcc8925cca43c961d7089cd24b0b84b) | plan: change: honor ordinary egress contracts in fragment forwarding | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [9ef6eb6438](https://github.com/mgonzalezlopezudc/inet/commit/9ef6eb6438653afdcb49108cd9280ad08969234c) | plan: change: schedule internal constant naming cleanup | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [063659d694](https://github.com/mgonzalezlopezudc/inet/commit/063659d6948ebf4cec932579f564298d7026bb00) | lowpan: add: virtual fragment forwarding | [C38](#c38). Port delta with its original test/document/registration obligations. |
| [10b553ffcd](https://github.com/mgonzalezlopezudc/inet/commit/10b553ffcdd5f1ce0d10d9533a14980e92eba9e1) | plan: change: bound recovery source fragment partitions | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [4e1c332205](https://github.com/mgonzalezlopezudc/inet/commit/4e1c33220535a4fe82e68ac7de5e7772d3fe91a6) | plan: change: pin recovery ownership and reset policy | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [461b5d4065](https://github.com/mgonzalezlopezudc/inet/commit/461b5d4065ce76ac551fc836307ddef1ae4ccb56) | plan: change: name recovery lifecycle and composition checks | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [9f9e5f59ff](https://github.com/mgonzalezlopezudc/inet/commit/9f9e5f59fff564de5c12b496e2a0fcd00f6d5796) | plan: change: honor IPv6 MTU in fragment forwarding | [C00](#c00), [C41](#c41). Consolidate applicable contract in C00; preserve executed revision as historical provenance at C41. |
| [66192266b8](https://github.com/mgonzalezlopezudc/inet/commit/66192266b89826c497928e81f9daa178fa3275ff) | lowpan: add: selective fragment recovery | [C40](#c40). Port delta with its original test/document/registration obligations. |
| [d571d35530](https://github.com/mgonzalezlopezudc/inet/commit/d571d3553054cb5596b1165c1245c1ea7725bea0) | lowpan: fix: honor IPv6 MTU before virtual forwarding | [C39](#c39). Port delta with its original test/document/registration obligations. |
| [f32ebb4a54](https://github.com/mgonzalezlopezudc/inet/commit/f32ebb4a54efb37b062ba5f74fbea9b0e340da56) | lowpan: name: align internal constant names | [C12](#c12), [C21](#c21). C12 owns three NHC-file tokens; C21 owns three IPv6-file tokens. |
| [4d4f211590](https://github.com/mgonzalezlopezudc/inet/commit/4d4f21159069ab0d1b18db0f1e34c0affe5faa61) | doc: change: close 6LoWPAN completion evidence | [C41](#c41). Final acceptance/documentation source, requiring new-series results. |
| [ed57a1e182](https://github.com/mgonzalezlopezudc/inet/commit/ed57a1e18202548e4b65c4717e2c02b7e83ddc18) | plan: location: archive completed 6LoWPAN plan | [C41](#c41). Retain original archived snapshot; C42 archives a different, new publication plan. |
| [9908c007bd](https://github.com/mgonzalezlopezudc/inet/commit/9908c007bdb1eda9d74f6ca1f99fe1cae33fbc61) | doc: change: point 6LoWPAN evidence to the archived plan | [C41](#c41). Retain original archived-plan links; C43 separately links the new publication plan. |
