# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

C2–C5 and the independent KeyUpdate review unit have executed debug evidence. This boundary retains the prior immediate-close behavior; graceful closure and its lifecycle extensions are not implemented here.

Verification used OMNeT++ 6.4.0aipre2 with clang on Linux. The isolated source at `aa1a69c4431a1be0f81c299a5f5b79df8eeae991` was checked out and rebuilt before execution. Adding these reviewer documents and fixtures does not change its compiled INET inputs. Build: `audit/dtls/c6-series/build-keys-matching.log`; source manifest: `audit/dtls/c6-series/source-keys.sha256`.

| Check | Executed evidence | Limit |
| --- | --- | --- |
| Foundation through application records | All 99 C3–C5 controls and RecordParsing pass on this isolated library. | Selected profile, recovery, framing, delivery/replay/cost and size/padding boundaries. Earlier isolated C2 evidence remains under `audit/dtls/c2-isolated/`. |
| KeyUpdate | All 18 cases pass, seed 0. Exact commands/group status: `audit/dtls/c6-series/keys-isolated-protocol-run-record.json`; raw: `keys-protocol-raw/`. | The owning [reviewer document](../../../../../tests/protocol/dtls/key-updates.md) maps every stimulus, assertion, timing condition and model limitation. |
| Six units | All pass in `audit/dtls/c6-series/unit-keys.log`. | Record/body octets, reconstruction, immutable representation, selection, range retention and RTT accounting. |
| Seven modules | All pass in `audit/dtls/c6-series/module-keys.log`. | Existing engine/queue/recovery cancellation plus actual sender-state and three rekey configuration fixtures. The later restart/close module is absent from this boundary. |

All 118 selected protocol fixtures pass in 17 bounded groups. None is a close fixture. The command record retains exact working directory, mode, selectors, seed and exit statuses; `keys-library-resolution.log` records resolution to the isolated INET and protocoltest libraries and the INET binary SHA256.

Initial versions of twelve update fixtures failed on C5 because its request method emitted no update. Later additions and strengthened assertions use source-based comparisons unless their exact execution is recorded. The original twenty C5 cases were controls already passing before their boundary extensions. These distinctions prevent a passing integration matrix from becoming a claim that every control repaired a parent defect.

Independent review found no remaining actionable update defect after corrections, including epoch aliases, stage ordering, peer-response barriers, request admission and callback ownership. The final C++ gate disposition and publication checks are recorded separately from this behavioral checkpoint.

Cryptography remains IPsec-style retained content. Real key derivation, AEAD, signatures and certificate validation are absent. Full protected-packet export, CID/migration and far-old authenticated alias disambiguation remain outside scope. No existing baseline was changed.
