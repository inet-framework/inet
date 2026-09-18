# HT/GI Devin comment closure

Status: completed, with the pre-existing full-interface gate findings recorded below.

Base: `dffd66845707303f661ef148e9bca2c3cca94262`. Scope: the five comments assessed in this session.

## Implementation contract

- Invariant and owner: transmitter compatibility rejection must precede the radio/MAC transaction.
  The transmitter owns exact-tuple resolution; radio coordinates updates. Failures after PHY mutation
  remain fatal. A rejected preflight changes no catalog, current mode, peer cache or notification count.
- Entry/control path: both radio setters call `changeModeSet`; catalog-only changes preflight a const
  transmitter query before `beginModeSetChange`. The transmitter setter reuses the same query.
  Explicit changes keep their existing membership check and virtual setter dispatch.
- Affected artifacts: radio/transmitter C++ and transmitter declaration; management serializer masks;
  peer-mode selector citations; `IIeee80211Mode` and existing `Ieee80211ModeBase`; transition module
  and VHT management-element unit fixtures. HT/VHT duration overrides remain authoritative.
- Siblings/terminal paths: same/null catalogs retain existing transmitter semantics; explicit mode
  changes and post-mutation participant failures retain their contract. Serializer read/write masks
  stay symmetric. No lifecycle or packet ownership change.
- Boundaries: exact bitrate/bandwidth/NSS/GI tuple; null mode/catalog behavior unchanged. Duration
  units and arithmetic unchanged. VHT subtype placement remains identical to HT, with distinct bits;
  this does not add band/capability presence validation.
- Verification: debug and release library builds; filtered transition/failure/registration and VHT
  association modules; HT/GI/VHT mode, management-element and peer-selection units; the earlier
  capability/BSS evidence's focused units/modules/protocols and two legacy ad hoc fingerprints.
  Extend the transition fixture with rejection followed by successful explicit retry, and VHT codec
  coverage for request-operation rejection and independent HT/VHT presence.

Self-validation: owners, callers, base-class implementations, fixture names and runner interfaces
checked before editing. No affected source path is sealed. One deterministic run/seed is sufficient
for these API and codec invariants. No baseline update, commit, or push is part of this task.

## Standards

IEEE Std 802.11-2024: 10.6.13.1/.2 (directional VHT MCS/NSS and long-GI rate limits),
10.17 (short GI), 9.4.2.156.2 and Table 9-313 (width capability), 11.38.1 (BSS width),
Table 9-315 (rate field encoding). Subtype permissions: 9.3.3.2 and 9.3.3.5–10,
Tables 9-62 and 9-64–69. Retrieved from the local standards corpus; no normative behavior changes
are intended by adding citations. The selector's rate ceiling and fallback are model policy.

## Evidence

Date: 2026-09-19. Tested tree: base commit above plus the source/test patch with SHA-256
`cd580f78b349b24d84077b135a674591f2d3f01d6a6c518116c0206a277753b3`.
The patch and per-file hashes are retained in `report/ht-gi-devin-closure/source.patch` and
`source-manifest.json`. These identify an uncommitted tree, not an unchanged HEAD.
The historical V3 evidence remains historical.

All commands ran at the checkout root except the fingerprint wrapper, run in `tests/fingerprint`.
OMNeT++ 6.4.0aipre2 / clang; debug libraries and runtime assertions for behavioral tests.
Module/protocol fixtures use their checked-in deterministic configuration, run 0 and seed 0/default;
the failure fixture deliberately exercises its 14 configuration combinations and asserts fatal outcomes.
Fingerprints use run 0, seed 0/default, and 10 seconds simulated time.

| Gate | Outcome | Log under `/tmp/` (also archived in `report/ht-gi-devin-closure/logs/`) |
|---|---|---|
| `make -j12 MODE=debug` | PASS, exit 0; final incremental build also exit 0 | `ht-gi-closure-debug.log`, `ht-gi-closure-debug-final.log` |
| `make -j12 MODE=release` | PASS, exit 0 | `ht-gi-closure-release.log` |
| Focused units | Initial 8 PASS / 1 FAIL, exit 1; corrected VHT fixture rerun PASS, exit 0; all 9 selected cases now pass | `ht-gi-closure-unit.log`, `ht-gi-closure-vht-codec-retest.log` |
| Focused modules | Initial 27 PASS / 1 FAIL, exit 1; corrected stale discovery assertion rerun PASS, exit 0; all 28 selected cases now pass | `ht-gi-closure-module.log`, `ht-gi-closure-discovery-retest.log` |
| Focused protocols | 4 PASS, exit 0; no declared expected failures | `ht-gi-closure-protocol.log` |
| Legacy fingerprints | 2 PASS, all three ingredients match, exit 0 | `ht-gi-closure-fingerprint.log` |
| Scoped MAC architecture | PASS, exit 0 | `ht-gi-closure-architecture-mac.log` |
| Scoped PHY architecture | PASS, exit 0 | `ht-gi-closure-architecture-phy.log` |
| Mode interfaces | 5 PASS, exit 0 | `ht-gi-closure-mode-interfaces.log` |
| Full interfaces | FAIL, exit 1: 15 violations in unchanged files; not waived | `ht-gi-closure-interfaces.log` |
| `git diff --check` | PASS, exit 0 | No output |

