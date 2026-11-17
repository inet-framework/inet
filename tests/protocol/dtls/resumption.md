# DTLS 1.3 ticket resumption

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [extension plan](../../../plan/pending/dtls-extensions.md), [ticket obligation](../../../doc/project/evidence/standard/rfc9147/extensions.md#dtls-ext-ticket)

C3 adds optional, stateful session tickets under RFC 9147 §5.8.4 and RFC 9846 §§4.2.11/4.7.1. Previously the engine could establish only through its configured full-handshake authentication. With `resumptionEnabled=true`, the server issues one ticket per association. A later `open(peer, port, callback, true)` offers the newest compatible cached ticket. `isResumed(handle)` reports selection after establishment. Existing callers retain their original open operation; `DtlsSocketIo.resumeSession` exposes the opt-in to composed applications. Callback implementations require no change; custom `IDtls` implementations must supply the two new pure operations.

```mermaid
sequenceDiagram
    participant C as Client engine
    participant S as Server engine
    C->>S: Full handshake (certificate or external PSK)
    S->>C: NewSessionTicket
    C->>S: ACK (one fixture drops this)
    Note over C,S: Association closes; each engine retains its bounded session row
    C->>S: ClientHello, cached ticket identity, key share
    S->>C: ServerHello, selected ticket PSK, key share
    S->>C: EncryptedExtensions, Finished
    C->>S: Finished
    Note over C,S: Application datagrams use the new association
```

Tickets use the actual NewSessionTicket field layout, including nonce, lifetime, age-add and opaque identity. The engine table shares its configured capacity between client and server entries, removes expired rows on insertion, and evicts the oldest admitted row when full. Duplicate delivery does not extend expiry. The identity combines the issuing engine's component ID and lifetime serial; stop/start clears rows while preserving the serial. This is a simulation namespace and stateful lookup, not encrypted tickets or cryptographic authentication. Ordinary close retains rows; stop and crash erase them.

Once issuance consumes a handshake sequence number, output admission failure or an unacknowledged sending deadline fails that association; abandoning the sequence would obstruct later ordered handshake messages. A client may ignore a received ticket when local processing admission or its receive deadline expires, advancing that received sequence without caching it. These are finite model failure policies.

All production cases below use two Ethernet-connected `StandardHost` nodes with an intervening `PacketTap`, 1 ms links, seed 0, declared checksums/FCS and a finite 10–12 s limit. The default cookie policy remains enabled. Full and resumed handshakes use PSK-DHE when a PSK is selected; tickets do not promise a shorter exchange than an initial external-PSK handshake.

| Case | Setup, stimulus and required observations |
| --- | --- |
| [Rfc9147Resumption.test](Rfc9147Resumption.test) | Certificate profile at both ends. Observe the initial Certificate and ticket; drop the first ticket ACK, then request a server KeyUpdate. The ticket retransmits with a fresh record number under its original epoch after the update. Close, wait for closure, explicitly resume, observe no Certificate in the resumed exchange, and require both peers to report ticket selection plus exactly one marker 42 delivery. |
| [Rfc9147PskResumption.test](Rfc9147PskResumption.test) | Same ACK-loss/update/reconnect stimulus with external PSK as the full-handshake origin. Require ticket selection on the second association and exactly one application delivery. No certificate-flight reduction is claimed for this origin. |
| [Rfc9147ResumptionExpired.test](Rfc9147ResumptionExpired.test) | One-second ticket lifetime; reconnect after expiry. The client cannot select the expired row. Both peers report a full certificate handshake and deliver marker 43. |
| [Rfc9147ResumptionUnknown.test](Rfc9147ResumptionUnknown.test) | Alter one byte of the offered opaque ticket identity, preserving the vectors. The enabled server has no matching row, declines ticket selection, completes ordinary certificate authentication and delivers marker 43. |
| [Rfc9147ResumptionIssuerIsolation.test](Rfc9147ResumptionIssuerIsolation.test) | Two independent server engines use equal identity labels and issue serial 1. Both have live rows. Replace server B's offered ticket with A's captured identity. B must complete non-resumed certificate authentication and deliver marker 44. This checks issuer namespaces, not a cryptographic forgery oracle. |
| [Rfc9147ResumptionServerLifecycle.test](Rfc9147ResumptionServerLifecycle.test) | Client retains tickets while `ScenarioManager` stops/restarts, then crashes/restarts the server. Re-register the listener and reconnect after each operation. Both attempts must fall back to full authentication; all three issued identities must differ. |
| [Rfc9147ResumptionTimeout.test](Rfc9147ResumptionTimeout.test) | Two clients share one server engine. Drop every ticket destined for the first client's port and use a 0.5 s handshake/exchange deadline. The first association reports the ticket timeout; the second client's ticket succeeds and marker 45 is delivered through the same server afterward. |
| [Rfc9147ResumptionReceiveDeadline.test](Rfc9147ResumptionReceiveDeadline.test) | Charge one second of client ticket input processing against a 0.5 s receive deadline. Require that cost to start, let the delayed continuation finish, then close and reconnect. The expired receive operation must not cache a ticket or invoke a late outcome; full certificate fallback still delivers marker 43. |
| [DtlsSessionLifecycle_1.test](../../module/DtlsSessionLifecycle_1.test) | Enable ticket reconnects in the existing real-peer restart fixture. Stop/crash the client, including queued protection, and establish four times. Every restarted client performs full authentication because its own cache was erased. Original alert/queue cleanup assertions remain active. |
| [DtlsSessionStore_1.test](../../unit/DtlsSessionStore_1.test) | Exercise disabled storage, aggregate client/server capacity, oldest-row eviction, duplicate expiry preservation, exact expiry boundary, newest compatible client selection, identity/port/version filtering and clear. |
| [DtlsTicketSelection_1.test](../../unit/DtlsTicketSelection_1.test) | Use the production ClientHello validator with a known server row. Require ticket mode for compatible SHA-256; require certificate fallback for SHA-384 in the current single-hash profile, mismatched identity and an unknown row. This is helper coverage; C5 owns production alternate-hash exchanges. |
| [TlsNewSessionTicket_1.test](../../unit/TlsNewSessionTicket_1.test) | Byte-exact field encoding, full decoding, every truncated prefix and empty-ticket rejection. Reassembly coverage also checks that discarding one sequence preserves an unrelated reservation. |

Existing full-handshake, client-authentication, simultaneous-update, KeyUpdate, shared-peer and shared-port cases remain preservation controls. The new cases require APIs and NED parameters absent from the exact parent, so no parent wire-failure result is claimed. Executed debug results and exact selectors are in the [extension execution record](../../../doc/project/evidence/model/dtls/extensions.md). Release mode, exact-parent comparisons, fingerprints and diagram rendering are not covered by these checks. Early data and additional hashes are later stages.
