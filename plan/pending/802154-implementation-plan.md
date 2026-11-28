# Implement IEEE 802.15.4 in INET

Status: **M1/M2 implementation and validation complete, 2026-11-15 — steps 0–10 closed**.
The [current results](../../doc/project/evidence/model/ieee802154/results.md) record
17 protocol fixtures/564 passing simulations, 14 module fixtures, eight unit fixtures,
native capture and external PHY checks, managed cycles, migration, stored fingerprints,
feature isolation and independent lifecycle rereview. All 87 selected statements
have an explicit bounded verdict; PIB-8 is conditional on an indexed service absent
from this binding. Inherited serializer/global mechanical failures remain visible.
No whole-standard or waveform conformance claim is made. Section 6 lists separate
later feature series. The completed plan remains here until the change lands.
Prepared against INET `c913a63a8335ca39a319cb2089daf835ea5e95cd` and the local
IEEE 802.15.4 survey on 2026-09-19.

Starting point: [initial gap analysis](802154-initial-plan.md). This document is the execution
plan; the initial analysis remains background. Implement the bounded profiles below before
extending coverage. “Correct” means satisfying the selected profile's applicable requirements,
with specification-derived tests and declared modeling limitations. It does not mean that every
PHY and optional mechanism in IEEE 802.15.4 has been implemented.

## 1. Scope and release milestones

