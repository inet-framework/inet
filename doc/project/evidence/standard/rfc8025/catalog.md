# RFC 8025 (6LoWPAN) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC8025-*` · **Stands on:** [standards.md](../../protocol/6lowpan/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This is the statement-catalog artifact for RFC 8025 in the selected 6LoWPAN family.
Source: [rfc8025.txt](rfc8025.txt), downloaded unmodified on 2026-09-18 from
<https://www.rfc-editor.org/rfc/rfc8025.txt>. The publication version and family
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
| [RFC8025-PAGE-1](#rfc8025-page-1) | Paging dispatch uses 1111 followed by the four-bit page number. |
| [RFC8025-PAGE-2](#rfc8025-page-2) | Each packet starts in the default page-zero parsing context. |
| [RFC8025-PAGE-3](#rfc8025-page-3) | A selected page stays active until another paging dispatch. |
| [RFC8025-PAGE-4](#rfc8025-page-4) | Page one retains the LOWPAN_IPHC encodings from page zero. |
| [RFC8025-ORDER-1](#rfc8025-order-1) | Mesh headers precede the first page-one paging dispatch. |
| [RFC8025-ORDER-2](#rfc8025-order-2) | RFC 4944 fragmentation headers precede the first page-one paging dispatch. |
| [RFC8025-NALP-1](#rfc8025-nalp-1) | NALP is defined only for the first octet of the packet. |

## Checkable statements

### RFC8025-PAGE-1

**Paging dispatch uses 1111 followed by the four-bit page number.**

> Pages are delimited in a 6LoWPAN packet by a Paging Dispatch value
> that indicates the next current Page.  The Page Number is encoded in
> a Paging Dispatch with the Value Bit Pattern of 11 11xxxx, where xxxx
> is the Page Number, 0 to 15, as described in Figure 1:

— §3, `rfc8025.txt:138-141`.

- Strength: description. Class: encoding.
- Check idea: Encode and decode all page numbers 0 through 15.

### RFC8025-PAGE-2

**Each packet starts in the default page-zero parsing context.**

> Values of the Dispatch byte defined in [RFC4944] are considered as
> belonging to the Page 0 parsing context, which is the default and
> does not need to be signaled explicitly at the beginning of a 6LoWPAN
> packet.  This ensures backward compatibility with existing
> implementations of 6LoWPAN.

— §3, `rfc8025.txt:151-155`.

- Strength: description. Class: encoding.
- Check idea: Decode an unprefixed base packet after a packet that switched pages and verify no page state leaks between packets.

### RFC8025-PAGE-3

**A selected page stays active until another paging dispatch.**

> A Page (say Page N) is said to be active once the Page N Paging
> Dispatch is parsed, and it remains active until another Paging
> Dispatch is parsed.

— §3, `rfc8025.txt:185-187`.

- Strength: description. Class: encoding.
- Check idea: Decode multiple dispatches on one page followed by a page switch and verify each interpretation.

### RFC8025-PAGE-4

**Page one retains the LOWPAN_IPHC encodings from page zero.**

> The Dispatch bits defined for LOWPAN_IPHC by the "Compression
> Format for IPv6 Datagrams over IEEE 802.15.4-Based Networks"
> [RFC6282] are defined with the same values in Page 1, so there is
> no need to switch context from Page 1 to Page 0 to decode a packet
> that is encoded per [RFC6282].

— §4, `rfc8025.txt:194-198`.

- Strength: description. Class: encoding.
- Check idea: Decode the same IPHC datagram in page zero and after a page-one switch.

### RFC8025-ORDER-1

**Mesh headers precede the first page-one paging dispatch.**

> Mesh Headers represent Layer 2 information and are processed
> before any Layer 3 information that is encoded in Page 1.  If a
> 6LoWPAN packet requires a Mesh Header, the Mesh Header MUST always
> be placed in the packet before the first Page 1 Paging Dispatch,
> if any.

— §4, `rfc8025.txt:200-204`.

- Strength: must. Class: encoding.
- Check idea: Encode a mesh packet using page one and inspect the ordering.

### RFC8025-ORDER-2

**RFC 4944 fragmentation headers precede the first page-one paging dispatch.**

> For the same reason, Fragment Headers as defined in [RFC4944] MUST
> always be placed in the packet before the first Page 1 Paging
> Dispatch, if any.

— §4, `rfc8025.txt:206-208`.

- Strength: must. Class: encoding.
- Check idea: Encode fragmented page-one traffic and verify header placement.

### RFC8025-NALP-1

**NALP is defined only for the first octet of the packet.**

> The NALP Dispatch Bit Pattern as defined in [RFC4944] is only
> defined for the first octet in the packet.  Switching back to Page
> 0 for NALP inside a 6LoWPAN packet does not make sense.

— §4, `rfc8025.txt:210-212`.

- Strength: description. Class: encoding.
- Check idea: Check that an internal page switch to zero does not reinterpret later bytes as a new packet's NALP prefix.

## Areas outside this selection

The parsing context and header-order mechanisms in §§3–4 are selected. Registry administration and experimental allocation policy (§6) and security inherited from companion documents (§5) are outside the check selection. No support for a protocol allocated on another page is implied by paging support.
