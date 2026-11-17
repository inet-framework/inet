# DTLS model coverage ledger

> **Kind:** ledger · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [standards.md](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/doc/project/evidence/protocol/dtls/standards.md)

The selected behavioral profile and exclusions are in [scope.md](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/doc/project/evidence/model/dtls/scope.md). Execution evidence is recorded in [results.md](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/doc/project/evidence/model/dtls/results.md). A catalog entry or a planned test is not executed conformance evidence.

## Initial selection

| Feature | Planned evidence | Current result |
| --- | --- | --- |
| Record framing | Rfc9147RecordParsing; serializer/state unit cases | NOT_RUN |
| Establishment/cookie/negotiation | Rfc9147Handshake, CookieRetry, NegotiationFailure | NOT_RUN |
| Peer independence | Rfc9147AssociationIsolation | NOT_RUN |
| Flight recovery | Rfc9147HandshakeLoss, FragmentReassembly, SelectiveAck | NOT_RUN |
| Datagrams and overhead | Rfc9147ApplicationDatagrams, RecordOverhead | NOT_RUN |
| Replay/invalid input | Rfc9147ReplayWindow, InvalidRecord | NOT_RUN |
| Updates and close | Rfc9147KeyUpdate, Close | NOT_RUN |
| Lifecycle and processing | DtlsEngineLifecycle, DtlsProcessingQueue | NOT_RUN |

No achieved standards-test depth is claimed until the selected cases have executed and their observations have been examined. Cryptographic validation, protected-packet byte export, CID/migration and the omitted optional handshake modes are not supported claims.
