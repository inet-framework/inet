# CoAP — feature map

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `COAP-F-*` · **Stands on:** [standards.md](standards.md), [rfc7252/catalog.md](../../standard/rfc7252/catalog.md), [rfc6690/catalog.md](../../standard/rfc6690/catalog.md), [rfc7641/catalog.md](../../standard/rfc7641/catalog.md), [rfc7959/catalog.md](../../standard/rfc7959/catalog.md), [rfc9175/catalog.md](../../standard/rfc9175/catalog.md)

Step 4 artifact of the standards test workflow. This document maps the protocol capabilities defined by the in-scope CoAP specifications to their catalog statements. The support state of each feature in the simulation model is recorded in [`coverage.md`](../../model/coap/coverage.md).

## Index

| ID | Feature |
| --- | --- |
| [COAP-F-ENCODING](#coap-f-encoding) | Binary header, variable-length Token (`0..8` octets), delta-encoded options (`13`/`14` extensions), and `0xFF` payload marker framing with format-error detection. |
| [COAP-F-NON](#coap-f-non) | Non-confirmable (`NON`) request/response exchange and Token-based response correlation. |
| [COAP-F-CON](#coap-f-con) | Confirmable (`CON`) reliability with exponential backoff retries, piggybacked and separate responses, duplicate suppression, Empty `CON` ping (`RST`), and `NSTART`/`PROBING_RATE` congestion control. |
| [COAP-F-RESOURCES](#coap-f-resources) | RESTful methods (`GET`, `POST`, `PUT`, `DELETE`), URI/Location options, critical/elective option classification, conditional requests (`ETag`, `If-Match`, `If-None-Match`), format negotiation (`Content-Format`, `Accept`), and proxy rejection (`5.05`). |
| [COAP-F-DISCOVERY](#coap-f-discovery) | `/.well-known/core` link-format resource discovery (`Content-Format` `40`), attribute query filtering (`href`, `rt`, `if`), and `obs` advertisement. |
| [COAP-F-OBSERVE](#coap-f-observe) | Resource observation (`Observe` option `6`): registration, 24-bit serial ordering with `128s` rule, notification supersession and retry removal, freshness re-registration, and explicit/RST/error cancellation. |
| [COAP-F-BLOCKWISE](#coap-f-blockwise) | Block-wise transfers (`Block1` `27`, `Block2` `23`, `Size1` `60`, `Size2` `28`) with atomic `Block1` upload, `Block2` download, `SZX` negotiation, `4.08`/`4.13` error handling, combined `Block1`+`Block2`, and `Request-Tag` (`38`) concurrency isolation. |
| [COAP-F-OBSERVE-BLOCK2](#coap-f-observe-block2) | Combined `Observe` notifications and `Block2` follow-up retrieval with distinct exchange tokens, notification supersession, and `ETag` version-change recovery. |
| [COAP-F-DTLS](#coap-f-dtls) | CoAP over DTLS 1.3 (`coaps://`, default UDP port `5684`): unicast session-bound messaging, duplicate suppression, non-reused client Tokens across `KeyUpdate`, and session-close Observe/Blockwise cleanup. |

## Summary table

| Feature | Level | Sources | Core checks |
| --- | --- | --- | --- |
| [COAP-F-ENCODING](#coap-f-encoding) | mandatory | RFC 7252 §3, §4.1–§4.3 | RFC7252-FMT-1, FMT-2, FMT-3, FMT-4, FMT-5, FMT-6, FMT-7, FMT-8, ERR-1 |
| [COAP-F-NON](#coap-f-non) | mandatory | RFC 7252 §4.3, §5.2.3, §5.3 | RFC7252-NON-1, RFC7252-MATCH-1 |
| [COAP-F-CON](#coap-f-con) | mandatory | RFC 7252 §4.2, §4.5, §4.7, §5.2.1, §5.2.2 | RFC7252-CON-1, CON-2, PIGGY-1, SEP-1, DUP-1, PING-1, CC-1 |
| [COAP-F-RESOURCES](#coap-f-resources) | mandatory | RFC 7252 §5.4, §5.8, §5.10 | RFC7252-METH-1, URI-1, OPT-1, COND-1, PROXY-1 |
| [COAP-F-DISCOVERY](#coap-f-discovery) | mandatory | RFC 6690 §1.2, §4, RFC 7252 §7 | RFC6690-CORE-1, CORE-2, CORE-3 |
| [COAP-F-OBSERVE](#coap-f-observe) | optional | RFC 7641 §3, §4 | RFC7641-REG-1, REG-2, NOTIFY-1, ORDER-1, SUPER-1, FRESH-1, CANCEL-1 |
| [COAP-F-BLOCKWISE](#coap-f-blockwise) | optional | RFC 7959 §2, §3, RFC 9175 §3 | RFC7959-OPT-1, B1-1, B2-1, NEG-1, ERR-1, COMB-1, RFC9175-RTAG-1 |
| [COAP-F-OBSERVE-BLOCK2](#coap-f-observe-block2) | optional | RFC 7959 §2.6, §3.4, RFC 7641 | RFC7959-OBS-1 |
| [COAP-F-DTLS](#coap-f-dtls) | optional | RFC 7252 §6.2, §9.1, §12.6, RFC 9147, RFC 9175 §4 | RFC7252-DTLS-1, DTLS-2, DTLS-3, RFC9175-TOKEN-1 |

## COAP-F-ENCODING

**Binary header, variable-length Token (`0..8` octets), delta-encoded options (`13`/`14` extensions), and `0xFF` payload marker framing with format-error detection.**

- **Sources** — RFC 7252 §3, §3.1, §4.1, §4.2, §4.3; RFC 7641 §2; RFC 7959 §2.1–§2.2; RFC 9175 §3.
- **Level** — mandatory (reason: keyword, `must`).
- **Description** — every CoAP datagram starts with the 4-octet header (`Ver`, `T`, `TKL`, `Code`, `Message ID`), followed by `TKL` token octets, ascending delta-encoded options, and an optional `0xFF` marker plus payload. Truncated datagrams and unknown versions are silently ignored; malformed `CON` headers with at least 4 octets draw `RST`; malformed `NON`, `ACK`, and `RST` datagrams are silently ignored.
- **Checks** — core: RFC7252-FMT-1 through FMT-8, RFC7252-ERR-1, RFC7959-OPT-1.

## COAP-F-NON

**Non-confirmable (`NON`) request/response exchange and Token-based response correlation.**

- **Sources** — RFC 7252 §4.3, §5.2.3, §5.3.1, §5.3.2.
- **Level** — mandatory (reason: keyword, `must`).
- **Description** — a `NON` request carries a fresh Message ID and `0..8`-byte Token, elicits a `NON` response with the same Token and no `ACK`, and matches responses strictly by source endpoint and Token.
- **Checks** — core: RFC7252-NON-1, RFC7252-MATCH-1.

## COAP-F-CON

**Confirmable (`CON`) reliability with exponential backoff retries, piggybacked and separate responses, duplicate suppression, Empty `CON` ping (`RST`), and `NSTART`/`PROBING_RATE` congestion control.**

- **Sources** — RFC 7252 §4.2, §4.5, §4.7, §4.8, §5.2.1, §5.2.2.
- **Level** — mandatory (reason: keyword, `must`).
- **Description** — `CON` messages retransmit with exponential backoff (`ACK_TIMEOUT=2s`, `ACK_RANDOM_FACTOR=1.5`, `MAX_RETRANSMIT=4`), support both piggybacked `ACK` responses and separate `ACK` + `CON` responses, suppress duplicate executions within `EXCHANGE_LIFETIME`/`NON_LIFETIME`, answer Empty `CON` with `RST`, and enforce `NSTART=1` and `PROBING_RATE=1 byte/s`.
- **Checks** — core: RFC7252-CON-1, CON-2, PIGGY-1, SEP-1, DUP-1, PING-1, CC-1.

## COAP-F-RESOURCES

**RESTful methods (`GET`, `POST`, `PUT`, `DELETE`), URI/Location options, critical/elective option classification, conditional requests (`ETag`, `If-Match`, `If-None-Match`), format negotiation (`Content-Format`, `Accept`), and proxy rejection (`5.05`).**

- **Sources** — RFC 7252 §5.4, §5.8, §5.10.
- **Level** — mandatory (reason: keyword, `must`).
- **Description** — servers expose resources addressed by `Uri-*` options, support `GET`/`POST`/`PUT`/`DELETE`, enforce critical vs elective option rules (`4.02`/`RST`/ignore), evaluate `ETag` validation (`2.03 Valid`) and `If-Match`/`If-None-Match` preconditions (`4.12`), negotiate `Content-Format` and `Accept` (`4.06`), and reject proxy options with `5.05`.
- **Checks** — core: RFC7252-METH-1, URI-1, OPT-1, COND-1, PROXY-1.

## COAP-F-DISCOVERY

**`/.well-known/core` link-format resource discovery (`Content-Format` `40`), attribute query filtering (`href`, `rt`, `if`), and `obs` advertisement.**

- **Sources** — RFC 6690 §1.2, §4; RFC 7252 §7; RFC 7641 §6.
- **Level** — mandatory (reason: RFC 7252 §7 discovery profile).
- **Description** — `GET /.well-known/core` returns link-format descriptions (`Content-Format` `40`) for hosted resources, supports query filtering on `href`, `rt`, and `if` (including prefix `*`), and includes `;obs` on observable resources.
- **Checks** — core: RFC6690-CORE-1, CORE-2, CORE-3.

## COAP-F-OBSERVE

**Resource observation (`Observe` option `6`): registration, 24-bit serial ordering with `128s` rule, notification supersession and retry removal, freshness re-registration, and explicit/RST/error cancellation.**

- **Sources** — RFC 7641 §3, §4, §6.
- **Level** — optional (RFC 7641 extension; mandatory within the in-scope Observe profile).
- **Description** — clients register with `GET` + `Observe=0`, receive `2.05`/`2.03` notifications ordered by 24-bit serial arithmetic and the strict `128s` rule, re-register after `Max-Age + [5s, 15s]` silence, supersede in-flight `CON` notifications on rapid updates, and cancel via `Observe=1`, `RST`, or non-`2.xx` responses.
- **Checks** — core: RFC7641-REG-1, REG-2, NOTIFY-1, ORDER-1, SUPER-1, FRESH-1, CANCEL-1.

## COAP-F-BLOCKWISE

**Block-wise transfers (`Block1` `27`, `Block2` `23`, `Size1` `60`, `Size2` `28`) with atomic `Block1` upload, `Block2` download, `SZX` negotiation, `4.08`/`4.13` error handling, combined `Block1`+`Block2`, and `Request-Tag` (`38`) concurrency isolation.**

- **Sources** — RFC 7959 §2, §3, §4; RFC 9175 §3.
- **Level** — optional (RFC 7959 / RFC 9175 extension; mandatory within the in-scope blockwise profile).
- **Description** — endpoints fragment large request bodies with `Block1` (`2.31 Continue` until `M=0`) and large response bodies with `Block2`, negotiate smaller `SZX` values, reject incomplete/mismatched uploads with `4.08` and oversized uploads with `4.13`, restart `Block2` downloads once on `ETag` change, chain `Block1` upload into `Block2` response retrieval, and isolate concurrent `Block1` uploads using `Request-Tag` (`38`).
- **Checks** — core: RFC7959-OPT-1, B1-1, B2-1, NEG-1, ERR-1, COMB-1, RFC9175-RTAG-1.

## COAP-F-OBSERVE-BLOCK2

**Combined `Observe` notifications and `Block2` follow-up retrieval with distinct exchange tokens, notification supersession, and `ETag` version-change recovery.**

- **Sources** — RFC 7959 §2.6, §3.4; RFC 7641 §3, §4.
- **Level** — optional (RFC 7959 / RFC 7641 combined profile).
- **Description** — when an observed resource exceeds the block size, notifications carry the first `Block2` block (`NUM=0, M=1`) with the registration Token, follow-up `GET` requests retrieve `NUM >= 1` using a distinct exchange Token without `Observe`, newer notifications supersede in-progress block assembly, and mid-transfer `ETag` changes trigger jittered re-registration.
- **Checks** — core: RFC7959-OBS-1.

## COAP-F-DTLS

**CoAP over DTLS 1.3 (`coaps://`, default UDP port `5684`): unicast session-bound messaging, duplicate suppression, non-reused client Tokens across `KeyUpdate`, and session-close Observe/Blockwise cleanup.**

- **Sources** — RFC 7252 §6.2, §9.1, §9.1.1, §9.1.2, §12.6; RFC 9147; RFC 9175 §4.
- **Level** — optional (DTLS security binding; mandatory within the `coaps://` profile).
- **Description** — endpoints resolve `coaps://` URIs to default UDP port `5684`, forbid multicast, bind CoAP `CON`/`NON` exchanges, duplicate suppression, Observe relations, and Block1/Block2 assemblies to the active DTLS session (`AssociationId`), wait for DTLS establishment before starting CoAP retransmission timers, issue distinct 8-byte Tokens across `KeyUpdate` rotations, and tear down session-bound state when the DTLS session closes or is replaced.
- **Checks** — core: RFC7252-DTLS-1, DTLS-2, DTLS-3, RFC9175-TOKEN-1.
