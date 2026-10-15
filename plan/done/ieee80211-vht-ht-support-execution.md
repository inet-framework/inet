# Execution record: HT support in VHT stations

The local implementation and planned verification pass. Three local commits preserve the planned boundaries. Git access now permits their creation.

The user authorized implementation with “Execute plan ieee80211-vht-ht-support.md” on 13 October 2026. The implementation follows plan revision 10. The base commit is `a61363aba5e9492f0c98a24905e050f0cf36966a`.

The evidence below describes that initial execution and its three commits. Publication preparation replays them on upstream commit `0a29fb621e0db33471959666920792af1bff01a4`. The replay retains both sets of release and migration notes. All 45 other implementation files retain their exact contents. The PR description records the later publication checks.

The active checkout is `/home/user/omnetpp_ws/inet-ieee80211-vht-ht-support`. The source changes exclude the sealed packet subsystem. The original `AGENTS.md` edit remains separate from this work.

## Result and mechanism

The [mode catalog](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211ModeSet.cc) now includes all 128 existing HT combinations for the 5 GHz band in `ac`. Its HT flag activates the existing radio capability, management, and peer-selection paths. Existing VHT and legacy modes retain their order, flags, and reference mode.

The [mapped band](../../src/inet/physicallayer/wireless/ieee80211/mode/Ieee80211Band.cc) converts internal indices 0–3 to standard channels 36, 40, 44, and 48. Those channels use centers 5180, 5200, 5220, and 5240 MHz. Its HT40 validation accepts 36/40 and 44/48 in either direction. It rejects 40/44.

The [QoS selector](../../src/inet/linklayer/ieee80211/mac/rateselection/QosRateSelection.cc) and [DCF selector](../../src/inet/linklayer/ieee80211/mac/rateselection/RateSelection.cc) now require an explicit caller role. Sender prediction uses the receiver address and local reception capability for the peer's transmission. Actual response selection uses the transmitter address and local transmission capability for the peer's reception. All production callers and affected test overrides supply the role.

The shared `ac` protocol configuration, example, and 160 MHz test now use mapped index 0 at 5180 MHz. [Migration guidance](../../doc/src/migration-guide/index.rst) states the source compatibility break and changed frequencies. [User documentation](../../doc/src/users-guide/ch-80211.rst) explains catalog traversal, equal-rate adaptation steps, concrete radio requirements, and retained limits.

For example, associated `ac` stations send VHT data with two streams and short GI at approximately 14.444 Mbps. Compatible HT state permits a 13 Mbps HT ACK. The sender now predicts that actual HT ACK and reserves its duration. The network fixture establishes successful acknowledgment without a retry. This result requires the exact-prediction conditions stated below.

## Prerequisite results

| Check | Result | Evidence |
| --- | --- | --- |
| Debug build | PASS, exit 0 | `/tmp/vht-ht-execution/prerequisite-build.log` |
| Debug freshness check | PASS, exit 0 | `/tmp/vht-ht-execution/prerequisite-freshness.log` |
| Unit fixtures | 10 PASS, exit 0 | `/tmp/vht-ht-execution/prerequisite-unit.log` |
| Module fixtures | 7 PASS, exit 0 | `/tmp/vht-ht-execution/prerequisite-module.log` |
| MAC architecture | PASS, exit 0 | `/tmp/vht-ht-execution/architecture-mac.log` |
| PHY architecture | PASS, exit 0 | `/tmp/vht-ht-execution/architecture-phy.log` |
| Selected fingerprints | 2 PASS, exit 0 | `/tmp/vht-ht-execution/prerequisite-fingerprint.log` |

The unit selection includes the existing `n` catalog and the new production response queries. At this prerequisite stage, `ac` still lacks HT. The queries check both selectors, capability direction, unknown Tx advertisements, absent peers, and unset transmitter addresses for outgoing RTS and BAR inputs.

The module selection includes association, antenna rate control, discovery, and the existing HCF protection controls. Serialized discovery accepts 36/40 and 44/48 in both directions. It rejects 40/44 in both directions.

