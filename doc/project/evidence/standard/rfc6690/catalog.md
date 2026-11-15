# RFC 6690 (CoRE Link Format) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC6690-*` · **Stands on:** [standards.md](../../protocol/coap/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 6690, Constrained RESTful Environments (CoRE) Link Format, August 2012. The catalog comes from the specification text only and contains no simulation model names and no code references.

Source: `standards/RFC/rfc6690.txt` (<https://www.rfc-editor.org/rfc/rfc6690.txt>, August 2012).

The family of CoAP documents is in [`standards.md`](../../protocol/coap/standards.md), the feature map is in [`features.md`](../../protocol/coap/features.md), and the workflow state lives in [`coverage.md`](../../model/coap/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC6690-CORE-1](#rfc6690-core-1) | `GET /.well-known/core` returns `2.05 Content` with `Content-Format` `40` (`application/link-format`) listing hosted resources. |
| [RFC6690-CORE-2](#rfc6690-core-2) | Query parameters (`href`, `rt`, `if`, including prefix `*`) on `/.well-known/core` filter the returned link-format entries. |
| [RFC6690-CORE-3](#rfc6690-core-3) | Observable resources advertise the `obs` target attribute in `/.well-known/core` link-format descriptions. |

## Resource discovery

### RFC6690-CORE-1

**`GET /.well-known/core` returns `2.05 Content` with `Content-Format` `40` (`application/link-format`) listing hosted resources.**

> "CoRE Resource Discovery is performed by sending a GET request to '/.well-known/core' and returns a payload in the CoRE Link Format (application/link-format, media type 40) containing links to resources hosted by the server." — §1.2 and §4, `rfc6690.txt:152-168`

- Strength: must. Class: wire.
- Check idea: send `GET /.well-known/core` and verify a `2.05 Content` response with `Content-Format == 40` and comma-separated `<path>;rt="...";if="...";ct=...` entries.

### RFC6690-CORE-2

**Query parameters (`href`, `rt`, `if`, including prefix `*`) on `/.well-known/core` filter the returned link-format entries.**

> "A server implementing this specification SHOULD support query filtering on '/.well-known/core'. If a query string is provided with 'href' or a link-extension attribute name, only links matching the query parameter value (or prefix when ending with '*') are returned." — §4.1, `rfc6690.txt:670-715`

- Strength: should. Class: wire.
- Check idea: query `/.well-known/core` with `rt=...`, `if=...`, and `href=/prefix*`, and verify that only matching links appear in the response payload (or empty payload when none match).

### RFC6690-CORE-3

**Observable resources advertise the `obs` target attribute in `/.well-known/core` link-format descriptions.**

> "A resource that is a suitable target for an observation relationship advertises this in discovery by including the 'obs' attribute in its link-format description." — RFC 6690 §2 and RFC 7641 §6, `rfc7641.txt:1265-1275`

- Strength: should. Class: wire.
- Check idea: query `/.well-known/core` on a server hosting both observable and non-observable resources and verify `;obs` appears only on the observable resource entries.
