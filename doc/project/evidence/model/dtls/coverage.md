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
| Datagrams and overhead | The 23 C5 cases mapped in [protected-datagrams.md](../../../../../tests/protocol/dtls/protected-datagrams.md) | PASS for delivery, processing, capacity and record/padding boundaries |
| Replay/invalid input | Rfc9147ReplayWindow, ReplayWindow8, InvalidRecord, MalformedInner and RecordSizeLimits | PASS within the documented truncated-number reconstruction scope |
| Key updates | The 18 cases in [key-updates.md](../../../../../tests/protocol/dtls/key-updates.md) | PASS on the isolated update boundary |
| Graceful closure | The 7 cases in [closure-and-lifecycle.md](../../../../../tests/protocol/dtls/closure-and-lifecycle.md) | PASS for local completion, forced closure, alert disposition, shared peers and delivery boundaries |
| Lifecycle and processing | DtlsEngineLifecycle, DtlsProcessingQueue, DtlsRecoveryLifecycle, DtlsRestartLifecycle, sender/configuration cases | PASS for processing/recovery ownership, close preservation, crash and repeated restart |

No achieved standards-test depth is claimed until the selected cases have executed and their observations have been examined. Cryptographic validation, protected-packet byte export, CID/migration and the omitted optional handshake modes are not supported claims.
