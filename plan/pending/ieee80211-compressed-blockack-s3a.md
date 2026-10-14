# S3a execution record for Compressed Block Ack

S3a preserves the existing Basic demonstrations through explicit profile selection. HCF validates its new selector during initialization. Basic remains the temporary default. Compressed selection stops with an explicit diagnostic because its runtime exchange path does not exist. This local change implements S3a only.

This series covers HCF profile validation and explicit Basic selection in the existing demonstrations and fixtures. It also records exchange evidence and explains historical compatibility. DATA lifetime changes, agreement lifecycle repairs, independent BAR work, rate publication, and Compressed exchanges remain outside this scope. The source parent and comparison base are `1cdb362a2a4b97f564566487079d9d156a5d213d`.

The checkout is `/home/user/omnetpp_ws/inet-compressed-blockack-s3a`, on `work/compressed-blockack-s3a`. Verification used OMNeT++ `6.4.0aipre2`, build `260526-7b9114f72c`, with debug INET libraries. This record describes the S3a series for upstream review. The pull request records its publication status.

The original source manifest (`/tmp/inet-s3a-20261014/source-manifest.json`) records SHA-256 values for all twenty-nine files at published head `41a5c38c47`. It excludes the unrelated user changes and supplied plans. The full comparisons below cover that original publication. A separate section records checks after the review corrections. All `/tmp` paths refer to local artifacts that are unavailable through repository links. The committed fixtures and commands let a reviewer repeat the checks.

## Profile ownership and preserved behavior

Before S3a, the demonstrations and fixtures relied on the implicit Basic implementation. A later default change could convert these historical studies to another profile. S3a adds explicit Basic settings to six INI files and twelve existing fixtures. The settings preserve their topology, PHY modes, traffic, parameter loops, fragmentation thresholds, and assertions.

[HCF](../../src/inet/linklayer/ieee80211/mac/coordinationfunction/Hcf.cc) owns the local profile value. At local initialization, HCF reads the NED selector, `blockAckVariant`, once and converts it to a private typed value. HCF exposes no C++ getter because no current consumer needs one. Unknown values stop initialization, also when Block Ack support is disabled. The temporary Compressed rejection prevents a Compressed setting from silently producing Basic frames. Agreement handlers, frame builders, serializers, and rate selection retain their existing behavior.

The [profile fixture](../../tests/module/Ieee80211BlockAckVariant_1.test) checks six initialization cases. Default and explicit Basic settings succeed. The fixture uses a separate INI file with no selector assignment for the default case. Unknown and Compressed settings fail with support enabled or disabled. S3b's admission stage, A2, must replace the temporary Compressed rejection. A2 also adds capability-gap fallback: Normal Ack operation when required peer HT state is absent.

