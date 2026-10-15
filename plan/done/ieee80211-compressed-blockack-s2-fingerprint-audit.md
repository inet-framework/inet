# S2 fingerprint audit

## Verdict

The eight changed fingerprint rows are justified by the accepted S2 lifetime behavior. Each first changed transmission follows disposal of expired DATA. With a 100-second lifetime, S2 restores all 24 parent ingredient values across these eight rows. This control preserves the recorded run numbers, traffic, topology, time limits, and fingerprint ingredients.

The original S2 campaign completes all 19 selected rows without an execution error. Eleven rows remain unchanged. Eight rows change 23 ingredient values; the QoS showcase retains its `~tNl` value. The findings justify the replacement values for these recorded runs. The maintainer approved these eight rows and 23 changed values on 14 October 2026.

The parent is `1cdb362a2a4b97f564566487079d9d156a5d213d`. The first-exchange traces and control runs use S2 source `d10d0fed381570b9bacf6f2113e55b37d4e4ae57`. Both debug libraries pass a fresh build before those runs. The approved baseline update changes only the 23 identified values in the two fingerprint CSV files. The final source also supports untracked aggregation sources and exposes a native lifetime statistic. A fresh 19-row rerun retains every approved fingerprint value.

## Why expiration changes these results

S2 starts a deadline when HCF accepts QoS DATA. The default lifetime is 0.512 seconds. HCF discards expired DATA through its queue owner and removes its acknowledgment and retry state. The data service starts a new deadline for a new A-MSDU. Fragments retain the source deadline; retries preserve that deadline.

The parent can therefore select an older MSDU that S2 already discarded. A later aggregate can contain newer MSDUs but retain the same length, sequence number, and transmit time. Its payload bytes and checksums differ. In other cases, disposal removes an old fragment or the outstanding state that would trigger a BAR. These changes alter subsequent exchanges.

IEEE Std 802.11-2024 Clause 10.23.2.12.1 requires a timer at MAC acceptance and a timer restart for each MSDU addition to an A-MSDU. Its limit also applies to undelivered fragments. When a limit is reached, retry attempts cease and the associated unit is discarded. This audit applies those lifetime rules to S2's accepted QoS DATA scope. It makes no broader management, DCF, or Compressed conformance claim.

## Two independent checks

The full-duration control uses S2 with `msduLifetime=100s`. The Block Ack fragmentation row retains its one-second limit. The other seven rows retain their ten-second limits. All eight rows pass against their parent expectations, with all three selected ingredients. This control shows no residual fingerprint movement from S2's other source changes in these runs when no lifetime expires.

The exchange comparison uses the parent and the default S2 lifetime. A passive diagnostic library subscribes to native queue, drop, and radio transmission signals. It records frame names, MAC fields, acceptance events, disposal causes, and exact transmit endpoints. It also records serialized MAC bytes after the PHY header, with the remaining PHY padding. The observer creates no module, timer, traffic, or protocol decision.

The native radio listener includes the AP. This condition matters because `AccessPoint` has no built-in PCAP recorder. Host PCAP captures corroborate frame fields and host transmit endpoints, but they cannot establish the first AP difference. In `MacQos`, the first AP difference precedes the host retry delay. The native trace supplies that earlier cause.

The comparison excludes event numbers and local object IDs because S2 adds lifetime events and a timer object. It compares every radio's transmit start, transmit end, source module, and serialized bytes until the first difference. It retains frame names separately to identify the source MSDUs. Every preceding transmission matches on the compared fields.

## First changed transmission in each row

Times below are native transmission start times, in seconds. All rows use run 0 unless the row states run 1. The matching-prefix column counts transmissions across all radios before the first difference.

| Case | Matching prefix | First changed start | Parent transmission | S2 transmission and cause |
| --- | ---: | ---: | --- | --- |
| Block Ack showcase, `Fragmentation` | 1,006 | 0.517081714780 | Basic BAR starts at sequence 48. | DATA sequence 50, fragment 0 replaces the BAR. Expiration removes the remaining fragment of 48 and all unresolved fragments of 49. |
| QoS showcase, `Qos` | 3,247 | 1.551994303392 | Sequence 13, fragment 0 contains `background-52` through `background-55`. | The same MAC identity contains `background-174` through `background-177`. The older source MSDUs expired before aggregate construction. |
| Wireless example, `MacQos` | 5,399 | 1.716663856169 | The AP retries sequence 1, fragment 1, from aggregate `FTP-27` through `FTP-53`. | The AP sends sequence 2, fragment 0, from `FTP-54` through `FTP-80`. The old aggregate fragment expired. |
| Wireless example, `MacQosWithoutAggregation` | 4,893 | 1.536459549080 | The client sends `FTP-16`, sequence 16. | The client sends `FTP-25`, sequence 17. HCF discarded the retained `FTP-16` before this transmission. |
| Wireless example, `MacQosWithRtsCts` | 10,524 | 1.980497251834 | Sequence 24, fragment 0 contains `WWW-462` through `WWW-488`. | It contains `WWW-466` through `WWW-492`. Four old queued MSDUs expired before construction. The preceding CTS remains equal. |
| Wireless example, `MacQosWithBlockAck` | 5,460 | 2.206704819058 | Sequence 25, fragment 0 contains `FTP-625` through `FTP-651`. | It contains `FTP-686` through `FTP-712`. The older queued MSDUs expired before construction. |
| Ad hoc example, `MacQos`, run 0 | 2,464 | 1.079043803392 | Sequence 166, fragment 0 contains `Video-496` through `Video-498`. | It contains `Video-669` through `Video-671`. The older queued MSDUs expired before construction. |
| Ad hoc example, `MacQos`, run 1 | 1,931 | 1.066729876784 | Sequence 113, fragment 0 contains `Video-337` through `Video-339`. | It contains `Video-545` through `Video-547`. The older queued MSDUs expired before construction. |

