# IEEE 802.15.4 — applicability-pass evidence

> **Kind:** report · **Status:** snapshot 2026-09-19 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md), [applicability.md](applicability.md)

Initial baseline `fd6f800222`; continuation baseline `6cb03df7c8`. Working directory `/home/user/omnetpp_ws/inet-ieee802154-standards`.
Mode: documentation/source/capture inspection; no simulation or library build. The checkout has
no `out/` or `src/libINET*` artifacts. All nine new English checks are `NOT_RUN`; there are no
executable test paths yet. This pass does not establish a Level 2 result or a conformance verdict.

## Reproducible retrieval

Run from `/home/user/omnetpp_ws/inet-skills`. The launcher checkout is `d36cf47585eef165ba2cc7642fe4d0965616d6b8`; the source identity remains the hash in [source.md](../../standard/ieee802154/source.md).
Generated corpus output stays ignored and is not part of the evidence source tree.

```sh
./bin/inet_process_standards status \
  --standards-dir /home/user/omnetpp_ws/inet-ieee802154-standards/standards \
  --output /home/user/omnetpp_ws/inet-ieee802154-standards/standards/processed
./bin/inet_process_standards build \
  --pdf /home/user/omnetpp_ws/inet-ieee802154-standards/standards/802154-2024.pdf \
  --standards-dir /home/user/omnetpp_ws/inet-ieee802154-standards/standards \
  --output /home/user/omnetpp_ws/inet-ieee802154-standards/standards/processed
./bin/inet_process_standards lint \
  --standards-dir /home/user/omnetpp_ws/inet-ieee802154-standards/standards \
  --output /home/user/omnetpp_ws/inet-ieee802154-standards/standards/processed --json
```

Initial status reported a missing manifest. Build succeeded: 967 pages, 2814 nodes, 4107
occurrences and 3440 references, matching source hash prefix `851d81127297`. Lint exited zero
with 169 rejected false-heading candidates, one tiny node and 18 unresolved-reference warnings;
1148 duplicate-label candidates are informational. This is not a clean full-document reference audit.
An initial build using a relative `--pdf` path from the launcher checkout failed; the absolute-path
command above is the successful reproduction.

For each catalog source node, use the same directory arguments with these operations:

```sh
./bin/inet_process_standards get <canonical-node-id> --document ieee802154-2024 --children --json <directory-arguments>
./bin/inet_process_standards refs <canonical-node-id> --document ieee802154-2024 --json <directory-arguments>
```

The angle-bracket arguments above are templates; actual node IDs and physical page/source spans
are recorded per catalog entry. The initial 40 source excerpts were compared to their retrieved node text
with whitespace normalization. Twenty-seven distinct source nodes yielded 66 outgoing references,
all resolved. No request hit the 100-reference limit. Resolution does not establish applicability
or extraction of the target. In particular, the corpus places continuation prose under table/figure
nodes: retrieving only a parent clause does not retrieve all its normative text.

PDF pages 63, 64 and 91 were rendered and visually inspected for Figures 6-1, 6-2 and 7-15.
The inspected CSMA flowchart tests `NB > macMaxCsmaBackoffs`. The IFS figure shows the
size-dependent spacing after ACK. The ACK format has FCF, DSN and FCS, without addressing.
The continuation after Figure 7-16 was retrieved for the remaining ACK FCF rules.

## Current source findings, not runtime reproductions

| Surface | Reproducible observation | Evidence limit |
| --- | --- | --- |
| `src/inet/linklayer/ieee802154/Ieee802154MacHeaderSerializer.cc` | Fixed `0xCC01`; emitted header size is 2+1+2+8+2+8=23 octets; sequence write masks with `0xFF` | Source arithmetic, not an executed serializer fixture |
| `Ieee802154NarrowbandMac.ned` in the same directory | Default `headerLength=72 b` and MTU 127−9=118 bytes | The 9-octet declaration differs from the 23 emitted octets; actual byte-conversion failure remains to be reproduced |
| `Ieee802154Mac.cc` | `SeqNrParent` allocates per destination; `SeqNrChild` compares received `long` sequence against monotonically expected value with `<` | After serialized 255→0, this comparison can classify a fresh frame as old; no production wrap exchange was executed |
| `src/inet/networklayer/common/NetworkInterface.h` | Generic address/filter APIs use `MacAddress`; protocol data can be attached | Does not prove native-address tags, dispatch or upper consumers work |
| `src/inet/common/packet/recorder/PcapReader.cc` | Link type 195 sets FCS present; 230 sets FCS absent | Sealed source inspected only; no replay/writer correctness claim |

