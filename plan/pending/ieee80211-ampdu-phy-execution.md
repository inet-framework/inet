# PHY P0 through P4 execution record

The user authorizes P0, P1, and P2 on `4fa5cc0de15ee57388bcb3f20e866cab5972a670` before PRs #1343, #1344, and #1345 merge. This exception replaces the merge prerequisite for this execution only. The user also authorizes commits and a branch push. No baseline change forms part of this task.

The active checkout is `/home/user/omnetpp_ws/inet-ieee80211-ampdu-implicit-blockack`, on branch `work/ieee80211-ampdu-implicit-blockack`. The implementation uses the approved base. The commit series separates the plans, P1 representation, and P2 reception.

P0 through P4 are complete on 2026-10-17. The implementation supplies HT-SIG bytes, validation, bounded ranges, and cached partial-reception outcomes. P3 verifies NIST/Yans probabilities and seeded outcomes. P4 closes ordinary reception comparisons with the pinned parent. D1 and D2 are complete for the declared PHY profile. MAC acceptance and implicit Block Ack remain pending.

## Implementation contract

The radio owns HT-SIG fields and CRC construction. Its serializer preserves all 48 bits, including the received CRC. A shared validator rejects invalid CRC and unsupported fields before any range calculation. The existing mode and PSDU length remain the duration inputs.

The shared delimiter inspector owns bounded range discovery. It checks delimiter bytes and advances at each scan. Delimiters and complete MPDUs have separate ranges. Alignment padding has no range outcome. MAC acceptance remains outside P1 and P2.

The WLAN receiver owns PHY outcomes. The medium cache owns the PREAMBLE, HEADER, DATA, and WHOLE decisions. Direct success queries reuse those decisions. DATA retains copied range values; the result tag copies those values without reception pointers. A partial DATA failure preserves the aggregate bytes.

The optional WLAN error-model contract supplies pure support queries and range probabilities. The shared WLAN base implements this contract for NIST and Yans. The MAC supplies its exact capability inputs before the MIB update. An absent model retains the current common PHY checks without error draws.

The proposed wire fixture checks independent bytes, CRC, bounded ranges, and singleton identity. The proposed receiver fixture checks actual radio reception, range loss, query reuse, failure boundaries, metadata, and compatibility. Existing HT/VHT controls check ordinary reception. Debug compilation precedes behavioral checks. Scoped architecture checks cover both WLAN subtrees.

The contract matches the inspected callers and owners. Source edits exclude the sealed `src/inet/common/packet/` subtree. Required input changes include MSG fields, serializer registration consumers, NED contracts, the MAC capability handoff, and migration text.

## P0 source audit

The cache key contains receiver, transmission, and signal part. `RadioMedium::getReceptionDecision()` inserts a decision after a cache miss. `CommunicationCacheBase::setCachedReceptionDecision()` deletes a replaced decision. `ReceptionCacheEntry` deletes all retained decisions at destruction. `receivePacket()` removes and deletes the result but retains the decisions. The result owns its packet and decision-vector container; it does not own the decisions.

The direct `RadioMedium::isReceptionSuccessful()` call bypasses the decision lookup. The WLAN override must route aggregate queries through that lookup. PREAMBLE cannot calculate HEADER or DATA. Separate-part radio reception aborts after PREAMBLE or HEADER failure. Whole-signal result construction requests WHOLE.

The HT header mode declares 48 bits. The transmitter derives duration from the selected mode and `lengthField`. The empty parent serializer does not alter these duration inputs.

The metadata audit found three error-rate result filters. Each filter forwards present values without arithmetic. The radio attaches histogram recorders. OMNeT++ `StatisticsRecorder::collect()` ignores `NaN` for unweighted statistics. The receiver fixture must exercise these configured consumers before D1 closes.

The direct reader in `L3NetworkConfiguratorBase` constructs an ordinary MTU-sized probe. It calls radio encapsulation without an aggregate request or delimiter content. Thus, it does not consume aggregate probability metadata. Ordinary metadata retains its current calculation. Other source matches either produce tags or concern unrelated error-model interfaces.

