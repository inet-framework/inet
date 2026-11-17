# DTLS standards and selected profile

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

The base specification is [RFC 9147](https://www.rfc-editor.org/rfc/rfc9147.html), April 2022. Its TLS 1.3 dependency was originally [RFC 8446](https://www.rfc-editor.org/rfc/rfc8446.html), August 2018; [RFC 9846](https://www.rfc-editor.org/rfc/rfc9846.html), July 2026, replaces that document with a backward-compatible revision. Source texts were downloaded from the RFC Editor to the sibling `standards/RFC/` repository on 2026-11-16. The catalogs paraphrase selected requirements; they do not reproduce the standards.

## Selection

The selected profile is DTLS 1.3 over UDP, with TLS_AES_128_GCM_SHA256, external PSK with psk_dhe_ke, or certificate authentication with X25519 key shares and ecdsa_secp256r1_sha256 signatures. Initial client certificate authentication is included. Connection IDs, migration, tickets/resumption, early data, and post-handshake certificate authentication are excluded. This is a selected behavioral profile, not a claim of exhaustive conformance.

| Document | Role | Selected clauses |
| --- | --- | --- |
| RFC 9147 | DTLS record and handshake transport | Sections 4–8 and 10 |
| RFC 8446 | Original TLS handshake/body definitions | Sections 4, 5.2, 5.5, 6 and appendix B |
| RFC 9846 | Current TLS 1.3 clarification | Sections 1.2, 4.1, 4.7.3, 5.5 and 6 |
| RFC 9853 | Updates RFC 9147 for CID path validation | Excluded because the profile does not negotiate CID |

DTLS-specific framing, epochs, acknowledgements, and delayed sending-key transitions override the corresponding TLS record-delivery rules. In particular, a DTLS KeyUpdate does not enable sending under the next epoch until its ACK has arrived.

## Reconciliation with current TLS

RFC 9846 section 1.2 lists the changes from RFC 8446. Applicable clarifications are fresh key shares per association, mandatory update before usage exhaustion, a warning-level close_notify, ignoring NewSessionTicket when resumption is unsupported, and permitting zero bytes of CertificateRequest extensions. Cryptographic hash/key derivation changes do not establish a cryptographic claim in an abstraction that does not compute those values.

The revised KeyUpdate rule additionally prohibits another update_requested until a subsequent peer KeyUpdate has been received. Receiving an ACK alone does not satisfy this condition. The independent DTLS rule still prohibits a second unacknowledged outgoing update. Responses use update_not_requested.

The RFC Editor metadata was checked on 2026-11-16. Future errata or extensions must be assessed against their affected clause before widening this profile. No exclusion of a selected mandatory behavior may be justified solely by a failed check.

## Verification depth

The target includes normal paths, fault injection and timer behavior for the selected mechanisms. It is not an assertion that every mandatory statement of every companion document has been cataloged. Achieved coverage and execution results belong to the [model ledger](../../model/dtls/coverage.md), not this specification-derived map.