## PHY feasibility inspection

Read-only source inspection at the continuation baseline found an energy-observation path that
does not require a successfully decoded PPDU:

| Boundary | Source evidence | Remaining implementation/evidence |
| --- | --- | --- |
| Existing CCA | `Ieee802154Mac::updateStatusCCA()` checks reception state at timer expiry; duration is rxSetupTime + ccaDetectionTime | Does not establish observation over the complete interval |
| Medium observation | `IRadioMedium::listenOnMedium()` and `RadioMedium::listenOnMedium()` construct interference/background noise and invoke receiver listening decisions | Timed request, cancellation and terminal-result ownership |
| Scalar energy | `FlatReceiverBase::computeListeningDecision()` compares maximum noise power in W over an interval against the ED threshold | Quantitative ED requires averaging and calibration; a Boolean maximum-power result is insufficient |
| Available extension | Public virtual receiver listening decision; scalar noise exposes time-dependent power | Define a protocol-local measurement contract and check cache retention for energy that ends before completion |
| Propagation | IPropagation exposes speed and actual arrivals, but no universal maximum-delay guarantee | Pin a supported propagation model and finite geometry/movement assumptions or provide a separate explicit bound |

These symbols are under `src/inet/physicallayer/wireless/common/` except the existing MAC under
`src/inet/linklayer/ieee802154/`. The scalar medium supports compatible same-center/contained-band
interference; partial overlap has additional restrictions. Medium filters and retained interference
history must be pinned by the fixture. The existing instantaneous radio-state query does not retain
all information needed by a timed observation. No source change is needed in the sealed packet tree
for the candidate protocol-local provider/receiver extension.

The minimum production fixture is a timed CCA/ED request with an undecodable energy pulse wholly
inside the observation interval, ending before completion. Observe outcome, no PSDU delivery,
completion time, threshold cases and cancellation. A separate stationary two-radio fixture under
ConstantSpeedPropagation must compare actual arrivals with the declared ACK timing allowance.
These are proposed checks, not executed results; neither architectural feasibility nor declared
radio defaults establish PHY validation.

## Extended source audit

The continuation adds 42 source-checked statements, bringing the catalog to 82. It records all
31 MAC rows in Table 8-36 and 13 generic PHY rows in Table 12-2, including access markers,
PHY-specific domains and the absence of a PHY default column. Table 12-3 closes the peer-power
structure lookup. These field inventories are not 44 additional catalog statement IDs.

Fifty distinct catalog source nodes produced 122 extracted outgoing references, all resolved,
without query truncation. This is a first-hop target check, not transitive applicability closure.
Table 8-1 has 33 primitive rows (7 unstarred, 26 with starred references) and 73 distinct
cell labels; all 73 were resolved by direct target lookup. Service and PICS tables expose
cell references that the extractor omits; complete continuation inspection remains necessary. Annex E is informative; its generic
ED/LQI option labels do not override mandatory O-QPSK clauses 13.3.12/13.3.13. Extraction also
found false/unresolved Annex references, so the successful catalog-node edge check must not be
reported as a clean whole-standard reference audit.