The fingerprint controls use `showcases/wireless/txop`, `General`, run 0, for 5 seconds. They also use `examples/wireless/lan80211ac`, `Ping1`, run 0, for 100 seconds. The ingredients are `tplx` and `~tNl`. Both controls retain their base fingerprints at the prerequisite and activation stages. No recorded expectation changes.

The comparison checkout `/home/user/omnetpp_ws/inet` has the same base commit and no source edits. Its debug build passes. Its selected simulations use the same relevant features. Its feature profile also enables unused emulation, OSG, and other optional components. Both base controls match their recorded expectations.

The first fingerprint command failed because `.envrc` used the test subdirectory as `INET_ROOT`. The corrected command loads `.envrc` from the repository root. That corrected command passes. The unit runner also reports a make jobserver FIFO warning; all ten unit fixtures execute and pass.

## Final verification

| Check | Result | Evidence |
| --- | --- | --- |
| Fresh debug build | PASS, exit 0 | `/tmp/vht-ht-execution/activation-build.log` |
| Unit fixtures | 11 PASS, exit 0 | `/tmp/vht-ht-execution/final-unit.log` |
| Existing module fixtures | 7 PASS | `/tmp/vht-ht-execution/final-module.log` |
| New module fixture | All 7 scenarios PASS, exit 0 | `/tmp/vht-ht-execution/module-newfixture-rerun7.log` |
| Identical new-fixture repeat | All 7 scenarios PASS, exit 0 | `/tmp/vht-ht-execution/module-newfixture-repeat.log` |
| Protocol controls | 11 PASS; 1 expected FAIL; exit 0 | `/tmp/vht-ht-execution/final-protocol.log` |
| Example initialization | PASS, exit 0 | `/tmp/vht-ht-execution/example-initialization.log` |
| MAC architecture | PASS, exit 0 | `/tmp/vht-ht-execution/final-architecture-mac.log` |
| PHY architecture | PASS, exit 0 | `/tmp/vht-ht-execution/final-architecture-phy.log` |
| Selected fingerprints | 2 PASS, exit 0; expectations unchanged | `/tmp/vht-ht-execution/activation-fingerprint.log` |
| Whitespace check | PASS | `git diff --check` |

The final unit selection checks catalog parity, capabilities, management elements, response direction, and production peer filters. Capability tests cover antenna counts 1–4 and 8. Candidate sweeps check HT widths through both selectors and rate control. The existing antenna module fixture sees a raw 40 MHz candidate, but every actual HT data transmission uses 20 MHz.

The [new fixture](../../tests/module/Ieee80211VhtHtSupport_1.test) uses scenario values 0–6 with seed 0. Its seven scenarios establish these results:

1. Full management exchanges HT elements, commits HT peer state, and delivers HT data.
2. Simplified management supplies HT state, delivers HT data, and removes peer state during teardown.
3. A two-stream HT request to a one-stream peer becomes one-stream HT MCS 1 at 13 Mbps and reaches delivery.
4. A 20 MHz VHT transmission reaches delivery.
5. The default HT ACK matches prediction, reservation, and successful acknowledgment.
6. The default HT CTS matches prediction and RTS Duration/ID. The exchange completes without a retry.
7. HCF refuses a queued successor at the measured TXOP boundary. A new grant sends the successor without a retry.

The initial final-module selection reports seven existing passes and one new-fixture failure. The new fixture contained observation and control errors. Its observer used packet offsets without the current front offset. Its selector override did not replace the fixed NED submodule. Later corrections also cover ordinary DATA headers, management forwarding, RTS scope, and the queue that owns the TXOP limit.

The corrected fixture delegates queries to the production selector and checks actual PHY tuples. Its focused rerun and identical repeat both pass. The seven existing fixtures require no later source change.

### TXOP boundary evidence

Scenario 6 sets a 304 μs TXOP for the sender's video access category. It isolates the TXOP limit through the plan's explicit TXNAV control. The data frame takes 84 μs. The actual 13 Mbps HT ACK takes 48 μs. One exchange therefore costs 148 μs with its 16 μs SIFS. The retained legacy estimate costs 132 μs.