The first VHT fixture failed with `Invalid VHT MCS map entry 0`: generated wire-element defaults
are not legal VHT MCS map entries. The fixture now copies the valid reference elements already
used by the byte-encoding checks. The production codec required no further change.
The first discovery fixture failed at `dcfMulticastMode->getHtMcsIndex() >= 0`; see the scoped
correction below. Only those two failed cases were rerun after test-only corrections; the INET
library and other cases remained unchanged. No recorded fingerprint, statistical or stdout
expectation was rewritten.

Exact selectors and commands:

```bash
UNIT='Ieee80211(HtCapabilities_1|HtMgmtElements_1|HtModeSet_1|PeerModeSelection_1|MibAssociationId_1|HtGuardInterval_1|VhtModeSet_1|VhtMgmtElements_1|VhtPeerModeSelection_1)\.test$'
MODULE='Ieee80211(HtAssociation_1|HtAntennaRateControl_1|HtCapabilityPreparation_1|ConfigurationContracts_1|MgmtStaBeaconUpdate_1|MgmtApReassociationSnapshot_1|MgmtStaSimplifiedInitialization_1|MgmtAp(Lifecycle|Timeout|QueueDrop|HcfQueueDrop|HcfRtsTimeout|ChannelChange|GenericRadio|UnavailableChannel|MalformedHtCap)_1|MgmtSta(Lifecycle|Deauthentication|Disassociation|Discovery)_1|AgentStaReassociation_1|ModeSet(Transition|Failure|Registration|Retry)_1|ContentionModeSet_1|TxopModeSet_1|VhtAssociation_1)\.test$'
inet_run_unit_tests -m debug -f "$UNIT"
inet_run_module_tests -m debug -f "$MODULE"
inet_run_unit_tests -m debug -f 'Ieee80211VhtMgmtElements_1\.test$'
inet_run_module_tests -m debug -f 'Ieee80211MgmtStaDiscovery_1\.test$'
inet_run_protocol_tests -p inet -m debug -w '^tests/protocol/wifi$' -f 'Wifi(HtAssociation|Association|Reassociation|Deauth)\.test$'
# cwd: tests/fingerprint
./fingerprinttest -d -m '/examples/adhoc/qos/ .* -c Mac(NonQos|Qos) -r 0 ' -f tplx -f '~tNl' -f '~tND' examples.csv
# cwd: repository root
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee80211
doc/project/enforcement/check-interfaces.sh src/inet/physicallayer/wireless/ieee80211/mode
doc/project/enforcement/check-interfaces.sh
```

Python runners used `MPLCONFIGDIR=/tmp/ht-gi-closure-matplotlib` after initial setup to avoid an
unwritable user cache directory. The optional missing `py4j` IDE integration did not prevent tests.

## Self-audit and limits

- Radio preflight is read-only and occurs before either guard. The transmitter setter uses the same
  resolution logic, retaining same/null-catalog behavior and virtual setter dispatch. Post-mutation
  failures remain fatal; the existing failure fixture passes all its expected-error checks.
- The transition fixture checks both PHY catalogs, registered consumers, the selected transmit mode,
  peer-cache identity and notification counts after repeated rejection, then checks a successful
  explicit retry. It uses a 40 MHz catalog mode without claiming real 40 MHz packet-PHY support.
- VHT serializer read/write permissions are distinct and symmetric. All seven subtype cases cover
  independent HT/VHT capabilities presence; requests reject VHT Operation on encoding and decoding.
  Band/capability/role advertisement validation is outside this subtype-mask change.
- The interface declares pure duration operations; the existing base provides identical defaults.
  HT/VHT overrides retain their timing authority. Existing legacy `_get*` wrappers retain AS-03.
- Reviewed C++ dispatch/query purity, OMNeT++ synchronous notification and reentrancy, INET ownership
  and codec paths, and WLAN mode identity and standards traceability. No new exception or sealed-path
  edit. No packet-core change, baseline update, commit, or push.
- This is focused verification and author self-audit, not an independent review or a clean
  repository-wide compliance verdict. The full interface gate still has 15 pre-existing findings in 14 headers, all byte-identical to
  HEAD (`ht-gi-closure-interface-provenance.log`).


### Required scope adjustment found by verification

The 28-module run found one stale assertion in `Ieee80211MgmtStaDiscovery_1.test`: it still
requires HT group transmission, predating `ea818d0631` (legacy basic group rates). The source path
selects the fastest mandatory legacy mode for this fixture, 24 Mbps. IEEE 802.11-2024 10.6.5.4
requires non-HT transmission from the nonempty basic legacy set. Update those two assertions and
their explanatory comment, retaining unicast checks and recorded stdout. This is a test-code
correction, not regeneration of fingerprint/statistical/output baselines. No source seal applies.
Revalidate with the same discovery module only; production code is unchanged by this adjustment.

## Commit preparation

On the user's subsequent commit request, the verified changes were divided by decision into
new commits. Release and migration notes were added for preflight retry behavior and the pure
duration queries; these documentation-only additions do not change the tested source/test patch.
The source/test patch identity above remains relative to the pinned base, across the entire series.
