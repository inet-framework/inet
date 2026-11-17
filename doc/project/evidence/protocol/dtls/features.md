# DTLS selected features

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `DTLS-F-*` · **Stands on:** [standards.md](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/doc/project/evidence/protocol/dtls/standards.md)

These features describe the selected protocol mechanisms, independent of any implementation or test verdict.

| Feature | Mechanism | Governing requirements |
| --- | --- | --- |
| DTLS-F-RECORD | Plaintext and unified record framing | RFC9147-RECORD, NUMBER, INVALID |
| DTLS-F-SETUP | Version/suite/profile negotiation and Finished | RFC8446-NEGOTIATION, MESSAGES |
| DTLS-F-COOKIE | Address validation and retry | RFC9147-COOKIE, HELLO; RFC8446-RETRY |
| DTLS-F-PEERS | Independent associations and replacement | RFC9147-RESTART |
| DTLS-F-RECOVERY | Handshake fragments, selective ACKs and backoff | RFC9147-FRAGMENT, RECOVERY, ACK |
| DTLS-F-DATA | Unreliable application datagrams | RFC9147-DATA; RFC8446-PROTECTION |
| DTLS-F-REPLAY | Per-epoch record identity filtering | RFC9147-REPLAY, NUMBER |
| DTLS-F-UPDATE | Reliable directional epoch update | RFC9147-UPDATE; RFC8446-UPDATE |
| DTLS-F-CLOSE | Alerts and closure | RFC9147-CLOSE; RFC8446-ALERT |

The per-document statements live in the [DTLS catalog](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/doc/project/evidence/standard/rfc9147/catalog.md) and [TLS content catalog](https://github.com/mgonzalezlopezudc/inet/blob/5284847df92eea8451bf056d8722fabb0ddf65bd/doc/project/evidence/standard/rfc8446/catalog.md). Coverage belongs to the separate model ledger.
