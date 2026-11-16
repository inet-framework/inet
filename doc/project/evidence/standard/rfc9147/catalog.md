# RFC 9147 selected checkable requirements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC9147-*` · **Stands on:** [standards.md](../../protocol/dtls/standards.md)

These selected statements are paraphrased from RFC 9147, April 2022. The section links identify their normative context. This catalog is not exhaustive and contains no implementation verdicts.

## Index

| Identifier | Requirement |
| --- | --- |
| [RFC9147-RECORD](#rfc9147-record) | Record framing and datagram boundaries |
| [RFC9147-NUMBER](#rfc9147-number) | Sequence and epoch identity |
| [RFC9147-REPLAY](#rfc9147-replay) | Per-epoch duplicate filtering |
| [RFC9147-INVALID](#rfc9147-invalid) | Invalid records do not perturb handshake recovery |
| [RFC9147-COOKIE](#rfc9147-cookie) | Cookie validation and amplification control |
| [RFC9147-HELLO](#rfc9147-hello) | DTLS-specific hello fields and retry constraints |
| [RFC9147-FRAGMENT](#rfc9147-fragment) | Fragment ranges and immutable retransmitted bytes |
| [RFC9147-RECOVERY](#rfc9147-recovery) | Flight retention and timeout recovery |
| [RFC9147-ACK](#rfc9147-ack) | ACK eligibility and flight completion |
| [RFC9147-RESTART](#rfc9147-restart) | Same-quartet replacement preserves the old association until validation |
| [RFC9147-UPDATE](#rfc9147-update) | Reliable directional key updates |
| [RFC9147-CLOSE](#rfc9147-close) | Alerts and closure ordering |
| [RFC9147-DATA](#rfc9147-data) | Unreliable datagram application semantics |

### RFC9147-RECORD

**Records fit completely in one datagram.** Sections [4](https://www.rfc-editor.org/rfc/rfc9147.html#section-4) and [4.3](https://www.rfc-editor.org/rfc/rfc9147.html#section-4.3) define plaintext/unified headers, concatenated records and optional length on the final record. Reject unparseable boundaries without borrowing bytes from another datagram.

### RFC9147-NUMBER

**Record numbers are independent per epoch and retransmissions use fresh record numbers in their original epoch.** Sections [4.2](https://www.rfc-editor.org/rfc/rfc9147.html#section-4.2) and 6.1 define initial counters, truncated-number reconstruction and epoch roles. Plaintext sequence values cannot exceed the transmitted 48-bit space; counters and epochs must not wrap.

### RFC9147-REPLAY

**Duplicate detection has separate state for each epoch.** [Section 4.5.1](https://www.rfc-editor.org/rfc/rfc9147.html#section-4.5.1) describes the sliding window, older-record rejection and updating the window only after successful deprotection.

### RFC9147-INVALID

**An invalid record does not reset pending handshake retransmission timers.** [Section 4.5.2](https://www.rfc-editor.org/rfc/rfc9147.html#section-4.5.2) recommends silent discard and distinguishes record failure from a validly carried handshake error.

### RFC9147-COOKIE

**A retry carries the cookie extension and proceeds only after its validation.** [Section 5.1](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.1) defines HelloRetryRequest, invalid-cookie illegal_parameter, default cookie use and the recommended three-times pre-validation sending bound.

### RFC9147-HELLO

**DTLS 1.3-only ClientHello has empty legacy_cookie and advertises the selected version in supported_versions.** [Sections 5.3–5.4](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.3) define legacy version 0xfefd and supported DTLS 1.3 version 0xfefc. TLS compatibility ChangeCipherSpec is excluded by section 5.

### RFC9147-FRAGMENT

**Fragments of one message preserve its sequence, total length and body bytes.** [Section 5.5](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.5) requires handling overlapping retransmitted ranges; changed overlapping bytes should abort with illegal_parameter. A fragment belongs to one record/datagram.

### RFC9147-RECOVERY

**Outstanding flights are retained for retransmission with backoff.** [Section 5.8](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.8) defines the flight machine, recommended 1 s initial timeout, doubling to at least 60 s maximum, eligible RTT adjustment, and server responses to repeated final client flights for twice TCP MSL.

### RFC9147-ACK

**ACKs cover processed or buffered handshake records, never unretained content or another content type.** [Section 7](https://www.rfc-editor.org/rfc/rfc9147.html#section-7) requires the final-flight ACK, appropriate sending epoch and canceling retransmission of acknowledged content. Responding flights implicitly acknowledge preceding flights.

### RFC9147-RESTART

**A new epoch-0 ClientHello cannot immediately destroy an existing same-quartet association.** [Section 5.11](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.11) requires reachability validation and replacing old state after a correct new Finished. Protected records of overlapping associations can need trial decryption.

### RFC9147-UPDATE

**A sender advances to the next epoch only after its KeyUpdate is acknowledged.** [Section 8](https://www.rfc-editor.org/rfc/rfc9147.html#section-8) requires retaining pre-update receiving state until a new-epoch record is accepted and disallows simultaneous outstanding updates of the same type. The 128-bit suite's sending epoch cannot exceed 2^48-1; receivers do not impose that sender ceiling.

### RFC9147-CLOSE

**Alerts are not retransmitted and closure excludes later record identities.** [Section 5.10](https://www.rfc-editor.org/rfc/rfc9147.html#section-5.10) defines the closure alert's epoch/sequence ordering and warns against relying on reliable alert delivery.

### RFC9147-DATA

**Application data preserves unreliable datagram delivery.** [Section 10](https://www.rfc-editor.org/rfc/rfc9147.html#section-10) and section 5.8.1 prohibit application delivery before peer Finished; datagram loss/reordering does not create application retransmission or a reliable ordered stream.