After the first ACK, 156 μs remains. The successor also needs its initial SIFS. Its HT cost is 164 μs, while the incorrect legacy cost is 148 μs. HCF refuses continuation at 40.148 ms. A new grant starts the successor at 40.182 ms. Both actual ACKs use HT at 13 Mbps, and neither data frame requires a retry.

The raw observations are in `/tmp/vht-ht-execution/module-newfixture-rerun7.stdout`, lines 143–146. The identical repeat produces the same result. These assertions establish the current model's TXOP decision, not standards compliance of the retained HT ACK format.

### Expected protocol failure and channel evidence

`Ac_VhtIe` misses its step-0 deadline for a beacon with element ID 192, the VHT Operation element. The failure occurs after 2.5 seconds. The AP emits a valid 103-byte beacon with HT Capabilities and HT Operation at 0.038179866314 seconds. The PHY transmits that beacon, and the stations receive it. The missing VHT Operation element remains the cause; initialization succeeds.

The direct failure appears in `tests/protocol/wifi/work/Ac_VhtIe/test.out`, lines 981820–981821. The predicate uses `hasManagementElement()` from `tests/protocol/wifi/WifiTestSupport.h`. This expected failure does not establish VHT negotiation support.

The example initialization log shows mapped index 0 at 5.18 GHz for all three radios. The `Ac_PhyMode` and `Ac_160mhz` protocol logs show the same effective band and center. These observations cover all three migrated configuration sources. The wider VHT tests retain their existing MAC-output assertions; they do not establish wider PHY delivery.

## Independent review

The independent reviewer reports no actionable defect in the final source, configuration, tests, WHATSNEW, migration guide, or user guide. The review closes the earlier transmitter-address, RTS tuple, and HT guard-interval coverage gaps. It inspects the recorded results without another test run. The general checklist has 15 PASS and 19 N/A results. The WLAN checklist has 9 PASS and 4 N/A results. Neither checklist has a flag or question.

The review excludes the user's `AGENTS.md` edit and this execution record. No reviewed source path is sealed. No new architecture or naming exception requires a ledger entry. The scope supplies debug evidence for the selected configurations and seed. At review, release compilation, external selector implementations, and broader feature combinations remained unverified. The review precedes creation of the actual commit series.

Scenario 6 uses zero propagation and an explicit test control for TXNAV during admission. It proves the selected TXOP-limit decision. It does not validate TXNAV updates or behavior with nonzero propagation.

## Commands

Each command below starts at the active repository root. The comparison build and base fingerprints use `/home/user/omnetpp_ws/inet` instead.

```sh
source .envrc
make -j$(nproc) MODE=debug
bin/inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet|Band|HtCapabilities|HtMgmtElements|PeerModeSelection|ResponseModeSelection|SupportedRates|NonHtReferenceRate|MibPeerTeardown|ResponseTimeout)_1\.test'
bin/inet_run_module_tests -m debug -f 'Ieee80211(HtAssociation|HtAntennaRateControl|MgmtStaDiscovery|HcfMultipleProtection|HcfSingleProtection|SingleProtectionGroup|Txnav)_1\.test'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee80211
cd tests/fingerprint
./fingerprinttest -d -m '(showcases/wireless/txop/|examples/wireless/lan80211ac/)' -f 'tplx' -f '~tNl' -f '~tND'
```

The actual runner commands also select log files under `/tmp/vht-ht-execution`. Unit and module commands set `MPLCONFIGDIR` to that directory's `mplconfig` subdirectory. Later commands set `MAKEFLAGS=--jobserver-style=pipe` to avoid the shared FIFO warning. The build mode is debug throughout.

Final selections use these commands from the repository root:

