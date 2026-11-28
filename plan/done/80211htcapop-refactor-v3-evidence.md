# HT capability/BSS refactor V3: implementation evidence

Date: 2026-09-18. Baseline: `98117c3257b2e11661d2baf685c18911c8639b24`.
Tested implementation: the uncommitted working tree following that baseline. No commits, recorded
fingerprints, generated fingerprint expectations, or sealed packet-core sources were changed.
The proposal's relevant source/test paths match baseline `3f89b4b439c1dafd6b217ae9beb94b604f50c615`.

## Implemented decisions

- MAC assembles capabilities through `IIeee80211TransmitterCapabilities` and
  `IIeee80211ReceiverCapabilities`, configured catalog and antenna limits. Preparation is idempotent;
  MIB installation does not construct operation. A changed profile cannot replace an active BSS or
  installed peer relationship. Ordinary runtime profile reconfiguration is outside this change.
- Management constructs operation and owns accepted transitions. MIB stores an explicit active BSS,
  decoding band/channel context, optional operation, accepted capabilities and association state.
  Its BSS/profile structures are private, exposed through const queries and guarded writes.
- The AP retains physical channel context while down, reconstructs operation on restart, and uses
  its current operation after response completion. STA response/discovery provenance remains local
  to management. Scan tuning is not used as accepted BSS state.
- `bssStateChanged` is a MIB Boolean signal with no borrowed payload. Management publishes after
  required timer/transaction work. Observers query the committed MIB; nested MIB mutation is rejected.
  Equality suppresses redundant state notification, not accepted-beacon liveness refresh.
- Directional capability results are immutable shared values. Equal capability inputs reuse them;
  operation-only changes reevaluate eligibility without deriving again. Teardown and acknowledged
  relationship replacement remove the old cache. A retained shared pointer proves reuse/lifetime in
  tests without adding a public counter. Raw peer/BSS references must not survive their next mutation.
- Both selection modules pass accepted capabilities, explicit operation and HT eligibility to the
  common compatibility filter. Management/control/group dispatch and legacy fallback remain intact.
- Static catalog broadcasts and `ModeSetListener` are removed. The replacement base queries a declared
  provider at `LINK_LAYER`; the MAC supplies `**.modeSetModule` defaults to descendants in either
  standard or external-upper composition. Management retains its existing `macModule` parameter.
- `LINK_LAYER` now explicitly depends on `NETWORK_INTERFACE_CONFIGURATION`, as well as PHY readiness.
  Simplified management prepares the AP through `IIeee80211BssProvider`, then installs local and
  remote state through owner methods. Identity and HT state are ready before network configuration.
- Ad hoc management explicitly activates/clears its no-beacon abstraction without inventing learned
  channel, HT operation, or peer advertisements. Prepared local HT/legacy support survives lifecycle.

The concrete notification and lifetime contracts are documented in
[the WLAN architecture](../../doc/project/design/ieee80211-model-architecture.md).

## Consumer readiness worksheet

For all catalog consumers below, `modeSetModule` identifies a module implementing
`IIeee80211MacConfiguration`. The default is the enclosing MAC's absolute path, set by MAC NED;
standalone compositions specify their own path. Catalog selection is ready after all `LOCAL`
callbacks. `ModeSetModuleBase::initialize(LINK_LAYER)` obtains it before the derived callback.
All retain `NUM_INIT_STAGES`. Catalog acquisition performs no algorithm reset on ordinary queries.

