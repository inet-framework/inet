# CoAP — standards family and in-scope set

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

Step 2 artifact of the standards test workflow. This document maps the family of standards around the Constrained Application Protocol (CoAP), records which specification governs each shared mechanism, and pins the in-scope UDP (`NoSec`) profile.

## Target level

**Level 3 — Edge** (see [the levels of the guide](../../../guide/derive-tests-from-a-standard.md#levels-of-depth)). The in-scope set covers the base CoAP over UDP protocol (RFC 7252), CoRE Link Format discovery (RFC 6690), resource observation (RFC 7641), block-wise transfers including combined Observe + `Block2` (RFC 7959), and `Request-Tag` blockwise request-body correlation (RFC 9175 §3).

| To reach | Add to the in-scope set | Why |
| --- | --- | --- |
| level 2, Core | RFC 7252, RFC 6690, RFC 7641, RFC 7959 | in scope now. Defines CoAP over UDP messaging, REST methods, options, link-format discovery, Observe, and `Block1`/`Block2` transfers |
| level 3, Edge | RFC 9175 §3 (`Request-Tag`) plus malformed/loss/reorder/conflict cases of RFC 7252, RFC 7641, RFC 7959 | in scope now. Reaches format errors, duplicate suppression, congestion admission, notification reordering and supersession, block negotiation, incomplete/oversized transfers, and concurrent `Block1` assembly correlation |
| level 4, Dynamics | multi-seed statistical distribution tests | level 3 verifies deterministic bounds across seeds; full distribution fitting is level 4 |
| level 5, Complete | RFC 8323 (TCP/TLS/WebSockets), RFC 8613 (OSCORE), RFC 7967 (`No-Response`), RFC 8132 (`FETCH`/`PATCH`/`iPATCH`) | separate transport bindings, object security, and additional methods outside the UDP `NoSec` profile |

What the pass actually reached is recorded in [`model/coap/coverage.md`](../../model/coap/coverage.md#achieved-level).

## Document list

| Document | Title | Date | Status | Relation | Source |
| --- | --- | --- | --- | --- | --- |
| RFC 7252 | The Constrained Application Protocol (CoAP) | June 2014 | Proposed Standard | `base` | `standards/RFC/rfc7252.txt` (<https://www.rfc-editor.org/rfc/rfc7252.txt>) |
| RFC 6690 | Constrained RESTful Environments (CoRE) Link Format | August 2012 | Proposed Standard | `companion` (`/.well-known/core` discovery) | `standards/RFC/rfc6690.txt` (<https://www.rfc-editor.org/rfc/rfc6690.txt>) |
| RFC 7641 | Observing Resources in the Constrained Application Protocol (CoAP) | September 2015 | Proposed Standard | `extension` (updates RFC 7252 with option `6` `Observe`) | `standards/RFC/rfc7641.txt` (<https://www.rfc-editor.org/rfc/rfc7641.txt>) |
| RFC 7959 | Block-Wise Transfers in the Constrained Application Protocol (CoAP) | August 2016 | Proposed Standard | `extension` (updates RFC 7252 with `Block1`, `Block2`, `Size1`, `Size2`) | `standards/RFC/rfc7959.txt` (<https://www.rfc-editor.org/rfc/rfc7959.txt>) |
| RFC 9175 | CoAP: Echo, Request-Tag, and Token Processing | February 2022 | Proposed Standard | `updates` RFC 7252 and RFC 7959 (§3 `Request-Tag` option `38`) | `standards/RFC/rfc9175.txt` (<https://www.rfc-editor.org/rfc/rfc9175.txt>) |
| RFC 8323 | CoAP (Constrained Application Protocol) over TCP, TLS, and WebSockets | February 2018 | Proposed Standard | `companion` (alternative transports) | out of scope |
| RFC 8613 | Object Security for Constrained RESTful Environments (OSCORE) | July 2019 | Proposed Standard | `extension` (application-layer security) | out of scope |

## Override table

| Area | Base clause | Later clause | Governs | In scope now |
| --- | --- | --- | --- | --- |
| Resource observation over GET | RFC 7252 §5.8.1 defines one-shot `GET` | RFC 7641 extends `GET` with option `6` (`Observe`) so a single request yields an initial response plus asynchronous notifications | RFC 7641 | yes |
| Large representation transfer | RFC 7252 §4.6 recommends fitting messages into a single IP datagram | RFC 7959 defines `Block1` (`27`), `Block2` (`23`), `Size1` (`60`), and `Size2` (`28`) for multi-block request and response transfers, including interaction with RFC 7641 Observe (§2.6, §3.4) | RFC 7959 | yes |
| Concurrent `Block1` request-body correlation | RFC 7959 §2.5 associates `Block1` blocks by client endpoint and target resource | RFC 9175 §3 defines the elective repeatable `Request-Tag` option (`38`) so a server keys `Block1` state by `(client endpoint, target URI, Request-Tag vector)` | RFC 9175 §3 | yes |

## In-scope set

| Document | Scope | Catalog file |
| --- | --- | --- |
| RFC 7252 | CoAP over UDP (`NoSec` mode, default port `5683`, unicast client/server endpoints, no forward/reverse/cross-protocol proxying) | [`standard/rfc7252/catalog.md`](../../standard/rfc7252/catalog.md) |
| RFC 6690 | CoRE Link Format (`/.well-known/core`, `Content-Format` `40`, `href`/`rt`/`if` query filtering, `obs` attribute) | [`standard/rfc6690/catalog.md`](../../standard/rfc6690/catalog.md) |
| RFC 7641 | CoAP Observe (`Observe` option `6`, registration, 24-bit serial ordering, freshness re-registration, supersession, cancellation) | [`standard/rfc7641/catalog.md`](../../standard/rfc7641/catalog.md) |
| RFC 7959 | CoAP Block-Wise Transfers (`Block1` `27`, `Block2` `23`, `Size1` `60`, `Size2` `28`, `2.31 Continue`, `SZX` negotiation, `4.08`/`4.13`, Observe + `Block2`) | [`standard/rfc7959/catalog.md`](../../standard/rfc7959/catalog.md) |
| RFC 9175 | `Request-Tag` option (`38`, `0..8` octets) for concurrent `Block1` request-body correlation (§3 only) | [`standard/rfc9175/catalog.md`](../../standard/rfc9175/catalog.md) |

Out of scope, with the reason:

| Document or clause | Reason |
| --- | --- |
| RFC 7252 §9 (`coaps://` over DTLS), RFC 8613 (OSCORE) | Cryptographic security bindings; this profile targets `NoSec` UDP (`coap://`). |
| RFC 7252 §5.7 / §10 (forward/reverse/HTTP proxying) | Endpoint profile only; proxy options (`Proxy-Uri`, `Proxy-Scheme`) are parsed and rejected with `5.05 Proxying Not Supported`. |
| RFC 7252 §8 / RFC 7390 (multicast CoAP group communication) | Unicast client/server endpoint profile only. |
| RFC 8323 (CoAP over TCP, TLS, and WebSockets) | Separate transport framing (`Len`/`TKL` without Message ID or `CON`/`NON`). |
| RFC 9175 §2 (`Echo` option `252`) and §4 (extended token lifetimes) | Outside the `Block1` `Request-Tag` correlation scope. |
