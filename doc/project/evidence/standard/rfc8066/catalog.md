# RFC 8066 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8066-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 8066 in the selected 6LoWPAN family.
Source: [rfc8066.txt](rfc8066.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc8066.txt>. The publication version and family
relationships are pinned in the linked standards map.

The catalog is a scoped selection, not an exhaustive inventory or a claim of test
coverage. Its scope exclusions are listed at the end. No test-depth level is claimed.
Statements and check ideas are independent of any simulation implementation.

Quotes retain the wording and line breaks of the cached text; leading indentation
is omitted; excerpts may start or end within a line. Line references count physical
newline-delimited lines, including page
headers. Strength records the source keyword, including qualified permissions;
`description` denotes a format or procedure stated without a requirement keyword.
Each check idea is a proposed observation, not a test result. Conditional rules apply
only to the roles, modes and prerequisites named in their source paragraphs.

## Index

| ID | Statement |
| --- | --- |
| [RFC8066-ESC-1](#rfc8066-esc-1) | The ESC dispatch octet is 0x40. |
| [RFC8066-ESC-2](#rfc8066-esc-2) | The octet immediately after ESC identifies the extension type. |
| [RFC8066-ESC-3](#rfc8066-esc-3) | Extensions cannot redefine existing ESC extension codes. |
| [RFC8066-ESC-4](#rfc8066-esc-4) | An extension does not change existing dispatch semantics. |
| [RFC8066-ESC-5](#rfc8066-esc-5) | Processing an unknown EET drops the packet. |
| [RFC8066-ESC-6](#rfc8066-esc-6) | A router should forward an EET-bearing packet when it does not process unknown extension octets. |
| [RFC8066-ORDER-1](#rfc8066-order-1) | ESC dispatches precede LOWPAN_IPHC. |

## Checkable statements

### RFC8066-ESC-1

**The ESC dispatch octet is 0x40.**

> ESC: The left-most octet is the ESC dispatch type containing
> '01000000'.

— §3, `rfc8066.txt:160-161`.

- Strength: description. Class: encoding.
- Check idea: Encode ESC and check the exact dispatch value, independently of the EET value.

### RFC8066-ESC-2

**The octet immediately after ESC identifies the extension type.**

> ESC Extension Type (EET): It is the first octet following the ESC
> dispatch type.

— §3, `rfc8066.txt:163-164`.

- Strength: description. Class: encoding.
- Check idea: Use distinct EET values and verify that payload decoding is selected from the extension namespace.

### RFC8066-ESC-3

**Extensions cannot redefine existing ESC extension codes.**

> Extended Dispatch Payload (EDP): This part of the frame format must
> be defined by the corresponding extension type.  A specification is
> required to define the usage of each extension type and its
> corresponding Extension Payload.  For the sake of interoperability,
> specifications of extension octets MUST NOT redefine the existing ESC
> Extension Type codes.

— §3, `rfc8066.txt:182-187`.

- Strength: must not. Class: encoding.
- Check idea: Review extension codec assignments for collisions with the specified EET codes.

### RFC8066-ESC-4

**An extension does not change existing dispatch semantics.**

> Section 5.1 of RFC 4944 indicates that the Extension Type field may
> contain additional dispatch values larger than 63, as corrected by
> [Err4359].  For the sake of interoperability, the new dispatch type
> (EET) MUST NOT modify the behavior of existing dispatch types
> [RFC4944].

— §3, `rfc8066.txt:189-193`.

- Strength: must not. Class: encoding.
- Check idea: Decode existing dispatch types with the extension enabled and disabled and compare their meaning.

### RFC8066-ESC-5

**Processing an unknown EET drops the packet.**

> If an implementation following this document, during processing of
> the received packet, reaches an ESC dispatch type for which it does
> not understand the ESC Extension Type (EET) octets, it MUST drop that
> packet.  However, it is important to clarify that a router node
> SHOULD forward a 6LoWPAN packet with the EET octets as long as it
> does not attempt to process any unknown ESC extension octets.

— §3.1, `rfc8066.txt:203-208`.

- Strength: must. Class: end-to-end.
- Check idea: Reach an unsupported EET during parsing and verify no payload delivery; distinguish forwarding that does not process that EET.

### RFC8066-ESC-6

**A router should forward an EET-bearing packet when it does not process unknown extension octets.**

> If an implementation following this document, during processing of
> the received packet, reaches an ESC dispatch type for which it does
> not understand the ESC Extension Type (EET) octets, it MUST drop that
> packet.  However, it is important to clarify that a router node
> SHOULD forward a 6LoWPAN packet with the EET octets as long as it
> does not attempt to process any unknown ESC extension octets.

— §3.1, `rfc8066.txt:203-208`.

- Strength: should. Class: wire.
- Check idea: Forward a packet whose opaque extension is outside the router's processing path and check preservation.

### RFC8066-ORDER-1

**ESC dispatches precede LOWPAN_IPHC.**

> The sequence and order of ESC extension octets with respect to the
> 6LoWPAN Mesh header and LOWPAN_IPHC header are described below.  When
> the LOWPAN_IPHC dispatch type is present, ESC dispatch types MUST
> appear before the LOWPAN_IPHC dispatch type in order to maintain
> backward compatibility with Section 3.2 of RFC 6282.  The following
> diagrams provide examples of ESC extension octet usages:

— §3.2, `rfc8066.txt:233-238`.

- Strength: must. Class: encoding.
- Check idea: Encode an ESC-bearing compressed IPv6 packet and verify the extension precedes IPHC.

## Areas outside this selection

The ESC format, unknown-type processing and IPHC ordering in §§3–3.2 are selected. The complete EET allocation table, G.9903-specific use (§3.3), the NALP discussion (§3.4), registration procedures (§4) and inherited security considerations (§5) are outside this selection.
