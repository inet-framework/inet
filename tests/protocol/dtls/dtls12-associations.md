# DTLS 1.2 associations and mixed versions

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [extension profile](../../../doc/project/evidence/standard/rfc9147/extensions.md), [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md)

This is C13, `dtls: add: negotiate and exchange DTLS 1.2 associations`, in the [implementation plan](../../../plan/pending/dtls-extensions.md). The concrete owner is the existing `DtlsAssociation`, with version-specific handshake and record members in `Dtls12Handshake.cc` and `Dtls12Record.cc`; `Dtls12Negotiation.cc` handles prevalidation and server creation. One production UDP socket serves both versions and independent peer handles. The [record foundation](dtls12-record-foundation.md) owns independent byte and helper checks. Runtime outcomes and exact revisions belong to the execution ledger.

## Profile and exchange

`versions = "1.3"` preserves the default. The opt-in values `"1.2"` and `"1.3 1.2"` enable the fixed PSK/GCM suite `0x00a8` or certificate ECDHE-ECDSA/GCM suite `0xc02b`. The certificate profile uses P-256's 65-byte uncompressed point, ECDSA/SHA-256 and configured opaque identity/signature sizes. Mixed lists prefer 1.3 in either text order. Empty, duplicate and unknown version lists fail startup.

[RFC 6347 §§4.1–4.3](https://www.rfc-editor.org/rfc/rfc6347.html#section-4.1) defines legacy headers, cookie exchange, fragmentation, sequence spaces and whole-flight recovery. [RFC 5246 §7.4](https://www.rfc-editor.org/rfc/rfc5246.html#section-7.4) defines the selected full-handshake bodies. The GCM suites follow [RFC 5487](https://www.rfc-editor.org/rfc/rfc5487.html) and [RFC 5289](https://www.rfc-editor.org/rfc/rfc5289.html). [RFC 9147 §§5.3–5.4](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.3) supplies reversed version ordering, legacy fallback and downgrade-sentinel obligations.

```mermaid
sequenceDiagram
    participant C as Client
    participant S as Server
    C->>S: ClientHello, sequence 0
    S->>C: HelloVerifyRequest, sequence 0, cookie
    C->>S: ClientHello, sequence 1, cookie
    S->>C: ServerHello, optional Certificate / ServerKeyExchange / CertificateRequest, ServerHelloDone
    C->>S: Optional Certificate, ClientKeyExchange, optional CertificateVerify
    C->>S: CCS in epoch 0, Finished in epoch 1
    S->>C: CCS in epoch 0, Finished in epoch 1
    Note over S: Establish once; retain final response for 240 s
    C->>S: Genuine repeated final flight after loss
    S->>C: Same logical response with fresh record numbers
```

Finished carries twelve placeholder bytes. CCS is its own one-byte record and consumes no handshake message sequence. Protected records carry a full sequence number, an eight-byte explicit nonce and sixteen-byte tag; a non-CID application record costs 37 bytes. Logical message generation/verification is charged once; each protected record attempt pays the configured record service. Whole-flight retransmissions bypass 1.3's ten-record ACK burst stop. The initial 1.2 retry interval is one second and backs off to sixty seconds. A 1.2-only server can request a cookie from the covered initial prefix when a mixed Hello would exceed ten fragments; full selection still requires the complete cookie Hello.

The final-response timer is independent of the handshake deadline. A genuine duplicate final message triggers retained response output without extending retention or establishing again. Expiry releases the flight while ordinary application service remains available. Stop, crash and close cancel association-owned state through the existing lifecycle paths. A fresh same-tuple connection requires cookie validation before replacing the old legacy handle.

## Topology and deterministic setup

```mermaid
flowchart LR
    A[host1: production DtlsTestApp client] ---|1 ms| T[PacketTap]
    T ---|1 ms| B[host2: production DtlsTestApp server]
```

All cases use `TapPairNetwork`, Ethernet, static IPv4 configuration with GlobalArp, declared checksums/FCS and seed 0. The client starts at 0.2 s and listener at 0.1 s. Ordinary payload budget is 1200 bytes. Full-handshake cases set 10 ms input/output handshake service, 5 ms protection output and 3 ms protection input. Recovery cases use 11/17 ms logical input/output costs so exact charge counts can distinguish logical verification from record work. The certificate-loss case uses an 8192-byte certificate and 128-byte UDP budget. The fragmented mixed-version cases use a 57-byte budget, yielding a 20-byte application maximum; the large-Hello case adds a 400-byte identity. Every finite simulation limit is in its fixture.

`Rfc6347SharedVersions.test` uses two client engines, local ports 4434 and 4436, on host1 and one dual-version server socket. Closing its legacy handle must preserve the 1.3 peer's bidirectional service. `Rfc6347FirstArrivalDeadline.test` similarly gives an expired slow peer and a later progressing peer to one server. `Rfc6347Lifecycle.test` adds ScenarioManager: shutdown/start at 1/2 s and crash/start at 4/5 s, with separate client ports. The third real handshake must still deliver after 241 further seconds, beyond old deadlines and final retention. Crash deliberately reports one error plus one close through the existing callback contract.

## Direct production checks

| Cases | Trigger and required observations |
| --- | --- |
| `Rfc6347Handshake`, `HandshakeCertificate`, `HandshakeMutual` | Select PSK, server certificate or mutual certificate authentication. Observe one cookie exchange, two CCS, two epoch-one Finished of length twelve, correct certificate counts, no unified header/1.3-only messages, one establishment each and bidirectional whole datagrams. |
| `Rfc6347Recovery`, `RecoveryServerFlight`, `RecoveryCertificate`, `RecoveryCertificateNoCookie`, `RecoveryCcsReorder` | Drop the first HVR; drop ServerHello and ServerHelloDone; drop an actual certificate fragment; or delay the first client CCS by 150 ms. Observe the selected actual output/fault, fresh retry record numbers, recovery and exactly four logical verifications per PSK endpoint, or six at the certificate client. Without cookies the certificate case requires three server/five client verifications. Repeated fragments/messages do not recharge verification. |
| `Rfc6347FinalRetention` | Suppress server Finished until 121 s. Keep the waiting client deadline at 300 s and server deadline at 120 s. A genuine repeated client final flight must recover the retained server response between 120 and 240 s, establish each endpoint once and deliver data. |
| `Rfc6347Datagrams`, `Replay`, `Close` | Drop one application datagram without retry; preserve empty and whole datagrams; reject 1164-byte/unknown-handle/unaligned sends locally; admit the 1163-byte boundary; reject in-window and old replay after seventy fresh records; exchange selected-format close_notify and refuse later sends. Empty output is exactly 37 bytes. |
| `Rfc6347ApiBounds` | Refuse KeyUpdate, early send, PHA and all dynamic CID operations synchronously. With record-use limit three, Finished consumes record zero, applications one/two deliver, and the next send closes the handle with sendError/error rather than KeyUpdate. |
| `Rfc6347MalformedCcs`, `MalformedCertificate`, `MalformedFinished`, `Renegotiation` | Mutate actual CCS to zero, certificate identity, or protected Finished length; require alerts 50/42/50 and no establishment. Place a real generated ClientHello in a retained protected-record copy with a fresh modeled nonce/sequence; require epoch-one warning 100, no error and continued reverse data delivery. This is structural model evidence, not authentication of forged ciphertext. |
| `Rfc6347Coalesced` | Drop the first standalone ServerHello and prepend its actual generated handshake message to ServerHelloDone's record, updating DTLS/UDP/IP/enclosing lengths. Require direct mixed legacy selection before one second, two CCS and bidirectional data. |
| `Rfc6347ReassemblyLimit`, `AssociationLimit`, `DeadlineBeforeHandoff`, `FirstArrivalDeadline`, `SharedVersions`, `Lifecycle` | Bound a 4000-byte certificate at a 1000-byte reassembly cap; refuse promotion beyond one established association without evicting it; expire queued output before handoff; cancel slow expired input and progress another peer; preserve the other selected version; cancel retained state across restart and retention expiry. |
| `Rfc6347CallbackClose` | Close synchronously inside the first application delivery callback with another protected input in flight. Require one original delivery, one closure and no error/late delivery after the handle becomes unavailable. |
| `Rfc9147VersionNegotiation`, `Only13`, `LegacyClient`, `Fallback`, `CertificateFallback`, `NoCookie` | Both list orders prefer 1.3; legacy clients reach dual servers; dual clients explicitly negotiate 1.2 with PSK or certificate servers, including no-cookie direct selection before one second. Inspect offered suites, required sentinel and selected wire/overhead, then deliver data both ways. |
| `Rfc9147VersionNegotiationFragmented`, `LargeHello` | Fragment direct ServerHello or exceed the initial ten-fragment burst with a large mixed ClientHello. Complete before one second without unified output, with fitting 16-byte application markers in both directions. |
| `Rfc9147VersionNegotiationReject12`, `Reject13`, `Unoffered`, `Downgrade`, `Timeout` | Refuse unsupported/unoffered versions; mutate the initial offer to expose the sentinel check; or drop all server traffic. Require explicit errors and no establishment; timeout retries must keep the original supported_versions offer rather than silently falling back. |

All names in this table are `.test` files in this directory; grouped suffixes retain their row's `Rfc6347` or `Rfc9147VersionNegotiation` prefix. Tests call production `IDtls` through the driver, and the tap alters actual generated packets. Units extend the existing bounded reassembler with covered-prefix and explicit empty-message admission; module cases reject invalid version lists. Exact passing selections, commands, exit statuses and raw-log locations are in the execution ledger.

## Before/after and limits

C12 has no successful 1.2 association path; new version parameters and successful fixtures are unavailable there. No parent successful wire execution is claimed. During C13, unchanged fixtures reproduced premature direct-selection retry, inherited short retry timing and acceptance of retained CCS value zero; the ledger preserves their causal pre-fix raw logs. Fixture envelope, marker-size, callback-order and crash-outcome corrections are distinguished from source defects there.

Resumption, static legacy CID and 1.2 heartbeat/path discovery are subsequent stages. Key agreement, signatures, Finished and AEAD remain sized placeholders with retained plaintext. No real confidentiality, integrity, transcript authentication, arbitrary endpoint interoperability or hardware-performance result is claimed. These IPv4 checks do not establish legacy IPv6 mobility. Mermaid rendering has not been checked.
