# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

The C2–C5 foundation, association service, handshake recovery and application-record matrix have executed debug evidence. The model establishes the selected PSK/certificate profiles, recovers disrupted handshakes and preserves UDP application semantics with replay, record-size and processing checks. Key updates and graceful close remain **NOT_RUN**.

Verification used OMNeT++ 6.4.0aipre2 with clang on Linux, from the checkout root after loading `.envrc`. Production source was frozen before `make -j4 MODE=debug` completed. The current log is `audit/dtls/build-c5-immutable-payload.log`; its source manifest is `audit/dtls/source-c5-immutable-payload.sha256`.

| Check | Executed evidence | Limit |
| --- | --- | --- |
| C2 isolated foundation | Three units, two modules and RecordParsing pass on the separately built C2 foundation. Logs: `audit/dtls/c2-isolated/`. | Local framing, API and queue contracts, with no handshake claim. |
| C3 establishment regression | All 38 cases pass, seed 0. Logs: `audit/dtls/protocol-c5-regression-c3-batch0.log` through `batch4.log`. | Selected profiles, invalid-input boundaries, amplification and peer isolation. See [association-establishment.md](../../../../../tests/protocol/dtls/association-establishment.md). |
| C4 recovery regression | All 38 cases pass, seed 0. Logs: `audit/dtls/protocol-c5-regression-c4-batch0.log` through `batch4.log`. | Deterministic loss, fragmentation, ACK accounting, timing, retention and processing deadlines. See [handshake-recovery.md](../../../../../tests/protocol/dtls/handshake-recovery.md). |
| C5 application records | All 23 cases pass, seed 0. Logs: `audit/dtls/protocol-c5-immutable-final-batch0.log` through `batch3.log`. | Delivery/reordering/loss, replay, capacity, directional costs, size limits, padding and rejection accounting. See [protected-datagrams.md](../../../../../tests/protocol/dtls/protected-datagrams.md). |
| Six units | RecordSerializer, RecordState, TlsHandshakeSerializer, TlsHandshakeParameters, HandshakeReassembly and HandshakeFlight pass. Log: `audit/dtls/unit-c5-immutable-payload.log`. | Independent octets, representation/ownership, reconstruction/replay, selection, range retention and RTT. The extended padding check retains the actual payload chunk and region tags and rejects invalid inner representations atomically. |
| Three modules | EngineLifecycle, ProcessingQueue and RecoveryLifecycle pass. Log: `audit/dtls/module-c5-immutable-payload.log`. | Real queues and stop/crash cancellation, including recovery timers. C6 still extends graceful shutdown/restart. |

All 99 selected protocol cases pass on the same current library. Raw outputs are under `audit/dtls/c5-immutable-final/` and `audit/dtls/c5-regression/`. Groups contain at most eight cases and have an operating-system timeout.

The lost-ClientHello fixture failed its retry observation on the frozen C3 binary and passes with C4 recovery. The original twenty C5 fixtures already pass on the frozen C4 capture, before their new bit-size/queue-counter assertions. Those runs establish controls, not missing parent application delivery. Exact extended-fixture parent comparisons remain source-based or unexecuted, as distinguished in the reviewer documents.

Two large-record fixtures initially failed before UDP delivery because their MTU override exceeded EthernetEncapsulation's fixed payload cap. Removing that override lets normal IPv4 fragmentation/reassembly carry large UDP records while DTLS's configured budget remains large. Both corrected fixtures pass. A unit separately caught a payload copy from mutable chunk extraction; using an immutable view now preserves exact chunk identity and tags. Raw diagnostic failures remain under `audit/dtls/`; none is counted as a passing execution.

Independent review found no remaining actionable defect in the bounded C5 diff after corrections. Scoped architecture, naming, interface and source-protection checks pass. The separate C++ gate's mechanical corrections remain pending. The existing established fatal-alert epoch correction is assigned to C6, together with post-handshake alert handling and changing application epochs.

Protected content uses IPsec-style abstract cryptography, with no actual AEAD, binder, signature or certificate validation. Release compilation/execution, feature-off builds, later-stage tests, new-example first-record baselines and final publication gates remain pending. No existing baseline was changed.