BerTable's NED enum rejects the radio's `n(mixed-2.4Ghz)` string before initialization. An isolated BerTable with the allowed `n` value reaches its initializer, which also rejects HT. Both failures precede the new role check. Its packet-level PER calculation also dereferences the transmission bit model. The parent scalar control initializes with a valid table, then crashes at that null access. The feature library produces the same crash. This result establishes a prior limitation, not supported BerTable reception. The inherited receiver stage count covers the physical-layer check.

## Evidence

The existing HT/VHT module control exposed a required compatibility correction. `Ieee80211VhtSignalMode::createHeader()` returns an HT header in the parent. HT field construction therefore receives VHT's absent HT MCS value and refuses it. The VHT mode must create its existing VHT header type. This change preserves VHT duration and probability calculations and adds no VHT serializer implementation. The affected mode header is unsealed. The existing HT/VHT control verifies this required extension of the change surface.

The wire fixture verifies golden bytes from IEEE Std 802.11-2024, Figure 19-6 and Clause 19.3.9.4.4. A polynomial long-division calculation supplied the fixed CRC constants independently of the production shift register. Fresh byte-backed packets establish decode without the original typed header. Cases cover standalone and singleton identity, short GI, invalid CRC, unsupported fields, incomplete headers, and improper representation. HT Length 65535 and MCS 31 survive conversion; length 65536 causes refusal.

Delimiter cases cover lengths 0 and 4095, refusal of 4096, all alignment remainders, CRC and signature errors, truncation, final padding, and bounded recovery. An overrun delimiter cannot supply a valid member boundary.

The receiver fixture uses the real radio and medium with scalar and dimensional reception. It executes 31 scenarios in both reception modes, for 124 seed-0 runs. The scenarios cover ordinary NIST/Yans reception, an absent model, partial loss, delimiter and spacing-delimiter failure, total member failure, and failed common parts. Invalid CRC, unsupported fields, and structural refusal use distinct paths. Short-GI aggregate reception preserves the copied indication. An absent-model case with an unmet SNIR threshold supplies no accepted header or ranges and consumes no draws.

Repeated PREAMBLE, HEADER, DATA, WHOLE, direct success, and result queries add no draws or probability calls. The fixed-probability case consumes seven draws for seven ranges. Endpoint probabilities consume no draws. The fixture checks PREAMBLE before any HEADER or range call. It retains a copied indication after normal cache cleanup. The result preserves power, SNIR, time, mode, channel, and transmission identity.

The capability cases create a production WirelessHost. A missing role fails at the physical-layer check. Incomplete short-GI coverage fails before the MAC updates the MIB. Complete one-stream coverage passes despite unsupported catalog modes above the antenna limit. An absent model passes, including the unit-disk configuration. A non-HT model without the role retains ordinary reception; its runtime change to an HT set fails.

Separate cases verify BerTable's NED refusal, initializer refusal, and non-HT initialization.

The configured radio filters and histogram recorders receive aggregate metadata. The recorded aggregate probability histograms have zero samples because their input values are `NaN`. Ordinary NIST/Yans controls retain a zero PER and unknown BER/SER. Finite-SNIR controls without a model retain no error-rate tag. This evidence closes the metadata consumer audit for the configured paths.

The isolated parent checkout is `/tmp/inet-ampdu-parent-4fa5cc0de1`. Its six ordinary HT runs pass with NIST, Yans, and no model, in both reception modes. The feature's corresponding scalar controls retain 170 microseconds for a 100-byte PSDU at HT MCS 0 with long GI. NIST/Yans retain PER zero and BER/SER `NaN`. Every corresponding control retains the next RNG value `2357136044`.

## Commands and artifacts

Commands run from the active repository root unless this record states another directory. Debug is the behavioral test mode. Every new fixture uses seed 0. Existing fixtures retain their declared runs and seeds. Source changes require a fresh matching library before dependent tests.

Fixture commands use `MAKEFLAGS=--jobserver-style=pipe` and `MPLCONFIGDIR=/tmp/ampdu-matplotlib`. `/tmp/ampdu-source-sha256.txt` identifies the 22 changed source and test files. Its SHA256 is `6ab4156726c29065523a250443ade2cc67cbedceb220c70627001d7ca1017e16`. The manifest includes the final wire fixture and receiver fixture. Documentation does not form part of that source manifest.

