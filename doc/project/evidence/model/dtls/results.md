# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

The C2 foundation, C3 association service and C4 handshake recovery have executed debug-mode evidence. The model establishes external-PSK and certificate associations, performs cookie/key-share retry and initial client authentication, and recovers fragmented handshakes over disrupted UDP. The complete application-record matrix, key updates and graceful close remain **NOT_RUN** at this checkpoint.

Verification used OMNeT++ 6.4.0aipre2 with clang on Linux, from the checkout root after loading `.envrc`. Production source was frozen before `make -j4 MODE=debug` completed successfully. The current build log is `audit/dtls/build-c4-deadline-reviewed.log`; the corresponding source manifest is `audit/dtls/source-c4-deadline-reviewed.sha256`.

| Check | Executed evidence | Limit |
| --- | --- | --- |
| C2 isolated foundation | Three units, two modules and RecordParsing pass on the separately built C2 foundation. Logs: `audit/dtls/c2-isolated/`. | Local framing, API and queue contracts, with no handshake claim. |
| C3 establishment regression | All 38 cases pass, seed 0, on the current C4 library. Logs: `audit/dtls/protocol-c3-deadline-full-batch0.log` through `batch4.log`. | Selected profiles, invalid-input boundaries, amplification and peer isolation. See [association-establishment.md](../../../../../tests/protocol/dtls/association-establishment.md). |
| C4 recovery | All 38 cases pass, seed 0. The bounded full run passed 37 cases; QueuedRetryDeadline passes the same library with a corrected numeric observer. Logs: `audit/dtls/protocol-c4-deadline-full-batch0.log` through `batch4.log` and `protocol-c4-queued-retry-final.log`. | Deterministic loss, fragmentation, ACK accounting, timing, retained state and processing deadlines. See [handshake-recovery.md](../../../../../tests/protocol/dtls/handshake-recovery.md). |
| Six units | RecordSerializer, RecordState, TlsHandshakeSerializer, TlsHandshakeParameters, HandshakeReassembly and HandshakeFlight pass. Log: `audit/dtls/unit-c4-deadline-reviewed.log`. | Independent octet expectations, representation/ownership, reconstruction/replay, selection, range retention and whole-message RTT accounting. |
| Three modules | EngineLifecycle, ProcessingQueue and RecoveryLifecycle pass. Log: `audit/dtls/module-c4-deadline-reviewed.log`. | The run also passed an obsolete helper-only reassembly module, removed because the dedicated unit covers that contract. Retained modules exercise actual queues, deadlines and stop/crash cancellation, including HRR recovery. C6 still extends graceful shutdown/restart. |

Current protocol raw outputs are retained under `audit/dtls/c4-deadline-reviewed-full/` and the focused queued-retry output recorded in its reviewer document. Groups contain at most eight cases and use an operating-system timeout to bound execution.

The lost-ClientHello fixture was executed against the frozen C3 binary and failed its retry observation. It passes with C4 recovery. Other exact parent comparisons remain source-based or unexecuted; positive C3 tests do not establish loss recovery. Development failures and fixes are recorded in the owning reviewer documents without treating setup failures as protocol results.

The final queued-retry diagnostic initially missed a real processing-start signal because it compared a converted simulation-time value with the literal double 0.8 exactly. A concurrently registered observer with a one-nanosecond numeric tolerance observes both service starts and verifies original-deadline cancellation on the unchanged binary. The four new review regressions pass; no endpoint change was made to satisfy that observation correction.

Independent static review of the C2/C3 foundation and complete C4 recovery paths found no remaining actionable correctness defect after corrections. Scoped architecture, naming, interface and source-protection checks pass. The separate C++ gate identified mechanical corrections that remain pending; it is not reported as clean.

Protected content uses IPsec-style abstract cryptography. There is no actual AEAD, binder, signature or certificate validation. Release compilation/execution, feature-off builds, later-stage tests, new-example first-record baselines and final publication gates remain pending. No existing baseline was changed.
