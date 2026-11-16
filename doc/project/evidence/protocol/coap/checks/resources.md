# CoAP — English check procedures: resource methods, options, and discovery

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc7252/catalog.md](../../../standard/rfc7252/catalog.md), [rfc6690/catalog.md](../../../standard/rfc6690/catalog.md)

The common mockups, the scenario constants, and the index are in [`checks.md`](../checks.md). The procedures come from the specifications only and name no simulation model and no code.

## Resource methods GET, POST, PUT, and DELETE

- **Checks** — [RFC7252-METH-1](../../../standard/rfc7252/catalog.md#rfc7252-meth-1), [RFC7252-URI-1](../../../standard/rfc7252/catalog.md#rfc7252-uri-1).
- **Requirement** — `GET` returns `2.05 Content` with the resource representation and `ETag`; `POST` creates a child resource (`2.01 Created` with `Location-Path`) or updates an existing resource (`2.04 Changed`); `PUT` creates (`2.01 Created`) or replaces (`2.04 Changed`) a resource; `DELETE` removes a resource (`2.02 Deleted` whether present or already absent); requests to missing resources for `GET`/`POST` return `4.04 Not Found`; and disallowed methods return `4.05 Method Not Allowed`.

### Procedure

1. Build the plain pair mockup with a server hosting `/sensors/temp` (read-only `GET`) and `/actuators/led` (`GET`, `POST`, `PUT`, `DELETE`).
2. Issue `GET /sensors/temp`, `PUT /sensors/temp` (disallowed), `POST /actuators/led` (creating a sub-resource and updating), `PUT /actuators/new` (creating) and `PUT /actuators/new` (replacing), `DELETE /actuators/new` twice, and `GET /missing`.
3. Observe all request and response codes, options, and payloads on the wire.

### Expected observations

1. `GET /sensors/temp` returns `2.05 Content` with the configured payload and an `ETag` (`4`) option; `PUT /sensors/temp` returns `4.05 Method Not Allowed`.
2. `POST` creating a child resource returns `2.01 Created` with `Location-Path` (`8`) segments naming the created path; `PUT` on a new path returns `2.01 Created`, and a second `PUT` on the same path returns `2.04 Changed` with a fresh `ETag`.
3. Both the first and second `DELETE /actuators/new` return `2.02 Deleted`, and a subsequent `GET /actuators/new` or `GET /missing` returns `4.04 Not Found`.

## Critical options, conditional requests, and content negotiation

- **Checks** — [RFC7252-URI-1](../../../standard/rfc7252/catalog.md#rfc7252-uri-1), [RFC7252-OPT-1](../../../standard/rfc7252/catalog.md#rfc7252-opt-1), [RFC7252-COND-1](../../../standard/rfc7252/catalog.md#rfc7252-cond-1), [RFC7252-PROXY-1](../../../standard/rfc7252/catalog.md#rfc7252-proxy-1).
- **Requirement** — `ETag` (`4`) in `GET` validates a cached representation (`2.03 Valid` without payload on match, `2.05 Content` on mismatch); `If-Match` (`1`) and `If-None-Match` (`5`) enforce conditional updates (`4.12 Precondition Failed` on failure); `Accept` (`17`) returns `4.06 Not Acceptable` when the resource format differs; unrecognized critical (odd) options cause `4.02 Bad Option` in `CON` requests and `RST` in `CON` responses, whereas unrecognized elective (even) options are ignored; `Proxy-Uri` (`35`) and `Proxy-Scheme` (`39`) return `5.05 Proxying Not Supported`; and percent-encoded `Uri-Path` segments are decoded after segment splitting.

### Procedure

1. Build the plain pair mockup.
2. Fetch a resource to learn its `ETag`, re-send `GET` with that `ETag` (`2.03 Valid`), test `PUT` with matching and non-matching `If-Match` (including empty `If-Match` for any-existing) and `If-None-Match`, test `GET` with matching and non-matching `Accept` (`17`), test percent-encoded `Uri-Path`, and inject messages with unknown critical (odd) options, unknown elective (even) options, and `Proxy-Uri` (`35`) / `Proxy-Scheme` (`39`).
3. Observe all responses on the wire.

### Expected observations

1. `GET` with a matching `ETag` returns `2.03 Valid` echoing the `ETag` and carrying zero payload bytes; after the resource changes, `GET` with the old `ETag` returns `2.05 Content` with the new `ETag` and payload.
2. `PUT` with a non-matching `If-Match` or with `If-None-Match` on an existing resource returns `4.12 Precondition Failed` without modifying the resource; `GET` with an incompatible `Accept` value returns `4.06 Not Acceptable`.
3. An unknown even (elective) option is ignored and the request succeeds (`2.05`), an unknown odd (critical) option in a `CON` request returns `4.02 Bad Option`, an unknown odd option in a `CON` response triggers `RST`, and `Proxy-Uri` / `Proxy-Scheme` return `5.05 Proxying Not Supported`.

## CoRE Link Format resource discovery

- **Checks** — [RFC6690-CORE-1](../../../standard/rfc6690/catalog.md#rfc6690-core-1), [RFC6690-CORE-2](../../../standard/rfc6690/catalog.md#rfc6690-core-2), [RFC6690-CORE-3](../../../standard/rfc6690/catalog.md#rfc6690-core-3).
- **Requirement** — `GET /.well-known/core` returns `2.05 Content` with `Content-Format` `40` (`application/link-format`), listing hosted resources with their `rt`, `if`, `ct`, `title`, and `obs` attributes, and filters entries when `Uri-Query` (`15`) specifies `href`, `rt`, or `if` (with exact or prefix `*` matching).

### Procedure

1. Build the plain pair mockup with server resources carrying `rt`, `if`, `ct`, and `observable` attributes.
2. Send `GET /.well-known/core` with no query, then with `Uri-Query` `rt=temperature-c`, `if=sensor*`, `href=/sensors/*`, and a non-matching query `rt=nonexistent`.
3. Inspect the `Content-Format` option and payload text of each `2.05 Content` response.

### Expected observations

1. Unfiltered `GET /.well-known/core` returns `2.05 Content` with `Content-Format == 40` and comma-separated `<path>` entries for all hosted resources, including `;obs` on observable resources.
2. Filtered queries return only the matching link entries (including prefix `*` matches), and the non-matching query returns `2.05 Content` with `Content-Format == 40` and an empty payload.