| Consumer | Additional prerequisite / preparation at LINK_LAYER | First read / lifetime rule |
|---|---|---|
| `Dcaf` | Resolve queue/contention/Rx and calculate slot/SIFS/IFS/EIFS/CW defaults | Contention request; preserve subsequent CW changes |
| `Edcaf` | AC/parameter inputs initialized at LOCAL; register contention and calculate timing/CW | Contention request; preserve subsequent CW changes |
| `Dcf` | Existing coordination references | Protocol actions; retain unrelated drop listener |
| `Hcf` | Existing coordination references | Protocol actions; retain unrelated drop listener |
| `RateSelection` | Resolve control and configured frame modes; fastest mandatory mode | Frame selection; preserve transmitted-frame history |
| `QosRateSelection` | Resolve control and configured frame modes; fastest mandatory mode | Frame selection; preserve per-receiver/frame history |
| `AarfRateControl` | Concrete LOCAL parameters; base invokes `resetRateControl()` | Initial mode / first station estimate; no repeated preparation reset |
| `OnoeRateControl` | Concrete LOCAL parameters; base invokes `resetRateControl()` | Explicit first rate query at NETWORK_CONFIGURATION in the configuration-contract fixture; no repeated preparation reset |
| `OriginatorAckPolicy` | Catalog query | ACK-policy timing during exchange |
| `OriginatorQosAckPolicy` | Catalog query | QoS ACK-policy timing during exchange |
| `RtsPolicy` | Catalog query | RTS decision/timing |
| `QosRtsPolicy` | Catalog query | QoS RTS decision/timing |
| `RecipientAckPolicy` | Catalog query | Response timing |
| `RecipientQosAckPolicy` | Catalog query | QoS response timing |
| `CtsPolicy` | Catalog query | CTS timing |
| `QosCtsPolicy` | Catalog query | QoS CTS timing |
| `TxopProcedure` | Catalog query | TXOP timing; later TXOP state belongs to procedure |
| `SingleProtectionMechanism` | Catalog query | Protection timing |
| `OriginatorProtectionMechanism` | Catalog query | Originator protection timing |
| `OriginatorBlockAckAgreementPolicy` | Catalog query | Agreement policy; agreement lifecycle remains with existing owners |
| `Ieee80211MgmtBase` | Existing `macModule`; idempotent MAC profile preparation and supported-rate construction | First management frame/peer derivation; no repeated reset |
| AP management | PHY channel notification retained; profile + configured operation policy | Explicit `prepareBss()` or LINK_LAYER, before advertisement/peer installation |
| Simplified STA management | Both interface addresses after NETWORK_INTERFACE_CONFIGURATION; both profiles; explicit AP preparation | LINK_LAYER, before NETWORK_CONFIGURATION; no LAST retry |
| Ad hoc management | Prepared local profile | LINK_LAYER/start; no learned HT operation is fabricated |

`FrameSequenceContext` keeps a const catalog pointer supplied by its owner. It does not subscribe to
an initialization event. `opMode`, `mac.modeSet`, radio contribution parameters, and `macModule` paths
retain their forwarding; the new descendant default also covers external-upper composition.

## Acceptance evidence

| Claim | Production path and regression assertion |
|---|---|
| Independent, replaceable contribution assembly | `Ieee80211HtCapabilityPreparation_1`: production MAC queries a non-built-in transmitter wrapper and receiver contribution; widths/GI differ as declared; repeated preparation and installation preserve operation |
| Built-in advertisement/antenna limits | `Ieee80211HtAssociation_1`, `Ieee80211HtAntennaRateControl_1`, serialization/unit cases: real 20 MHz packet PHY and existing advertisement assertions |
| Directional support and bounded selection | Five selected unit cases plus beacon fixture invoking both production selection modules |
| Cache reuse and HT recovery | Beacon fixture: width-only/equal inputs retain shared result; changed GI/MCS refresh; missing HT removes; unsupported Basic MCS retains knowledge; Basic-MCS-only recovery reuses cache |
| Liveness and atomic STA notification | Two synchronous listeners, reversed registration order, inspect identity/band/operation/timer; equal Beacon refreshes deadline without notification; malformed Beacon/probe does not publish authoritative changes; nested clear is rejected |
| AP current operation versus response history | Reassociation snapshot fixture changes radio channel while response is pending; response bytes retain their captured value and MIB uses current channel |
| AP completion/replacement boundary | Same fixture observes pending status at channel change and cleared transaction/AID/peer state at completion; second equal-capability reassociation replaces the cache while retaining the correct AID |
| Scoped cleanup and lifecycle | Detailed AP/STA lifecycle, deauth/disassoc, timeout, queue drop, HCF queue/RTS timeout, malformed HT, discovery and agent reassociation fixtures |
| Early readiness independent of declaration order | Simplified initialization fixture contains both AP-first and STA-first pairs, checks before NETWORK_CONFIGURATION, repeats preparation and checks cache preservation; stop/crash/restart and missing-AP cleanup retained |
| Declared providers/compositions | `Ieee80211ConfigurationContracts_1`: standalone replacement catalog provider, missing-provider diagnostic, HT/legacy ad hoc stop/restart, inherited production external-upper composition with only TAP endpoint replaced before child initialization |
| Real peer sequences | `WifiAssociation`, new `WifiHtAssociation`, `WifiReassociation`, and migrated `WifiDeauth`: protocol-category packet exchange expectations |
| Legacy trajectory preservation | Two selected QoS/non-QoS ad hoc fingerprints, run 0, all three selected ingredients match recorded expectations |