```sh
source .envrc
export MPLCONFIGDIR=/tmp/vht-ht-execution/mplconfig
export MAKEFLAGS=--jobserver-style=pipe
make -j$(nproc) MODE=debug
bin/inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet|Band|HtCapabilities|HtMgmtElements|PeerModeSelection|ResponseModeSelection|VhtHtCapabilities|SupportedRates|NonHtReferenceRate|MibPeerTeardown|ResponseTimeout)_1\.test'
bin/inet_run_module_tests -m debug -f 'Ieee80211(HtAssociation|HtAntennaRateControl|MgmtStaDiscovery|HcfMultipleProtection|HcfSingleProtection|SingleProtectionGroup|Txnav|VhtHtSupport)_1\.test'
bin/inet_run_module_tests -m debug -f 'Ieee80211VhtHtSupport_1.test'
bin/inet_run_protocol_tests -p inet -m debug -w wifi -f '(11ac/Ac_(PhyMode|VhtIe|BlockAck|256qam|4ss|160mhz)|common/Wifi(Association|AuthOpen|Beacon|BeaconInterval|DataDelivery|Probe))\.test'
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211
doc/project/enforcement/check-architecture.sh src/inet/physicallayer/wireless/ieee80211
git diff --check
```

The fingerprint command above runs again after activation. Example initialization uses this command after `.envrc` loads at the root:

```sh
cd examples/wireless/lan80211ac
inet --debug -u Cmdenv -c Ping1 -r 0 --sim-time-limit=0s --cmdenv-express-mode=false --cmdenv-log-level=trace
```

## Commit series and patches

During initial execution, Git refuses creation of this worktree's `index.lock` because its metadata directory is read-only. The user later changes the permissions and requests the planned commits. Git then accepts the three commits on `feat/ieee80211-vht-ht-support`. No publication occurs.

The commit subjects describe these boundaries:

1. `ieee80211: add: map 5 GHz channels for HT management` adds the band prerequisite and approved plan.
2. `ieee80211: fix: predict HT responses for the correct peer` adds the response prerequisite with its direct checks and migration guidance.
3. `ieee80211: add: enable existing HT support in ac` adds HT activation, configuration migration, and final evidence documents.

Separate patches preserve the intended boundaries:

1. [01-mapped-band.patch](/tmp/vht-ht-execution/01-mapped-band.patch) adds the band, declarations, discovery checks, and compatibility documentation.
2. [02-response-prediction.patch](/tmp/vht-ht-execution/02-response-prediction.patch) adds the role contract, production consumers, response checks, and migration documentation.
3. [03-vht-ht-activation.patch](/tmp/vht-ht-execution/03-vht-ht-activation.patch) adds the HT catalog, configuration migration, activation tests, and user documentation.

All three patches pass sequential application checks against an isolated checkout of the base. After application, all 47 implementation files match the active checkout byte for byte. The patches exclude `AGENTS.md` and the plan documents. The commits use those same patches and include the plan documents described above.

The initial second commit's 36 prerequisite files match the saved prerequisite contents. The initial final commit's 47 implementation files match the tested final contents. Initial commit creation changes no implementation file. The prior debug evidence therefore applies to those same source contents. The first commit has no separate build or simulation result.

The original tip remains at `refs/backup/ieee80211-vht-ht-support-before-upstream-20261013`. The replay changes only release-note numbers and document placement within the implementation surface. It preserves the three commit subjects and boundaries. The source changes require no adaptation to the new upstream base.

The [file inventory](/tmp/vht-ht-execution/implementation-files.txt) lists those 47 files. The [SHA-256 manifest](/tmp/vht-ht-execution/implementation-files.sha256) identifies their final contents. The original `AGENTS.md` SHA-256 remains `4405182693b0fa0b0b5c41b8d7c9ff59eb5014e433f3b133ccbed1512a939a82`.

## Retained limits

The implementation retains advertised and negotiated HT support at 20 MHz. It adds no VHT negotiation, Compressed Block Ack, or primary/secondary PHY model. It retains the known short-GI duration defect and control-response format violations.

Exact response agreement requires identical catalogs and response configuration, consistent peer state, known equal Tx/Rx advertisements, and stable capabilities. Unknown responder Tx capability can produce a legacy prediction and an actual HT response. That legacy prediction is not a proved duration bound.
