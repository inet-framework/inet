# DTLS extension obligation matrix

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `DTLS-EXT-*` · **Stands on:** [catalog.md](catalog.md)

This matrix selects requirements from the linked primary sources. It describes protocol obligations, not achieved implementation coverage. The [model record](../../model/dtls/extensions.md) assigns implementation and test evidence. Cryptographic calculations remain outside the selected simulation abstraction.

## Index

- [DTLS-EXT-EXCHANGE](#dtls-ext-exchange)
- [DTLS-EXT-TICKET](#dtls-ext-ticket)
- [DTLS-EXT-EARLY](#dtls-ext-early)
- [DTLS-EXT-ADMISSION](#dtls-ext-admission)
- [DTLS-EXT-PROFILE](#dtls-ext-profile)
- [DTLS-EXT-CID](#dtls-ext-cid)
- [DTLS-EXT-CID-REQUEST](#dtls-ext-cid-request)
- [DTLS-EXT-PATH](#dtls-ext-path)
- [DTLS-EXT-HEARTBEAT](#dtls-ext-heartbeat)
- [DTLS-EXT-PHA](#dtls-ext-pha)
- [DTLS-EXT-PMTU](#dtls-ext-pmtu)
- [DTLS-EXT-VERSION](#dtls-ext-version)
- [DTLS-EXT-12-RECORD](#dtls-ext-12-record)
- [DTLS-EXT-12-RECOVERY](#dtls-ext-12-recovery)
- [DTLS-EXT-12-SESSION](#dtls-ext-12-session)
- [DTLS-EXT-12-PATH](#dtls-ext-12-path)

## Selected obligations

### DTLS-EXT-EXCHANGE

[RFC 9147 §5.8.4](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.8.4): ticket, key-update, CID-update, CID-request and client-authentication exchanges recover independently. KeyUpdate/NewConnectionId/RequestConnectionId each forbid a second unacknowledged outgoing message of that type.

### DTLS-EXT-TICKET

[RFC 8446 §4.6.1](https://www.rfc-editor.org/rfc/rfc8446.html#section-4.6.1), reconciled with [RFC 9846 §4.7.1](https://datatracker.ietf.org/doc/html/rfc9846#section-4.7.1): opaque tickets retain hash/identity context and bounded validity; retransmission is one logical issuance. A selected PSK must have the suite's hash.

### DTLS-EXT-EARLY

[RFC 8446 §4.2.10](https://www.rfc-editor.org/rfc/rfc8446.html#section-4.2.10): early data uses the first offered PSK and saved early parameters. HRR rejects it. The ticket allowance counts application bytes. [RFC 9147 §§5.6, 6.1](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.6) omit EndOfEarlyData and assign epoch 1 to early data.

### DTLS-EXT-ADMISSION

[RFC 8446 §8](https://www.rfc-editor.org/rfc/rfc8446.html#section-8): connection-level early-data replay admission is distinct from record replay windows. Acceptance is not a datagram-delivery acknowledgment.

### DTLS-EXT-PROFILE

[RFC 9846 §§4.3.3, 4.3.7–4.3.9, 5.5](https://datatracker.ietf.org/doc/html/rfc9846): select only offered compatible suites, groups and signatures; the selected hash controls Finished/binder lengths.

### DTLS-EXT-CID

[RFC 9147 §9](https://www.rfc-editor.org/rfc/rfc9147.html#section-9) and [RFC 9146 §3](https://www.rfc-editor.org/rfc/rfc9146.html#section-3): IDs are directional receiver choices. Empty IDs and absent negotiation restrict updates. Immediate IDs replace the outgoing choice; spare IDs can be retained within local bounds.

### DTLS-EXT-CID-REQUEST

[RFC 9147 §§5.8.4, 9](https://www.rfc-editor.org/rfc/rfc9147.html#section-9): request acknowledgment and fulfillment differ. An unfulfilled request prohibits another request; a response can contain fewer IDs, including none.

### DTLS-EXT-PATH

[RFC 9146 §6](https://www.rfc-editor.org/rfc/rfc9146.html#section-6): a changed address requires record validity, freshness and a return-reachability strategy. [RFC 9853](https://www.rfc-editor.org/rfc/rfc9853.html) recommends its negotiated RRC subprotocol; the selected profile instead uses bounded heartbeat validation and does not advertise RRC.

### DTLS-EXT-HEARTBEAT

[RFC 6520 §§2–4](https://www.rfc-editor.org/rfc/rfc6520.html#section-2): respect directional permissions and one outstanding request; validate body length, echo payload and include at least 16 padding bytes. [IANA](https://www.iana.org/assignments/tls-extensiontype-values/) places the 1.3 response extension in EncryptedExtensions.

The selected model retransmits requests with a one-second initial interval, doubling to 60 seconds, within a finite probe deadline. Expiry reports probe failure while preserving ordinary data. This is a deliberate profile choice relative to RFC 6520 §3's connection-termination recommendation: later candidate-path and size-discovery failure must preserve a usable confirmed path. The [heartbeat reviewer document](../../../../../tests/protocol/dtls/heartbeat-probes.md) records the implemented bounds and evidence; [PHA suspension](../../../../../tests/protocol/dtls/post-handshake-authentication.md) pauses and resumes the remaining probe deadline.

### DTLS-EXT-PHA

[RFC 9846 §§4.3.6, 4.4.2, 4.7.2](https://datatracker.ietf.org/doc/html/rfc9846#section-4.7.2): only clients advertise PHA. External-PSK authentication cannot request it without another specification. Certificate or resumption authentication can use unique request contexts. The client does not learn server acceptance from its own response.

### DTLS-EXT-PMTU

[RFC 8899 §§4–6](https://www.rfc-editor.org/rfc/rfc8899.html#section-4): confirm size with nonfragmenting probes, retain a confirmed bound separately from search candidates, recover from black holes and constrain PTB hints. Probe timer is at least one second.

### DTLS-EXT-VERSION

[RFC 9147 §§5.3–5.4](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.3): versions 1.3/1.2 encode as 0xfefc/0xfefd. Dual-version hellos preserve the legacy forms and downgrade representation; selection must be offered. A 1.3 ServerHello never echoes a 1.2 session ID.

### DTLS-EXT-12-RECORD

[RFC 6347 §4.1](https://www.rfc-editor.org/rfc/rfc6347.html#section-4.1), [RFC 5487 §2](https://www.rfc-editor.org/rfc/rfc5487.html#section-2), [RFC 5289 §3.2](https://www.rfc-editor.org/rfc/rfc5289.html#section-3.2): selected 1.2 AEAD records use explicit nonce, tag, full epoch and 48-bit sequence. CCS changes epochs; 1.3 ACKs/KeyUpdates are unavailable.

### DTLS-EXT-12-RECOVERY

[RFC 6347 §4.2.4](https://www.rfc-editor.org/rfc/rfc6347.html#section-4.2.4): recover whole flights, including CCS/Finished. Retain the final response for twice default TCP MSL, independently of establishment deadline.

### DTLS-EXT-12-SESSION

[RFC 5077 §3](https://www.rfc-editor.org/rfc/rfc5077.html#section-3), [RFC 5246 §7.3](https://www.rfc-editor.org/rfc/rfc5246.html#section-7.3): session-ID/ticket resumption follows abbreviated version-specific flights and may renew tickets; incompatible or unknown state falls back without early-data acceptance.

### DTLS-EXT-12-PATH

[RFC 9146 §§3–6](https://www.rfc-editor.org/rfc/rfc9146.html#section-3): static 1.2 CID uses content type 25 and its own inner plaintext/accounting. It does not acquire 1.3 CID-update messages.

The CID precedes the record length and has no length octet in that record. Each endpoint chooses its receive length; it must still send other peer-chosen lengths. Nonempty negotiated directions require the CID record form once protection starts. The complete inner plaintext, including real type and padding, is at most 2^14 bytes under §5. A resumed handshake negotiates fresh CIDs.

## Errata disposition

RFC Editor searches for RFCs 9147, 8446, 9846, 9146, 6347, 5246, 5077, 5487, 5289, 8422, 6520 and 8899 were retrieved on 2026-11-16. Raw pages and a machine-readable index are retained locally in `audit/dtls-extensions/standards/`. A reported erratum is not silently promoted to a normative requirement.

| Source | Selected disposition |
| --- | --- |
| RFC 9147 verified 8100 | Use the corrected record-demultiplexing diagram. |
| RFC 9147 reported 8047/8048/8050/8051/8141 | Keep local message-number exhaustion explicit; do not reset post-handshake numbering. Retain the published send-epoch limit and independent receiving reconstruction. AEAD-input changes are outside computed-cryptography scope. |
| RFC 9147 reported 8066/8067 | Dual-version tests cover empty 1.3 session-ID echo, 1.2 ticket/session-ID offers and version-specific CCS, resolving the compatibility ambiguity explicitly. |
| RFC 9147 reported 8107/8108 | Early traffic never carries ACKs in epoch 1. Reject an ACK listing an epoch newer than its carrier; retain the baseline's established epoch reconstruction limits. |
| RFC 8446 verified/held material | Use current RFC 9846 text for ticket lifetime/age, early parameters, key shares, PHA response context/order, supported groups and signature negotiation. DTLS-specific framing/recovery overrides the stream rules. X.509/crypto-computation changes remain excluded; rejected errata are not applied. |
| RFC 9846 verified 9040 | Security-analysis wording only; no new computed-cryptography claim. Reported 9042/9043 (hello text) and 9161 (change summary) do not override DTLS hello rules; 9157 changes secret naming only. |
| RFC 6347 reported 3917/4103/5186/8089/4104/4105/4642/5026/5903 | Preserve whole-flight recovery and 240-second final-response retention; fragment length and handshake-body length stay distinct; content length is capped at 2^14. Transcript/MAC computation remains excluded. |
| RFC 5246 verified material | Respect corrected handshake vectors, CertificateRequest lengths, compression/extension bounds and hello versions. CBC/MAC computation and SSL/TLS legacy negotiation are excluded. |
| RFC 5077 reported 4800 | Ticket key-distribution concerns do not apply to a local stateful opaque-ticket table. No stateless ticket encryption is claimed. |
| RFC 8422 verified 5703/6002; held 5466/5468; reported 8179 | Use corrected EC structure/registry references and the selected uncompressed secp256r1 share. No elliptic-curve computation is performed. |
| RFC 6520 verified 6825 | Editorial spelling correction; no protocol change. |
| RFC 9146/5487/5289/8899 | Search returned no errata. |

The selected obligations remain subject to their linked standards, rather than an assumption that the short matrix exhausts every statement. New ambiguities affecting behavior require an explicit model decision before their implementation.
