# Finite DTLS 1.3 algorithm profiles

> **Kind:** report · **Status:** snapshot 2026-11-16 · **Seal:** none · **Owns:** — · **Stands on:** [extension plan](../../../plan/pending/dtls-extensions.md), [profile obligation](../../../doc/project/evidence/standard/rfc9147/extensions.md#dtls-ext-profile)

C5 replaces the single hard-coded selection with an explicit finite catalog. Configure ordered `cipherSuites`, `keyExchangeGroups` and `signatureSchemes` lists; the server selects its first mutually supported value. Empty, unknown or duplicate entries fail initialization. Defaults remain AES-128-GCM/SHA-256, X25519 and ECDSA-P256/SHA-256. The external PSK remains bound to SHA-256; configuring it with only SHA-384 suites fails initialization. Certificate authentication and compatible tickets can select SHA-384.

| Catalog dimension | Represented sizes and policy |
| --- | --- |
| `TLS_AES_128_GCM_SHA256` / `TLS_CHACHA20_POLY1305_SHA256` | 32-byte Finished and PSK binder; 16-byte tag. |
| `TLS_AES_256_GCM_SHA384` | 48-byte Finished and matching ticket binder; 16-byte tag. |
| `x25519` / `secp256r1` | 32-byte shares / 65-byte uncompressed shares beginning with 0x04. These are placeholders, not validated curve points. |
| `ecdsa_secp256r1_sha256` / `rsa_pss_rsae_sha256` | Configured ECDSA signature size / a fixed modeled RSA-2048 label with 256 signature bytes. Certificate bodies remain configured identity bytes, not DER. |
| Every selected suite | Conservative record limit 2^24 and rekey threshold no greater than 2^23, with the existing sending-epoch bound. The catalog lists the policies explicitly; it does not compute a security proof. |

The association retains one negotiated selection, including its authentication mode and PSK index. Initial client-authentication signature selection is separate from the server's signature. Retry records carry the selected suite and requested group. A retained HRR cannot be completed with a changed suite or an unrequested share. Clients check received selections against their actual offer and verify represented signature lengths. Ticket rows save the selected suite and hash; cache selection skips unsupported hashes. A different same-hash suite can resume, while early acceptance requires the original saved suite.

Each suite exposes four optional processing parameters. Prefix `aes128Gcm`, `aes256Gcm` or `chacha20` to `HandshakeInDelay`, `HandshakeOutDelay`, `ProtectInDelay` or `ProtectOutDelay`. Defaults evaluate the existing corresponding base delay at the original sampling boundary. Before negotiation, ClientHello/HRR work and detached alerts use base delays. Early protection uses the cached ticket suite. Negotiated logical handshake operations and records use the selected suite's override. There is no hardware-performance interpretation or extra algorithm plugin layer.

All production fixtures use StandardHosts, the existing Ethernet PacketTap fixture, 1 ms links, seed 0, declared checksums/FCS and finite limits. They run real UDP-backed engines. A three-association fixture combines dimensions without a full production Cartesian product; unit selection checks enumerate every supported combination.

| Case | Setup, stimulus and required observations |
| --- | --- |
| [Rfc9147AlgorithmProfiles.test](Rfc9147AlgorithmProfiles.test) | Three concurrent certificate associations select AES256/secp256r1/RSA, ChaCha/X25519/ECDSA and AES128/secp256r1/RSA. The first requests initial client authentication. Different group preference orders require a genuine retry on the second. Observe all suite codes, exact share shapes, RSA/ECDSA and Finished lengths; deliver 1178 application bytes and refuse 1179 against a 1200-byte UDP budget for each profile. |
| [Rfc9147AlgorithmFragmentation.test](Rfc9147AlgorithmFragmentation.test) | Repeat the three profiles with a 512-byte UDP limit, splitting the modeled certificate while retaining exact signature/Finished observations. Every peer establishes and delivers its marker. |
| [Rfc9147Sha384Resumption.test](Rfc9147Sha384Resumption.test) | Repeat the ticket ACK-loss, KeyUpdate and certificate-origin reconnect fixture under AES256/secp256r1/RSA. Require cached ticket selection and ordinary delivery with the 48-byte hash profile. |
| [Rfc9147Sha384EarlyData.test](Rfc9147Sha384EarlyData.test) | Certificate-origin AES256/X25519/ECDSA with a deliberately small 32-byte certificate allows the initial cookie-free full flight within amplification credit. Explicit early delivery, duplicate filtering, local allowance refusal and ordinary delivery retain their separate outcomes. |
| [Rfc9147EarlyDataChangedSuite.test](Rfc9147EarlyDataChangedSuite.test) | Obtain an AES128 ticket, then remove that suite from the transmitted reconnect offer while retaining allowed ChaCha. Observe ChaCha selection, successful same-hash ticket resumption and negative early admission. No rejected early payload is retried. |
| [Rfc9147AlgorithmProfileCosts.test](Rfc9147AlgorithmProfileCosts.test) | Override only the AES256 cost parameters in the existing full-duplex fixture. Exact 5 ms output handoffs and independent 3 ms receive service preserve the expected 10/15 ms delivery offsets for two datagrams in each direction. Base delays remain zero. |
| [Rfc9147AlgorithmUpdateCosts.test](Rfc9147AlgorithmUpdateCosts.test) | Override AES256 logical handshake input service. Original, pending and processed fresh-numbered KeyUpdate duplicates produce exactly one 30 ms logical charge, ACK every copy and advance the receive epoch once. |
| [Rfc9147AlgorithmRetrySelection.test](Rfc9147AlgorithmRetrySelection.test) | Observe AES128 selection in HRR, then remove it from the retry offer while leaving allowed ChaCha. The server must reject that changed selection; the client reports one error and never establishes. |

`TlsHandshakeParameters_1` enumerates the three suites, two groups and two signatures, PSK hash compatibility, catalog policies and invalid preference lists. `TlsHandshakeSerializer_1` checks RSA field bytes and 48-byte Finished round-trips. `DtlsRecordSerializer_1` verifies each catalog entry's common tag/budget mapping. Store and ticket-selection units check hash-filtered cache selection and incompatible ticket refusal. Existing unsupported-suite, unoffered-selection, retry, full-duplex, duplicate-cost, ticket, early-data and lifecycle cases remain controls.

These fixtures need new parameters or APIs absent from the exact parent, so no measured parent wire-failure result is claimed. The [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md) records commands, counts and remaining validation. Cryptographic authentication, point validation, real interoperability and release/fingerprint comparisons are outside these results.