| Check | Command or selection | Status and artifact |
| --- | --- | --- |
| Debug build | `make MODE=debug -j8` with `MAKEFLAGS=--jobserver-style=pipe`. | Pass, exit 0; `/tmp/ampdu-debug.log`. |
| Release build | `make MODE=release -j6`, followed by `make MODE=release -j8` with the same `MAKEFLAGS`. | Both pass, exit 0; `/tmp/ampdu-release.log` and `/tmp/ampdu-release-final.log`. |
| Wire fixture | `inet_run_unit_tests -m debug -f 'Ieee80211AmpduWire_1\.test$' --no-concurrent`. | Pass, exit 0; `/tmp/ampdu-wire.log`. |
| Receiver fixture | `inet_run_module_tests -m debug -f 'Ieee80211AmpduPhyReception_1\.test$' --no-concurrent`. | All 124 runs pass, exit 0; `/tmp/ampdu-phy.log`. |
| Unit controls | `inet_run_unit_tests -m debug -f 'Ieee80211(AmpduWire\|MpduSubframeHeaderSerializer\|MpduDelimiterWire\|HtModeSet\|HtModeCache\|VhtHtCapabilities)_1\.test$' --no-concurrent`. | Six fixtures pass; `/tmp/ampdu-final-unit.log`. The wire fixture has a later focused refresh. |
| Module controls | `inet_run_module_tests -m debug -f 'Ieee80211(AmpduPhyReception\|VhtHtSupport\|HtAssociation\|HtAntennaRateControl)_1\.test$' --no-concurrent`. | Four fixtures pass, exit 0; `/tmp/ampdu-final-module.log`. That run includes 120 receiver scenarios. The later focused receiver run adds four absent-model SNIR controls. |
| Functional fingerprints | From `tests/fingerprint`: `./fingerprinttest -d -F tyf -t 3 -m 'showcases/wireless/(aggregation\|qos)/' showcases.csv`. | All five configurations pass, exit 0; `/tmp/ampdu-fingerprints.log`. Graphical ingredients are excluded. No expectation changes occur. |
| Architecture | `doc/project/enforcement/check-architecture.sh` for each WLAN subtree. | Both pass; `/tmp/ampdu-phy-architecture.log` and `/tmp/ampdu-mac-architecture.log`. |
| New interface | `doc/project/enforcement/check-interfaces.sh src/inet/physicallayer/wireless/ieee80211/contract`. | One interface passes; `/tmp/ampdu-interfaces.log`. |
| Source protection | `doc/project/enforcement/check-source-seals.sh --diff`. | Pass; `/tmp/ampdu-seals.log`. No sealed source path changes. |
| Naming | Scoped naming checks in the active and parent checkouts. | The same three PHY and 32 MAC candidates remain. The PHY log differs only in one line number. `/tmp/ampdu-phy-naming.log`, `/tmp/ampdu-mac-naming.log`, and their `ampdu-parent-*` controls retain the comparison. |

The parent build command is `make MODE=debug -j12` from the isolated checkout. Its runner uses the same receiver-fixture selector and debug mode after `source setenv -q`. `/tmp/ampdu-clean-parent-debug.log` and `/tmp/ampdu-parent-controls.log` retain its results. The parent fixture contains only the ordinary controls and the same node setup.

The isolated BerTable reproduction is `tests/module/Ieee80211AmpduBerTableControl_1.test` in the parent checkout. The parent runner records the failure in `/tmp/ampdu-parent-bertable.log`. The same generated executable also runs with the feature library first in `LD_LIBRARY_PATH`. `/tmp/ampdu-feature-bertable-libraries.log` confirms that library path. `/tmp/ampdu-feature-bertable.log` records the same null-bit-model failure. The fixture supplies all eight table rates. These failures do not count as passing reception controls.

The runner's dry-run path fails with `ValueError('list.index(x): x not in list')`. Those previews establish no behavioral result. Normal filtered runs select and execute the named fixtures. A FIFO collision also affected an early parallel test build. Later commands use `MAKEFLAGS=--jobserver-style=pipe`. Fixture setup corrections precede the recorded passing runs.

