# RFC 9175 (CoAP Request-Tag) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC9175-*` · **Stands on:** [standards.md](../../protocol/coap/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for the `Request-Tag` option (`38`) of RFC 9175, CoAP: Echo, Request-Tag, and Token Processing, February 2022, scoped to blockwise request-body association alongside RFC 7959. The catalog comes from the specification text only and contains no simulation model names and no code references.

Source: `standards/RFC/rfc9175.txt` (<https://www.rfc-editor.org/rfc/rfc9175.txt>, February 2022).

The family of CoAP documents is in [`standards.md`](../../protocol/coap/standards.md), the feature map is in [`features.md`](../../protocol/coap/features.md), and the workflow state lives in [`coverage.md`](../../model/coap/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC9175-RTAG-1](#rfc9175-rtag-1) | The repeatable opaque `Request-Tag` option (`38`, `0..8` octets) keys concurrent `Block1` request-body assemblies per client endpoint and rotates when a new blockwise request operation starts. |

## Blockwise request-body association

### RFC9175-RTAG-1

**The repeatable opaque `Request-Tag` option (`38`, `0..8` octets) keys concurrent `Block1` request-body assemblies per client endpoint and rotates when a new blockwise request operation starts.**

> "The Request-Tag Option (number 38, elective, repeatable, 0-8 bytes opaque) allows a CoAP client to mark blocks of a block-wise request body as belonging to the same request operation, so a server can reassemble concurrent block-wise requests from the same client endpoint independently and reject mismatched blocks." — §3, `rfc9175.txt:410-520`

- Strength: must. Class: wire plus end-to-end.
- Check idea: interleave two multi-block `Block1` uploads from the same client endpoint with distinct `Request-Tag` values (`38`) and verify the server reassembles both bodies independently without mixing blocks; verify that an upload with a fresh `Request-Tag` starting at `NUM > 0` is rejected with `4.08 Request Entity Incomplete`, and verify option length `> 8` octets is rejected with `4.02 Bad Option`.