The 40/20 MHz transition fixtures are synthetic management/selection evidence. They do not establish
real 40 MHz packet-PHY support. The external-upper fixture does not claim real TAP I/O. One pinned
run/seed is used for deterministic transitions; no throughput/performance or standards-expansion
claim is made.

## Commands and results

All commands run from the repository root, except fingerprint commands from `tests/fingerprint`.
The active toolchain is OMNeT++ 6.4.0aipre2 / clang with the checkout's enabled features. Debug is the
behavioral test mode. Module/protocol tests use their checked-in bounded configuration, run 0 and
seed 0/default; protocol base explicitly pins `seed-set = 0`.

Exact selectors:

```bash
UNIT='Ieee80211(HtCapabilities_1|HtMgmtElements_1|HtModeSet_1|PeerModeSelection_1|MibAssociationId_1)\.test'
MODULE='Ieee80211(HtAssociation_1|HtAntennaRateControl_1|HtCapabilityPreparation_1|ConfigurationContracts_1|MgmtStaBeaconUpdate_1|MgmtApReassociationSnapshot_1|MgmtStaSimplifiedInitialization_1|MgmtAp(Lifecycle|Timeout|QueueDrop|HcfQueueDrop|HcfRtsTimeout|ChannelChange|GenericRadio|UnavailableChannel|MalformedHtCap)_1|MgmtSta(Lifecycle|Deauthentication|Disassociation|Discovery)_1|AgentStaReassociation_1)\.test'
make -j$(nproc) MODE=debug
inet_run_unit_tests -m debug -f "$UNIT"
inet_run_module_tests -m debug -f "$MODULE"
inet_run_module_tests -m debug -f 'Ieee80211MgmtStaBeaconUpdate_1\.test'
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'Wifi(HtAssociation|Association|Reassociation)\.test$'
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'WifiDeauth\.test$'
make -j$(nproc) MODE=release
# cwd: tests/fingerprint; two configurations, 10s simulated time, run 0
./fingerprinttest -d -m '/examples/adhoc/qos/ .* -c Mac(NonQos|Qos) -r 0 ' -f tplx -f '~tNl' -f '~tND' examples.csv
```

| Run | Executed | Outcome / log |
|---|---:|---|
| Baseline debug build | 1 | PASS, `/tmp/htcapop-baseline-build.log` |
| Baseline existing unit / core / lifecycle-module checks | 4 / 5 / 14 | PASS, `/tmp/htcapop-baseline-*` |
| P1/P2/P3 builds and focused migration checks | See implementation record | Corrected failures described below; `/tmp/htcapop-p1-*`, `p2-*`, `p3-*` |
| Final debug build | 1 | PASS, `/tmp/htcapop-final2-debug.log` |
| Final unit suite | 5 | PASS, `/tmp/htcapop-final2-unit.log` |
| Final module suite | 21 | PASS, `/tmp/htcapop-final2-module.log` |
| Added Basic-MCS-only recovery assertion | 1 existing module case | PASS, `/tmp/htcapop-basic-recovery.log` |
| Final association / HT association / reassociation protocol cases | 3 | PASS, `/tmp/htcapop-final2-protocol.log` |
| Migrated deauthentication protocol case | 1 | PASS, `/tmp/htcapop-deauth-protocol.log` |
| Release build | 1 + final incremental build | PASS (including final rebuilds); `/tmp/htcapop-final-release.log`, `/tmp/htcapop-final2-release.log` |
| Legacy fingerprints | 2 | PASS (initial and final-tree repeat), `/tmp/htcapop-final2-fingerprint.log` |

Baseline unit/module evidence was captured before implementation. The protocol/fingerprint selectors
were finalized and executed later, a sequencing deviation from P0; fingerprint comparison uses the
unchanged repository expectations. An initial fingerprint regex anchored before trailing tags
selected zero cases (`NOT_RUN`); removing that anchor selected exactly the two intended cases.

During development, tests exposed and drove corrections to missing-channel error precedence, AP
restart operation reconstruction, synthetic discovery-frame provenance, and lost fastest-mandatory
rate preparation after signal removal. These are resolved; their failed logs are retained rather
than presented as passes. Generated test build/work files were not used as source edits.

## Mechanical gates and review boundaries

- Scoped architecture gates for linklayer WLAN, PHY WLAN and external-upper WLAN pass.
- Full architecture gate reports 11 application/transport dependency candidates. An archived HEAD
  comparison finds the identical 11; no added/removed candidate.
- Full interface gate reports 15 interface-purity violations. Archived HEAD has the identical 15;
  all four new paired contracts satisfy the mechanical purity check.
