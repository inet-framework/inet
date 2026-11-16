# DTLS model coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](../../protocol/dtls/standards.md)

The selected behavioral profile and exclusions are in [scope.md](scope.md). Execution evidence is recorded in [results.md](results.md). A catalog entry or a planned test is not executed conformance evidence.

## Initial selection

| Feature | Planned evidence | Current result |
| --- | --- | --- |
| Record framing | Rfc9147RecordParsing; serializer/state unit cases | C2 foundation PASS |
| Establishment/cookie/negotiation | The 38 C3 cases mapped in [association-establishment.md](../../../../../tests/protocol/dtls/association-establishment.md) | PASS for the selected profile and listed invalid-input boundaries |
| Peer independence | AssociationIsolation, Multihomed, SameTupleReplacement and AmplificationIsolation | PASS |
| Flight recovery | The 38 C4 cases mapped in [handshake-recovery.md](../../../../../tests/protocol/dtls/handshake-recovery.md) | PASS for the selected recovery and resource boundaries |
| Datagrams and overhead | Rfc9147ApplicationDatagrams, RecordOverhead | Basic queued delivery PASS in C3; complete C5 matrix NOT_RUN |
| Replay/invalid input | Rfc9147ReplayWindow, InvalidRecord | NOT_RUN |
| Updates and close | Rfc9147KeyUpdate, Close | NOT_RUN |
| Lifecycle and processing | DtlsEngineLifecycle, DtlsProcessingQueue, DtlsRecoveryLifecycle | Association, processing and recovery-timer cancellation PASS; C6 graceful close/restart extensions NOT_RUN |

No achieved standards-test depth is claimed until the selected cases have executed and their observations have been examined. Cryptographic validation, protected-packet byte export, CID/migration and the omitted optional handshake modes are not supported claims.
