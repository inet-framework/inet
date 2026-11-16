# CoAP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

This document records implementation and testing notes for the CoAP over UDP (`NoSec`) model and protocol test suite.

## Design and testing notes

- **Payload representation.** `CoapHeader` (`FieldsChunk`) carries the 4-byte fixed header, the variable-length Token (`0..8` octets), the ascending delta-encoded options list, and `payloadMarkerPresent`. Application bodies travel as a separate `BytesChunk` after `CoapHeader` inside the `Packet`, so header dissection never copies large block payloads into header fields.
- **Malformed wire inputs.** `CoapHeaderSerializer::deserializeFields` marks malformed headers via `markIncorrect()` without throwing or aborting simulation execution, allowing `CoapEndpointBase` to silently drop `< 4`-byte, `Ver != 1`, and malformed `NON`/`ACK`/`RST` datagrams while answering malformed `CON` headers (`>= 4` bytes, `Ver == 1`) with `RST`.
- **Custom UDP ports in tests.** `CoapProtocol` registers default UDP port `5683` in `ProtocolGroup::getUdpProtocolGroup()`; UDP port `5684` is NOT registered as cleartext CoAP because `coaps://` wire packets carry `dtlsProtocol` records dissected by `DtlsProtocolDissector`.
- **8192-block upload history bound.** Because an 8192-block `Block1` transfer (`128 KiB` at `16`-byte blocks) sends 8192 distinct `CON` messages within `EXCHANGE_LIFETIME`, `maxDedupEntriesPerPeer` defaults to `8192` so active transfers never evict earlier blocks within the same transfer window.
- **DTLS 1.3 session binding and record limits.** CoAP over DTLS uses `IDtls` (`DtlsSocket`) and inherits the IPsec-style cryptographic abstraction from [`model/dtls/scope.md`](../dtls/scope.md) (no real AEAD, signatures, or computed UDP checksums over protected records). CoAP exchanges, duplicate suppression, Observe relations, and Block1/Block2 state are bound to `IDtls::AssociationId` and preserved across in-session `KeyUpdate` rotations alongside non-reused 8-byte Tokens (`RFC 9175 §4.2`). Outbound CoAP datagrams exceeding `Dtls.udpPayloadLimit - 22` (`1178 B` at the default `1200 B` limit) fail via `IDtls::ICallback::sendError`.
