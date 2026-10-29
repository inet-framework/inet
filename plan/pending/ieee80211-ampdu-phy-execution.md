# HT A-MPDU PHY implementation and verification

## Scope

The branch supplies partial HT A-MPDU reception through PHY stages P0 through P4. Its base is `master` at `f5612791b773bc83f654dbfdefa26e243f756c86`. Master contains the explicit immediate Compressed Block Ack implementation from PR #1345. The topic contains ten commits above that base.

The supported aggregate profile is HT mixed format at 20 MHz with NIST or Yans, with long GI or short GI. The model uses independent range outcomes at the configured scalar SNIR reduction. It does not model correlated FEC errors or interference changes within one member. Alignment padding contributes duration without a separate outcome.

For example, an aggregate contains three MPDUs, and the middle MPDU fails. The receiver preserves the aggregate bytes and each range outcome. Repeated queries reuse those outcomes without extra random draws. Rx rejects the aggregate before standalone FCS or MAC header checks. MAC member acceptance, upper delivery, and implicit Block Ack remain pending.

## Owners and failure behavior

The radio constructs HT-SIG fields and CRC. The serializer preserves all 48 bits, including the received CRC. The shared validator rejects invalid CRC and unsupported fields before range calculations. A valid header whose length exceeds the available PSDU fails DATA bounds checks.

The delimiter inspector supplies bounded delimiter and MPDU ranges. The optional `IIeee80211AmpduErrorModel` role supplies pure support queries and range probabilities. NIST and Yans reuse their DATA calculations for each range length. Before its MIB update, the MAC supplies exact receive capabilities for coverage validation. Unsupported advertised modes cause `cRuntimeError`. An absent model retains common PHY checks without error draws.

The medium cache owns PREAMBLE, HEADER, DATA, and WHOLE decisions. DATA owns copied range outcomes. WHOLE summarizes DATA. The result indication copies values without reception pointers. With a present model, aggregate PER, BER, and SER contain `NaN` because this profile supplies no aggregate probability estimate.

Separate-part reception preserves PREAMBLE or HEADER aborts. Whole-signal reception can return a failed container. Rx rejects containers before standalone checks and excludes them from accepted predecessor history. The radio retains metadata for Radiotap. Ordinary MPDUs retain their metadata path.

## Verification contract

The current task requires a protocol regression on each applicable commit and its parent. Each comparison uses identical fixture bytes and fresh debug libraries. A parent must fail during execution. The behavior commit must pass every selected run. Generation, compilation, or link errors do not establish the required parent failure. Stored expectations remain unchanged.

The earlier verification campaign ran only at its final commit. That execution limit does not apply to the current per-commit comparisons.

## Commit review and protocol coverage

Four commits change observable protocol behavior. Their fixtures follow the production header factory, deaggregator, transmit serializer, and receive path. The helper commits introduce no production consumer. Their direct calculation tests remain appropriate. Documentation and test-only commits change no protocol behavior, so they need no artificial parent failure.

| Commit purpose | Protocol test decision | Evidence or reason |
| --- | --- | --- |
| Define the PHY stages. | No protocol regression applies. | The commit adds plans without runtime changes. |
| Create VHT headers for VHT signal modes. | Retain the direct transmission fixture. | [Header comparison](../../tests/protocol/wifi/vht-header-identity-comparison.md) checks the VHT class on both revisions. |
| Inspect bounded MPDU aggregate ranges. | Retain the direct unit fixture. | No production caller uses the new helper in this commit. |
| Recover bounded MPDUs during deaggregation. | Retain the extraction fixture. | [Deaggregation comparison](../../tests/protocol/wifi/mpdu-deaggregation-comparison.md) checks valid input and recovery after an overlong delimiter. |
| Preserve HT-SIG fields through byte conversion. | Make the wire fixture compile on the parent. | [Wire comparison](../../tests/protocol/wifi/ampdu-ht-sig-wire-comparison.md) checks independent bytes through production transmission. |
| Expose pure HT A-MPDU range probabilities. | Retain the direct probability fixture. | No receiver uses the optional role in this commit. |
| Retain partial HT A-MPDU PHY outcomes. | Make the fixture compile on the parent. Extend loss coverage. | [Reception comparison](../../tests/protocol/wifi/ampdu-partial-reception-comparison.md) checks each MPDU position, delimiter loss, and a subsequent CTS. |
| Verify seeded NIST and Yans aggregate reception. | Retain the module campaign. | The commit changes tests without new protocol behavior. |
| Verify ordinary PHY compatibility. | Retain the module controls. | These controls require unchanged behavior on both revisions. |
| Record PHY completion. | Record the results and limits. | The commit changes documents without runtime changes. |

Each behavior commit includes its fixture, guide, and comparison record. The four comparisons use the original production revisions before these fixture amendments. Structural checks must establish identical production trees after the amendments. The source review found no additional protocol behavior without an applicable fixture.

