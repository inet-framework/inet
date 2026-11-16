# DTLS 1.2 record foundation

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [extension profile](../../../doc/project/evidence/standard/rfc9147/extensions.md), [execution ledger](../../../doc/project/evidence/model/dtls/extensions.md)

This stage supplies representations and concrete helpers. Successful DTLS 1.2 negotiation is not enabled. The engine still negotiates only 1.3; receiving a 1.2-only offer produces `protocol_version`, not a 1.2 association. No 1.2 application delivery or flight recovery is claimed here.

## Represented fields

[RFC 6347 §§4.1–4.3](https://www.rfc-editor.org/rfc/rfc6347.html#section-4.1) supplies the shared 13-byte record header: content type, reversed wire version, full 16-bit epoch, full 48-bit sequence and length. The existing 12-byte handshake header and ClientHello cookie field are reused. Generic dissection recognizes legacy CCS and application records in addition to handshake/alert records; it does not enable their reception by the engine.

The selected GCM shape has an eight-byte explicit nonce and sixteen-byte tag, following [RFC 5288 §3](https://www.rfc-editor.org/rfc/rfc5288.html#section-3). The model chooses epoch concatenated with the record sequence as the nonce. `Dtls12RecordProtection` retains immutable plaintext rather than encrypting it, adds no 1.3 inner content-type octet, preserves region tags and handles empty content. Its inverse checks the selected nonce convention, byte/size limits and retained metadata before changing the caller's packet. This is modeled structural validity, with no cryptographic verification. Real raw ciphertext cannot be recovered through this helper.

Concrete chunks cover HelloVerifyRequest, one opaque modeled Certificate or an empty list, the separate PSK/ECDHE server/client key exchanges, CertificateRequest, ServerHelloDone, CCS and the explicit nonce. CertificateVerify and the twelve-byte Finished placeholder reuse field-compatible TLS bodies. [RFC 5246 §7.4](https://www.rfc-editor.org/rfc/rfc5246.html#section-7.4), [RFC 4279 §2](https://www.rfc-editor.org/rfc/rfc4279.html#section-2) and [RFC 8422 §5](https://www.rfc-editor.org/rfc/rfc8422.html#section-5) define the bodies. The planned PSK/GCM and ECDHE-ECDSA/GCM suites come from [RFC 5487](https://www.rfc-editor.org/rfc/rfc5487.html) and [RFC 5289](https://www.rfc-editor.org/rfc/rfc5289.html). The selected certificate profile uses secp256r1's 65-byte uncompressed point and ECDSA/SHA-256. Serializer vector framing permits other encoded group/signature values so the later handshake consumer can reject unsupported selections. Multiple certificate chains, real DER, cryptographic signatures and unrestricted real-peer compatibility remain excluded.

Legacy Hello bodies explicitly distinguish an omitted extension vector from an encoded empty vector. Omission is accepted only with `0xfefd`; 1.3 uses that same legacy version but still requires its mandatory extensions at negotiation. Default-present fields preserve existing outgoing encodings. Empty PSK identity is outside the selected local profile and is rejected; an empty server identity hint is represented. Certificate authorities are an opaque encoded distinguished-name sequence; later negotiation supports the empty sequence only.

## Direct checks

| Artifact | Setup, stimulus and required observation |
| --- | --- |
| `tests/unit/Dtls12RecordSerializer_1.test` | Independent header/nonce octets, CCS=1, malformed CCS/truncated nonce, maximum epoch/sequence and overflow rejection. Dissect concatenated CCS/application records and require both headers without an incorrect marker. |
| `tests/unit/Dtls12HandshakeSerializer_1.test` | Independent byte vectors for every new nonempty body, empty Certificate/hint, the selected 65-byte EC point, reused CertificateVerify/Finished, absent/present legacy extensions and malformed lengths. Serialize ServerHelloDone as zero bytes. Reject omission under the TLS 1.3 legacy version. |
| `tests/module/Dtls12RecordProtection_1.test` | A standalone module calls the concrete helpers with content/tags, empty content and the maximum content size. Check actual 24-byte overhead, expected nonce, failed nonce/epoch unprotection without mutation, 48-bit overflow and byte-alignment rejection, truncated/raw input, and duplicate/window behavior with full record numbers. These are helper checks, not wire exchange evidence. |
| `Rfc9147UnsupportedVersion.test` | Two StandardHosts and PacketTap on two 1 ms half-links, seed 0, declared checksum/FCS modes and GlobalArp. Mutate a real ClientHello's supported-versions offer to 1.2 only. Require server alert 70, one client error, zero establishments/deliveries and zero available application size. |
| Existing 1.3 controls | The ordinary certificate and PSK handshakes, initial client authentication and delayed range-ACK PMTU/PHA fixture use production UDP/IP/Ethernet. They check continued success after the shared Hello representation changes. |

The exact selections, executed outcomes, build mode and limits belong to the linked execution ledger. No helper result proves a working 1.2 handshake, replay integration or application path; those are later stages.
