# CoAP — model claims and conformance matrix

> **Kind:** report · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [features.md](../../protocol/coap/features.md), [coverage.md](coverage.md), [standards.md](../../protocol/coap/standards.md)

Step 8 artifact of the standards test workflow. This skeleton defines the target conformance claims for the CoAP over UDP (`NoSec`) model and is updated with final run verdicts in `C9`.

## Part 1 — what the model claims

The `inet.applications.coap` package (`CoapEndpointBase`, `CoapClient`, `CoapServer`, `CoapHeader`) claims the UDP `NoSec` endpoint profile of RFC 7252, CoRE Link Format discovery (`/.well-known/core`) of RFC 6690, resource observation (`Observe` option `6`) of RFC 7641, block-wise transfers (`Block1`, `Block2`, `Size1`, `Size2`, and Observe + `Block2`) of RFC 7959, and `Request-Tag` (`38`) blockwise request-body association of RFC 9175 §3. Forward/reverse proxying (`5.05`), DTLS (`coaps://`), OSCORE, multicast CoAP, and TCP/TLS/WebSockets bindings are explicitly out of scope.

## Part 2 — conformance matrix

| Feature | Level | Claimed | Support | Verdict |
| --- | --- | --- | --- | --- |
| [COAP-F-ENCODING](../../protocol/coap/features.md#coap-f-encoding) | mandatory | yes (RFC 7252, RFC 7959) | pending | pending (C2/C6) |
| [COAP-F-NON](../../protocol/coap/features.md#coap-f-non) | mandatory | yes (RFC 7252) | pending | pending (C2) |
| [COAP-F-CON](../../protocol/coap/features.md#coap-f-con) | mandatory | yes (RFC 7252) | pending | pending (C3) |
| [COAP-F-RESOURCES](../../protocol/coap/features.md#coap-f-resources) | mandatory | yes (RFC 7252) | pending | pending (C4) |
| [COAP-F-DISCOVERY](../../protocol/coap/features.md#coap-f-discovery) | mandatory | yes (RFC 6690, RFC 7252, RFC 7641) | pending | pending (C4/C5) |
| [COAP-F-OBSERVE](../../protocol/coap/features.md#coap-f-observe) | optional | yes (RFC 7641) | pending | pending (C5) |
| [COAP-F-BLOCKWISE](../../protocol/coap/features.md#coap-f-blockwise) | optional | yes (RFC 7959, RFC 9175 §3) | pending | pending (C6) |
| [COAP-F-OBSERVE-BLOCK2](../../protocol/coap/features.md#coap-f-observe-block2) | optional | yes (RFC 7959 §2.6, §3.4) | pending | pending (C7) |
