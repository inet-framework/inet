# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

The selected DTLS protocol behavior through graceful closure has executed debug evidence. KeyUpdate and graceful closure were verified at separate source boundaries, with the queue mechanisms introduced before their first production consumers.

Verification used OMNeT++ 6.4.0aipre2 with clang on Linux. The full isolated source `e65868cd302808b5757b68806700162853a2c120` was checked out and rebuilt before execution. Adding its reviewer documents and fixtures does not change compiled INET inputs. Build: `audit/dtls/c6-series/build-close-matching.log`; source manifest: `audit/dtls/c6-series/source-close.sha256`.

| Check | Executed evidence | Limit |
| --- | --- | --- |
| Foundation through application records | All 99 C3–C5 controls and RecordParsing pass on the current library. | Selected profiles, recovery, framing, datagrams, replay, processing and size/padding boundaries. |
| KeyUpdate | All 18 update cases pass on the current full source and separately on the preceding immediate-close boundary. | [key-updates.md](../../../../../tests/protocol/dtls/key-updates.md) maps tests, state/cost conditions, parent comparisons and numeric probes. |
| Closure | All 7 cases pass. | [closure-and-lifecycle.md](../../../../../tests/protocol/dtls/closure-and-lifecycle.md) maps alert delivery/loss, forced/interrupted close, shared ports and receive boundaries. |
| Six units | All pass in `audit/dtls/c6-series/unit-close.log`. | Octet oracles, reconstruction, representation/ownership, profile selection, range retention and RTT accounting. |
| Eight modules | All pass in `audit/dtls/c6-series/module-close.log`. | Engine/queue/recovery cancellation, actual sender/configuration boundaries and real-host stop/close preservation, crash and repeated restart. |

The complete 125-fixture protocol suite passes in 18 bounded groups: 7 CLOSE, 18 KEYS, 99 C3–C5 controls and framing. Exact commands/results are in `audit/dtls/c6-series/close-isolated-protocol-run-record.json`; raw pairs are in `close-protocol-raw/`. Library resolution confirms the isolated INET and protocoltest libraries, with INET SHA256 `4919e089a2a0ff25d6bacbeaf9eebbfb799e49ba2f007e2a77de577720da751a`.

The preceding KeyUpdate-only boundary independently passed 118 protocol fixtures, six units and seven modules; no graceful-close fixture or state was present. Both queue preparation commits separately built and passed their direct module case. These are actual boundary checks, not an inference from the final model alone.

The combined development run first found a promotion-refusal alert sent under epoch two after server Finished handoff. The corrected server uses its already-available application sending epoch; the unchanged AssociationLimit fixture and full suite pass. Earlier parent observations and fixture-only corrections are distinguished from source-based comparisons in each reviewer document.

Independent correctness review found no remaining actionable defect in the selected behavior after corrections. Final C++ gate disposition, release/feature-off compilation, example references and publication checks are recorded with the completed branch verification rather than inferred from these debug protocol runs.

Cryptography remains the IPsec retained-content abstraction. Actual AEAD, key derivation, signature/certificate validation, protected ciphertext export, CID/migration and authenticated far-old alias disambiguation are outside scope. No existing baseline was changed.