The table identifies actual serialized differences, not only different frame names. In the aggregate cases, the initial MAC sequence and fragment fields can remain equal while the source MSDUs differ. This explains why a length-based ingredient can remain equal while the other ingredients change.

## Deadline checks for the changed source units

Each disposal below uses `LIFETIME_EXPIRED`, distinct from queue overflow or retry-limit disposal. Each observed disposal time equals its recorded deadline. The aggregate row uses its construction time as the timer origin. The fragmentation row uses MAC acceptance of the original MSDU.

| Source unit | Timer origin | Disposal | Observed age | Effect on the first changed exchange |
| --- | ---: | ---: | ---: | --- |
| Block Ack `Data-48`, fragment 15 | 0.004900000000 | 0.516900000000 | 0.512 s | Its fragment leaves outstanding state before the BAR decision. |
| Block Ack `Data-49`, unresolved fragments | 0.005000000000 | 0.517000000000 | 0.512 s | Their disposal also removes outstanding state before that decision. |
| QoS `background-52` | 1.015614674658 | 1.527614674658 | 0.512 s | It cannot enter the aggregate constructed at 1.545166090102 seconds. |
| AP aggregate `FTP-27` through `FTP-53`, fragment 1 | 1.178842523297 | 1.690842523297 | 0.512 s | The AP cannot retry this fragment at 1.716663856169 seconds. |
| Wireless `FTP-16` | 1.016000000000 | 1.528000000000 | 0.512 s | Its original sequence number does not extend its deadline. |
| RTS/CTS `WWW-462` | 1.462250000000 | 1.974250000000 | 0.512 s | It cannot enter the aggregate constructed at 1.978236539496 seconds. |
| Basic Block Ack `FTP-625` | 1.625000000000 | 2.137000000000 | 0.512 s | It cannot enter the aggregate constructed at 2.197478943462 seconds. |
| Ad hoc run 0 `Video-496` | 0.549600000000 | 1.061600000000 | 0.512 s | It cannot enter the aggregate constructed at 1.078868469136 seconds. |
| Ad hoc run 1 `Video-337` | 0.533700000000 | 1.045700000000 | 0.512 s | It cannot enter the aggregate constructed at 1.066472542528 seconds. |

For the Block Ack showcase, the data service creates `Data-48`'s fragments at 0.482373863 seconds. Fragment 15 still expires at 0.5169 seconds. Its age from fragment creation is only 0.034526137 seconds. Its source MSDU age is 0.512 seconds, which is the controlling lifetime. A new fragment must not restart that deadline.

For the AP example, the aggregate restarts its timer at 1.178842523297 seconds. Its fragment expires at 1.690842523297 seconds. The parent retries it later, while S2 selects the next aggregate. This case checks the distinct aggregate rule rather than the source-MSDU rule.

The Block Ack request policy counts outstanding frames for each receiver. HCF's expiration path removes expired in-progress frames and their acknowledgment status. The remaining count no longer triggers the parent's BAR at that point. The existing policy instead selects DATA 50. This result follows [the request policy](../../src/inet/linklayer/ieee80211/mac/originator/OriginatorQosAckPolicy.cc), [expiration](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc), and [acknowledgment disposal](../../src/inet/linklayer/ieee80211/mac/originator/QosAckHandler.cc).

## Exact observed values

These are the approved values from the completed default-lifetime campaign. The source commit includes these values in the tracked CSV files. Each 100-second control reproduces the parent column exactly. The update preserves all other ingredients, row settings, and configurations.

