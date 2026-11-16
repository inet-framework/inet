# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

The C2 foundation has executed debug-mode evidence. It supplies record framing, serializers, modeled protection helpers, replay arithmetic, a socket contract and engine-shell ownership. It does not establish associations or provide an application datagram service. The remaining selected protocol behavior is **NOT_RUN** at this checkpoint.

Verification ran in an isolated C2 checkout with the matching library built by `make -j8 MODE=debug`, followed by `make -j4 MODE=debug` after final source corrections. Environment: OMNeT++ 6.4.0aipre2, clang, Linux. Load `.envrc` before the following commands, from the checkout root.

| Command | Evidence and limits |
| --- | --- |
| `opp_run_unit_tests --load . -p inet -m debug --no-build -f '(DtlsRecordSerializer\|DtlsRecordState\|TlsHandshakeSerializer)_1\.test$'` | Three cases pass. Independent octet expectations, malformed/native-length rejection, truncated-number boundaries, empty protected payloads and region-tag preservation. These helper checks do not establish an exchange. |
| `opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(EngineLifecycle\|ProcessingQueue)_1\.test$'` | Two cases pass. Queue capacity/service/cancellation, pending work at destruction, initial-down/start, stop/crash/restart and delayed engine-queue cancellation. Real association/deadline cancellation is added with C3. |
| `opp_run_protocol_tests --load . -p inet -m debug --no-build -w '^tests/protocol/dtls$' -f 'Rfc9147RecordParsing\.test$'` | One case passes, seed 0. Crafted UDP input traverses the ordinary host stack and demonstrates record framing, malformed-tail isolation and unknown-epoch rejection. No establishment or application delivery occurs. |

Raw build/test logs and generated outputs are retained locally under `audit/dtls/c2-isolated/` in the implementation checkout. They are ignored artifacts, not committed fixtures. Native encoded-length checks use explicit throwing assertions that remain active under `NDEBUG`; their release-mode execution is still required before final publication. No fingerprint or existing statistical baseline is changed. No real cryptographic or interoperability claim is made.
