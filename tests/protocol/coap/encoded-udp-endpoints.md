# Encoded UDP Endpoints Verification (`C2`)

This review note summarizes the C2 implementation and verification scope under `src/inet/applications/coap/` and `tests/protocol/coap/`.

## Components Added in C2

- `CoapProtocol` (`src/inet/applications/coap/CoapProtocol.h/.cc`): Feature-local `Protocol("coap", "CoAP")` descriptor and `COAP_UDP_PORT = 5683` registration on `ProtocolGroup::getUdpProtocolGroup()` via `EXECUTE_PRE_NETWORK_SETUP`.
- `CoapHeader` & `CoapHeaderSerializer` (`CoapHeader.msg`, `CoapHeaderSerializer.h/.cc`): RFC 7252 fixed header, 0–8B token, sorted TLV options with nibble `13`/`14` extended delta and length encoding, minimal unsigned integer encoding (`encodeCoapUintValue`/`decodeCoapUintValue`), `0xFF` payload marker rules, and structural/format validation (`parseCoapDatagramBytes`, `validateCoapHeaderStructure`).
- `CoapProtocolDissector` & `CoapProtocolPrinter`: Registered protocol dissector and printer for `udp.destPort == 5683 || udp.srcPort == 5683`, guaranteeing `packet->getDataLength() == B(0)` on every return path.
- `CoapEndpointBase`, `CoapClient`, `CoapServer`: Lifecycle-managed UDP endpoints implementing RFC 7252 message parsing, silent-ignore vs Empty `RST` malformed rules, NON deduplication within `NON_LIFETIME = 145s`, 8-byte monotonic token and 16-bit MID allocation/reservation, RFC 7252 §6.4 URI decomposition, response matching (`token`, `peerAddr`, `peerPort`), critical option checks, and minimal read-only XML resource `GET` serving.
- `.oppfeatures` and `doc/whatsnew/compatible/coap-udp-profile.txt`: Added `Coap` (`-DINET_WITH_COAP`), `CoapTests`, and release note entry.

## Conformance Tests Run

- `Rfc7252MessageEncoding.test`: Independent raw RFC 7252 encoder/decoder checks on option delta/length `13`/`14`, percent-splitting `Uri-Path` and `Uri-Query` before `%XX` decode, empty-body `0xFF` omission, and port `5683` vs `15683` dissection boundaries.
- `Rfc7252MalformedDatagrams.test`: Validates 0B/3B datagrams, `Ver != 1`, `TKL > 8`, reserved code classes (`1`/`6`/`7`), truncated/nibble-`15` options, trailing `0xFF` without payload, empty `NON`, non-empty `RST`, unrecognized critical options in `CON` (`4.02`) vs `NON` (ignored), and invalid typed `CoapHeader` chunks.
- `Rfc7252NonConfirmable.test`: Verifies IPv4 and IPv6 NON `GET` exchanges, `4.04`/`4.05`/`4.06` errors, and duplicate NON suppression inside `145s` followed by fresh execution after `145s`.
- `Rfc7252ResponseMatching.test`: Verifies token/source-port mismatch `RST`, critical response option rejection, pending NON request expiry at `EXCHANGE_LIFETIME = 247s`, and token/MID wrap and exhaustion handling.
