# DTLS verification results

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [coverage.md](coverage.md)

The C2 foundation and C3 loss-free association service have executed debug-mode evidence. The model establishes external-PSK and certificate associations, performs cookie/key-share retry, supports initial client authentication, isolates peers and reports negotiation/resource failures. Loss recovery, the complete application-record matrix, key updates and graceful close remain **NOT_RUN** at this checkpoint.

Verification used OMNeT++ 6.4.0aipre2 with clang on Linux. Commands run from the checkout root after loading `.envrc`. The final C3 source was frozen before building; its DTLS/TLS headers were touched to force every dependent object to use the same layout. `make -j4 MODE=debug` completed successfully.

| Check | Executed evidence | Limit |
| --- | --- | --- |
| Three C2 units, two C2 modules and `Rfc9147RecordParsing` | All six cases pass on the separately built C2 foundation. Logs are retained under `audit/dtls/c2-isolated/`. | Engine-shell and local helper contracts, with no handshake claim. |
| C3 protocol set | All 38 cases pass, seed 0. Exact selectors, topology, per-case assertions and bounded-batch records are in [association-establishment.md](../../../../../tests/protocol/dtls/association-establishment.md). Logs: `audit/dtls/protocol-c3-final-batch0.log` through `batch4.log` and `protocol-c3-required-certificate.log`; raw per-case outputs: `audit/dtls/c3-final/`. | Selected modeled profile, malformed-input and resource boundaries. No cryptographic verification or loss-recovery claim. |
| Four current unit cases | `opp_run_unit_tests --load . -p inet -m debug --no-build -f '(DtlsRecordSerializer\|DtlsRecordState\|TlsHandshakeSerializer\|TlsHandshakeParameters)_1\.test$'` passes, recorded in `audit/dtls/unit-c3-frozen-final.log`. | Octet oracles, native-length rejection, protection ownership/region tags, reconstruction and offer selection. |
| Two current module cases | `opp_run_module_tests --load . -p inet -m debug --no-build -f 'Dtls(EngineLifecycle\|ProcessingQueue)_1\.test$'` passes, recorded in `audit/dtls/module-c3-frozen-final.log`. | Real open, delayed output, pending application packet, stop/crash failure callbacks, rejected reconnect and absence of old deadlines. C6 extends graceful shutdown and restart coverage. |

The coalesced-amplification regression first reproduced double counting in a development snapshot: two ClientHello records shared one UDP payload, but the server emitted more than three times its actual received bytes. The corrected implementation counts that datagram once, reserves eligible output before queue admission and commits transmitted bytes at UDP handoff. The final regression passes. This is a development-snapshot comparison; the owning C3 commit's C2 parent has no handshake service.

Early compiler/setup failures and their successful retries remain in the raw logs. A prior highly concurrent test build encountered compiler errors while fixture headers changed; all affected cases passed after headers were frozen and compilation was bounded. Those setup failures are not represented as protocol failures or successful executions.

Independent static review of the C2/C3 source found no remaining actionable defect after corrections. Scoped architecture, naming and interface checks pass. Protected content retains IPsec-style abstract cryptography; no actual AEAD, binder, signature or certificate validation occurs. Release-mode execution, feature-off builds, later-stage tests and final publication checks remain pending. No existing baseline was changed.
