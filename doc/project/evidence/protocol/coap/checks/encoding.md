# CoAP — English check procedures: encoding and format errors

> **Kind:** procedure · **Status:** current · **Seal:** none · **Owns:** — · **Stands on:** [checks.md](../checks.md), [rfc7252/catalog.md](../../../standard/rfc7252/catalog.md), [rfc7959/catalog.md](../../../standard/rfc7959/catalog.md)

The common mockups, the scenario constants, and the index are in [`checks.md`](../checks.md). The procedures come from the specifications only and name no simulation model and no code.

## Message encoding and option delta framing

- **Checks** — [RFC7252-FMT-1](../../../standard/rfc7252/catalog.md#rfc7252-fmt-1), [RFC7252-FMT-3](../../../standard/rfc7252/catalog.md#rfc7252-fmt-3), [RFC7252-FMT-5](../../../standard/rfc7252/catalog.md#rfc7252-fmt-5), [RFC7959-OPT-1](../../../standard/rfc7959/catalog.md#rfc7959-opt-1).
- **Requirement** — a CoAP message starts with a 4-octet header (`Ver=1`, `T`, `TKL` in `0..8`, `Code`, `Message ID`), followed by `TKL` token octets, options encoded in ascending option-number order using nibble (`0..12`), 1-byte (`13`), and 2-byte (`14`) extended delta/length fields, and a `0xFF` payload marker before any non-empty payload; `Observe` (`6`) and `Block1`/`Block2` (`27`/`23`) use variable-length big-endian unsigned integer encoding (`0..3` octets).

### Procedure

1. Build the plain pair mockup.
2. Send CoAP messages exercising 0-byte, 4-byte, and 8-byte tokens, empty payloads (no `0xFF`), non-empty payloads (with `0xFF`), options with deltas and lengths in `0..12`, `13..268` (nibble `13`), and `269..65804` (nibble `14`), `Observe` values of 0, 1, 2, and 3 octets, and `Block1`/`Block2` values of 0, 1, 2, and 3 octets.
3. Inspect the serialized UDP payload octets on the wire and the dissected fields at the receiver.

### Expected observations

1. Every datagram begins with version bits `01`, the expected message type `T`, token length `TKL`, code byte `(class << 5) | detail`, and big-endian 16-bit Message ID, followed by `TKL` token octets.
2. Options appear in non-decreasing option-number order; deltas and lengths in `13..268` use nibble `13` plus `value - 13`, and deltas and lengths in `269..65804` use nibble `14` plus 16-bit `value - 269`.
3. Messages without payload carry no `0xFF` marker, whereas messages with payload carry `0xFF` immediately before the payload octets.
4. `Observe` (`0` as 0 octets, `1..255` as 1 octet, `256..65535` as 2 octets, `65536..16777215` as 3 octets) and `Block1`/`Block2` (`(NUM << 4) | (M << 3) | SZX` in `0..3` octets) round-trip without loss.

## Malformed datagram rejection and silent discard

- **Checks** — [RFC7252-FMT-2](../../../standard/rfc7252/catalog.md#rfc7252-fmt-2), [RFC7252-FMT-4](../../../standard/rfc7252/catalog.md#rfc7252-fmt-4), [RFC7252-FMT-5](../../../standard/rfc7252/catalog.md#rfc7252-fmt-5), [RFC7252-FMT-6](../../../standard/rfc7252/catalog.md#rfc7252-fmt-6), [RFC7252-FMT-7](../../../standard/rfc7252/catalog.md#rfc7252-fmt-7), [RFC7252-FMT-8](../../../standard/rfc7252/catalog.md#rfc7252-fmt-8), [RFC7252-ERR-1](../../../standard/rfc7252/catalog.md#rfc7252-err-1), [RFC7959-OPT-1](../../../standard/rfc7959/catalog.md#rfc7959-opt-1).
- **Requirement** — datagrams shorter than 4 octets and datagrams with `Ver != 1` MUST be silently ignored; format errors (`TKL` in `9..15`, truncated token, option delta `15` when byte != `0xFF`, option length `15`, truncated extended option delta/length/value, `0xFF` payload marker with zero payload bytes, Code `0.00` with `TKL > 0` or trailing bytes, reserved code classes `1`, `6`, `7`) MUST be silently ignored on `NON`/`ACK`/`RST` and rejected with `RST` on `CON` when a full 4-octet header is present; overlong `Observe` (`> 3` octets) or `Block1`/`Block2` (`> 3` octets) options cause `4.02 Bad Option`, and `SZX == 7` causes `4.00 Bad Request`.

### Procedure

1. Build the plain pair mockup.
2. Inject each malformed datagram variant into the CoAP endpoint (`< 4` octets, `Ver != 1`, `TKL=9`, truncated token, `0xF0` delta-15 byte, length-15 nibble, truncated option value, trailing `0xFF` with no payload, non-empty `0.00` message, reserved code class `1`/`6`/`7`, 4-octet `Observe`, 4-octet `Block1`/`Block2`, and `SZX=7`), testing both `NON` and `CON` forms.
3. Observe the endpoint's outbound UDP datagrams.

### Expected observations

1. Datagrams shorter than 4 octets and datagrams with `Ver != 1` produce no outbound packet even when the `T` bits indicate `CON`.
2. Malformed `NON`, `ACK`, and `RST` datagrams produce no outbound packet.
3. Malformed `CON` datagrams that contain a complete 4-octet header with `Ver == 1` provoke an Empty `RST` (`0.00`, `TKL=0`) echoing the incoming Message ID.
4. `CON` requests with a 4-octet `Observe` or 4-octet `Block1`/`Block2` option return `4.02 Bad Option`, and `CON` requests with `SZX == 7` return `4.00 Bad Request`.