## Standards evidence and limits

The local corpus identifies `ieee80211-2024` as IEEE Std 802.11-2024. Table 19-11 spans physical PDF pages 3431–3432. Clause 19.3.9.4.4 is on page 3434. Its Figure 19-8 reference resolves. The corpus cannot resolve Clause 9.7.1, so PDF text on pages 1841–1843 supplies delimiter evidence. PDF text on pages 3486–3487 confirms invalid-CRC and unsupported-mode behavior. These clauses are normative. The corpus linter reports unrelated heading ambiguities and unresolved references; the selected CRC node is unambiguous. Local artifacts `/tmp/ampdu-htsig-clause.json`, `/tmp/ampdu-htsig-table.json`, `/tmp/ampdu-htsig-refs.json`, and `/tmp/ampdu-standards-lint.json` retain the retrieved evidence.

The implementation does not enable MAC receipt, MPDU release, or an implicit Block Ack response. Feature A2 and A4 retain those acceptance duties. A failed MPDU range must become that member's bit-error flag before the normal Rx FCS check. The aggregate probability fields are not member estimates. P3 and P4 supply the additional evidence below. The commit and push authorization does not permit baseline changes.

Independent range draws at the current scalar SNIR are a packet-level model choice. This abstraction does not model correlated FEC errors or interference changes within one member. Alignment padding contributes duration but has no separate outcome. These checks establish representation, integration, and selected regression behavior. They establish no new PHY accuracy result. Release-mode behavioral tests and feature-isolation builds remain unverified.

The final self-audit traces cache ownership, prerequisite order, result copies, header refusal, structural refusal, capability publication, and the ordinary MPDU branches. It finds no unresolved defect in the implemented scope. The audit uses the current architecture, naming, quality, test, and WLAN guidance. It is a self-audit, not an independent review.

## Publication checks

The first publication attempt rejected Git metadata writes with `Read-only file system`. A separate repository at `/tmp/inet-ampdu-publication` held the commit series. Its source and test files matched the tested checkout. The separate `AGENTS.md` edit remained outside the series. Shell access could not resolve GitHub, so that attempt required the GitHub connector.

The final debug and release builds pass in `/tmp/ampdu-push-debug.log` and `/tmp/ampdu-push-release.log`. The repeated filters pass six unit fixtures and four module fixtures. Their logs are `/tmp/ampdu-push-unit.log` and `/tmp/ampdu-push-module.log`. The new series passes commit, classification, include, and NED parameter checks. The audit supplies no PR description because this task creates no PR. Global architecture, naming, and interface findings match the pinned parent. Source seals pass. These checks establish final-tree behavior; intermediate commits have no separate compilation result.

The repeated fingerprint filter passes all five controls without expectation changes. Its log is `/tmp/ampdu-push-fingerprints.log`. The full series since `origin/master` also passes its mechanical checks, with notes. The first publication attempt did not complete. The connector rejected the first tree request because tool approval was required and the approval policy was `never`. No remote branch update occurred. The original checkout retained its previous HEAD and local files.

The user requested a retry after the permissions changed. Git metadata writes and shell access to GitHub now succeed. All 28 prepared files match the active checkout before commit transfer. The active branch now contains the three prepared commits. The separate `AGENTS.md` edit remains outside those commits. Only this publication record changes after the transfer; the tested source and test files remain unchanged.

## P3 and P4 execution

The user requested P3 and P4 after publication of P1 and P2. The active HEAD is `0221daf515e1bdc7dcaa79b12b6547c11a977d75`. The comparison parent remains `4fa5cc0de15ee57388bcb3f20e866cab5972a670`. The active checkout and branch remain those recorded above. This execution changes tests, migration text, and plan records. It changes no production source, interface, ownership rule, model assumption, or stored fingerprint expectation. The separate `AGENTS.md` edit remains unchanged.

The additional module fixture separates ordinary reception from the new aggregate interfaces. The same fixture source compiles on both revisions. This necessary test separation adds one PHY fixture, so the combined PHY and MAC inventory increases from nine to ten. The MAC feature's seven fixtures remain proposed.

