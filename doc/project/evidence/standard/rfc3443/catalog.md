# RFC 3443 (Time To Live (TTL) Processing in Multi-Protocol Label Switching (MPLS) Networks) — catalog of checkable statements

> **Kind:** what · **Status:** current · **Seal:** none · **Owns:** `RFC3443-*` · **Stands on:** [standards.md](../../protocol/mpls/standards.md), [derive-tests-from-a-standard.md](../../../guide/derive-tests-from-a-standard.md)

This document is the step 3 artifact of the standards test workflow for RFC 3443, Time To Live (TTL) Processing in Multi-Protocol Label Switching (MPLS) Networks, of
January 2003, a document of the in-scope set of MPLS. The catalog comes from the RFC text
only. It contains no simulation model names and no code references.

Source, in the `standards` project beside the INET tree:

- [`standards/RFC/rfc3443.txt`](../../../../../../standards/RFC/rfc3443.txt) —
  Time To Live (TTL) Processing in Multi-Protocol Label Switching (MPLS) Networks, January 2003. Downloaded 2026-09-23 from
  <https://www.rfc-editor.org/rfc/rfc3443.txt>.

It updates RFC 3032, whose TTL rules are in [`rfc3032/catalog.md`](../rfc3032/catalog.md). The
in-scope sections are §2 and §3.

The family of documents and the in-scope set are in
[`standards.md`](../../protocol/mpls/standards.md). The features these statements build are in
[`features.md`](../../protocol/mpls/features.md).

The state of the workflow — which statement a check targets, which test carries it, and
what the run said — is **not** in this document. It lives in the coverage ledger,
[`mpls/coverage.md`](../../model/mpls/coverage.md). Keeping it out is deliberate: this catalog
states what the standard says, so a new test or a new run must never force an edit here.

Quotes are verbatim. A reference such as `rfc3443.txt:1203-1205` points to lines of that
file. Where a quote crosses a page break of the RFC, the reference names both line groups.

The entries were drafted section by section and merged; the sections are the areas below, in
the order of the document.

## How to read an entry