Physical PDF page 84 was visually checked for the 7.2.11 independent CRC example. The transmitted
bit sequence corresponds to `02 00 6A`, with FCS `E4 79`; polynomial long division using
`x^16+x^12+x^5+1` independently reproduced the remainder. The resulting five-octet ACK has
352 µs PPDU duration under the selected O-QPSK format. This checks source arithmetic, not the
INET serializer. The exact octets and check procedure live in the
[wire check](../../protocol/ieee802154/checks.md#ieee802154-c-wire).

Four bounded specialist lanes supplied the mandatory/service/PICS inventory, PIB/PHY extraction,
receive interpretation and existing medium feasibility. Root integration retained source versus
model distinctions and did not accept the extraction summaries as conformance verdicts.

## Migration inventory

| Consumer | Required later evidence |
| --- | --- |
| `examples/wireless/nic/omnetpp.ini`: CSMAWithUnitDiskRadio, CSMAWithApskScalarRadio, CSMAWithApskDimensionalRadio | These select `Ieee802154Mac` with generic radios; identify explicit legacy retention versus supported replacement PHY before migration. |
| `showcases/wireless/ieee802154/`: Ieee802154, Ieee802154Power | Native-address/payload migration plus radio and power integration. The plain Ieee802154 row is commented out in `tests/fingerprint/showcases.csv`; do not count it as an active baseline. |
| `showcases/wireless/coexistence/`: Coexistence and WpanHosts | Mixed-radio medium behavior and upper-protocol adapters; preserve scenario meaning. |
| `.oppfeatures`: Ieee802154 | MAC and PHY package selection plus feature-disabled build; avoid accidental UWB changes. |
| `tests/serializer/lib/fillers/ChunkFillers_linklayer.cc` and `tests/serializer/REMAINING_GAPS.md` | Replace assumptions deliberately when the new chunk/codec lands; the capture is currently excluded as unrepresentable. |
| `tests/fingerprint/examples.csv`, `tests/fingerprint/showcases.csv` | Scoped before/after evidence for the named configurations; no baseline update in this pass. |
| `Ieee802154UwbIrInterface.ned` | Keep its distinct AckingMac composition; narrowband retirement does not retire UWB. |

This is a first consumer inventory, not proof that all callers have been migrated.

## Existing capture inventory

Executed from the INET checkout; inspection commands exited zero:

```sh
capinfos tests/serializer/pcap/ieee802154.pcap
sha256sum tests/serializer/pcap/ieee802154.pcap
tshark --version
tshark -n -r tests/serializer/pcap/ieee802154.pcap -T fields \
  -e frame.number -e frame.len -e frame.protocols -e wpan.frame_type -e wpan.seq_no
tshark -n -r tests/serializer/pcap/ieee802154.pcap -T fields \
  -e frame.number -e wpan.frame_type -e wpan.version -e wpan.fcs_ok
git log --follow --format='%h %s' -- tests/serializer/pcap/ieee802154.pcap
```

- File SHA-256: `7b8b59bc88f3a23fc979e41570646618b353c9779cb9aa3e9f0d97d6576d5855`.
- 440-byte classic PCAP, 13 packets, 208 captured data bytes, link type **195** (confirmed in
  the PCAP header; capinfos's internal encapsulation number 104 is not the link type).
- Decoder: TShark/Wireshark **4.6.4**. Observed types include legacy beacons/data, immediate or
  enhanced ACK, version-2 commands and multipurpose frames. The file is not a homogeneous M1 fixture.
- `wpan.fcs_ok` is false for frames 3, 5, 7, 9 and 12; empty values for other frames do not prove
  valid FCS. Capture admission must inspect the exact selected frames and decoder preferences.
- History traces to `0a30ea4a5b` and suite relocation `7287f347aa`. The former describes a mixture
  of Wireshark samples, derived and generated captures, without identifying this file's exact
  upstream source. Provenance remains incomplete. Container timestamps are not transmission timing
  validation; capinfos reports dates in 2104.

Do not repair captured bytes or relabel this file as a known-good standard vector. Step 3 needs
independent bounded vectors and explicit per-frame admission.

## Document validation

Executed from the INET checkout, all exited zero:

```sh
python3 doc/project/enforcement/check_links.py doc/project/evidence/standard/ieee802154
python3 doc/project/enforcement/check_links.py doc/project/evidence/protocol/ieee802154
python3 doc/project/enforcement/check_links.py doc/project/evidence/model/ieee802154
git diff --check
```

The three scoped link checks covered 2, 3 and 4 files respectively, with zero broken links.
The initial structural check confirmed 40 unique catalog headings and the 37-selected/3-deferred
partition. The continuation checks 82 unique catalog headings, nine unique check headings, every
catalog ID present in the feature map/checks/ledger, and the 79-selected/3-deferred partition.
Specification documents were checked for model class names and run-verdict leakage.

The broader command `python3 doc/project/enforcement/check_links.py plan/pending` exited 1:
four existing relative links in `pr-1155-resolve-audit-findings.md` do not resolve. None is in the
802.15.4 plan or changed by this pass; that separate plan was left unchanged. Documentation checks
are not simulation evidence, and no serializer, module, protocol or fingerprint result is claimed.

The targeted independent documentation review identified an overbroad group-ACK receive rule,
an invented ED cutoff and interrupted Markdown table rendering. All three were corrected and
confirmed resolved by the same reviewer. The stable results sections received no additional
actionable findings. This review did not exhaust transitive normative dependencies or verify
production behavior; the applicability gate remains open.

## Service and integration closure continuation

Baseline `837ee6dbe8`. Corpus status remained ready/fresh for the pinned IEEE 802.15.4-2024
source; unrelated 802.11 corpora remain absent. Retrieved 11.1.3.1 and Table 12-2 to close the
channel descriptor lookup. The selected M1 descriptor and its distinction from a legacy page
number are recorded in [applicability.md](applicability.md#selected-channel-descriptor).

The [IEEE 802-2024 publication record](https://ieeexplore.ieee.org/document/10935844/footnotes)
identifies the companion standard and its IEEE GET distribution route. Attempts to access the
GET page and PDF through the web tool failed (HTTP 418 and inaccessible PDF endpoint).
No normative companion text was retrieved; the group-address prerequisite remains open.
Publication metadata and informative address guidance are not replacements for that source.

The service lookup resolved direct-data error routing from 8.2.4.4/Table 8-5, 6.6.1–6.6.2,
9.2.2/9.2.4 and Table 8-31. The additional SERVICE-8 excerpt was compared against the complete
Table 8-5 continuation. The catalog now has 83 statements (80 selected, 3 deferred), with nine
English procedures; no executable results were added.

Read-only consumer tracing produced the
[native compatibility decision](applicability.md#native-address-compatibility-decision).
In particular, NetworkInterface's parameter named address is parsed as MacAddress, generic
ARP/ND/forwarding consumers remain 48-bit, and radio-medium address filtering does not provide
a native-address contract. Public attachment, tag, registration and classifier APIs provide the
candidate integration route. This is static source evidence, not successful module initialization
or an architecture-compliance verdict.

The clause-6 walk inventoried 28 descendant clause nodes and 14 table/figure nodes. Detailed
6.4/6.5 leaf retrieval included the passive-scan prose under Figure 6-3, and the 11 distinct
Table 6-1 IE targets resolved through direct lookup despite zero extracted table edges.
Root also retrieved 6.6.4–6.6.6 and Figures 6-7–6-13 to classify ATI/guard-time predicates and
preserve distinct lost-data versus lost-ACK checks. No new timing oracle was inferred from the
diagrams; this pass used their continuation prose. Full M2 service extraction remains deferred.

Continuation validation passed the three scoped link checks (2/3/4 files), git diff --check,
83 unique catalog headings with complete feature/check/ledger mapping, the 80-selected/3-deferred
partition, nine check headings and exact normalized comparison of the new SERVICE-8 excerpt.
Independent targeted review corrected SERVICE-8 metadata to the defined vocabulary and retained
mandatory scan termination at descriptor capacity. The same reviewer confirmed both findings
resolved. This was documentation verification; all executable checks remain NOT_RUN.

## Companion IEEE 802 source closure

The supplied `standards/802-2024.pdf` resolves the earlier retrieval failure. Its pinned identity
is in the [companion source record](../../standard/ieee802/source.md). The tracked processor's
reviewed document list does not include this document, so direct PDF fallback was used without
inventing a processed-corpus node ID or modifying the shared processor.

```sh
sha256sum standards/802-2024.pdf
pdfinfo standards/802-2024.pdf
pdftotext -layout standards/802-2024.pdf /tmp/ieee802-2024.txt
pdftoppm -f 41 -l 41 -scale-to 1500 -png -singlefile standards/802-2024.pdf /tmp/ieee802-address
pdftoppm -f 43 -l 43 -scale-to 1500 -png -singlefile standards/802-2024.pdf /tmp/ieee802-eui64
```

Commands exited zero. Physical pages 41 and 43 were visually inspected; clause 8.2.2 identifies
the group bit in conventional notation and Figure 10 identifies the first octet. Clause 8.5 on
physical page 49 distinguishes group MAC addresses from EUI identities. Combining that source
with IEEE 802.15.4 4.5.1 yields the masks and three synthetic vectors in C-WIRE. Their byte-order
arithmetic was checked independently; it is not an executed production codec or filter result.
Two companion definitions bring the ledger to 85 statements: 83 from IEEE 802.15.4 plus two
from IEEE 802, with 82 selected and three deferred. Source availability and the group-bit layout
are closed; reception/ACK policy remains governed separately by IEEE 802.15.4.

A targeted independent review confirmed the companion source, derived masks and vectors. It
also checked 7.2.2.5, 6.6.2 and 6.6.3.1: no additional receive rejection predicate invalidates a
nonbroadcast group frame solely because AR=1. The documented receive decision applies ordinary
filtering and ACK rules while preserving the sender prohibition. This closes that interpretation;
it does not assert that the violating injected transmitter conforms.

The four scoped link checks passed (2/2/3/4 files), as did whitespace checks and the 85-ID mapping,
82-selected/3-deferred partition and three synthetic byte-vector calculations. The added source
PDF remains an ignored local artifact; no executable runtime check has been added or run.

## Bounded M1 step-0 gate review

At baseline `5f3845f055`, the remaining malformed/capability policy was resolved using 8.2.2,
Table 8-11 and the ordered security early returns. SERVICE-9 records unsupported-parameter
INVALID_PARAMETER; ACCESS-3 makes the IFS size boundary explicitly checkable. Table 8-37 adds
22 Boolean capability/enable fields to the source inventory. Optional Data timestamping is
explicitly disabled; invalid typed MCPS metadata is a documented model representation.

The [dependency record](dependencies.md) classifies all 86 first-frontier targets. Expanding the
34 selected targets produced 80 occurrences: 79 resolved plus a non-protocol trademark footnote
that remains an unresolved parser record. The 18 additional target IDs contain one original
source (Figure 7-16), leaving 17 new to the original source/target union. Figure 4-5 was visually
checked on physical page 51; its outgoing-reference query returned zero. Table 9-6 selects level
zero, records level 4 as reserved/deprecated, and references only already reviewed 9.2.4.
Manual IFS root 6.3.1/Table 8-35 adds the 18-octet boundary. Predicate cuts stop disabled features
without declaring mandatory deferred profile capabilities inapplicable.

Independent standards review returned PASS for the bounded M1 applicability gate, then rechecked
the dedicated IFS addition and retained that verdict without findings. It did not assess runtime
support, a concrete propagation bound, integration fixtures or complete-device conformance.
Detailed mutation, event ordering and API representation remain the assigned implementation work.

Documentation validation after closure passed: scoped `check_links.py` runs for the base-standard,
protocol and model evidence directories (2/3/5 files, zero broken links), `check-seals.sh`
(19 document seal units, index consistent), and `git diff --check`. A structural check confirmed
87 unique catalog IDs, exact coverage-row correspondence, the 84-selected/3-deferred partition,
and nine English checks. These are documentation checks, not executable model evidence.

## Step-1a native address value

The first production subunit is the standalone
[Ieee802154Address](../../../../../src/inet/linklayer/ieee802154/Ieee802154Address.h) value, with
[implementation](../../../../../src/inet/linklayer/ieee802154/Ieee802154Address.cc) and a focused
[unit case](../../../../../tests/unit/Ieee802154Address_1.test). It has no existing MAC, PHY,
serializer, packet-tag or interface caller. Runtime simulation behavior is not changed by this
addition. The service-contract subunit is recorded below; production integration remains later work.

The pre-write contract was completed read-only by the implementer and independently validated
before write authorization. The value owns mode plus complete numeric identity, while parsing
validates before assignment. Wrong-mode access and throwing parse APIs use `cRuntimeError`;
nonthrowing parse refusal retains the prior value. There are no packet-ownership, timer, callback
or lifecycle paths in this subunit. The target subtree is unsealed and no shared-core edit is
needed. The contract deliberately defers byte order, MSG declarations and runtime integration.

Source classification uses IEEE Std 802-2024, 8.2.2 for the numeric bit-56 group flag and all-ones
broadcast; IEEE Std 802.15.4-2024, 6.2 for broadcast and 10.21.5.2/10.4.12.2 for `0xfffe` meaning
association without short allocation. Clause 6.6.1 independently excludes `0xfffe` and `0xffff`
from allocated short source values. All values remain representable, including group extended
values; constructing a value does not certify a valid device EUI identity.

Validation from the repository root, debug mode, default enabled feature set:

- `make MODE=debug -j8`: fresh full build passed, exit 0; rerun after adding the source passed
  and explicitly compiled `Ieee802154Address.cc`. Logs: `/tmp/802154-debug-build.log` and
  `/tmp/802154-address-build.log`.
- `MPLCONFIGDIR=/tmp/802154-matplotlib inet_run_unit_tests -m debug -f 'Ieee802154Address_1\.test'`:
  one executed case, PASS, exit 0. Log: `/tmp/802154-address-unit.log`; normalized envelope:
  `/tmp/802154-address-verification.json`. The earlier test compilation failed on three
  `std::string` arguments passed to the `const char *` constructor; the test calls were corrected
  and the same focused command passed. No runtime assertion failed in that earlier attempt.
- Scoped `check-architecture.sh` and `check-naming.sh` on `src/inet/linklayer/ieee802154` passed,
  as did `git diff --check`; implementer syntax and formatting checks also passed.

The unit test proves the standalone value API, including high-16-bit distinction through a hashed
container, sentinel/mode separation, strict format, malformed/null rejection and full-width boundary
values. It does not prove serialization, production delivery, frame exchanges or M1 support.
Release compilation and runtime/fingerprint campaigns have not been run for this subunit.

Independent review of the final three-file source/test change found no actionable correctness
findings: **11 PASS, 15 N/A, 0 FLAG, 0 QUESTION** under the general semantic checklist. Its API
visibility observation was resolved by making the comparison helper private; retained public mode,
comparison and stream operations have direct behavior assertions. The final debug rebuild and
focused unit rerun both passed after that change. Review artifact with exact file hashes:
`/tmp/802154-address-review.md`. No exception-ledger change or sealed-path approval was needed.

## Step-1a MAC service contracts

The bounded M1 facade adds paired, gate-free
[provider](../../../../../src/inet/linklayer/ieee802154/contract/IIeee802154MacServiceProvider.h)
and [client](../../../../../src/inet/linklayer/ieee802154/contract/IIeee802154MacServiceClient.h)
roles with matching NED interfaces. Protocol-local values represent DATA, GET, SET and RESET,
normal/promiscuous reception and COMM-STATUS. IEEE completion statuses remain separate from
local admission refusals and cancellation/reset/stop outcomes. This adds declarations and a
test-local provider; no existing MAC implements the facade yet.

The pre-write contract assigns packet ownership at acceptance, copies borrowed metadata for
deferred work, detaches pending state before callbacks, and permits inline completion. Request
IDs remain distinct from repeating MSDU handles and cannot be reused across refusal or lifecycle
boundaries. RESET is a barrier; client deletion requires cleanup without invoking a dead module.
The selected facade fixes LegacyTx and DataRate, keeps source identity in the future PIB owner,
and distinguishes wire-carried Source PAN ID from effective receive context. ACK RSSI's numeric
interpretation and uint8 representation are explicit local choices for Table 8-31's inconsistent
type entry. These are contract decisions, not evidence of operational IEEE service behavior.

The test-only [module fixture](../../../../../tests/module/Ieee802154MacServiceContract_1.test)
is the direct observation boundary for these ownership and callback claims. Production PIB
mutation, native interface delivery, PHY services and wire exchanges remain later packages.

Validation from the repository root, debug mode, default enabled feature set:

- `make MODE=debug -j8` passed, exit 0 (`/tmp/802154-service-build.log`). This establishes
  library freshness; existing runtime sources do not include the new facade. The implementer's
  standalone C++17 syntax check and the module fixture compile exercise the new headers.
- `MPLCONFIGDIR=/tmp/802154-matplotlib inet_run_module_tests -m debug -f 'Ieee802154MacServiceContract_1\.test'`
  compiled and executed one case, PASS, exit 0, General run 0. Raw log:
  `/tmp/802154-service-module.log`; normalized envelope: `/tmp/802154-service-verification.json`.
- The fixture checks inline completion and reentrancy, borrowed metadata copying, request-ID and
  MSDU-handle correlation, refusal/acceptance/indication ownership, packet destruction counts,
  DATA/GET/SET cancellation, stale completion, reset abort ordering, shutdown/crash/restart and
  client-deletion cleanup. It also exercises typed GET/SET values, unknown GET, separate
  promiscuous metadata and absent M1 timestamps. These are test-local contract checks.
- An intermediate assertion run failed because the fixture assumed newly created packets had
  no owner. OMNeT++ assigns the current module as owner. Corrected assertions check retained
  caller ownership on refusal and provider-to-client transfer on indication; the final focused
  rerun passed. Failure artifacts: `/tmp/802154-service-module-owner-failure.log` and `.err`.
- `opp_nedtool validate src/inet/linklayer/ieee802154/contract/*.ned`, scoped interface,
  architecture and naming checks, and `git diff --check` passed. NED interfaces declare parameter
  types only; concrete modules supply defaults and validate the typed binding.

Release compilation, operational MAC/PHY tests and fingerprint campaigns were not run for this
contract-only subunit. The nine normative protocol checks retain NOT_RUN verdicts.

Independent review of the final seven production declaration files and module fixture found no
actionable correctness findings: **18 PASS, 8 N/A, 0 FLAG, 0 QUESTION**. The reviewer confirmed
resolution of the fixture ownership assertion defect and the passing final run. Full checklist
and exact file hashes: `/tmp/802154-service-review.md`. Production MAC/PIB behavior and
reset-interruption edge paths remain unverified; no exception-ledger or seal changes were needed.

## Step-1b selected MAC PIB

The [Ieee802154MacPib](../../../../../src/inet/linklayer/ieee802154/Ieee802154MacPib.h)
[implementation](../../../../../src/inet/linklayer/ieee802154/Ieee802154MacPib.cc) owns the 43
selected MAC attributes described in the [mutation contract](pib.md). It is a synchronous class,
with no module, RNG, timers, callbacks or existing MAC caller. The future provider owns admission,
request tracking, effective operational changes and notifications. No mutable PHY PIB is owned here.

The implementation reuses the package-1a GET-confirm and SET-request values. GET returns canonical
unsigned integers, exact Boolean variants or native extended-address values. SET rejects unknown
names, read-only writes, invalid types, unsupported values and inconsistent BE pairs without
changing any attribute. Startup overrides validate as a complete candidate, independent of order.
Reset(false) retains all values; reset(true) restores standard/profile defaults using an injected
DSN and retaining device identity and immutable PHY initialization context, without replaying
startup overrides. Store reset alone is not evidence of an operational MLME reset.

The architecture and normative lanes preceded a complete read-only implementation contract;
the root validated that contract and separately authorized the three source/test files. Normative
review confirmed the explicit local representations for unspecified coordinator context and the
backoff range. Exact table/RESET extraction is in `/tmp/802154-1b-pib-evidence.md`; the tracked
catalog and mutation contract preserve its relevant rules and locators.

Validation from the repository root, debug mode, default enabled feature set:

- `make MODE=debug -j8`: PASS, exit 0; explicitly compiled `Ieee802154MacPib.cc` and linked the
  debug library. Log: `/tmp/802154-pib-build.log`.
- `MPLCONFIGDIR=/tmp/802154-matplotlib inet_run_unit_tests -m debug -f 'Ieee802154MacPib_1\.test'`:
  one executed case, PASS, exit 0. Log: `/tmp/802154-pib-unit.log`; normalized envelope:
  `/tmp/802154-pib-verification.json`.
- Scoped `check-architecture.sh` and `check-naming.sh` passed for
  `src/inet/linklayer/ieee802154`; formatting and `git diff --check` passed.
- Additional static C++ tooling was not run successfully: its compilation database was absent,
  and the include checker stopped on a UTF-8 error in historical commit data. These are tooling
  limits, not passing checks or failures of the focused unit case.

The [unit case](../../../../../tests/unit/Ieee802154MacPib_1.test) independently enumerates 43
expected defaults/access markers, checks exact-name lookup and canonical value types, rejected
writes and state preservation, writable boundaries, unsupported capability enables, full-width
identity, startup errors and BE ordering, CCA rounding, and reset retention/default restoration.
Three draft fixture mistakes (a group-address identity, writing the unknown-coordinator sentinel,
and a successful write inside a failure-only snapshot) were corrected before the first test run.

This evidence establishes the compiled store's behavior only. Production service integration,
NED parameter import, operational change notifications, PHY reset, and RNG stream/draw scheduling
remain provider/composition work. Release compilation and simulation/fingerprint campaigns were
not run. The nine normative protocol-check verdicts remain NOT_RUN.

Independent review of the stable source/test change returned **11 PASS, 15 N/A, 0 FLAG,
0 QUESTION**, with no actionable correctness findings. It independently reran the scoped
architecture and naming checks. Exact file hashes and the full checklist are recorded in
`/tmp/802154-pib-review.md`. Documentation links, seal-index consistency and whitespace checks
also passed. No exception-ledger, source-seal or baseline changes were needed.