- Scoped naming reports 35 declaration candidates in eight files. Each candidate-bearing file is
  byte-identical to HEAD. The project-wide naming gate additionally reports existing resource names.
  Its `--base HEAD` declaration scan selects zero committed changes, so scoped scans are the useful
  evidence for this uncommitted implementation.
- Document seal/index gate passes. The source-seal `--base HEAD` gate selects no committed paths;
  explicit working-tree/untracked-path inspection confirms no change under sealed `common/packet/`.
- Commit/classification checks are not applicable to an uncommitted working tree. The empty-range
  commit checker emits a spurious empty-subject violation; the classification checker selects zero
  commits. Neither is reported as source verification or a validated future commit series.
- C++ quality scan: stable scan completed on 36 changed translation units, with zero compiler errors.
  Ten changed-line redundant-virtual diagnostics were corrected. The final rescan has zero
  diagnostics on changed/new lines (4,079 unique diagnostics elsewhere in included legacy code).
  The compilation database is generated from the
  active debug build's exact compiler options at `/tmp/htcapop-compile-db/compile_commands.json`.
- Semantic self-review covers both selection callers, every old listener, full/simplified/ad hoc
  lifecycle ownership, transaction matching and snapshot provenance. No independent reviewer was
  commissioned, and repository-wide legacy gate violations are not silently waived or changed.

## Semantic self-audit

This is the author's final diff audit, not an independent code-review verdict.

| General checklist item | Result and basis |
|---|---|
| AR-ORG-CONTRACTS | PASS — replacement contributors and standalone catalog provider exercise the declared query outcomes |
| PR-MSG-BODY / PR-MSG-WHY | N/A — no new commits authored |
| AR-ORG-CONTRACT-PURITY | PASS — four role interfaces contain pure operations and trivial destructors only |
| AR-ORG-VIS-SPLIT | PASS — model state remains in MIB; visualizer changes are const-reader migration |
| AR-ORG-KERNEL | PASS — adds an edge to the existing INET initialization-stage graph |
| AR-MOD-COMPOSITION | PASS — provider interfaces use existing modules; shared initialization base contains only dependency acquisition |
| AR-COM-SOCKETS | N/A — no application/transport integration |
| AR-COM-DIRECT | PASS — readiness uses direct typed calls; no zero-time handshake event |
| AR-COM-NOTIFY | PASS — committed state notification after timers/transactions, no required subscriber |
| AR-OBS-SIGNALS | PASS — synchronous observation and rejected nested state mutation tested in both registration orders |
| AR-OBS-NED-TRUTH | PASS — dependency defaults and signal declaration live in NED; documentation adds readiness/lifetime semantics |
| AR-OBS-INTROSPECTION | N/A — no new wire protocol or packet representation |
| AR-CFG-INFER / QR-DUP | PASS — remove copied operation and derive eligibility from accepted state |
| QR-OBJECT-OWNERSHIP | PASS — immutable shared cache, retained test snapshots, existing timer/packet cleanup paths |
| AR-CFG-PARAMS | PASS — new provider paths default empty and are mandatory dependencies, filled by composition |
| AR-EXT-NOCORE | N/A — no new protocol registration; common edits remove a signal and declare readiness |
| AR-EXT-MINIMAL-SURFACE | PASS — state private; public queries/preparation/commit operations serve production roles; old eligible-peer query retained as a const compatibility query |
| AR-EXT-VIRTUAL-IS-A-PROMISE | PASS — provider roles, framework hooks, and AP/ad hoc operation template step |
| AR-BUILD-DECLARATIVE | PASS — no machine paths or flags added to production build descriptors |
| RR-NUMERIC-STABLE | N/A — no enum/numeric wire code changed |
| AR-QUAL-NAMING | PASS — new role/type/parameter names follow role and camelCase conventions; existing findings separately identified |
| AR-QUAL-LOGGING | PASS — invalid preparation/dependency/mutation throws; no logged invariant failure proceeds |
| AR-QUAL-DETERMINISM | PASS — semantic equality and existing deterministic mode tie-breaks; pointer identity used only for test cache observation |
| AR-QUAL-TESTS | PASS — focused unit, module, protocol and legacy fingerprint evidence |
| AR-QUAL-TRACEABILITY | N/A — no recorded fingerprint/statistical baseline changed |
| AR-QUAL-DISPLAY | N/A — no new concrete production NED module; existing MIB inspection retained |