Pin **IEEE Std 802.15.4-2024**, without subsequent amendments, using the existing
[source identity](../../doc/project/evidence/standard/ieee802154/source.md) and
[standards map](../../doc/project/evidence/protocol/ieee802154/standards.md).
The [IEEE register](https://standards.ieee.org/ieee/802.15.4/11041/) was checked on
2026-09-19; amendment implementation is a separate scope decision.

The following are project delivery profiles, not IEEE-defined conformance classes.

| Milestone | Selected behavior | Explicit limits | Release gate |
| --- | --- | --- | --- |
| M1: static non-beacon data transfer | 2.4 GHz O-QPSK, channel page 0/channels 11–26, 250 kbit/s; static PAN and native short/extended addressing; legacy frame versions 0/1 as applicable; data and immediate ACK; unslotted CSMA-CA; retries, filtering and DSN handling; PHY CCA, ED and LQI services | Unsecured operation; no dynamic association, periodic beacons, GTS, enhanced frames or ranging; packet-level PHY fidelity; an engineering milestone, not complete device conformance | Steps 0–6 and 10; every obligation in the M1 engineering subset has passing direct evidence; deferred profile obligations remain visible |
| M2: managed non-beacon PAN | M1 plus coordinator start, scan, association/disassociation, indirect transactions, polling, receiver control and selected orphan recovery/PAN conflict procedures | No claim of beacon-enabled operation or security | Steps 7–10; join, exchange, sleep/poll, failure and recovery scenarios pass |
| M3: beacon and secured operation | Beacon synchronization/superframes, slotted CSMA-CA, GTS/CFP, and a separately selectable base-standard security capability | Security and beacon mode have independent coverage; neither is implied merely by landing shared fields | Follow-on work packages B1–B3 and S1 |
| Later profiles | Enhanced frames/IEs, TSCH, DSME, additional PHYs and modern UWB/ranging | Each requires its own applicability matrix and validation | Follow-on packages E1, T1, D1, P1 and A1 |

Before implementing each profile, extend step 0 to reconcile its mandatory and conditional
requirements for the selected roles and modes; the first PR closes the bounded M1 audit. A necessary dependency discovered there joins the profile;
it cannot be excluded merely to meet this schedule. If needed for an M1 behavior, it blocks that
behavior until implemented; otherwise record its later delivery milestone without claiming profile
completion at M1. Until that closure is complete, use
“implementation of the selected subset,” not a whole-standard conformance claim.

In particular, 6.4.1.1 requires passive scanning for all devices. Its implementation is scheduled
in step 8, so it remains **owed** at M1 rather than being declared inapplicable to static PANs.
M2 is the first candidate for a complete selected-device-profile claim, subject to step 0's
mandatory-service inventory and the final evidence gate.

Maintain two explicit sets in the existing model coverage ledger: the **M1 engineering subset**
and **deferred selected-profile obligations**, including passive scan. Distinguish known deferred
implementation from unresolved applicability. M1 completion closes the first set only; a complete
selected-device-profile claim requires every applicable obligation to have passing direct evidence
and no unresolved applicability questions. M2 is a candidate, not a guarantee of that closure.

6LoWPAN, IPv6 neighbor discovery, routing and CoAP are separate projects. This plan supplies their
MAC address, payload and service boundaries; it does not promise an operational IPv6 stack merely
because MAC frames are correct. Native MAC test applications are sufficient for M1/M2 acceptance.

## 2. Corrections to the starting plan

1. Introduce PIB ownership and PHY service contracts **before** production MAC behavior uses them.
2. Ship each new wire chunk with its serializer, dissector/printer support and tests. Do not merge
   a packet model that has no byte representation; see
   [adding a protocol](../../doc/project/guide/add-a-protocol.md).
3. Implement the indirect-transaction and Data Request mechanism **before association**.
   In the selected association procedure, the coordinator delivers its response indirectly
   (2024, 10.21.2). An ACK to an Association Request is not successful association.
4. Distinguish representability, codec support and operational support. A parsed security header
   or IE is not an implemented security or TSCH feature.
5. Scope the two-octet FCS and 127-octet PSDU limit to M1/M2's PHY. Do not hard-code them as
   universal properties of all 2024 PHYs. Payload capacity depends on the actual MAC overhead.
6. Verify the sequence-wrap defect through a byte boundary. The current in-memory sequence
   field is wider than its serialized value; simulation-only exchanges may hide the mismatch.
   Do not prescribe a generic modulo ordering comparison as the entire duplicate algorithm.
7. Keep metadata local. Removing the Source PAN ID protocol-number workaround also requires
   a receive-side payload dispatch design; a sender's tag cannot act as an on-wire discriminator.
8. Retain the existing UWB composition as a separately documented legacy model. Replacing the
   narrowband MAC must not silently replace `AckingMac` on the UWB interface.
9. Use a device-wide DSN, not a counter per destination (6.6.1). Indirect delivery has a different
   retry rule from direct transmission: a failed attempt waits for another Data Request (6.6.3.4).

## 3. Evidence and current change surface

The starting-state observations in this section describe the preparation baseline named above, before M1/M2 implementation. They explain the original change surface; they are not a fresh inventory of the completed native implementation. Current results are linked at the top, and section 6 links the follow-on plans' verified starting points.

The existing [claim survey](../../doc/project/evidence/model/ieee802154/conformance.md) and
[coverage ledger](../../doc/project/evidence/model/ieee802154/coverage.md) establish Level 1
survey evidence only. Extend them rather than creating a competing status inventory.

The following observations were checked in this checkout; they are implementation facts, not
normative expectations for the new tests.

| Surface | Observed limitation | Planned owner/change |
| --- | --- | --- |
| [MAC header](../../src/inet/linklayer/ieee802154/Ieee802154MacHeader.msg) and [serializer](../../src/inet/linklayer/ieee802154/Ieee802154MacHeaderSerializer.cc) | Fixed `0xCC01`, padded 48-bit addresses, Source PAN ID carries `networkProtocol`; emitted header is 23 octets | Address value type in 1a; version-aware frame codec, steps 2–3 |
| [Narrowband MAC NED](../../src/inet/linklayer/ieee802154/Ieee802154NarrowbandMac.ned) | Fixed 72-bit header and 118-byte MTU derived from minimum overhead | Derive actual frame sizes; validate per-request payload capacity, steps 3–6 |
| [MAC implementation](../../src/inet/linklayer/ieee802154/Ieee802154Mac.cc) | ACK detection uses `CSMA-Ack`; reception-state CCA; host-integer sequence maps | Transaction, access and receive owners, steps 4–6 |
| [Dissector](../../src/inet/linklayer/ieee802154/Ieee802154ProtocolDissector.cc) | Payload decoding uses the nonstandard protocol field | Opaque payload or explicit receiver-side adaptation, step 3 |
| [Narrowband PHY](../../src/inet/physicallayer/wireless/ieee802154/packetlevel) | Existing transmitter/receiver/error model needs scoped timing and reception validation | PHY service adapter and O-QPSK profile, step 4 |
| [UWB interface](../../src/inet/linklayer/ieee802154/Ieee802154UwbIrInterface.ned) | Uses `AckingMac` | Preserve explicit legacy scope; modern UWB is P1 |
| [Serializer gaps](../../tests/serializer/REMAINING_GAPS.md) and [capture](../../tests/serializer/pcap/ieee802154.pcap) | Known frame-model blocker; capture exists but is not proof of codec support | Inspect provenance/link type and restore bounded capture coverage, step 3 |

Reuse `Protocol::ieee802154` and existing registrations. The PCAP reader already recognizes
link types 195 and 230 and attaches FCS-presence metadata; recognition alone does not prove
correct stripping, writing or replay. Test those paths explicitly.

### Normative lookup record

The local corpus was built for `802154-2024.pdf` and linted with the tracked
`inet-skills/bin/inet_process_standards` launcher. The PDF hash is in the source identity above.
These are normative clause entry points; a parent heading's page is **not** its complete scope.
Retrieve its children and referenced tables when deriving tests.

| Topic | Canonical node in `ieee802154-2024` | Physical PDF page(s) retrieved |
| --- | --- | --- |
| Unslotted access | `ieee802154-2024:clause:6.3.2.1` | 63–64 |
| Reception/rejection | `ieee802154-2024:clause:6.6.2` | 70–72 |
| ACK/retry entry | `ieee802154-2024:clause:6.6.3` | 72 |
| General format entry | `ieee802154-2024:clause:7.2` | 78 |
| Individual frame types entry | `ieee802154-2024:clause:7.3` | 86 |
| Constants/PIB entry | `ieee802154-2024:clause:8.4` | 151 |
| Association procedure | `ieee802154-2024:clause:10.21.2` | 356–358 |
| PHY services entry | `ieee802154-2024:clause:12` | 600 |
| O-QPSK entry | `ieee802154-2024:clause:13` | 614 |

The three extracted outgoing references from 6.3.2.1 resolved: 10.22.4.1 (Data Request),
10.11 (slotted access), and Figure 6-2. Corpus lint exited successfully but reported 169 rejected
false-heading candidates, one tiny node and 18 unresolved references across the document, plus
informational duplicate candidates. This is not a clean-reference or full-standard audit.
Figure 6-2 was visually inspected on physical page 64, and Table 7-2, including its continuation
and footnote, on physical pages 80–81. The flowchart's failure branch tests `NB >
macMaxCsmaBackoffs`, not equality. Step 0 still owes the remaining applicable figures/tables;
extracted flowchart text alone is insufficient. Outgoing references retrieved for 6.6.3.4,
10.21.2 and 11.2.8 all resolved. Resolution identifies a target; it does not mean the target's
requirements have all been implemented or audited.

### Source-derived constraints that must shape implementation

All clause/table IDs below belong to `ieee802154-2024`; their canonical form is
`ieee802154-2024:clause:<label>` or `ieee802154-2024:table:<label>`. These are bounded planning
findings from normative text, not the complete catalog to be produced in step 0.

| Source and physical PDF pages | Implementation consequence and required check |
| --- | --- |
| 6.4.1.1, pp. 64–65 | Passive scan is mandatory; scan channel order is ascending. Keep this obligation visible through the M1/M2 split and test restoration after scan. |
| 6.6.1, pp. 69–70 | Initialize sequence state through the simulation RNG; allocate a device-wide DSN, wrap at its representable limit, and keep beacon sequence spaces separate. Interleave two destinations in a test to expose the current per-destination counter model. |
| 6.6.3.3, pp. 72–73; 6.3.1, pp. 62–63 | ACK start timing and interframe spacing have separate rules. For this PHY the ACK timing uses `macSifsPeriod`; derive waits from the selected PIB/PHY values instead of retaining an unexplained fixed delay. Test long/short frames and acknowledged/unacknowledged exchanges. |
| 6.6.3.4, p. 73 | Direct retransmissions retain DSN and obey their retry limit. Failed indirect transmissions remain pending for another poll, retaining DSN; apply this to association responses too. |
| 7.2.2.6, p. 80; Table 7-2, pp. 80–81 | Legacy PAN compression rules and version-2 rules differ. In version 2, extended/extended with compression clear includes destination PAN only; with compression set it includes neither PAN. Test both and the table's short-address footnote when E1 lands. |
| 7.2.2.7–7.2.2.8, p. 81 | Legacy versions cannot enable sequence suppression or IEs. Reject unsupported combinations deliberately instead of silently changing offsets. |
| 7.3.3, p. 91; 6.6.2, pp. 70–72 | Immediate ACK layout lacks source/destination addresses; matching uses DSN and the outstanding exchange context. Version-2 traffic requires enhanced-ACK behavior rather than reusing immediate ACK blindly. |
| 7.2.11, pp. 84–85 | FCS covers MHR plus MAC payload. Use the standard's ACK CRC example as one independent vector, respecting transmission bit order; do not apply an Ethernet CRC preset. |
| 9.2.5, pp. 165–166 | An unsecured profile still defines security-policy behavior: with `macSecurityEnabled=false`, the unsecured incoming procedure succeeds. Secure inputs and attempts to enable unavailable security need explicit outcomes. |
| 11.2.8, pp. 596–597; Table 12-2, pp. 601–603 | CCA uses `phyCcaDuration` and `phyCcaEdThreshold`; a request during PPDU reception reports busy. M1 selects Mode 1 (energy detection); other modes are unsupported until separately implemented. |
| Table 8-36, pp. 152–156, especially p. 156 | Derive the default `macUnitBackoffPeriod` from turnaround plus CCA duration converted to whole symbols with upward rounding. A fixed 320 microseconds is not valid for every configured CCA duration. Test a nonintegral symbol conversion. |
| Table 12-1, p. 601; 13.1.2.2, p. 614; 13.1.3.2, p. 615 | O-QPSK turnaround is 12 symbols, preamble is 8 symbols, and PHR length counts the entire PSDU. Assert the duration calculation against bytes including MAC FCS. |
| 10.21.2, pp. 356–358 | Association ACK is not admission; response delivery uses indirect transactions. Keep higher-layer admission/address-allocation policy separate from the MAC exchange. |

The retrieved transmission/ACK clauses do not specify a general-purpose duplicate-cache algorithm.
6.6.1 explicitly limits the usefulness of the 8-bit DSN; 8.3.6 exposes it in the data indication.
Step 0 must establish any further applicable duplicate obligations. Until then, treat suppression
as a separately documented model/adaptation policy, not an invented standard requirement.
DSN alone cannot guarantee exactly-once application delivery across wraparound or peer restart.

## 4. Architecture and ownership decisions

Follow the current [project map](../../doc/project/README.md),
[protocol anatomy](../../doc/project/design/protocol-anatomy.md),
[packet anatomy](../../doc/project/design/packet-anatomy.md) and
[architecture rules](../../doc/project/rule/architecture.md). Proposed subpackages below are
implementation destinations, not existing APIs. Settle concrete names and paired C++/NED
contracts in step 1 using the [naming rules](../../doc/project/rule/naming.md).

| Owner | State and responsibility | Boundary |
| --- | --- | --- |
| Protocol-local address/frame values | None/short/extended address, FCF fields, PAN fields, command payloads, optional wire structures and FCS | `linklayer/ieee802154`; independent of MAC timers and radio implementations |
| MAC PIB | Validated MAC attributes, defaults and access restrictions | One authoritative store; no duplicate NED/management/runtime copies |
| PHY service provider | Channel/page, transceiver operation, CCA/ED, transmission timing, receive metadata | Paired contracts in PHY-owned `contract`; implementation under `physicallayer/wireless/ieee802154`; no dependency back to concrete MAC |
| MAC transaction controller | Explicit operation type, current request, DSN, ACK matching, retry count, exactly one terminal result | Calls access and PHY contracts; owns packet lifetime through completion |
| Unslotted/slotted access provider | Per-attempt backoff state and timers | Returns access success/failure; does not own retries, association or upper-layer policy |
| Receive processing | Length/FCS/filter checks, ACK eligibility and delivery; optional duplicate policy has explicit ownership | Single receive decision path for typed and byte-backed packets |
| Management | Scan, start, association/disassociation and recovery procedures | Uses shared transmission services; PAN choice/address-allocation policy is supplied by a higher-layer client |
| Pending transaction store | Per-device queued data/commands, expiration and purge | Shared by association and indirect data; no second association-only queue |
| Security provider, later | Key/device state, counters and security transformation | Defined hooks and ownership early; implemented only with S1 evidence |

Queueing uses existing queue contracts. Do not create a submodule for every value type; use
replaceable modules for independently selectable behavior and plain values for packet/state data.
Keep management and channel access out of one expanding MAC FSM. Specify separate direct,
indirect-delivery, poll and association operation lifetimes and completion events. Direct retry
exhaustion, indirect retention for another poll, poll response waiting and association completion
must not share an implicit success/failure rule. Decide whether to share an exchange engine only
after these contracts are explicit; no particular policy-class hierarchy is required.

Every asynchronous operation has a request identity, result/status, packet ownership rules,
cancellation behavior and lifecycle semantics. PHY busy, unsupported operation, invalid request,
channel-access failure, no ACK and successful completion must remain distinguishable. All timers
have one owner; late callbacks after cancellation or stop cannot complete a replacement request.

**Address integration:** native 16/64-bit identity lives in the protocol. Use protocol-specific
request/indication tags and attached interface data where existing extension points permit it.
Do not widen `MacAddress`, truncate EUI-64, or add protocol switches to the core. In step 1d,
before codec and operational MAC work, prove that two extended addresses sharing their low 48 bits remain distinct through the actual
interface. Any framework contract gap is a separate design decision with its own change surface.
Produce a compatibility matrix for interface identity, upper destination requests, lower source
indications, queue classification, neighbor/forwarding lookups, filtering, display/configuration,
and generic consumers expecting `MacAddress`. Mark each as supported, adapted or outside M1;
name the actual extension point and required evidence for supported paths. M1 need not make every
generic upper layer compatible, but its native MAC application path must work end to end.

**Payload integration:** M1 accepts opaque MAC service payloads and optionally one configured
upper protocol per interface. The receiver supplies that protocol tag for delivered data payloads;
without configuration, payloads remain opaque and use the native MAC service boundary. Unknown
capture payloads stay opaque. Mixed upper protocols on one interface are unsupported until an
adaptation-layer discriminator exists. The MAC and dissector must use the same context rule.

**PIB mutation:** step 1b defines each attribute's access, startup/runtime mutability, validation,
cross-attribute constraints, reset default and retention, effective-change event and notifications.
NED parameters initialize the authoritative store; they are not a competing runtime value. Define
how MLME-SET interacts with active operations and reject unsupported changes with explicit status.
The owning procedure applies operational changes after validation; the store must not reach into
unrelated modules. Preserve consistent state across failed changes. Add atomic multi-attribute
operations only if a supported procedure requires them, with explicit commit/failure semantics.

**Frame order:** follow the selected version's layout: MAC header (including applicable auxiliary
security/header IEs), MAC payload (including applicable payload IEs in their specified position),
then FCS. Do not append payload IEs after application data merely because the initial sketch did.

**Protected surface:** `src/inet/common/packet/` is sealed in the
[registry](../../doc/project/audit/seal-list.md). Protocol serializers and dissectors can register
outside it. If capture integration exposes a core defect, produce the reproduction and a bounded
repair proposal, then follow the current seal procedure before editing that path. This plan does
not grant permission to edit sealed code.

## 5. Ordered implementation work

Numbered steps are roadmap stages, not promises of one PR each. Split them at the gates below,
with a bounded change surface and direct checks for each PR. Land dependencies first; every PR
must build and keep its selected operational mode coherent. The owner column in section 4 names
responsibility, not staffing assignments.

### Step 0 — Close the profile and write the checks

Dependencies: none. Source baseline and local survey already exist; refresh them if HEAD moves.

Execution record: [applicability audit](../../doc/project/evidence/model/ieee802154/applicability.md)
and [inspection evidence](../../doc/project/evidence/model/ieee802154/results.md). The bounded
M1 audit passed independent review; the [reference dispositions](../../doc/project/evidence/model/ieee802154/dependencies.md)
record its closure boundary. This does not close the later M2 audit or any runtime gate.

- [x] Extract the selected normative statements into
  `doc/project/evidence/standard/ieee802154/catalog.md`, including conditions, revision, clause,
  physical page/source span and cross-references. Inspect ambiguous tables/figures in the PDF.
- [x] Create protocol-only `features.md`, `checks.md` and focused check descriptions under
  `doc/project/evidence/protocol/ieee802154/`, following
  [standard-derived tests](../../doc/project/guide/derive-tests-from-a-standard.md).
- [x] Pin the first audit to M1's static non-beacon O-QPSK data service and its necessary
  dependencies. Declare which device, coordinator and PAN-coordinator roles are exercised;
  distinguish configured roles from support for their management procedures. Record exposed
  services, selected CCA mode, conditional predicates and unsupported PIB requests/statuses.
- [x] Record the M1 engineering subset and known deferred profile obligations in the model ledger.
  Follow applicable cross-references to closure for the bounded M1 claim. Extend the audit to M2
  roles/services before steps 7–9; the first PR need not close unrelated M2 procedures. Keep
  implementation status and outcomes out of the standard catalog and protocol feature map.
- [x] Produce a version matrix separating transmitted, structurally decoded and operationally
  processed formats, legal-but-unsupported combinations and malformed/reserved combinations.
  Resolve version 0/1 selection before codec work. Version-2 recognition must never imply
  legacy-layout parsing or operational support; E1 remains the implementation gate.
- [x] Derive a receive decision table from the applicable clauses: for each rejection or acceptance
  cause, record ordering, promiscuous visibility, ACK eligibility, DSN indication, delivery,
  duplicate-state effects and status/statistic. Include malformed security fields, valid secured
  but unsupported traffic and unsecured traffic; add companion outcomes for local requests
  asking for unavailable security. Resolve the table before step 6; do not reduce ACK eligibility to one generic accepted flag.
- [x] Record apparent source contradictions with exact clauses/figures and resolve their
  interpretation before turning them into timing oracles. Do not silently substitute an older
  edition, an extracted table fragment or another simulator's algorithm.
- [x] Audit existing examples, callers, address tags, PCAP paths, `.oppfeatures` and fingerprints;
  identify migration consumers and capture provenance/admission. Suspected defects remain assigned
  to separate production-path reproductions; source arithmetic is not reported as a reproduced run.

**Exit:** a reviewer can determine the profile's obligations without reading implementation code.
Every applicable obligation in the bounded audit has a planned observable check and delivery
milestone; exclusions have explicit predicates and source justification. The bounded M1 audit has
zero unresolved applicability questions or unclassified necessary references. Known implementation
debt remains visible and does not count as an applicability exclusion. Unresolved questions for
later procedures block those procedures and any complete-profile claim, not unrelated M1 work.
A machine-readable coverage export, if useful, is derived from the ledger rather than maintained
as another source of truth.

### Step 1 — Fix contracts, state ownership and migration boundaries

Dependencies: step 0. Deliver as separate bounded PRs; each builds with the existing default.

| Package | Deliverable and exit gate | Dependencies |
| --- | --- | --- |
| 1a: protocol values and MAC services | Native none/short/extended address type, parsing/formatting, equality/hash and reserved values; MCPS and minimal MLME primitives, statuses, request identity, ownership and cancellation. Value/contract tests cover refusal and distinct identities. | Step 0 |
| 1b: MAC PIB | Authoritative store plus the attribute mutation table specified in section 4; initialization, validation, reset and failed-change tests. | 1a |
| 1c: PHY contracts | Paired substitutable contracts, units, status/lifecycle rules and event/timer ownership table below. Contract tests distinguish acceptance from completion and stale callbacks. | 1a |
| 1d: address and payload integration | Compatibility matrix and minimal production interface fixture, including two EUI-64 identities with identical low 48 bits, request/indication identity and configured/opaque delivery. Resolve required adapters and framework gaps. | 1a–1c |
| 1e: selectable composition | Public type names, paired C++/NED contracts and explicit replacement selection. Prove existing default initialization and replacement contract wiring; isolate old wire assumptions. | 1d |

The following package paragraphs preserve their initial boundary checkpoints;
steps 2–10 and the final execution record close their then-pending production gates.

Package 1a is complete: the standalone native address value and paired MAC service contracts have
passing focused unit/module cases and independent reviews. See the
[value evidence](../../doc/project/evidence/model/ieee802154/results.md#step-1a-native-address-value) and
[service-contract evidence](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/results.md#step-1a-mac-service-contracts).
The module fixture uses a test-local provider. The selected PIB store is implemented in 1b;
operational service integration, the real interface fixture (1d), and wire-codec
checks were still owed at that checkpoint. Those gates are now closed by steps 2–10.

Package 1b is complete at the store boundary: the selected 43-attribute MAC PIB has a passing
focused unit case and independent review. The
[selected MAC PIB contract](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/pib.md) records access,
mutation, startup and reset semantics; [results](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/results.md#step-1b-selected-mac-pib)
record the evidence and operational integration limits.

Package 1c is complete at the structural contract boundary: paired PHY roles, typed operation
values and the [event/timer ownership contract](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/phy-services.md)
have a passing test-local module case. Production PHY timing/measurements, radio adaptation,
runtime PHY-to-MAC PIB context refresh and propagation-bound validation remain steps 4–6 work.
The contract uses the 2024 source's undefined PD/PLME-SAP boundary; its methods and local statuses
do not claim standardized primitive signatures. See the
[execution evidence](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/results.md#step-1c-phy-service-contracts).

#### IEEE802154-1C — paired PHY services and timing ownership

Proposed subject: `ieee802154: add: define typed PHY service contracts`.
Parent dependency: packages 1a/1b. Purpose: make admission, owned PSDUs, readiness, channel
mutation and exact air-event boundaries explicit before a real PHY adapter or MAC exchange uses
them. Previously there was no typed PHY provider/client boundary; acceptance could not establish
when an on-air operation began or when its result became usable.

The owner is the PHY service provider. Changes are confined to
`src/inet/physicallayer/wireless/ieee802154/contract/`, the model contract/evidence documents,
`tests/module/Ieee802154PhyServiceContract_1.test` and the compatible release-note fragment.
The caller uses pure paired C++/NED roles. A requested PPDU start is absolute; the actual-start
callback and successful terminal timing distinguish admission, prediction and completion.
Configuration stays unchanged on refusal or failed validation. Reset/stop retire old IDs and
packets before callbacks. No existing MAC, PHY, default interface, wire format or baseline changes.

Verification: fresh debug INET library, then the exact module selector
`Ieee802154PhyServiceContract_1\.test$` with `--load . -p inet -m debug --no-build`.
The new fixture fixes run/seed 0 and uses typed calls plus scheduled completion, stop/crash and
receive events. It asserts retained ownership on refusal, provider ownership after acceptance,
packet destruction before confirmation, atomic channel validation, copied request identity,
readiness/airtime/measurement metadata, scheduled-start cancellation, callback reentry, reset
ordering, stale completion rejection, same-time deadline precedence and deleted-client cleanup.
Before this commit the fixture cannot compile because the API is absent; no runtime before/after
protocol comparison is claimed. Protocol tests and a protocol reviewer document do not apply
to this structural contract addition. The production PHY and protocol checks remain step 4–6
deliverables. Scoped architecture, interface, naming and NED checks apply to the new contract
subtree; the run record distinguishes executed checks from those later obligations.

Record these decisions before dependent behavior, without requiring all contracts to land in one
PR. No concrete PHY-to-MAC dependency or protocol switch in core is permitted.

The PHY event/timer table must define request acceptance, PPDU start, PSDU/FCS completion, PPDU end,
turnaround completion and receive indication, including timestamp reference points and result
availability. Assign one owner to CCA/ED observation, turnaround, backoff, immediate-ACK scheduling,
interframe spacing, ACK timeout and transmit completion. Specify which events start each wait,
which intervals overlap, units/rounding and cancellation/reset behavior. Resolve exact assignments
against the selected clauses before step 4; no timing implementation may rely on an unassigned row.

**Exit:** all five package gates pass; validation, refusal ownership, reset/stop and distinct
operation results are explicit. Step 2 consumes the address type already proven by 1d.

Continuation on 2026-11-15: package 1d's production native adapter and attached interface data
pass `Ieee802154NativeInterface_1.test`, including distinct high address bits and receiver-owned
payload dispatch. Package 1e's replacement composition passes `Ieee802154Composition_1.test`.
Existing-default initialization and operational radio/access wiring remain gates; replacement
DATA explicitly refuses unsupported operation at this stage. The
[integration contract](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/integration.md) records the
compatibility matrix and exact public type names. No default cutover has occurred.

Series entries for these additions are `IEEE802154-1D` (`ieee802154: add: integrate native MAC
service identity`) and `IEEE802154-1E` (`ieee802154: add: expose selectable standard MAC
composition`). Their production gate/module fixtures and limitations are recorded in that
contract and in the execution results. The composition is structural; it does not claim a new
on-air protocol capability. Later operational packages must include protocol tests and reviewer
documents when they enable that capability.

### Step 2 — Basic frame codec using native addresses

Dependencies: step 1.

- [x] Use the step 1a address type and step 0 version matrix; carry broadcast and
  unallocated-address semantics through the actual wire boundary.
- [x] Add the M1 FCF/address/sequence representation and registered serializers together.
  Derive the version-dependent address/PAN presence table from the specification.
- [x] Support data and immediate ACK layout, variable lengths and exact octet order; distinguish
  unsupported legal formats from malformed inputs. Do not use `ASSERT` for untrusted wire input.
- [x] Establish the extension boundary for security and IEs. Until their codec work lands,
  reject unsupported combinations explicitly rather than consuming them as ordinary data.

**Exit:** independent golden bytes for every supported addressing/PAN-presence combination,
high EUI-64 bits, sequence boundaries and ACK format; truncated/reserved inputs terminate safely.
Typed-to-bytes and bytes-to-typed checks agree on consumed length and field presence.

Codec work is implemented behind the new complete `Ieee802154Frame` representation and its
registered serializer. `Ieee802154FrameWire_1.test` and `Ieee802154Fcs_1.test` pass in debug mode;
the [codec contract](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/frame-codec.md) records source
locators and the independent CRC oracle. Production byte injection and observations now live
in `Ieee802154ReceiveSecurity.test`, `Ieee802154DataAck.test` and their reviewer companions,
under `tests/protocol/ieee802154/`. These actual artifact names supersede the planned
`frame-wire.test`/`frame-wire.md` names; no empty placeholder is required.

### Step 3 — Complete MPDU, FCS and capture tooling

Dependencies: step 2.

- [x] Add the selected PHY's MAC FCS trailer and CRC with independent known-answer vectors;
  derive total MPDU length from actual chunks. Reject oversized PSDUs before transmission.
- [x] Remove `networkProtocol` from the new wire format; finish opaque/configured payload
  handling in the MAC and dissector. Add printer/filter visibility and capture registration.
- [x] Exercise both with-FCS and no-FCS capture input and output through production tooling.
  Define absent, computed, invalid and simulation-declared FCS handling. Never invent captured
  bytes to make a truncated packet round-trip; preserve supplied invalid FCS in forensic decoding.
- [x] Restore supported captures to the serializer suite, recording provenance, hashes, frame
  selection and external decoder version. Compare MPDU bytes, separately from PCAP timestamps
  and container metadata. Use independent decoded fields as well as round trips.

**Exit:** supported MPDUs round-trip byte-for-byte; generated captures decode independently;
packet length, FCS coverage, PHY PSDU length and observed bytes agree. No sender metadata is
required to decode captured payload boundaries. Core capture defects are isolated if encountered.

The direct production capture fixture `Ieee802154Capture_1.test` passes complete and truncated
records in both FCS modes. The user-approved
[bounded reader repair](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/capture-reader-repair.md)
preserves actual captured bytes and checks stripped-header offsets. The admitted
`serializer_ieee802154.test` passes four frames from two independently generated captures,
also decoded with Scapy 2.8.0. Existing radiotap and receiver-copy module checks pass. The
generic serializer corpus has two unchanged Wi-Fi byte mismatches; the
[results](../../doc/project/evidence/model/ieee802154/results.md) record them without refreshing
expectations. Final request-boundary and migration gates remain in step 10.

### Step 4 — O-QPSK PHY services before MAC timing

Dependencies: steps 1 and 3. A read-only feasibility investigation or isolated fixture may run
once 1c is defined, before complete MPDU integration.

- [x] Establish a CCA/ED feasibility gate: identify the existing medium/radio energy observation
  API and demonstrate energy detection without a decodable frame. If insufficient, define and
  validate a bounded PHY-local listening/energy contract before promising Mode 1 behavior.
  Treat insufficiency as an investigated result, not an assumption about the current medium.
- [x] Implement the PHY provider over the existing narrowband radio infrastructure: timed CCA,
  ED, state changes, channel/page selection, PD-DATA completion and receive metadata/LQI.
- [x] Derive symbol timing, SHR/PHR/PSDU duration, turnaround and size checks from the selected
  O-QPSK mode. Account for each PHY/MAC overhead exactly once.
- [x] Implement and advertise CCA Mode 1, observation duration and thresholds;
  unsupported modes fail explicitly. Test energy/interference without a decodable frame, not
  just the radio's current reception-state enum.
- [x] Define ED/LQI mapping and saturation with documented model limits. Validate timing and
  channel selection independently of the existing generic BER/PER formula.

**Exit:** module tests cover successful/refused/interrupted operations, channel switching,
CCA threshold/time boundaries, ED bounds and LQI delivery. A PHY timing oracle matches
generated transmissions. PER claims await the separate validation gate in step 10.

The [scalar energy observation design](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/energy-observation.md)
and production helper have a passing feasibility module fixture with real scalar radios/medium:
background energy and two undecodable BPSK transmissions contribute to peak/mean power without
packet delivery. Production timed PHY/PIB/lifecycle fixtures also pass; ED/LQI saturation,
remaining energy boundaries and external reception/error validation remain open.

### Step 5 — Unslotted access and transmission lifecycle

Dependencies: steps 3–4.

- [x] Implement specification-derived NB/BE initialization, random backoff range, busy-CCA
  updates, exponent cap and access-failure boundary, using a named simulation RNG stream.
- [x] Wire access success to real PHY transmission and completion. Separate access attempts
  from frame retransmission attempts; handle zero backoff and configured limit boundaries.
- [x] Define queue admission, refusal, cancellation and one terminal MCPS result per request.
  Handle stop/crash/reset during backoff, CCA and transmission; cancel owned timers.

**Exit:** deterministic production-path traces match the clause-derived state transitions and
symbol times for idle, persistently busy and busy-then-idle channels. No transmission follows
an access failure, and no request is completed twice or stranded.

### Step 6 — Receive filtering, ACKs, retries and DSN handling

Dependencies: step 5.

- [x] Apply length/FCS, version, frame type, PAN/address and receive-mode checks in the required
  order from step 0's decision table, including promiscuous behavior. Test ACK eligibility
  independently of upper delivery and optional suppression.
- [x] Implement unsecured-profile security-policy handling through the applicable branches
  (9.2.2 and 9.2.5) with `macSecurityEnabled=false`; reject unsupported configuration and
  secured traffic with defined outcomes. This does not implement cryptographic security.
- [x] Generate real immediate ACKs with specified timing and transaction matching. Cover
  ACK-disabled traffic and broadcasts; a packet name never determines frame type.
- [x] Implement ACK timeout/retry exhaustion and interframe timing. Keep DSN stable across a
  retransmission; allocate new DSNs device-wide with the specified width and ownership (6.6.1).
- [x] Deliver DSN and source identity through the data indication. Resolve duplicate policy from
  step 0's evidence. Unless an applicable requirement is found, suppression is disabled by
  default and remains outside the conformance claim. If retained as an opt-in policy, document
  its cache key, lifetime, reset behavior and ambiguity limits. It must not prevent required ACKs.
  Test wraparound, peer restart, interleaved destinations, late ACKs and unexpected ACK sequences.

**Exit / M1 candidate:** two real interfaces exchange addressed data through the real radio;
loss injection proves retries and the declared indication/suppression policy; a serialized 254→255→0→1 sequence remains
usable. Run the same cases on typed and byte-backed frames. Complete step 10 before releasing M1.

### Step 7 — Command frames and indirect transaction engine

Dependencies: step 6 and closure of the M2 applicability extension from step 0.

Current execution: selected legacy command and beacon codecs pass independent vector/unit
checks. The shared immutable pending store is connected to production indirect DATA, POLL,
PURGE and lifecycle barriers, with 62 passing indirect typed/raw protocol runs plus
24 broadcast and 76 receiver-enable runs. See
[indirect transactions](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/indirect-transactions.md),
[polling](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/polling.md) and the
[managed applicability extension](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/managed-applicability.md).
Association-response integration, broadcast release, active-extraction interruption
and scheduled receiver control now have focused production evidence in the
[results ledger](../../doc/project/evidence/model/ieee802154/results.md).

- [x] Add each selected command payload with codec and independent vectors; include Data
  Request and association/disassociation commands required by M2.
- [x] Implement pending transactions keyed by native device identity, expiration, bounded
  queue capacity, purge and ownership. Association responses and data use the same store.
- [x] Implement polling, Frame Pending behavior, empty response/no-data behavior and the
  specified data-release timing/access rules. Implement receiver-enable/sleep coordination.
- [x] Keep a failed indirect frame pending for the next Data Request, retaining its DSN;
  do not route it through the direct-transmission retry loop (6.6.3.4).

**Exit:** poll with zero/one/multiple pending transactions; expiration at a poll boundary;
lost poll ACK/data ACK; queue refusal; two sleeping devices. No duplicate dequeue, delivery
to the wrong device, or success confirmation before its defining event.

### Step 8 — PAN start, scanning and recovery primitives

Dependencies: steps 4, 6 and 7.

- [x] Implement non-beacon coordinator start and selected beacon/command generation for scan
  responses. A scan-response beacon does not imply periodic beacon support.
- [x] Add ED, active and passive scan contracts, channel iteration, durations, PAN descriptor
  collection/deduplication, cancellation and restoration of radio/PIB state.
- [x] Add the selected orphan scan/realignment and PAN conflict procedures with their own
  command fields, timeout outcomes and scope. Keep beacon-loss synchronization for B1.
- [x] Keep PAN selection policy outside the MAC; expose sufficient descriptors/status to a
  small management client used by examples and tests.

**Exit:** discovery on multiple configured channels, no coordinator, duplicate responses,
partial/invalid channel sets, scan interruption, and state restoration all have direct evidence.

### Step 9 — Association, disassociation and managed operation

Dependencies: steps 7–8.

- [x] Implement reset/scan/select/associate orchestration and device/coordinator procedure
  states. Higher-layer policy decides admission and allocates unique permitted addresses.
- [x] Deliver Association Response through the pending store and device polling procedure.
  Apply the specified response wait and failure semantics; do not infer success from request ACK.
- [x] Support associated devices without an allocated short address, rejection, timeout,
  repeated association and stale responses; keep identities distinct across PANs.
- [x] Implement both selected disassociation directions and cleanup of pending transactions,
  duplicate state and timers. Test reassociation and recovery after interruption.

**Exit / M2 candidate:** an initially unassociated device discovers a PAN, joins, exchanges
data, sleeps, polls, leaves and rejoins. Failure cases yield the specified status and consistent
PIB/state. Complete step 10 again for the expanded M2 claim.

### Step 10 — Validate, migrate and publish each milestone

Dependencies: step 6 for M1; step 9 for M2. Prepare fixtures and evidence throughout the series.

- [x] Close every selected obligation in the coverage ledger with test identity, exact command,
  result, revision and artifacts; leave unimplemented obligations visible.
- [x] Add external validation for timing and the chosen PHY reception/error abstraction.
  Specify modulation, channel/noise assumptions, frame length, reference source and tolerance
  before comparing results. Correct or narrow unsupported PER claims.
- [x] Inventory old consumers and migrate examples deliberately. At M1 cutover, replace the
  default narrowband path only after migration tests pass; document removed/changed parameters,
  addresses, MTU and wire incompatibility. Retire the temporary old narrowband path on a stated
  schedule: retain an explicitly named legacy narrowband type for the first release shipping
  the replacement as default, deprecate it in that release, and remove it no earlier than the
  following release after inventoried consumers are migrated and regression gates pass.
  Set concrete type names in 1e and release identifiers at cutover. Do not add silent address,
  wire-format or parameter conversions; document any explicit adapter's limits. This retirement
  policy excludes the separately retained legacy UWB composition. Do not reinterpret old
  captures as standard frames.
- [x] Update `.oppfeatures`, examples, user/migration documentation and `WHATSNEW`; distinguish
  packet-level abstraction from waveform fidelity and M1/M2 from whole-standard support.
- [x] Run relevant regression gates and feature-disabled builds. Attribute every changed
  fingerprint to the behavior that moved it; retain the known UWB baseline or investigate changes.

**Exit:** reproducible evidence bundle, migration instructions and a profile-specific support
statement. Missing checks, setup failures and unsupported obligations remain explicit blockers
to the corresponding claim, even if the broad regression suite passes.

## 6. Later feature series and their gates

These are scoped follow-on packages, not promises hidden inside M1/M2. Before starting one,
repeat step 0 for its requirements and expand it into PR-sized tasks.

The approved [B1–B3 plan](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/plan/pending/802154-superframe-gts-plan.md) and [E1 plan](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/plan/pending/802154-enhanced-frames-plan.md) are implemented and verified for their selected profiles. The [execution record](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/followup-results.md#e1-99-publication-closure) distinguishes protocol passes, source review and mechanical-gate limits. Scheduled enhanced operation reuses the B1–B3 epoch and grant owners. S1, T1, D1, P1 and A1 below remain separate, unexecuted packages.

| Package | Prerequisites | Work and acceptance gate |
| --- | --- | --- |
| [B1: beacon scheduler](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/plan/pending/802154-superframe-gts-plan.md) | M2; B2 CAP backend for complete managed operation | Periodic beacons, BO/SO, active/inactive intervals, synchronization/loss, linked coordinator timing and realignment; direct radio/clock and lifecycle checks |
| [B2: slotted access](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/plan/pending/802154-superframe-gts-plan.md) | B1 timing/tracking core | Slotted NB/BE/CW, BLE, CAP fit and distinct pause/resample rules; actual managed exchanges and boundary/lifecycle checks |
| [B3: GTS/CFP](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/plan/pending/802154-superframe-gts-plan.md) | Complete B1/B2 | Request/allocation, both CFP data directions, descriptor persistence, deallocation/repacking and inactivity expiration; real beacon/command/DATA/ACK evidence |
| [E1: modern frame machinery](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/plan/pending/802154-enhanced-frames-plan.md) | M1/M2 for core; B1–B3 for scheduled integration | Version-2 PAN/sequence presence, Header/Payload/Nested IEs, enhanced ACK and beacon exchanges; separate structural, operational and scheduled gates |
| S1: base MAC security | M1; E1 for enhanced-frame claims | Auxiliary header, key/device/security-level state, AES-based operations, authenticated fields, nonce/counter rules, replay protection and status handling; independent crypto vectors plus secured exchanges, tampering, counter exhaustion and reset/persistence tests |
| T1: TSCH | E1, PHY switching/timing, selected S1 joining policy | ASN, slotframes/links, templates, hopping, shared/dedicated links, enhanced beacons, synchronization/drift and keepalive; fixed schedule first, pluggable scheduling policy later; test slot/channel oracles and synchronization loss |
| D1: DSME | E1 and relevant beacon/security services | Separate scheduler, multi-superframes, allocation commands and conflict recovery; test simultaneous allocation, expiration and channel use |
| P1: additional PHYs/UWB | Stable PHY service contract and selected common MAC | One PHY/mode per profile; independent PPDU/timing/reception validation. Modern ranging adds timestamps, ranging exchanges and a ranging-error model; legacy UWB constants are not proof |
| A1: amendments | Applicable base capability and locally retrieved amendment | Pin edition and override map before work; Ascon and privacy remain independent additions with their own evidence |

For S1, specify the incoming/outgoing processing order from clause 9, including when authentication
precedes state mutation/delivery, when ACKs are permitted, how encrypted bytes are represented,
and how FCS covers the transmitted frame. The initial plan's short security pipeline is not a
complete procedure. A simulated “security enabled” flag cannot establish cryptographic support.

## 7. Minimum test matrix

Use [test categories](../../doc/project/design/test-anatomy.md) and the
[protocol authoring guide](../../tests/protocol/lib/AUTHORING.md). Proposed case names below
are work items, not existing tests or runnable commands. Allocate actual paths in each PR.

| Case family | Category | Required observable evidence |
| --- | --- | --- |
| `Ieee802154Address`, `Ieee802154FrameWire` | Unit/serializer | Native identity; independent octets; supported presence matrix; malformed/truncated inputs; exact lengths |
| `Ieee802154FcsCapture` | Serializer plus production recorder/replay fixture | CRC oracle; link types with/without FCS; unchanged supported MPDU bytes; external decoded fields |
| `Ieee802154PhyServices` | Module | Timed CCA/ED/state results, channel changes, LQI, cancellation and refusal |
| `Ieee802154UnslottedAccess` | Module and protocol | Backoff/CCA trace, attempt limits, RNG reproducibility and actual transmit/no-transmit outcome |
| `Ieee802154DataAck` | Protocol | Fields/times, ACK loss, retry exhaustion, broadcast, wrong/late ACK, filtering, declared delivery policy and one terminal request result |
| `Ieee802154SequenceWrap` | Protocol with byte boundary | More than 256 transfers; retries near wrap; peer restart and two peers without false duplicate suppression |
| `Ieee802154Lifecycle` | Module and protocol | Reset/stop/crash at each asynchronous state; no leaked packet, stale timer or double completion |
| `Ieee802154Indirect`, `Ieee802154Association` | Protocol | Poll/response/ACK ordering, expiration, admission failure, no-short-address join and leave/rejoin |
| `Ieee802154Scan` | Protocol | Selected-channel coverage, durations, descriptors, cancellation and restored configuration |
| O-QPSK timing/PER | Validation | Independent duration calculations and a documented external reception reference with uncertainty |
| Feature isolation and migrated examples | Features/networks plus focused fingerprints | Build without optional siblings; examples initialize; explained trajectory changes |

Use fixed seeds for deterministic exchange checks. For contention robustness use an explicitly
bounded campaign, initially 20 seeds × 2 and 5 contenders, and require the same safety/termination
invariants in every run; this is not a throughput/fairness claim. For PER validation select SNRs
and packet lengths from the reference, use independent repetitions and predeclare sample size,
confidence interval and acceptance tolerance in step 10. Do not rerun failing seeds until green.

All protocol checks enter production gates/services. Helper tests alone cannot establish that the
MAC calls the helper with correct units, addresses or timing. Capture round trips alone cannot
detect a symmetric encoder/decoder mistake. Cross-model comparisons can supplement the standard
and independent vectors, but another simulator's behavior is not a normative oracle.

## 8. Per-PR verification and completion record

### Proposed PR and commit series

Publication mapping, 2026-11-17: inspected branch `feat/inet-ieee802154-standard` at `ec1ee66b1b`, against local `master` at `91219b0fa7`. The implemented profiles and their recorded results remain complete. This section proposes how to present that implementation as twelve protocol PRs; reconstruction, intermediate-tree builds, new parent comparisons and PR publication are NOT_RUN. The current request authorizes updating the plans, not rewriting history or opening PRs. The unrelated local `AGENTS.md` edit stays outside this work.

Keep the existing implementation changes wherever their boundaries fit the table. “Retain” means preserve the decision, source change and tests, while revising the message and documentation only as needed for the new parent. Replaying a retained change onto a different parent gives it a new SHA. “Split” means construct new commits from the named source commit. Documentation refinements are consolidated into their owning capability rather than replayed as an editing history. Source SHA values below identify the old commits, not promised future SHA values. The reconstruction must preserve the final implementation tree and baseline contents; any necessary semantic change requires its own explanation and verification before proceeding.

The review order is a linear stack. PR01 starts on the resolved shared-work base below; each subsequent numbered PR targets the preceding PR's branch until that predecessor lands. PR02 also follows the separate capture repair when its native capture checks require it. This order is a publication choice, not a claim that enhanced codecs intrinsically require GTS. Existing B/E unit IDs remain stable and their detailed test designs stay in the linked follow-on plans.

| PR | Review topic and capability | Commits in review order | Prerequisite |
| --- | --- | --- | --- |
| PR01 | Native identities, service ownership, PIB and legacy wire representation | N01, N02, N03, N04 | Resolved shared-work base |
| PR02 | Timed native O-QPSK PHY services | N05 | PR01; capture repair before dependent capture checks |
| PR03 | Selectable nonbeacon DATA/ACK operation | N06 | PR02 |
| PR04 | Nonbeacon PAN management and indirect delivery | N07 | PR03 |
| PR05 | Managed example followed by public default migration | N08, N09 | PR04 |
| PR06 | Conventional beacon scheduling, acquisition and synchronization | B-00, B-01, B1-02, B1-03 | PR05 |
| PR07 | Slotted CAP access, linked coordinator timing and managed CAP exchanges | B2-01, B2-02, B1-04, B2-03 | PR06 |
| PR08 | GTS allocation, CFP exchanges, removal, expiry and observations | B3-01, B3-02, B3-03, B3-04, B3-05, B3-06 | PR07 |
| PR09 | Version-2 layouts and Header/Payload/Nested IE framing | E1-00, E1-01, E1-02, E1-03 | PR08 in this stack |
| PR10 | Operational enhanced DATA followed by enhanced ACK | E1-04, E1-05 | PR09 |
| PR11 | Enhanced beacon request/response and discovery | E1-06 | PR10 |
| PR12 | Scheduled enhanced beacons and superframe/GTS integration | E1-07 | PR11 and complete PR06–PR08 |

#### Native commit mapping

Each row depends on the previous row in publication order, with the separate capture repair between N04 and N05. Keep separate decisions as separate commits even when they share a PR. Paths in the verification column refer to existing fixtures, not tests to be invented after reconstruction. Unit fixtures are under `tests/unit/`, module fixtures under `tests/module/`, and protocol fixtures and their reviewer documents under `tests/protocol/ieee802154/`.

| ID / PR | Proposed subject | Existing source / treatment | Decision, surface and direct verification |
| --- | --- | --- | --- |
| N01 / PR01 | `ieee802154: add: native address value type` | `b589ce2520`, retain | Native none/short/EUI-64 values without truncating identity. `Ieee802154Address_1.test`. |
| N02 / PR01 | `ieee802154: add: bounded MAC service contracts` | `0bfda7105a`, retain | Admission, ownership, request identity, completion and cancellation contracts. `Ieee802154MacServiceContract_1.test`. |
| N03 / PR01 | `ieee802154: add: selected MAC PIB store` | `0336bad8b2`, retain | Authoritative validated attribute store. `Ieee802154MacPib_1.test`, using expectations applicable at this parent. |
| N04 / PR01 | `ieee802154: add: complete native legacy-frame codecs` | `b45197b916`, retain | Wire chunks, codecs, serializer/dissector/printer and independent vectors. FrameWire, Fcs, Commands and Beacons units and `tests/serializer/serializer_ieee802154.test`. |
| N05 / PR02 | `ieee802154: add: timed native O-QPSK PHY services` | `b8ed8930b6`, retain | Paired PHY roles, CCA/ED, state/TX timing, measurements and PPDU visibility. PhyServiceContract, PhyServices, Measurements and ExternalPhy module fixtures. |
| N06 / PR03 | `ieee802154: add: native nonbeacon DATA and ACK services` | Core of `2e4708066a`, split | StandardMac transaction/receive owner, unslotted access, native interface data, gate adapter, direct DATA/ACK, filtering, retries, DSN and core lifecycle. Exact protocol and document allocation below. |
| N07 / PR04 | `ieee802154: add: native nonbeacon PAN management` | Management remainder of `2e4708066a`, split | Management request values/handlers, pending store, start/scan/poll/join/leave/recovery and receiver schedules, extending the same transaction owner. Exact protocol and document allocation below. |
| N08 / PR05 | `ieee802154: add: managed PAN example policy` | `b1eb3d10ed`, retain | Application-owned PAN choice/address allocation and managed cycles. Explicitly select StandardInterface before public cutover. Typed/raw example runs and scalar checker. |
| N09 / PR05 | `ieee802154: change: select native narrowband interface` | `dffdddb416`, retain | Public default, explicit deprecated legacy type and consumer/migration updates. Composition, NativeGate, EnergyObservation and LegacyUwb module fixtures, plus stored/fake-GUI trajectory controls. |

#### Split of the nonbeacon MAC commit

N06 introduces a usable direct-data provider on explicitly selected native modules. For a configured pair, DATA goes through unslotted CCA and RF transmission, the receiver filters and acknowledges it, and the sender reports exactly one terminal result, including retry exhaustion. GET/SET/RESET and lifecycle handling required by this path belong here. Management mechanisms not yet implemented must have a coherent explicit unsupported admission/result, following the existing service contract; they must not appear operational. Do not add a second transaction controller, temporary queue or duplicate lifecycle state just to divide the history.

N07 extends that same owner with management. For example, association is followed by polling an indirect response before the child commits its assigned identity. Its pending transactions, timeout/cancellation behavior and scan channel restoration belong with those procedures. N07 updates the shared cleanup and receive paths for the operations it introduces. Both commits must leave all exposed service methods coherent and all owned packets/timers retired correctly.

Retain and, where necessary, adapt the following existing fixtures and write/update their reviewer documents in the responsible commit. Shared fixture extensions must be staged with the capability they exercise; do not copy the final fixture wholesale into an earlier tree that lacks its management APIs. For each named test, retain its existing scenario and assertions and explain the reconstructed immediate parent's missing capability or existing control result. Execute the candidate; use a source-based parent comparison unless the accelerated contract below calls for a runtime probe. An absent API or failed build is not a runtime protocol failure. Existing scenario descriptions below are incorporated through their exact documents.

| Owner | Existing protocol tests, each prefixed `Ieee802154` and suffixed `.test` | Reviewer documents to retain/update in that commit |
| --- | --- | --- |
| N06 | Admission, UnslottedAccess, DataAck, Broadcast, Contention, InterframeSpacing, SequenceWrap, ReceiveSecurity, Lifecycle, PhyLifecycle, PhyPib | `admission.md`, `unslotted-access.md`, `data-ack.md`, `broadcast.md`, `contention.md`, `interframe-spacing.md`, `sequence-wrap.md`, `receive-security.md`, `lifecycle.md`, `phy-lifecycle.md`, `phy-pib.md` |
| N07 | Indirect, Association, Reassociation, Orphan, PanConflict, ReceiverEnable | `indirect.md`, `association.md`, `reassociation.md`, `orphan.md`, `pan-conflict.md`, `receiver-enable.md` |

For N06, preserve the NativeInterface and UnslottedAccess module checks and the core portions of MacPib/PendingTransactions-independent coverage. For N07, include the PendingTransactions unit, Start/Discovery/PassiveScan module fixtures, and the management-dependent extensions to earlier fixtures. Common test support in `fixtures/ReceiveCases.h` lands with its first actual consumer; later extensions accompany their consumer. These tests establish the selected unsecured O-QPSK profile, not successful security transforms or scheduled operation.

Create two commit-level companions, `tests/protocol/ieee802154/nonbeacon-data-services.md` for N06 and `tests/protocol/ieee802154/nonbeacon-pan-management.md` for N07, to associate these multi-scenario allocations with the new stable IDs. Link the scenario documents rather than repeating their full explanations. Each companion must explain its own parent comparison, fixture adaptations, production path, expected observations, execution status and limits, with exchange/timing diagrams where useful. These two files are proposed and do not yet exist.

The split is proposed, not proven buildable. Before accepting it, build each intermediate tree and execute its selected tests, including cancellation/reset/stop and external PHY availability controls. If separating management would require artificial temporary architecture or change the final source tree, stop reconstruction of N06/N07 and revise this mapping to one combined PR03/PR04 unit with a documented size rationale. Do not silently combine them or report the twelve-PR structure as validated.

#### Shared work and documentation allocation

| Existing source commits | Destination and treatment |
| --- | --- |
| `a818404126`, `45f4bb862e`, `da1012b549` | Separate shared/Wi-Fi series, outside the twelve protocol PRs. Preserve their existing order and tests. Resolve whether they land first or which changes are genuinely required before choosing the protocol stack's base. No removal or independent buildability is assumed from this mapping. |
| `a14657683d` | Separate capture PR, C01: `packet: fix: preserve captured length in truncated records`, after N04 and before N05 in the initial stack. Retain its generic repair and Capture unit. Keep the native fixture dependency explicit. Publishing it before PR01 would require a separate fixture adaptation and verification. This plan does not renew permission for edits to protected source. |
| `2dd3ca3535`, `272a453f3b`, `5766ad4fac`, `53b4849913`, `8f925e3f43`, `0e68fc8b26`, `14a358bc96`, `6a8bbd320a` | Consolidate the survey, bounded source audit, implementation plan and publication mapping into PR01's preparation material, retaining source provenance and limitations. Stage later-capability sections with their owning PR if an early copy would claim nonexistent support. No separate PR for each refinement. |
| `efced47d1f` | Distribute native coverage/results, migration and completion records across PR01–PR05 according to the capability they establish. Final native summary belongs in PR05; earlier commits carry their own evidence and reviewer documents. |
| `8406180907` | Separate its two follow-on plans: conventional plan in PR06 preparation and enhanced plan in PR09 preparation. Keep roadmap links valid at each parent. |
| `035c89dc62`, `fb0dff1f35` | Retain as B-00 in PR06 and E1-00 in PR09 respectively, with their source/profile/check mappings. |
| `84f7242591` | Distribute B-99 evidence/configuration/documentation among PR06–PR08; final conventional summary in PR08. No missing tests or runtime code may be deferred to that summary. |
| `ec1ee66b1b` | Distribute E1-99 evidence/documentation among PR09–PR12; final combined summary in PR12. Preserve recorded failures and unsupported-profile limits. |

Documentation distribution changes retained commits' metadata/documentation where needed; it does not authorize altering the recorded historical outcomes to imply that the new parents were tested. Within each PR, preparation precedes its consumers and evidence updates accompany the responsible implementation commit. If a final summary remains useful, fold it into the last owning commit's documentation rather than introducing unspecified future commits. The detailed conventional and enhanced mappings below enumerate every retained feature commit.

#### Verification of the reconstructed series

Apply [PR-SERIES-BUILDS](../../doc/project/rule/pull-request.md#pr-series-builds), [PR-SERIES-PLAN](../../doc/project/rule/pull-request.md#pr-series-plan) and [TR-PLAN-PROTOCOL](../../doc/project/rule/testing.md#tr-plan-protocol) to each future commit. The existing per-unit designs supply the scenario, production path and coverage limits for retained commits. Update each commit message and its reviewer documents against its actual new parent, and validate their diagrams. Stage test extensions only when their capability exists.

Before each reconstructed commit's tests, build a matching debug library; before publication compile release and run the applicable scoped/release checks. Use the current runners' documented exact-file selectors, with selected and executed counts recorded. Run each row's direct checks and existing compatibility controls; for N06/N07 derive the exact selectors from the allocation above and confirm they select every named fixture. At PR05 run the native migration/examples and legacy trajectory controls; at PR08 run the combined conventional campaign; at PR12 run the combined conventional/enhanced campaign, feature-disabled and Clock-disabled checks. No baseline updates are planned. Record exact old/new parent identities and any fixture adaptation in the PR evidence.

Existing results in [results.md](../../doc/project/evidence/model/ieee802154/results.md) and [followup-results.md](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/followup-results.md#e1-99-publication-closure) establish their recorded revisions only. Reconstructed-parent comparisons, intermediate builds and final tree-equivalence checks remain NOT_RUN. Preserve the failed C++ warning gate and repository-wide gate dispositions as failures with their evidence; neither historical focused passes nor an unchanged final tree establishes passing intermediate commits.

#### Accelerated execution contract

On 2026-11-17 the user requested speeding up this process “in the same way as we did with the 6LoWPAN case”. Adopt the scheduling used by that workflow: automated incremental debug builds and directly assigned tests for each behavior commit, source-based parent comparisons, and combined acceptance at PR boundaries. The inspected 6LoWPAN executor and records are under `/home/miguel/workspace/.codex-artifacts/lowpan-publication-20261117/` (`execute_series.py`, `c14-automated/record.json` and `pr08-combined/record.json`); these are local workflow provenance, not portable 802.15.4 test evidence. This instruction approves the accelerated verification schedule, not history reconstruction, pushing or PR creation by itself. All 802.15.4 reconstruction results remain NOT_RUN.

This contract controls the verification scheduling in steps 3–6 below. Keep the full cleanup workflow for the substantive N06/N07 split; the simple-cleanup admission rules do not establish that split as mechanically safe. Reduce repeated work as follows:

| Point | Required work | Work deferred or avoided |
| --- | --- | --- |
| Preflight, once | Pin inputs, assign hunks and exact tests, check the planned ordered source projection and prepare machine-readable commit/PR manifests. Reuse existing standards decisions and scenario documents. | No new full standards audit or repeated reconstruction of the same inventory. |
| Every behavior commit | Incremental matching debug build, directly assigned unit/module/protocol tests and narrow already-introduced controls for changed owners. Record commands, source/library/fixture hashes, exits and counts. Run cheap commit/classification/release-note gates. | Release compilation and combined PR selection occur at the PR tip. Full repository gates and full C++ analysis occur at final acceptance. |
| Pure documentation or metadata commit | Whitespace/link/series checks and proof that executable inputs are unchanged from a validated safe point. Record the reused evidence identity explicitly. | No repeated simulation or compilation when code, generated inputs, features, fixtures and configurations are identical. |
| Immediate-parent comparison | Explain the mechanism from the actual parent diff; record `NOT_RUN_SOURCE_BASED` when no runtime probe ran. Execute a compatible parent probe only for an ambiguous cause, unexpected candidate result, or a claim that specifically requires observed before/after evidence. | No automatic parent worktree build and failing-parent campaign for every test. Historical parent results keep their historical SHA. |
| Each PR tip, including C01 | Debug build already current, incremental release compilation, union of that PR's assigned tests plus changed-owner controls, and mechanical series/scoped checks against its actual base. | Reuse exact matching per-commit results for cases whose complete executable inputs are unchanged, with their identities recorded. No full earlier-PR campaign at every tip. |
| Feature/Clock matrix | Run when feature dependencies, composition or clock ownership change, and once on the final tree. Restore exact feature bytes and build state after each profile. | No repeated isolated feature/Clock builds for an unrelated codec or documentation change. |
| Final acceptance | Final tree/source/baseline proof, full selected combined campaign, required feature/Clock and example/trajectory controls, both builds, full gates and canonical scoped C++ analysis. | Do not repeat the same final campaign merely because later changes affect documentation only. No redundant middle-commit spot checks if exact trees already passed and were not rewritten. |

Use a single sequential executor and one retained incremental build workspace. The executor stages only manifest-owned patches, verifies each candidate's expected source projection, runs its assigned checks, writes a concise result/ledger row, and continues automatically after an expected clean pass. Use `make -j$(nproc)` for builds and the runners' supported concurrency for independent cases; serialize builders, feature changes and runners that share generated/support outputs. Preserve evidence before any runner replaces its work directory. Generate review refs, old/new SHA maps and description drafts from the same manifest rather than rebuilding them manually at each boundary.

Stop only on an unexpected failure, unassigned delta, changed approved boundary, protected-source issue or unresolved verification requirement. Diagnose that specific commit, preserve the previous safe point and rerun only checks invalidated by the repair. An unexplained result does not become acceptable because execution is automated. Keep baseline bytes, final implementation behavior, per-commit debug evidence and original-worktree preservation requirements. Failed/deferred gates must remain visibly FAIL/NOT_RUN rather than becoming a blanket PASS. At PR05, PR08 and PR12 retain the milestone acceptance already assigned above, with exact-input evidence reuse rather than duplicate runs.

### Cleanup and PR creation procedure

This is the execution procedure for the mapping above, not a record of actions already performed. Its steps are NOT_RUN. The current request covers adding this procedure only. A later instruction to execute cleanup must identify its authorized scope; creating or publishing PRs additionally requires an explicit request. Reuse authorization already given for the same action instead of requesting it again. Follow the active checkout's [project map](../../doc/project/README.md), [cleanup workflow](../../.agents/skills/inet-branch-cleanup/SKILL.md), its [state-log format](../../.agents/skills/inet-branch-cleanup/references/state-log.md) and [verification contract](../../.agents/skills/inet-branch-cleanup/references/opp-repl-contract.md), and the [PR authoring workflow](../../.agents/skills/inet-pull-request-authoring/SKILL.md). These routes supply the current mechanics and policy; the steps below select the branch-specific inputs and order.

#### 1. Pin inputs and preserve the original work

Record the actual topic branch, full topic SHA, local/remote master SHAs, merge base, tracking branch, remote URL and working-tree status in the execution log. The inspected inputs above are historical starting values, not moving references to resolve silently during execution. Preserve the original topic branch without resetting, rebasing or repointing it. Leave the unrelated `AGENTS.md` edit in its original worktree. Save the complete base-to-topic diff, per-commit patches/messages and source/baseline identities as evidence before reconstruction.

Resolve the shared/Wi-Fi base before building the protocol series. If those changes are already landed, record their exact equivalent base and compare the selected trees. Otherwise retain the three original prerequisite commits in their existing order and treat their tip as the protocol stack's shared-work base; arrange their separate review rather than hiding them in PR01. Keep the original fixed base for cleanup. Absorbing new upstream commits is a separate rebase task and must not be mixed into this reconstruction.

The approved plan edits and the two proposed N06/N07 companions are intentional documentation additions beyond the original topic. Before reconstruction, prepare a separate input snapshot containing the original final implementation plus only those reviewed documentation additions. Record every added/modified documentation path and the snapshot SHA, and prove its source, tests, configurations and baseline files identical to the original topic. This snapshot is the cleanup `topic` oracle; the original branch remains untouched. Do not use the snapshot to introduce new protocol behavior or silently incorporate unrelated working-tree changes. If the documentation cannot yet be finalized, finish that preparation before claiming an exact cleanup target.

#### 2. Write the resumable execution record and approve the material groups

Create `ai-logs/executions/<date>_ieee802154-pr-series.analysis.md` and `ai-logs/executions/<date>_ieee802154-pr-series.md`. Record the pinned original and snapshot SHAs, fixed base, proposed `cleanup/ieee802154-pr-series` branch, paths to raw artifacts, authorization scope, and the exact build/test environment. Use the state-log sections linked above so another session can identify the latest safe point and next action without this conversation.

Classify the complete diff into numbered groups at hunk or sub-hunk granularity. Map every source, test, configuration, baseline and documentation slice to N01–N09, C01, the retained B/E IDs, or the separate shared series. Add explicit preparation commit IDs/subjects if documentation cannot coherently accompany the first consuming commit; update the mapping before construction. Record each group's dependencies, predicted behavior effect and tests. Resolve any gaps in the current broad documentation allocation into exact path/hunk assignments. Obtain the cleanup workflow's group/commit-plan approval unless the execution instruction already covers that same concrete grouping and order. The twelve-PR table alone does not prove hunk completeness.

#### 3. Establish the test contract and reusable cleanup worktree

Discover the installed `opp_repl` entrypoints and category-runner options through the [verification workflow](../../.agents/skills/inet-opp-repl/SKILL.md). Record the exact scoped commands, configurations, runs/seeds, modes, fingerprint ingredients and baseline/dependency-store revisions. Use the direct tests allocated above and in the B/E plans, with the recorded Power100s/Coexistence5s trajectory controls where applicable. Assign a directly related executed oracle to each executable change, including refactors; pure documentation/metadata uses the accelerated contract's unchanged-input proof and identified prior evidence. A zero-case selection or missing tool is NOT_RUN, not a runtime safe point. Do not invent passing coverage by selecting unrelated configurations.

Create the new cleanup branch at the pinned fixed base in a separate, unused sibling worktree. Retain one compatible incremental build workspace through the reconstruction. Inspect the environment/build instructions there before invoking tools; do not reuse a library built from a different source state as evidence. Where the shared prerequisites are retained unchanged, record their SHAs and verification scope before starting N01. No original worktree switch, stash or cleanup is needed.

#### 4. Reconstruct and validate one commit at a time

Apply only the current group's assigned material. Use whole existing commits when their content fits, selected hunks or hand-authored intermediate files when it does not, and `apply_patch` for hand-authored edits. Stage explicit paths/hunks. Include that commit's tests, reviewer documents, release notes and evidence links. For N06/N07 follow the split design above, including compatible unsupported management behavior at N06 and management-specific cleanup at N07. Do not copy final management tests into a tree lacking their APIs.

Before accepting each output commit, apply the accelerated contract: incrementally build the required matching debug artifacts, run its scoped direct tests and cleanup oracle, inspect its diff/message, and compare the selected behavior signal with the previous safe point. Pure documentation uses the unchanged-executable-input proof above. Features may change only their predicted scope. Record whether each parent comparison was executed or source-based and any compatible fixture adaptation. Commit and promote a safe point only when its required evidence is complete; do not conceal failed gates, missing comparisons or setup errors in a generic passing summary. Continue through the manifest without a manual pause after each clean pass; perform combined/release checks at the mapped PR endpoints.

After each commit, recompute `git diff <clean-head> <snapshot-sha> --` and update the coverage ledger with applied, remaining and unassigned material. Inspect the content, not just its size. Any temporary intermediate scaffold must have an ID, exact content, reason, opening commit and named closing commit; its final net effect must be zero. No baseline update is planned. Stop and diagnose unexpected behavior or an unexplained residual delta rather than changing the oracle or baseline to match it.

#### 5. Handle failures and checkpoint validated progress

Compare a failure against the pinned base, snapshot and previous safe point to locate the first causal divergence. Before rewriting a validated cleanup commit, preserve its tip as an immutable `cleanup/ieee802154-pr-series/checkpoint-<timestamp>` branch and append the failed attempt and evidence to the logbook. Keep checkpoint branches rather than deleting or repointing them. Revise the group/commit mapping and use the cleanup workflow's approval rules when order, boundaries, behavior classification or verification exceptions change. The proposed N06/N07 fallback requires an explicit plan revision; it must not silently alter PR numbering or dependencies.

#### 6. Prove completion before preparing delivery

Require an empty full-tree diff between cleanup HEAD and the pinned documentation-inclusive snapshot, no unassigned material and no open detours. Also compare implementation/test/configuration/baseline paths with the original topic to prove that only the enumerated documentation additions distinguish the deliverable from that original tree. Record the exact commands and exit statuses. No baseline exceptions are expected in this plan.

Run the union of directly related selected tests against the final matching library, the milestone campaigns above, feature/Clock isolation, managed examples and stored trajectory controls under the accelerated scheduling and exact-input reuse rules. Do not repeat intermediate safe-point checks already executed on those exact trees; revisit N06, N07, default migration, slotted access or scheduled enhanced integration only if rework invalidated their evidence or left a specific coverage gap. Apply the current per-commit and pre-push gates, including debug/release compilation, and audit the whole series against the project review rules. Preserve known mechanical FAIL dispositions and identify newly introduced findings separately. A successful final simulation does not substitute for intermediate build/test evidence. Finish the logbook with the old-to-new SHA map, endpoint SHAs, tree proof and behavior-to-evidence map.

#### 7. Create review branches at validated endpoints

Create the following local refs at the exact already-validated cleanup commits. Do not cherry-pick the series again into twelve independently reconstructed histories. These are cumulative branch tips; each PR's base selects only its own review unit. Verify ancestry and the per-PR commit list/diff against the mapping. Record the actual repository default branch and resolved shared-work ref as `delivery-base` before replacing the table's placeholders.

| Review unit | Proposed branch | Endpoint | Initial PR base |
| --- | --- | --- | --- |
| PR01 | `review/ieee802154-01-foundation` | N04 | `delivery-base` |
| Capture | `review/ieee802154-capture-reader` | C01 | `review/ieee802154-01-foundation` |
| PR02 | `review/ieee802154-02-phy` | N05 | `review/ieee802154-capture-reader` |
| PR03 | `review/ieee802154-03-data-ack` | N06 | `review/ieee802154-02-phy` |
| PR04 | `review/ieee802154-04-management` | N07 | `review/ieee802154-03-data-ack` |
| PR05 | `review/ieee802154-05-native-default` | N09 | `review/ieee802154-04-management` |
| PR06 | `review/ieee802154-06-beacons` | B1-03 | `review/ieee802154-05-native-default` |
| PR07 | `review/ieee802154-07-slotted-cap` | B2-03 | `review/ieee802154-06-beacons` |
| PR08 | `review/ieee802154-08-gts` | B3-06 | `review/ieee802154-07-slotted-cap` |
| PR09 | `review/ieee802154-09-enhanced-codecs` | E1-03 | `review/ieee802154-08-gts` |
| PR10 | `review/ieee802154-10-enhanced-data-ack` | E1-05 | `review/ieee802154-09-enhanced-codecs` |
| PR11 | `review/ieee802154-11-enhanced-discovery` | E1-06 | `review/ieee802154-10-enhanced-data-ack` |
| PR12 | `review/ieee802154-12-enhanced-superframe` | E1-07 | `review/ieee802154-11-enhanced-discovery` |

The separate shared/Wi-Fi PRs are resolved in step 1. If they have not landed, PR01 targets their resolved topic ref and discloses that dependency; when landed it targets the repository default branch. C01 is an additional PR, not one of the twelve protocol units. Its insertion also means PR02 initially targets the capture branch rather than PR01 directly. Never overwrite an existing local/remote review ref without inspecting its ownership and contents first.

#### 8. Prepare descriptions and publish only within delivery authorization

Prepare one description per review unit under `ai-logs/executions/<date>_ieee802154-pr-descriptions/`, named by PR ID plus a separate capture description. Explain the concrete missing behavior, mechanism, resulting capability and limits, then list the actual commits with links and their tests, compatible immediate-parent comparisons, architecture/compatibility surface, and evidence. Disclose failed/unverified gates and protected-source requirements. Link predecessor/successor PRs and this plan; add exchange/timing diagrams where they help and validate their rendering. An overview description may index the stack, but must not become an additional PR containing the whole implementation.

When push and PR creation are explicitly authorized, verify the remote and absence of conflicting refs, push the validated review branches in dependency order, then create the stacked PRs with the exact heads/bases above, using draft status while prerequisites remain unmerged. Prefer the available GitHub connector; if it cannot perform the required operation, use authenticated `gh` with descriptions supplied through body files. Follow current publication and protected-source checks. Do not infer publication authorization from the earlier instruction to add this procedure.

Read back every remote head SHA and PR's head, base, title, description, commit list and changed-file list. Record URLs, numbers and CI state in the logbook and update the plan's mapping with those actual identifiers. Report the difference between created drafts, CI completion and merge readiness. Do not merge PRs as part of creating the series.

#### 9. Maintain the stack after predecessor merges

Land prerequisites before dependents under the maintainer's chosen merge policy. After a predecessor merges, check whether the next branch still contains that predecessor's exact commits. If ancestry is preserved, retarget the next PR to the default branch and verify its diff. If squash/rebase merging changed those identities, preserve the validated tip, replay only the next unit onto the actual landed base, update dependent branches in order and rerun affected checks under the appropriate rebase workflow. Do not remove apparently duplicated commits without comparing their behavior.

Before any authorized replacement push, verify the expected remote tip and use a lease. Refresh the remote SHA/base/commit mapping, links, descriptions and CI evidence after each update. Keep the original topic, input snapshot, checkpoints and execution evidence until the user explicitly requests their cleanup. Completion of this procedure means the authorized review branches/PRs exist with verified contents and evidence; it does not mean they have been approved or merged.

### Historical native publication sequence

The following records the original implementation boundaries. The proposed mapping above supersedes it for future PR construction, particularly its single nonbeacon MAC commit; it does not change the historical execution record.

The remaining work is published as the following dependency-ordered commits.
The managed example explicitly selects Ieee802154StandardInterface so its commit
does not depend on the later public-default cutover. The isolated intermediate
run reproduced the former ordering defect (unknown extendedAddress on the old
default); this configuration-only correction preserves the final native module
graph. Its direct check is the same two-run example and scalar checker.
The final integrated tree has focused executed evidence; isolated intermediate
builds and selected checks are verified before publication. No recorded
fingerprint changes are planned. The new MAC is one protocol service owner with
coupled DATA, ACK, management and lifecycle transitions; its initial addition
exceeds the source-size advisory. Independent codecs, PHY, example policy and
default cutover remain separate commits rather than being bundled into it.

| Commit | Decision and surface | Direct verification |
| --- | --- | --- |
| `ieee802154: add: complete native legacy-frame codecs` | Native wire chunks, frame/command/beacon codecs, serializer/dissector/printer and admitted vectors. | Fcs, FrameWire, Commands, Beacons unit fixtures; dedicated serializer. |
| `packet: fix: preserve captured length in truncated records` | Approved bounded PcapReader repair and capture-boundary unit fixture. Depends on wire fixtures for native round-trip checks. | Capture unit; parent/current generic serializer comparison. |
| `ieee802154: add: timed native O-QPSK PHY services` | Paired PHY roles, measurements, state/TX timing and observable PPDU representation. | PhyServiceContract, PhyServices, Measurements, ExternalPhy module fixtures. |
| `ieee802154: add: native nonbeacon MAC services` | Native identity/adapter, unslotted access, DATA/ACK, queued management, scans, join/leave/recovery, receiver schedules and lifecycle. | NativeInterface, UnslottedAccess, Start, Discovery, PassiveScan modules; MacPib/PendingTransactions units; all native protocol fixtures and companions. |
| `ieee802154: add: managed PAN example policy` | Application-owned PAN selection/allocation and repeated managed cycles; feature package registration. | Two typed/raw managed example runs and scalar checker. |
| `ieee802154: change: select native narrowband interface` | Public native default, explicit deprecated legacy type, inventoried consumer selections and migration/release guidance. | Composition, NativeGate, EnergyObservation, LegacyUwb module fixtures; stored legacy fingerprints and fake-GUI visual fingerprint. |
| `doc: change: close native IEEE 802.15.4 milestones` | Final obligation mapping, source/artifact identity, commands, scoped review and known inherited gate failures. | Link/seal/WHATSNEW checks and final diff review. |

Follow [run the gates](../../doc/project/guide/run-the-gates.md). For each implementation PR:

1. Record changed paths/contracts, normative check IDs and explicitly selected test cases.
2. Build a fresh debug library before behavioral tests; compile release before push. Use the
   relevant runner's documented selector, verified with its current help, and record selected
   versus executed counts. Zero selected cases are `NOT_RUN`.
3. Run scoped architecture/naming/interface checks and the semantic review checklist; then the
   prescribed pre-push gates. Verify feature isolation when feature dependencies change.
4. Record command, working directory, build mode, HEAD/worktree state, configuration, seed,
   expected invariant, verdict and artifact path. Update model results/coverage, not normative text.
5. Explain baseline changes under the
   [baseline procedure](../../doc/project/guide/change-a-baseline.md); ship evidence and release
   notes with the behavior. Keep mechanical renames separate from behavioral commits.

Commands already documented in this checkout include:

```sh
make -j$(nproc) MODE=debug
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee802154
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee802154
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee802154
doc/project/enforcement/check-interfaces.sh
# Run the explicit unit/module/protocol selectors recorded by the implementing PR.
make -j$(nproc) MODE=release
```

These gates were not executed while initially writing the plan. Their final
executions and explicit inherited failures are now recorded in the results ledger.

## 9. Completed execution record

Steps 0–10 deliver the selected M1/M2 services. The publication sequence above
separates independent wire, shared capture, PHY, native MAC, example policy and
default cutover decisions. Every source-bearing unit has a matching debug build
and selected checks in the isolated control checkout. The final tree also compiles
in release mode; enabled/disabled feature configurations and stored legacy
trajectories are verified without baseline changes.

The source/test manifest, exact commands, per-statement mapping, parent controls,
review scope, model limits and inherited failures are linked from the
[results ledger](../../doc/project/evidence/model/ieee802154/results.md) and
[machine-readable record](https://github.com/mgonzalezlopezudc/inet/blob/ec1ee66b1b/doc/project/evidence/model/ieee802154/verification-20261115.json).
The matched-object reuse for the example/cutover checks is explicitly recorded;
it is not reported as independent recompilation of every translation unit.
Future feature packages in section 6 require their own applicability and
implementation gates. Completing this plan does not claim those features.