### P3: NIST/Yans probabilities and seeded outcomes

The extended `Ieee80211AmpduPhyReception_1.test` executes 468 runs. Its original 31 scenarios now run with seeds 0, 1, and 2. Eight additional scenarios cover NIST and Yans, MCS 0 and 7, and long GI and short GI. Every scenario uses scalar and dimensional reception with whole and separate signal parts. The bounded campaign adds no distributional or error-accuracy claim.

Test subclasses call the unchanged NIST and Yans implementations through the production receiver. Each range probability must equal the existing per-length DATA calculation at the configured scalar SNIR. The subclasses observe calls; they do not substitute probabilities. A separate reference matrix checks all 64 HT20 modes, including MCS 0 through 31 with both guard intervals. Each mode uses seven SNIR inputs and lengths 4, 100, 101, 102, 4095, and 65535 bytes. The minimum and mean reductions use distinct synthetic SNIR values. The four matrix instances execute 10752 exact comparisons without random draws. This matrix establishes calculation coverage; actual aggregate radio runs use MCS 0 and 7.

For example, a three-member aggregate includes a spacing delimiter and three alignment bytes. Seven scored ranges use the real model probabilities. An independent generator with the same seed replays the HEADER draw and each eligible range draw. The fixture compares every retained outcome, draw count, and subsequent random value with that reference. Partial member failure preserves the envelope and copied outcomes. Alignment padding causes no eighth range or extra draw.

At the PREAMBLE boundary, repeated cached and direct success queries cause no HEADER or DATA call or draw. At the result boundary, repeated queries reuse all retained outcomes and metadata. The fixture also checks indication values after cache cleanup. All 96 real-model records match between whole and separate signal parts. A repeated execution produces the same 96 records. The campaign contains 76 all-good records and 20 partial-failure records. Both models produce partial failure in this campaign.

### P4: Ordinary reception and compatibility limits

`Ieee80211AmpduPhyCompatibility_1.test` executes 576 runs on each revision. The inputs cover NIST, Yans, an absent model, and a non-HT custom model without the aggregate role. The HT controls select MCS 0 or 7 with either guard interval. The custom model selects ERP OFDM at 6 Mbit/s. Each input uses scalar or dimensional reception, whole or separate signal parts, and seeds 0, 1, and 2. The non-HT controls repeat the unused MCS and GI labels but always select that same ERP mode.

Three noise levels distinguish all-good controls, fractional probabilities, and failures. The low-noise power is -110 dBm. The two higher powers are -30 and -28 dBm for MCS 0, or -50 and -43 dBm for MCS 7. The receiver's energy threshold exceeds the configured noise power. This condition avoids an initialization failure that would supply no reception evidence. The SNIR threshold remains -10 dB for these bounded controls.

All 576 records match the parent exactly. The comparison includes delivery count, packet error, duration, PER, BER, SER, received power, minimum SNIR, reception endpoints, draw count, and subsequent random value. Assertions also check mode, channel presence, and transmission identity availability. The records include 498 successful receptions, 66 delivered failure containers, and 12 attempts without a delivered container. NIST supplies the twelve separate-part header aborts. The absent and non-HT custom models add no error draws.

The ordinary record stream has SHA-256 `f62dbea0e379e33e13df3246a2213d53ba6e52cc4a46852510f7f61bd5a01869`. `/tmp/ampdu-p4-comparison.json` records the fixture hash, revisions, counts, and equality result. The parent executable resolves `libINET_dbg.so` and `libtest_dbg.so` from the parent checkout. The active executable resolves the corresponding active libraries.

The extended reception fixture retains the unsupported-model checks from P2. The production MAC handoff refuses a missing role and incomplete short-GI coverage before its MIB update. It accepts unsupported modes outside the effective NSS limit. The fixture retains absent-model, runtime mode-set, non-HT custom, and BerTable initialization controls. The migration guide now states independent range draws, scalar SNIR reduction, guard intervals, padding treatment, and accuracy limits.