| WLAN checklist item | Result and basis |
|---|---|
| AR-WLAN-STD-TRACE | PASS — existing normative references retained; no-air/ad hoc behavior explicitly identified as abstraction |
| AR-WLAN-STD-GATING | PASS — local profile, accepted capabilities, operation and eligibility remain separate gates |
| AR-WLAN-ARCH-BOUNDARIES | PASS — capability assembly uses role contracts; simplified STA calls AP owner operations |
| AR-WLAN-ARCH-OWNERSHIP | PASS — current BSS operation has one writer; discovery/pending snapshots retain their historical purpose |
| AR-WLAN-ARCH-VARIANTS | PASS — AP/ad hoc operation uses existing management roles |
| AR-WLAN-FRAME-REPRESENTATION | N/A — no new on-air fields/tags or serializer changes |
| AR-WLAN-PHY-AUTHORITY / AR-WLAN-PHY-TIMING | PASS — band/mode APIs remain authority for legality and timing |
| AR-WLAN-MAC-EXCHANGE | PASS — no new exchange matcher; management continues receiving existing transmission outcomes |
| AR-WLAN-MAC-SEQUENCE | N/A — no sequence/window arithmetic change |
| AR-WLAN-MAC-QOS | PASS — existing EDCA state and callback timing preparation retained |
| AR-WLAN-MAC-MULTIUSER | N/A — no MU feature work |
| AR-WLAN-OBS-EVENTS | PASS — equal information does not republish state; protocol outcomes retain their distinct signal |
| AR-WLAN-QUAL-TESTS | PASS — acceptance matrix above, including legacy fingerprints |

REVIEW: 29 PASS, 10 N/A, 0 FLAG, 0 QUESTION.
Mechanical gate failures and test boundaries are recorded separately above.

## Final review corrections

- `prepareBss()` rejects calls while AP management is down; AP lifecycle coverage asserts that
  preparation cannot resurrect cleared state. Both it and simplified initialization pass after the
  guard (`/tmp/htcapop-inactive-ap.log`).
- New catalog-path NED parameters have explicit empty defaults; standard composition supplies the
  mandatory path, and standalone missing-provider behavior remains an error. New contracts/base
  follow the checker's namespace, guard, trivial-destructor and override conventions. A transient
  namespace edit failed compilation and was fixed before the final builds.
- Fresh final debug/release builds pass (`/tmp/htcapop-final5-debug.log`,
  `/tmp/htcapop-final5-release.log`). Three provider/contribution/composition cases pass afterward
  (`/tmp/htcapop-provider-defaults.log`). Final-tree legacy fingerprints pass, two cases / three
  ingredients each (`/tmp/htcapop-final2-fingerprint.log`).
- The long whole-WLAN C++ scan overlapped header edits and is invalid as final evidence. The final
  stable-source scan uses the same compilation database/check configuration on every changed `.cc`
  in six parallel processes, preserving one log per translation unit under `/tmp/htcapop-tidy-final`.
  A focused new-provider/base scan reports no diagnostic in those new files
  (`/tmp/htcapop-cpp-new-provider.log`; included legacy headers still generate suppressed warnings).

## Completion record

Final debug and release builds: PASS, `/tmp/htcapop-final-debug-build.log` and
`/tmp/htcapop-final-release-build.log`. Both Aarf and Onoe focused cases pass after the final
header/default cleanup (`/tmp/htcapop-final-ratecontrol.log`). The final stable C++ scan covers
36 changed translation units: zero tool/compiler errors, zero diagnostics on changed/new lines;
4,079 unique diagnostics remain elsewhere in included legacy code. These are not represented as a
clean repository-wide lint result. See `/tmp/htcapop-tidy-final-summary.log` and
`/tmp/htcapop-tidy-final-diff.log`. All new contracts/base files are included in the changed-line
filter regardless of line number.

The full final 21-module / 5-unit / 4-protocol runs precede only the explicitly recorded inactive-AP
guard and declaration/default cleanup; affected lifecycle, composition, contribution, and rate-control
cases were rerun afterward. The guard does not enter the ad hoc fingerprint paths. Final fingerprint
and code-style changes do not modify recorded expectations. `git diff --check` is clean.

Logs and the per-translation-unit C++ results are preserved locally under
`report/htcapop-v3/logs/` in addition to their original `/tmp` paths. This evidence directory follows
the checkout's existing ignored-report convention; the plan/evidence Markdown files are reviewable
new files under `plan/done/`. No commit or push was performed.

Source/test manifest SHA-256: `7cd8705f802b9f599c76ad696cef7daffdc5b57e2a2a0a874c7c7d58ce129375` (130 paths, including deletions).
