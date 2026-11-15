# CoAP — quirks and follow-ups

> **Kind:** reference · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [results.md](results.md), [coverage.md](coverage.md)

This document records implementation and testing notes for the CoAP over UDP (`NoSec`) model and protocol test suite.

## Design and testing notes

- **Payload representation.** `CoapHeader` (`FieldsChunk`) carries the 4-byte fixed header, the variable-length Token (`0..8` octets), the ascending delta-encoded options list, and `payloadMarkerPresent`. Application bodies travel as a separate `BytesChunk` after `CoapHeader` inside the `Packet`, so header dissection never copies large block payloads into header fields.
- **Malformed wire inputs.** `CoapHeaderSerializer::deserializeFields` marks malformed headers via `markIncorrect()` without throwing or aborting simulation execution, allowing `CoapEndpointBase` to silently drop `< 4`-byte, `Ver != 1`, and malformed `NON`/`ACK`/`RST` datagrams while answering malformed `CON` headers (`>= 4` bytes, `Ver == 1`) with `RST`.
- **Custom UDP ports in tests.** `CoapProtocol` registers default UDP ports `5683` and `5684` in `ProtocolGroup::getUdpProtocolGroup()`, and `CoapEndpointBase` registers any non-default `localPort` or `destPort` at initialization so `PacketFilter` expressions (`coap.*` / `CoapHeader.*`) dissect CoAP packets on arbitrary ports.
- **8192-block upload history bound.** Because an 8192-block `Block1` transfer (`128 KiB` at `16`-byte blocks) sends 8192 distinct `CON` messages within `EXCHANGE_LIFETIME`, `maxDedupEntriesPerPeer` defaults to `8192` so active transfers never evict earlier blocks within the same transfer window.