- **ID** — `RFC3443-AREA-n`. The ID is stable forever. New entries go at the end of an area.
- **Quote** — the verbatim sentence, with a line reference into the text.
- **Strength** — the word the document uses. RFC 3443 uses the keywords of RFC 2119 in capitals;
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
| [RFC3443-TERM-1](#rfc3443-term-1) | An LSR performs no checks on the iTTL value. |
| [RFC3443-TERM-2](#rfc3443-term-2) | The oTTL value equals the iTTL value minus one, unless a later rule states otherwise. |
| [RFC3443-TERM-3](#rfc3443-term-3) | An LSR does not forward a packet whose oTTL value is not greater than zero. |
| [RFC3443-TERM-4](#rfc3443-term-4) | An LSR performs the oTTL check only when it sets an outgoing TTL field to oTTL. |
| [RFC3443-MOD-1](#rfc3443-mod-1) | For a Uniform Model LSP, the inner and the outer TTL values are synchronized at the LSP ingress and at the LSP egress. |
| [RFC3443-MOD-2](#rfc3443-mod-2) | For a Short Pipe Model LSP with penultimate hop popping, the egress LSR only decrements the exposed header's TTL, because the penultimate node already popped the label. |
| [RFC3443-MOD-3](#rfc3443-mod-3) | For a Short Pipe Model LSP, the tunneled packet's TTL decreases by two in total, with or without penultimate hop popping. |
| [RFC3443-MOD-4](#rfc3443-mod-4) | For a Pipe Model LSP without penultimate hop popping, the TTL treatment equals the Short Pipe Model without penultimate hop popping. |
| [RFC3443-ITTL-1](#rfc3443-ittl-1) | When the incoming packet is an IP packet, the iTTL equals that packet's TTL value. |
| [RFC3443-ITTL-2](#rfc3443-ittl-2) | When an LSR pushes, swaps, or penultimate-hop-pops an incoming MPLS packet, the iTTL equals the TTL of the topmost incoming label. |
| [RFC3443-ITTL-3](#rfc3443-ittl-3) | When an LSR pops a label of a Pipe Model LSP, the iTTL equals the TTL field of the header exposed after the pop, whether that header is an IP header or an MPLS label. |
| [RFC3443-ITTL-4](#rfc3443-ittl-4) | When an LSR pops a label of a Uniform Model LSP, the iTTL equals the TTL of the popped label. |
| [RFC3443-ITTL-5](#rfc3443-ittl-5) | When an LSR pops two or more labels in sequence, it uses the iTTL computed at the previous pop as the TTL of the next label popped, and ignores that label's own TTL. |
| [RFC3443-OTTL-1](#rfc3443-ottl-1) | When an LSR routes a packet as an IP packet, it sets the IP packet's TTL value to oTTL. |
| [RFC3443-OTTL-2](#rfc3443-ottl-2) | For a swap-only operation, an LSR sets the TTL field of the outgoing label to oTTL. |
| [RFC3443-OTTL-3](#rfc3443-ottl-3) | For a penultimate hop pop, an LSR performs the oTTL check whether or not the oTTL is used to update the outgoing header's TTL field. |
| [RFC3443-OTTL-4](#rfc3443-ottl-4) | For a penultimate hop pop of a Short Pipe Model label, an LSR neither checks nor updates the TTL field of the exposed header. |
| [RFC3443-OTTL-5](#rfc3443-ottl-5) | For a penultimate hop pop of a Uniform Model label, an LSR sets the TTL field of the exposed header to oTTL. |
| [RFC3443-PUSH-1](#rfc3443-push-1) | For a pushed Uniform Model label, an LSR copies the TTL from the label or the IP packet immediately underneath it. |
| [RFC3443-PUSH-2](#rfc3443-push-2) | For a pushed Pipe Model or Short Pipe Model label, an LSR sets the TTL field to a value the network operator configures. |
| [RFC3443-PUSH-3](#rfc3443-push-3) | By default, most implementations set a pushed Pipe Model or Short Pipe Model label's TTL to 255. |
| [RFC3443-IMPL-1](#rfc3443-impl-1) | An LSR may decrement iTTL, or determine oTTL, by more than one, only to compensate for network nodes that cannot decrement TTL values themselves. |
| [RFC3443-IMPL-2](#rfc3443-impl-2) | When an LSR decrements the iTTL, it never lets the value become negative. |
| [RFC3443-IMPL-3](#rfc3443-impl-3) | For a Short Pipe Model LSP with penultimate hop popping enabled, the tunneled packet's TTL is unchanged after the pop. |

## TERM: Terminology

### RFC3443-TERM-1

**An LSR performs no checks on the iTTL value.**

> "iTTL: The TTL value to use as the incoming TTL.  No checks are
> performed on the iTTL." — §2.3, `rfc3443.txt:139-140`

- Strength: description. Class: internal.
- Check idea: give an LSR an incoming packet with an unusual TTL value, for example a very low
  one, and check that the LSR does not reject the packet on account of that incoming value alone.

### RFC3443-TERM-2

**The oTTL value equals the iTTL value minus one, unless a later rule states otherwise.**

> "oTTL: This is the TTL value used as the outgoing TTL value (see
> section 3.5 for exception).  It is always (iTTL - 1) unless otherwise
> stated." — §2.3, `rfc3443.txt:142-144`

- Strength: description. Class: internal.
- Check idea: give an LSR an incoming packet with a known TTL value and check that the value it
  later uses as the outgoing TTL is exactly one less, unless a specific rule states another value.

### RFC3443-TERM-3

**An LSR does not forward a packet whose oTTL value is not greater than zero.**

> "oTTL Check: Check if oTTL is greater than 0.  If the oTTL Check is
> false, then the packet is not forwarded." — §2.3, `rfc3443.txt:146-147`

- Strength: description. Class: wire.
- Check idea: engineer an incoming packet whose computed oTTL value is zero and check that the
  LSR does not forward it onto any outgoing link.

### RFC3443-TERM-4

**An LSR performs the oTTL check only when it sets an outgoing TTL field to oTTL.**

> "Note that the oTTL check is
> performed only if any outgoing TTL (either IP or MPLS) is set to oTTL
> (see section 3.5 for exception)." — §2.3, `rfc3443.txt:147-149`

- Strength: description. Class: internal.
- Check idea: engineer a case where no outgoing TTL field is set to oTTL, for example the Short
  Pipe Model penultimate-hop-pop case, and check that the LSR skips the oTTL check there.

## MOD: TTL processing in the three models

### RFC3443-MOD-1

**For a Uniform Model LSP, the inner and the outer TTL values are synchronized at the LSP ingress and at the LSP egress.**

> "This picture shows TTL processing for a Uniform Model MPLS LSP.  Note
> that the inner and outer TTLs of the packets are synchronized at
> tunnel ingress and egress." — §3.1, `rfc3443.txt:194-196`

- Strength: description. Class: wire.
- Check idea: send a packet across a Uniform Model LSP and check that the TTL seen at the LSP
  egress equals the TTL that the same path would show without the tunnel.

### RFC3443-MOD-2

**For a Short Pipe Model LSP with penultimate hop popping, the egress LSR only decrements the exposed header's TTL, because the penultimate node already popped the label.**

> "Since the label has already been popped by the LSP's penultimate
> node, the LSP egress node just decrements the header TTL." — §3.2.2, `rfc3443.txt:247-248`

- Strength: description. Class: wire.
- Check idea: configure a Short Pipe Model LSP with penultimate hop popping, send a packet across
  it, and check that the egress LSR only decrements the exposed header's TTL by one.

### RFC3443-MOD-3

**For a Short Pipe Model LSP, the tunneled packet's TTL decreases by two in total, with or without penultimate hop popping.**

> "Also note that at the end of the Short Pipe Model LSP, the TTL of the
> tunneled packet has been decremented by two, with or without PHP." — §3.2.2, `rfc3443.txt:250-251`

- Strength: description. Class: wire.
- Check idea: send a packet across a Short Pipe Model LSP, once with penultimate hop popping and
  once without, and check that the tunneled packet's TTL is two less at the LSP egress in both cases.

### RFC3443-MOD-4

**For a Pipe Model LSP without penultimate hop popping, the TTL treatment equals the Short Pipe Model without penultimate hop popping.**

> "From the TTL perspective, the treatment for a Pipe Model LSP is
> identical to the Short Pipe Model without PHP." — §3.3, `rfc3443.txt:269-270`

- Strength: description. Class: wire.
- Check idea: send the same packet across a Pipe Model LSP and across a Short Pipe Model LSP,
  neither with penultimate hop popping, and check that the TTL values match at every point.

## ITTL: Incoming TTL determination

### RFC3443-ITTL-1

**When the incoming packet is an IP packet, the iTTL equals that packet's TTL value.**

> "If the incoming packet is an IP packet, then the iTTL is the TTL
> value of the incoming IP packet." — §3.4, `rfc3443.txt:289-290`

- Strength: description. Class: internal.
- Check idea: send an IP packet with a known TTL value into an LSR and check that the LSR uses
  that same value as the iTTL.

### RFC3443-ITTL-2

**When an LSR pushes, swaps, or penultimate-hop-pops an incoming MPLS packet, the iTTL equals the TTL of the topmost incoming label.**

> "If the incoming packet is an MPLS packet and we are performing a
> Push/Swap/PHP, then the iTTL is the TTL of the topmost incoming
> label." — §3.4, `rfc3443.txt:292-294`

- Strength: description. Class: internal.
- Check idea: send an MPLS packet with a known topmost-label TTL into an LSR that pushes, swaps,
  or pops the penultimate label, and check that the LSR uses that TTL as the iTTL.

### RFC3443-ITTL-3

**When an LSR pops a label of a Pipe Model LSP, the iTTL equals the TTL field of the header exposed after the pop, whether that header is an IP header or an MPLS label.**

> "If the popped label belonged to
> a Pipe Model LSP, then the iTTL is the value of the TTL field of the
> header, exposed after the label was popped" — §3.4, `rfc3443.txt:298-300`; "the exposed
> header may be either an IP header or an
> MPLS label" — §3.4, `rfc3443.txt:301-302`

- Strength: may (lower case). Class: internal.
- Check idea: send a packet across a Pipe Model LSP and, at the LSR that pops the tunnel label,
  check that the iTTL equals the TTL field of the header exposed after the pop, whether that
  header is an IP header or another MPLS label.

### RFC3443-ITTL-4

**When an LSR pops a label of a Uniform Model LSP, the iTTL equals the TTL of the popped label.**

> "If the popped label belonged to a Uniform Model LSP,
> then the iTTL is equal to the TTL of the popped label." — §3.4, `rfc3443.txt:302-303`

- Strength: description. Class: internal.
- Check idea: send a packet across a Uniform Model LSP and, at the LSR that pops the tunnel
  label, check that the iTTL equals the TTL of the popped label.

### RFC3443-ITTL-5

**When an LSR pops two or more labels in sequence, it uses the iTTL computed at the previous pop as the TTL of the next label popped, and ignores that label's own TTL.**

> "If multiple
> Pop operations are performed sequentially, then the procedure given
> above is repeated with one exception: the iTTL computed during the
> previous Pop is used as the TTL of subsequent labels being popped;
> i.e. the TTL contained in the subsequent label is essentially ignored
> and replaced with the iTTL computed during the previous pop." — §3.4, `rfc3443.txt:303-308`

- Strength: description. Class: internal.
- Check idea: build a hierarchy of nested LSPs so an LSR pops two or more labels at once, and
  check that the iTTL computed after the first pop, not the TTL carried in the next label,
  becomes the TTL of the label popped next.

## OTTL: Outgoing TTL and packet processing

### RFC3443-OTTL-1

**When an LSR routes a packet as an IP packet, it sets the IP packet's TTL value to oTTL.**

> "If the packet was routed as an IP packet, the TTL value of the IP
> packet is set to oTTL (iTTL - 1)." — §3.5, `rfc3443.txt:317-318`

- Strength: description. Class: wire.
- Check idea: send a packet that an LSR routes as a plain IP packet and check that the outgoing
  IP packet's TTL equals iTTL minus one.

### RFC3443-OTTL-2

**For a swap-only operation, an LSR sets the TTL field of the outgoing label to oTTL.**

> "1) Swap-only: The routed label is swapped with another label and the
> TTL field of the outgoing label is set to oTTL." — §3.5, `rfc3443.txt:323-324`

- Strength: description. Class: wire.
- Check idea: send a packet that an LSR swaps without a push and check that the TTL field of the
  outgoing (swapped) label equals oTTL.

### RFC3443-OTTL-3

**For a penultimate hop pop, an LSR performs the oTTL check whether or not the oTTL is used to update the outgoing header's TTL field.**

> "The oTTL
> check should be performed irrespective of whether the oTTL is used
> to update the TTL field of the outgoing header." — §3.5, `rfc3443.txt:330-332`

- Strength: should (lower case). Class: internal.
- Check idea: configure a penultimate hop pop whose exposed header's TTL is not updated (a Short
  Pipe Model LSP) and check that the LSR still performs the oTTL check before forwarding.

### RFC3443-OTTL-4

**For a penultimate hop pop of a Short Pipe Model label, an LSR neither checks nor updates the TTL field of the exposed header.**

> "If the PHPed
> label belonged to a Short Pipe Model LSP, then the TTL field of
> the PHP exposed header is neither checked nor updated." — §3.5, `rfc3443.txt:332-334`

- Strength: description. Class: wire.
- Check idea: configure a penultimate hop pop of a Short Pipe Model label and check that the TTL
  field of the exposed header is unchanged and does not affect whether the packet is forwarded.

### RFC3443-OTTL-5

**For a penultimate hop pop of a Uniform Model label, an LSR sets the TTL field of the exposed header to oTTL.**

> "If the
> PHPed label was a Uniform Model LSP, then the TTL field of the PHP
> exposed header is set to the oTTL." — §3.5, `rfc3443.txt:334`, `rfc3443.txt:343-344`

- Strength: description. Class: wire.
- Check idea: configure a penultimate hop pop of a Uniform Model label and check that the TTL
  field of the exposed header equals oTTL.

## PUSH: Tunnel ingress processing

### RFC3443-PUSH-1

**For a pushed Uniform Model label, an LSR copies the TTL from the label or the IP packet immediately underneath it.**

> "For each pushed Uniform Model label, the TTL is copied from the
> label/IP-packet immediately underneath it." — §3.6, `rfc3443.txt:352-353`

- Strength: description. Class: wire.
- Check idea: push a Uniform Model label onto a packet and check that the new label's TTL equals
  the TTL of the label or IP packet immediately below it, before the push.

### RFC3443-PUSH-2

**For a pushed Pipe Model or Short Pipe Model label, an LSR sets the TTL field to a value the network operator configures.**

> "For each pushed Pipe Model or Short Pipe Model label, the TTL field
> is set to a value configured by the network operator." — §3.6, `rfc3443.txt:355-356`

- Strength: description. Class: wire.
- Check idea: configure an operator TTL value for a Pipe Model or a Short Pipe Model tunnel,
  push its ingress label, and check that the pushed label's TTL equals the configured value.

### RFC3443-PUSH-3

**By default, most implementations set a pushed Pipe Model or Short Pipe Model label's TTL to 255.**

> "In most
> implementations, this value is set to 255 by default." — §3.6, `rfc3443.txt:356-357`

- Strength: description. Class: wire.
- Check idea: push a Pipe Model or a Short Pipe Model label with no operator-configured TTL value
  and check that the pushed label's TTL is 255.

## IMPL: Implementation remarks

### RFC3443-IMPL-1

**An LSR may decrement iTTL, or determine oTTL, by more than one, only to compensate for network nodes that cannot decrement TTL values themselves.**

> "Although iTTL can be decremented by a value larger than 1 while it
> is being updated or oTTL is being determined, this feature should
> be only used for compensating for network nodes that are not
> capable of decrementing TTL values." — §3.7, `rfc3443.txt:361-364`

- Strength: should (lower case). Class: wire.
- Check idea: connect an LSR to a node that never decrements TTL and check that the LSR may
  decrement the iTTL, or the value it uses to determine oTTL, by more than one to compensate.

### RFC3443-IMPL-2

**When an LSR decrements the iTTL, it never lets the value become negative.**

> "Whenever iTTL is decremented, the implementer must make sure that
> the value does not become negative." — §3.7, `rfc3443.txt:366-367`

- Strength: must (lower case). Class: internal.
- Check idea: engineer an iTTL value close to the lower bound and check that the LSR's
  decremented result never goes below zero.

### RFC3443-IMPL-3

**For a Short Pipe Model LSP with penultimate hop popping enabled, the tunneled packet's TTL is unchanged after the pop.**

> "In the Short Pipe Model with PHP enabled, the TTL of the tunneled
> packet is unchanged after the PHP operation." — §3.7, `rfc3443.txt:369-370`

- Strength: description. Class: wire.
- Check idea: configure a Short Pipe Model LSP with penultimate hop popping enabled, send a
  packet, and check that the tunneled packet's TTL is the same immediately before and immediately
  after the pop.

## Out of scope in this catalog

The entries above hold every `MUST`, `MUST NOT`, `SHOULD`, `SHOULD NOT` and `MAY` of
§2 and §3, and every field of every header. What the catalog leaves out:

- **The sections outside the in-scope set**: §1 (introduction) and §4 to §9 (conclusion,
  security, references, acknowledgements, addresses).

Within the in-scope sections, these parts have no entry, with the reason:

In §2 and §3:

- `rfc3443.txt:74-89` — §2 and §2.1: states what RFC 3032 (MPLS-ENCAPS) did not cover (the Pipe
  Model, the Short Pipe Model, hierarchical LSPs, penultimate hop popping) and that this document
  adds it; a changelog of this document's own scope, not a rule of LSR behavior.
- `rfc3443.txt:90-98` — restates the MPLS shim header fields (Label, TTL, Bottom of stack,
  Experimental bits) "as defined in [MPLS-ENCAPS]"; the fields themselves are cataloged from
  RFC 3032 directly.
- `rfc3443.txt:100-102` — the Experimental bits were later redefined in RFC 3270 (MPLS-DS); a
  statement about another RFC, outside this range.
- `rfc3443.txt:104-123` — general definitions of the Pipe Model, the Short Pipe Model, and the
  Uniform Model (which LSP nodes are visible outside the tunnel), restated from RFC 3270; the
  TTL-specific rule for each model is in §3 (area MOD).
- `rfc3443.txt:125-128` — organizational sentence: TTL processing is broken into iTTL
  determination and outgoing-TTL packet processing; describes the document's own structure.
- `rfc3443.txt:130-135` — signaling the LSP type is out of scope of this document and not
  addressed by LDP or RSVP-TE; the LSP type is set by manual operator configuration. About
  label-distribution-protocol signaling and configuration, not the forwarding of a labeled packet.
- `rfc3443.txt:136-138` — blank lines and the heading "2.3. New Terminology"; heading only.
- `rfc3443.txt:151-155` — introduces §3 as covering the three models in the presence/absence of
  PHP; organizational.
- `rfc3443.txt:156-193` — heading and diagram for §3.1 (Uniform Model); the diagram's outcome is
  stated in RFC3443-MOD-1.
- `rfc3443.txt:197-215` — headings §3.2/§3.2.1 and the diagram for the Short Pipe Model without
  PHP; no additional sentence beyond the diagram.
- `rfc3443.txt:216-218` — the Short Pipe Model was introduced in RFC 3270, whose forwarding
  treatment at the egress LSR is based on the tunneled packet; restates the model's general
  definition, not a TTL rule.
- `rfc3443.txt:219-246` — heading §3.2.2 and the diagram for the Short Pipe Model with PHP.
- `rfc3443.txt:252-268` — heading §3.3 and the diagram for the Pipe Model.
- `rfc3443.txt:271-286` — blank lines and the page break to the end of §3.3.
- `rfc3443.txt:296-298` — "If the incoming packet is an MPLS packet and we are performing a Pop
  (tunnel termination), the iTTL is based on the tunnel type (Pipe or Uniform) of the LSP that was
  popped."; a lead-in to the two specific rules RFC3443-ITTL-3 and RFC3443-ITTL-4, not a rule of
  its own.
- `rfc3443.txt:312-316` — introduces §3.5's procedure (iTTL computed, then the oTTL check, then
  the outgoing TTL); restates the drop rule of RFC3443-TERM-3.
- `rfc3443.txt:318-319` — "The TTL value(s) for any pushed label(s) is determined as described in
  section 3.6."; a cross-reference to the PUSH area, no independent rule.
- `rfc3443.txt:321` — "For packets that are routed as MPLS, we have four cases:"; organizational
  lead-in.
- `rfc3443.txt:326-329` — case 2 (swap followed by a push) only cross-references case 1 and
  section 3.6; no new rule.
- `rfc3443.txt:330` — "The routed label is popped." (opening of case 3); restates the
  penultimate-hop-pop operation already defined in RFC 3031 (assignment A, area PHP).
- `rfc3443.txt:344-345` — "The TTL value(s) of additional labels are determined as described in
  section 3.6"; a cross-reference, no independent rule.
- `rfc3443.txt:347-348` — case 4 (Pop) states the pop happens before routing and "is not
  considered here"; a scoping note. The pop's iTTL rule is in the ITTL area (§3.4).