The earlier direct scope covers HT-SIG bytes, bounded ranges, NIST/Yans probabilities, cached samples, ordinary metadata, Rx rejection, Radiotap, and local interframe observations. The current comparison scope covers the four protocol fixtures. Explicit file selectors define this scope. Automatic dependency selection remains unverified because this checkout has no dependency store.

## Observed results

Verification date: 2026-10-29. Eight debug build invocations prepare seven production revisions for the parent comparisons. Every build and fixture compilation passes. Each parent fixture fails during execution at its intended assertion or production error. Each behavior commit passes its complete selected fixture. Commands, input hashes, logs, and native results reside in `/tmp/ampdu-protocol-parent-20261029/`.

| Protocol fixture | Parent result | Behavior commit result |
| --- | --- | --- |
| `WifiVhtHeaderIdentity.test` | The VHT class assertion fails. One later run is skipped. | Both runs pass. |
| `WifiMpduDeaggregation.test` | Two complete-input controls pass. Malformed extraction errors. One later run is skipped. | All four runs pass. |
| `WifiHtAmpduWire.test` | Serialization writes zero bits for the 48-bit header. One later run is skipped. | Both runs pass. |
| `WifiHtAmpduPartialReception.test` | The copied indication is absent at the Rx drop. The 31 later runs are skipped. | All 32 runs pass. |

The four behavior fixtures complete 40 successful runs. They declare no expected failures. Cmdenv stops each parent execution after its first error. Each comparison record identifies the executed parent case and the skipped cases. Those skipped cases supply no parent evidence.

Earlier debug and release builds passed on the original topic. Retained results cover nine unchanged unit fixtures and fifteen unchanged module fixtures. The module results include 468 aggregate runs, 576 ordinary controls, and sixteen MAC boundary runs. The earlier protocol campaign also covered eight fixtures, with eight partial-reception runs before this extension. These are retained results, not fresh unit, module, or release execution for this task.

Retained fingerprint controls use run 0 with `tplx`, `~tNl`, and `~tND`. `MacQos` and `MsduAggregation` use ten-second limits. `Aggregation`, `NoAggregation`, and `VoicePriorityAggregation` use one-second limits. The source and fingerprint store remain byte-identical. These matches supply regression evidence, not proof of protocol correctness.

The production `src` tree remains `8e187a213a126c244a3f49d40047758d7aa3e493`. Each amended commit must retain its original production tree, shared test library, descriptor, unit tests, module tests, and fingerprint files. The comparison records identify the original source revisions. `commit-map.json` records their amended owners.

The VHT and deaggregation fixture bytes remain unchanged. The wire fixture replaces unavailable getters with descriptors. The reception fixture adds descriptors and four loss patterns.

The earlier mechanical series and source-seal checks passed. Earlier global architecture, naming, and interface checks reported existing findings without additions against pinned master. Those global results remain retained evidence. The current final procedure repeats the series and source-seal checks. It also checks the preserved local edit and the final protocol selection. Local link validation passes all 87 file links in the selected guides, comparison records, and PHY plan documents.

The installed runner rejects the coverage-report setting in `inet.opp`. Temporary compatible descriptors omit that setting and retain all selected test settings. The checked-in descriptor remains unchanged. The isolated checkout reuses the active build output. Symbolic output links permit that reuse across filesystems. This setup changes no compiler flag or runtime assertion.

The final procedure refreshes changed source dependencies before the selected debug build. This step prevents stale objects after reuse of the output by the parent comparisons. The final command selects all four fixtures. Its native result file records the candidate's final-head outcome. The procedure accepts the candidate checkout as its working directory.

```sh
export MAKEFLAGS='--jobserver-style=pipe LN=ln\ -sf'
make MODE=debug -j8

record=/tmp/ampdu-protocol-parent-20261029
common=(--load "$record/inet-final.opp" -p inet -m debug --no-build --no-concurrent)
opp_run_protocol_tests "${common[@]}" -w '^tests/protocol/wifi$' \
  -f 'Wifi(VhtHeaderIdentity|MpduDeaggregation|HtAmpduWire|HtAmpduPartialReception)\.test$' \
  --result-file "$record/final-protocol.json"
```

The result files retain each observed and expected verdict. The artifact directory contains `compare.py`, `comparison.json`, per-revision evidence, `fold.py`, and `verify-final.sh`. The final procedure retains the exact commands, environment, and output paths. Per-revision simulation output remains compressed beside its native result. Retained earlier records reside in `/tmp/ampdu-process-correction-20261029/` and `/tmp/ampdu-master-20261029/`.

## Limits

The parent comparisons establish fixture-level regressions. They do not establish parent outcomes for cases skipped after the first runtime error. The behavior commits execute every selected case. Ordinary controls remain retained evidence without a fresh master comparison. This task runs no release behavioral tests, feature-isolation builds, C++ lint, or full inherited-source campaign.

The new partial-reception cases use scalar radios and HCF. Existing module results retain the separate dimensional-radio and DCF coverage. The fixtures make no new error-accuracy or goodput claim. MAC member acceptance and implicit Block Ack remain outside the implemented scope.