The documentation identifies Basic as a historical format from [IEEE Std 802.11e-2005](https://standards.ieee.org/ieee/802.11e/3131/). Clauses 7.2.1.7 and 7.2.1.8 define its BAR and response formats. Physical PDF pages 52–54 supplied the historical text. [IEEE Std 802.11-2024](https://standards.ieee.org/ieee/802.11/10548/) reserves type 0 in Tables 9-36 and 9-37. The canonical identifiers are `ieee80211-2024:table:9-36` and `ieee80211-2024:table:9-37`, on physical pages 719–723. Retained Basic support does not establish complete conformance to either edition.

## Direct exchange evidence

The [observation fixture](../../tests/module/Ieee80211BasicCompatibility_1.test) observes the production MAC boundary without a change to protocol decisions. It records accepted ADDBA responses, Block Ack-policy DATA, Basic BAR, and positive Basic responses per peer and TID. It matches response sequence numbers to the transmitted BAR. It checks receipt bits against transmitted sequence and fragment numbers. Its `fragmentBits` counter counts matched responses that acknowledge a nonzero fragment number. The [evidence script](../../tests/module/basic-compatibility-evidence.py) applies these checks to the actual demonstration networks and configurations.

For example, `Fragmentation` sends Block Ack-policy fragments after accepted setup. In 50 ms, the originator transmits 68 such DATA frames and four Basic BAR frames. It receives three matched responses with positive receipt bits. All three responses acknowledge nonzero fragment numbers. Parent and candidate produce the same counts. This establishes retained Basic fragment exchanges in the selected sample.

All thirteen selected cases pass. Their peer addresses, TIDs, exchange counts, and Normal Ack counts match the parent exactly. Every case uses seed 0. These short windows establish exchanges; the separate fingerprint checks retain the full recorded time limits.

| Source | Configuration | Run | Window | Required observation |
| --- | --- | ---: | --- | --- |
| Block Ack showcase | `General`, `NoFragmentation`, `MixedTraffic` | 0 | 50 ms each | Complete Basic exchange on one wireless hop. |
| Block Ack showcase | `Fragmentation` | 0 | 50 ms | Complete Basic exchange with positive fragment bits. |
| TXOP showcase | `General` | 0 | 50 ms | Complete Basic exchange in the retained mixed traffic. |
| Wireless QoS example | `MacQosWithBlockAck` | 0 | 1.2 s | Complete Basic exchanges on both wireless hops. |
| Ad hoc QoS example | `MacQos` | 0 | 0.8 s | Normal Ack DATA transmissions; no ADDBA, BAR, or Block Ack. |
| Ad hoc QoS example | `MacQos` | 1 | 0.8 s | Complete Basic exchange with support enabled. |
| Fragmentation showcase | `HCFfragblockack` | 0 | 100 ms | Positive fragment receipt bits. |
| Parameter study | `PacketSizeHCFfragblockack` | 8 | 100 ms | Positive fragment receipt bits at 2000 B. |
| Parameter study | `TxPowerHCFfragblockack` | 12 | 100 ms | Positive fragment receipt bits at 0.12 mW. |
| Parameter study | `ThresholdHCFfragblockack` | 0 | 100 ms | Positive fragment receipt bits with the 100 B threshold. |
| Parameter study | `OnlyPacketSizeHCFfragblockack` | 8 | 100 ms | Positive fragment receipt bits at 2000 B. |

The longer parent QoS run records two Block Ack-policy fragments for TID 1 on each hop, without a BAR. These counts remain below the BAR threshold of five. The short comparisons also retain incomplete TID 1 work. S4 owns independent BAR work; S3a does not repair it. The 0.06 mW parameter point sends seven ADDBA requests but accepts no agreement within 100 ms. Its loss cause remains unconfirmed; the retained 0.12 mW sample establishes the required exchange without a change to the sweep.

The parent evidence (`/tmp/inet-s3a-20261014/parent-demonstrations.json`) and candidate evidence (`/tmp/inet-s3a-20261014/candidate-demonstrations/evidence.json`) retain commands, windows, exit statuses, counters, and raw log paths. The parent record combines twelve passing samples with the separately measured 0.12 mW sample. It also retains the unsuccessful 0.06 mW observation. These samples do not validate every point in each parameter sweep.

## Fixture inventory and deferred requirements

The tracked inventory contains eight protocol fixtures and four module fixtures that can enable Block Ack. Five other module fixtures explicitly disable support. The S1 recipient fixture supplies Basic agreements without an HCF selector. The response-mode unit fixture and serializer fixture consume Basic types without an enablement assignment. The enablement inventory (`/tmp/inet-s3a-20261014/parent-enablements.txt`) and Basic consumers (`/tmp/inet-s3a-20261014/parent-basic-consumers.txt`) retain the source searches.

Separate capability probes load the generated protocol configurations with their original `_n.ini`, `_ac.ini`, and `_base.ini` includes. They retain full AP/STA management and real association. They disable the registered protocol assertions for observation only; the normal protocol runner executes those assertions separately. Every probe observes valid local HT support and valid peer HT records during DATA transmission. Parent and candidate produce the same recorded exchange counts. These observations do not establish negotiated agreement parameters or Compressed exchanges.

| Protocol fixture | Actual configured mode | Observed local/peer HT state | Normal runner result and retained obligation |
| --- | --- | --- | --- |
| `N_BlockAck` | `n(mixed-2.4Ghz)` | Valid / valid | PASS for existing generic Action, BAR, and response types. |
| `N_CompressedBa` | `n(mixed-2.4Ghz)` | Valid / valid | PASS for its existing Basic class and positive bitmap assertion. C2 must convert it to actual Compressed frames. |
| `N_Amsdu` | `n(mixed-2.4Ghz)` | Valid / valid | PASS for `aMsduPresent`; this assertion does not prove an accepted agreement. |
| `N_Ampdu` | `n(mixed-2.4Ghz)` | Valid / valid | Expected FAIL: step 0 observes no required A-MPDU subframe header within two seconds. |
| `Ac_BlockAck` | `ac` | Valid / valid | PASS for existing positive Basic coverage. C2 must retain real VHT/HT capability exchange. |
| `Ac_Ampdu` | `ac` | Valid / valid | Expected FAIL: step 0 observes no required A-MPDU subframe header within two seconds. |
| `Ac_MultiTidBa` | `ac` | Valid / valid | Expected FAIL: step 2 observes no Multi-TID response within its deadline. |
| `WifiQosDelayedBa` | `n(mixed-2.4Ghz)` | Valid / valid | Expected FAIL: step 3 receives Basic Block Ack where it requires the immediate ACK of BAR. |

The capability evidence (`/tmp/inet-s3a-20261014/candidate-protocol-capabilities/evidence.json`) retains all eight probes. The failure evidence (`/tmp/inet-s3a-20261014/deferred-failures.txt`) compares each first failed requirement with the parent. All four failures occur at the same step with the same reason. Their assertions and expected-failure declarations remain unchanged. Valid HT state in these positive fixtures does not establish capability-gap fallback. That fallback requires S3b/A2.

## Verification commands and results

Both source boundaries used a fresh `make -j8 MODE=debug` build. Both builds exited 0. The following focused commands ran from the checkout root. The module selector includes four Basic consumers, five support-disabled controls, all eight S1 recipient scenarios, and both new fixtures.

```sh
MPLCONFIGDIR=/tmp/inet-s3a-mpl inet_run_module_tests -m debug -f 'Ieee80211(BasicCompatibility_1|BlockAckVariant_1|BlockAckReordering_1|HcfHeader_1|HcfMultipleProtection_[12]|SingleProtectionGroup_1|RecipientNav_1|PreparedCancellation_1|Txnav_1|HcfSingleProtection_1|VhtHtSupport_1)\.test$' --no-concurrent --log-file /tmp/inet-s3a-candidate-modules.log
MPLCONFIGDIR=/tmp/inet-s3a-mpl inet_run_unit_tests -m debug -f 'Ieee80211(HtModeSet_1|VhtHtCapabilities_1|ResponseModeSelection_1)\.test$' --no-concurrent --log-file /tmp/inet-s3a-candidate-units.log
MPLCONFIGDIR=/tmp/inet-s3a-mpl inet_run_protocol_tests -m debug -w '^tests/protocol/wifi$' -f '(N_(Ampdu|Amsdu|BlockAck|CompressedBa)|Ac_(Ampdu|BlockAck|MultiTidBa)|WifiQosDelayedBa)\.test$' --no-concurrent --log-file /tmp/inet-s3a-candidate-protocol.log
python3 tests/module/basic-compatibility-evidence.py --output /tmp/inet-s3a-20261014/candidate-demonstrations --compare /tmp/inet-s3a-20261014/parent-demonstrations.json
python3 tests/module/basic-compatibility-evidence.py --protocol-inventory --output /tmp/inet-s3a-20261014/candidate-protocol-capabilities --compare /tmp/inet-s3a-20261014/parent-protocol-capabilities-focused/evidence.json
doc/project/enforcement/check-architecture.sh src/inet/linklayer/ieee80211/mac
doc/project/enforcement/check-naming.sh src/inet/linklayer/ieee80211/mac/coordinationfunction
git diff --check
```

The evidence script requires a new output directory. It uses the debug executable from the observation fixture. Generate that executable with the module runner before direct evidence runs. Generate protocol fixture inputs with the protocol runner before capability probes.

| Check | Candidate result | Exit status | Evidence |
| --- | --- | ---: | --- |
| Debug build | PASS | 0 | Build log (`/tmp/inet-s3a-candidate-build.log`). |
| Module fixtures | 12 PASS | 0 | Runner output (`/tmp/inet-s3a-candidate-modules-output.txt`). |
| Unit fixtures | 3 PASS | 0 | Runner output (`/tmp/inet-s3a-candidate-units-output.txt`). |
| Protocol fixtures | 4 PASS; 4 expected FAIL | 0 | Runner output (`/tmp/inet-s3a-candidate-protocol-output.txt`). |
| Demonstrations | 13 PASS; exact parent counts | 0 | Comparison output (`/tmp/inet-s3a-candidate-demonstrations.log`). |
| Capability probes | 8 PASS; parent counts retained | 0 | Comparison output (`/tmp/inet-s3a-candidate-capabilities.log`). |
| Architecture and naming | PASS in both source boundaries | 0 | Architecture (`/tmp/inet-s3a-candidate-architecture.log`), naming (`/tmp/inet-s3a-candidate-naming.log`). |

The parent also passes the ten existing module fixtures and the three unit fixtures. The repaired observation fixture passes separately against the parent. The normal protocol outcomes match the candidate. The source changes require no new architecture or naming exception. The packet subsystem remains outside the changed paths. The three edited showcase pages pass checks of their literalinclude paths and boundaries.

## Full fingerprint comparison

The following command ran from `tests/fingerprint` against both source boundaries. It selects all nineteen rows from the plan and preserves every recorded ingredient, including `tyf`.

```sh
./fingerprinttest -d -t 2 -m 'showcases/wireless/(blockack|aggregation|fragmentation|qos|txop)/|examples/wireless/qos/.*MacQos|examples/adhoc/qos/.*MacQos'
```

Both runner invocations exit 1 because fifteen rows retain graphical `tyf` mismatches against their recorded baselines. All other ingredients match their recorded baselines. Every calculated ingredient in all nineteen rows matches the parent exactly. The comparison record (`/tmp/inet-s3a-20261014/fingerprint-comparison.json`) contains both sets of values.

The parent log (`/tmp/inet-s3a-20261014/parent-fingerprinttest.out`) and candidate runner output (`/tmp/inet-s3a-candidate-fingerprint.log`) retain the original results. The cause of the existing graphical mismatches remains unresolved; S3a introduces no fingerprint difference. Recorded baseline files remain unchanged.

## Completion limits and next boundary

S3a is complete as a locally verified change. The self-audit confirms that HCF owns the selector and reads it only at initialization. No frame path, timer, queue operation, capability record, or agreement parameter changes. The tests establish Basic preservation and selector validation within the recorded samples.

- S2, the DATA lifetime stage, and S3b, the agreement lifecycle stage, remain outside this change.
- Compressed admission and capability-gap fallback require S3b/A2.
- Independent BAR work below the threshold requires S4.
- Production rate publication requires R1.
- Compressed runtime exchanges and default activation require S5.

Pin the final S3a revision as S3b's parent. Before later lifecycle work, refresh its source observations and review against that parent. Publication preparation also checks debug and release builds, the commit series, naming, source seals, and repository-wide architecture and interfaces. The results appear below. No independent review result is claimed. Existing user changes to `AGENTS.md` and the four supplied plan files remain intact.

## Publication checks

The first commit adds profile validation and its fixture. The second commit adds explicit Basic settings, exchange observations, and documentation. Both commits share the same production C++ and NED source. The profile fixture now uses its own INI file, so its default case checks the NED default directly. All six initialization cases pass.

| Check | Result |
| --- | --- |
| Debug compilation | PASS with `make -j8 MODE=debug`. |
| Release compilation | PASS with `make -j8 MODE=release`. |
| Repeated module fixtures | 12 PASS against the fresh debug library. |
| Repeated unit fixtures | 3 PASS against the fresh debug library. |
| Repeated protocol fixtures | 4 PASS and 4 expected FAIL; all failed steps and reasons equal the parent. |
| Commit series and description | PASS with no violations or notes. |
| Source seals | PASS for all changed source files. |
| Changed declarations | PASS for the new NED parameter. |
| Repository-wide architecture | FAIL with 36 findings, identical to the parent after sort by finding. |
| Repository-wide naming | FAIL with 23 violations, identical to the parent. |
| Repository-wide interfaces | FAIL with 15 interface classes; the complete output equals the parent. |

The local comparison record is `/tmp/inet-s3a-publication-gate-comparison.json`. These findings remain outside the S3a source change. The source-scoped checks above pass. Publication preparation repeated the recorded module, unit, and protocol selections against the fresh debug library. The failed-step comparison is `/tmp/inet-s3a-publication-deferred-failures.json`. The pull request states the final series and publication check results.

## Verification after review corrections

The review correction removes the unused public getter. The profile type and cached value become private because only HCF uses them. The correction preserves member order and all initialization branches. The opening now states the complete S3a scope without a dependency on an unpublished plan. The two corrections belong to the profile commit and the execution-record commit, respectively.

The corrected source passes fresh debug and release builds. The recorded module, unit, and protocol selections ran against the fresh debug library. Direct exchange comparisons and all nineteen fingerprint rows also ran again. The results below cover the corrected source.

| Check | Result |
| --- | --- |
| Debug and release compilation | PASS with `make -j8 MODE=debug` and `make -j8 MODE=release`. |
| Module and unit fixtures | 12 module fixtures and 3 unit fixtures PASS. |
| Protocol fixtures | 4 PASS and 4 expected FAIL; each failed step and reason equals the parent. |
| Demonstration comparisons | 13 PASS with exact parent counts. |
| Capability probes | 8 PASS with exact parent counts and valid local and peer HT state. |
| Full fingerprint comparison | Every calculated ingredient in all 19 rows equals the parent. |
| Stored fingerprint baselines | The runner exits 1 because 15 rows retain the existing graphical `tyf` mismatches. |
| Source-scoped architecture and naming | PASS. |
| Repository-wide architecture, naming, and interfaces | The existing findings equal the original publication and parent. |

The exchange records are `/tmp/inet-s3a-review-demonstrations/evidence.json` and `/tmp/inet-s3a-review-capabilities/evidence.json`. The fingerprint comparison is `/tmp/inet-s3a-review-fingerprint-comparison.json`. The failed-step comparison is `/tmp/inet-s3a-review-deferred-failures.json`. The original verification commands above apply with these new output paths. Recorded baseline files remain unchanged.
