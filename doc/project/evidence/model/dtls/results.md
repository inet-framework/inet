# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-17 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

The selected model now includes the opt-in extensions described in [scope.md](scope.md). Their stage-specific executed evidence is in [extensions.md](extensions.md), including legacy full/abbreviated exchanges, CID/heartbeat/path support and their limits. The historical initial-profile results below retain their original source boundaries and counts; they are not a claim that the current extension union was run by those earlier commands.

## Final extension integration matrix

Matching debug and release builds pass on the corrected model source `62b7af5f28`; the subsequent helper correction `467b99fa9f` changes test checks only. Both native library hashes and all 61 tracked model/application source hashes remain verified in `audit/dtls-extensions/c16-corrected-source-manifest.json`. The [execution ledger](extensions.md) records exact commands and the 60-case release selection.

| Check | Debug | Release |
| --- | ---: | ---: |
| DTLS protocol cases | 351 PASS | 60 PASS |
| DTLS/TLS units | 15 PASS | 15 PASS |
| DTLS module cases | 21 PASS | 21 PASS |
| Ordinary UDP and IPv6 fragmentation controls | 6 PASS | 6 PASS |
| Fixed-seed application demonstrations | 16 PASS | 16 PASS |

Every final selection is nonempty and exits zero. The full debug union includes the corrected resource-admission case; the release selection covers every extension and their principal interactions. Helper checks explicitly execute in both modes, preserving the model library's normal compiler flags. An isolated debug build with Dtls/TlsCommon disabled also passes all three selected UDP controls. Its loader resolves the isolated library, with no DTLS/TLS model symbols. Both corrected legacy source files are excluded there; all other tracked source inputs match the final model.

Scoped architecture/naming, source/document protection, owned interfaces, selected links, release formatting and commit-series gates pass. The global interface gate retains sixteen findings in byte-identical base files. clang-tidy is unavailable. The supplied skill-source package scripts and test directory are absent from this INET checkout; their attempted checks remain unavailable. No fingerprint, statistical or chart baseline was compared or changed, and no cryptographic or arbitrary real-endpoint interoperability result is claimed. Raw attempts, final outputs, hashes and the matrix are retained under `audit/dtls-extensions/`, including `c16-final-run-record.json`.

## Extension demonstration results

The [example verifier](../../../../../examples/dtls/README.md) runs one deterministic seed-zero execution per configuration. It exports engine scalars through opp_scavetool CSV-R, checks one expected run and both endpoints, and requires callback-specific outcomes for the opt-in actions. The following executed counts agree between debug and release for datagrams received by the server engine, including explicitly accepted early delivery. They are not independent repetitions or estimates of physical performance.

| Configuration | Server datagrams | Required behavior |
| --- | ---: | --- |
| Basic, Certificate, Fragmentation, Ipv6 | 20 each | Existing setup, sized authentication, fragmentation and IPv6 controls |
| LossyHandshake | 50 | Fixed-seed error/recovery demonstration |
| ProcessingCost | 4949 | Finite directional service, with pending work at the cutoff |
| Reconnect, LegacyReconnect | 19 each | Both endpoints establish twice; the second association is resumed |
| EarlyData | 20 | Explicit early intent and exactly one accepted status |
| Migration, LegacyMigration | 20 each | Both actual rebind paths validate with one establishment |
| DeferredAuthentication | 20 | One successful server authentication result after establishment |
| AlgorithmCost | 20 | Certificate AES-256/SHA-384 profile with nonzero selected service |
| PathMtu | 20 | Routed MTU 828 confirms a 770-byte application budget with CID |
| Legacy, MixedVersions | 20 each | Actual selected 1.2 record expansion and data |

All sixteen demonstrations have zero association failures in each mode. All 160 selected endpoint measurements agree between modes (`c16-examples-mode-comparison.json`). The reconnect schedule deliberately has a closed interval in which one generic source packet is discarded. The initial six counts equal their historical controls below. Final raw inputs/exports are under `audit/dtls-extensions/c16-examples-corrected-debug/` and `c16-examples-final-release/`. The execution ledger records commands and limits separately from this application demonstration.

## Pre-extension receive-error correction and review boundaries

The current suite contains 129 protocol cases: the original 125 plus four [malformed-handshake cases](../../../../../tests/protocol/dtls/malformed-handshake-records.md). A raw handshake-header conversion used to escape delayed input processing and abort the simulation. Two decode-only guards now discard those records before replay or association state changes; application callbacks remain outside the guards.