| Case | Parent values | Default S2 values |
| --- | --- | --- |
| Block Ack `Fragmentation`, run 0 | `2af7-c4f6/tplx;bdc7-cfd0/~tNl;3135-dd51/~tND` | `2235-516d/tplx;7b51-e67c/~tNl;9a3f-afc7/~tND` |
| QoS `Qos`, run 0 | `5a33-2cf4/tplx;0683-d978/~tNl;f448-e820/~tND` | `2f30-f17c/tplx;0683-d978/~tNl;a879-de03/~tND` |
| Wireless `MacQos`, run 0 | `7ae8-d762/tplx;7a17-6a1d/~tNl;93a3-323a/~tND` | `d929-5913/tplx;f602-4424/~tNl;f2fb-4d98/~tND` |
| Wireless `MacQosWithoutAggregation`, run 0 | `8241-1905/tplx;60a4-5565/~tNl;a06d-81c9/~tND` | `0473-f9d2/tplx;e92c-7352/~tNl;317d-aa4f/~tND` |
| Wireless `MacQosWithRtsCts`, run 0 | `de32-56b9/tplx;00f9-9ce3/~tNl;e305-c88a/~tND` | `e07a-4c5f/tplx;9f6c-7889/~tNl;29ea-169c/~tND` |
| Wireless `MacQosWithBlockAck`, run 0 | `8dc8-a4a7/tplx;0826-0ca5/~tNl;0b4c-d9a2/~tND` | `fbab-b84e/tplx;47cb-b9e5/~tNl;c4f3-fa00/~tND` |
| Ad hoc `MacQos`, run 0 | `898b-aa3c/tplx;fbaf-8b67/~tNl;3b12-ca4e/~tND` | `dd36-1076/tplx;b703-3ef2/~tNl;e07d-2980/~tND` |
| Ad hoc `MacQos`, run 1 | `017f-bfed/tplx;adbc-faea/~tNl;9e6e-f139/~tND` | `b80d-4ae8/tplx;72b7-86c2/~tNl;d385-62e0/~tND` |

## Reproduction and evidence

The [implementation record](ieee80211-compressed-blockack-s2-execution.md) preserves the original default-lifetime results and the 19-row comparison. The new audit evidence is under `tests/module/work/s2-causality/`. These local paths are ignored test output; they are not published PR artifacts.

- `long-lifetime-control.csv`, `.log`, and `.json` preserve the eight-row selection, runner status, and all verified control values.
- `native-captures/<case>/<version>/owners.tsv` records native queue and drop observations.
- `native-captures/<case>/<version>/owners.tsv.air.tsv` records every radio's native transmissions, including the AP.
- `native-first-divergences.json` records each matching prefix and first changed transmission.
- `observer/CausalObserver.cc`, `run_captures.py`, and `compare_native.py` preserve the passive observer and comparison procedure.
- `captures/`, `start-captures/`, and `full-captures/` preserve supplementary host PCAP captures and commands.

The control command runs from `tests/fingerprint`:

```sh
./fingerprinttest /tmp/s2-lifetime-control.csv -d -t 4 \
  -m '(showcases/wireless/(blockack|qos)|examples/(wireless|adhoc)/qos)' \
  -f tplx -f '~tNl' -f '~tND' -a '--**.msduLifetime=100s'
```

The runner returns exit status 0: eight passes, zero mismatches, and zero execution errors. The control uses the same computed checksum and FCS settings as the original fingerprint campaign. The fresh debug builds also return exit status 0.

The native comparison uses these commands from the repository root after the diagnostic library build:

```sh
python3 tests/module/work/s2-causality/run_captures.py \
  --no-pcap --output-subdir native-captures
python3 tests/module/work/s2-causality/compare_native.py
```

All 16 native runs complete their selected intervals without an execution error. The intervals cover the first changed exchange in every case. The parent and S2 use their respective source trees, libraries, NED paths, and original INI files. The scripts set `INET_ROOT` explicitly for each checkout. Earlier NED-path setup failures and the unsupported drop-PCAP attempt supply no behavioral evidence.

The standards corpus reports a fresh IEEE 802.11-2024 document. The selected clause has the exact node `ieee80211-2024:clause:10.23.2.12.1`. Its span is `ieee80211-2024@8388218:8393301`, on physical PDF pages 2016–2017. The corpus linter retains unrelated structural findings. The clause retrieval has no ambiguous choice.

## Limits and baseline decision

After approval, the complete 19-row debug fingerprint selection passes against the updated CSV values. The runner returns exit status 0, with zero mismatches and zero execution errors. Debug and release compilation pass again. The 21 module fixtures and four unit fixtures pass again. The eight protocol fixtures retain four positive passes and four declared expected failures. The aggregation fixture adds non-QoS and mixed-source lifetime coverage, with native scalar checks.

The approved verification input is `/tmp/s2-approved-validation.csv`. Its rows come from the updated source CSV files. The runner log is `/tmp/s2-approved-fingerprints.log`. Each result remains under `tests/fingerprint/results/tmp/s2-approved-validation.csv/`. The evidence folder also retains copies of the selection and final logs.

The control covers the exact eight configurations and recorded runs. It does not test additional seeds, platforms, or release-mode behavior. The native traces establish the first changed transmission, not every later causal step or successful PHY reception. No independent review or CI result follows from these local checks.

The verification selects `tplx`, `~tNl`, and `~tND`. Seven changed rows also carry historical graphical `tyf` values. No recorded local run verifies those values, and fingerprint CI excludes that ingredient. Their status remains unknown. The approval covers only the 23 changed values in the selected ingredients, with no graphical baseline update.

The evidence supports the 23 changed ingredient values in the table. It shows intended disposal at the accepted deadline and a resulting exchange change in each case. No unexplained fingerprint movement remains within this selection. The maintainer's approval satisfies [the baseline procedure](../../doc/project/guide/change-a-baseline.md) for these exact rows and values. The source commit includes the approved update.
