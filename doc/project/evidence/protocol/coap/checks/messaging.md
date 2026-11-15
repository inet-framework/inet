# CoAP — English check procedures: messaging and congestion control

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc7252/catalog.md](../../../standard/rfc7252/catalog.md)

The common mockups, the scenario constants, and the index are in [`checks.md`](../checks.md). The procedures come from the specifications only and name no simulation model and no code.

## Non-confirmable request and response

- **Checks** — [RFC7252-NON-1](../../../standard/rfc7252/catalog.md#rfc7252-non-1).
- **Requirement** — a Non-confirmable (`NON`) request carries a Message ID and Token, is answered with a `NON` response echoing the request Token, and neither endpoint sends an `ACK`.

### Procedure

1. Build the plain pair mockup.
2. Have the client send a `NON` `GET /sensors/temp` request to the server on UDP port `5683`.
3. Observe all UDP datagrams on the link.

### Expected observations

1. Client `C` sends a `NON` request (`Ver=1, T=NON, Code=0.01 GET`) with a non-empty Token and `Uri-Path` options `"sensors"` and `"temp"`.
2. Server `S` answers with a `NON` response (`Ver=1, T=NON, Code=2.05 Content`) echoing the exact request Token and carrying the resource payload.
3. Neither endpoint sends an `ACK` or `RST` datagram.

## Response matching by endpoint and token

- **Checks** — [RFC7252-MATCH-1](../../../standard/rfc7252/catalog.md#rfc7252-match-1).
- **Requirement** — a client matches responses by source endpoint address/port and exact Token (`0..8` octets, including zero-length Token `TKL=0`), ignores a response from an unexpected source endpoint or a `NON` response with an unknown Token, and rejects an unexpected `CON` response with an unknown Token by sending `RST`.

### Procedure

1. Build a mockup with client `C`, server `S`, and a third peer `F`.
2. Send a request with a zero-length Token (`TKL=0`) and a request with a multi-byte Token, and inject responses with a mismatched Token (`NON` and `CON`) and a response with a matching Token from the wrong source endpoint `F`.
3. Observe which responses the client accepts or rejects.

### Expected observations

1. The client accepts the response with `TKL=0` when its open request used `TKL=0`, and accepts the response whose multi-byte Token and source endpoint match its open request.
2. An injected `NON` response with an unknown Token or from foreign endpoint `F` is silently ignored and does not complete the client's open request.
3. An injected `CON` response with an unknown Token is rejected by the client with an Empty `RST` (`0.00`) echoing the injected response's Message ID.

## Confirmable retransmission and exponential backoff

- **Checks** — [RFC7252-CON-1](../../../standard/rfc7252/catalog.md#rfc7252-con-1), [RFC7252-CON-2](../../../standard/rfc7252/catalog.md#rfc7252-con-2), [RFC7252-PIGGY-1](../../../standard/rfc7252/catalog.md#rfc7252-piggy-1).
- **Requirement** — a `CON` request is answered with a piggybacked `ACK` echoing its Message ID and Token; when the `CON` is lost, the sender retransmits with the identical Message ID and Token at an initial timeout in `[ACK_TIMEOUT, ACK_TIMEOUT * ACK_RANDOM_FACTOR]` (`[2s, 3s]`) that doubles on each retry up to `MAX_RETRANSMIT` (`4`) times.

### Procedure

1. Build the plain pair mockup with relay `T`.
2. Drop the first two `CON` request transmissions at `T` and let the third pass, then in a second exchange drop all five transmissions (`1` initial + `4` retries).
3. Observe transmission timestamps, Message IDs, Tokens, and the server's piggybacked `ACK`.

### Expected observations

1. The first retry leaves `C` within `[2.0s, 3.0s]` after the initial `CON`, and the second retry leaves `C` after twice the initial timeout (`2 * d1`), all carrying the identical Message ID and Token.
2. When the third transmission reaches `S`, `S` answers with a single piggybacked `ACK` (`T=ACK, Code=2.05`) echoing the request's Message ID and Token.
3. When all transmissions of a `CON` exchange are dropped, `C` sends exactly `1 + MAX_RETRANSMIT` (`5`) transmissions and then declares the exchange timed out with no sixth transmission.

## Deferred separate response exchange

- **Checks** — [RFC7252-SEP-1](../../../standard/rfc7252/catalog.md#rfc7252-sep-1), [RFC7252-CON-2](../../../standard/rfc7252/catalog.md#rfc7252-con-2).
- **Requirement** — when a server defers its response to a `CON` request, it sends an Empty `ACK` (`0.00`, `TKL=0`) echoing the request Message ID to stop client request retransmissions, and later sends the response as a separate `CON` with a new Message ID and the client's Token, which the client acknowledges with an Empty `ACK` (`0.00`).

### Procedure

1. Build the plain pair mockup with server `S` configured to defer its response and send an immediate Empty `ACK`.
2. Send a `CON` `GET` request from `C` to `S`, and drop the first separate `CON` response on the relay so the server also exercises separate-response retransmission.
3. Observe the four-message (plus retry) sequence on the wire.

### Expected observations

1. `S` answers the `CON` request with an Empty `ACK` (`T=ACK, Code=0.00, TKL=0`) echoing the request's Message ID, and `C` does not retransmit the request while waiting for the separate response.
2. When the response is ready, `S` sends a `CON` response (`T=CON, Code=2.05`) carrying the client's request Token and a newly allocated server Message ID, and retransmits that `CON` response with the same server Message ID when the first attempt is dropped.
3. `C` answers the separate `CON` response with an Empty `ACK` (`T=ACK, Code=0.00, TKL=0`) echoing the server's response Message ID.

## Duplicate suppression and cached acknowledgment

- **Checks** — [RFC7252-DUP-1](../../../standard/rfc7252/catalog.md#rfc7252-dup-1).
- **Requirement** — an endpoint deduplicates incoming `CON` and `NON` messages by `(source endpoint, Message ID)` within `EXCHANGE_LIFETIME` or `NON_LIFETIME`, executing a request at most once, re-sending the cached `ACK` (or piggybacked response) when a duplicate `CON` arrives, and suppressing duplicate responses at the client.

### Procedure

1. Build the plain pair mockup with relay `T`.
2. Drop the server's first piggybacked `ACK` to a non-idempotent `CON` `POST` request so `C` retransmits the `CON` with the same Message ID; also duplicate a `NON` request and a separate `CON` response on the relay.
3. Observe the server's resource state and outbound `ACK` messages, and the client's response delivery count.

### Expected observations

1. When the retransmitted `CON` `POST` arrives with the same Message ID, `S` does not execute the `POST` a second time and re-sends the cached piggybacked `ACK`.
2. When a duplicate `NON` request arrives with the same Message ID within `NON_LIFETIME`, `S` suppresses the duplicate execution.
3. When a duplicate separate `CON` response arrives at `C`, `C` re-sends the Empty `ACK` and delivers the response to the application only once.

## Empty Confirmable ping and Reset handling

- **Checks** — [RFC7252-PING-1](../../../standard/rfc7252/catalog.md#rfc7252-ping-1), [RFC7252-CON-2](../../../standard/rfc7252/catalog.md#rfc7252-con-2).
- **Requirement** — an Empty (`0.00`, `TKL=0`) `CON` message provokes an Empty `RST` (`0.00`, `TKL=0`) echoing its Message ID, whereas an Empty `NON`, `ACK`, or `RST` elicits no packet; a `RST` answering an active `CON` request immediately cancels retransmission.

### Procedure

1. Build the plain pair mockup.
2. Inject an Empty `CON` (`0.00`) to `S`, inject Empty `NON`, `ACK`, and `RST` messages, and have `S` (or a test peer) answer an active `CON` request from `C` with `RST`.
3. Observe the wire.

### Expected observations

1. `S` answers the Empty `CON` (`0.00`) with an Empty `RST` (`T=RST, Code=0.00, TKL=0`) echoing the incoming Message ID.
2. Empty `NON`, `ACK`, and `RST` messages provoke no outbound packet.
3. When `C` receives `RST` matching its active `CON` request's Message ID, `C` cancels its retransmission timer immediately and sends no retries.

## Congestion admission and probing rate limit

- **Checks** — [RFC7252-CC-1](../../../standard/rfc7252/catalog.md#rfc7252-cc-1).
- **Requirement** — a client maintains at most `NSTART` (`1`) simultaneous outstanding interaction per destination endpoint, queuing subsequent requests until the in-flight interaction completes, and after an interaction to a non-responding peer times out, delays the next transmission to that peer by at least `lastAttemptWireBytes / PROBING_RATE`.

### Procedure

1. Build the plain pair mockup with `NSTART = 1` and `PROBING_RATE = 1 byte/s`.
2. Queue two `CON` requests at `C` for the same server `S`; in a second scenario, drop all transmissions of the first `CON` request so it times out and queue a follow-up request to the same silent peer.
3. Observe the send timestamps of each request on the wire.

### Expected observations

1. In the responsive scenario, `C` sends the second `CON` request only after receiving the `ACK`/response for the first `CON` request.
2. In the silent-peer scenario, after the first `CON` exchange exhausts `MAX_RETRANSMIT`, `C` holds the next request until at least `lastAttemptWireBytes / PROBING_RATE` seconds have elapsed.
