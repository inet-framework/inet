# RFC 7252 (CoAP) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC7252-*` · **Stands on:** [standards.md](../../protocol/coap/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 7252, The Constrained Application Protocol (CoAP), June 2014. The catalog comes from the specification text only and contains no simulation model names and no code references.

Source: `standards/RFC/rfc7252.txt` (<https://www.rfc-editor.org/rfc/rfc7252.txt>, June 2014).

The family of CoAP documents and the in-scope UDP (`NoSec`) profile are defined in [`standards.md`](../../protocol/coap/standards.md). The protocol capabilities built from these statements are in [`features.md`](../../protocol/coap/features.md), and the state of the workflow lives in [`coverage.md`](../../model/coap/coverage.md).

## Index

| ID | Statement |
| --- | --- |
| [RFC7252-FMT-1](#rfc7252-fmt-1) | The 4-octet fixed header encodes version 1, message type, token length 0..8, code `c.dd`, and 16-bit Message ID. |
| [RFC7252-FMT-2](#rfc7252-fmt-2) | Token lengths 9..15 are reserved and MUST be processed as a message format error. |
| [RFC7252-FMT-3](#rfc7252-fmt-3) | Options are delta-encoded in ascending order using 1-byte (`13`) and 2-byte (`14`) extended delta and length fields. |
| [RFC7252-FMT-4](#rfc7252-fmt-4) | Option delta `15` (when the byte is not `0xFF`) and option length `15` are reserved and MUST be processed as a message format error. |
| [RFC7252-FMT-5](#rfc7252-fmt-5) | A non-empty payload is prefixed by `0xFF`; a `0xFF` marker followed by zero payload bytes is a message format error. |
| [RFC7252-FMT-6](#rfc7252-fmt-6) | Messages with unknown version numbers (`Ver != 1`) or fewer than 4 octets MUST be silently ignored. |
| [RFC7252-FMT-7](#rfc7252-fmt-7) | An Empty message (`0.00`) MUST have `TKL == 0` and no bytes after the 4-octet header. |
| [RFC7252-FMT-8](#rfc7252-fmt-8) | Message codes in reserved classes `1`, `6`, and `7` MUST be processed as a message format error. |
| [RFC7252-ERR-1](#rfc7252-err-1) | A message format error in a `CON` message with a full 4-octet header is rejected with `RST`, while format errors in `NON`, `ACK`, or `RST` are silently ignored. |
| [RFC7252-CON-1](#rfc7252-con-1) | A Confirmable (`CON`) message is retransmitted with the same Message ID and exponentially doubled timeout up to `MAX_RETRANSMIT` times until acknowledged or reset. |
| [RFC7252-CON-2](#rfc7252-con-2) | An `ACK` or `RST` echoes the 16-bit Message ID of the `CON` message it answers, and an Empty `ACK` or `RST` has code `0.00` and zero token length. |
| [RFC7252-NON-1](#rfc7252-non-1) | A Non-confirmable (`NON`) message carries a Message ID for duplicate detection, is never acknowledged with `ACK`, and draws a `NON` response. |
| [RFC7252-PIGGY-1](#rfc7252-piggy-1) | A piggybacked response is carried in the `ACK` answering a `CON` request, echoing both Message ID and Token without a separate empty `ACK`. |
| [RFC7252-SEP-1](#rfc7252-sep-1) | When a server defers its response to a `CON` request, it first sends an Empty `ACK` (`0.00`) and later sends the response as a separate `CON` with a new Message ID and the request Token. |
| [RFC7252-DUP-1](#rfc7252-dup-1) | An endpoint suppresses duplicate `CON` and `NON` messages within `EXCHANGE_LIFETIME` or `NON_LIFETIME`, executing a duplicate request at most once and re-sending the `ACK` for a duplicate `CON`. |
| [RFC7252-PING-1](#rfc7252-ping-1) | An Empty (`0.00`) `CON` message provokes a `RST` with the same Message ID, while an Empty `NON`, `ACK`, or `RST` elicits no response. |
| [RFC7252-CC-1](#rfc7252-cc-1) | An endpoint limits concurrent outstanding interactions per destination endpoint to `NSTART` (`1`) and bounds transmission to a non-responding peer by `PROBING_RATE` (`1 byte/s`). |
| [RFC7252-MATCH-1](#rfc7252-match-1) | A client matches responses to open requests by source endpoint and `Token` (`0..8` octets), rejecting an unmatched `CON` response with `RST` and ignoring an unmatched `NON` response. |
| [RFC7252-METH-1](#rfc7252-meth-1) | `GET`, `POST`, `PUT`, and `DELETE` return `2.05`/`2.03`, `2.01`/`2.04`, `2.01`/`2.04`, and `2.02`, with `4.04 Not Found` on missing target and `4.05 Method Not Allowed` on unsupported method. |
| [RFC7252-URI-1](#rfc7252-uri-1) | `Uri-Host` (`3`), `Uri-Port` (`7`), `Uri-Path` (`11`), and `Uri-Query` (`15`) identify the target resource, and `Location-Path` (`8`) and `Location-Query` (`20`) identify a created resource. |
| [RFC7252-OPT-1](#rfc7252-opt-1) | Unrecognized critical (odd) options cause `4.02 Bad Option` in requests and `RST` in `CON` responses, while unrecognized elective (even) options are silently ignored. |
| [RFC7252-COND-1](#rfc7252-cond-1) | `ETag` (`4`), `If-Match` (`1`), `If-None-Match` (`5`), `Content-Format` (`12`), `Accept` (`17`), and `Max-Age` (`14`, default `60s`) govern cache validation, conditional updates (`4.12`), and format negotiation (`4.06`). |
| [RFC7252-PROXY-1](#rfc7252-proxy-1) | An endpoint that does not support proxying rejects requests carrying `Proxy-Uri` (`35`) or `Proxy-Scheme` (`39`) with `5.05 Proxying Not Supported`. |
| [RFC7252-DTLS-1](#rfc7252-dtls-1) | The `coaps` URI scheme selects DTLS-secured CoAP with default UDP port `5684`. |
| [RFC7252-DTLS-2](#rfc7252-dtls-2) | In DTLS mode, CoAP endpoints MUST NOT send multicast messages; only unicast DTLS associations are used. |
| [RFC7252-DTLS-3](#rfc7252-dtls-3) | Over DTLS, Confirmable reliability, duplicate suppression, and response matching are bound to the active DTLS session and never cross distinct sessions. |

## Message format and error handling

### RFC7252-FMT-1

**The 4-octet fixed header encodes version 1, message type, token length 0..8, code `c.dd`, and 16-bit Message ID.**

> "CoAP messages are encoded in a simple binary format. The message format starts with a fixed-size 4-byte header. This is followed by a variable-length Token value, which can be between 0 and 8 bytes long. Following the Token value comes a sequence of zero or more CoAP Options in Type-Length-Value (TLV) format, optionally followed by a payload that takes up the rest of the datagram." — §3, `rfc7252.txt:699-705`

- Strength: must. Class: encoding plus wire.
- Check idea: serialize and dissect a CoAP message on the wire and verify `Ver == 1`, `T`, `TKL` (`0..8`), `Code`, `Message ID`, and `Token` octets.

### RFC7252-FMT-2

**Token lengths 9..15 are reserved and MUST be processed as a message format error.**

> "Lengths 9-15 are reserved, MUST NOT be sent, and MUST be processed as a message format error." — §3, `rfc7252.txt:753-754`

- Strength: must. Class: wire.
- Check idea: inject a datagram with `TKL` in `9..15` and verify message format error handling (`RST` on `CON`, silent drop on `NON`).

### RFC7252-FMT-3

**Options are delta-encoded in ascending order using 1-byte (`13`) and 2-byte (`14`) extended delta and length fields.**

> "Instead of specifying the Option Number directly, the instances MUST appear in order of their Option Numbers and a delta encoding is used between them: the Option Number for each instance is calculated as the sum of its delta and the Option Number of the preceding instance in the message." — §3.1, `rfc7252.txt:805-810`

- Strength: must. Class: encoding plus wire.
- Check idea: send messages carrying options with small deltas (`0..12`), 1-byte extended deltas/lengths (`13..268`), and 2-byte extended deltas/lengths (`269..65804`), and verify wire octets.

### RFC7252-FMT-4

**Option delta `15` (when the byte is not `0xFF`) and option length `15` are reserved and MUST be processed as a message format error.**

> "15: Reserved for the Payload Marker. If the field is set to this value but the entire byte is not the payload marker, this MUST be processed as a message format error." — §3.1, `rfc7252.txt:843-845`
>
> "15: Reserved for future use. If the field is set to this value, it MUST be processed as a message format error." — §3.1, `rfc7252.txt:865-866`

- Strength: must. Class: wire.
- Check idea: inject datagrams with nibble `0xF0` (delta 15, length 0) or option length nibble `15`, and verify format error rejection.

### RFC7252-FMT-5

**A non-empty payload is prefixed by `0xFF`; a `0xFF` marker followed by zero payload bytes is a message format error.**

> "Payload is prefixed by a fixed, one-byte Payload Marker (0xFF), which indicates the end of options and the start of the payload. The absence of the Payload Marker denotes a zero-length payload. The presence of a marker followed by a zero-length payload MUST be processed as a message format error." — §3, `rfc7252.txt:776-781`

- Strength: must. Class: wire.
- Check idea: verify that messages with payload include `0xFF`, messages without payload omit `0xFF`, and a message ending with `0xFF` and zero payload bytes is rejected as a format error.

### RFC7252-FMT-6

**Messages with unknown version numbers (`Ver != 1`) or fewer than 4 octets MUST be silently ignored.**

> "Messages with unknown version numbers MUST be silently ignored." — §3, `rfc7252.txt:734-735`
>
> "Messages with a size smaller than four bytes are not valid CoAP messages and MUST be silently ignored." — §3, `rfc7252.txt:793-795`

- Strength: must. Class: wire.
- Check idea: inject a 2-octet datagram and a datagram with `Ver != 1`, and verify that the receiver emits no `ACK`, no `RST`, and no response.

### RFC7252-FMT-7

**An Empty message (`0.00`) MUST have `TKL == 0` and no bytes after the 4-octet header.**

> "An Empty message has the Code field set to 0.00. The Token Length field MUST be set to 0 and bytes of data MUST NOT be present after the Message ID field. If there are any bytes, they MUST be processed as a message format error." — §4.1, `rfc7252.txt:930-934`

- Strength: must. Class: wire.
- Check idea: inject an Empty (`0.00`) `CON` with `TKL > 0` or trailing option/payload bytes and verify format error rejection (`RST`).

### RFC7252-FMT-8

**Message codes in reserved classes `1`, `6`, and `7` MUST be processed as a message format error.**

> "As a special case, Code 0.00 indicates an Empty message. In case of a request, the Code field indicates the Request Method; in case of a response, a Response Code. Possible values are maintained in the CoAP Code Registries (Section 12.1). The class values 1, 6, and 7 are reserved for future use; if received, they MUST be processed as a message format error." — §3, `rfc7252.txt:762-768`

- Strength: must. Class: wire.
- Check idea: inject a `CON` message with code class `1`, `6`, or `7` and verify `RST`.

### RFC7252-ERR-1

**A message format error in a `CON` message with a full 4-octet header is rejected with `RST`, while format errors in `NON`, `ACK`, or `RST` are silently ignored.**

> "Rejecting a Confirmable message is effected by sending a matching Reset message and otherwise ignoring it." — §4.2, `rfc7252.txt:1032-1033`
>
> "Rejecting a Non-confirmable message MAY involve sending a matching Reset message, and apart from the Reset message the rejected message MUST be silently ignored. Rejecting an Acknowledgment or Reset message (including the case where the Acknowledgment carries a piggybacked response) is effected by silently ignoring it." — §4.3, `rfc7252.txt:1118-1126`

- Strength: must. Class: wire.
- Check idea: inject malformed `CON`, `NON`, `ACK`, and `RST` datagrams and verify `RST` for `CON` with a complete 4-octet header and silence for `NON`, `ACK`, and `RST`.

## Messaging, reliability, and congestion control

### RFC7252-CON-1

**A Confirmable (`CON`) message is retransmitted with the same Message ID and exponentially doubled timeout up to `MAX_RETRANSMIT` times until acknowledged or reset.**

> "For a new Confirmable message, the initial timeout is set to a random duration (often not an integral number of seconds) between ACK_TIMEOUT and (ACK_TIMEOUT * ACK_RANDOM_FACTOR) (see Section 4.8), and the retransmission counter is set to 0. When the timeout is triggered and the retransmission counter is less than MAX_RETRANSMIT, the message is retransmitted, the retransmission counter is incremented, and the timeout is doubled." — §4.2, `rfc7252.txt:993-1001`

- Strength: must. Class: wire.
- Check idea: drop `CON` frames on the link and verify initial timeout in `[2s, 3s]`, doubled intervals, identical Message ID and Token across up to `MAX_RETRANSMIT` (`4`) retries, and timeout failure when all retries are lost.

### RFC7252-CON-2

**An `ACK` or `RST` echoes the 16-bit Message ID of the `CON` message it answers, and an Empty `ACK` or `RST` has code `0.00` and zero token length.**

> "The Acknowledgment message MUST echo the Message ID of the Confirmable message and MUST carry a response or be Empty (see Sections 5.2.1 and 5.2.2). The Reset message MUST echo the Message ID of the Confirmable message and MUST be Empty." — §4.2, `rfc7252.txt:1014-1018`

- Strength: must. Class: wire.
- Check idea: verify that every `ACK` and `RST` carries the exact Message ID of the `CON` it answers and that Empty `ACK` and `RST` carry `Code == 0.00` and `TKL == 0`.

### RFC7252-NON-1

**A Non-confirmable (`NON`) message carries a Message ID for duplicate detection, is never acknowledged with `ACK`, and draws a `NON` response.**

> "A Non-confirmable message always carries either a request or response and MUST NOT be Empty. A Non-confirmable message MUST NOT be acknowledged by the recipient." — §4.3, `rfc7252.txt:1110-1113`
>
> "If the request message is Non-confirmable, then the response SHOULD be returned in a Non-confirmable message as well." — §5.2.3, `rfc7252.txt:1601-1603`

- Strength: must. Class: wire.
- Check idea: send a `NON` request and verify that the server returns a `NON` response echoing the request Token without sending any `ACK`.

### RFC7252-PIGGY-1

**A piggybacked response is carried in the `ACK` answering a `CON` request, echoing both Message ID and Token without a separate empty `ACK`.**

> "In the most basic case for a Confirmable request, the response is carried directly in the Acknowledgment message that acknowledges the request. This is called a Piggybacked Response." — §5.2.1, `rfc7252.txt:1540-1543`

- Strength: must. Class: wire.
- Check idea: send a `CON` request when the server is configured for piggybacked responses and verify a single `ACK` carrying the response code, the request Message ID, and the request Token.

### RFC7252-SEP-1

**When a server defers its response to a `CON` request, it first sends an Empty `ACK` (`0.00`) and later sends the response as a separate `CON` with a new Message ID and the request Token.**

> "If the server is not able to respond immediately to a request carried in a Confirmable message, it simply responds with an Empty Acknowledgment message so that the client can stop retransmitting the request. When the response is ready, the server sends it in a new Confirmable message (which then in turn needs to be acknowledged by the client)." — §5.2.2, `rfc7252.txt:1559-1565`

- Strength: must. Class: wire.
- Check idea: configure a deferred server response delay, verify the immediate Empty `ACK` (`0.00`) stops client request retries, and verify the later separate `CON` response carries the client Token and a new Message ID that the client acknowledges with an Empty `ACK`.

### RFC7252-DUP-1

**An endpoint suppresses duplicate `CON` and `NON` messages within `EXCHANGE_LIFETIME` or `NON_LIFETIME`, executing a duplicate request at most once and re-sending the `ACK` for a duplicate `CON`.**

> "A recipient MUST be prepared to receive the same Confirmable message (as indicated by the Message ID and source endpoint) multiple times within EXCHANGE_LIFETIME, for example, when its Acknowledgment went missing or didn't reach the original sender before the timeout. The recipient SHOULD acknowledge each duplicate copy of a Confirmable message using the same Acknowledgment or Reset message, but SHOULD process any request or response in the message only once." — §4.5, `rfc7252.txt:1174-1182`

- Strength: must. Class: wire plus end-to-end.
- Check idea: duplicate a `CON` request (or drop the first `ACK` so the client retries) and duplicate a `NON` request; verify the server executes the request once, re-sends the cached `ACK` for the duplicate `CON`, and suppresses duplicate responses at the client.

### RFC7252-PING-1

**An Empty (`0.00`) `CON` message provokes a `RST` with the same Message ID, while an Empty `NON`, `ACK`, or `RST` elicits no response.**

> "Sending an Empty Confirmable message is a special case that has no meaning in the request/response layer. It is used to provoke a Reset message, which is sometimes Informally referred to as a 'CoAP ping'." — §4.2, `rfc7252.txt:1039-1042`

- Strength: must. Class: wire.
- Check idea: send an Empty `CON` (`0.00`) and verify an Empty `RST` echoing its Message ID; send Empty `NON`, `ACK`, and `RST` messages and verify they provoke no outbound packet.

### RFC7252-CC-1

**An endpoint limits concurrent outstanding interactions per destination endpoint to `NSTART` (`1`) and bounds transmission to a non-responding peer by `PROBING_RATE` (`1 byte/s`).**

> "In order to not cause congestion, clients (including proxies) MUST strictly limit the number of simultaneous outstanding interactions that they maintain to a given server (including proxies) to NSTART." — §4.7, `rfc7252.txt:1303-1306`
>
> "An endpoint that does not receive a response within expected time MUST limit its average data rate of transmission to a destination to PROBING_RATE." — §4.7, `rfc7252.txt:1322-1325`

- Strength: must. Class: wire.
- Check idea: queue multiple `CON` requests to one server and verify the second request waits until the first outstanding interaction completes; let a `CON` request time out completely and verify the next probe to that silent peer waits for `lastAttemptWireBytes / PROBING_RATE`.

## Request/response matching, methods, and options

### RFC7252-MATCH-1

**A client matches responses to open requests by source endpoint and `Token` (`0..8` octets), independently of Message ID in separate or `NON` responses, rejecting an unmatched `CON` response with `RST` and ignoring an unmatched `NON` response.**

> "A response is matched to a prior request by the Token, which is a 0- to 8-byte sequence that is generated by the client and echoed by the server without modification." — §5.3.1, `rfc7252.txt:1619-1622`
>
> "A client receiving a Confirmable response that is not a response to any request it is waiting for MUST reject the response with a Reset message." — §5.3.2, `rfc7252.txt:1665-1668`

- Strength: must. Class: wire.
- Check idea: verify zero-length and multi-byte tokens match responses to requests, a response from the wrong source endpoint is ignored, an unexpected `CON` response with an unknown token triggers `RST`, and an unexpected `NON` response is silently ignored.

### RFC7252-METH-1

**`GET`, `POST`, `PUT`, and `DELETE` return `2.05`/`2.03`, `2.01`/`2.04`, `2.01`/`2.04`, and `2.02`, with `4.04 Not Found` on missing target and `4.05 Method Not Allowed` on unsupported method.**

> "CoAP makes use of GET, POST, PUT, and DELETE methods in a similar manner to HTTP, with the semantics specified in this section." — §5.8, `rfc7252.txt:1975-1977`

- Strength: must. Class: wire.
- Check idea: exercise `GET` (`2.05 Content`), `POST` create (`2.01 Created` with `Location-Path`) and update (`2.04 Changed`), `PUT` create (`2.01`) and replace (`2.04`), `DELETE` (`2.02 Deleted` on existing and missing targets), missing `GET`/`POST` (`4.04 Not Found`), and disallowed methods (`4.05 Method Not Allowed`).

### RFC7252-URI-1

**`Uri-Host` (`3`), `Uri-Port` (`7`), `Uri-Path` (`11`), and `Uri-Query` (`15`) identify the target resource, and `Location-Path` (`8`) and `Location-Query` (`20`) identify a created resource.**

> "The Uri-Host, Uri-Port, Uri-Path, and Uri-Query Options are used to specify the target resource of a request to a CoAP origin server." — §5.10.1, `rfc7252.txt:2309-2311`
>
> "The Location-Path and Location-Query Options together indicate the relative URI of a resource created in response to a POST request." — §5.10.7, `rfc7252.txt:2548-2550`

- Strength: must. Class: wire.
- Check idea: send multi-segment `Uri-Path` and `Uri-Query` requests (including percent-encoded segments), verify resource resolution, and verify `Location-Path` on `2.01 Created`.

### RFC7252-OPT-1

**Unrecognized critical (odd) options cause `4.02 Bad Option` in requests and `RST` in `CON` responses, while unrecognized elective (even) options are silently ignored.**

> "Upon reception, unrecognized options of class 'elective' MUST be silently ignored. Unrecognized options of class 'critical' that occur in a Confirmable request MUST cause the return of a 4.02 (Bad Option) response. Unrecognized options of class 'critical' that occur in a Confirmable response, or piggybacked in an Acknowledgment, MUST cause the response to be rejected (Section 4.2)." — §5.4.1, `rfc7252.txt:1716-1723`

- Strength: must. Class: wire.
- Check idea: inject requests and responses carrying unknown odd (critical) and unknown even (elective) options and verify `4.02 Bad Option`, `RST`, or normal processing with the elective option ignored.

### RFC7252-COND-1

**`ETag` (`4`), `If-Match` (`1`), `If-None-Match` (`5`), `Content-Format` (`12`), `Accept` (`17`), and `Max-Age` (`14`, default `60s`) govern cache validation, conditional updates (`4.12`), and format negotiation (`4.06`).**

> "Upon receipt of a GET request with one or more ETag Options, the server checks whether one of the ETags matches the ETag of the selected representation. If so, the server issues a 2.03 (Valid) response instead of a 2.05 (Content) response." — §5.10.6.2, `rfc7252.txt:2525-2529`
>
> "If none of the conditions are met, then the server MUST NOT perform the requested method; instead, the server MUST respond with the 4.12 (Precondition Failed) Response Code." — §5.10.8, `rfc7252.txt:2575-2578`
>
> "If no preferred Content-Format can be returned, then a 4.06 'Not Acceptable' MUST be sent as a response." — §5.10.4, `rfc7252.txt:2456-2457`

- Strength: must. Class: wire.
- Check idea: test `GET` with matching `ETag` (`2.03 Valid` with echoed `ETag` and empty payload), `If-Match` and `If-None-Match` (`4.12 Precondition Failed` on mismatch and `2.04`/`2.01` on match), `Accept` mismatch (`4.06 Not Acceptable`), and `Max-Age` option encoding.

### RFC7252-PROXY-1

**An endpoint that does not support proxying rejects requests carrying `Proxy-Uri` (`35`) or `Proxy-Scheme` (`39`) with `5.05 Proxying Not Supported`.**

> "Proxy-Uri and Proxy-Scheme are critical options; an endpoint that does not support proxying returns 5.05 (Proxying Not Supported)." — §5.10.2, `rfc7252.txt:2423-2427`

- Strength: must. Class: wire.
- Check idea: inject a request containing `Proxy-Uri` (`35`) or `Proxy-Scheme` (`39`) and verify a `5.05 Proxying Not Supported` response.

## DTLS-secured CoAP

### RFC7252-DTLS-1

**The `coaps` URI scheme selects DTLS-secured CoAP with default UDP port `5684`.**

> "The 'coaps' URI scheme is used to identify CoAP resources accessed over DTLS ... The default port is 5684." — §6.2, §9.1, §12.6

- Strength: must. Class: wire plus end-to-end.
- Check idea: resolve `coaps://` requests to default UDP port `5684`, emit DTLS-protected UDP datagrams rather than cleartext CoAP, and reject mismatched `coap://` URIs in DTLS mode and `coaps://` URIs in `NoSec` UDP mode.

### RFC7252-DTLS-2

**In DTLS mode, CoAP endpoints MUST NOT send multicast messages; only unicast DTLS associations are used.**

> "In DTLS mode, a CoAP endpoint MUST NOT send multicast messages." — §9.1

- Strength: must. Class: end-to-end.
- Check idea: attempt a CoAP-over-DTLS request to a multicast destination address and verify the endpoint rejects the request without transmitting a multicast DTLS packet.

### RFC7252-DTLS-3

**Over DTLS, Confirmable reliability, duplicate suppression, and response matching are bound to the active DTLS session and never cross distinct sessions.**

> "The following rules are added for matching an Acknowledgement message or Reset message to a Confirmable message, or a Reset message to a Non-confirmable message: The DTLS session MUST be the same ... The following rules are added for matching a response to a request: The DTLS session MUST be the same ..." — §9.1.1, §9.1.2

- Strength: must. Class: wire plus end-to-end.
- Check idea: drop DTLS application datagrams to verify CoAP `CON` retransmission and duplicate `MID` suppression inside one DTLS session, and replace a DTLS session on the same `(IP, port)` tuple while a separate response or incomplete transfer is pending to verify pending work is cancelled and the replacement session does not suppress the reused `MID` or deliver old responses.
