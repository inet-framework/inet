# TLS 1.3 selected handshake requirements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8446-*` · **Stands on:** [standards.md](../../protocol/dtls/standards.md)

These paraphrases select the original TLS 1.3 content inherited by DTLS. RFC 9846 replaces RFC 8446; the [standards map](../../protocol/dtls/standards.md) records current clarification and the DTLS-specific overrides. This is not an exhaustive TLS conformance catalog.

## Index

| Identifier | Requirement |
| --- | --- |
| [RFC8446-NEGOTIATION](#rfc8446-negotiation) | Select only offered compatible parameters |
| [RFC8446-MESSAGES](#rfc8446-messages) | Handshake bodies, extension vectors and ordering |
| [RFC8446-RETRY](#rfc8446-retry) | One combined HelloRetryRequest |
| [RFC8446-PROTECTION](#rfc8446-protection) | Inner type, padding, tag and usage limits |
| [RFC8446-UPDATE](#rfc8446-update) | KeyUpdate request/response flags |
| [RFC8446-ALERT](#rfc8446-alert) | Closure and fatal error distinctions |

### RFC8446-NEGOTIATION

**The server selects offered parameters and an acceptable authentication mode.** Sections [4.1–4.2](https://www.rfc-editor.org/rfc/rfc8446.html#section-4.1) define version, suite, key-share, signature and PSK negotiation. No common version produces protocol_version; no common suite produces handshake_failure; an unoffered selection is illegal_parameter.

### RFC8446-MESSAGES

**Handshake bodies have explicit vector lengths and a prescribed order.** [Section 4](https://www.rfc-editor.org/rfc/rfc8446.html#section-4) and appendix B define ClientHello, ServerHello, encrypted extensions, certificate messages and Finished. PSK authentication omits certificate messages; CertificateRequest controls initial client authentication.

### RFC8446-RETRY

**One HelloRetryRequest combines the server's retry conditions.** [Section 4.1.4](https://www.rfc-editor.org/rfc/rfc8446.html#section-4.1.4) constrains the second ClientHello and requires unexpected_message on a second retry request.

### RFC8446-PROTECTION

**Protected content includes its inner content type and optional zero padding, with the negotiated AEAD expansion.** [Section 5.2](https://www.rfc-editor.org/rfc/rfc8446.html#section-5.2) defines content layout; section 5.5 defines per-key usage bounds. RFC 9846 strengthens initiating updates before those bounds.

### RFC8446-UPDATE

**A requested KeyUpdate response uses update_not_requested before further application data.** [Section 4.6.3](https://www.rfc-editor.org/rfc/rfc8446.html#section-4.6.3) rejects invalid request flags. RFC 9846 section 4.7.3 additionally disallows another update_requested before receiving a subsequent peer update. DTLS delays the sending-epoch transition until ACK.

### RFC8446-ALERT

**Closure and fatal handshake errors have distinct alerts.** [Section 6](https://www.rfc-editor.org/rfc/rfc8446.html#section-6) supplies alert descriptions. RFC 9846 restores explicit warning-level close_notify and clarifies user_canceled handling. DTLS supplies unreliable delivery and record-order closure semantics.