The same four full-UDP fixtures were executed on the preserved original source and the corrected source. Both zero-cost controls pass on both versions. Both 1 ms cases fail on the parent with an uncaught decoder exception and pass after the correction. Four existing input-processing controls also pass on the corrected matching debug library, along with six units and eight modules. Both build modes compile. Exact fixture hashes, source/library identity, commands and outputs are retained under `audit/dtls/revision/{parent-f1,fixed-f1}/` and the reviewer document above. The discarded initial fixture-timing/runner attempt is distinguished from the final successful parent reproduction.

The revised series introduces the directional queue and record state before the endpoint shell, common offer selection before its association consumer, and flight/reassembly helpers before recovery integration. Each preparation has its own direct test. The unit-only public flight reset method is removed; the unit constructs a fresh empty value instead. Clause references accompany normative decisions, while deliberately modeled limits remain labeled as model policy. These preparation, API-surface and comment changes introduce no baseline update.

The reconstructed pre-CLOSE correction and final union require their own boundary records. The original 125-case executions below are historical controls, not an inference that all 129 cases were run together by those earlier commands. Final reconstruction evidence and exact new commit links are kept with the PR's verification record.

## Separate behavior boundaries

Verification used OMNeT++ 6.4.0aipre2 with clang on Linux. The full isolated source `e65868cd302808b5757b68806700162853a2c120` was checked out and rebuilt before execution. Adding its reviewer documents and fixtures does not change compiled INET inputs. Build: `audit/dtls/c6-series/build-close-matching.log`; source manifest: `audit/dtls/c6-series/source-close.sha256`.

| Check | Executed evidence | Limit |
| --- | --- | --- |
| Foundation through application records | All 99 C3–C5 controls and RecordParsing pass on that isolated library. | Selected profiles, recovery, framing, datagrams, replay, processing and size/padding boundaries. |
| KeyUpdate | All 18 update cases pass on that full source and separately on the preceding immediate-close boundary. | [key-updates.md](../../../../../tests/protocol/dtls/key-updates.md) maps tests, state/cost conditions, parent comparisons and numeric probes. |
| Closure | All 7 cases pass. | [closure-and-lifecycle.md](../../../../../tests/protocol/dtls/closure-and-lifecycle.md) maps alert delivery/loss, forced/interrupted close, shared ports and receive boundaries. |
| Six units | All pass in `audit/dtls/c6-series/unit-close.log`. | Octet oracles, reconstruction, representation/ownership, profile selection, range retention and RTT accounting. |
| Eight modules | All pass in `audit/dtls/c6-series/module-close.log`. | Engine/queue/recovery cancellation, actual sender/configuration boundaries and real-host stop/close preservation, crash and repeated restart. |

The complete 125-fixture protocol suite passes in 18 bounded groups: 7 CLOSE, 18 KEYS, 99 C3–C5 controls and framing. Exact commands/results are in `audit/dtls/c6-series/close-isolated-protocol-run-record.json`; raw pairs are in `close-protocol-raw/`. Library resolution confirms the isolated INET and protocoltest libraries, with INET SHA256 `4919e089a2a0ff25d6bacbeaf9eebbfb799e49ba2f007e2a77de577720da751a`.

The preceding KeyUpdate-only boundary independently passed 118 protocol fixtures, six units and seven modules; no graceful-close fixture or state was present. Both queue preparation commits separately built and passed their direct module case. These are actual boundary checks, not an inference from the final model alone.

The combined development run first found a promotion-refusal alert sent under epoch two after server Finished handoff. The corrected server uses its already-available application sending epoch; the unchanged AssociationLimit fixture and full suite pass. Earlier parent observations and fixture-only corrections are distinguished from source-based comparisons in each reviewer document.

## Original series verification, before the receive-error correction

The original source series was rebuilt and tested in order on upstream `980f5c3f9eb903fd9155dd8d24216a1c265d5f55`. Its last tested source was `42bcad6ad65453fb32497ba88f6be43e5733b686`. Correcting one plan commit's classification trailer produced the then-public source commit `6e61239b84a8de06d18751fc8aff349c21f1e5d2` with the identical tree. The original example commit `87b64031ee68e9c59398b19fd6a71b36a0437120` added configurations, documentation and their baseline pin without changing compiled DTLS inputs. These identifiers retain their historical execution meaning; the receive-error correction and reconstructed source series are separate evidence.

| Prefix | Units | Modules | Protocol cases | Result |
| --- | ---: | ---: | ---: | --- |
| Upstream base | — | — | 2 UDP controls | PASS |
| Typed interception preparation | — | — | 3 UDP/predicate controls | PASS |
| C2 foundation | 3 | 2 | 1 | PASS |
| C3 establishment | 4 | 2 | 39 | PASS |
| C4 recovery | 6 | 3 | 77 | PASS |
| C5 datagrams | 6 | 3 | 100 | PASS |
| C6 input continuation preparation | — | 1 | — | PASS |
| C6 key updates | 6 | 7 | 118 | PASS |
| C6 atomic cancellation preparation | — | 1 | — | PASS |
| C6 graceful closure | 6 | 8 | 125 | PASS |

