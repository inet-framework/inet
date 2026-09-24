# RFC 5462 (MPLS Label Stack Entry: "EXP" Field Renamed to "Traffic Class" Field) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC5462-*` · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 5462, MPLS Label Stack Entry: "EXP" Field Renamed to "Traffic Class" Field, of
February 2009, a document of the in-scope set of MPLS. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc5462.txt`](../../../../../../standards/RFC/rfc5462.txt) —
  MPLS Label Stack Entry: "EXP" Field Renamed to "Traffic Class" Field, February 2009. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc5462.txt>.

It updates RFC 3032, whose label stack entry is in [`rfc3032/catalog.md`](../rfc3032/catalog.md).
The in-scope sections are §2.1 and §3.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/mpls/standards.md). The features these statements build are in
[`features.md`](../../protocol/mpls/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`mpls/coverage.md`](../../model/mpls/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc5462.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC5462-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 5462 uses the keywords of RFC 2119 in capitals;
  the entry records `must`, `must not`, `should`, `should not` or `may` for them (`must` for
  REQUIRED), and `description` for a normative sentence with no keyword. Where the text writes a
  keyword in lower case, the entry records the word and says so, for example `must (lower
  case)`. An entry with two keywords names what each one covers. A step of a numbered procedure
  that states an action is an entry of its own.
- **Class** — how a test can observe the statement: `wire` (fields of a header on a link, or
  the presence or absence of a packet), `end-to-end` (what an implementation accepts, delivers
  or discards, seen in what it sends or delivers afterwards), `error-signal` (an ICMP error
  report), `internal` (state or a database inside an implementation, an audit log), `encoding`
  (the exact bit layout, sizes and alignment).
- **Check idea** — one or two sentences, still without implementation names.

## Index

| ID | Statement |
| --- | --- |
| [RFC5462-TC-1](#rfc5462-tc-1) | In the label stack entry, the three-bit field renamed from Exp to Traffic Class (TC) keeps its bit width and position between the Label field and the Bottom-of-Stack bit. |
| [RFC5462-TC-2](#rfc5462-tc-2) | A QoS or an ECN function may rewrite all or some of the bits of the TC field, because the field has few bits. |
| [RFC5462-TC-3](#rfc5462-tc-3) | When an LSR pushes a label onto an existing label stack, it may copy the TC field into the pushed label stack entry, so every entry in the stack carries the same TC field. |

## TC: The Traffic Class field

### RFC5462-TC-1

**In the label stack entry, the three-bit field renamed from Exp to Traffic Class (TC) keeps its bit width and position between the Label field and the Bottom-of-Stack bit.**

> "3.  Traffic Class (TC) field" — §2.1, `rfc5462.txt:177`; "This three-bit field is used to carry traffic class information,
> and the change of the name is applicable to all places it occurs
> in IETF RFCs and other IETF documents." — §2.1, `rfc5462.txt:179-181`; "|                Label                  | TC  |S|       TTL     |" — §2.1,
> `rfc5462.txt:208`; "TC:     Traffic Class field, 3 bits" — §2.1, `rfc5462.txt:212`

- Strength: description. Class: encoding.
- Check idea: decode the label stack entry of a labeled packet and check that the three-bit
  field between the Label field and the Bottom-of-Stack bit is named the Traffic Class field,
  at the same width and position as the former Exp field.

### RFC5462-TC-2

**A QoS or an ECN function may rewrite all or some of the bits of the TC field, because the field has few bits.**

> "Due to the limited number of bits in the TC field, their use for QoS
> and ECN (Explicit Congestion Notification) functions is intended to
> be flexible.  These functions may rewrite all or some of the bits in
> the TC field." — §3, `rfc5462.txt:377-380`

- Strength: may (lower case). Class: wire.
- Check idea: capture the TC field of a labeled packet at successive LSRs that apply a QoS or an
  ECN function and check that the function can change all or part of the TC bits.

### RFC5462-TC-3

**When an LSR pushes a label onto an existing label stack, it may copy the TC field into the pushed label stack entry, so every entry in the stack carries the same TC field.**

> "the TC field may be copied to the label stack entries
> that are pushed onto the label stack.  This is done to avoid label
> stack entries that are pushed onto an existing label stack having
> different TC fields from the rest of the label stack entries." — §3, `rfc5462.txt:383-386`

- Strength: may (lower case). Class: wire.
- Check idea: have an LSR push a new label onto an existing label stack and check that the
  pushed label stack entry's TC field can carry the same value as the rest of the stack.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§2.1 and §3, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 (introduction), §2.2 to §2.4 (the changes to
  RFC 3270 and RFC 5129, and the scope of the change) and §4 to §6.

Within the in-scope sections, these parts have no entry, with the reason:

In §2.1 and §3:

- `rfc5462.txt:153-157` — introduces §2 ("The three RFCs 3032, 3270, and 5129 are now updated
  according to the following."); organizational.
- `rfc5462.txt:158-176` — quotes the pre-amendment RFC 3032 text ("This three-bit field is
  reserved for experimental use.") for context before the rename; the current field is cataloged
  from RFC 3032 itself and superseded by RFC5462-TC-1.
- `rfc5462.txt:183-184` — "RFC 3270 and RFC 5129 update the definition of the TC field and
  describe how to use the field."; changes to RFC 3270 and RFC 5129 are outside this range.
- `rfc5462.txt:186-201` — introduces and shows the OLD Figure 1 (Label/Exp/S/TTL) from RFC 3032,
  quoted for context before the rename; already cataloged from RFC 3032 itself.
- `rfc5462.txt:202-207` — introduces the NEW Figure 1 and its unchanged ruler and box-drawing
  lines; the renamed field row and its width are in RFC5462-TC-1.
- `rfc5462.txt:209-217` — the rest of the new Figure 1 (box-bottom line, the unchanged
  Label/S/TTL field lines, and the "Figure 1 (new)" caption); only the renamed TC field row is
  new content, cataloged in RFC5462-TC-1.
- `rfc5462.txt:218-220` — a note that "Figure 1 (new)" is only a label used in this document to
  distinguish the two figures, and will still be "Figure 1" in RFC 3032; editorial, not a rule.
- `rfc5462.txt:221-230` — blank lines, a page break, and the heading "2.2.  RFC 3270", which
  begins the next (out-of-range) subsection.
- `rfc5462.txt:375-376` — heading "3.  Use of the TC field"; heading only.
- `rfc5462.txt:382-383` — "Current implementations look at the TC field with and without label
  context"; an informative statement of practice, with no concrete, testable difference between
  "with" and "without label context".
- `rfc5462.txt:387-398` — blank lines and the page break (page footer/header) to the end of §3.