The separate BerTable reproduction still exits with SIGSEGV, status 139, on both revisions. Both logs identify `bitModel->getDataLength()` at line 62 with a null bit model. They are `/tmp/ampdu-p4-bertable-parent.log` and `/tmp/ampdu-p4-bertable-feature.log`. This comparison confirms the documented prior limitation. It is not a passing reception result. P4 does not repair BerTable or claim support for that configuration.

### Commands and evidence

All active commands run from `/home/user/omnetpp_ws/inet-ieee80211-ampdu-implicit-blockack` unless a command names another directory. Debug compilation precedes the tests. Release compilation also passes. The final source tree equals the published P2 source tree. The debug library has SHA-256 `3df8db6dba916c23bada8ea83583b41e9a425c81fc5a3bf90c83f7b75566044d`. The parent debug library has SHA-256 `877fa150f8cc41a21c765277aeb0ec478f99a6a9092a60462110496a68c99841`.

```sh
MAKEFLAGS=--jobserver-style=pipe make MODE=debug -j8
MAKEFLAGS=--jobserver-style=pipe make MODE=release -j8
MAKEFLAGS=--jobserver-style=pipe MPLCONFIGDIR=/tmp/ampdu-matplotlib inet_run_unit_tests -m debug -f 'Ieee80211(AmpduWire|MpduSubframeHeaderSerializer|MpduDelimiterWire|HtModeSet|HtModeCache|VhtHtCapabilities)_1\.test$' --no-concurrent
MAKEFLAGS=--jobserver-style=pipe MPLCONFIGDIR=/tmp/ampdu-matplotlib inet_run_module_tests -m debug -f 'Ieee80211(AmpduPhyReception|AmpduPhyCompatibility|VhtHtSupport|HtAssociation|HtAntennaRateControl)_1\.test$' --no-concurrent
```

The build logs are `/tmp/ampdu-p34-debug.log` and `/tmp/ampdu-p34-release.log`. Six unit fixtures and five module fixtures pass. The unit log is `/tmp/ampdu-p34-units.log`. The final module log is `/tmp/ampdu-p34-final-modules.log`. The direct compatibility logs are `/tmp/ampdu-p4-feature.log` and `/tmp/ampdu-p4-parent.log`.

The parent command uses the identical compatibility fixture at `/tmp/inet-ampdu-parent-4fa5cc0de1`, after `source setenv -q`, with the explicit filter `Ieee80211AmpduPhyCompatibility_1\.test$`. Its debug build remains the clean P0 parent build. The receiver's first model records are `/tmp/ampdu-p3-first-model-results.txt`. Later records match them exactly.

The module fixture writes its detailed output under `tests/module/work/<fixture-name>/test.out` in each checkout. To reproduce the comparison, select the lines that start with `CONTROL ` from each compatibility output. Require exactly 576 lines in each file. Compare the lines byte for byte in their original order. The fixture's post-run checker separately requires all 576 successful simulation runs and unique input combinations. The reception fixture's checker requires all 468 successful simulation runs and equality across signal-part modes.

Scoped architecture checks pass for both WLAN subtrees. Their logs are `/tmp/ampdu-p34-architecture-phy.log` and `/tmp/ampdu-p34-architecture-mac.log`. The source-seal check reports no changed production files. `git diff --check` passes. The earlier filtered fingerprint results remain evidence for the unchanged production source. This test-only extension requires no new fingerprint baseline or broad fingerprint campaign.

Early fixture revisions failed on invalid noise thresholds, typename expression syntax, and an incorrect ERP header-length assumption. A post-run check also exposed insufficient ordinary failure coverage. The final fixture corrects those setup and coverage defects. The earlier mixed batches retain their actual failure status. They do not replace the final clean runs.

The self-audit checks actual model dispatch, byte-to-bit conversion, mode coverage, SNIR reduction, probability bounds, cache order, sample reuse, metadata, and normal cleanup. It also checks identical parent inputs and library selection. It finds no unresolved defect in the requested scope. This result is a self-audit. Release behavioral tests, feature-isolation builds, correlated FEC behavior, and MAC acceptance remain outside this evidence. P4 closes D2 for the declared PHY profile, not the complete MAC feature.