Nine debug builds pass. Three plan/profile prefixes have identical compiled inputs and are marked not applicable for runtime checks. Across the walk, 465 protocol, 31 unit and 27 module executions pass, with no timeout, setup error, unexpected outcome or waiver. These repeated executions include controls and do not represent 465 distinct protocol scenarios. This verifies each prefix's own capabilities; it does not supply an exact strengthened-fixture parent comparison where the reviewer documents explicitly identify a source-based comparison.

The retained record is `audit/dtls/final-series/run-record.json`, with `prefix-summary.json`, individual source inventories, raw outputs and the incremental driver. The final isolated debug library SHA256 is `12d31442cfa35d09a1ae7cc30753b1a396d71e94abdf8649663f631dc92f5685`. Message-only old/new commit mappings and equal tree hashes are in `audit/dtls/message-correction-map.json`. Local audit paths are retained execution evidence, not files distributed with INET.

With Dtls and TlsCommon disabled on the final source, debug and release builds pass and DatagramDelivery, EmptyDatagram and TypedTapPredicate pass against the disabled debug library. The feature-off commands, feature state and library hashes are in `audit/dtls/final-feature-off/run-record.md`.

Final enabled debug and release compilation pass. All six examples finish without an error in both modes, with one establishment and no association failure at each endpoint. All 48 selected endpoint counters agree between modes. Server datagram counts are Basic 20, Certificate 20, Fragmentation 20, LossyHandshake 50, ProcessingCost 4949 and Ipv6 20. Counts were exported through `opp_scavetool` CSV-R. ProcessingCost deliberately overloads service and ends with pending work at the configured cutoff. Loss-free examples may retransmit when adaptive recovery timers expire behind network-interface backlog; these runs do not assert zero retransmissions. Build logs are `audit/dtls/build-final-{debug,release}.log`; example commands and outputs are in `audit/dtls/examples-final-{debug,release}/`, with `examples-mode-comparison.json` recording the mode comparison.

## Statistical references

[CI run 43813828035](https://github.com/mgonzalezlopezudc/inet/actions/runs/43813828035) built release mode and completed exactly the six new examples. Comparisons reported missing references for those six configurations, with no other selected error. Its candidate artifact adds six files and changes none. Exported endpoint counters agree with the debug example counts above. The [statistics proposal](https://github.com/inet-framework/statistics/pull/1) records the actual CI files and environment at `3d3a8995a00d75f2f21ccd42a648db591974611c`, pinned by the example commit with `first-record` attribution. No earlier test values exist, so no parent run is claimed. [CI run 43815452715](https://github.com/mgonzalezlopezudc/inet/actions/runs/43815452715) passes the attribution tool against the exact example commit and pin: all six selected statistical comparisons pass. Its `dtls-attribution` artifact holds build, baseline checkout and comparison logs. The temporary validation branch changes only the workflow used to invoke that check; those workflow edits do not enter the implementation series.

The CI recording environment is OMNeT++ `66ab32fb913825fa0371b42cb1397c6ffff5fb73` (6.5.0 internal), clang 18.1.3, Ubuntu 24.04.5 and opp_repl `0.6.dev31+g155ac508b`. The baseline commit also records the Python package versions. These first records detect future trajectory changes; they do not establish statistical confidence or standards correctness.

## Review and mechanical checks

Independent correctness and final checklist review found no remaining actionable defect in the selected behavior. Owned architecture, naming and interface scopes pass. Source-seal, seal-index and whitespace checks pass. Whole-tree architecture and naming findings match the upstream base exactly. The same 16 existing interface violations remain; the new interface adds one passing class. The full documentation link check has 181 pre-existing broken links versus 182 at the base and adds none; all DTLS-specific document scopes pass.

The C++ gate's raw exit status is one, not a clean pass. Its final owned diagnostic set contains fourteen easily-swappable-parameter warnings whose actual calls were reviewed, and the sanctioned named-instance spelling `dtlsProtocol`. No actionable owned diagnostic remains. Generated code and existing dependency diagnostics are outside that owned set. No suppression or exception-ledger change was introduced. The classification trailer correction passes its gate; final publication gates are recorded separately.

Cryptography remains the IPsec retained-content abstraction. Actual AEAD, key derivation, signature/certificate validation, protected ciphertext export and authenticated far-old alias disambiguation remain outside scope. CID/migration was outside the historical initial implementation; its current finite coverage is recorded in the extension ledger. No existing baseline was changed by these historical results.
