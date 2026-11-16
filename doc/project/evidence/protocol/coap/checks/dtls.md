# CoAP — English check procedures: DTLS-secured CoAP

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc7252/catalog.md](../../../standard/rfc7252/catalog.md), [rfc9175/catalog.md](../../../standard/rfc9175/catalog.md)

The common mockups, the scenario constants, and the index are in [`checks.md`](../checks.md). The procedures come from the specifications only and name no simulation model and no code.

## DTLS handshake, coaps:// addressing, and multicast rejection

- **Checks** — [RFC7252-DTLS-1](../../../standard/rfc7252/catalog.md#rfc7252-dtls-1), [RFC7252-DTLS-2](../../../standard/rfc7252/catalog.md#rfc7252-dtls-2).
- **Requirement** — `coaps://` selects DTLS-secured CoAP on default UDP port `5684`, protects CoAP messages in DTLS application records without cleartext CoAP on the wire, and forbids multicast transmissions.

### Procedure

1. Build the plain pair mockup with DTLS 1.3 endpoints on port `5684` (IPv4 PSK and IPv6 certificate with mutual client authentication).
2. Have client `C` issue `CON` and `NON` `coaps://` requests to server `S`, including a delayed handshake exceeding `ACK_TIMEOUT`.
3. Separately reject scheme mismatches (`coap://` in DTLS mode, `coaps://` in `NoSec` mode) and a multicast target in DTLS mode.

### Expected observations

1. Wire packets on port `5684` carry DTLS 1.3 records rather than cleartext CoAP headers.
2. Client `C` waits for DTLS establishment before emitting `CON` application records and does not trigger premature CoAP retransmissions during the handshake.
3. Scheme mismatches and multicast targets fail locally without transmitting multicast DTLS packets.

## DTLS session isolation and replacement cleanup

- **Checks** — [RFC7252-DTLS-3](../../../standard/rfc7252/catalog.md#rfc7252-dtls-3).
- **Requirement** — CoAP exchanges, duplicate suppression, and deferred responses are scoped to the active DTLS session and do not cross into a replacement session on the same address/port tuple.

### Procedure

1. Establish DTLS session `S1` from `C` to `S` (`responseDelay > 0`), send a `CON` request with `MID = 42`, and before the separate response is sent, replace `S1` with a new DTLS session `S2` on the same `(IP, port)` tuple.
2. Send a new `CON` request with the same `MID = 42` over `S2`.

### Expected observations

1. Server `S` cancels `S1`'s pending separate response when `S1` closes and never transmits it over `S2`.
2. Server `S` processes the `MID = 42` request on `S2` as fresh rather than suppressing it as a duplicate of `S1`.

## Confirmable recovery, duplicate suppression, and KeyUpdate over DTLS

- **Checks** — [RFC7252-DTLS-3](../../../standard/rfc7252/catalog.md#rfc7252-dtls-3), [RFC9175-TOKEN-1](../../../standard/rfc9175/catalog.md#rfc9175-token-1).
- **Requirement** — lost DTLS application records are recovered by CoAP `CON` retransmissions and deduplicated within the session, malformed inner CoAP `CON` draws `RST` inside DTLS, and client Tokens are never reused across `KeyUpdate` rotations.

### Procedure

1. Drop the first DTLS application request and later the first DTLS application `ACK` on relay `T`.
2. Inject malformed inner CoAP `CON` and `NON` datagrams and an Empty `CON` ping over an established DTLS session.
3. Configure frequent `KeyUpdate` key rotation and issue sequential `CON` requests on one DTLS association.

### Expected observations

1. Client `C` retransmits `CON` with exponential backoff inside fresh DTLS record numbers; server `S` executes the update once and replays the cached `ACK`.
2. Malformed inner `CON` and Empty `CON` draw `RST` inside DTLS; malformed `NON` is silently ignored.
3. All requests across `KeyUpdate` epochs carry distinct 8-byte Tokens and complete without error.

## Observe and Blockwise lifecycle over DTLS

- **Checks** — [RFC7252-DTLS-3](../../../standard/rfc7252/catalog.md#rfc7252-dtls-3), [RFC7641-CANCEL-1](../../../standard/rfc7641/catalog.md#rfc7641-cancel-1), [RFC7959-B1-1](../../../standard/rfc7959/catalog.md#rfc7959-b1-1).
- **Requirement** — Observe notifications (`RFC 7641`), `Block1`/`Block2` transfers (`RFC 7959`), and `Request-Tag` concurrent uploads (`RFC 9175 §3`) operate over DTLS, survive in-session `KeyUpdate`, and tear down cleanly on DTLS session closure so new sessions cannot inherit observations or partial blocks.

### Procedure

1. Run Observe (`CON` notifications, supersession, `KeyUpdate`, and `Observe + Block2`) and `Block1`/`Block2` (`Request-Tag`) transfers over DTLS.
2. Close or replace DTLS session `S1` while an Observe relation and an incomplete `Block1` (`M=1`) upload exist, then send `NUM=1` on replacement session `S2`.

### Expected observations

1. Both client and server remove `S1`'s Observe relation upon `S1` closure and emit zero notifications onto `S2`.
2. Server `S` rejects `S2`'s orphaned `NUM=1` `Block1` fragment with `4.08 Request Entity Incomplete`.
